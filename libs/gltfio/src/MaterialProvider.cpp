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

#include <string.h>

namespace filament::gltfio {

// MaterialKey is always kept properly padded/packed. It is safe to memcmp it directly.
bool operator==(const MaterialKey& k1, const MaterialKey& k2) {
    return memcmp(&k1, &k2, sizeof(MaterialKey)) == 0;
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

    key->hasRetroreflectionTexture = constrainTexture(
            key->retroreflectionUV, key->hasRetroreflectionTexture, "retroreflectionTexture");
    key->retroreflectionUV = clampUv(key->retroreflectionUV);

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
    const auto& retroreflectionUV = getUvString(config.retroreflectionUV);

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
    replaceAll("${retroreflection}", retroreflectionUV);
}

} // namespace filament::gltfio
