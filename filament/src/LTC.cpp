/*
 * Copyright (C) 2016 The Android Open Source Project
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

#include "LTC.h"

#include "details/Engine.h"
#include "details/Texture.h"

namespace filament {

#include "ltc.inc"

void LTC::init(FEngine& engine) noexcept {
    constexpr size_t fp16Count = LTC_LUT_SIZE * LTC_LUT_SIZE * 4;
    constexpr size_t byteCount = fp16Count * sizeof(uint16_t);

    static_assert(sizeof(LTC_LUT_1) == byteCount, "LTC_LUT_SIZE doesn't match size of the LTC LUT 1");

    Texture* lut1 = Texture::Builder()
                            .width(LTC_LUT_SIZE)
                            .height(LTC_LUT_SIZE)
                            .format(backend::TextureFormat::RGBA16F)
                            .build(engine);

    Texture::PixelBufferDescriptor buffer1(LTC_LUT_1, byteCount,
            Texture::Format::RGBA, Texture::Type::HALF);

    lut1->setImage(engine, 0, std::move(buffer1));

    mLUT1 = downcast(lut1);

    static_assert(sizeof(LTC_LUT_2) == byteCount, "LTC_LUT_SIZE doesn't match size of the LTC LUT 2");

    Texture* ltc2 = Texture::Builder()
                            .width(LTC_LUT_SIZE)
                            .height(LTC_LUT_SIZE)
                            .format(backend::TextureFormat::RGBA16F)
                            .build(engine);

    Texture::PixelBufferDescriptor buffer2(LTC_LUT_2, byteCount,
            Texture::Format::RGBA, Texture::Type::HALF);

    ltc2->setImage(engine, 0, std::move(buffer2));

    mLUT2 = downcast(ltc2);
}

void LTC::terminate(FEngine& engine) noexcept {
    if (mLUT1) {
        engine.destroy(mLUT1);
    }

    if (mLUT2) {
        engine.destroy(mLUT2);
    }
}

}  // namespace filament
