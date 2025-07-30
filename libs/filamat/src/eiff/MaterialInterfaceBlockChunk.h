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
#include "private/filament/ConstantInfo.h"

#include <backend/Program.h>

#include <utils/CString.h>
#include <utils/FixedCapacityVector.h>

#include <tuple>

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
    ~MaterialUniformInterfaceBlockChunk() override = default;

private:
    void flatten(Flattener&) override;

    filament::BufferInterfaceBlock const& mUib;
};

// ------------------------------------------------------------------------------------------------

class MaterialSamplerInterfaceBlockChunk final : public Chunk {
public:
    explicit MaterialSamplerInterfaceBlockChunk(filament::SamplerInterfaceBlock const& sib);
    ~MaterialSamplerInterfaceBlockChunk() override = default;

private:
    void flatten(Flattener&) override;

    filament::SamplerInterfaceBlock const& mSib;
};

// ------------------------------------------------------------------------------------------------

class MaterialSubpassInterfaceBlockChunk final : public Chunk {
public:
    explicit MaterialSubpassInterfaceBlockChunk(filament::SubpassInfo const& subpass);
    ~MaterialSubpassInterfaceBlockChunk() override = default;

private:
    void flatten(Flattener&) override;

    filament::SubpassInfo const& mSubpass;
};

// ------------------------------------------------------------------------------------------------

class MaterialConstantParametersChunk final : public Chunk {
public:
    explicit MaterialConstantParametersChunk(
            FixedCapacityVector<filament::MaterialConstant> constants);
    ~MaterialConstantParametersChunk() override = default;

private:
    void flatten(Flattener&) override;

    FixedCapacityVector<filament::MaterialConstant> mConstants;
};

// ------------------------------------------------------------------------------------------------

class MaterialMutableConstantParametersChunk final : public Chunk {
public:
    explicit MaterialMutableConstantParametersChunk(
            FixedCapacityVector<filament::MaterialMutableConstant> constants);
    ~MaterialMutableConstantParametersChunk() override = default;

private:
    void flatten(Flattener&) override;

    FixedCapacityVector<filament::MaterialMutableConstant> mConstants;
};

// ------------------------------------------------------------------------------------------------

class MaterialPushConstantParametersChunk final : public Chunk {
public:
    explicit MaterialPushConstantParametersChunk(CString const& structVarName,
            FixedCapacityVector<filament::MaterialPushConstant> constants);
    ~MaterialPushConstantParametersChunk() override = default;

private:
    void flatten(Flattener&) override;

    CString mStructVarName;
    FixedCapacityVector<filament::MaterialPushConstant> mConstants;
};

// ------------------------------------------------------------------------------------------------

class MaterialBindingUniformInfoChunk final : public Chunk {
    using Container = FixedCapacityVector<std::tuple<
            uint8_t, CString, filament::backend::Program::UniformInfo>>;
public:
    explicit MaterialBindingUniformInfoChunk(Container list) noexcept;
    ~MaterialBindingUniformInfoChunk() override = default;

private:
    void flatten(Flattener &) override;

    Container mBindingUniformInfo;
};

// ------------------------------------------------------------------------------------------------

class MaterialAttributesInfoChunk final : public Chunk {
    using Container = FixedCapacityVector<std::pair<CString, uint8_t>>;
public:
    explicit MaterialAttributesInfoChunk(Container list) noexcept;
    ~MaterialAttributesInfoChunk() override = default;

private:
    void flatten(Flattener &) override;

    Container mAttributeInfo;
};

// ------------------------------------------------------------------------------------------------

class MaterialDescriptorBindingsChuck final : public Chunk {
    using Container = filament::SamplerInterfaceBlock;
public:
    explicit MaterialDescriptorBindingsChuck(Container const& sib) noexcept;
    ~MaterialDescriptorBindingsChuck() override = default;

private:
    void flatten(Flattener&) override;

    Container const& mSamplerInterfaceBlock;
};

// ------------------------------------------------------------------------------------------------

class MaterialDescriptorSetLayoutChunk final : public Chunk {
    using Container = filament::SamplerInterfaceBlock;
public:
    explicit MaterialDescriptorSetLayoutChunk(Container const& sib) noexcept;
    ~MaterialDescriptorSetLayoutChunk() override = default;

private:
    void flatten(Flattener&) override;

    Container const& mSamplerInterfaceBlock;
};

} // namespace filamat

#endif // TNT_FILAMAT_MAT_INTEFFACE_BLOCK_CHUNK_H
