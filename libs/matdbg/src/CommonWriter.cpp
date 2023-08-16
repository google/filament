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

#include "CommonWriter.h"

#include <private/filament/EngineEnums.h>
#include <private/filament/Variant.h>

namespace filament::matdbg {

std::string formatVariantString(Variant variant, MaterialDomain domain) noexcept {
    if (domain == MaterialDomain::COMPUTE) {
        // Currently we have no variants for compute materials
        return "---";
    }

    if (domain == MaterialDomain::POST_PROCESS) {
        switch (PostProcessVariant{variant.key}) {
            case PostProcessVariant::OPAQUE: return "OPA";
            case PostProcessVariant::TRANSLUCENT: return "TRN";
        }
        return "UNK";
    }

    std::string variantString = "";

    // NOTE: The 3-character nomenclature used here is consistent with the ASCII art seen in the
    // Variant header file and allows the information to fit in a reasonable amount of space on
    // the page. The HTML file has a legend.
    if (variant.key) {
        if (variant.key & Variant::DIR) variantString += "DIR|";
        if (variant.key & Variant::DYN) variantString += "DYN|";
        if (variant.key & Variant::SRE) variantString += "SRE|";
        if (variant.key & Variant::SKN) variantString += "SKN|";
        if (variant.key & Variant::DEP) variantString += "DEP|";
        if (variant.key & Variant::DEP) {
            if (variant.key & Variant::PCK) variantString += "PCK|";
        } else {
            if (variant.key & Variant::FOG) variantString += "FOG|";
        }
        if (variant.key & Variant::VSM) variantString += "VSM|";
        if (variant.key & Variant::STE) variantString += "STE|";
        variantString = variantString.substr(0, variantString.length() - 1);
    }

    return variantString;
}

} // namespace filament::matdbg
