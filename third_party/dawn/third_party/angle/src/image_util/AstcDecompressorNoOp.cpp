//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// AstcDecompressorNoOp.cpp: No-op implementation if support for ASTC textures wasn't enabled

#include "image_util/AstcDecompressor.h"

namespace angle
{

namespace
{

class AstcDecompressorNoOp : public AstcDecompressor
{
  public:
    bool available() const override { return false; }

    int32_t decompress(std::shared_ptr<WorkerThreadPool> singleThreadPool,
                       std::shared_ptr<WorkerThreadPool> multiThreadPool,
                       uint32_t imgWidth,
                       uint32_t imgHeight,
                       uint32_t blockWidth,
                       uint32_t blockHeight,
                       const uint8_t *astcData,
                       size_t astcDataLength,
                       uint8_t *output) override
    {
        return -1;
    }

    const char *getStatusString(int32_t statusCode) const override
    {
        return "ASTC CPU decomp not available";
    }
};

}  // namespace

AstcDecompressor &AstcDecompressor::get()
{
    static AstcDecompressorNoOp *instance = new AstcDecompressorNoOp();
    return *instance;
}

}  // namespace angle
