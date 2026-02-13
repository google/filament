// Copyright 2025 The Dawn & Tint Authors
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

#include "src/tint/lang/core/ir/transform/vertex_pulling.h"

#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/manager.h"

namespace tint::core::ir::transform {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

/// @returns the vector width of @p format
uint32_t FormatWidth(VertexFormat format) {
    switch (format) {
        case VertexFormat::kUint8:
        case VertexFormat::kSint8:
        case VertexFormat::kUnorm8:
        case VertexFormat::kSnorm8:
        case VertexFormat::kUint16:
        case VertexFormat::kSint16:
        case VertexFormat::kUnorm16:
        case VertexFormat::kSnorm16:
        case VertexFormat::kFloat16:
        case VertexFormat::kFloat32:
        case VertexFormat::kUint32:
        case VertexFormat::kSint32:
            return 1;
        case VertexFormat::kUint8x2:
        case VertexFormat::kSint8x2:
        case VertexFormat::kUnorm8x2:
        case VertexFormat::kSnorm8x2:
        case VertexFormat::kUint16x2:
        case VertexFormat::kSint16x2:
        case VertexFormat::kUnorm16x2:
        case VertexFormat::kSnorm16x2:
        case VertexFormat::kFloat16x2:
        case VertexFormat::kFloat32x2:
        case VertexFormat::kUint32x2:
        case VertexFormat::kSint32x2:
            return 2;
        case VertexFormat::kFloat32x3:
        case VertexFormat::kUint32x3:
        case VertexFormat::kSint32x3:
            return 3;
        case VertexFormat::kUint8x4:
        case VertexFormat::kSint8x4:
        case VertexFormat::kUnorm8x4:
        case VertexFormat::kSnorm8x4:
        case VertexFormat::kUint16x4:
        case VertexFormat::kSint16x4:
        case VertexFormat::kUnorm16x4:
        case VertexFormat::kSnorm16x4:
        case VertexFormat::kFloat16x4:
        case VertexFormat::kFloat32x4:
        case VertexFormat::kUint32x4:
        case VertexFormat::kSint32x4:
        case VertexFormat::kUnorm10_10_10_2:
        case VertexFormat::kUnorm8x4BGRA:
            return 4;
    }
    TINT_UNREACHABLE();
}

/// PIMPL state for the transform.
struct State {
    /// The vertex pulling configuration.
    const VertexPullingConfig& config;

    /// The IR module.
    core::ir::Module& ir;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// A map from location to a descriptor that holds the format, buffer, the offset of the
    /// vertex data from the start of the buffer, and the offset of the attribute from the start of
    /// the element.
    struct LocationInfo {
        VertexFormat format;
        core::ir::Value* buffer = nullptr;
        // Word offset within the buffer to the start of the data for this invocation.
        core::ir::Value* base_offset = nullptr;
        // Byte offset within the data for this invocation to the start of the attribute data.
        uint32_t attr_byte_offset;
    };
    Hashmap<uint32_t, LocationInfo, 4> locations_{};

    /// The vertex index function parameter.
    core::ir::FunctionParam* vertex_index_ = nullptr;
    /// The instance index function parameter.
    core::ir::FunctionParam* instance_index_ = nullptr;

    /// Process the module.
    void Process() {
        // Find the vertex shader entry point. There should be at most one.
        core::ir::Function* ep = nullptr;
        for (auto& func : ir.functions) {
            if (func->IsVertex()) {
                TINT_ASSERT(!ep);
                ep = func;
            }
        }
        if (!ep) {
            return;
        }

        Vector<core::ir::FunctionParam*, 4> new_params;
        b.InsertBefore(ep->Block()->Front(), [&] {  //
            // Create the storage buffers and record vertex attribute information.
            CreateBuffers();

            // Pull vertex attributes out of the entry point parameters and replace them.
            for (auto* param : ep->Params()) {
                if (auto* str = param->Type()->As<core::type::Struct>()) {
                    ProcessStructParameter(param, str);
                    param->Destroy();
                } else if (auto loc = param->Attributes().location) {
                    // Load the vertex attribute and replace uses of the parameter with it.
                    auto* input = Load(*loc, param->Type());
                    param->ReplaceAllUsesWith(input);
                    param->Destroy();
                } else {
                    // Other parameters should be builtins, which can only be the vertex and
                    // instance indices. Replace any user-declared indices with the ones that we
                    // created when setting up the buffers.
                    auto builtin = param->Builtin();
                    TINT_ASSERT(builtin);
                    switch (*builtin) {
                        case core::BuiltinValue::kVertexIndex:
                            if (vertex_index_) {
                                param->ReplaceAllUsesWith(vertex_index_);
                                param->Destroy();
                            } else {
                                new_params.Push(param);
                            }
                            break;
                        case core::BuiltinValue::kInstanceIndex:
                            if (instance_index_) {
                                param->ReplaceAllUsesWith(instance_index_);
                                param->Destroy();
                            } else {
                                new_params.Push(param);
                            }
                            break;
                        default:
                            TINT_UNREACHABLE();
                    }
                }
            }
        });

        // Update the entry point with the new parameter list.
        if (vertex_index_) {
            new_params.Push(vertex_index_);
        }
        if (instance_index_) {
            new_params.Push(instance_index_);
        }
        ep->SetParams(std::move(new_params));
    }

    /// @returns the vertex index parameter, creating one if needed
    core::ir::FunctionParam* GetVertexIndex() {
        if (!vertex_index_) {
            vertex_index_ = b.FunctionParam<u32>("tint_vertex_index");
            vertex_index_->SetBuiltin(core::BuiltinValue::kVertexIndex);
        }
        return vertex_index_;
    }

    /// @returns the instance index parameter, creating one if needed
    core::ir::FunctionParam* GetInstanceIndex() {
        if (!instance_index_) {
            instance_index_ = b.FunctionParam<u32>("tint_instance_index");
            instance_index_->SetBuiltin(core::BuiltinValue::kInstanceIndex);
        }
        return instance_index_;
    }

    /// Create storage buffers and record vertex attribute information.
    /// Record instructions that need vertex and instance indices.
    void CreateBuffers() {
        for (uint32_t i = 0; i < config.vertex_state.size(); i++) {
            // Create the storage buffer.
            auto& buffer = config.vertex_state[i];
            auto buffer_name = "tint_vertex_buffer_" + std::to_string(i);
            core::ir::Var* var = nullptr;
            b.Append(ir.root_block, [&] {
                var = b.Var(buffer_name, ty.ptr<storage, array<u32>, read>());
                var->SetBindingPoint(config.pulling_group, i);
            });

            // Determine the base offset of the vertex data in the storage buffer.
            core::ir::Value* index = nullptr;
            if (buffer.step_mode == VertexStepMode::kVertex) {
                index = GetVertexIndex();
            } else if (buffer.step_mode == VertexStepMode::kInstance) {
                index = GetInstanceIndex();
            }
            if (buffer.array_stride != 4) {
                // Multiply the index by the stride in words.
                TINT_ASSERT((buffer.array_stride & 3u) == 0u);
                index = b.Multiply<u32>(index, u32(buffer.array_stride / 4))->Result();
                ir.SetName(index, buffer_name + "_base");
            }

            // Register the format, buffer, and offset for each location slot.
            for (auto& attr : buffer.attributes) {
                locations_.Add(attr.shader_location,
                               LocationInfo{attr.format, var->Result(), index, attr.offset});
            }
        }
    }

    /// Pull vertex attributes out of structure parameter.
    /// Replace the parameter with a new structure created from the attributes.
    /// @param param the parameter
    /// @param str the structure type
    void ProcessStructParameter(core::ir::FunctionParam* param, const core::type::Struct* str) {
        Vector<core::ir::Value*, 4> construct_args;
        for (auto* member : str->Members()) {
            if (auto loc = member->Attributes().location) {
                construct_args.Push(Load(*loc, member->Type()));
            } else {
                // Other parameters should be builtins, which can only be the vertex and instance
                // indices. Use the separate parameters that we created for these indices. Because
                // there are no duplicates, this doesn't conflict with the param handling above.
                TINT_ASSERT(member->Attributes().builtin);
                switch (*member->Attributes().builtin) {
                    case core::BuiltinValue::kVertexIndex:
                        construct_args.Push(GetVertexIndex());
                        break;
                    case core::BuiltinValue::kInstanceIndex:
                        construct_args.Push(GetInstanceIndex());
                        break;
                    default:
                        TINT_UNREACHABLE();
                }
            }
        }
        param->ReplaceAllUsesWith(b.Construct(str, std::move(construct_args))->Result());
    }

    /// Load a vertex attribute.
    /// @param location the location index of the attribute
    /// @param shader_type the type of the attribute declared in the shader
    /// @returns the loaded attribute value
    core::ir::Value* Load(uint32_t location, const core::type::Type* shader_type) {
        auto info = locations_.Get(location);
        TINT_ASSERT(info);

        // Load the attribute data from the buffer.
        auto* value = LoadFromBuffer(*info, shader_type->DeepestElement());

        // The shader type may have a different component width to the vertex attribute, so we may
        // need to add or remove components.
        auto src_width = FormatWidth(info->format);
        auto dst_width = 1u;
        if (auto* vec = shader_type->As<core::type::Vector>()) {
            dst_width = vec->Width();
        }
        if (dst_width < src_width) {
            // The type declared in the shader is narrower than the vertex attribute format, so
            // truncate the value with a swizzle.
            switch (dst_width) {
                case 1:
                    value = b.Swizzle(shader_type, value, Vector{0u})->Result();
                    break;
                case 2:
                    value = b.Swizzle(shader_type, value, Vector{0u, 1u})->Result();
                    break;
                case 3:
                    value = b.Swizzle(shader_type, value, Vector{0u, 1u, 2u})->Result();
                    break;
                default:
                    TINT_UNREACHABLE() << dst_width;
            }
        } else if (dst_width > src_width) {
            // The type declared in the shader is wider than the vertex attribute format, so append
            // values to pad it out. Append a `1` value for the fourth element of a vector,
            // otherwise append zero.
            auto* elem_ty = shader_type->DeepestElement();
            auto one = [&] {
                return tint::Switch(
                    elem_ty,                                                  //
                    [&](const core::type::I32*) { return b.Constant(1_i); },  //
                    [&](const core::type::U32*) { return b.Constant(1_u); },  //
                    [&](const core::type::F32*) { return b.Constant(1_f); },  //
                    [&](const core::type::F16*) { return b.Constant(1_h); },  //
                    TINT_ICE_ON_NO_MATCH);
            };
            Vector<core::ir::Value*, 4> values{value};
            for (uint32_t i = src_width; i < dst_width; i++) {
                values.Push(i == 3 ? one() : b.Zero(elem_ty));
            }
            value = b.Construct(shader_type, std::move(values))->Result();
        }

        return value;
    }

    /// Load attribute data from a buffer.
    /// @param info the location descriptor
    /// @param shader_element_type the element type of the attribute declared in the shader
    /// @returns the loaded attribute data
    core::ir::Value* LoadFromBuffer(const LocationInfo& info,
                                    const core::type::Type* shader_element_type) {
        // Helper for loading a single word from the buffer at an offset.
        auto load_u32 = [&](uint32_t offset) {
            auto offset_value = info.base_offset;
            offset += (info.attr_byte_offset / 4u);
            if (offset > 0) {
                offset_value = b.Add<u32>(offset_value, u32(offset))->Result();
            }
            auto* word =
                b.Load(b.Access<ptr<storage, u32, read>>(info.buffer, offset_value))->Result();
            // If the offset is not 4-byte aligned, shift the word so that the requested data starts
            // at the first byte. The shift amount is the offset of the byte within a word
            // multiplied by 8 to get the bit offset.
            if (info.attr_byte_offset & 3) {
                word = b.ShiftRight<u32>(word, u32((info.attr_byte_offset & 3) * 8))->Result();
            }
            return word;
        };
        // Helpers for loading non-u32 data from the buffer.
        auto load_i32 = [&](uint32_t offset) { return b.Bitcast<i32>(load_u32(offset))->Result(); };
        auto load_f32 = [&](uint32_t offset) { return b.Bitcast<f32>(load_u32(offset))->Result(); };
        auto load_ivec = [&](uint32_t offset, uint32_t bits, const core::type::Vector* vec) {
            // For a vec2<u32>, we read the `xxxx'yyyy` u32 word. We then splat to a vec2 and left
            // shift so we have `(xxxx'yyyy, yyyy'xxxx)`. Finally, we right shift to produce
            // `(0000'xxxx, 0000'yyyy)`
            auto* uvec = ty.MatchWidth(ty.u32(), vec);
            // yyyyxxxx
            auto* word = load_u32(offset);
            if (vec->Type()->Is<core::type::I32>()) {
                word = b.Bitcast<i32>(word)->Result();
            }
            // yyyyxxxx, yyyyxxxx
            auto* splat = b.Construct(vec, word);
            // xxxxyyyy, yyyyxxxx
            core::ir::Instruction* shift_left = nullptr;
            switch (vec->Width()) {
                case 2:
                    if (bits == 8) {
                        shift_left = b.ShiftLeft(vec, splat, b.Composite(uvec, 24_u, 16_u));
                    } else if (bits == 16) {
                        shift_left = b.ShiftLeft(vec, splat, b.Composite(uvec, 16_u, 0_u));
                    } else {
                        TINT_UNREACHABLE();
                    }
                    break;
                case 4:
                    TINT_ASSERT(bits == 8);
                    shift_left = b.ShiftLeft(vec, splat, b.Composite(uvec, 24_u, 16_u, 8_u, 0_u));
                    break;
                default:
                    TINT_UNREACHABLE();
            }
            // 0000xxxx, 0000yyyy
            return b.ShiftRight(vec, shift_left, b.Splat(uvec, u32(32 - bits)))->Result();
        };
        // Helper to convert a value to f16 if required by the shader, otherwise returns the f32.
        auto float_value = [&](core::ir::Value* value) -> core::ir::Value* {
            // If the shader expects an f16 value, convert the value.
            if (shader_element_type->Is<core::type::F16>()) {
                return b.Convert(ty.MatchWidth(ty.f16(), value->Type()), value)->Result();
            }
            return value;
        };

        switch (info.format) {
            // Formats that are always u32 in the shader (or vectors of u32).
            // Shift/mask values to expand to 32-bits.
            case VertexFormat::kUint8:
                return b.And<u32>(load_u32(0), 0xFF_u)->Result();
            case VertexFormat::kUint8x2:
                return load_ivec(0, 8, ty.vec2<u32>());
            case VertexFormat::kUint8x4:
                return load_ivec(0, 8, ty.vec4<u32>());
            case VertexFormat::kUint16:
                return b.And<u32>(load_u32(0), 0xFFFF_u)->Result();
            case VertexFormat::kUint16x2:
                return load_ivec(0, 16, ty.vec2<u32>());
            case VertexFormat::kUint16x4: {
                auto* xy = load_ivec(0, 16, ty.vec2<u32>());
                auto* zw = load_ivec(1, 16, ty.vec2<u32>());
                return b.Construct<vec4<u32>>(xy, zw)->Result();
            }
            case VertexFormat::kUint32:
                return load_u32(0);
            case VertexFormat::kUint32x2: {
                auto* x = load_u32(0);
                auto* y = load_u32(1);
                return b.Construct<vec2<u32>>(x, y)->Result();
            }
            case VertexFormat::kUint32x3: {
                auto* x = load_u32(0);
                auto* y = load_u32(1);
                auto* z = load_u32(2);
                return b.Construct<vec3<u32>>(x, y, z)->Result();
            }
            case VertexFormat::kUint32x4: {
                auto* x = load_u32(0);
                auto* y = load_u32(1);
                auto* z = load_u32(2);
                auto* w = load_u32(3);
                return b.Construct<vec4<u32>>(x, y, z, w)->Result();
            }

            // Formats that are always i32 in the shader (or vectors of i32).
            // Shift values to expand to 32-bits.
            case VertexFormat::kSint8: {
                // ******xx
                auto* word = b.Bitcast<i32>(load_u32(0));
                // 000000xx
                return b.ShiftRight<i32>(b.ShiftLeft<i32>(word, 24_u), 24_u)->Result();
            }
            case VertexFormat::kSint8x2:
                return load_ivec(0, 8, ty.vec2<i32>());
            case VertexFormat::kSint8x4:
                return load_ivec(0, 8, ty.vec4<i32>());
            case VertexFormat::kSint16: {
                // ****xxxx
                auto* word = b.Bitcast<i32>(load_u32(0));
                // 0000xxxx
                return b.ShiftRight<i32>(b.ShiftLeft<i32>(word, 16_u), 16_u)->Result();
            }
            case VertexFormat::kSint16x2:
                return load_ivec(0, 16, ty.vec2<i32>());
            case VertexFormat::kSint16x4: {
                auto* xy = load_ivec(0, 16, ty.vec2<i32>());
                auto* zw = load_ivec(1, 16, ty.vec2<i32>());
                return b.Construct<vec4<i32>>(xy, zw)->Result();
            }
            case VertexFormat::kSint32:
                return load_i32(0);
            case VertexFormat::kSint32x2: {
                auto* x = load_i32(0);
                auto* y = load_i32(1);
                return b.Construct<vec2<i32>>(x, y)->Result();
            }
            case VertexFormat::kSint32x3: {
                auto* x = load_i32(0);
                auto* y = load_i32(1);
                auto* z = load_i32(2);
                return b.Construct<vec3<i32>>(x, y, z)->Result();
            }
            case VertexFormat::kSint32x4: {
                auto* x = load_i32(0);
                auto* y = load_i32(1);
                auto* z = load_i32(2);
                auto* w = load_i32(3);
                return b.Construct<vec4<i32>>(x, y, z, w)->Result();
            }

            // Unsigned normalized formats.
            // Use unpack builtins to convert to f32.
            case VertexFormat::kUnorm8: {
                // ******xx
                auto* word = load_u32(0);
                // 000000xx, ********, ********, ********
                auto* unpack = b.Call<vec4<f32>>(core::BuiltinFn::kUnpack4X8Unorm, word);
                // 000000xx
                return float_value(b.Access<f32>(unpack, 0_u)->Result());
            }
            case VertexFormat::kUnorm8x2: {
                // ****yyxx
                auto* word = load_u32(0);
                // 000000xx, 000000yy, ********, ********
                auto* unpack = b.Call<vec4<f32>>(core::BuiltinFn::kUnpack4X8Unorm, word);
                // 000000xx, 000000yy
                return float_value(b.Swizzle<vec2<f32>>(unpack, Vector{0u, 1u})->Result());
            }
            case VertexFormat::kUnorm8x4: {
                // wwzzyyxx
                auto* word = load_u32(0);
                // 000000xx, 000000yy, 000000zz, 000000ww
                auto* unpack = b.Call<vec4<f32>>(core::BuiltinFn::kUnpack4X8Unorm, word);
                return float_value(unpack->Result());
            }
            case VertexFormat::kUnorm8x4BGRA: {
                // wwzzyyxx
                auto* word = load_u32(0);
                // 000000xx, 000000yy, 000000zz, 000000ww
                auto* unpack = b.Call<vec4<f32>>(core::BuiltinFn::kUnpack4X8Unorm, word);
                // 000000zz, 000000yy, 000000xx, 000000ww
                return float_value(b.Swizzle<vec4<f32>>(unpack, Vector{2u, 1u, 0u, 3u})->Result());
            }
            case VertexFormat::kUnorm16: {
                // ****xxxx
                auto* word = load_u32(0);
                // 0000xxxx, ********
                auto* unpack = b.Call<vec2<f32>>(core::BuiltinFn::kUnpack2X16Unorm, word);
                // 0000xxxx
                return float_value(b.Access<f32>(unpack, 0_u)->Result());
            }
            case VertexFormat::kUnorm16x2: {
                // yyyyxxxx
                auto* word = load_u32(0);
                // 0000xxxx, 0000yyyy
                auto* unpack = b.Call<vec2<f32>>(core::BuiltinFn::kUnpack2X16Unorm, word);
                return float_value(unpack->Result());
            }
            case VertexFormat::kUnorm16x4: {
                // yyyyxxxx, wwwwzzzz
                auto* word0 = load_u32(0);
                auto* word1 = load_u32(1);
                // 0000xxxx, 0000yyyy, 0000zzzz, 0000wwww
                auto* unpack0 = b.Call<vec2<f32>>(core::BuiltinFn::kUnpack2X16Unorm, word0);
                auto* unpack1 = b.Call<vec2<f32>>(core::BuiltinFn::kUnpack2X16Unorm, word1);
                return float_value(b.Construct<vec4<f32>>(unpack0, unpack1)->Result());
            }

            // Signed normalized formats.
            // Use unpack builtins to expand to f32.
            case VertexFormat::kSnorm8: {
                // ******xx
                auto* word = load_u32(0);
                // 000000xx, ********, ********, ********
                auto* unpack = b.Call<vec4<f32>>(core::BuiltinFn::kUnpack4X8Snorm, word);
                // 000000xx
                return float_value(b.Access<f32>(unpack, 0_u)->Result());
            }
            case VertexFormat::kSnorm8x2: {
                // ****yyxx
                auto* word = load_u32(0);
                // 000000xx, 000000yy, ********, ********
                auto* unpack = b.Call<vec4<f32>>(core::BuiltinFn::kUnpack4X8Snorm, word);
                // 000000xx, 000000yy
                return float_value(b.Swizzle<vec2<f32>>(unpack, Vector{0u, 1u})->Result());
            }
            case VertexFormat::kSnorm8x4: {
                // wwzzyyxx
                auto* word = load_u32(0);
                // 000000xx, 000000yy, 000000zz, 000000ww
                auto* unpack = b.Call<vec4<f32>>(core::BuiltinFn::kUnpack4X8Snorm, word);
                return float_value(unpack->Result());
            }
            case VertexFormat::kSnorm16: {
                // ****xxxx
                auto* word = load_u32(0);
                // 0000xxxx, ********
                auto* unpack = b.Call<vec2<f32>>(core::BuiltinFn::kUnpack2X16Snorm, word);
                // 0000xxxx
                return float_value(b.Access<f32>(unpack, 0_u)->Result());
            }
            case VertexFormat::kSnorm16x2: {
                // yyyyxxxx
                auto* word = load_u32(0);
                // 0000xxxx, 0000yyyy
                auto* unpack = b.Call<vec2<f32>>(core::BuiltinFn::kUnpack2X16Snorm, word);
                return float_value(unpack->Result());
            }
            case VertexFormat::kSnorm16x4: {
                // yyyyxxxx, wwwwzzzz
                auto* word0 = load_u32(0);
                auto* word1 = load_u32(1);
                // 0000xxxx, 0000yyyy, 0000zzzz, 0000wwww
                auto* unpack0 = b.Call<vec2<f32>>(core::BuiltinFn::kUnpack2X16Snorm, word0);
                auto* unpack1 = b.Call<vec2<f32>>(core::BuiltinFn::kUnpack2X16Snorm, word1);
                return float_value(b.Construct<vec4<f32>>(unpack0, unpack1)->Result());
            }

            // F16 formats that can either be f16 or f32 in the shader.
            // If f16 is expected just bitcast, otherwise use unpack builtins to convert to f32.
            case VertexFormat::kFloat16: {
                // ****xxxx
                auto* word = load_u32(0);
                if (shader_element_type->Is<core::type::F16>()) {
                    // xxxx, ****
                    auto* bitcast = b.Bitcast<vec2<f16>>(word);
                    // xxxx
                    return b.Access<f16>(bitcast, 0_u)->Result();
                } else {
                    // 0000xxxx, ********
                    auto* unpack = b.Call<vec2<f32>>(core::BuiltinFn::kUnpack2X16Float, word);
                    // 0000xxxx
                    return b.Access<f32>(unpack, 0_u)->Result();
                }
            }
            case VertexFormat::kFloat16x2: {
                // yyyyxxxx
                auto* word = load_u32(0);
                if (shader_element_type->Is<core::type::F16>()) {
                    // xxxx, yyyy
                    return b.Bitcast<vec2<f16>>(word)->Result();
                } else {
                    // 0000xxxx, 0000yyyy
                    auto* unpack = b.Call<vec2<f32>>(core::BuiltinFn::kUnpack2X16Float, word);
                    return unpack->Result();
                }
            }
            case VertexFormat::kFloat16x4: {
                // yyyyxxxx, wwwwzzzz
                auto* word0 = load_u32(0);
                auto* word1 = load_u32(1);
                if (shader_element_type->Is<core::type::F16>()) {
                    // xxxx, yyyy, zzzz, wwww
                    auto* bitcast0 = b.Bitcast<vec2<f16>>(word0);
                    auto* bitcast1 = b.Bitcast<vec2<f16>>(word1);
                    return b.Construct<vec4<f16>>(bitcast0, bitcast1)->Result();
                } else {
                    // 0000xxxx, 0000yyyy, 0000zzzz, 0000wwww
                    auto* unpack0 = b.Call<vec2<f32>>(core::BuiltinFn::kUnpack2X16Float, word0);
                    auto* unpack1 = b.Call<vec2<f32>>(core::BuiltinFn::kUnpack2X16Float, word1);
                    return b.Construct<vec4<f32>>(unpack0, unpack1)->Result();
                }
            }

            // F32 formats that can either be f16 or f32 in the shader.
            // Load the f32 data and downconvert to f16 if needed.
            case VertexFormat::kFloat32:
                return float_value(load_f32(0));
            case VertexFormat::kFloat32x2: {
                auto* x = load_f32(0);
                auto* y = load_f32(1);
                return float_value(b.Construct<vec2<f32>>(x, y)->Result());
            }
            case VertexFormat::kFloat32x3: {
                auto* x = load_f32(0);
                auto* y = load_f32(1);
                auto* z = load_f32(2);
                return float_value(b.Construct<vec3<f32>>(x, y, z)->Result());
            }
            case VertexFormat::kFloat32x4: {
                auto* x = load_f32(0);
                auto* y = load_f32(1);
                auto* z = load_f32(2);
                auto* w = load_f32(3);
                return float_value(b.Construct<vec4<f32>>(x, y, z, w)->Result());
            }

            // Miscellaneous other formats that need custom handling.
            case VertexFormat::kUnorm10_10_10_2: {
                auto* u32s = b.Construct<vec4<u32>>(load_u32(0));
                // shr = u32s >> vec4u(0, 10, 20, 30);
                auto* shr =
                    b.ShiftRight<vec4<u32>>(u32s, b.Composite<vec4<u32>>(0_u, 10_u, 20_u, 30_u));
                // mask = shr & vec4u(0x3FF, 0x3FF, 0x3FF, 0x3);
                auto* mask =
                    b.And<vec4<u32>>(shr, b.Composite<vec4<u32>>(0x3FF_u, 0x3FF_u, 0x3FF_u, 0x3_u));
                // vec4f(mask) / vec4f(1023, 1023, 1023, 3);
                auto* div = b.Composite<vec4<f32>>(1023_f, 1023_f, 1023_f, 3_f);
                return float_value(b.Divide<vec4<f32>>(b.Convert<vec4<f32>>(mask), div)->Result());
            }
        }
        TINT_UNREACHABLE();
    }
};

}  // namespace

Result<SuccessType> VertexPulling(core::ir::Module& ir, const VertexPullingConfig& config) {
    auto result = ValidateAndDumpIfNeeded(ir, "core.VertexPulling", kVertexPullingCapabilities);
    if (result != Success) {
        return result.Failure();
    }

    State{config, ir}.Process();

    return Success;
}

}  // namespace tint::core::ir::transform
