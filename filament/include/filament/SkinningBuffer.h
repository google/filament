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

#ifndef TNT_FILAMENT_SKINNINGBUFFER_H
#define TNT_FILAMENT_SKINNINGBUFFER_H

#include <filament/FilamentAPI.h>

#include <filament/RenderableManager.h>

#include <utils/compiler.h>

#include <math/mathfwd.h>

#include <stddef.h>
#include <stdint.h>

namespace filament {

/**
 * SkinningBuffer is used to hold skinning data (bones). It is a simple wraper around
 * a structured UBO.
 * @see RenderableManager::setSkinningBuffer
 */
class UTILS_PUBLIC SkinningBuffer : public FilamentAPI {
    struct BuilderDetails;

public:
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
         * Size of the skinning buffer in bones.
         *
         * Due to limitation in the GLSL, the SkinningBuffer must always by a multiple of
         * 256, this adjustment is done automatically, but can cause
         * some memory overhead. This memory overhead can be mitigated by using the same
         * SkinningBuffer to store the bone information for multiple RenderPrimitives.
         *
         * @param boneCount Number of bones the skinning buffer can hold.
         * @return A reference to this Builder for chaining calls.
         */
        Builder& boneCount(uint32_t boneCount) noexcept;

        /**
         * The new buffer is created with identity bones
         * @param initialize true to initializing the buffer, false to not.
         * @return A reference to this Builder for chaining calls.
         */
        Builder& initialize(bool initialize = true) noexcept;

        /**
         * Associate an optional name with this SkinningBuffer for debugging purposes.
         *
         * name will show in error messages and should be kept as short as possible. The name is
         * truncated to a maximum of 128 characters.
         *
         * The name string is copied during this method so clients may free its memory after
         * the function returns.
         *
         * @param name A string to identify this SkinningBuffer
         * @param len Length of name, should be less than or equal to 128
         * @return This Builder, for chaining calls.
         */
        // Builder& name(const char* UTILS_NONNULL name, size_t len) noexcept; // inherited

        /**
         * Creates the SkinningBuffer object and returns a pointer to it.
         *
         * @param engine Reference to the filament::Engine to associate this SkinningBuffer with.
         *
         * @return pointer to the newly created object.
         *
         * @exception utils::PostConditionPanic if a runtime error occurred, such as running out of
         *            memory or other resources.
         * @exception utils::PreConditionPanic if a parameter to a builder function was invalid.
         *
         * @see SkinningBuffer::setBones
         */
        SkinningBuffer* UTILS_NONNULL build(Engine& engine);
    private:
        friend class FSkinningBuffer;
    };

    /**
     * Updates the bone transforms in the range [offset, offset + count).
     * @param engine Reference to the filament::Engine to associate this SkinningBuffer with.
     * @param transforms pointer to at least count Bone
     * @param count number of Bone elements in transforms
     * @param offset offset in elements (not bytes) in the SkinningBuffer (not in transforms)
     * @see RenderableManager::setSkinningBuffer
     */
    void setBones(Engine& engine, RenderableManager::Bone const* UTILS_NONNULL transforms,
            size_t count, size_t offset = 0);

    /**
     * Updates the bone transforms in the range [offset, offset + count).
     * @param engine Reference to the filament::Engine to associate this SkinningBuffer with.
     * @param transforms pointer to at least count mat4f
     * @param count number of mat4f elements in transforms
     * @param offset offset in elements (not bytes) in the SkinningBuffer (not in transforms)
     * @see RenderableManager::setSkinningBuffer
     */
    void setBones(Engine& engine, math::mat4f const* UTILS_NONNULL transforms,
            size_t count, size_t offset = 0);

    /**
     * Returns the size of this SkinningBuffer in elements.
     * @return The number of bones the SkinningBuffer holds.
     */
    size_t getBoneCount() const noexcept;

protected:
    // prevent heap allocation
    ~SkinningBuffer() = default;
};

} // namespace filament

#endif //TNT_FILAMENT_SKINNINGBUFFER_H
