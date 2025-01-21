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

#include "details/RenderTarget.h"

#include "details/Texture.h"

namespace filament {

using namespace backend;

Texture* RenderTarget::getTexture(AttachmentPoint const attachment) const noexcept {
    return downcast(this)->getAttachment(attachment).texture;
}

uint8_t RenderTarget::getMipLevel(AttachmentPoint const attachment) const noexcept {
    return downcast(this)->getAttachment(attachment).mipLevel;
}

RenderTarget::CubemapFace RenderTarget::getFace(AttachmentPoint const attachment) const noexcept {
    return downcast(this)->getAttachment(attachment).face;
}

uint32_t RenderTarget::getLayer(AttachmentPoint const attachment) const noexcept {
    return downcast(this)->getAttachment(attachment).layer;
}

uint8_t RenderTarget::getSupportedColorAttachmentsCount() const noexcept {
    return downcast(this)->getSupportedColorAttachmentsCount();
}

} // namespace filament
