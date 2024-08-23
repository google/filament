/*
* Copyright (C) 2023 The Android Open Source Project
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

#ifndef TNT_FILAMENT_INSTANCEBUFFER_H
#define TNT_FILAMENT_INSTANCEBUFFER_H

#include <filament/FilamentAPI.h>
#include <filament/Engine.h>

#include <utils/compiler.h>

#include <math/mathfwd.h>

#include <stddef.h>

namespace filament {

/**
 * InstanceBuffer holds draw (GPU) instance transforms. These can be provided to a renderable to
 * "offset" each draw instance.
 *
 * @see RenderableManager::Builder::instances(size_t, InstanceBuffer*)
 */
class UTILS_PUBLIC InstanceBuffer : public FilamentAPI {
    struct BuilderDetails;

public:
    class Builder : public BuilderBase<BuilderDetails>, public BuilderNameMixin<Builder> {
        friend struct BuilderDetails;

    public:

        /**
         * @param instanceCount the number of instances this InstanceBuffer will support, must be
         *                      >= 1 and <= \c Engine::getMaxAutomaticInstances()
         * @see Engine::getMaxAutomaticInstances
         */
        explicit Builder(size_t instanceCount) noexcept;

        Builder(Builder const& rhs) noexcept;
        Builder(Builder&& rhs) noexcept;
        ~Builder() noexcept;
        Builder& operator=(Builder const& rhs) noexcept;
        Builder& operator=(Builder&& rhs) noexcept;

        /**
         * Provide an initial local transform for each instance. Each local transform is relative to
         * the transform of the associated renderable. This forms a parent-child relationship
         * between the renderable and its instances, so adjusting the renderable's transform will
-        * affect all instances.
         *
         * The array of math::mat4f must have length instanceCount, provided when constructing this
         * Builder.
         *
         * @param localTransforms an array of math::mat4f with length instanceCount, must remain
         *                        valid until after build() is called
         */
        Builder& localTransforms(math::mat4f const* UTILS_NULLABLE localTransforms) noexcept;

        /**
         * Associate an optional name with this InstanceBuffer for debugging purposes.
         *
         * name will show in error messages and should be kept as short as possible. The name is
         * truncated to a maximum of 128 characters.
         *
         * The name string is copied during this method so clients may free its memory after
         * the function returns.
         *
         * @param name A string to identify this InstanceBuffer
         * @param len Length of name, should be less than or equal to 128
         * @return This Builder, for chaining calls.
         */
        // Builder& name(const char* UTILS_NONNULL name, size_t len) noexcept; // inherited

        /**
         * Creates the InstanceBuffer object and returns a pointer to it.
         */
        InstanceBuffer* UTILS_NONNULL build(Engine& engine);

    private:
        friend class FInstanceBuffer;
    };

    /**
     * Returns the instance count specified when building this InstanceBuffer.
     */
    size_t getInstanceCount() const noexcept;

    /**
     * Sets the local transform for each instance. Each local transform is relative to the transform
     * of the associated renderable. This forms a parent-child relationship between the renderable
     * and its instances, so adjusting the renderable's transform will affect all instances.
     *
     * @param localTransforms an array of math::mat4f with length count, need not outlive this call
     * @param count the number of local transforms
     * @param offset index of the first instance to set local transforms
     */
    void setLocalTransforms(math::mat4f const* UTILS_NONNULL localTransforms,
            size_t count, size_t offset = 0);

protected:
    // prevent heap allocation
    ~InstanceBuffer() = default;
};

} // namespace filament

#endif //TNT_FILAMENT_INSTANCEBUFFER_H
