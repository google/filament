//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RenderTargetWgpu.cpp:
//    Implements the class methods for RenderTargetWgpu.
//

#include "libANGLE/renderer/wgpu/RenderTargetWgpu.h"

namespace rx
{
RenderTargetWgpu::RenderTargetWgpu() {}

RenderTargetWgpu::~RenderTargetWgpu()
{
    reset();
}

RenderTargetWgpu::RenderTargetWgpu(RenderTargetWgpu &&other)
    : mImage(other.mImage),
      mTextureView(std::move(other.mTextureView)),
      mLevelIndex(other.mLevelIndex),
      mLayerIndex(other.mLayerIndex),
      mFormat(other.mFormat)
{}

void RenderTargetWgpu::set(webgpu::ImageHelper *image,
                           const wgpu::TextureView &texture,
                           const webgpu::LevelIndex level,
                           uint32_t layer,
                           const wgpu::TextureFormat &format)
{
    mImage       = image;
    mTextureView = texture;
    mLevelIndex  = level;
    mLayerIndex  = layer;
    mFormat      = &format;
}

void RenderTargetWgpu::reset()
{
    mTextureView = nullptr;
    mLevelIndex  = webgpu::LevelIndex(0);
    mLayerIndex  = 0;
    mFormat      = nullptr;
}

angle::Result RenderTargetWgpu::flushImageStagedUpdates(ContextWgpu *contextWgpu,
                                                        webgpu::ClearValuesArray *deferredClears,
                                                        uint32_t deferredClearIndex)
{
    gl::LevelIndex targetLevel = mImage->toGlLevel(mLevelIndex);
    ANGLE_TRY(mImage->flushSingleLevelUpdates(contextWgpu, targetLevel, deferredClears,
                                              deferredClearIndex));
    return angle::Result::Continue;
}
}  // namespace rx
