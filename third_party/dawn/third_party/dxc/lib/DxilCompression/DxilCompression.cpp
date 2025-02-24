///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxilCompression.cpp                                                       //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////
//
// Helper wrapper functions for zlib deflate and inflate. Entirely
// self-contained, only depends on IMalloc interface.
//

#include "dxc/DxilCompression/DxilCompression.h"
#include "dxc/Support/Global.h"
#include "dxc/Support/WinIncludes.h"

#include "miniz.h"
typedef size_t ZlibSize_t;
typedef const Bytef ZlibInputBytesf;

namespace {
//
// A resource managment class for a zlib stream that calls the appropriate init
// and end routines.
//
class Zlib {
public:
  enum Operation { INFLATE, DEFLATE };
  Zlib(Operation Op, IMalloc *pAllocator)
      : m_Stream{}, m_Op(Op), m_Initalized(false) {
    m_Stream = {};

    if (pAllocator) {
      m_Stream.zalloc = ZAlloc;
      m_Stream.zfree = ZFree;
      m_Stream.opaque = pAllocator;
    }

    int ret = Z_ERRNO;
    if (Op == INFLATE) {
      ret = inflateInit(&m_Stream);
    } else {
      ret = deflateInit(&m_Stream, Z_DEFAULT_COMPRESSION);
    }

    if (ret != Z_OK) {
      m_InitializationResult = TranslateZlibResult(ret);
      return;
    }

    m_Initalized = true;
  }

  ~Zlib() {
    if (m_Initalized) {
      if (m_Op == INFLATE) {
        inflateEnd(&m_Stream);
      } else {
        deflateEnd(&m_Stream);
      }
    }
  }

  z_stream *GetStream() {
    if (m_Initalized)
      return &m_Stream;
    return nullptr;
  }

  hlsl::ZlibResult GetInitializationResult() const {
    return m_InitializationResult;
  }

  static hlsl::ZlibResult TranslateZlibResult(int zlibResult) {
    switch (zlibResult) {
    default:
      return hlsl::ZlibResult::InvalidData;
    case Z_MEM_ERROR:
    case Z_BUF_ERROR:
      return hlsl::ZlibResult::OutOfMemory;
    }
  }

private:
  z_stream m_Stream;
  Operation m_Op;
  bool m_Initalized;
  hlsl::ZlibResult m_InitializationResult = hlsl::ZlibResult::Success;

  static void *ZAlloc(void *context, ZlibSize_t items, ZlibSize_t size) {
    IMalloc *mallocif = (IMalloc *)context;
    return mallocif->Alloc(items * size);
  }

  static void ZFree(void *context, void *pointer) {
    IMalloc *mallocif = (IMalloc *)context;
    mallocif->Free(pointer);
  }
};

} // namespace

hlsl::ZlibResult hlsl::ZlibDecompress(IMalloc *pMalloc,
                                      const void *pCompressedBuffer,
                                      size_t BufferSizeInBytes,
                                      void *pUncompressedBuffer,
                                      size_t UncompressedBufferSize) {
  Zlib zlib(Zlib::INFLATE, pMalloc);
  z_stream *pStream = zlib.GetStream();
  if (!pStream)
    return zlib.GetInitializationResult();

  pStream->avail_in = BufferSizeInBytes;
  pStream->next_in = (ZlibInputBytesf *)pCompressedBuffer;
  pStream->next_out = (Byte *)pUncompressedBuffer;
  pStream->avail_out = UncompressedBufferSize;

  // Compression should finish in one call because of the call to deflateBound.
  int status = inflate(pStream, Z_FINISH);
  if (status != Z_STREAM_END) {
    return Zlib::TranslateZlibResult(status);
  }

  return ZlibResult::Success;
}

hlsl::ZlibResult hlsl::ZlibCompress(IMalloc *pMalloc, const void *pData,
                                    size_t pDataSize, void *pUserData,
                                    ZlibCallbackFn *Callback,
                                    size_t *pOutCompressedSize) {
  Zlib zlib(Zlib::DEFLATE, pMalloc);
  z_stream *pStream = zlib.GetStream();
  if (!pStream)
    return zlib.GetInitializationResult();

  const size_t UpperBound = deflateBound(pStream, pDataSize);
  void *pDestBuffer = Callback(pUserData, UpperBound);
  if (!pDestBuffer)
    return ZlibResult::OutOfMemory;

  pStream->next_in = (ZlibInputBytesf *)pData;
  pStream->avail_in = pDataSize;
  pStream->next_out = (Byte *)pDestBuffer;
  pStream->avail_out = UpperBound;

  int status = deflate(pStream, Z_FINISH);
  if (status != Z_STREAM_END) {
    return Zlib::TranslateZlibResult(status);
  }

  *pOutCompressedSize = pStream->total_out;
  return ZlibResult::Success;
}
