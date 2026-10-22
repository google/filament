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

#ifndef TNT_FILAMENT_TAGGED_ARRAY_BUFFERS_TEST_H
#define TNT_FILAMENT_TAGGED_ARRAY_BUFFERS_TEST_H

#include <utils/compiler.h>
#include <utils/Slice.h>

#include <math/mat4.h>
#include <math/vec2.h>
#include <math/vec4.h>

#include <string_view>
#include <type_traits>

#include <stdint.h>

namespace filament {

class UTILS_PUBLIC TaggedArrayBuffersTest {
public:
    template <typename T>
    using is_supported_t = std::enable_if_t<
        std::is_same_v<float, T> ||
        std::is_same_v<math::float2, T> ||
        std::is_same_v<math::float4, T> ||
        std::is_same_v<math::mat4f, T> ||
        std::is_same_v<int32_t, T> ||
        std::is_same_v<math::int4, T> ||
        std::is_same_v<bool, T> ||
        std::is_same_v<math::bool4, T>
    >;

    /**
     * Set a uniform buffer parameter array by name.
     *
     * @param name Name of the parameter.
     * @param values Slice of values to set.
     */
    template <typename T, typename = is_supported_t<T>>
    void setBuffer(std::string_view name,
            UTILS_APIGEN_TAGGED_ARRAY utils::Slice<const T> values);
};

} // namespace filament

#endif // TNT_FILAMENT_TAGGED_ARRAY_BUFFERS_TEST_H
