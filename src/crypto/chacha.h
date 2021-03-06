#pragma once

#include <stdint.h>
#include <stddef.h>

#define CHACHA8_KEY_SIZE 32
#define CHACHA8_IV_SIZE 8

#if defined(__cplusplus)
#include <memory.h>
#include <string>

#include "hash.h"

namespace crypto {
  extern "C" {
#endif
    void chacha8(const void* data, size_t length, const uint8_t* key, const uint8_t* iv, char* cipher);
#if defined(__cplusplus)
  }

#pragma pack(push, 1)
  struct chacha_key {
    uint8_t data[CHACHA8_KEY_SIZE];

    ~chacha_key()
    {
      memset(data, 0, sizeof(data));
    }
  };

  // MS VC 2012 doesn't interpret `class chacha_iv` as POD in spite of [9.0.10], so it is a struct
  struct chacha_iv {
    uint8_t data[CHACHA8_IV_SIZE];
  };
#pragma pack(pop)

  static_assert(sizeof(chacha_key) == CHACHA8_KEY_SIZE && sizeof(chacha_iv) == CHACHA8_IV_SIZE, "Invalid structure size");

  inline void chacha8(const void* data, size_t length, const chacha_key& key, const chacha_iv& iv, char* cipher) {
    chacha8(data, length, reinterpret_cast<const uint8_t*>(&key), reinterpret_cast<const uint8_t*>(&iv), cipher);
  }

  inline void generate_chacha_key(const std::string& password, chacha_key& key) {
    static_assert(sizeof(chacha_key) <= sizeof(Hash), "Size of hash must be at least that of chacha_key");
    Hash pwd_hash;
    cn_slow_hash(password.data(), password.size(), (char *)&pwd_hash, 0, 0);
    memcpy(&key, &pwd_hash, sizeof(key));
    memset(&pwd_hash, 0, sizeof(pwd_hash));
  }
}

#endif
