/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include "ssao.h"

#include "generated/resources/ssao.h"

#include <materials/StaticMaterialInfo.h>

#include <utils/Slice.h>

#include <iterator>

#include <stddef.h>

namespace filament {

static const StaticMaterialInfo sMaterialList[] = {
    { "bilateralBlur",              MATERIAL(SSAO, BILATERALBLUR) },
    { "bilateralBlurBentNormals",   MATERIAL(SSAO, BILATERALBLURBENTNORMALS) },
    { "mipmapDepth",                MATERIAL(SSAO, MIPMAPDEPTH) },
    { "sao",                        MATERIAL(SSAO, SAO) },
    { "saoBentNormals",             MATERIAL(SSAO, SAOBENTNORMALS) },
#ifndef FILAMENT_DISABLE_GTAO
    { "gtao",                       MATERIAL(SSAO, GTAO) },
    { "gtaoBentNormals",            MATERIAL(SSAO, GTAOBENTNORMALS) },
#endif
};

utils::Slice<StaticMaterialInfo> getSsaoMaterialList() noexcept {
    return { std::begin(sMaterialList), std::end(sMaterialList) };
}

} // namespace filament
