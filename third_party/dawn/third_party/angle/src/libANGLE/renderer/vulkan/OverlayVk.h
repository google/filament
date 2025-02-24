//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// OverlayVk.h:
//    Defines the OverlayVk class that does the actual rendering of the overlay.
//

#ifndef LIBANGLE_RENDERER_VULKAN_OVERLAYVK_H_
#define LIBANGLE_RENDERER_VULKAN_OVERLAYVK_H_

#include "common/angleutils.h"
#include "libANGLE/Overlay.h"
#include "libANGLE/renderer/OverlayImpl.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"

namespace rx
{
class ContextVk;

class OverlayVk : public OverlayImpl
{
  public:
    OverlayVk(const gl::OverlayState &state);
    ~OverlayVk() override;

    void onDestroy(const gl::Context *context) override;

    angle::Result onPresent(ContextVk *contextVk,
                            vk::ImageHelper *imageToPresent,
                            const vk::ImageView *imageToPresentView,
                            bool is90DegreeRotation);

    uint32_t getEnabledWidgetCount() const { return mState.getEnabledWidgetCount(); }

  private:
    angle::Result createFont(ContextVk *contextVk);

    vk::ImageHelper mFontImage;
    vk::ImageView mFontImageView;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_OVERLAYVK_H_
