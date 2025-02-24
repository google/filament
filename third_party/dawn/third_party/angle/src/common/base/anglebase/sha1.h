// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ANGLEBASE_SHA1_H_
#define ANGLEBASE_SHA1_H_

#include <stddef.h>
#include <stdint.h>

#include <array>
#include <string>

#include "anglebase/base_export.h"

namespace angle
{

namespace base
{

// These functions perform SHA-1 operations.

static const size_t kSHA1Length = 20;  // Length in bytes of a SHA-1 hash.

// Computes the SHA-1 hash of the input string |str| and returns the full
// hash.
ANGLEBASE_EXPORT std::string SHA1HashString(const std::string &str);

// Computes the SHA-1 hash of the |len| bytes in |data| and puts the hash
// in |hash|. |hash| must be kSHA1Length bytes long.
ANGLEBASE_EXPORT void SHA1HashBytes(const unsigned char *data, size_t len, unsigned char *hash);

// Implementation of SHA-1. Only handles data in byte-sized blocks,
// which simplifies the code a fair bit.

// Identifier names follow notation in FIPS PUB 180-3, where you'll
// also find a description of the algorithm:
// http://csrc.nist.gov/publications/fips/fips180-3/fips180-3_final.pdf

// Usage example:
//
// SecureHashAlgorithm sha;
// while(there is data to hash)
//   sha.Update(moredata, size of data);
// sha.Final();
// memcpy(somewhere, sha.Digest(), 20);
//
// to reuse the instance of sha, call sha.Init();

// TODO(jhawkins): Replace this implementation with a per-platform
// implementation using each platform's crypto library.  See
// http://crbug.com/47218

class SecureHashAlgorithm
{
  public:
    SecureHashAlgorithm() { Init(); }

    static const int kDigestSizeBytes;

    void Init();
    void Update(const void *data, size_t nbytes);
    void Final();

    // 20 bytes of message digest.
    const unsigned char *Digest() const { return reinterpret_cast<const unsigned char *>(H); }

    std::array<uint8_t, kSHA1Length> DigestAsArray() const;

  private:
    void Pad();
    void Process();

    uint32_t A, B, C, D, E;

    uint32_t H[5];

    union
    {
        uint32_t W[80];
        uint8_t M[64];
    };

    uint32_t cursor;
    uint64_t l;
};

}  // namespace base

}  // namespace angle

#endif  // ANGLEBASE_SHA1_H_
