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

#include <private/filament/EngineEnums.h>

#include <backend/Program.h>

#include <utils/FixedCapacityVector.h>

namespace filament {
class SamplerBindingMap;
class SamplerInterfaceBlock;
class BufferInterfaceBlock;
struct SubpassInfo;
struct MaterialConstant;
struct MaterialPushConstant;
} // namespace filament

namespace filamat {

class MaterialUniformInterfaceBlockChunk final : public Chunk {
public:
    explicit MaterialUniformInterfaceBlockChunk(filament::BufferInterfaceBlock const& uib);
    ~MaterialUniformInterfaceBlockChunk() final = default;

private:
    void flatten(Flattener&) final;

    filament::BufferInterfaceBlock const& mUib;
};

// ------------------------------------------------------------------------------------------------

class MaterialSamplerInterfaceBlockChunk final : public Chunk {
public:
    explicit MaterialSamplerInterfaceBlockChunk(filament::SamplerInterfaceBlock const& sib);
    ~MaterialSamplerInterfaceBlockChunk() final = default;

private:
    void flatten(Flattener&) final;

    filament::SamplerInterfaceBlock const& mSib;
};

// ------------------------------------------------------------------------------------------------

class MaterialSubpassInterfaceBlockChunk final : public Chunk {
public:
    explicit MaterialSubpassInterfaceBlockChunk(filament::SubpassInfo const& subpass);
    ~MaterialSubpassInterfaceBlockChunk() final = default;

private:
    void flatten(Flattener&) final;

    filament::SubpassInfo const& mSubpass;
};

// ------------------------------------------------------------------------------------------------

class MaterialConstantParametersChunk final : public Chunk {
public:
    explicit MaterialConstantParametersChunk(
            utils::FixedCapacityVector<filament::MaterialConstant> constants);
    ~MaterialConstantParametersChunk() final = default;

private:
    void flatten(Flattener&) final;

    utils::FixedCapacityVector<filament::MaterialConstant> mConstants;
};

// ------------------------------------------------------------------------------------------------

class MaterialPushConstantParametersChunk final : public Chunk {
public:
    explicit MaterialPushConstantParametersChunk(utils::CString const& structVarName,
            utils::FixedCapacityVector<filament::MaterialPushConstant> constants);
    ~MaterialPushConstantParametersChunk() final = default;

private:
    void flatten(Flattener&) final;

    utils::CString mStructVarName;
    utils::FixedCapacityVector<filament::MaterialPushConstant> mConstants;
};

// ------------------------------------------------------------------------------------------------

class MaterialUniformBlockBindingsChunk final : public Chunk {
    using Container = utils::FixedCapacityVector<
            std::pair<std::string_view, filament::UniformBindingPoints>>;
public:
    explicit MaterialUniformBlockBindingsChunk(Container list);
    ~MaterialUniformBlockBindingsChunk() final = default;

private:
    void flatten(Flattener&) final;

    Container mBindingList;
};

// ------------------------------------------------------------------------------------------------

class MaterialSamplerBlockBindingChunk final : public Chunk {
public:
    explicit MaterialSamplerBlockBindingChunk(filament::SamplerBindingMap const& samplerBindings);
    ~MaterialSamplerBlockBindingChunk() final = default;

private:
    void flatten(Flattener &) final;

    filament::SamplerBindingMap const& mSamplerBindings;
};

// ------------------------------------------------------------------------------------------------

class MaterialBindingUniformInfoChunk final : public Chunk {
    using Container = FixedCapacityVector<
            std::pair<filament::UniformBindingPoints, filament::backend::Program::UniformInfo>>;
public:
    explicit MaterialBindingUniformInfoChunk(Container list) noexcept;
    ~MaterialBindingUniformInfoChunk() final = default;

private:
    void flatten(Flattener &) final;

    Container mBindingUniformInfo;
};

// ------------------------------------------------------------------------------------------------

class MaterialAttributesInfoChunk final : public Chunk {
    using Container = FixedCapacityVector<std::pair<utils::CString, uint8_t>>;
public:
    explicit MaterialAttributesInfoChunk(Container list) noexcept;
    ~MaterialAttributesInfoChunk() final = default;

private:
    void flatten(Flattener &) final;

    Container mAttributeInfo;
};

} // namespace filamat

#endif // TNT_FILAMAT_MAT_INTEFFACE_BLOCK_CHUNK_H
