/*
 * Copyright (C) 2026 The Android Open Source Project
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

#ifndef TNT_FILAMENT_DEPRECATED_TEST_H
#define TNT_FILAMENT_DEPRECATED_TEST_H

#include <filament/Engine.h>
#include <filament/FilamentAPI.h>

#include <math/vec3.h>

#include <utils/compiler.h>

#include <stdint.h>

namespace filament {

/**
 * Test class validating @Deprecated annotation generation.
 *
 * @deprecated Use ModernApi instead.
 */
class UTILS_PUBLIC DeprecatedTest : public FilamentAPI {
    struct BuilderDetails;

public:
    /**
     * Mode enum that is deprecated as a whole.
     *
     * @deprecated Use Quality instead.
     */
    enum class LegacyMode : uint8_t {
        /** Non-deprecated entry inside a deprecated enum. */
        FAST = 0,
        /**
         * Entry that is also deprecated.
         *
         * @deprecated Use Quality::HIGH instead.
         */
        SLOW = 1,
    };

    /**
     * Active enum with a deprecated entry.
     */
    enum class Quality : uint8_t {
        LOW = 0,
        /**
         * @deprecated Use HIGH instead.
         */
        MEDIUM = 1,
        /** High quality. */
        HIGH = 2,
    };

    /**
     * Deprecated options struct.
     *
     * @deprecated Use modern configuration methods instead.
     */
    struct LegacyOptions {
        /**
         * Nested enum inside an options struct.
         */
        enum class SubMode : uint8_t {
            DEFAULT = 0,
            /**
             * @deprecated Use SubMode::DEFAULT instead.
             */
            LEGACY = 1,
        };

        /**
         * @deprecated No longer used.
         */
        float legacyScale = 1.0f;

        bool enabled = true;
    };

    class Builder : public BuilderBase<BuilderDetails>, public BuilderNameMixin<Builder> {
        friend struct BuilderDetails;
    public:
        /**
         * @deprecated Legacy builder flag enum.
         */
        enum class BuilderFlag : uint8_t {
            NONE = 0,
            /**
             * @deprecated Legacy flag value.
             */
            LEGACY = 1,
        };

        Builder() noexcept;
        Builder(Builder const& rhs) noexcept;
        Builder(Builder&& rhs) noexcept;
        ~Builder() noexcept;
        Builder& operator=(Builder const& rhs) noexcept;
        Builder& operator=(Builder&& rhs) noexcept;

        /**
         * Configures a legacy option.
         *
         * @param value Legacy option value.
         * @return This Builder, for chaining calls.
         * @deprecated Use modernOption(int) instead.
         */
        Builder& legacyOption(int value) noexcept;

        /**
         * Configures a modern option.
         *
         * @param value Modern option value.
         * @return This Builder, for chaining calls.
         */
        Builder& modernOption(int value) noexcept;

        /**
         * Creates the DeprecatedTest object.
         *
         * @param engine Reference to the filament::Engine to associate this object with.
         * @return Pointer to the newly created DeprecatedTest object.
         * @deprecated DeprecatedTest is deprecated.
         */
        DeprecatedTest* UTILS_NONNULL build(Engine& engine);
    };

    /**
     * Legacy instance method.
     *
     * @deprecated Use modernMethod() instead.
     */
    void legacyMethod() noexcept;

    /**
     * Active non-deprecated instance method.
     */
    void modernMethod() noexcept;

    /**
     * Legacy method with default arguments to verify convenience overloads are annotated.
     *
     * @param count Item count.
     * @param factor Scaling factor.
     * @deprecated Use modernMethod() instead.
     */
    void legacyMethodWithDefaults(int count = 1, float factor = 2.0f) noexcept;

    /**
     * Legacy method taking a math vector to verify array overloads are annotated.
     *
     * @param value 3D vector value.
     * @deprecated Use modernMethod() instead.
     */
    void legacyVectorMethod(math::float3 value) noexcept;

    /**
     * Legacy static method.
     *
     * @param x Input value.
     * @return Computed value.
     * @deprecated No longer supported.
     */
    static int legacyStaticMethod(int x) noexcept;
};

} // namespace filament

#endif // TNT_FILAMENT_DEPRECATED_TEST_H
