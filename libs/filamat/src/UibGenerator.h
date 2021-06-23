/*
 * Copyright (C) 2015 The Android Open Source Project
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

#ifndef TNT_FILAMAT_UIBGENERATOR_H
#define TNT_FILAMAT_UIBGENERATOR_H

namespace filament {

class UniformInterfaceBlock;

class UibGenerator {
public:
    static UniformInterfaceBlock const& getPerViewUib() noexcept;
    static UniformInterfaceBlock const& getPerRenderableUib() noexcept;
    static UniformInterfaceBlock const& getLightsUib() noexcept;
    static UniformInterfaceBlock const& getShadowUib() noexcept;
    static UniformInterfaceBlock const& getPerRenderableBonesUib() noexcept;
    static UniformInterfaceBlock const& getFroxelRecordUib() noexcept;
    // When adding an UBO here, make sure to also update
    //      FMaterial::getSurfaceProgramSlow and FMaterial::getPostProcessProgramSlow if needed
};

} // namespace filament

#endif // TNT_FILAMAT_UIBGENERATOR_H
