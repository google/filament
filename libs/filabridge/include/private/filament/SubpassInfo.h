/*
 * Copyright (C) 2020 The Android Open Source Project
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

#ifndef TNT_FILAMENT_SUBPASSINFO_H
#define TNT_FILAMENT_SUBPASSINFO_H

#include <backend/DriverEnums.h>

#include <utils/CString.h>

namespace filament {

using Type = backend::SubpassType;
using Format = backend::SamplerFormat;
using Precision = backend::Precision;

struct SubpassInfo {
    SubpassInfo() = default;
    SubpassInfo(utils::CString block, utils::CString name, Type type, Format format,
            Precision precision, uint8_t attachmentIndex, uint8_t binding) noexcept
            : block(std::move(block)), name(std::move(name)), type(type), format(format),
            precision(precision), attachmentIndex(attachmentIndex), binding(binding),
            isValid(true) {
    }
    // name of the block this subpass belongs to
    utils::CString block = utils::CString("MaterialParams");
    utils::CString name;    // name of this subpass
    Type type;              // type of this subpass
    Format format;          // format of this subpass
    Precision precision;    // precision of this subpass
    uint8_t attachmentIndex = 0;
    uint8_t binding = 0;
    bool isValid = false;
};

} // namespace filament

#endif // TNT_FILAMENT_SUBPASSINFO_H
