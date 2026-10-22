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

#ifndef TNT_FILAMENT_TEMPLATE_TRAIT_SPECIALIZATIONS_TEST_H
#define TNT_FILAMENT_TEMPLATE_TRAIT_SPECIALIZATIONS_TEST_H

#include <filament/Engine.h>

#include <utils/compiler.h>

#include <math/vec3.h>

#include <type_traits>

#include <stdint.h>

#define UTILS_NOAPIGEN [[clang::annotate("apigen:skip")]]

namespace filament {

struct UTILS_NOAPIGEN TraitProvider {
    template <typename T>
    using supported_param_t = std::enable_if_t<
        std::is_same_v<float, T> ||
        std::is_same_v<int32_t, T> ||
        std::is_same_v<bool, T> ||
        std::is_same_v<math::float3, T>
    >;
};

class UTILS_PUBLIC TemplateTraitSpecializationsTest {
    struct BuilderDetails;
public:
    template <typename T>
    using is_supported_parameter_t = TraitProvider::supported_param_t<T>;

    /**
     * Set a configuration parameter by name.
     *
     * @param name Name of the parameter.
     * @param value Value of the parameter.
     */
    template <typename T, typename = is_supported_parameter_t<T>>
    void setParameter(const char* UTILS_NONNULL name, T value);

    /**
     * Get a configuration parameter by name.
     *
     * @param name Name of the parameter.
     * @return Value of the parameter.
     */
    template <typename T, typename = is_supported_parameter_t<T>>
    T getParameter(const char* UTILS_NONNULL name) const;

    class Builder : public BuilderBase<BuilderDetails> {
    public:
        Builder() noexcept;
        ~Builder() noexcept;

        template <typename T, typename = is_supported_parameter_t<T>>
        Builder& parameter(const char* UTILS_NONNULL name, T value);

        TemplateTraitSpecializationsTest* UTILS_NONNULL build(Engine& engine);
    };
};

} // namespace filament

#endif // TNT_FILAMENT_TEMPLATE_TRAIT_SPECIALIZATIONS_TEST_H
