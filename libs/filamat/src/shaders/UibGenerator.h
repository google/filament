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

class BufferInterfaceBlock;

class UibGenerator {
public:
    static BufferInterfaceBlock const& getPerViewUib() noexcept;
    static BufferInterfaceBlock const& getPerRenderableUib() noexcept;
    static BufferInterfaceBlock const& getLightsUib() noexcept;
    static BufferInterfaceBlock const& getShadowUib() noexcept;
    static BufferInterfaceBlock const& getPerRenderableBonesUib() noexcept;
    static BufferInterfaceBlock const& getPerRenderableMorphingUib() noexcept;
    static BufferInterfaceBlock const& getFroxelRecordUib() noexcept;
    static BufferInterfaceBlock const& getFroxelsUib() noexcept;
    // When adding an UBO here, make sure to also update
    //      MaterialBuilder::writeCommonChunks() if needed
};

} // namespace filament

#endif // TNT_FILAMAT_UIBGENERATOR_H
