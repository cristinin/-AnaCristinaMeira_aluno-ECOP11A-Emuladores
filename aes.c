#include "aes.h"
#include <string.h>

static uint8_t xtime(uint8_t x);


static const uint8_t sbox[256] = {
    
};

static void KeyExpansion(uint8_t* RoundKey, const uint8_t* Key);
static void AddRoundKey(uint8_t round, uint8_t* state, const uint8_t* RoundKey);
static void SubBytes(uint8_t* state);
static void ShiftRows(uint8_t* state);
static void MixColumns(uint8_t* state);

static void KeyExpansion(uint8_t* RoundKey, const uint8_t* Key) {
    memcpy(RoundKey, Key, 16);
}

static void AddRoundKey(uint8_t round, uint8_t* state, const uint8_t* RoundKey) {
    for (int i = 0; i < 16; ++i) {
        state[i] ^= RoundKey[i];
    }
}

static void SubBytes(uint8_t* state) {
    for (int i = 0; i < 16; ++i) {
        state[i] = sbox[state[i]];
    }
}

static void ShiftRows(uint8_t* state) {
    uint8_t tmp;

    tmp = state[1];
    state[1] = state[5];
    state[5] = state[9];
    state[9] = state[13];
    state[13] = tmp;

    tmp = state[2];
    state[2] = state[10];
    state[10] = tmp;
    tmp = state[6];
    state[6] = state[14];
    state[14] = tmp;

    tmp = state[3];
    state[3] = state[15];
    state[15] = state[11];
    state[11] = state[7];
    state[7] = tmp;
}

static void MixColumns(uint8_t* state) {
    for (int i = 0; i < 16; i += 4) {
        uint8_t t = state[i] ^ state[i + 1] ^ state[i + 2] ^ state[i + 3];
        uint8_t tmp = state[i];
        state[i] ^= t ^ xtime(state[i] ^ state[i + 1]);
        state[i + 1] ^= t ^ xtime(state[i + 1] ^ state[i + 2]);
        state[i + 2] ^= t ^ xtime(state[i + 2] ^ state[i + 3]);
        state[i + 3] ^= t ^ xtime(state[i + 3] ^ tmp);
    }
}

static uint8_t xtime(uint8_t x) {
    return (x << 1) ^ (((x >> 7) & 1) * 0x1b);
}

void AES_init_ctx(struct AES_ctx* ctx, const uint8_t* key) {
    KeyExpansion(ctx->RoundKey, key);
}

void AES_ECB_encrypt(const struct AES_ctx* ctx, uint8_t* buf) {
    uint8_t round = 0;
    AddRoundKey(0, buf, ctx->RoundKey);

    for (round = 1; round < 10; ++round) {
        SubBytes(buf);
        ShiftRows(buf);
        MixColumns(buf);
        AddRoundKey(round, buf, ctx->RoundKey);
    }

    SubBytes(buf);
    ShiftRows(buf);
    AddRoundKey(10, buf, ctx->RoundKey);
}
