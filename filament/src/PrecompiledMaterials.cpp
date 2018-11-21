/*
 * Copyright (C) 2018 The Android Open Source Project
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

#include "PrecompiledMaterials.h"

#include <stdint.h>

namespace filament {
namespace details {

// This package is generated with matc and contains default material shader code.
const uint8_t DEFAULT_MATERIAL_PACKAGE[] = {
#include "generated/material/defaultMaterial.inc"
};
const size_t DEFAULT_MATERIAL_PACKAGE_SIZE = sizeof(DEFAULT_MATERIAL_PACKAGE);

} // namespace details
} //namespace filament
