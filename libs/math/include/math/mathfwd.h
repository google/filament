/*
 * Copyright (C) 2020 The Android Open Source Project
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

#ifndef MATH_MATHFWD_H_
#define MATH_MATHFWD_H_

#ifdef _MSC_VER

// MSVC cannot compute the size of math types correctly when this file is included before the
// actual implementations.
// See github.com/google/filament/issues/2190.
#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>
#include <math/mat2.h>
#include <math/mat3.h>
#include <math/mat4.h>

#else

#include <stdint.h>

namespace filament {
namespace math {
namespace details {

template<typename T> class TVec2;
template<typename T> class TVec3;
template<typename T> class TVec4;

template<typename T> class TMat22;
template<typename T> class TMat33;
template<typename T> class TMat44;

}  // namespace details

using double2   = details::TVec2<double>;
using float2    = details::TVec2<float>;
using int2      = details::TVec2<int32_t>;
using uint2     = details::TVec2<uint32_t>;
using short2    = details::TVec2<int16_t>;
using ushort2   = details::TVec2<uint16_t>;
using byte2     = details::TVec2<int8_t>;
using ubyte2    = details::TVec2<uint8_t>;
using bool2     = details::TVec2<bool>;

using double3   = details::TVec3<double>;
using float3    = details::TVec3<float>;
using int3      = details::TVec3<int32_t>;
using uint3     = details::TVec3<uint32_t>;
using short3    = details::TVec3<int16_t>;
using ushort3   = details::TVec3<uint16_t>;
using byte3     = details::TVec3<int8_t>;
using ubyte3    = details::TVec3<uint8_t>;
using bool3     = details::TVec3<bool>;

using double4   = details::TVec4<double>;
using float4    = details::TVec4<float>;
using int4      = details::TVec4<int32_t>;
using uint4     = details::TVec4<uint32_t>;
using short4    = details::TVec4<int16_t>;
using ushort4   = details::TVec4<uint16_t>;
using byte4     = details::TVec4<int8_t>;
using ubyte4    = details::TVec4<uint8_t>;
using bool4     = details::TVec4<bool>;

using mat2      = details::TMat22<double>;
using mat2f     = details::TMat22<float>;

using mat3      = details::TMat33<double>;
using mat3f     = details::TMat33<float>;

using mat4      = details::TMat44<double>;
using mat4f     = details::TMat44<float>;

}  // namespace math
}  // namespace filament

#endif // _MSC_VER

#endif  // MATH_MATHFWD_H_
