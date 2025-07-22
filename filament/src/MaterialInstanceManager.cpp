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

namespace filament {

FMaterialInstance* MaterialInstanceManager::Record::getInstance() {
    if (mAvailable.any()) {
        int picked = -1;
        mAvailable.forEachSetBit([&picked](size_t index) {
            if (picked >= 0) {
                return;
            }
            picked = index;
        });
        mAvailable.unset(picked);
        return mInstances[picked];
    }
    auto& name = mMaterial->getName();
    FMaterialInstance* inst = mMaterial->createInstance(name.c_str_safe());
    mInstances.push_back(inst);
    return inst;
}

void MaterialInstanceManager::Record::terminate(FEngine& engine) {
    std::for_each(mInstances.begin(), mInstances.end(), [&engine](auto instance) {
        engine.destroy(instance);
    });
}

void MaterialInstanceManager::terminate(FEngine& engine) {
    std::for_each(mMaterials.begin(), mMaterials.end(), [&engine](auto& record) {
        record.terminate(engine);
    });
}


FMaterialInstance* MaterialInstanceManager::getMaterialInstance(
        FMaterial const* ma) {
    auto itr = std::find_if(mMaterials.begin(), mMaterials.end(), [ma](auto& record) {
        return ma == record.mMaterial;
    });

    if (itr != mMaterials.end()) {
        return itr->getInstance();
    }

    mMaterials.emplace_back(ma);
    return mMaterials.back().getInstance();
}

} // namespace filament
