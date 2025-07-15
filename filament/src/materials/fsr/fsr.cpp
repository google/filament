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

#include "fsr.h"

#include "generated/resources/fsr.h"

#include <materials/StaticMaterialInfo.h>

#include <utils/Slice.h>

#include <iterator>

#include <stddef.h>

namespace filament {

static const StaticMaterialInfo sMaterialList[] = {
    { "fsr_easu",                   MATERIAL(FSR, FSR_EASU) },
    { "fsr_easu_mobile",            MATERIAL(FSR, FSR_EASU_MOBILE) },
    { "fsr_easu_mobileF",           MATERIAL(FSR, FSR_EASU_MOBILEF) },
    { "fsr_rcas",                   MATERIAL(FSR, FSR_RCAS) },
};

utils::Slice<StaticMaterialInfo> getFsrMaterialList() noexcept {
    return { std::begin(sMaterialList), std::end(sMaterialList) };
}

} // namespace filament
