#include "ocake.h"


extern int fd;

int32_t ocake_pake_sessionkey(uint8_t *session_key, uint32_t session_key_length, uint8_t *password, uint32_t password_length) {
    uint8_t dk[AES_KEYLEN];
    uint8_t sk[CRYPTO_SECRETKEYBYTES];
    uint8_t pk[CRYPTO_PUBLICKEYBYTES];
    uint8_t apk[CRYPTO_PUBLICKEYBYTES];
    uint8_t ct[CRYPTO_CIPHERTEXTBYTES];
    uint8_t ss[CRYPTO_BYTES];
    uint8_t tag_client[64];
    uint8_t tag_server[64];
    uint8_t tag_client_prime[64];
    struct AES_ctx ctx;
    sha3_512incctx ctx_sha512_c;
    sha3_512incctx ctx_sha512_s;
    shake256incctx ctx_shake256;

    shake256(dk, sizeof dk, password, password_length);
    recv(fd, apk, sizeof apk);
    memcpy(pk, apk, sizeof apk);

    AES_init_ctx_iv(&ctx, dk, iv);
    AES_CBC_decrypt_buffer(&ctx, pk, sizeof pk);

    crypto_kem_enc(ct, ss, pk);
    send(fd, ct, sizeof ct);

    uint32_t tmp;
    recv(fd, &tmp, sizeof tmp);

    sha3_512_inc_init(&ctx_sha512_s);
    sha3_512_inc_absorb(&ctx_sha512_s, password, password_length);
    sha3_512_inc_absorb(&ctx_sha512_s, apk, sizeof apk);
    sha3_512_inc_absorb(&ctx_sha512_s, pk, sizeof pk);
    sha3_512_inc_absorb(&ctx_sha512_s, ct, sizeof ct);
    sha3_512_inc_absorb(&ctx_sha512_s, ss, sizeof ss);

    sha3_512_inc_ctx_clone(&ctx_sha512_c, &ctx_sha512_s);
    sha3_512_inc_absorb(&ctx_sha512_s, (uint8_t *) "s", 1);
    sha3_512_inc_absorb(&ctx_sha512_c, (uint8_t *) "c", 1);

    sha3_512_inc_finalize(tag_server, &ctx_sha512_s);
    send(fd, tag_server, sizeof tag_server);

    recv(fd, tag_client, sizeof tag_client);
    sha3_512_inc_finalize(tag_client_prime, &ctx_sha512_c);
    
    if(!memcmp(tag_client, tag_client_prime, sizeof tag_client)){
        shake256_inc_init(&ctx_shake256);
        shake256_inc_absorb(&ctx_shake256, ss, sizeof ss);
        shake256_inc_absorb(&ctx_shake256, tag_server, sizeof tag_server);
        shake256_inc_finalize(&ctx_shake256);
        shake256_inc_squeeze(session_key, session_key_length, &ctx_shake256);
        shake256_inc_ctx_release(&ctx_shake256);
        printf(":)\n");
    }
    
    return 0;
}