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

#ifndef TNT_FILAMENT_INDEXBUFFER_H
#define TNT_FILAMENT_INDEXBUFFER_H

#include <filament/FilamentAPI.h>

#include <filament/driver/DriverEnums.h>
#include <filament/driver/BufferDescriptor.h>

#include <utils/compiler.h>

#include <stddef.h>

namespace filament {

namespace details {
class FIndexBuffer;
} // namespace details

class Engine;

class UTILS_PUBLIC IndexBuffer : public FilamentAPI {
    struct BuilderDetails;

public:
    using BufferDescriptor = driver::BufferDescriptor;

    enum class IndexType : uint8_t {
        USHORT = uint8_t(driver::ElementType::USHORT),
        UINT = uint8_t(driver::ElementType::UINT),
    };

    class Builder : public BuilderBase<BuilderDetails> {
        friend struct BuilderDetails;
    public:
        Builder() noexcept;
        Builder(Builder const& rhs) noexcept;
        Builder(Builder&& rhs) noexcept;
        ~Builder() noexcept;
        Builder& operator=(Builder const& rhs) noexcept;
        Builder& operator=(Builder&& rhs) noexcept;

        Builder& indexCount(uint32_t indexCount) noexcept;
        Builder& bufferType(IndexType indexType) noexcept;

        /**
         * Creates the IndexBuffer object and returns a pointer to it.
         *
         * @param engine Reference to the filament::Engine to associate this IndexBuffer with.
         *
         * @return pointer to the newly created object or nullptr if exceptions are disabled and
         *         an error occurred.
         *
         * @exception utils::PostConditionPanic if a runtime error occurred, such as running out of
         *            memory or other resources.
         * @exception utils::PreConditionPanic if a parameter to a builder function was invalid.
         */
        IndexBuffer* build(Engine& engine);
    private:
        friend class details::FIndexBuffer;
    };

    void setBuffer(Engine& engine,
            BufferDescriptor&& buffer,
            uint32_t byteOffset = 0,
            uint32_t byteSize = 0);

    size_t getIndexCount() const noexcept;
};

} // namespace filament

#endif // TNT_FILAMENT_INDEXBUFFER_H
