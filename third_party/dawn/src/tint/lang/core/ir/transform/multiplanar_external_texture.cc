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

#include "src/tint/lang/core/ir/transform/multiplanar_external_texture.h"

#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/external_texture.h"
#include "src/tint/lang/core/type/sampled_texture.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::core::ir::transform {

namespace {

/// PIMPL state for the transform.
struct State {
    /// The external texture options.
    const tint::transform::multiplanar::BindingsMap& multiplanar_map;

    /// The IR module.
    Module& ir;

    /// The IR builder.
    Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// The symbol table.
    SymbolTable& sym{ir.symbols};

    /// The gamma transfer parameters structure.
    const core::type::Struct* gamma_transfer_params_struct = nullptr;

    /// The external texture parameters structure.
    const core::type::Struct* external_texture_params_struct = nullptr;

    /// The helper function that implements `textureLoad()`.
    Function* texture_load_external = nullptr;

    /// The helper function that implements `textureSampleBaseClampToEdge()`.
    Function* texture_sample_external = nullptr;

    /// The gamma correction helper function.
    Function* gamma_correction = nullptr;

    /// Process the module.
    Result<SuccessType> Process() {
        // Find module-scope variables that need to be replaced.
        if (!ir.root_block->IsEmpty()) {
            Vector<Instruction*, 4> to_remove;
            for (auto inst : *ir.root_block) {
                auto* var = inst->As<Var>();
                if (!var) {
                    continue;
                }
                auto* ptr = var->Result()->Type()->As<core::type::Pointer>();
                if (ptr->StoreType()->Is<core::type::ExternalTexture>()) {
                    if (auto res = ReplaceVar(var); DAWN_UNLIKELY(res != Success)) {
                        return res.Failure();
                    }
                    to_remove.Push(var);
                }
            }
            for (auto* remove : to_remove) {
                remove->Destroy();
            }
        }

        // Find function parameters that need to be replaced.
        auto functions = ir.functions;
        for (auto& func : functions) {
            for (uint32_t index = 0; index < func->Params().Length(); index++) {
                auto* param = func->Params()[index];
                if (param->Type()->Is<core::type::ExternalTexture>()) {
                    ReplaceParameter(func, param, index);
                }
            }
        }

        return Success;
    }

    /// @returns a 2D sampled texture type with a f32 sampled type
    const core::type::SampledTexture* SampledTexture() {
        return ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32());
    }

    /// Replace an external texture variable declaration.
    /// @param old_var the variable declaration to replace
    Result<SuccessType> ReplaceVar(Var* old_var) {
        auto name = ir.NameOf(old_var);
        auto bp = old_var->BindingPoint();
        auto itr = multiplanar_map.find(bp.value());
        if (DAWN_UNLIKELY(itr == multiplanar_map.end())) {
            std::stringstream err;
            err << "ExternalTextureOptions missing binding entry for " << bp.value();
            return Failure{err.str()};
        }
        const auto& new_binding_points = itr->second;

        // Create a sampled texture for the first plane.
        auto* plane_0 = b.Var(ty.ptr(handle, SampledTexture()));
        plane_0->SetBindingPoint(bp->group, bp->binding);
        plane_0->InsertBefore(old_var);
        if (name) {
            ir.SetName(plane_0, name.Name() + "_plane0");
        }

        // Create a sampled texture for the second plane.
        auto* plane_1 = b.Var(ty.ptr(handle, SampledTexture()));
        plane_1->SetBindingPoint(new_binding_points.plane_1.group,
                                 new_binding_points.plane_1.binding);
        plane_1->InsertBefore(old_var);
        if (name) {
            ir.SetName(plane_1, name.Name() + "_plane1");
        }

        // Create a uniform buffer for the external texture parameters.
        auto* external_texture_params = b.Var(ty.ptr(uniform, ExternalTextureParams()));
        external_texture_params->SetBindingPoint(new_binding_points.params.group,
                                                 new_binding_points.params.binding);
        external_texture_params->InsertBefore(old_var);
        if (name) {
            ir.SetName(external_texture_params, name.Name() + "_params");
        }

        // Replace all uses of the old variable with the new ones.
        ReplaceUses(old_var->Result(), plane_0->Result(), plane_1->Result(),
                    external_texture_params->Result());

        return Success;
    }

    /// Replace an external texture function parameter.
    /// @param func the function
    /// @param old_param the function parameter to replace
    /// @param index the index of the function parameter
    void ReplaceParameter(Function* func, FunctionParam* old_param, uint32_t index) {
        auto name = ir.NameOf(old_param);

        // Create a sampled texture for the first plane.
        auto* plane_0 = b.FunctionParam(SampledTexture());
        if (name) {
            ir.SetName(plane_0, name.Name() + "_plane0");
        }

        // Create a sampled texture for the second plane.
        auto* plane_1 = b.FunctionParam(SampledTexture());
        if (name) {
            ir.SetName(plane_1, name.Name() + "_plane1");
        }

        // Create the external texture parameters struct.
        auto* external_texture_params = b.FunctionParam(ExternalTextureParams());
        if (name) {
            ir.SetName(external_texture_params, name.Name() + "_params");
        }

        Vector<FunctionParam*, 4> new_params;
        for (uint32_t i = 0; i < func->Params().Length(); i++) {
            if (i == index) {
                new_params.Push(plane_0);
                new_params.Push(plane_1);
                new_params.Push(external_texture_params);
            } else {
                new_params.Push(func->Params()[i]);
            }
        }
        func->SetParams(std::move(new_params));

        // Replace all uses of the old parameter with the new ones.
        ReplaceUses(old_param, plane_0, plane_1, external_texture_params);
    }

    /// Recursively replace the uses of @p value with @p new_value.
    /// @param old_value the external texture value whose usages should be replaced
    /// @param plane_0 the first plane of the replacement texture
    /// @param plane_1 the second plane of the replacement texture
    /// @param params the parameters of the replacement texture
    void ReplaceUses(Value* old_value, Value* plane_0, Value* plane_1, Value* params) {
        old_value->ForEachUseUnsorted([&](Usage use) {
            tint::Switch(
                use.instruction,
                [&](Load* load) {
                    // Load both of the planes and the parameters struct.
                    Value* plane_0_load = nullptr;
                    Value* plane_1_load = nullptr;
                    Value* params_load = nullptr;
                    b.InsertBefore(load, [&] {
                        plane_0_load = b.Load(plane_0)->Result();
                        plane_1_load = b.Load(plane_1)->Result();
                        params_load = b.Load(params)->Result();
                    });
                    ReplaceUses(load->Result(), plane_0_load, plane_1_load, params_load);
                    load->Destroy();
                },
                [&](CoreBuiltinCall* call) {
                    if (call->Func() == core::BuiltinFn::kTextureDimensions) {
                        // Use params.apparentSize + vec2u(1, 1) instead of the textureDimensions.
                        b.InsertBefore(call, [&] {
                            auto* apparent_size = b.Access<vec2<u32>>(params, 12_u);
                            auto* vec2u_1_1 = b.Splat<vec2<u32>>(1_u);
                            auto* dimensions = b.Add<vec2<u32>>(apparent_size, vec2u_1_1);
                            dimensions->SetResult(call->DetachResult());
                        });
                        call->Destroy();
                    } else if (call->Func() == core::BuiltinFn::kTextureLoad) {
                        // Convert the coordinates to unsigned integers if necessary.
                        auto* coords = call->Args()[1];
                        if (coords->Type()->IsSignedIntegerVector()) {
                            auto* convert = b.Convert(ty.vec2<u32>(), coords);
                            convert->InsertBefore(call);
                            coords = convert->Result();
                        }

                        // Call the `TextureLoadExternal()` helper function.
                        auto* helper = b.CallWithResult(call->DetachResult(), TextureLoadExternal(),
                                                        plane_0, plane_1, params, coords);
                        helper->InsertBefore(call);
                        call->Destroy();
                    } else if (call->Func() == core::BuiltinFn::kTextureSampleBaseClampToEdge) {
                        // Call the `TextureSampleExternal()` helper function.
                        auto* sampler = call->Args()[1];
                        auto* coords = call->Args()[2];
                        auto* helper =
                            b.CallWithResult(call->DetachResult(), TextureSampleExternal(), plane_0,
                                             plane_1, params, sampler, coords);
                        helper->InsertBefore(call);
                        call->Destroy();
                    } else {
                        TINT_ICE() << "unhandled texture_external builtin call: " << call->Func();
                    }
                },
                [&](UserCall* call) {
                    // Decompose the external texture operand into both planes and the parameters.
                    Vector<Value*, 4> operands;
                    for (uint32_t i = 0; i < call->Operands().Length(); i++) {
                        if (i == use.operand_index) {
                            operands.Push(plane_0);
                            operands.Push(plane_1);
                            operands.Push(params);
                        } else {
                            operands.Push(call->Operands()[i]);
                        }
                    }
                    call->SetOperands(std::move(operands));
                },  //
                TINT_ICE_ON_NO_MATCH);
        });
    }

    /// @returns the gamma transfer parameters struct
    const core::type::Struct* GammaTransferParams() {
        if (!gamma_transfer_params_struct) {
            gamma_transfer_params_struct = ty.Struct(sym.Register("tint_GammaTransferParams"),
                                                     {
                                                         {sym.Register("G"), ty.f32()},
                                                         {sym.Register("A"), ty.f32()},
                                                         {sym.Register("B"), ty.f32()},
                                                         {sym.Register("C"), ty.f32()},
                                                         {sym.Register("D"), ty.f32()},
                                                         {sym.Register("E"), ty.f32()},
                                                         {sym.Register("F"), ty.f32()},
                                                         {sym.Register("padding"), ty.u32()},
                                                     });
        }
        return gamma_transfer_params_struct;
    }

    /// @returns the external textures parameters struct
    const core::type::Struct* ExternalTextureParams() {
        if (!external_texture_params_struct) {
            external_texture_params_struct =
                ty.Struct(sym.Register("tint_ExternalTextureParams"),
                          {{sym.Register("numPlanes"), ty.u32()},
                           {sym.Register("doYuvToRgbConversionOnly"), ty.u32()},
                           {sym.Register("yuvToRgbConversionMatrix"), ty.mat3x4<f32>()},
                           {sym.Register("gammaDecodeParams"), GammaTransferParams()},
                           {sym.Register("gammaEncodeParams"), GammaTransferParams()},
                           {sym.Register("gamutConversionMatrix"), ty.mat3x3<f32>()},
                           {sym.Register("sampleTransform"), ty.mat3x2<f32>()},
                           {sym.Register("loadTransform"), ty.mat3x2<f32>()},
                           {sym.Register("samplePlane0RectMin"), ty.vec2<f32>()},
                           {sym.Register("samplePlane0RectMax"), ty.vec2<f32>()},
                           {sym.Register("samplePlane1RectMin"), ty.vec2<f32>()},
                           {sym.Register("samplePlane1RectMax"), ty.vec2<f32>()},
                           {sym.Register("apparentSize"), ty.vec2<u32>()},
                           {sym.Register("plane1CoordFactor"), ty.vec2<f32>()}});
        }
        return external_texture_params_struct;
    }

    /// Gets or creates the gamma correction helper function.
    /// @returns the function
    Function* GammaCorrection() {
        if (gamma_correction) {
            return gamma_correction;
        }

        // The helper function implements the following:
        //   fn tint_GammaCorrection(v : vec3f, params : GammaTransferParams) -> vec3f {
        //     let abs_v = abs(v);
        //     let sign_v = sign(v);
        //     let cond = abs_v < vec3f(params.D);
        //     let t = sign_v * ((params.C * abs_v) + params.F);
        //     let f = sign_v * (pow((params.A * abs_v) + params.B, vec3f(params.G)) + params.E);
        //     return select(f, t, cond);
        //   }
        gamma_correction = b.Function("tint_GammaCorrection", ty.vec3<f32>());
        auto* v = b.FunctionParam("v", ty.vec3<f32>());
        auto* params = b.FunctionParam("params", GammaTransferParams());
        gamma_correction->SetParams({v, params});
        b.Append(gamma_correction->Block(), [&] {
            auto* vec3f = ty.vec3<f32>();
            auto* G = b.Access(ty.f32(), params, 0_u);
            auto* A = b.Access(ty.f32(), params, 1_u);
            auto* B = b.Access(ty.f32(), params, 2_u);
            auto* C = b.Access(ty.f32(), params, 3_u);
            auto* D = b.Access(ty.f32(), params, 4_u);
            auto* E = b.Access(ty.f32(), params, 5_u);
            auto* F = b.Access(ty.f32(), params, 6_u);
            auto* G_splat = b.Construct(vec3f, G);
            auto* D_splat = b.Construct(vec3f, D);
            auto* abs_v = b.Call(vec3f, core::BuiltinFn::kAbs, v);
            auto* sign_v = b.Call(vec3f, core::BuiltinFn::kSign, v);
            auto* cond = b.LessThan(ty.vec3<bool>(), abs_v, D_splat);
            auto* t = b.Multiply(vec3f, sign_v, b.Add(vec3f, b.Multiply(vec3f, C, abs_v), F));
            auto* f =
                b.Multiply(vec3f, sign_v,
                           b.Add(vec3f,
                                 b.Call(vec3f, core::BuiltinFn::kPow,
                                        b.Add(vec3f, b.Multiply(vec3f, A, abs_v), B), G_splat),
                                 E));
            b.Return(gamma_correction, b.Call(vec3f, core::BuiltinFn::kSelect, f, t, cond));
        });

        return gamma_correction;
    }

    /// Gets or creates the texture load helper function.
    /// @returns the function
    Function* TextureLoadExternal() {
        if (texture_load_external) {
            return texture_load_external;
        }

        // The helper function implements the following:
        // fn tint_TextureLoadExternal(plane0 : texture_2d<f32>,
        //                             plane1 : texture_2d<f32>,
        //                             coords : vec2<u32>,
        //                             params : ExternalTextureParams) ->vec4f {
        //     let clampedCoords = min(coords, params.apparentSize);
        //     let plane0_clamped = vec2<u32>(
        //         round(params.loadTransform * vec3<f32>(vec2<f32>(clampedCoords), 1)));
        //     var color : vec4<f32>;
        //     if ((params.numPlanes == 1)) {
        //         color = textureLoad(plane0, plane0_clamped, 0).rgba;
        //     } else {
        //         let plane1_clamped = vec2<f32>(plane0_clamped) * params.plane1CoordFactor;
        //
        //         color = vec4<f32>((vec4<f32>(textureLoad(plane0, plane0_clamped, 0).r,
        //                                      textureLoad(plane1, plane1_clamped, 0).rg, 1) *
        //                            params.yuvToRgbConversionMatrix),
        //                           1);
        //     }
        //     if ((params.doYuvToRgbConversionOnly == 0)) {
        //         color = vec4<f32>(gammaCorrection(color.rgb, params.gammaDecodeParams), color.a);
        //         color = vec4<f32>((params.gamutConversionMatrix * color.rgb), color.a);
        //         color = vec4<f32>(gammaCorrection(color.rgb, params.gammaEncodeParams), color.a);
        //     }
        //     return color;
        // }
        texture_load_external = b.Function("tint_TextureLoadExternal", ty.vec4<f32>());
        auto* plane_0 = b.FunctionParam("plane_0", SampledTexture());
        auto* plane_1 = b.FunctionParam("plane_1", SampledTexture());
        auto* params = b.FunctionParam("params", ExternalTextureParams());
        auto* coords = b.FunctionParam("coords", ty.vec2<u32>());
        texture_load_external->SetParams({plane_0, plane_1, params, coords});
        b.Append(texture_load_external->Block(), [&] {
            auto* vec2f = ty.vec2<f32>();
            auto* vec3f = ty.vec3<f32>();
            auto* vec4f = ty.vec4<f32>();
            auto* vec2u = ty.vec2<u32>();
            auto* yuv_to_rgb_conversion_only = b.Access(ty.u32(), params, 1_u);
            auto* yuv_to_rgb_conversion = b.Access(ty.mat3x4<f32>(), params, 2_u);
            auto* load_transform_matrix = b.Access(ty.mat3x2<f32>(), params, 7_u);
            auto* apparent_size = b.Access(ty.vec2<u32>(), params, 12_u);
            auto* plane1_coord_factor = b.Access(ty.vec2<f32>(), params, 13_u);

            auto* clamped_coords = b.Call(vec2u, core::BuiltinFn::kMin, coords, apparent_size);
            auto* clamped_coords_f = b.Convert(vec2f, clamped_coords);
            auto* modified_coords =
                b.Multiply(vec2f, load_transform_matrix, b.Construct(vec3f, clamped_coords_f, 1_f));
            auto* plane0_clamped_f = b.Call(vec2f, core::BuiltinFn::kRound, modified_coords);

            auto* plane0_clamped = b.Convert(vec2u, plane0_clamped_f);

            auto* rgb_result = b.InstructionResult(vec3f);
            auto* alpha_result = b.InstructionResult(ty.f32());
            auto* num_planes = b.Access(ty.u32(), params, 0_u);
            auto* if_planes_eq_1 = b.If(b.Equal(ty.bool_(), num_planes, 1_u));
            if_planes_eq_1->SetResults(rgb_result, alpha_result);
            b.Append(if_planes_eq_1->True(), [&] {
                // Load the texel from the first plane and split into separate rgb and a values.
                auto* texel =
                    b.Call(vec4f, core::BuiltinFn::kTextureLoad, plane_0, plane0_clamped, 0_u);
                auto* rgb = b.Swizzle(vec3f, texel, {0u, 1u, 2u});
                auto* a = b.Access(ty.f32(), texel, 3_u);
                b.ExitIf(if_planes_eq_1, rgb, a);
            });
            b.Append(if_planes_eq_1->False(), [&] {
                // Load the y value from the first plane.
                auto* y = b.Access(
                    ty.f32(),
                    b.Call(vec4f, core::BuiltinFn::kTextureLoad, plane_0, plane0_clamped, 0_u),
                    0_u);

                // Load the uv value from the second plane.
                auto* plane1_clamped_f = b.Multiply(vec2f, plane0_clamped_f, plane1_coord_factor);

                auto* plane1_clamped = b.Convert(vec2u, plane1_clamped_f);
                auto* uv = b.Swizzle(
                    vec2f,
                    b.Call(vec4f, core::BuiltinFn::kTextureLoad, plane_1, plane1_clamped, 0_u),
                    {0u, 1u});

                // Convert the combined yuv value into rgb and set the alpha to 1.0.
                b.ExitIf(if_planes_eq_1,
                         b.Multiply(vec3f, b.Construct(vec4f, y, uv, 1_f), yuv_to_rgb_conversion),
                         1_f);
            });

            // Apply gamma correction if needed.
            auto* final_result = b.InstructionResult(vec3f);
            auto* if_gamma_correct = b.If(b.Equal(ty.bool_(), yuv_to_rgb_conversion_only, 0_u));
            if_gamma_correct->SetResult(final_result);
            b.Append(if_gamma_correct->True(), [&] {
                auto* gamma_decode_params = b.Access(GammaTransferParams(), params, 3_u);
                auto* gamma_encode_params = b.Access(GammaTransferParams(), params, 4_u);
                auto* gamut_conversion_matrix = b.Access(ty.mat3x3<f32>(), params, 5_u);
                auto* decoded = b.Call(vec3f, GammaCorrection(), rgb_result, gamma_decode_params);
                auto* converted = b.Multiply(vec3f, gamut_conversion_matrix, decoded);
                auto* encoded = b.Call(vec3f, GammaCorrection(), converted, gamma_encode_params);
                b.ExitIf(if_gamma_correct, encoded);
            });
            b.Append(if_gamma_correct->False(), [&] {  //
                b.ExitIf(if_gamma_correct, rgb_result);
            });

            b.Return(texture_load_external, b.Construct(vec4f, final_result, alpha_result));
        });

        return texture_load_external;
    }

    /// Gets or creates the texture sample helper function.
    /// @returns the function
    Function* TextureSampleExternal() {
        if (texture_sample_external) {
            return texture_sample_external;
        }

        // The helper function implements the following:
        // fn textureSampleExternal(plane0 : texture_2d<f32>,
        //                          plane1 : texture_2d<f32>,
        //                          smp    : sampler,
        //                          coord  : vec2f,
        //                          params : ExternalTextureParams) ->vec4f {
        //     let modifiedCoords = (params.sampleTransform * vec3<f32>(coord, 1));
        //     let plane0_clamped =
        //         clamp(modifiedCoords, params.samplePlane0RectMin, params.samplePlane0RectMax);
        //     var color : vec4<f32>;
        //
        //     if ((params.numPlanes == 1)) {
        //         color = textureSampleLevel(plane0, smp, plane0_clamped, 0).rgba;
        //     } else {
        //         let plane1_clamped =
        //             clamp(modifiedCoords, params.samplePlane1RectMin,
        //             params.samplePlane1RectMax);
        //        color = vec4<f32>(
        //                   vec4<f32>(textureSampleLevel(plane0, smp, plane0_clamped, 0).r,
        //                             textureSampleLevel(plane1, smp, plane1_clamped, 0).rg, 1) *
        //                   params.yuvToRgbConversionMatrix), 1);
        //     }
        //
        //     if ((params.doYuvToRgbConversionOnly == 0)) {
        //         color = vec4<f32>(gammaCorrection(color.rgb, params.gammaDecodeParams), color.a);
        //         color = vec4<f32>((params.gamutConversionMatrix * color.rgb), color.a);
        //         color = vec4<f32>(gammaCorrection(color.rgb, params.gammaEncodeParams), color.a);
        //     }
        //
        //     return color;
        // }
        texture_sample_external = b.Function("tint_TextureSampleExternal", ty.vec4<f32>());
        auto* plane_0 = b.FunctionParam("plane_0", SampledTexture());
        auto* plane_1 = b.FunctionParam("plane_1", SampledTexture());
        auto* params = b.FunctionParam("params", ExternalTextureParams());
        auto* sampler = b.FunctionParam("tint_sampler", ty.sampler());
        auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
        texture_sample_external->SetParams({plane_0, plane_1, params, sampler, coords});
        b.Append(texture_sample_external->Block(), [&] {
            auto* vec2f = ty.vec2<f32>();
            auto* vec3f = ty.vec3<f32>();
            auto* vec4f = ty.vec4<f32>();
            auto* yuv_to_rgb_conversion_only = b.Access(ty.u32(), params, 1_u);
            auto* yuv_to_rgb_conversion = b.Access(ty.mat3x4<f32>(), params, 2_u);
            auto* transformation_matrix = b.Access(ty.mat3x2<f32>(), params, 6_u);
            auto* sample_plane0_rect_min = b.Access(ty.vec2<f32>(), params, 8_u);
            auto* sample_plane0_rect_max = b.Access(ty.vec2<f32>(), params, 9_u);
            auto* sample_plane1_rect_min = b.Access(ty.vec2<f32>(), params, 10_u);
            auto* sample_plane1_rect_max = b.Access(ty.vec2<f32>(), params, 11_u);

            auto* modified_coords =
                b.Multiply(vec2f, transformation_matrix, b.Construct(vec3f, coords, 1_f));
            auto* plane0_clamped = b.Call(vec2f, core::BuiltinFn::kClamp, modified_coords,
                                          sample_plane0_rect_min, sample_plane0_rect_max);

            auto* rgb_result = b.InstructionResult(vec3f);
            auto* alpha_result = b.InstructionResult(ty.f32());
            auto* num_planes = b.Access(ty.u32(), params, 0_u);
            auto* if_planes_eq_1 = b.If(b.Equal(ty.bool_(), num_planes, 1_u));
            if_planes_eq_1->SetResults(rgb_result, alpha_result);
            b.Append(if_planes_eq_1->True(), [&] {
                // Sample the texel from the first plane and split into separate rgb and a values.
                auto* texel = b.Call(vec4f, core::BuiltinFn::kTextureSampleLevel, plane_0, sampler,
                                     plane0_clamped, 0_f);
                auto* rgb = b.Swizzle(vec3f, texel, {0u, 1u, 2u});
                auto* a = b.Access(ty.f32(), texel, 3_u);
                b.ExitIf(if_planes_eq_1, rgb, a);
            });
            b.Append(if_planes_eq_1->False(), [&] {
                // Sample the y value from the first plane.
                auto* y = b.Access(ty.f32(),
                                   b.Call(vec4f, core::BuiltinFn::kTextureSampleLevel, plane_0,
                                          sampler, plane0_clamped, 0_f),
                                   0_u);
                auto* plane1_clamped = b.Call(vec2f, core::BuiltinFn::kClamp, modified_coords,
                                              sample_plane1_rect_min, sample_plane1_rect_max);

                // Sample the uv value from the second plane.
                auto* uv = b.Swizzle(vec2f,
                                     b.Call(vec4f, core::BuiltinFn::kTextureSampleLevel, plane_1,
                                            sampler, plane1_clamped, 0_f),
                                     {0u, 1u});

                // Convert the combined yuv value into rgb and set the alpha to 1.0.
                b.ExitIf(if_planes_eq_1,
                         b.Multiply(vec3f, b.Construct(vec4f, y, uv, 1_f), yuv_to_rgb_conversion),
                         1_f);
            });

            // Apply gamma correction if needed.
            auto* final_result = b.InstructionResult(vec3f);
            auto* if_gamma_correct = b.If(b.Equal(ty.bool_(), yuv_to_rgb_conversion_only, 0_u));
            if_gamma_correct->SetResult(final_result);
            b.Append(if_gamma_correct->True(), [&] {
                auto* gamma_decode_params = b.Access(GammaTransferParams(), params, 3_u);
                auto* gamma_encode_params = b.Access(GammaTransferParams(), params, 4_u);
                auto* gamut_conversion_matrix = b.Access(ty.mat3x3<f32>(), params, 5_u);
                auto* decoded = b.Call(vec3f, GammaCorrection(), rgb_result, gamma_decode_params);
                auto* converted = b.Multiply(vec3f, gamut_conversion_matrix, decoded);
                auto* encoded = b.Call(vec3f, GammaCorrection(), converted, gamma_encode_params);
                b.ExitIf(if_gamma_correct, encoded);
            });
            b.Append(if_gamma_correct->False(), [&] {  //
                b.ExitIf(if_gamma_correct, rgb_result);
            });

            b.Return(texture_sample_external, b.Construct(vec4f, final_result, alpha_result));
        });

        return texture_sample_external;
    }
};

}  // namespace

Result<SuccessType> MultiplanarExternalTexture(
    Module& ir,
    const tint::transform::multiplanar::BindingsMap& multiplanar_map) {
    auto result = ValidateAndDumpIfNeeded(ir, "core.MultiplanarExternalTexture");
    if (result != Success) {
        return result;
    }

    return State{multiplanar_map, ir}.Process();
}

}  // namespace tint::core::ir::transform
