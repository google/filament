/*
 * Copyright (C) 2022 The Android Open Source Project
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

#ifndef TNT_TESTMATERIALCOMPILER_H
#define TNT_TESTMATERIALCOMPILER_H

#include <stddef.h>

#include <matc/MaterialCompiler.h>

class TestMaterialCompiler {
public:
    explicit TestMaterialCompiler(const matc::MaterialCompiler& materialCompiler) :
            mMaterialCompiler(materialCompiler) {}

    bool parseMaterial(const char* buffer, size_t size, filamat::MaterialBuilder& builder)
    const noexcept{
        return mMaterialCompiler.parseMaterial(buffer, size, builder);
    }

    bool parseMaterialAsJSON(const char* buffer, size_t size, filamat::MaterialBuilder& builder)
    const noexcept{
        return mMaterialCompiler.parseMaterialAsJSON(buffer, size, builder);
    }

private:
    const matc::MaterialCompiler& mMaterialCompiler;
};

#endif //TNT_TESTMATERIALCOMPILER_H
