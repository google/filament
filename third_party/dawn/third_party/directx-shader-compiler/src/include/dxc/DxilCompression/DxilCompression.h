///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxilCompression.h                                                         //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////
//
// Helper wrapper functions for zlib deflate and inflate. Entirely
// self-contained, only depends on IMalloc interface.
//

#pragma once
#include <cstddef>
struct IMalloc;
namespace hlsl {
enum class ZlibResult {
  Success = 0,
  InvalidData = 1,
  OutOfMemory = 2,
};

ZlibResult ZlibDecompress(IMalloc *pMalloc, const void *pCompressedBuffer,
                          size_t BufferSizeInBytes, void *pUncompressedBuffer,
                          size_t UncompressedBufferSize);

//
// This is a user-provided callback function. The compression routine does
// not need to know how the destination data is being managed. For example:
// appending to an std::vector.
//
// During compression, the routine will call this callback to request the
// amount of memory required for the next segment, and then write to it.
//
// See DxilCompressionHelpers for example of usage.
//
typedef void *ZlibCallbackFn(void *pUserData, size_t RequiredSize);

ZlibResult ZlibCompress(IMalloc *pMalloc, const void *pData, size_t pDataSize,
                        void *pUserData, ZlibCallbackFn *Callback,
                        size_t *pOutCompressedSize);
} // namespace hlsl
