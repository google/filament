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
        (k1.hasVolume == k2.hasVolume) &&
        (k1.hasSpecular == k2.hasSpecular) &&
        (k1.hasSpecularTexture == k2.hasSpecularTexture) &&
        (k1.hasSpecularColorTexture == k2.hasSpecularColorTexture) &&
        (k1.specularTextureUV == k2.specularTextureUV) &&
        (k1.specularColorTextureUV == k2.specularColorTextureUV)
        ;
}

// Filament supports up to 2 UV sets. glTF has arbitrary texcoord set indices, but it allows
// implementations to support only 2 simultaneous sets. Here we build a mapping table with 1-based
// indices where 0 means unused. Note that the order in which we drop textures can affect the look
// of certain assets. This "order of degradation" is stipulated by the glTF 2.0 specification.
void constrainMaterial(MaterialKey* key, UvMap* uvmap) {
    const int MAX_INDEX = 2;
    UvMap retval {};
    int index = 1;
    auto updateUv = [&retval, &index](uint8_t uv) -> bool {
        if (uv >= UvMapSize) {
            return false;
        }
        if (retval[uv] == UNUSED) {
            if (index > MAX_INDEX) {
                return false;
            }
            retval[uv] = (UvSet) index++;
        }
        return true;
    };
    if (key->hasBaseColorTexture) {
        key->hasBaseColorTexture = updateUv(key->baseColorUV);
    }
    if (key->hasMetallicRoughnessTexture) {
        key->hasMetallicRoughnessTexture = updateUv(key->metallicRoughnessUV);
    }
    if (key->hasNormalTexture) {
        key->hasNormalTexture = updateUv(key->normalUV);
    }
    if (key->hasOcclusionTexture) {
        key->hasOcclusionTexture = updateUv(key->aoUV);
    }
    if (key->hasEmissiveTexture) {
        key->hasEmissiveTexture = updateUv(key->emissiveUV);
    }
    if (key->hasTransmissionTexture) {
        key->hasTransmissionTexture = updateUv(key->transmissionUV);
    }
    if (key->hasClearCoatTexture) {
        key->hasClearCoatTexture = updateUv(key->clearCoatUV);
    }
    if (key->hasClearCoatRoughnessTexture) {
        key->hasClearCoatRoughnessTexture = updateUv(key->clearCoatRoughnessUV);
    }
    if (key->hasClearCoatNormalTexture) {
        key->hasClearCoatNormalTexture = updateUv(key->clearCoatNormalUV);
    }
    if (key->hasSheenColorTexture) {
        key->hasSheenColorTexture = updateUv(key->sheenColorUV);
    }
    if (key->hasSheenRoughnessTexture) {
        key->hasSheenRoughnessTexture = updateUv(key->sheenRoughnessUV);
    }
    if (key->hasVolumeThicknessTexture) {
        key->hasVolumeThicknessTexture = updateUv(key->volumeThicknessUV);
    }
    if (key->hasSpecularTexture) {
        key->hasSpecularTexture = updateUv(key->specularTextureUV);
    }
    if (key->hasSpecularColorTexture) {
        key->hasSpecularColorTexture = updateUv(key->specularColorTextureUV);
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
    auto getUvString = [&uvmap](uint8_t uv) -> const std::string& {
        if (uv >= UvMapSize) {
            return uvstrings[0];
        }
        UvSet const set = uvmap[uv];
        return set < UvSetSize ? uvstrings[set] : uvstrings[0];
    };
    const auto& normalUV = getUvString(config.normalUV);
    const auto& baseColorUV = getUvString(config.baseColorUV);
    const auto& metallicRoughnessUV = getUvString(config.metallicRoughnessUV);
    const auto& emissiveUV = getUvString(config.emissiveUV);
    const auto& transmissionUV = getUvString(config.transmissionUV);
    const auto& aoUV = getUvString(config.aoUV);
    const auto& clearCoatUV = getUvString(config.clearCoatUV);
    const auto& clearCoatRoughnessUV = getUvString(config.clearCoatRoughnessUV);
    const auto& clearCoatNormalUV = getUvString(config.clearCoatNormalUV);
    const auto& sheenColorUV = getUvString(config.sheenColorUV);
    const auto& sheenRoughnessUV = getUvString(config.sheenRoughnessUV);
    const auto& volumeThicknessUV = getUvString(config.volumeThicknessUV);
    const auto& specularUV = getUvString(config.specularTextureUV);
    const auto& specularColorUV = getUvString(config.specularColorTextureUV);

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
    replaceAll("${specular}", specularUV);
    replaceAll("${specularColor}", specularColorUV);
}

} // namespace filament::gltfio
