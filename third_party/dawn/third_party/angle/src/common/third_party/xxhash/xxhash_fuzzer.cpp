//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// xxHash Fuzzer test:
//      Integration with Chromium's libfuzzer for xxHash.

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "xxhash.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
#if !defined(XXH_NO_LONG_LONG)
    // Test 64-bit hash.
    unsigned long long seed64 = 0ull;
    size_t seedSize64         = sizeof(seed64);
    if (size >= seedSize64)
    {
        memcpy(&seed64, data, seedSize64);
    }
    else
    {
        seedSize64 = 0;
    }
    XXH64(&data[seedSize64], size - seedSize64, seed64);
#endif  // !defined(XXH_NO_LONG_LONG)

    // Test 32-bit hash.
    unsigned int seed32 = 0u;
    size_t seedSize32   = sizeof(seed32);
    if (size >= seedSize32)
    {
        memcpy(&seed32, data, seedSize32);
    }
    else
    {
        seedSize32 = 0;
    }
    XXH32(&data[seedSize32], size - seedSize32, seed32);
    return 0;
}
