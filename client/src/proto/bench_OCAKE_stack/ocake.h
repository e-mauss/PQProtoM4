#ifndef OCAKE_H
#define OCAKE_H

#include "stdint.h"
#include "api.h"
#include "hal.h"
#include "fips202.h"
#include "aes.h"
#include "string.h"
#include "bench.h"

static const uint8_t iv[16]  = { 
  0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff };

int32_t ocake_pake_sessionkey(uint8_t *session_key, uint32_t session_key_length, uint8_t *password, uint32_t password_length);

#endif
