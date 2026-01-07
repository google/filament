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

#ifndef TNT_FILAMENT_MORPHTARGETBUFFER_H
#define TNT_FILAMENT_MORPHTARGETBUFFER_H

#include <filament/FilamentAPI.h>

#include <filament/Engine.h>

#include <utils/compiler.h>
#include <utils/StaticString.h>

#include <math/mathfwd.h>

#include <stddef.h>

namespace filament {

/**
 * A container for vertex morphing data that supports both automatic and manual morphing.
 *
 * MorphTargetBuffer operates in a hybrid model depending on the attribute being morphed:
 *
 * 1.  Automatic for Built-ins (positions/tangents):
 *     Enable via `withPositions(true)` or `withTangents(true)`. The MorphTargetBuffer will
 *     allocate internal storage and hold the data for these attributes, which you upload
 *     via `setPositionsAt()` or `setTangentsAt()`. The framework automatically applies
 *     the morphing logic in the vertex shader.
 *
 * 2.  Manual for Custom Data (e.g., UVs, colors):
 *     The MorphTargetBuffer does NOT hold data for custom targets. The user is responsible
 *     for the full data pipeline:
 *     - Create and manage a separate `Texture` to hold the morph target data (offsets).
 *     - In the material, declare a `sampler2d_array` parameter.
 *     - Bind the `Texture` to the material instance.
 *     - In the vertex shader, manually call `morphData2`, `morphData3`, or `morphData4`
 *       with the custom sampler to apply the morphing.
 *
 * A MorphTargetBuffer object must be associated with a Renderable via
 * `RenderableManager::Builder::morphing()` to enable the morphing pipeline for all cases.
 *
 * @see RenderableManager
 */
class UTILS_PUBLIC MorphTargetBuffer : public FilamentAPI {
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
         * Size of the morph targets in vertex counts.
         * @param vertexCount Number of vertex counts the morph targets can hold.
         * @return A reference to this Builder for chaining calls.
         */
        Builder& vertexCount(size_t vertexCount) noexcept;

        /**
         * Size of the morph targets in targets.
         * @param count Number of targets the morph targets can hold.
         * @return A reference to this Builder for chaining calls.
         */
        Builder& count(size_t count) noexcept;

        /**
         * Associate an optional name with this MorphTargetBuffer for debugging purposes.
         *
         * name will show in error messages and should be kept as short as possible. The name is
         * truncated to a maximum of 128 characters.
         *
         * The name string is copied during this method so clients may free its memory after
         * the function returns.
         *
         * @param name A string to identify this MorphTargetBuffer
         * @param len Length of name, should be less than or equal to 128
         * @return This Builder, for chaining calls.
         * @deprecated Use name(utils::StaticString const&) instead.
         */
        UTILS_DEPRECATED
        Builder& name(const char* UTILS_NONNULL name, size_t len) noexcept;

        /**
         * Associate an optional name with this MorphTargetBuffer for debugging purposes.
         *
         * name will show in error messages and should be kept as short as possible.
         *
         * @param name A string literal to identify this MorphTargetBuffer
         * @return This Builder, for chaining calls.
         */
        Builder& name(utils::StaticString const& name) noexcept;

        /**
         * Enables and allocates the built-in buffer for position morphing.
         *
         * If enabled, `setPositionsAt` can be called to set the position data for each target.
         * The vertex position will be morphed automatically without any further actions.
         *
         * @param enable true to enable, false to disable. Default is true.
         * @return A reference to this Builder for chaining calls.
         */
        Builder& withPositions(bool enable = true) noexcept;

        /**
         * Enables and allocates the built-in buffer for tangent/normal morphing.
         *
         * If enabled, `setTangentsAt` can be called to set the tangent data for each target.
         * The vertex position will be morphed automatically without any further actions.
         *
         * @param enable true to enable, false to disable. Default is true.
         * @return A reference to this Builder for chaining calls.
         */
        Builder& withTangents(bool enable = true) noexcept;

        /**
         * Enables the custom morphing pipeline.
         *
         * When enabled, the `morphData2`, `morphData3`, and `morphData4` helper functions are
         * available in the vertex shader. You must provide a 2D array texture containing the morph
         * deltas, bind it to a `sampler2DArray` uniform, and call the appropriate `morphData`
         * function to apply the morphing to your custom attributes.
         *
         * Note: Unlike `withPositions` or `withTangents`, this does NOT allocate any internal
         * storage. You are responsible for managing the morph data texture.
         *
         * Custom morphing can be used together with automatic position and/or tangent morphing.
         *
         * @param enable true to enable, false to disable. Default is false.
         * @return A reference to this Builder for chaining calls.
         */
        Builder& enableCustomMorphing(bool enable) noexcept;

        /**
         * Creates the MorphTargetBuffer object and returns a pointer to it.
         *
         * @param engine Reference to the filament::Engine to associate this MorphTargetBuffer with.
         *
         * @return pointer to the newly created object.
         *
         * @exception utils::PostConditionPanic if a runtime error occurred, such as running out of
         *            memory or other resources.
         * @exception utils::PreConditionPanic if a parameter to a builder function was invalid.
         */
        MorphTargetBuffer* UTILS_NONNULL build(Engine& engine);
    private:
        friend class FMorphTargetBuffer;
    };

    /**
     * Updates positions for the given morph target.
     *
     * This method can only be called if the MorphTargetBuffer was built with `withPositions(true)`.
     * This is equivalent to the float4 method, but uses 1.0 for the 4th component.
     *
     * @param engine Reference to the filament::Engine associated with this MorphTargetBuffer.
     * @param targetIndex the index of morph target to be updated.
     * @param positions pointer to at least "count" positions
     * @param count number of float3 vectors in positions
     * @param offset offset into the target buffer, expressed as a number of float3 vectors
     */
    void setPositionsAt(Engine& engine, size_t targetIndex,
            math::float3 const* UTILS_NONNULL positions, size_t count, size_t offset = 0);

    /**
     * Updates positions for the given morph target.
     *
     * This method can only be called if the MorphTargetBuffer was built with `withPositions(true)`.
     *
     * @param engine Reference to the filament::Engine associated with this MorphTargetBuffer.
     * @param targetIndex the index of morph target to be updated.
     * @param positions pointer to at least "count" positions
     * @param count number of float4 vectors in positions
     * @param offset offset into the target buffer, expressed as a number of float4 vectors
     */
    void setPositionsAt(Engine& engine, size_t targetIndex,
            math::float4 const* UTILS_NONNULL positions, size_t count, size_t offset = 0);

    /**
     * Updates tangents for the given morph target.
     *
     * This method can only be called if the MorphTargetBuffer was built with `withTangents(true)`.
     * These quaternions must be represented as signed shorts, where real numbers in the [-1,+1]
     * range multiplied by 32767.
     *
     * @param engine Reference to the filament::Engine associated with this MorphTargetBuffer.
     * @param targetIndex the index of morph target to be updated.
     * @param tangents pointer to at least "count" tangents
     * @param count number of short4 quaternions in tangents
     * @param offset offset into the target buffer, expressed as a number of short4 vectors
     */
    void setTangentsAt(Engine& engine, size_t targetIndex,
            math::short4 const* UTILS_NONNULL tangents, size_t count, size_t offset = 0);

    /**
     * Returns the vertex count of this MorphTargetBuffer.
     * @return The number of vertices the MorphTargetBuffer holds.
     */
    size_t getVertexCount() const noexcept;

    /**
     * Returns the target count of this MorphTargetBuffer.
     * @return The number of targets the MorphTargetBuffer holds.
     */
    size_t getCount() const noexcept;

    /**
     * Returns true if this MorphTargetBuffer has a position buffer.
     * @see Builder::withPositions
     */
    bool hasPositions() const noexcept;

    /**
     * Returns true if this MorphTargetBuffer has a tangent buffer.
     * @see Builder::withTangents
     */
    bool hasTangents() const noexcept;

    /**
     * Returns true if custom morphing is enabled
     * @see Builder::enableCustomMorphing
     */
    bool isCustomMorphingEnabled() const noexcept;

protected:
    // prevent heap allocation
    ~MorphTargetBuffer() = default;
};

} // namespace filament

#endif //TNT_FILAMENT_MORPHTARGETBUFFER_H
