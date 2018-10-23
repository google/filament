/*
 * Copyright (C) 2017 The Android Open Source Project
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

#ifndef TNT_FILAMAT_MAT_INTEFFACE_BLOCK_CHUNK_H
#define TNT_FILAMAT_MAT_INTEFFACE_BLOCK_CHUNK_H

#include "Chunk.h"
#include "../shaders/MaterialInfo.h"

#include <private/filament/SamplerInterfaceBlock.h>
#include <filament/SamplerBindingMap.h>
#include <private/filament/UniformInterfaceBlock.h>

namespace filamat {

class MaterialUniformInterfaceBlockChunk : public Chunk {
public:
    MaterialUniformInterfaceBlockChunk(filament::UniformInterfaceBlock& uib);
    virtual ~MaterialUniformInterfaceBlockChunk() = default;

    virtual void flatten(Flattener &) override;
private:
    filament::UniformInterfaceBlock& mUib;
};

class MaterialSamplerInterfaceBlockChunk : public Chunk {
public:
    MaterialSamplerInterfaceBlockChunk(filament::SamplerInterfaceBlock& sib);
    virtual ~MaterialSamplerInterfaceBlockChunk() = default;

    virtual void flatten(Flattener &) override;
private:
    filament::SamplerInterfaceBlock& mSib;
};

class MaterialSamplerBindingsChunk : public Chunk {
public:
    MaterialSamplerBindingsChunk(const filament::SamplerBindingMap& samplerBindings);
    virtual ~MaterialSamplerBindingsChunk() = default;

    virtual void flatten(Flattener &) override;
private:
    const filament::SamplerBindingMap& mSamplerBindings;
};

} // namespace filamat
#endif // TNT_FILAMAT_MAT_INFO_CHUNK_H
