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

const uint16_t LTC::LTC_LUT[] = {
#include "ltc.inc"
};

void LTC::init(FEngine& engine) noexcept {
    constexpr size_t fp16Count = LTC_LUT_SIZE * LTC_LUT_SIZE * 4 * 2;
    constexpr size_t byteCount = fp16Count * sizeof(uint16_t);

    static_assert(sizeof(LTC_LUT) == byteCount, "LTC_LUT_SIZE doesn't match size of the LTC LUT");

    Texture* lut = Texture::Builder()
            .width(LTC_LUT_SIZE)
            .height(LTC_LUT_SIZE)
            .depth(2)
            .sampler(Texture::Sampler::SAMPLER_2D_ARRAY)
            .format(backend::TextureFormat::RGBA16F)
            .build(engine);

    Texture::PixelBufferDescriptor buffer(LTC_LUT, byteCount,
            Texture::Format::RGBA, Texture::Type::HALF);

    lut->setImage(engine, 0, 0, 0, 0, LTC_LUT_SIZE, LTC_LUT_SIZE, 2, std::move(buffer));

    mLUT = downcast(lut);
}

void LTC::terminate(FEngine& engine) noexcept {
    if (mLUT) {
        engine.destroy(mLUT);
    }
}

}  // namespace filament
