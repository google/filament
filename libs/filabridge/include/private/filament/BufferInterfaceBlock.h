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

#ifndef TNT_FILAMENT_DRIVER_BUFFERINTERFACEBLOCK_H
#define TNT_FILAMENT_DRIVER_BUFFERINTERFACEBLOCK_H

#include <backend/DriverEnums.h>

#include <utils/CString.h>
#include <utils/compiler.h>
#include <utils/FixedCapacityVector.h>

#include <math/vec4.h>

#include <initializer_list>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <assert.h>

namespace filament {

class BufferInterfaceBlock {
public:
    struct InterfaceBlockEntry {
        std::string_view name;
        uint32_t size;
        backend::UniformType type;
        backend::Precision precision{};
        uint8_t associatedSampler;
        backend::FeatureLevel minFeatureLevel = backend::FeatureLevel::FEATURE_LEVEL_1;
        std::string_view structName{};
        uint32_t stride{};
        std::string_view sizeName{};
    };

    BufferInterfaceBlock();

    BufferInterfaceBlock(const BufferInterfaceBlock& rhs) = delete;
    BufferInterfaceBlock& operator=(const BufferInterfaceBlock& rhs) = delete;

    BufferInterfaceBlock(BufferInterfaceBlock&& rhs) noexcept;
    BufferInterfaceBlock& operator=(BufferInterfaceBlock&& rhs) noexcept;

    ~BufferInterfaceBlock() noexcept;

    using Type = backend::UniformType;
    using Precision = backend::Precision;

    struct FieldInfo {
        utils::CString name;        // name of this field
        uint16_t offset;            // offset in "uint32_t" of this field in the buffer
        uint8_t stride;             // stride in "uint32_t" to the next element
        Type type;                  // type of this field
        bool isArray;               // true if the field is an array
        uint32_t size;              // size of the array in elements, or 0 if not an array
        Precision precision;        // precision of this field
        uint8_t associatedSampler;   // sampler associated with this field
        backend::FeatureLevel minFeatureLevel; // below this feature level, this field is not needed
        utils::CString structName;  // name of this field structure if type is STRUCT
        utils::CString sizeName;    // name of the size parameter in the shader
        // returns offset in bytes of this field (at index if an array)
        inline size_t getBufferOffset(size_t index = 0) const {
            assert_invariant(index < std::max(1u, size));
            return (offset + stride * index) * sizeof(uint32_t);
        }
    };

    enum class Alignment : uint8_t {
        std140,
        std430
    };

    enum class Target : uint8_t  {
        UNIFORM,
        SSBO
    };

    enum class Qualifier : uint8_t {
        COHERENT  = 0x01,
        WRITEONLY = 0x02,
        READONLY  = 0x04,
        VOLATILE  = 0x08,
        RESTRICT  = 0x10
    };

    class Builder {
    public:
        Builder() noexcept;
        ~Builder() noexcept;

        Builder(Builder const& rhs) = default;
        Builder(Builder&& rhs) noexcept = default;
        Builder& operator=(Builder const& rhs) = default;
        Builder& operator=(Builder&& rhs) noexcept = default;

        // Give a name to this buffer interface block
        Builder& name(std::string_view interfaceBlockName);

        // Buffer target
        Builder& target(Target target);

        // build and return the BufferInterfaceBlock
        Builder& alignment(Alignment alignment);

        // add a qualifier
        Builder& qualifier(Qualifier qualifier);

        // a list of this buffer's fields
        Builder& add(std::initializer_list<InterfaceBlockEntry> list);

        // add a variable-sized array. must be the last entry.
        Builder& addVariableSizedArray(InterfaceBlockEntry const& item);

        BufferInterfaceBlock build();

        bool hasVariableSizeArray() const;

    private:
        friend class BufferInterfaceBlock;
        utils::CString mName;
        std::vector<FieldInfo> mEntries;
        Alignment mAlignment = Alignment::std140;
        Target mTarget = Target::UNIFORM;
        uint8_t mQualifiers = 0;
        bool mHasVariableSizeArray = false;
    };

    // name of this BufferInterfaceBlock interface block
    std::string_view getName() const noexcept { return { mName.data(), mName.size() }; }

    // size needed for the buffer in bytes
    size_t getSize() const noexcept { return mSize; }

    // list of information records for each field
    utils::FixedCapacityVector<FieldInfo> const& getFieldInfoList() const noexcept {
        return mFieldInfoList;
    }

    // negative value if name doesn't exist or Panic if exceptions are enabled
    ssize_t getFieldOffset(std::string_view name, size_t index) const;

    // returns offset in bytes of the transform matrix for the given external texture binding
    // returns -1 if the field doesn't exist
    ssize_t getTransformFieldOffset(uint8_t binding) const;

    FieldInfo const* getFieldInfo(std::string_view name) const;

    bool hasField(std::string_view name) const noexcept {
        return mInfoMap.find(name) != mInfoMap.end();
    }

    bool isEmpty() const noexcept { return mFieldInfoList.empty(); }

    bool isEmptyForFeatureLevel(backend::FeatureLevel featureLevel) const noexcept;

    Alignment getAlignment() const noexcept { return mAlignment; }

    Target getTarget() const noexcept { return mTarget; }

    uint8_t getQualifier() const noexcept { return mQualifiers; }

private:
    friend class Builder;

    explicit BufferInterfaceBlock(Builder const& builder) noexcept;

    static uint8_t baseAlignmentForType(Type type) noexcept;
    static uint8_t strideForType(Type type, uint32_t stride) noexcept;

    utils::CString mName;
    utils::FixedCapacityVector<FieldInfo> mFieldInfoList;
    std::unordered_map<std::string_view , uint32_t> mInfoMap;
    std::unordered_map<uint8_t, uint32_t> mTransformOffsetMap;
    uint32_t mSize = 0; // size in bytes rounded to multiple of 4
    Alignment mAlignment = Alignment::std140;
    Target mTarget = Target::UNIFORM;
    uint8_t mQualifiers = 0;
};

} // namespace filament

#endif // TNT_FILAMENT_DRIVER_BUFFERINTERFACEBLOCK_H
