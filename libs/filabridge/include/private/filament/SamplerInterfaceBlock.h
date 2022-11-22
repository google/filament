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

#ifndef TNT_FILAMENT_SAMPLERINTERFACEBLOCK_H
#define TNT_FILAMENT_SAMPLERINTERFACEBLOCK_H


#include <backend/DriverEnums.h>

#include <utils/compiler.h>
#include <utils/CString.h>
#include <utils/FixedCapacityVector.h>

#include <initializer_list>
#include <unordered_map>
#include <string_view>

namespace filament {

class SamplerInterfaceBlock {
public:
    SamplerInterfaceBlock();

    SamplerInterfaceBlock(const SamplerInterfaceBlock& rhs) = delete;
    SamplerInterfaceBlock(SamplerInterfaceBlock&& rhs) noexcept;

    SamplerInterfaceBlock& operator=(const SamplerInterfaceBlock& rhs) = delete;
    SamplerInterfaceBlock& operator=(SamplerInterfaceBlock&& rhs) noexcept;

    ~SamplerInterfaceBlock() noexcept;

    using Type = backend::SamplerType;
    using Format = backend::SamplerFormat;
    using Precision = backend::Precision;
    using SamplerParams = backend::SamplerParams;

    struct SamplerInfo { // NOLINT(cppcoreguidelines-pro-type-member-init)
        utils::CString name;        // name of this sampler
        utils::CString uniformName; // name of the uniform holding this sampler (needed for glsl/MSL)
        uint8_t offset;             // offset in "Sampler" of this sampler in the buffer
        Type type;                  // type of this sampler
        Format format;              // format of this sampler
        Precision precision;        // precision of this sampler
        bool multisample;           // multisample capable
    };

    class Builder {
    public:
        Builder();
        ~Builder() noexcept;

        Builder(Builder const& rhs) = default;
        Builder(Builder&& rhs) noexcept = default;
        Builder& operator=(Builder const& rhs) = default;
        Builder& operator=(Builder&& rhs) noexcept = default;

        struct ListEntry { // NOLINT(cppcoreguidelines-pro-type-member-init)
            std::string_view name;          // name of this sampler
            Type type;                      // type of this sampler
            Format format;                  // format of this sampler
            Precision precision;            // precision of this sampler
            bool multisample = false;       // multisample capable
        };

        // Give a name to this sampler interface block
        Builder& name(std::string_view interfaceBlockName);

        Builder& stageFlags(backend::ShaderStageFlags stageFlags);

        // Add a sampler
        Builder& add(std::string_view samplerName, Type type, Format format,
                Precision precision = Precision::MEDIUM,
                bool multisample = false) noexcept;

        // Add multiple samplers
        Builder& add(std::initializer_list<ListEntry> list) noexcept;

        // build and return the SamplerInterfaceBlock
        SamplerInterfaceBlock build();
    private:
        friend class SamplerInterfaceBlock;
        utils::CString mName;
        backend::ShaderStageFlags mStageFlags = backend::ShaderStageFlags::ALL_SHADER_STAGE_FLAGS;
        std::vector<SamplerInfo> mEntries;
    };

    // name of this sampler interface block
    const utils::CString& getName() const noexcept { return mName; }

    backend::ShaderStageFlags getStageFlags() const noexcept { return mStageFlags; }

    // size needed to store the samplers described by this interface block in a SamplerGroup
    size_t getSize() const noexcept { return mSamplersInfoList.size(); }

    // list of information records for each sampler
    utils::FixedCapacityVector<SamplerInfo> const& getSamplerInfoList() const noexcept {
        return mSamplersInfoList;
    }

    // information record for sampler of the given name
    SamplerInfo const* getSamplerInfo(std::string_view name) const;

    bool hasSampler(std::string_view name) const noexcept {
        return mInfoMap.find(name) != mInfoMap.end();
    }

    bool isEmpty() const noexcept { return mSamplersInfoList.empty(); }

    static utils::CString generateUniformName(const char* group, const char* sampler) noexcept;

private:
    friend class Builder;


    explicit SamplerInterfaceBlock(Builder const& builder) noexcept;

    utils::CString mName;
    backend::ShaderStageFlags mStageFlags{}; // It's needed to check if MAX_SAMPLER_COUNT is exceeded.
    utils::FixedCapacityVector<SamplerInfo> mSamplersInfoList;
    std::unordered_map<std::string_view, uint32_t> mInfoMap;
};

} // namespace filament

#endif // TNT_FILAMENT_SAMPLERINTERFACEBLOCK_H
