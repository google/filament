//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// StreamProducerD3DTexture.h: Interface for a D3D11 texture stream producer

#ifndef LIBANGLE_RENDERER_D3D_D3D11_STREAM11_H_
#define LIBANGLE_RENDERER_D3D_D3D11_STREAM11_H_

#include "libANGLE/renderer/StreamProducerImpl.h"

namespace rx
{
class Renderer11;

class StreamProducerD3DTexture : public StreamProducerImpl
{
  public:
    StreamProducerD3DTexture(Renderer11 *renderer);
    ~StreamProducerD3DTexture() override;

    egl::Error validateD3DTexture(const void *pointer,
                                  const egl::AttributeMap &attributes) const override;
    void postD3DTexture(void *pointer, const egl::AttributeMap &attributes) override;
    egl::Stream::GLTextureDescription getGLFrameDescription(int planeIndex) override;

    // Gets a pointer to the internal D3D texture
    ID3D11Texture2D *getD3DTexture();

    // Gets the slice index for the D3D texture that the frame is in
    UINT getArraySlice();

  private:
    Renderer11 *mRenderer;

    ID3D11Texture2D *mTexture;
    UINT mArraySlice;
    UINT mPlaneOffset;
};
}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D11_STREAM11_H_
