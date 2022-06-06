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

#include <gltfio/MaterialProvider.h>

#include <string>

namespace filament::gltfio {

bool operator==(const MaterialKey& k1, const MaterialKey& k2) {
    return
        (k1.doubleSided == k2.doubleSided) &&
        (k1.unlit == k2.unlit) &&
        (k1.hasVertexColors == k2.hasVertexColors) &&
        (k1.hasBaseColorTexture == k2.hasBaseColorTexture) &&
        (k1.hasNormalTexture == k2.hasNormalTexture) &&
        (k1.hasOcclusionTexture == k2.hasOcclusionTexture) &&
        (k1.hasEmissiveTexture == k2.hasEmissiveTexture) &&
        (k1.useSpecularGlossiness == k2.useSpecularGlossiness) &&
        (k1.alphaMode == k2.alphaMode) &&
        (k1.hasMetallicRoughnessTexture == k2.hasMetallicRoughnessTexture) &&
        (k1.metallicRoughnessUV == k2.metallicRoughnessUV) &&
        (k1.baseColorUV == k2.baseColorUV) &&
        (k1.hasClearCoatTexture == k2.hasClearCoatTexture) &&
        (k1.clearCoatUV == k2.clearCoatUV) &&
        (k1.hasClearCoatRoughnessTexture == k2.hasClearCoatRoughnessTexture) &&
        (k1.clearCoatRoughnessUV == k2.clearCoatRoughnessUV) &&
        (k1.hasClearCoatNormalTexture == k2.hasClearCoatNormalTexture) &&
        (k1.clearCoatNormalUV == k2.clearCoatNormalUV) &&
        (k1.hasClearCoat == k2.hasClearCoat) &&
        (k1.hasTransmission == k2.hasTransmission) &&
        (k1.hasTextureTransforms == k2.hasTextureTransforms) &&
        (k1.emissiveUV == k2.emissiveUV) &&
        (k1.aoUV == k2.aoUV) &&
        (k1.normalUV == k2.normalUV) &&
        (k1.hasTransmissionTexture == k2.hasTransmissionTexture) &&
        (k1.transmissionUV == k2.transmissionUV) &&
        (k1.hasSheenColorTexture == k2.hasSheenColorTexture) &&
        (k1.sheenColorUV == k2.sheenColorUV) &&
        (k1.hasSheenRoughnessTexture == k2.hasSheenRoughnessTexture) &&
        (k1.sheenRoughnessUV == k2.sheenRoughnessUV) &&
        (k1.hasVolumeThicknessTexture == k2.hasVolumeThicknessTexture) &&
        (k1.volumeThicknessUV == k2.volumeThicknessUV) &&
        (k1.hasSheen == k2.hasSheen) &&
        (k1.hasIOR == k2.hasIOR) &&
        (k1.hasVolume == k2.hasVolume);
}

// Filament supports up to 2 UV sets. glTF has arbitrary texcoord set indices, but it allows
// implementations to support only 2 simultaneous sets. Here we build a mapping table with 1-based
// indices where 0 means unused. Note that the order in which we drop textures can affect the look
// of certain assets. This "order of degradation" is stipulated by the glTF 2.0 specification.
void constrainMaterial(MaterialKey* key, UvMap* uvmap) {
    const int MAX_INDEX = 2;
    UvMap retval {};
    int index = 1;
    if (key->hasBaseColorTexture) {
        retval[key->baseColorUV] = (UvSet) index++;
    }
    if (key->hasMetallicRoughnessTexture && retval[key->metallicRoughnessUV] == UNUSED) {
        retval[key->metallicRoughnessUV] = (UvSet) index++;
    }
    if (key->hasNormalTexture && retval[key->normalUV] == UNUSED) {
        if (index > MAX_INDEX) {
            key->hasNormalTexture = false;
        } else {
            retval[key->normalUV] = (UvSet) index++;
        }
    }
    if (key->hasOcclusionTexture && retval[key->aoUV] == UNUSED) {
        if (index > MAX_INDEX) {
            key->hasOcclusionTexture = false;
        } else {
            retval[key->aoUV] = (UvSet) index++;
        }
    }
    if (key->hasEmissiveTexture && retval[key->emissiveUV] == UNUSED) {
        if (index > MAX_INDEX) {
            key->hasEmissiveTexture = false;
        } else {
            retval[key->emissiveUV] = (UvSet) index++;
        }
    }
    if (key->hasTransmissionTexture && retval[key->transmissionUV] == UNUSED) {
        if (index > MAX_INDEX) {
            key->hasTransmissionTexture = false;
        } else {
            retval[key->transmissionUV] = (UvSet) index++;
        }
    }
    if (key->hasClearCoatTexture && retval[key->clearCoatUV] == UNUSED) {
        if (index > MAX_INDEX) {
            key->hasClearCoatTexture = false;
        } else {
            retval[key->clearCoatUV] = (UvSet) index++;
        }
    }
    if (key->hasClearCoatRoughnessTexture && retval[key->clearCoatRoughnessUV] == UNUSED) {
        if (index > MAX_INDEX) {
            key->hasClearCoatRoughnessTexture = false;
        } else {
            retval[key->clearCoatRoughnessUV] = (UvSet) index++;
        }
    }
    if (key->hasClearCoatNormalTexture && retval[key->clearCoatNormalUV] == UNUSED) {
        if (index > MAX_INDEX) {
            key->hasClearCoatNormalTexture = false;
        } else {
            retval[key->clearCoatNormalUV] = (UvSet) index++;
        }
    }
    if (key->hasSheenColorTexture && retval[key->sheenColorUV] == UNUSED) {
        if (index > MAX_INDEX) {
            key->hasSheenColorTexture = false;
        } else {
            retval[key->sheenColorUV] = (UvSet) index++;
        }
    }
    if (key->hasSheenRoughnessTexture && retval[key->sheenRoughnessUV] == UNUSED) {
        if (index > MAX_INDEX) {
            key->hasSheenRoughnessTexture = false;
        } else {
            retval[key->sheenRoughnessUV] = (UvSet) index++;
        }
    }
    if (key->hasVolumeThicknessTexture && retval[key->volumeThicknessUV] == UNUSED) {
        if (index > MAX_INDEX) {
            key->hasVolumeThicknessTexture = false;
        } else {
            retval[key->volumeThicknessUV] = (UvSet) index++;
        }
    }
    // NOTE: KHR_materials_clearcoat does not provide separate UVs, we'll assume UV0
    *uvmap = retval;
}

void processShaderString(std::string* shader, const UvMap& uvmap, const MaterialKey& config) {
    auto replaceAll = [shader](const std::string& from, const std::string& to) {
        size_t pos = shader->find(from);
        for (; pos != std::string::npos; pos = shader->find(from, pos)) {
            shader->replace(pos, from.length(), to);
        }
    };
    static const std::string uvstrings[] = { "vec2(0)", "getUV0()", "getUV1()" };
    const auto& normalUV = uvstrings[uvmap[config.normalUV]];
    const auto& baseColorUV = uvstrings[uvmap[config.baseColorUV]];
    const auto& metallicRoughnessUV = uvstrings[uvmap[config.metallicRoughnessUV]];
    const auto& emissiveUV = uvstrings[uvmap[config.emissiveUV]];
    const auto& transmissionUV = uvstrings[uvmap[config.transmissionUV]];
    const auto& aoUV = uvstrings[uvmap[config.aoUV]];
    const auto& clearCoatUV = uvstrings[uvmap[config.clearCoatUV]];
    const auto& clearCoatRoughnessUV = uvstrings[uvmap[config.clearCoatRoughnessUV]];
    const auto& clearCoatNormalUV = uvstrings[uvmap[config.clearCoatNormalUV]];
    const auto& sheenColorUV = uvstrings[uvmap[config.sheenColorUV]];
    const auto& sheenRoughnessUV = uvstrings[uvmap[config.sheenRoughnessUV]];
    const auto& volumeThicknessUV = uvstrings[uvmap[config.volumeThicknessUV]];

    replaceAll("${normal}", normalUV);
    replaceAll("${color}", baseColorUV);
    replaceAll("${metallic}", metallicRoughnessUV);
    replaceAll("${ao}", aoUV);
    replaceAll("${emissive}", emissiveUV);
    replaceAll("${transmission}", transmissionUV);
    replaceAll("${clearCoat}", clearCoatUV);
    replaceAll("${clearCoatRoughness}", clearCoatRoughnessUV);
    replaceAll("${clearCoatNormal}", clearCoatNormalUV);
    replaceAll("${sheenColor}", sheenColorUV);
    replaceAll("${sheenRoughness}", sheenRoughnessUV);
    replaceAll("${volumeThickness}", volumeThicknessUV);
}

} // namespace filament::gltfio
