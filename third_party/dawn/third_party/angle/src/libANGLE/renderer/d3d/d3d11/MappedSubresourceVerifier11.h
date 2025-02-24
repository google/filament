//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// MappedSubresourceVerifier11.h: Defines the rx::MappedSubresourceVerifier11
// class, a simple wrapper to D3D11 Texture2D mapped memory so that ASAN
// MSAN can catch memory errors done with a pointer to the mapped texture
// memory.

#ifndef LIBANGLE_RENDERER_D3D_D3D11_MAPPED_SUBRESOURCE_VERIFIER11_H_
#define LIBANGLE_RENDERER_D3D_D3D11_MAPPED_SUBRESOURCE_VERIFIER11_H_

#include "common/MemoryBuffer.h"
#include "common/angleutils.h"

namespace rx
{

class MappedSubresourceVerifier11 final : angle::NonCopyable
{
  public:
    MappedSubresourceVerifier11();
    ~MappedSubresourceVerifier11();

    void setDesc(const D3D11_TEXTURE2D_DESC &desc);
    void setDesc(const D3D11_TEXTURE3D_DESC &desc);
    void reset();

    bool wrap(D3D11_MAP mapType, D3D11_MAPPED_SUBRESOURCE *map);
    void unwrap();

  private:
#if defined(ADDRESS_SANITIZER) || defined(MEMORY_SANITIZER) || defined(ANGLE_ENABLE_ASSERTS)
    UINT D3D11_MAPPED_SUBRESOURCE::*mPitchType = nullptr;
    size_t mPitchCount                         = 0;
    angle::MemoryBuffer mWrapData;
    uint8_t *mOrigData = nullptr;
#endif
};

#if !(defined(ADDRESS_SANITIZER) || defined(MEMORY_SANITIZER) || defined(ANGLE_ENABLE_ASSERTS))

inline MappedSubresourceVerifier11::MappedSubresourceVerifier11()  = default;
inline MappedSubresourceVerifier11::~MappedSubresourceVerifier11() = default;

inline void MappedSubresourceVerifier11::setDesc(const D3D11_TEXTURE2D_DESC &desc) {}
inline void MappedSubresourceVerifier11::setDesc(const D3D11_TEXTURE3D_DESC &desc) {}
inline void MappedSubresourceVerifier11::reset() {}

inline bool MappedSubresourceVerifier11::wrap(D3D11_MAP mapType, D3D11_MAPPED_SUBRESOURCE *map)
{
    return true;
}

inline void MappedSubresourceVerifier11::unwrap() {}

#endif

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D11_MAPPED_SUBRESOURCE_VERIFIER11_H_
