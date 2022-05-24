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

#include <math/mathfwd.h>


namespace filament {

/**
 * MorphTargetBuffer is used to hold morphing data (positions and tangents).
 *
 * Both positions and tangents are required.
 *
 */
class UTILS_PUBLIC MorphTargetBuffer : public FilamentAPI {
    struct BuilderDetails;

public:
    class Builder : public BuilderBase<BuilderDetails> {
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
         * Creates the MorphTargetBuffer object and returns a pointer to it.
         *
         * @param engine Reference to the filament::Engine to associate this MorphTargetBuffer with.
         *
         * @return pointer to the newly created object or nullptr if exceptions are disabled and
         *         an error occurred.
         *
         * @exception utils::PostConditionPanic if a runtime error occurred, such as running out of
         *            memory or other resources.
         * @exception utils::PreConditionPanic if a parameter to a builder function was invalid.
         */
        MorphTargetBuffer* build(Engine& engine);
    private:
        friend class FMorphTargetBuffer;
    };

    /**
     * Updates positions for the given morph target.
     *
     * This is equivalent to the float4 method, but uses 1.0 for the 4th component.
     *
     * Both positions and tangents must be provided.
     *
     * @param engine Reference to the filament::Engine associated with this MorphTargetBuffer.
     * @param targetIndex the index of morph target to be updated.
     * @param positions pointer to at least "count" positions
     * @param count number of float3 vectors in positions
     * @param offset offset into the target buffer, expressed as a number of float4 vectors
     * @see setTangentsAt
     */
    void setPositionsAt(Engine& engine, size_t targetIndex,
            math::float3 const* positions, size_t count, size_t offset = 0);

    /**
     * Updates positions for the given morph target.
     *
     * Both positions and tangents must be provided.
     *
     * @param engine Reference to the filament::Engine associated with this MorphTargetBuffer.
     * @param targetIndex the index of morph target to be updated.
     * @param positions pointer to at least "count" positions
     * @param count number of float4 vectors in positions
     * @param offset offset into the target buffer, expressed as a number of float4 vectors
     * @see setTangentsAt
     */
    void setPositionsAt(Engine& engine, size_t targetIndex,
            math::float4 const* positions, size_t count, size_t offset = 0);

    /**
     * Updates tangents for the given morph target.
     *
     * These quaternions must be represented as signed shorts, where real numbers in the [-1,+1]
     * range multiplied by 32767.
     *
     * @param engine Reference to the filament::Engine associated with this MorphTargetBuffer.
     * @param targetIndex the index of morph target to be updated.
     * @param tangents pointer to at least "count" tangents
     * @param count number of short4 quaternions in tangents
     * @param offset offset into the target buffer, expressed as a number of short4 vectors
     * @see setPositionsAt
     */
    void setTangentsAt(Engine& engine, size_t targetIndex,
            math::short4 const* tangents, size_t count, size_t offset = 0);

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
};

} // namespace filament

#endif //TNT_FILAMENT_MORPHTARGETBUFFER_H
