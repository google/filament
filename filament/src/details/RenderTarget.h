/*
 * Copyright (C) 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TNT_FILAMENT_DETAILS_RENDERTAGET_H
#define TNT_FILAMENT_DETAILS_RENDERTAGET_H

#include "downcast.h"

#include <backend/Handle.h>

#include <filament/RenderTarget.h>

#include <utils/compiler.h>

namespace filament {

class FEngine;
class FTexture;

class FRenderTarget : public RenderTarget {
public:
    using HwHandle = backend::Handle<backend::HwRenderTarget>;

    struct Attachment {
        FTexture* texture = nullptr;
        uint8_t mipLevel = 0;
        CubemapFace face = RenderTarget::CubemapFace::POSITIVE_X;
        uint32_t layer = 0;
        // Indicates the number of layers used for multiview, starting from the `layer` (baseIndex).
        // This means `layer` + `layerCount` cannot exceed the number of depth for the attachment.
        uint16_t layerCount = 0;
    };

    FRenderTarget(FEngine& engine, const Builder& builder);

    void terminate(FEngine& engine);

    HwHandle getHwHandle() const noexcept { return mHandle; }

    Attachment getAttachment(AttachmentPoint attachment) const noexcept {
        return mAttachments[(int) attachment];
    }

    backend::TargetBufferFlags getAttachmentMask() const noexcept {
        return mAttachmentMask;
    }

    backend::TargetBufferFlags getSampleableAttachmentsMask() const noexcept {
        return mSampleableAttachmentsMask;
    }

    uint8_t getSupportedColorAttachmentsCount() const noexcept {
        return mSupportedColorAttachmentsCount;
    }

    bool hasSampleableDepth() const noexcept;

    bool supportsReadPixels() const noexcept {
        return mSupportsReadPixels;
    }

private:
    friend class RenderTarget;
    static constexpr size_t ATTACHMENT_COUNT = MAX_SUPPORTED_COLOR_ATTACHMENTS_COUNT + 1u;
    Attachment mAttachments[ATTACHMENT_COUNT];
    HwHandle mHandle{};
    backend::TargetBufferFlags mAttachmentMask = {};
    backend::TargetBufferFlags mSampleableAttachmentsMask = {};
    const uint8_t mSupportedColorAttachmentsCount;
    bool mSupportsReadPixels = false;
};

FILAMENT_DOWNCAST(RenderTarget)

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_RENDERTARGET_H
