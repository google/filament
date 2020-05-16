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

#include "details/DFG.h"

#include "details/Engine.h"
#include "details/Texture.h"

namespace filament {

const uint16_t DFG::DFG_LUT[] = {
#include "generated/data/dfg.inc"
};

DFG::DFG(FEngine& engine) noexcept : mEngine(engine) {
    constexpr size_t fp16Count = DFG_LUT_SIZE * DFG_LUT_SIZE * 3;
    constexpr size_t byteCount = fp16Count * sizeof(uint16_t);

    static_assert(sizeof(DFG_LUT) == byteCount, "DFG_LUT_SIZE doesn't match size of the DFG LUT");

    Texture* lut = Texture::Builder()
            .width(DFG_LUT_SIZE)
            .height(DFG_LUT_SIZE)
            .format(backend::TextureFormat::RGB16F)
            .build(mEngine);

    Texture::PixelBufferDescriptor buffer(DFG_LUT, byteCount,
            Texture::Format::RGB, Texture::Type::HALF);

    lut->setImage(mEngine, 0, std::move(buffer));

    mLUT = upcast(lut);
}

void DFG::terminate() {
    if (mLUT) {
        mEngine.destroy(mLUT);
    }
}

} // namespace filament
