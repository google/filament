// Copyright 2023 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/lang/msl/writer/common/printer_support.h"

#include <cmath>
#include <limits>

#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/atomic.h"
#include "src/tint/lang/core/type/f16.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/utils/ice/ice.h"
#include "src/tint/utils/rtti/switch.h"
#include "src/tint/utils/strconv/float_to_string.h"

TINT_BEGIN_DISABLE_WARNING(UNSAFE_BUFFER_USAGE);

namespace tint::msl::writer {

std::string BuiltinToAttribute(core::BuiltinValue builtin) {
    switch (builtin) {
        case core::BuiltinValue::kPosition:
            return "position";
        case core::BuiltinValue::kVertexIndex:
            return "vertex_id";
        case core::BuiltinValue::kInstanceIndex:
            return "instance_id";
        case core::BuiltinValue::kFrontFacing:
            return "front_facing";
        case core::BuiltinValue::kFragDepth:
            return "depth(any)";
        case core::BuiltinValue::kLocalInvocationId:
            return "thread_position_in_threadgroup";
        case core::BuiltinValue::kLocalInvocationIndex:
            return "thread_index_in_threadgroup";
        case core::BuiltinValue::kGlobalInvocationId:
            return "thread_position_in_grid";
        case core::BuiltinValue::kWorkgroupId:
            return "threadgroup_position_in_grid";
        case core::BuiltinValue::kNumWorkgroups:
            return "threadgroups_per_grid";
        case core::BuiltinValue::kSampleIndex:
            return "sample_id";
        case core::BuiltinValue::kSampleMask:
            return "sample_mask";
        case core::BuiltinValue::kPointSize:
            return "point_size";
        case core::BuiltinValue::kSubgroupInvocationId:
            return "thread_index_in_simdgroup";
        case core::BuiltinValue::kSubgroupSize:
            return "threads_per_simdgroup";
        case core::BuiltinValue::kClipDistances:
            return "clip_distance";
        default:
            break;
    }
    return "";
}

std::string InterpolationToAttribute(core::InterpolationType type,
                                     core::InterpolationSampling sampling) {
    std::string attr;
    switch (sampling) {
        case core::InterpolationSampling::kCenter:
            attr = "center_";
            break;
        case core::InterpolationSampling::kCentroid:
            attr = "centroid_";
            break;
        case core::InterpolationSampling::kSample:
            attr = "sample_";
            break;
        case core::InterpolationSampling::kUndefined:
            if (type != core::InterpolationType::kFlat) {
                attr = "center_";
            }
            break;
        case core::InterpolationSampling::kFirst:
        case core::InterpolationSampling::kEither:
            break;
    }
    switch (type) {
        case core::InterpolationType::kPerspective:
            attr += "perspective";
            break;
        case core::InterpolationType::kLinear:
            attr += "no_perspective";
            break;
        case core::InterpolationType::kFlat:
            attr += "flat";
            break;
        case core::InterpolationType::kUndefined:
            break;
    }
    return attr;
}

SizeAndAlign MslPackedTypeSizeAndAlign(const core::type::Type* ty) {
    return tint::Switch(
        ty,

        // https://developer.apple.com/metal/Metal-Shading-Language-Specification.pdf
        // 2.1 Scalar Data Types
        [&](const core::type::U32*) {
            return SizeAndAlign{4, 4};
        },
        [&](const core::type::I32*) {
            return SizeAndAlign{4, 4};
        },
        [&](const core::type::F32*) {
            return SizeAndAlign{4, 4};
        },
        [&](const core::type::F16*) {
            return SizeAndAlign{2, 2};
        },

        [&](const core::type::Vector* vec) {
            auto num_els = vec->Width();
            auto* el_ty = vec->Type();
            SizeAndAlign el_size_align = MslPackedTypeSizeAndAlign(el_ty);
            if (el_ty->IsAnyOf<core::type::U32, core::type::I32, core::type::F32,
                               core::type::F16>()) {
                // Use a packed_vec type for 3-element vectors only.
                if (num_els == 3) {
                    // https://developer.apple.com/metal/Metal-Shading-Language-Specification.pdf
                    // 2.2.3 Packed Vector Types
                    return SizeAndAlign{num_els * el_size_align.size, el_size_align.align};
                } else {
                    // https://developer.apple.com/metal/Metal-Shading-Language-Specification.pdf
                    // 2.2 Vector Data Types
                    // Vector data types are aligned to their size.
                    return SizeAndAlign{num_els * el_size_align.size, num_els * el_size_align.size};
                }
            }
            TINT_UNREACHABLE() << "Unhandled vector element type " << el_ty->TypeInfo().name;
        },

        [&](const core::type::Matrix* mat) {
            // https://developer.apple.com/metal/Metal-Shading-Language-Specification.pdf
            // 2.3 Matrix Data Types
            auto cols = mat->Columns();
            auto rows = mat->Rows();
            auto* el_ty = mat->Type();
            // Metal only support half and float matrix.
            if (el_ty->IsAnyOf<core::type::F32, core::type::F16>()) {
                static constexpr SizeAndAlign table_f32[] = {
                    /* float2x2 */ {16, 8},
                    /* float2x3 */ {32, 16},
                    /* float2x4 */ {32, 16},
                    /* float3x2 */ {24, 8},
                    /* float3x3 */ {48, 16},
                    /* float3x4 */ {48, 16},
                    /* float4x2 */ {32, 8},
                    /* float4x3 */ {64, 16},
                    /* float4x4 */ {64, 16},
                };
                static constexpr SizeAndAlign table_f16[] = {
                    /* half2x2 */ {8, 4},
                    /* half2x3 */ {16, 8},
                    /* half2x4 */ {16, 8},
                    /* half3x2 */ {12, 4},
                    /* half3x3 */ {24, 8},
                    /* half3x4 */ {24, 8},
                    /* half4x2 */ {16, 4},
                    /* half4x3 */ {32, 8},
                    /* half4x4 */ {32, 8},
                };
                if (cols >= 2 && cols <= 4 && rows >= 2 && rows <= 4) {
                    if (el_ty->Is<core::type::F32>()) {
                        return table_f32[(3 * (cols - 2)) + (rows - 2)];
                    } else {
                        return table_f16[(3 * (cols - 2)) + (rows - 2)];
                    }
                }
            }

            TINT_UNREACHABLE() << "Unhandled matrix element type " << el_ty->TypeInfo().name;
        },

        [&](const core::type::Array* arr) {
            if (DAWN_UNLIKELY(!arr->IsStrideImplicit())) {
                TINT_ICE()
                    << "arrays with explicit strides should not exist past the SPIR-V reader";
            }
            if (arr->Count()->Is<core::type::RuntimeArrayCount>()) {
                return SizeAndAlign{arr->Stride(), arr->Align()};
            }
            if (auto count = arr->ConstantCount()) {
                return SizeAndAlign{arr->Stride() * count.value(), arr->Align()};
            }
            TINT_ICE() << core::type::Array::kErrExpectedConstantCount;
        },

        [&](const core::type::Struct* str) {
            // TODO(crbug.com/tint/650): There's an assumption here that MSL's
            // default structure size and alignment matches WGSL's. We need to
            // confirm this.
            return SizeAndAlign{str->Size(), str->Align()};
        },

        [&](const core::type::Atomic* atomic) { return MslPackedTypeSizeAndAlign(atomic->Type()); },

        TINT_ICE_ON_NO_MATCH);
}

void PrintF32(StringStream& out, float value) {
    // Note: Currently inf and nan should not be constructable, but this is implemented for the day
    // we support them.
    if (std::isinf(value)) {
        out << (value >= 0 ? "INFINITY" : "-INFINITY");
    } else if (std::isnan(value)) {
        out << "NAN";
    } else {
        out << tint::strconv::FloatToString(value) << "f";
    }
}

void PrintF16(StringStream& out, float value) {
    // Note: Currently inf and nan should not be constructable, but this is implemented for the day
    // we support them.
    if (std::isinf(value)) {
        // HUGE_VALH evaluates to +infinity.
        out << (value >= 0 ? "HUGE_VALH" : "-HUGE_VALH");
    } else if (std::isnan(value)) {
        // There is no NaN expr for half in MSL, "NAN" is of float type.
        out << "NAN";
    } else {
        out << tint::strconv::FloatToString(value) << "h";
    }
}

void PrintI32(StringStream& out, int32_t value) {
    // MSL (and C++) parse `-2147483648` as a `long` because it parses unary minus and `2147483648`
    // as separate tokens, and the latter doesn't fit into an (32-bit) `int`.
    // WGSL, on the other hand, parses this as an `i32`.
    // To avoid issues with `long` to `int` casts, emit `(-2147483647 - 1)` instead, which ensures
    // the expression type is `int`.
    if (auto int_min = std::numeric_limits<int32_t>::min(); value == int_min) {
        out << "(" << int_min + 1 << " - 1)";
    } else {
        out << value;
    }
}

}  // namespace tint::msl::writer

TINT_END_DISABLE_WARNING(UNSAFE_BUFFER_USAGE);
