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

#ifndef TNT_FILAMENT_DRIVER_UNIFORMINTERFACEBLOCK_H
#define TNT_FILAMENT_DRIVER_UNIFORMINTERFACEBLOCK_H

#include <backend/DriverEnums.h>

#include <utils/CString.h>
#include <utils/compiler.h>

#include <math/vec4.h>

#include <tsl/robin_map.h>

#include <assert.h>
#include <vector>

namespace filament {

class UniformInterfaceBlock {
public:
    UniformInterfaceBlock();
    UniformInterfaceBlock(const UniformInterfaceBlock& rhs);
    UniformInterfaceBlock(UniformInterfaceBlock&& rhs) noexcept;
    UniformInterfaceBlock& operator=(const UniformInterfaceBlock& rhs);
    UniformInterfaceBlock& operator=(UniformInterfaceBlock&& rhs) /*noexcept*/;
    ~UniformInterfaceBlock() noexcept;

    using Type = backend::UniformType;
    using Precision = backend::Precision;

    class Builder {
    public:
        // Give a name to this uniform interface block
        Builder& name(utils::CString const& interfaceBlockName);
        Builder& name(utils::CString&& interfaceBlockName);
        Builder& name(utils::StaticString const& interfaceBlockName);

        template<size_t N>
        Builder& name(utils::StringLiteral<N> const& interfaceBlockName) {
            return name(utils::StaticString{ interfaceBlockName });
        }

        // Add a uniform
        Builder& add(utils::CString const& uniformName, size_t size,
                Type type, Precision precision = Precision::DEFAULT);
        Builder& add(utils::CString&& uniformName, size_t size,
                Type type, Precision precision = Precision::DEFAULT);
        Builder& add(utils::StaticString const& uniformName, size_t size,
                Type type, Precision precision = Precision::DEFAULT);

        // Add a known struct
        Builder& add(utils::CString const& uniformName, size_t size,
                utils::CString const& structName, size_t stride);

        template<size_t N>
        Builder& add(utils::StringLiteral<N> const& uniformName, size_t size,
                Type type, Precision precision = Precision::DEFAULT) {
            return add(utils::StaticString{ uniformName }, size, type, precision);
        }

        template<size_t N0, size_t N1>
        Builder& add(utils::StringLiteral<N0> const& uniformName, size_t size,
                utils::StringLiteral<N1> const& structName, size_t stride) {
            return add(utils::StaticString{ uniformName }, size,
                    utils::StaticString{ structName }, stride);
        }

        // build and return the UniformInterfaceBlock
        UniformInterfaceBlock build();
    private:
        friend class UniformInterfaceBlock;
        struct Entry {
            Entry(utils::CString name, uint32_t size, Type type, Precision precision) noexcept
                    : name(std::move(name)), size(size), type(type), precision(precision), stride(strideForType(type, 0)) { }
            Entry(utils::CString name, uint32_t size, utils::CString structName, size_t stride) noexcept
                    : name(std::move(name)), size(size), type(Type::STRUCT), structName(std::move(structName)), stride(stride) { }
            utils::CString name;
            uint32_t size;
            Type type;
            Precision precision{};
            utils::CString structName{};
            uint32_t stride;
        };
        utils::CString mName;
        std::vector<Entry> mEntries;
    };

    struct UniformInfo {
        utils::CString name;// name of this uniform
        uint16_t offset;    // offset in "uint32_t" of this uniform in the buffer
        uint8_t stride;     // stride in "uint32_t" to the next element
        Type type;          // type of this uniform
        uint32_t size;      // size of the array in elements, or 1 if not an array
        Precision precision;// precision of this uniform
        utils::CString structName;// name of this uniform structure if type is STRUCT
        // returns offset in bytes of this uniform (at index if an array)
        inline size_t getBufferOffset(size_t index = 0) const {
            assert(index < size);
            return (offset + stride * index) * sizeof(uint32_t);
        }
    };

    // name of this uniform interface block
    const utils::CString& getName() const noexcept { return mName; }

    // size in bytes needed to store the uniforms described by this interface block in a UniformBuffer
    size_t getSize() const noexcept { return mSize; }

    // list of information records for each uniform
    std::vector<UniformInfo> const& getUniformInfoList() const noexcept { return mUniformsInfoList; }

    // negative value if name doesn't exist or Panic if exceptions are enabled
    ssize_t getUniformOffset(const char* name, size_t index) const;

    UniformInfo const* getUniformInfo(const char* name) const;

    bool hasUniform(const char* name) const noexcept {
        return mInfoMap.find(name) != mInfoMap.end();
    }

    bool isEmpty() const noexcept { return mUniformsInfoList.empty(); }

private:
    friend class Builder;

    explicit UniformInterfaceBlock(Builder const& builder) noexcept;

    static uint8_t baseAlignmentForType(Type type) noexcept;
    static uint8_t strideForType(Type type, uint32_t stride) noexcept;

    utils::CString mName;
    std::vector<UniformInfo> mUniformsInfoList;
    tsl::robin_map<const char*, uint32_t, utils::hashCStrings, utils::equalCStrings> mInfoMap;
    uint32_t mSize = 0; // size in bytes
};

} // namespace filament

#endif // TNT_FILAMENT_DRIVER_UNIFORMINTERFACEBLOCK_H
