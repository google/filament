// Copyright 2020 The Dawn & Tint Authors
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

#include "src/tint/lang/wgsl/ast/transform/vertex_pulling.h"

#include <algorithm>
#include <unordered_map>
#include <utility>

#include "src/tint/lang/core/builtin_value.h"
#include "src/tint/lang/wgsl/ast/assignment_statement.h"
#include "src/tint/lang/wgsl/ast/variable_decl_statement.h"
#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"
#include "src/tint/lang/wgsl/sem/variable.h"
#include "src/tint/utils/containers/map.h"
#include "src/tint/utils/macros/compiler.h"
#include "src/tint/utils/math/math.h"
#include "src/tint/utils/rtti/switch.h"
#include "src/tint/utils/text/string_stream.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::transform::VertexPulling);
TINT_INSTANTIATE_TYPEINFO(tint::ast::transform::VertexPulling::Config);

namespace tint::ast::transform {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace {

/// The base WGSL type of a component.
/// The format type is either this type or a vector of this type.
enum class BaseWGSLType {
    kInvalid,
    kU32,
    kI32,
    kF32,
    kF16,
};

/// The data type of a vertex format.
/// The format type is either this type or a vector of this type.
enum class VertexDataType {
    kInvalid,
    kUInt,   // unsigned int
    kSInt,   // signed int
    kFloat,  // unsigned normalized, signed normalized, and float
};

/// Writes the VertexFormat to the diagnostic.
/// @param out the stream to write to
/// @param format the VertexFormat to write
/// @returns out so calls can be chained
diag::Diagnostic& operator<<(diag::Diagnostic& out, VertexFormat format) {
    switch (format) {
        case VertexFormat::kUint8:
            return out << "uint8";
        case VertexFormat::kUint8x2:
            return out << "uint8x2";
        case VertexFormat::kUint8x4:
            return out << "uint8x4";
        case VertexFormat::kSint8:
            return out << "sint8";
        case VertexFormat::kSint8x2:
            return out << "sint8x2";
        case VertexFormat::kSint8x4:
            return out << "sint8x4";
        case VertexFormat::kUnorm8:
            return out << "unorm8";
        case VertexFormat::kUnorm8x2:
            return out << "unorm8x2";
        case VertexFormat::kUnorm8x4:
            return out << "unorm8x4";
        case VertexFormat::kSnorm8:
            return out << "snorm8";
        case VertexFormat::kSnorm8x2:
            return out << "snorm8x2";
        case VertexFormat::kSnorm8x4:
            return out << "snorm8x4";
        case VertexFormat::kUint16:
            return out << "uint16";
        case VertexFormat::kUint16x2:
            return out << "uint16x2";
        case VertexFormat::kUint16x4:
            return out << "uint16x4";
        case VertexFormat::kSint16:
            return out << "sint16";
        case VertexFormat::kSint16x2:
            return out << "sint16x2";
        case VertexFormat::kSint16x4:
            return out << "sint16x4";
        case VertexFormat::kUnorm16:
            return out << "unorm16";
        case VertexFormat::kUnorm16x2:
            return out << "unorm16x2";
        case VertexFormat::kUnorm16x4:
            return out << "unorm16x4";
        case VertexFormat::kSnorm16:
            return out << "snorm16";
        case VertexFormat::kSnorm16x2:
            return out << "snorm16x2";
        case VertexFormat::kSnorm16x4:
            return out << "snorm16x4";
        case VertexFormat::kFloat16:
            return out << "float16";
        case VertexFormat::kFloat16x2:
            return out << "float16x2";
        case VertexFormat::kFloat16x4:
            return out << "float16x4";
        case VertexFormat::kFloat32:
            return out << "float32";
        case VertexFormat::kFloat32x2:
            return out << "float32x2";
        case VertexFormat::kFloat32x3:
            return out << "float32x3";
        case VertexFormat::kFloat32x4:
            return out << "float32x4";
        case VertexFormat::kUint32:
            return out << "uint32";
        case VertexFormat::kUint32x2:
            return out << "uint32x2";
        case VertexFormat::kUint32x3:
            return out << "uint32x3";
        case VertexFormat::kUint32x4:
            return out << "uint32x4";
        case VertexFormat::kSint32:
            return out << "sint32";
        case VertexFormat::kSint32x2:
            return out << "sint32x2";
        case VertexFormat::kSint32x3:
            return out << "sint32x3";
        case VertexFormat::kSint32x4:
            return out << "sint32x4";
        case VertexFormat::kUnorm10_10_10_2:
            return out << "unorm10-10-10-2";
        case VertexFormat::kUnorm8x4BGRA:
            return out << "unorm8x4-bgra";
    }
    return out << "<unknown>";
}

/// Type information of a vertex input attribute.
struct AttributeWGSLType {
    BaseWGSLType base_type;
    uint32_t width;  // 1 for scalar, 2+ for a vector
};

/// Type information of a vertex format.
struct VertexFormatType {
    VertexDataType base_type;
    uint32_t width;  // 1 for scalar, 2+ for a vector
};

// Check if base types match between the WGSL variable and the vertex format
bool IsTypeCompatible(AttributeWGSLType wgslType, VertexFormatType vertexFormatType) {
    switch (wgslType.base_type) {
        case BaseWGSLType::kF32:
        case BaseWGSLType::kF16:
            return (vertexFormatType.base_type == VertexDataType::kFloat);
        case BaseWGSLType::kU32:
            return (vertexFormatType.base_type == VertexDataType::kUInt);
        case BaseWGSLType::kI32:
            return (vertexFormatType.base_type == VertexDataType::kSInt);
        default:
            return false;
    }
}

AttributeWGSLType WGSLTypeOf(const core::type::Type* ty) {
    return Switch(
        ty,
        [](const core::type::I32*) -> AttributeWGSLType {
            return {BaseWGSLType::kI32, 1};
        },
        [](const core::type::U32*) -> AttributeWGSLType {
            return {BaseWGSLType::kU32, 1};
        },
        [](const core::type::F32*) -> AttributeWGSLType {
            return {BaseWGSLType::kF32, 1};
        },
        [](const core::type::F16*) -> AttributeWGSLType {
            return {BaseWGSLType::kF16, 1};
        },
        [](const core::type::Vector* vec) -> AttributeWGSLType {
            return {WGSLTypeOf(vec->Type()).base_type, vec->Width()};
        },
        [](Default) -> AttributeWGSLType {
            return {BaseWGSLType::kInvalid, 0};
        });
}

VertexFormatType VertexFormatTypeOf(VertexFormat format) {
    switch (format) {
        case VertexFormat::kUint8:
        case VertexFormat::kUint16:
        case VertexFormat::kUint32:
            return {VertexDataType::kUInt, 1};
        case VertexFormat::kUint8x2:
        case VertexFormat::kUint16x2:
        case VertexFormat::kUint32x2:
            return {VertexDataType::kUInt, 2};
        case VertexFormat::kUint32x3:
            return {VertexDataType::kUInt, 3};
        case VertexFormat::kUint8x4:
        case VertexFormat::kUint16x4:
        case VertexFormat::kUint32x4:
            return {VertexDataType::kUInt, 4};
        case VertexFormat::kSint8:
        case VertexFormat::kSint16:
        case VertexFormat::kSint32:
            return {VertexDataType::kSInt, 1};
        case VertexFormat::kSint8x2:
        case VertexFormat::kSint16x2:
        case VertexFormat::kSint32x2:
            return {VertexDataType::kSInt, 2};
        case VertexFormat::kSint32x3:
            return {VertexDataType::kSInt, 3};
        case VertexFormat::kSint8x4:
        case VertexFormat::kSint16x4:
        case VertexFormat::kSint32x4:
            return {VertexDataType::kSInt, 4};
        case VertexFormat::kUnorm8:
        case VertexFormat::kSnorm8:
        case VertexFormat::kUnorm16:
        case VertexFormat::kSnorm16:
        case VertexFormat::kFloat16:
        case VertexFormat::kFloat32:
            return {VertexDataType::kFloat, 1};
        case VertexFormat::kUnorm8x2:
        case VertexFormat::kSnorm8x2:
        case VertexFormat::kUnorm16x2:
        case VertexFormat::kSnorm16x2:
        case VertexFormat::kFloat16x2:
        case VertexFormat::kFloat32x2:
            return {VertexDataType::kFloat, 2};
        case VertexFormat::kFloat32x3:
            return {VertexDataType::kFloat, 3};
        case VertexFormat::kUnorm8x4:
        case VertexFormat::kSnorm8x4:
        case VertexFormat::kUnorm16x4:
        case VertexFormat::kSnorm16x4:
        case VertexFormat::kFloat16x4:
        case VertexFormat::kFloat32x4:
        case VertexFormat::kUnorm10_10_10_2:
        case VertexFormat::kUnorm8x4BGRA:
            return {VertexDataType::kFloat, 4};
    }
    return {VertexDataType::kInvalid, 0};
}

}  // namespace

/// PIMPL state for the transform
struct VertexPulling::State {
    /// Constructor
    /// @param program the source program
    /// @param c the VertexPulling config
    State(const Program& program, const VertexPulling::Config& c) : src(program), cfg(c) {}

    /// Runs the transform
    /// @returns the new program or SkipTransform if the transform is not required
    ApplyResult Run() {
        // Find entry point
        const Function* func = nullptr;
        for (auto* fn : src.AST().Functions()) {
            if (fn->PipelineStage() == PipelineStage::kVertex) {
                if (func != nullptr) {
                    b.Diagnostics().AddError(Source{})
                        << "VertexPulling found more than one vertex entry point";
                    return resolver::Resolve(b);
                }
                func = fn;
            }
        }
        if (func == nullptr) {
            b.Diagnostics().AddError(Source{}) << "Vertex stage entry point not found";
            return resolver::Resolve(b);
        }

        AddVertexStorageBuffers();
        Process(func);

        ctx.Clone();
        return resolver::Resolve(b);
    }

  private:
    /// LocationReplacement describes an Variable replacement for a location input.
    struct LocationReplacement {
        /// The variable to replace in the source Program
        Variable* from;
        /// The replacement to use in the target ProgramBuilder
        Variable* to;
    };

    /// LocationInfo describes an input location
    struct LocationInfo {
        /// A builder that builds the expression that resolves to the (transformed) input location
        std::function<const Expression*()> expr;
        /// The store type of the location variable
        const core::type::Type* type;
    };

    /// The source program
    const Program& src;
    /// The transform config
    VertexPulling::Config const cfg;
    /// The target program builder
    ProgramBuilder b;
    /// The clone context
    program::CloneContext ctx = {&b, &src, /* auto_clone_symbols */ true};
    std::unordered_map<uint32_t, LocationInfo> location_info;
    std::function<const Expression*()> vertex_index_expr = nullptr;
    std::function<const Expression*()> instance_index_expr = nullptr;
    Symbol pulling_position_name;
    Symbol struct_buffer_name;
    std::unordered_map<uint32_t, Symbol> vertex_buffer_names;
    tint::Vector<const Parameter*, 8> new_function_parameters;

    /// Generate the vertex buffer binding name
    /// @param index index to append to buffer name
    Symbol GetVertexBufferName(uint32_t index) {
        return tint::GetOrAdd(vertex_buffer_names, index, [&] {
            static const char kVertexBufferNamePrefix[] = "tint_pulling_vertex_buffer_";
            return b.Symbols().New(kVertexBufferNamePrefix + std::to_string(index));
        });
    }

    /// Lazily generates the structure buffer symbol
    Symbol GetStructBufferName() {
        if (!struct_buffer_name.IsValid()) {
            static const char kStructBufferName[] = "tint_vertex_data";
            struct_buffer_name = b.Symbols().New(kStructBufferName);
        }
        return struct_buffer_name;
    }

    /// Adds storage buffer decorated variables for the vertex buffers
    void AddVertexStorageBuffers() {
        // Creating the struct type
        static const char kStructName[] = "TintVertexData";
        auto* struct_type = b.Structure(b.Symbols().New(kStructName),
                                        tint::Vector{
                                            b.Member(GetStructBufferName(), b.ty.array<u32>()),
                                        });
        for (uint32_t i = 0; i < cfg.vertex_state.size(); ++i) {
            // The decorated variable with struct type
            b.GlobalVar(GetVertexBufferName(i), b.ty.Of(struct_type), core::AddressSpace::kStorage,
                        core::Access::kRead, b.Binding(AInt(i)), b.Group(AInt(cfg.pulling_group)));
        }
    }

    /// Creates and returns the assignment to the variables from the buffers
    const BlockStatement* CreateVertexPullingPreamble() {
        // Assign by looking at the vertex descriptor to find attributes with
        // matching location.

        tint::Vector<const Statement*, 8> stmts;

        for (uint32_t buffer_idx = 0; buffer_idx < cfg.vertex_state.size(); ++buffer_idx) {
            const VertexBufferLayoutDescriptor& buffer_layout = cfg.vertex_state[buffer_idx];

            if ((buffer_layout.array_stride & 3) != 0) {
                b.Diagnostics().AddError(Source{})
                    << "WebGPU requires that vertex stride must be a multiple of 4 bytes, "
                       "but VertexPulling array stride for buffer "
                    << buffer_idx << " was " << buffer_layout.array_stride << " bytes";
                return nullptr;
            }

            auto* index_expr = buffer_layout.step_mode == VertexStepMode::kVertex
                                   ? vertex_index_expr()
                                   : instance_index_expr();

            // buffer_array_base is the base array offset for all the vertex
            // attributes. These are units of uint (4 bytes).
            auto buffer_array_base =
                b.Symbols().New("buffer_array_base_" + std::to_string(buffer_idx));

            auto* attribute_offset = index_expr;
            if (buffer_layout.array_stride != 4) {
                attribute_offset = b.Mul(index_expr, u32(buffer_layout.array_stride / 4u));
            }

            // let pulling_offset_n = <attribute_offset>
            stmts.Push(b.Decl(b.Let(buffer_array_base, attribute_offset)));

            for (const VertexAttributeDescriptor& attribute_desc : buffer_layout.attributes) {
                auto it = location_info.find(attribute_desc.shader_location);
                if (it == location_info.end()) {
                    continue;
                }
                auto& var = it->second;

                // Data type of the target WGSL variable
                auto var_dt = WGSLTypeOf(var.type);
                // Data type of the vertex stream attribute
                auto fmt_dt = VertexFormatTypeOf(attribute_desc.format);

                // Base types must match between the vertex stream and the WGSL variable
                if (!IsTypeCompatible(var_dt, fmt_dt)) {
                    b.Diagnostics().AddError(Source{})
                        << "VertexAttributeDescriptor for location "
                        << attribute_desc.shader_location << " has format " << attribute_desc.format
                        << " but shader expects " << var.type->FriendlyName();
                    return nullptr;
                }

                // Load the attribute value according to vertex format and convert the element type
                // of result to match target WGSL variable. The result of `Fetch` should be of WGSL
                // types `f32`, `i32`, `u32`, and their vectors, while WGSL variable can be of
                // `f16`.
                auto* fetch = Fetch(buffer_array_base, attribute_desc.offset, buffer_idx,
                                    attribute_desc.format);
                // Convert the fetched scalar/vector if WGSL variable is of `f16` types
                if (var_dt.base_type == BaseWGSLType::kF16) {
                    // The type of the same element number of base type of target WGSL variable
                    Type loaded_data_target_type;
                    if (fmt_dt.width == 1) {
                        loaded_data_target_type = b.ty.f16();
                    } else {
                        loaded_data_target_type = b.ty.vec(b.ty.f16(), fmt_dt.width);
                    }

                    fetch = b.Call(loaded_data_target_type, fetch);
                }

                // The attribute value may not be of the desired vector width. If it is not, we'll
                // need to either reduce the width with a swizzle, or append 0's and / or a 1.
                auto* value = fetch;
                if (var_dt.width < fmt_dt.width) {
                    // WGSL variable vector width is smaller than the loaded vector width
                    switch (var_dt.width) {
                        case 1:
                            value = b.MemberAccessor(fetch, "x");
                            break;
                        case 2:
                            value = b.MemberAccessor(fetch, "xy");
                            break;
                        case 3:
                            value = b.MemberAccessor(fetch, "xyz");
                            break;
                        default:
                            TINT_UNREACHABLE() << var_dt.width;
                    }
                } else if (var_dt.width > fmt_dt.width) {
                    // WGSL variable vector width is wider than the loaded vector width, do padding.

                    // The components of result vector variable, initialized with type-converted
                    // loaded data vector.
                    tint::Vector<const Expression*, 8> values{fetch};

                    // Add padding elements. The result must be of vector types of signed/unsigned
                    // integer or float, so use the abstract integer or abstract float value to do
                    // padding.
                    for (uint32_t i = fmt_dt.width; i < var_dt.width; i++) {
                        if (var_dt.base_type == BaseWGSLType::kI32 ||
                            var_dt.base_type == BaseWGSLType::kU32) {
                            values.Push(b.Expr((i == 3) ? 1_a : 0_a));
                        } else {
                            values.Push(b.Expr((i == 3) ? 1.0_a : 0.0_a));
                        }
                    }

                    value = b.Call(CreateASTTypeFor(ctx, var.type), values);
                }

                // Assign the value to the WGSL variable
                stmts.Push(b.Assign(var.expr(), value));
            }
        }

        if (stmts.IsEmpty()) {
            return nullptr;
        }

        return b.Block(std::move(stmts));
    }

    /// Generates an expression reading a specific vertex format from a buffer. Any vertex format of
    /// signed normailized, unsigned normailized, or float will result in `f32` or `vecN<f32>` WGSL
    /// type.
    /// @param array_base the symbol of the variable holding the base array offset
    /// of the vertex array (each index is 4-bytes).
    /// @param offset the byte offset of the data from `buffer_base`
    /// @param buffer the index of the vertex buffer
    /// @param format the vertex format to read
    const Expression* Fetch(Symbol array_base,
                            uint32_t offset,
                            uint32_t buffer,
                            VertexFormat format) {
        // Returns a u32 loaded from buffer_base + offset.
        auto load_u32 = [&] {
            return LoadPrimitive(array_base, offset, buffer, VertexFormat::kUint32);
        };

        // Returns a i32 loaded from buffer_base + offset.
        auto load_i32 = [&] { return b.Bitcast<i32>(load_u32()); };

        // Returns a u32 loaded from buffer_base + offset + 4.
        auto load_next_u32 = [&] {
            return LoadPrimitive(array_base, offset + 4, buffer, VertexFormat::kUint32);
        };

        // Returns a i32 loaded from buffer_base + offset + 4.
        auto load_next_i32 = [&] { return b.Bitcast<i32>(load_next_u32()); };

        // Returns a u16 loaded from offset, packed in the high 16 bits of a u32.
        // The low 16 bits are 0.
        // `min_alignment` must be a power of two.
        // `offset` must be `min_alignment` bytes aligned.
        auto load_u16_h = [&] {
            auto low_u32_offset = offset & ~3u;
            auto* low_u32 =
                LoadPrimitive(array_base, low_u32_offset, buffer, VertexFormat::kUint32);
            switch (offset & 3) {
                case 0:
                    return b.Shl(low_u32, 16_u);
                case 1:
                    return b.And(b.Shl(low_u32, 8_u), 0xffff0000_u);
                case 2:
                    return b.And(low_u32, 0xffff0000_u);
                default: {  // 3:
                    auto* high_u32 = LoadPrimitive(array_base, low_u32_offset + 4, buffer,
                                                   VertexFormat::kUint32);
                    auto* shr = b.Shr(low_u32, 8_u);
                    auto* shl = b.Shl(high_u32, 24_u);
                    return b.And(b.Or(shl, shr), 0xffff0000_u);
                }
            }
        };

        // Returns a u16 loaded from offset, packed in the low 16 bits of a u32.
        // The high 16 bits are 0.
        auto load_u16_l = [&] {
            auto low_u32_offset = offset & ~3u;
            auto* low_u32 =
                LoadPrimitive(array_base, low_u32_offset, buffer, VertexFormat::kUint32);
            switch (offset & 3) {
                case 0:
                    return b.And(low_u32, 0xffff_u);
                case 1:
                    return b.And(b.Shr(low_u32, 8_u), 0xffff_u);
                case 2:
                    return b.Shr(low_u32, 16_u);
                default: {  // 3:
                    auto* high_u32 = LoadPrimitive(array_base, low_u32_offset + 4, buffer,
                                                   VertexFormat::kUint32);
                    auto* shr = b.Shr(low_u32, 24_u);
                    auto* shl = b.Shl(high_u32, 8_u);
                    return b.And(b.Or(shl, shr), 0xffff_u);
                }
            }
        };

        // Returns a i16 loaded from offset, packed in the high 16 bits of a u32.
        // The low 16 bits are 0.
        auto load_i16_h = [&] { return b.Bitcast<i32>(load_u16_h()); };

        // Returns a u8 loaded from offset, in the low 8 bits of a u32. Other bits are 0.
        auto load_u8_l = [&] {
            auto u32_offset = offset & ~3u;
            auto remainder_offset = offset - u32_offset;

            auto* u8s = LoadPrimitive(array_base, u32_offset, buffer, VertexFormat::kUint32);
            auto* shifts = b.Call<vec4<u32>>(0_u, 8_u, 16_u, 24_u);
            auto* shift = b.IndexAccessor(shifts, u32(remainder_offset));

            return b.And(b.Shr(u8s, shift), 0xFF_u);
        };

        // Returns an i8 loaded from offset, in the low 8 bits of a u32. Other bits are 0.
        auto load_i8_l = [&] {
            auto u32_offset = offset & ~3u;
            auto remainder_offset = offset - u32_offset;

            auto* i8s = LoadPrimitive(array_base, u32_offset, buffer, VertexFormat::kSint32);
            auto* shifts = b.Call<vec4<u32>>(24_u, 16_u, 8_u, 0_u);
            auto* shift = b.IndexAccessor(shifts, u32(remainder_offset));

            auto* i8_high = b.Shl(i8s, shift);
            return b.Shr(i8_high, 24_u);
        };

        // Assumptions are made that alignment must be at least as large as the size
        // of a single component.
        switch (format) {
            // Basic primitives
            case VertexFormat::kUint32:
            case VertexFormat::kSint32:
            case VertexFormat::kFloat32:
                return LoadPrimitive(array_base, offset, buffer, format);

                // Vectors of basic primitives
            case VertexFormat::kUint32x2:
                return LoadVec(array_base, offset, buffer, 4, b.ty.u32(), VertexFormat::kUint32, 2);
            case VertexFormat::kUint32x3:
                return LoadVec(array_base, offset, buffer, 4, b.ty.u32(), VertexFormat::kUint32, 3);
            case VertexFormat::kUint32x4:
                return LoadVec(array_base, offset, buffer, 4, b.ty.u32(), VertexFormat::kUint32, 4);
            case VertexFormat::kSint32x2:
                return LoadVec(array_base, offset, buffer, 4, b.ty.i32(), VertexFormat::kSint32, 2);
            case VertexFormat::kSint32x3:
                return LoadVec(array_base, offset, buffer, 4, b.ty.i32(), VertexFormat::kSint32, 3);
            case VertexFormat::kSint32x4:
                return LoadVec(array_base, offset, buffer, 4, b.ty.i32(), VertexFormat::kSint32, 4);
            case VertexFormat::kFloat32x2:
                return LoadVec(array_base, offset, buffer, 4, b.ty.f32(), VertexFormat::kFloat32,
                               2);
            case VertexFormat::kFloat32x3:
                return LoadVec(array_base, offset, buffer, 4, b.ty.f32(), VertexFormat::kFloat32,
                               3);
            case VertexFormat::kFloat32x4:
                return LoadVec(array_base, offset, buffer, 4, b.ty.f32(), VertexFormat::kFloat32,
                               4);

            case VertexFormat::kUint8: {
                return load_u8_l();
            }
            case VertexFormat::kUint8x2: {
                // yyxx0000, yyxx0000
                auto* u16s = b.Call<vec2<u32>>(load_u16_h());
                // xx000000, yyxx0000
                auto* shl = b.Shl(u16s, b.Call<vec2<u32>>(8_u, 0_u));
                // 000000xx, 000000yy
                return b.Shr(shl, b.Call<vec2<u32>>(24_u));
            }
            case VertexFormat::kUint8x4: {
                // wwzzyyxx, wwzzyyxx, wwzzyyxx, wwzzyyxx
                auto* u32s = b.Call<vec4<u32>>(load_u32());
                // xx000000, yyxx0000, zzyyxx00, wwzzyyxx
                auto* shl = b.Shl(u32s, b.Call<vec4<u32>>(24_u, 16_u, 8_u, 0_u));
                // 000000xx, 000000yy, 000000zz, 000000ww
                return b.Shr(shl, b.Call<vec4<u32>>(24_u));
            }
            case VertexFormat::kUint16: {
                return load_u16_l();
            }
            case VertexFormat::kUint16x2: {
                // yyyyxxxx, yyyyxxxx
                auto* u32s = b.Call<vec2<u32>>(load_u32());
                // xxxx0000, yyyyxxxx
                auto* shl = b.Shl(u32s, b.Call<vec2<u32>>(16_u, 0_u));
                // 0000xxxx, 0000yyyy
                return b.Shr(shl, b.Call<vec2<u32>>(16_u));
            }
            case VertexFormat::kUint16x4: {
                // yyyyxxxx, wwwwzzzz
                auto* u32s = b.Call<vec2<u32>>(load_u32(), load_next_u32());
                // yyyyxxxx, yyyyxxxx, wwwwzzzz, wwwwzzzz
                auto* xxyy = b.MemberAccessor(u32s, "xxyy");
                // xxxx0000, yyyyxxxx, zzzz0000, wwwwzzzz
                auto* shl = b.Shl(xxyy, b.Call<vec4<u32>>(16_u, 0_u, 16_u, 0_u));
                // 0000xxxx, 0000yyyy, 0000zzzz, 0000wwww
                return b.Shr(shl, b.Call<vec4<u32>>(16_u));
            }
            case VertexFormat::kSint8: {
                return load_i8_l();
            }
            case VertexFormat::kSint8x2: {
                // yyxx0000, yyxx0000
                auto* i16s = b.Call<vec2<i32>>(load_i16_h());
                // xx000000, yyxx0000
                auto* shl = b.Shl(i16s, b.Call<vec2<u32>>(8_u, 0_u));
                // ssssssxx, ssssssyy
                return b.Shr(shl, b.Call<vec2<u32>>(24_u));
            }
            case VertexFormat::kSint8x4: {
                // wwzzyyxx, wwzzyyxx, wwzzyyxx, wwzzyyxx
                auto* i32s = b.Call<vec4<i32>>(load_i32());
                // xx000000, yyxx0000, zzyyxx00, wwzzyyxx
                auto* shl = b.Shl(i32s, b.Call<vec4<u32>>(24_u, 16_u, 8_u, 0_u));
                // ssssssxx, ssssssyy, sssssszz, ssssssww
                return b.Shr(shl, b.Call<vec4<u32>>(24_u));
            }
            case VertexFormat::kSint16: {
                return b.Shr(load_i16_h(), 16_u);
            }
            case VertexFormat::kSint16x2: {
                // yyyyxxxx, yyyyxxxx
                auto* i32s = b.Call<vec2<i32>>(load_i32());
                // xxxx0000, yyyyxxxx
                auto* shl = b.Shl(i32s, b.Call<vec2<u32>>(16_u, 0_u));
                // ssssxxxx, ssssyyyy
                return b.Shr(shl, b.Call<vec2<u32>>(16_u));
            }
            case VertexFormat::kSint16x4: {
                // yyyyxxxx, wwwwzzzz
                auto* i32s = b.Call<vec2<i32>>(load_i32(), load_next_i32());
                // yyyyxxxx, yyyyxxxx, wwwwzzzz, wwwwzzzz
                auto* xxyy = b.MemberAccessor(i32s, "xxyy");
                // xxxx0000, yyyyxxxx, zzzz0000, wwwwzzzz
                auto* shl = b.Shl(xxyy, b.Call<vec4<u32>>(16_u, 0_u, 16_u, 0_u));
                // ssssxxxx, ssssyyyy, sssszzzz, sssswwww
                return b.Shr(shl, b.Call<vec4<u32>>(16_u));
            }
            case VertexFormat::kUnorm8:
                return b.MemberAccessor(b.Call("unpack4x8unorm", load_u8_l()), "x");
            case VertexFormat::kSnorm8:
                return b.MemberAccessor(b.Call("unpack4x8snorm", load_u8_l()), "x");
            case VertexFormat::kUnorm8x2:
                return b.MemberAccessor(b.Call("unpack4x8unorm", load_u16_l()), "xy");
            case VertexFormat::kSnorm8x2:
                return b.MemberAccessor(b.Call("unpack4x8snorm", load_u16_l()), "xy");
            case VertexFormat::kUnorm8x4:
                return b.Call("unpack4x8unorm", load_u32());
            case VertexFormat::kSnorm8x4:
                return b.Call("unpack4x8snorm", load_u32());
            case VertexFormat::kUnorm16:
                return b.MemberAccessor(b.Call("unpack2x16unorm", load_u16_l()), "x");
            case VertexFormat::kSnorm16:
                return b.MemberAccessor(b.Call("unpack2x16snorm", load_u16_l()), "x");
            case VertexFormat::kFloat16:
                return b.MemberAccessor(b.Call("unpack2x16float", load_u16_l()), "x");
            case VertexFormat::kUnorm16x2:
                return b.Call("unpack2x16unorm", load_u32());
            case VertexFormat::kSnorm16x2:
                return b.Call("unpack2x16snorm", load_u32());
            case VertexFormat::kFloat16x2:
                return b.Call("unpack2x16float", load_u32());
            case VertexFormat::kUnorm16x4:
                return b.Call<vec4<f32>>(b.Call("unpack2x16unorm", load_u32()),
                                         b.Call("unpack2x16unorm", load_next_u32()));
            case VertexFormat::kSnorm16x4:
                return b.Call<vec4<f32>>(b.Call("unpack2x16snorm", load_u32()),
                                         b.Call("unpack2x16snorm", load_next_u32()));
            case VertexFormat::kFloat16x4:
                return b.Call<vec4<f32>>(b.Call("unpack2x16float", load_u32()),
                                         b.Call("unpack2x16float", load_next_u32()));
            case VertexFormat::kUnorm10_10_10_2: {
                auto* u32s = b.Call<vec4<u32>>(load_u32());
                // shr = u32s >> vec4u(0, 10, 20, 30);
                auto* shr = b.Shr(u32s, b.Call<vec4<u32>>(0_u, 10_u, 20_u, 30_u));
                // mask = shr & vec4u(0x3FF, 0x3FF, 0x3FF, 0x3);
                auto* mask = b.And(shr, b.Call<vec4<u32>>(0x3FF_u, 0x3FF_u, 0x3FF_u, 0x3_u));
                // return vec4f(mask) / vec4f(1023, 1023, 1023, 3);
                return b.Div(b.Call<vec4<f32>>(mask),
                             b.Call<vec4<f32>>(1023_f, 1023_f, 1023_f, 3_f));
            }
            case VertexFormat::kUnorm8x4BGRA: {
                return b.MemberAccessor(b.Call("unpack4x8unorm", load_u32()), "zyxw");
            }
        }

        TINT_UNREACHABLE() << "format " << static_cast<int>(format);
    }

    /// Generates an expression reading an aligned basic type (u32, i32, f32) from
    /// a vertex buffer.
    /// @param array_base the symbol of the variable holding the base array offset
    /// of the vertex array (each index is 4-bytes).
    /// @param offset the byte offset of the data from `buffer_base`
    /// @param buffer the index of the vertex buffer
    /// @param format VertexFormat::kUint32, VertexFormat::kSint32 or
    /// VertexFormat::kFloat32
    const Expression* LoadPrimitive(Symbol array_base,
                                    uint32_t offset,
                                    uint32_t buffer,
                                    VertexFormat format) {
        const Expression* u = nullptr;
        if ((offset & 3) == 0) {
            // Aligned load.

            const ast ::Expression* index = nullptr;
            if (offset > 0) {
                index = b.Add(array_base, u32(offset / 4));
            } else {
                index = b.Expr(array_base);
            }
            u = b.IndexAccessor(
                b.MemberAccessor(GetVertexBufferName(buffer), GetStructBufferName()), index);

        } else {
            // Unaligned load
            uint32_t offset_aligned = offset & ~3u;
            auto* low = LoadPrimitive(array_base, offset_aligned, buffer, VertexFormat::kUint32);
            auto* high =
                LoadPrimitive(array_base, offset_aligned + 4u, buffer, VertexFormat::kUint32);

            uint32_t shift = 8u * (offset & 3u);

            auto* low_shr = b.Shr(low, u32(shift));
            auto* high_shl = b.Shl(high, u32(32u - shift));
            u = b.Or(low_shr, high_shl);
        }

        switch (format) {
            case VertexFormat::kUint32:
                return u;
            case VertexFormat::kSint32:
                return b.Bitcast(b.ty.i32(), u);
            case VertexFormat::kFloat32:
                return b.Bitcast(b.ty.f32(), u);
            default:
                break;
        }
        TINT_UNREACHABLE() << "invalid format for LoadPrimitive" << static_cast<int>(format);
    }

    /// Generates an expression reading a vec2/3/4 from a vertex buffer.
    /// @param array_base the symbol of the variable holding the base array offset
    /// of the vertex array (each index is 4-bytes).
    /// @param offset the byte offset of the data from `buffer_base`
    /// @param buffer the index of the vertex buffer
    /// @param element_stride stride between elements, in bytes
    /// @param base_type underlying AST type
    /// @param base_format underlying vertex format
    /// @param count how many elements the vector has
    const Expression* LoadVec(Symbol array_base,
                              uint32_t offset,
                              uint32_t buffer,
                              uint32_t element_stride,
                              Type base_type,
                              VertexFormat base_format,
                              uint32_t count) {
        tint::Vector<const Expression*, 8> expr_list;
        for (uint32_t i = 0; i < count; ++i) {
            // Offset read position by element_stride for each component
            uint32_t primitive_offset = offset + element_stride * i;
            expr_list.Push(LoadPrimitive(array_base, primitive_offset, buffer, base_format));
        }

        return b.Call(b.ty.vec(base_type, count), std::move(expr_list));
    }

    /// Process a non-struct entry point parameter.
    /// Generate function-scope variables for location parameters, and record
    /// vertex_index and instance_index builtins if present.
    /// @param func the entry point function
    /// @param param the parameter to process
    void ProcessNonStructParameter(const Function* func, const Parameter* param) {
        if (HasAttribute<LocationAttribute>(param->attributes)) {
            // Create a function-scope variable to replace the parameter.
            auto func_var_sym = ctx.Clone(param->name->symbol);
            auto func_var_type = ctx.Clone(param->type);
            auto* func_var = b.Var(func_var_sym, func_var_type);
            ctx.InsertFront(func->body->statements, b.Decl(func_var));
            // Capture mapping from location to the new variable.
            LocationInfo info;
            info.expr = [this, func_var] { return b.Expr(func_var); };

            auto* sem = src.Sem().Get(param);
            info.type = sem->Type();

            if (DAWN_UNLIKELY(!sem->Attributes().location.has_value())) {
                TINT_ICE() << "Location missing value";
            }
            location_info[sem->Attributes().location.value()] = info;
        } else {
            auto* builtin_attr = GetAttribute<BuiltinAttribute>(param->attributes);
            if (DAWN_UNLIKELY(!builtin_attr)) {
                TINT_ICE() << "Invalid entry point parameter";
            }
            auto builtin = builtin_attr->builtin;
            // Check for existing vertex_index and instance_index builtins.
            if (builtin == core::BuiltinValue::kVertexIndex) {
                vertex_index_expr = [this, param] {
                    return b.Expr(ctx.Clone(param->name->symbol));
                };
            } else if (builtin == core::BuiltinValue::kInstanceIndex) {
                instance_index_expr = [this, param] {
                    return b.Expr(ctx.Clone(param->name->symbol));
                };
            }
            new_function_parameters.Push(ctx.Clone(param));
        }
    }

    /// Process a struct entry point parameter.
    /// If the struct has members with location attributes, push the parameter to
    /// a function-scope variable and create a new struct parameter without those
    /// attributes. Record expressions for members that are vertex_index and
    /// instance_index builtins.
    /// @param func the entry point function
    /// @param param the parameter to process
    /// @param struct_ty the structure type
    void ProcessStructParameter(const Function* func,
                                const Parameter* param,
                                const Struct* struct_ty) {
        auto param_sym = ctx.Clone(param->name->symbol);

        // Process the struct members.
        bool has_locations = false;
        tint::Vector<const StructMember*, 8> members_to_clone;
        for (auto* member : struct_ty->members) {
            auto member_sym = ctx.Clone(member->name->symbol);
            std::function<const Expression*()> member_expr = [this, param_sym, member_sym] {
                return b.MemberAccessor(param_sym, member_sym);
            };

            if (HasAttribute<LocationAttribute>(member->attributes)) {
                // Capture mapping from location to struct member.
                LocationInfo info;
                info.expr = member_expr;

                auto* sem = src.Sem().Get(member);
                info.type = sem->Type();

                TINT_ASSERT(sem->Attributes().location.has_value());
                location_info[sem->Attributes().location.value()] = info;
                has_locations = true;
            } else {
                auto* builtin_attr = GetAttribute<BuiltinAttribute>(member->attributes);
                if (DAWN_UNLIKELY(!builtin_attr)) {
                    TINT_ICE() << "Invalid entry point parameter";
                }
                auto builtin = builtin_attr->builtin;
                // Check for existing vertex_index and instance_index builtins.
                if (builtin == core::BuiltinValue::kVertexIndex) {
                    vertex_index_expr = member_expr;
                } else if (builtin == core::BuiltinValue::kInstanceIndex) {
                    instance_index_expr = member_expr;
                }
                members_to_clone.Push(member);
            }
        }

        if (!has_locations) {
            // Nothing to do.
            new_function_parameters.Push(ctx.Clone(param));
            return;
        }

        // Create a function-scope variable to replace the parameter.
        auto* func_var = b.Var(param_sym, ctx.Clone(param->type));
        ctx.InsertFront(func->body->statements, b.Decl(func_var));

        if (!members_to_clone.IsEmpty()) {
            // Create a new struct without the location attributes.
            tint::Vector<const StructMember*, 8> new_members;
            for (auto* member : members_to_clone) {
                auto member_name = ctx.Clone(member->name);
                auto member_type = ctx.Clone(member->type);
                auto member_attrs = ctx.Clone(member->attributes);
                new_members.Push(b.Member(member_name, member_type, std::move(member_attrs)));
            }
            auto* new_struct = b.Structure(b.Sym(), new_members);

            // Create a new function parameter with this struct.
            auto* new_param = b.Param(b.Sym(), b.ty.Of(new_struct));
            new_function_parameters.Push(new_param);

            // Copy values from the new parameter to the function-scope variable.
            for (auto* member : members_to_clone) {
                auto member_name = ctx.Clone(member->name->symbol);
                ctx.InsertFront(func->body->statements,
                                b.Assign(b.MemberAccessor(func_var, member_name),
                                         b.MemberAccessor(new_param, member_name)));
            }
        }
    }

    /// Process an entry point function.
    /// @param func the entry point function
    void Process(const Function* func) {
        if (func->body->Empty()) {
            return;
        }

        // Process entry point parameters.
        for (auto* param : func->params) {
            auto* sem = src.Sem().Get(param);
            if (auto* str = sem->Type()->As<sem::Struct>()) {
                ProcessStructParameter(func, param, str->Declaration());
            } else {
                ProcessNonStructParameter(func, param);
            }
        }

        // Insert new parameters for vertex_index and instance_index if needed.
        if (!vertex_index_expr) {
            for (const VertexBufferLayoutDescriptor& layout : cfg.vertex_state) {
                if (layout.step_mode == VertexStepMode::kVertex) {
                    auto name = b.Symbols().New("tint_pulling_vertex_index");
                    new_function_parameters.Push(
                        b.Param(name, b.ty.u32(),
                                tint::Vector{b.Builtin(core::BuiltinValue::kVertexIndex)}));
                    vertex_index_expr = [this, name] { return b.Expr(name); };
                    break;
                }
            }
        }
        if (!instance_index_expr) {
            for (const VertexBufferLayoutDescriptor& layout : cfg.vertex_state) {
                if (layout.step_mode == VertexStepMode::kInstance) {
                    auto name = b.Symbols().New("tint_pulling_instance_index");
                    new_function_parameters.Push(
                        b.Param(name, b.ty.u32(),
                                tint::Vector{b.Builtin(core::BuiltinValue::kInstanceIndex)}));
                    instance_index_expr = [this, name] { return b.Expr(name); };
                    break;
                }
            }
        }

        // Generate vertex pulling preamble.
        if (auto* block = CreateVertexPullingPreamble()) {
            ctx.InsertFront(func->body->statements, block);
        }

        // Rewrite the function header with the new parameters.
        auto func_sym = ctx.Clone(func->name->symbol);
        auto ret_type = ctx.Clone(func->return_type);
        auto* body = ctx.Clone(func->body);
        auto attrs = ctx.Clone(func->attributes);
        auto ret_attrs = ctx.Clone(func->return_type_attributes);
        auto* new_func =
            b.create<Function>(func->source, b.Ident(func_sym), new_function_parameters, ret_type,
                               body, std::move(attrs), std::move(ret_attrs));
        ctx.Replace(func, new_func);
    }
};

VertexPulling::VertexPulling() = default;
VertexPulling::~VertexPulling() = default;

Transform::ApplyResult VertexPulling::Apply(const Program& src,
                                            const DataMap& inputs,
                                            DataMap&) const {
    auto cfg = cfg_;
    if (auto* cfg_data = inputs.Get<Config>()) {
        cfg = *cfg_data;
    }

    return State{src, cfg}.Run();
}

VertexPulling::Config::Config() = default;
VertexPulling::Config::Config(const Config&) = default;
VertexPulling::Config::~Config() = default;
VertexPulling::Config& VertexPulling::Config::operator=(const Config&) = default;

}  // namespace tint::ast::transform
