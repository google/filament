/*
 * Copyright (C) 2021 The Android Open Source Project
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

//! \file

#ifndef TNT_FILAMENT_BUFFEROBJECT_H
#define TNT_FILAMENT_BUFFEROBJECT_H

#include <filament/FilamentAPI.h>

#include <backend/DriverEnums.h>
#include <backend/BufferDescriptor.h>

#include <utils/compiler.h>

#include <stdint.h>
#include <stddef.h>

namespace filament {

class FBufferObject;

class Engine;

/**
 * A generic GPU buffer containing data.
 *
 * Usage of this BufferObject is optional. For simple use cases it is not necessary. It is useful
 * only when you need to share data between multiple VertexBuffer instances. It also allows you to
 * efficiently swap-out the buffers in VertexBuffer.
 *
 * NOTE: For now this is only used for vertex data, but in the future we may use it for other things
 * (e.g. compute).
 *
 * @see VertexBuffer
 */
class UTILS_PUBLIC BufferObject : public FilamentAPI {
    struct BuilderDetails;

public:
    using BufferDescriptor = backend::BufferDescriptor;
    using BindingType = backend::BufferObjectBinding;

    class Builder : public BuilderBase<BuilderDetails>, public BuilderNameMixin<Builder> {
        friend struct BuilderDetails;
    public:
        Builder() noexcept;
        Builder(Builder const& rhs) noexcept;
        Builder(Builder&& rhs) noexcept;
        ~Builder() noexcept;
        Builder& operator=(Builder const& rhs) noexcept;
        Builder& operator=(Builder&& rhs) noexcept;

        /**
         * Size of the buffer in bytes.
         * @param byteCount Maximum number of bytes the BufferObject can hold.
         * @return A reference to this Builder for chaining calls.
         */
        Builder& size(uint32_t byteCount) noexcept;

        /**
         * The binding type for this buffer object. (defaults to VERTEX)
         * @param bindingType Distinguishes between SSBO, VBO, etc. For now this must be VERTEX.
         * @return A reference to this Builder for chaining calls.
         */
        Builder& bindingType(BindingType bindingType) noexcept;

        /**
         * Associate an optional name with this BufferObject for debugging purposes.
         *
         * name will show in error messages and should be kept as short as possible. The name is
         * truncated to a maximum of 128 characters.
         *
         * The name string is copied during this method so clients may free its memory after
         * the function returns.
         *
         * @param name A string to identify this BufferObject
         * @param len Length of name, should be less than or equal to 128
         * @return This Builder, for chaining calls.
         */
        Builder& name(const char* UTILS_NONNULL name, size_t len) noexcept;

        /**
         * Creates the BufferObject and returns a pointer to it. After creation, the buffer
         * object is uninitialized. Use BufferObject::setBuffer() to initialize it.
         *
         * @param engine Reference to the filament::Engine to associate this BufferObject with.
         *
         * @return pointer to the newly created object
         *
         * @exception utils::PostConditionPanic if a runtime error occurred, such as running out of
         *            memory or other resources.
         * @exception utils::PreConditionPanic if a parameter to a builder function was invalid.
         *
         * @see IndexBuffer::setBuffer
         */
        BufferObject* UTILS_NONNULL build(Engine& engine);
    private:
        friend class FBufferObject;
    };

    /**
     * Asynchronously copy-initializes a region of this BufferObject from the data provided.
     *
     * @param engine Reference to the filament::Engine associated with this BufferObject.
     * @param buffer A BufferDescriptor representing the data used to initialize the BufferObject.
     * @param byteOffset Offset in bytes into the BufferObject
     */
    void setBuffer(Engine& engine, BufferDescriptor&& buffer, uint32_t byteOffset = 0);

    /**
     * Returns the size of this BufferObject in elements.
     * @return The maximum capacity of the BufferObject.
     */
    size_t getByteCount() const noexcept;

protected:
    // prevent heap allocation
    ~BufferObject() = default;
};

} // namespace filament

#endif // TNT_FILAMENT_BUFFEROBJECT_H
