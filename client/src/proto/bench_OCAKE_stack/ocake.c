#include "ocake.h"


extern uint32_t fd;

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

    shake256(dk, sizeof dk, password, password_length);
    crypto_kem_keypair(pk, sk);
    memcpy(apk, pk, sizeof pk);
    AES_init_ctx_iv(&ctx, dk, iv);
    AES_CBC_encrypt_buffer(&ctx, apk, sizeof pk);
    send(fd, apk, sizeof apk);

    recv(fd, ct, sizeof ct);
    crypto_kem_dec(ss, ct, sk);

    uint32_t tmp = 0;
    send(fd, (uint8_t *) &tmp, sizeof tmp);

    recv(fd, tag_server, sizeof tag_server);

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
    send(fd, tag_client, sizeof tag_client);
    sha3_512_inc_finalize(tag_client, &ctx_sha512_s);

    if(!memcmp(tag_client, tag_server, sizeof tag_client)){
        shake256_inc_init(&ctx_shake256);
        shake256_inc_absorb(&ctx_shake256, ss, sizeof ss);
        shake256_inc_absorb(&ctx_shake256, tag_client, sizeof tag_client);
        shake256_inc_finalize(&ctx_shake256);
        shake256_inc_squeeze(session_key, session_key_length, &ctx_shake256);
        shake256_inc_ctx_release(&ctx_shake256);
    }

    return 0;
}