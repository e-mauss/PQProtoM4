#include "ocake.h"


extern uint32_t fd;
extern bench_t b;

int32_t ocake_pake_sessionkey(uint8_t *session_key, uint32_t session_key_length, uint8_t *password, uint32_t password_length) {
    uint8_t dk[AES_KEYLEN];
    uint8_t sk[CRYPTO_SECRETKEYBYTES];
    uint8_t pk[CRYPTO_PUBLICKEYBYTES];
    uint8_t apk[CRYPTO_PUBLICKEYBYTES];
    uint8_t ct[CRYPTO_CIPHERTEXTBYTES];
    uint8_t ss[CRYPTO_BYTES];
    uint8_t tag_client[64];
    uint8_t tag_server[64];
    struct AES_ctx ctx;
    sha3_512incctx ctx_sha512_c;
    sha3_512incctx ctx_sha512_s;
    shake256incctx ctx_shake256;
    
    uint64_t proto = hal_get_time();

    uint64_t shake = hal_get_time();
    shake256(dk, sizeof dk, password, password_length);
    shake = hal_get_time() - b.shake;

    uint64_t key_gen = hal_get_time();
    crypto_kem_keypair(pk, sk);
    key_gen = hal_get_time() - key_gen;

    uint64_t other = hal_get_time();
    memcpy(apk, pk, sizeof pk);
    other = hal_get_time() - other;

    uint64_t aes = hal_get_time();
    AES_init_ctx_iv(&ctx, dk, iv);
    AES_CBC_encrypt_buffer(&ctx, apk, sizeof pk);
    aes = hal_get_time() - aes;

    uint64_t comm = hal_get_time();
    send(fd, apk, sizeof apk);
    recv(fd, ct, sizeof ct);
    comm = hal_get_time() - comm;

    uint64_t decap = hal_get_time();
    crypto_kem_dec(ss, ct, sk);
    decap = hal_get_time() - decap;

    uint32_t tmp = 0;
    uint64_t comm2 = hal_get_time();
    send(fd, (uint8_t *) &tmp, sizeof tmp);

    recv(fd, tag_server, sizeof tag_server);
    comm2 = hal_get_time() - comm2;

    uint64_t sha = hal_get_time();
    sha3_512_inc_init(&ctx_sha512_c);
    sha3_512_inc_absorb(&ctx_sha512_c, password, password_length);
    sha3_512_inc_absorb(&ctx_sha512_c, apk, sizeof apk);
    sha3_512_inc_absorb(&ctx_sha512_c, pk, sizeof pk);
    sha3_512_inc_absorb(&ctx_sha512_c, ct, sizeof ct);
    sha3_512_inc_absorb(&ctx_sha512_c, ss, sizeof ss);

    sha3_512_inc_ctx_clone(&ctx_sha512_s, &ctx_sha512_c);
    sha3_512_inc_absorb(&ctx_sha512_c, (uint8_t *) "c", 1);
    sha3_512_inc_absorb(&ctx_sha512_s, (uint8_t *) "s", 1);

    sha3_512_inc_finalize(tag_client, &ctx_sha512_c);
    sha = hal_get_time() - sha;

    uint64_t comm3 = hal_get_time();
    send(fd, tag_client, sizeof tag_client);
    comm3 = hal_get_time() - comm3;

    uint64_t sha2 = hal_get_time();
    sha3_512_inc_finalize(tag_client, &ctx_sha512_s);
    sha += hal_get_time() - sha2;

    uint64_t other2 = hal_get_time();
    int32_t failure = memcmp(tag_client, tag_server, sizeof tag_client);
    other2 = hal_get_time() - other2;
    if(failure){
        uint64_t shake2 = hal_get_time();
        shake256_inc_init(&ctx_shake256);
        shake256_inc_absorb(&ctx_shake256, ss, sizeof ss);
        shake256_inc_absorb(&ctx_shake256, tag_client, sizeof tag_client);
        shake256_inc_finalize(&ctx_shake256);
        shake256_inc_squeeze(session_key, session_key_length, &ctx_shake256);
        shake256_inc_ctx_release(&ctx_shake256);
        shake2 = hal_get_time() - shake2;
    }
    proto = hal_get_time() - proto;

    b.proto += proto;
    b.aes += aes;
    b.key_gen += key_gen;
    b.decap += decap;
    b.sha += sha;
    b.shake += shake;
    b.other += other + other2;
    b.comm += comm + comm2 + comm3;
    return 0;
}