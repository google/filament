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

#include <utils/Log.h>

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

    auto constrainTexture = [&retval, &index](
            uint8_t uv, bool hasTexture, const char* name) -> bool {
        if (uv >= UvMapSize) {
            if (hasTexture) {
                utils::slog.e << "Texture coordinate index " << int(uv) << " for " << name
                        << " exceeds maximum supported index (" << (UvMapSize - 1) << ")."
                        << utils::io::endl;
            }
            return false;
        }
        if (hasTexture && retval[uv] == UNUSED) {
            if (index > MAX_INDEX) {
                return false;
            }
            retval[uv] = (UvSet) index++;
        }
        return hasTexture;
    };

    auto clampUv = [](uint8_t uv) -> uint8_t {
        return uv < UvMapSize ? uv : 0;
    };

    key->hasBaseColorTexture = constrainTexture(
            key->baseColorUV, key->hasBaseColorTexture, "baseColorTexture");
    key->baseColorUV = clampUv(key->baseColorUV);

    key->hasMetallicRoughnessTexture = constrainTexture(key->metallicRoughnessUV,
            key->hasMetallicRoughnessTexture,
            key->useSpecularGlossiness ? "specularGlossinessTexture" : "metallicRoughnessTexture");
    key->metallicRoughnessUV = clampUv(key->metallicRoughnessUV);

    key->hasNormalTexture = constrainTexture(
            key->normalUV, key->hasNormalTexture, "normalTexture");
    key->normalUV = clampUv(key->normalUV);

    key->hasOcclusionTexture = constrainTexture(
            key->aoUV, key->hasOcclusionTexture, "occlusionTexture");
    key->aoUV = clampUv(key->aoUV);

    key->hasEmissiveTexture = constrainTexture(
            key->emissiveUV, key->hasEmissiveTexture, "emissiveTexture");
    key->emissiveUV = clampUv(key->emissiveUV);

    key->hasTransmissionTexture = constrainTexture(
            key->transmissionUV, key->hasTransmissionTexture, "transmissionTexture");
    key->transmissionUV = clampUv(key->transmissionUV);

    key->hasClearCoatTexture = constrainTexture(
            key->clearCoatUV, key->hasClearCoatTexture, "clearCoatTexture");
    key->clearCoatUV = clampUv(key->clearCoatUV);

    key->hasClearCoatRoughnessTexture = constrainTexture(key->clearCoatRoughnessUV,
            key->hasClearCoatRoughnessTexture, "clearCoatRoughnessTexture");
    key->clearCoatRoughnessUV = clampUv(key->clearCoatRoughnessUV);

    key->hasClearCoatNormalTexture = constrainTexture(
            key->clearCoatNormalUV, key->hasClearCoatNormalTexture, "clearCoatNormalTexture");
    key->clearCoatNormalUV = clampUv(key->clearCoatNormalUV);

    key->hasSheenColorTexture = constrainTexture(
            key->sheenColorUV, key->hasSheenColorTexture, "sheenColorTexture");
    key->sheenColorUV = clampUv(key->sheenColorUV);

    key->hasSheenRoughnessTexture = constrainTexture(
            key->sheenRoughnessUV, key->hasSheenRoughnessTexture, "sheenRoughnessTexture");
    key->sheenRoughnessUV = clampUv(key->sheenRoughnessUV);

    key->hasVolumeThicknessTexture = constrainTexture(
            key->volumeThicknessUV, key->hasVolumeThicknessTexture, "volumeThicknessTexture");
    key->volumeThicknessUV = clampUv(key->volumeThicknessUV);

    key->hasSpecularTexture = constrainTexture(
            key->specularTextureUV, key->hasSpecularTexture, "specularTexture");
    key->specularTextureUV = clampUv(key->specularTextureUV);

    key->hasSpecularColorTexture = constrainTexture(
            key->specularColorTextureUV, key->hasSpecularColorTexture, "specularColorTexture");
    key->specularColorTextureUV = clampUv(key->specularColorTextureUV);

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
    const auto& specularUV = uvstrings[uvmap[config.specularTextureUV]];
    const auto& specularColorUV = uvstrings[uvmap[config.specularColorTextureUV]];

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
