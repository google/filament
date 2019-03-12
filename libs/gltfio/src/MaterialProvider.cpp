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

using namespace gltfio;

bool gltfio::operator==(const MaterialKey& k1, const MaterialKey& k2) {
    return
        (k1.doubleSided == k2.doubleSided) &&
        (k1.unlit == k2.unlit) &&
        (k1.hasVertexColors == k2.hasVertexColors) &&
        (k1.hasBaseColorTexture == k2.hasBaseColorTexture) &&
        (k1.hasMetallicRoughnessTexture == k2.hasMetallicRoughnessTexture) &&
        (k1.hasNormalTexture == k2.hasNormalTexture) &&
        (k1.hasOcclusionTexture == k2.hasOcclusionTexture) &&
        (k1.hasEmissiveTexture == k2.hasEmissiveTexture) &&
        (k1.alphaMode == k2.alphaMode) &&
        (k1.baseColorUV == k2.baseColorUV) &&
        (k1.metallicRoughnessUV == k2.metallicRoughnessUV) &&
        (k1.emissiveUV == k2.emissiveUV) &&
        (k1.aoUV == k2.aoUV) &&
        (k1.normalUV == k2.normalUV) &&
        (k1.alphaMaskThreshold == k2.alphaMaskThreshold);
}

// Filament supports up to 2 UV sets. glTF has arbitrary texcoord set indices, but it allows
// implementations to support only 2 simultaneous sets. Here we build a mapping table with 1-based
// indices where 0 means unused. Note that the order in which we drop textures can affect the look
// of certain assets. This "order of degradation" is stipulated by the glTF 2.0 specification.
void details::constrainMaterial(MaterialKey* key, UvMap* uvmap) {
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
    *uvmap = retval;
}

void details::processShaderString(std::string* shader, const UvMap& uvmap,
        const MaterialKey& config) {
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
    const auto& aoUV = uvstrings[uvmap[config.aoUV]];
    replaceAll("${normal}", normalUV);
    replaceAll("${color}", baseColorUV);
    replaceAll("${metallic}", metallicRoughnessUV);
    replaceAll("${ao}", aoUV);
    replaceAll("${emissive}", emissiveUV);
}
