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

#include <backend/DriverEnums.h>
#include <backend/Program.h>

#include <utils/CString.h>
#include <utils/FixedCapacityVector.h>

#include <tuple>
#include <utility>

#include <stdint.h>

namespace filament {
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

class MaterialBindingUniformInfoChunk final : public Chunk {
    using Container = FixedCapacityVector<std::tuple<
            uint8_t, utils::CString, filament::backend::Program::UniformInfo>>;
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

// ------------------------------------------------------------------------------------------------

class MaterialDescriptorBindingsChuck final : public Chunk {
    using Container = filament::SamplerInterfaceBlock;
public:
    explicit MaterialDescriptorBindingsChuck(Container const& sib,
            filament::backend::DescriptorSetLayout const& perViewLayout) noexcept;
    ~MaterialDescriptorBindingsChuck() final = default;

private:
    void flatten(Flattener&) final;

    Container const& mSamplerInterfaceBlock;
    filament::backend::DescriptorSetLayout mPerViewLayout;
};

// ------------------------------------------------------------------------------------------------

class MaterialDescriptorSetLayoutChunk final : public Chunk {
    using Container = filament::SamplerInterfaceBlock;
public:
    explicit MaterialDescriptorSetLayoutChunk(Container const& sib,
            filament::backend::DescriptorSetLayout const& perViewLayout) noexcept;
    ~MaterialDescriptorSetLayoutChunk() final = default;

private:
    void flatten(Flattener&) final;

    Container const& mSamplerInterfaceBlock;
    filament::backend::DescriptorSetLayout mPerViewLayout;
};

} // namespace filamat

#endif // TNT_FILAMAT_MAT_INTEFFACE_BLOCK_CHUNK_H
