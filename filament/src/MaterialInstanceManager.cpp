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


#include "MaterialInstanceManager.h"

#include "details/Engine.h"
#include "details/Material.h"
#include "details/MaterialInstance.h"

#include <utils/debug.h>

namespace filament {

using Record = MaterialInstanceManager::Record;

std::pair<FMaterialInstance*, int32_t> Record::getInstance() {
    if (mAvailable < mInstances.size()) {
        auto index = mAvailable++;
        return { mInstances[index], index };
    }
    assert_invariant(mAvailable == mInstances.size());
    auto& name = mMaterial->getName();
    FMaterialInstance* inst = mMaterial->createInstance(name.c_str_safe());
    mInstances.push_back(inst);
    return { inst, mAvailable++ };
}

FMaterialInstance* Record::getInstance(int32_t fixedInstanceindex) {
    assert_invariant(fixedInstanceindex >= 0 &&  fixedInstanceindex < (int32_t) mInstances.size());
    return mInstances[fixedInstanceindex];
}

// Defined in cpp to avoid inlining
Record::Record(Record const& rhs) noexcept = default;
Record& Record::operator=(Record const& rhs) noexcept = default;
Record::Record(MaterialInstanceManager::Record&& rhs) noexcept = default;
Record& Record::operator=(Record&& rhs) noexcept = default;

void Record::terminate(FEngine& engine) {
    std::for_each(mInstances.begin(), mInstances.end(),
            [&engine](auto instance) { engine.destroy(instance); });
}

MaterialInstanceManager::MaterialInstanceManager() noexcept {}

MaterialInstanceManager::MaterialInstanceManager(
        MaterialInstanceManager const& rhs) noexcept = default;
MaterialInstanceManager::MaterialInstanceManager(MaterialInstanceManager&& rhs) noexcept = default;
MaterialInstanceManager& MaterialInstanceManager::operator=(
        MaterialInstanceManager const& rhs) noexcept = default;
MaterialInstanceManager& MaterialInstanceManager::operator=(
        MaterialInstanceManager&& rhs) noexcept = default;

MaterialInstanceManager::~MaterialInstanceManager() = default;

void MaterialInstanceManager::terminate(FEngine& engine) {
    std::for_each(mMaterials.begin(), mMaterials.end(), [&engine](auto& record) {
        record.terminate(engine);
    });
}

Record& MaterialInstanceManager::getRecord(FMaterial const* ma) {
    auto itr = std::find_if(mMaterials.begin(), mMaterials.end(), [ma](auto& record) {
        return ma == record.mMaterial;
    });
    if (itr == mMaterials.end()) {
        mMaterials.emplace_back(ma);
        itr = std::prev(mMaterials.end());
    }
    return *itr;
}

FMaterialInstance* MaterialInstanceManager::getMaterialInstance(FMaterial const* ma) {
    auto [inst, index] = getRecord(ma).getInstance();
    return inst;
}

FMaterialInstance* MaterialInstanceManager::getMaterialInstance(FMaterial const* ma,
        int32_t const fixedIndex) {
    return getRecord(ma).getInstance(fixedIndex);
}

std::pair<FMaterialInstance*, int32_t> MaterialInstanceManager::getFixedMaterialInstance(
        FMaterial const* ma) {
    return getRecord(ma).getInstance();
}


} // namespace filament
