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

#include "DependencyGraph.h"

using namespace filament;
using namespace utils;

namespace gltfio {

size_t DependencyGraph::popRenderables(Entity* result, size_t count) noexcept {
    if (result == nullptr) {
        return mReadyRenderables.size();
    }
    size_t numWritten = 0;
    while (numWritten < count && !mReadyRenderables.empty()) {
        result[numWritten++] = mReadyRenderables.front();
        mReadyRenderables.pop();
    }
    return numWritten;
}

void DependencyGraph::addEdge(Entity entity, MaterialInstance* mi) {

    // Permit adding an Entity-Material edge to a finalized graph as long as the material is already
    // known. Since we already encountered this material instance, we already know what textures it
    // is associated with.
    assert(!mFinalized || mMaterialToEntity.find(mi) != mMaterialToEntity.end());

    mMaterialToEntity[mi].insert(entity);
    mEntityToMaterial[entity].materials.insert(mi);
}

void DependencyGraph::addEdge(MaterialInstance* mi, const char* parameter) {
    assert(!mFinalized);
    mMaterialToTexture[mi].params[parameter] = nullptr;
}

// During finalization, the structure of the glTF is known but we have not yet created texture
// objects. Find all non-textured entities and immediately add mark them as ready.
void DependencyGraph::finalize() {
    assert(!mFinalized);
    for (auto pair : mMaterialToEntity) {
        auto mi = pair.first;
        if (mMaterialToTexture.find(mi) == mMaterialToTexture.end()) {
            markAsReady(mi);
        }
    }
    mFinalized = true;
}

void DependencyGraph::refinalize() {
    assert(mFinalized);
    for (auto pair : mMaterialToEntity) {
        auto material = pair.first;
        if (mMaterialToTexture.find(material) == mMaterialToTexture.end()) {
            markAsReady(material);
        } else {
            checkReadiness(material);
        }
    }
}

void DependencyGraph::addEdge(Texture* texture, MaterialInstance* material, const char* parameter) {
    assert(mFinalized);
    mTextureToMaterial[texture].insert(material);
    mMaterialToTexture.at(material).params.at(parameter) = getStatus(texture);
}

void DependencyGraph::checkReadiness(Material* material) {
    auto& status = mMaterialToTexture.at(material);

    // Check this material's texture parameters, there are 5 in the worst case.
    bool materialIsReady = true;
    for (auto pair : status.params) {
        if (!pair.second->ready) {
            materialIsReady = false;
            break;
        }
    }

    // If all of its textures are ready, then the material has become ready.
    if (materialIsReady) {
        markAsReady(material);
    }
}

void DependencyGraph::markAsReady(Texture* texture) {
    assert(texture && mFinalized);
    mTextureNodes.at(texture)->ready = true;

    // Iterate over the materials associated with this texture to check if any have become ready.
    // This is O(n2) but the inner loop is always small.
    auto& materials = mTextureToMaterial.at(texture);
    for (auto material : materials) {
        checkReadiness(material);
    }
}

void DependencyGraph::markAsReady(MaterialInstance* material) {
    auto& entities = mMaterialToEntity.at(material);
    for (auto entity : entities) {
        auto& status = mEntityToMaterial.at(entity);
        assert(status.numReadyMaterials <= status.materials.size());
        if (status.numReadyMaterials == status.materials.size()) {
            continue;
        }
        if (++status.numReadyMaterials == status.materials.size()) {
            mReadyRenderables.push(entity);
        }
    }
}

DependencyGraph::TextureNode* DependencyGraph::getStatus(Texture* texture) {
    auto iter = mTextureNodes.find(texture);
    if (iter == mTextureNodes.end()) {
        TextureNode* status = (mTextureNodes[texture] = std::make_unique<TextureNode>()).get();
        *status = {texture, false};
        return status;
    }
    return iter->second.get();
}

} // namespace gltfio
