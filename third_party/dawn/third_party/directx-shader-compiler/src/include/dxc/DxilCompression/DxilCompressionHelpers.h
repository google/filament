///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxilCompressionHelpers.h                                                  //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////
//
// Helper wrapper functions for common buffer types.
//
// Calling ZlibCompressAppend on a buffer appends the compressed data to the
// end. If at any point the compression fails, the buffer will be shrunk to
// the original size.
//

#include "DxilCompression.h"

#include "llvm/ADT/SmallVector.h"
#include <vector>

namespace hlsl {

template <typename Buffer>
ZlibResult ZlibCompressAppend(IMalloc *pMalloc, const void *pData,
                              size_t dataSize, Buffer &outBuffer) {
  static_assert(sizeof(typename Buffer::value_type) == sizeof(uint8_t),
                "Cannot append to a non-byte-sized buffer.");

  // This helper resets the buffer to its original size in case of failure or
  // exception
  class RAIIResizer {
    Buffer &m_Buffer;
    size_t m_OriginalSize;
    bool m_Resize = true;

  public:
    RAIIResizer(Buffer &buffer)
        : m_Buffer(buffer), m_OriginalSize(buffer.size()) {}
    ~RAIIResizer() {
      if (m_Resize) {
        m_Buffer.resize(m_OriginalSize);
      }
    }
    void DoNotResize() { m_Resize = false; }
  };
  RAIIResizer resizer(outBuffer);

  const size_t sizeBeforeCompress = outBuffer.size();
  size_t compressedDataSize = 0;

  ZlibResult ret = ZlibCompress(
      pMalloc, pData, dataSize, &outBuffer,
      [](void *pUserData, size_t requiredSize) -> void * {
        Buffer *pBuffer = (Buffer *)pUserData;
        const size_t lastSize = pBuffer->size();
        pBuffer->resize(pBuffer->size() + requiredSize);
        void *ptr = pBuffer->data() + lastSize;
        return ptr;
      },
      &compressedDataSize);

  if (ret == ZlibResult::Success) {
    // Resize the buffer to what was actually added to the end.
    outBuffer.resize(sizeBeforeCompress + compressedDataSize);
    resizer.DoNotResize();
  }

  return ret;
}

template ZlibResult ZlibCompressAppend<llvm::SmallVectorImpl<char>>(
    IMalloc *pMalloc, const void *pData, size_t dataSize,
    llvm::SmallVectorImpl<char> &outBuffer);
template ZlibResult ZlibCompressAppend<llvm::SmallVectorImpl<uint8_t>>(
    IMalloc *pMalloc, const void *pData, size_t dataSize,
    llvm::SmallVectorImpl<uint8_t> &outBuffer);
template ZlibResult
ZlibCompressAppend<std::vector<char>>(IMalloc *pMalloc, const void *pData,
                                      size_t dataSize,
                                      std::vector<char> &outBuffer);
template ZlibResult
ZlibCompressAppend<std::vector<uint8_t>>(IMalloc *pMalloc, const void *pData,
                                         size_t dataSize,
                                         std::vector<uint8_t> &outBuffer);
} // namespace hlsl
