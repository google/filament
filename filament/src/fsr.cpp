/*
 * Copyright (C) 2021 The Android Open Source Project
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

#include "fsr.h"

#include <math/vec4.h>

#include <math.h>
#include <stdint.h>
#include <stdlib.h>

namespace filament {

using namespace math;

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wignored-qualifiers"
#endif

#define A_CPU  1
#include "materials/fsr/ffx_a.h"
#define FSR_EASU_F 1
#define FSR_RCAS_F 1
#include "materials/fsr/ffx_fsr1.h"

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

void FSR_ScalingSetup(FSRUniforms* outUniforms, FSRScalingConfig config) noexcept {
    // FsrEasu API claims it needs the left-top offset, however that's not true with OpenGL,
    // in which case it uses the left-bottom offset.

    auto yoffset = config.input.bottom;
    if (config.backend == backend::Backend::METAL || config.backend == backend::Backend::VULKAN ||
            config.backend == backend::Backend::WEBGPU) {
        yoffset = config.inputHeight - (config.input.bottom + config.input.height);
    }

    FsrEasuConOffset( outUniforms->EasuCon0.v, outUniforms->EasuCon1.v,
                outUniforms->EasuCon2.v, outUniforms->EasuCon3.v,
            // Viewport size (top left aligned) in the input image which is to be scaled.
            config.input.width, config.input.height,
            // The size of the input image.
            config.inputWidth, config.inputHeight,
            // The output resolution.
            config.outputWidth, config.outputHeight,
            // Input image offset
            config.input.left, yoffset);
}

void FSR_SharpeningSetup(FSRUniforms* outUniforms, FSRSharpeningConfig config) noexcept {
    FsrRcasCon(outUniforms->RcasCon.v, config.sharpness);
}

} // namespace filament


