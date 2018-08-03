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

#ifndef TNT_FILAMENT_DRIVER_SAMPLERINTERFACEBLOCK_H
#define TNT_FILAMENT_DRIVER_SAMPLERINTERFACEBLOCK_H

#include <assert.h>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include <utils/compiler.h>
#include <utils/CString.h>
#include <math/vec4.h>
#include <filament/driver/DriverEnums.h>

namespace filament {

class SamplerInterfaceBlock {
public:
    SamplerInterfaceBlock();
    SamplerInterfaceBlock(const SamplerInterfaceBlock& rhs);
    SamplerInterfaceBlock(SamplerInterfaceBlock&& rhs) noexcept;
    SamplerInterfaceBlock& operator=(const SamplerInterfaceBlock& rhs);
    SamplerInterfaceBlock& operator=(SamplerInterfaceBlock&& rhs) noexcept;
    ~SamplerInterfaceBlock() noexcept;

    using Type = driver::SamplerType;
    using Format = driver::SamplerFormat;
    using Precision = driver::Precision;
    using SamplerParams = driver::SamplerParams;

    class Builder {
    public:
        // Give a name to this sampler interface block
        Builder& name(const std::string& interfaceBlockName);
        // Add a sampler
        Builder& add(const std::string& samplerName, Type type, Format format, Precision precision = Precision::MEDIUM,
                bool multisample = false) noexcept;
        // build and return the SamplerInterfaceBlock
        SamplerInterfaceBlock build();
    private:
        friend class SamplerInterfaceBlock;
        struct Entry {
            Entry(utils::CString name, Type type, Format format, Precision precision, bool multisample) noexcept
                    : name(std::move(name)), type(type), format(format),
                      multisample(multisample), precision(precision) { }
            utils::CString name;
            Type type;
            Format format;
            bool multisample;
            Precision precision;
        };
        utils::CString mName;
        std::vector<Entry> mEntries;
    };

    struct SamplerInfo {
        SamplerInfo() noexcept = default;
        SamplerInfo(utils::CString name,
                uint8_t offset, Type type, Format format, Precision precision, bool multisample) noexcept
                : name(std::move(name)),
                  offset(offset), type(type), format(format),
                  multisample(multisample), precision(precision) {}
        utils::CString name;        // name of this sampler
        uint8_t offset;      // offset in "Sampler" of this sampler in the buffer
        Type type;           // type of this sampler
        Format format;       // format of this sampler
        bool multisample;    // multisample capable
        Precision precision; // precision of this sampler
    private:
        friend std::ostream& operator<<(std::ostream& out, const SamplerInfo& info);
    };

    // name of this sampler interface block
    const utils::CString& getName() const noexcept { return mName; }

    // size needed to store the samplers described by this interface block in a SamplerBuffer
    size_t getSize() const noexcept { return mSize; }

    // list of information records for each sampler
    std::vector<SamplerInfo> const& getSamplerInfoList() const noexcept { return mSamplersInfoList; }

    // information record for sampler of the given name
    SamplerInfo const* getSamplerInfo(const char* name) const;

    bool hasSampler(const char* name) const noexcept {
        return mInfoMap.find(name) != mInfoMap.end();
    }

    bool isEmpty() const noexcept { return mSamplersInfoList.empty(); }

private:
    friend class Builder;

    explicit SamplerInterfaceBlock(Builder& builder) noexcept;

    utils::CString mName;
    std::vector<SamplerInfo> mSamplersInfoList;
    std::unordered_map<const char*, uint32_t, utils::hashCStrings, utils::equalCStrings> mInfoMap;
    uint32_t mSize = 0; // size in Samplers (i.e.: count)
};

} // namespace filament

#endif // TNT_FILAMENT_DRIVER_SAMPLERINTERFACEBLOCK_H
