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
template <class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};

using MultiplanarTexture = tint::transform::multiplanar::MultiplanarTexture;
using YCBCRTexture = tint::transform::multiplanar::YCBCRTexture;

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

    /// The transfer function parameters structure.
    const core::type::Struct* transfer_function_params_struct = nullptr;

    /// The external texture parameters structure.
    const core::type::Struct* external_texture_params_struct = nullptr;

    /// The helper function that implements `textureLoad()`.
    Function* texture_load_multiplanar_external = nullptr;
    Function* texture_load_ycbcr_external = nullptr;

    /// The helper function that implements `textureSampleBaseClampToEdge()`.
    Function* texture_sample_clamp_to_edge_multiplanar_external = nullptr;
    Function* texture_sample_clamp_to_edge_ycbcr_external = nullptr;

    /// The transfer function application helper functions.
    Function* apply_src_transfer_function = nullptr;
    Function* apply_gamma_transfer_function = nullptr;
    Function* apply_hlg_transfer_function = nullptr;
    Function* apply_pq_transfer_function = nullptr;

    enum class UsedAs : uint8_t {
        kMultiplanar,
        kYcbcr,
    };

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
                    TINT_CHECK_RESULT(ReplaceVar(var));
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

        // Create a uniform buffer for the external texture parameters. The binding point is set in
        // the specific type in the replace calls below.
        auto* external_texture_params = b.Var(ty.ptr(uniform, ExternalTextureParams()));
        external_texture_params->InsertBefore(old_var);
        if (name) {
            ir.SetName(external_texture_params, name.Name() + "_params");
        }

        // Create a sampled texture for texture. The name is set inside the specific calls below
        auto* texture = b.Var(ty.ptr(handle, SampledTexture()));
        texture->SetBindingPoint(bp->group, bp->binding);
        texture->InsertBefore(old_var);

        std::visit(overloaded{[&](const MultiplanarTexture& m) {
                                  ReplaceMultiplanarVar(old_var, name, m, texture,
                                                        external_texture_params);
                              },
                              [&](const YCBCRTexture& t) {
                                  ReplaceYCBCRVar(old_var, name, t, texture,
                                                  external_texture_params);
                              }},
                   itr->second);

        return Success;
    }

    void ReplaceMultiplanarVar(Var* old_var,
                               Symbol name,
                               const MultiplanarTexture& new_binding_points,
                               Var* plane_0,
                               Var* external_texture_params) {
        if (name) {
            ir.SetName(plane_0, name.Name() + "_plane0");
        }

        external_texture_params->SetBindingPoint(new_binding_points.params.group,
                                                 new_binding_points.params.binding);

        // Create a sampled texture for the second plane.
        auto* plane_1 = b.Var(ty.ptr(handle, SampledTexture()));
        plane_1->SetBindingPoint(new_binding_points.plane_1.group,
                                 new_binding_points.plane_1.binding);
        plane_1->InsertBefore(old_var);
        if (name) {
            ir.SetName(plane_1, name.Name() + "_plane1");
        }

        // Replace all uses of the old variable with the new ones.
        ReplaceUses(UsedAs::kMultiplanar, old_var->Result(), plane_0->Result(), plane_1->Result(),
                    external_texture_params->Result());
    }
    void ReplaceYCBCRVar(Var* old_var,
                         Symbol name,
                         const YCBCRTexture& new_binding_points,
                         Var* texture,
                         Var* external_texture_params) {
        if (name) {
            ir.SetName(texture, name.Name());
        }

        external_texture_params->SetBindingPoint(new_binding_points.params.group,
                                                 new_binding_points.params.binding);

        // Create a sampler
        auto* sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(new_binding_points.sampler.group,
                                 new_binding_points.sampler.binding);
        sampler->InsertBefore(old_var);
        if (name) {
            ir.SetName(sampler, name.Name() + "_ycbcr_sampler");
        }

        // Replace all uses of the old variable with the new ones.
        ReplaceUses(UsedAs::kYcbcr, old_var->Result(), texture->Result(), sampler->Result(),
                    external_texture_params->Result());
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
        ReplaceUses(UsedAs::kMultiplanar, old_param, plane_0, plane_1, external_texture_params);
    }

    /// Recursively replace the uses of @p value with @p new_value.
    /// @param used_as how the parameter is used
    /// @param old_value the external texture value whose usages should be replaced
    /// @param first the first parameter of the replacement, a texture
    /// @param second the second parameter of the replacement, a texture or a sampler
    /// @param params the parameters of the replacement texture
    void ReplaceUses(UsedAs used_as, Value* old_value, Value* first, Value* second, Value* params) {
        old_value->ForEachUseUnsorted([&](Usage use) {
            tint::Switch(
                use.instruction,
                [&](Load* load) {
                    // Load both of the planes and the parameters struct.
                    Value* et_first_load = nullptr;
                    Value* et_second_load = nullptr;
                    Value* et_params_load = nullptr;
                    b.InsertBefore(load, [&] {
                        et_first_load = b.Load(first)->Result();
                        et_second_load = b.Load(second)->Result();
                        et_params_load = b.Load(params)->Result();
                    });
                    ReplaceUses(used_as, load->Result(), et_first_load, et_second_load,
                                et_params_load);
                    load->Destroy();
                },
                [&](CoreBuiltinCall* call) {
                    if (call->Func() == core::BuiltinFn::kTextureDimensions) {
                        // Use params.apparentSize + vec2u(1, 1) instead of the textureDimensions.
                        b.InsertBefore(call, [&] {
                            auto* apparent_size = b.Access<vec2u>(params, 12_u);
                            auto* vec2u_1_1 = b.Splat<vec2u>(1_u);
                            auto* dimensions = b.Add(apparent_size, vec2u_1_1);
                            dimensions->SetResult(call->DetachResult());
                        });
                        call->Destroy();
                    } else if (call->Func() == core::BuiltinFn::kTextureLoad) {
                        // Convert the coordinates to unsigned integers if necessary.
                        auto* coords = call->Args()[1];
                        if (coords->Type()->IsSignedIntegerVector()) {
                            auto* convert = b.Convert(ty.vec2u(), coords);
                            convert->InsertBefore(call);
                            coords = convert->Result();
                        }

                        // Call the `TextureLoadExternal()` helper function.
                        Call* helper = nullptr;
                        if (used_as == UsedAs::kMultiplanar) {
                            Function* func = TextureLoadMultiplanarExternal();
                            helper = b.CallWithResult(call->DetachResult(), func, first, second,
                                                      params, coords);
                        } else {
                            Function* func = TextureLoadYCBCRExternal();
                            helper =
                                b.CallWithResult(call->DetachResult(), func, first, params, coords);
                        }
                        helper->InsertBefore(call);
                        call->Destroy();
                    } else if (call->Func() == core::BuiltinFn::kTextureSampleBaseClampToEdge) {
                        // Call the `TextureSampleClampToEdgeMultiplanarExternal()` helper function.
                        auto* sampler = call->Args()[1];
                        auto* coords = call->Args()[2];

                        Call* helper = nullptr;
                        if (used_as == UsedAs::kMultiplanar) {
                            Function* func = TextureSampleClampToEdgeMultiplanarExternal();
                            helper = b.CallWithResult(call->DetachResult(), func, first, second,
                                                      params, sampler, coords);
                        } else {
                            Function* func = TextureSampleClampToEdgeYCBCRExternal();
                            helper = b.CallWithResult(call->DetachResult(), func, first, second,
                                                      params, coords);
                        }
                        helper->InsertBefore(call);
                        call->Destroy();
                    } else {
                        TINT_IR_ICE(ir)
                            << "unhandled texture_external builtin call: " << call->Func();
                    }
                },
                [&](UserCall* call) {
                    TINT_ASSERT(used_as != UsedAs::kYcbcr);

                    // Decompose the external texture operand into both planes and the parameters.
                    Vector<Value*, 4> operands;
                    for (uint32_t i = 0; i < call->Operands().Length(); i++) {
                        if (i == use.operand_index) {
                            operands.Push(first);
                            operands.Push(second);
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

    /// @returns the transfer function parameters struct
    const core::type::Struct* TransferFunctionParams() {
        if (!transfer_function_params_struct) {
            transfer_function_params_struct = ty.Struct(sym.Register("tint_TransferFunctionParams"),
                                                        {
                                                            {sym.Register("mode"), ty.u32()},
                                                            {sym.Register("A"), ty.f32()},
                                                            {sym.Register("B"), ty.f32()},
                                                            {sym.Register("C"), ty.f32()},
                                                            {sym.Register("D"), ty.f32()},
                                                            {sym.Register("E"), ty.f32()},
                                                            {sym.Register("F"), ty.f32()},
                                                            {sym.Register("G"), ty.f32()},
                                                        });
        }
        return transfer_function_params_struct;
    }

    /// @returns the external textures parameters struct
    const core::type::Struct* ExternalTextureParams() {
        if (!external_texture_params_struct) {
            external_texture_params_struct =
                ty.Struct(sym.Register("tint_ExternalTextureParams"),
                          {
                              {sym.Register("numPlanes"), ty.u32()},
                              {sym.Register("doYuvToRgbConversionOnly"), ty.u32()},
                              {sym.Register("yuvToRgbConversionMatrix"), ty.mat3x4<f32>()},
                              {sym.Register("srcTransferFunction"), TransferFunctionParams()},
                              {sym.Register("dstTransferFunction"), TransferFunctionParams()},
                              {sym.Register("gamutConversionMatrix"), ty.mat3x3<f32>()},
                              {sym.Register("sampleTransform"), ty.mat3x2<f32>()},
                              {sym.Register("loadTransform"), ty.mat3x2<f32>()},
                              {sym.Register("samplePlane0RectMin"), ty.vec2f()},
                              {sym.Register("samplePlane0RectMax"), ty.vec2f()},
                              {sym.Register("samplePlane1RectMin"), ty.vec2f()},
                              {sym.Register("samplePlane1RectMax"), ty.vec2f()},
                              {sym.Register("apparentSize"), ty.vec2u()},
                              {sym.Register("plane1CoordFactor"), ty.vec2f()},
                          });
        }
        return external_texture_params_struct;
    }

    /// Gets or creates the gamma transfer function application helper function.
    /// @returns the function
    Function* ApplyGammaTransferFunction() {
        if (apply_gamma_transfer_function) {
            return apply_gamma_transfer_function;
        }

        // The helper function implements the following:
        //   fn tint_ApplyGammaTransferFunction(v : vec3f, params : TransferFunctionParams) -> vec3f
        //   {
        //     let abs_v = abs(v);
        //     let sign_v = sign(v);
        //     let cond = abs_v < vec3f(params.D);
        //     let t = sign_v * ((params.C * abs_v) + params.F);
        //     let f = sign_v * (pow((params.A * abs_v) + params.B, vec3f(params.G)) + params.E);
        //     return select(f, t, cond);
        //   }
        apply_gamma_transfer_function = b.Function("tint_ApplyGammaTransferFunction", ty.vec3f());
        auto* v = b.FunctionParam("v", ty.vec3f());
        auto* params = b.FunctionParam("params", TransferFunctionParams());
        apply_gamma_transfer_function->SetParams({v, params});
        b.Append(apply_gamma_transfer_function->Block(), [&] {
            auto* vec3f = ty.vec3f();
            auto* A = b.Access(ty.f32(), params, 1_u);
            auto* B = b.Access(ty.f32(), params, 2_u);
            auto* C = b.Access(ty.f32(), params, 3_u);
            auto* D = b.Access(ty.f32(), params, 4_u);
            auto* E = b.Access(ty.f32(), params, 5_u);
            auto* F = b.Access(ty.f32(), params, 6_u);
            auto* G = b.Access(ty.f32(), params, 7_u);

            b.Name("A", A);
            b.Name("B", B);
            b.Name("C", C);
            b.Name("D", D);
            b.Name("E", E);
            b.Name("F", F);
            b.Name("G", G);

            auto* G_splat = b.Construct(vec3f, G);
            auto* D_splat = b.Construct(vec3f, D);
            auto* abs_v = b.Call(vec3f, core::BuiltinFn::kAbs, v);
            auto* sign_v = b.Call(vec3f, core::BuiltinFn::kSign, v);
            auto* cond = b.LessThan(abs_v, D_splat);
            auto* t = b.Multiply(sign_v, b.Add(b.Multiply(C, abs_v), F));
            auto* f = b.Multiply(sign_v, b.Add(b.Call(vec3f, core::BuiltinFn::kPow,
                                                      b.Add(b.Multiply(A, abs_v), B), G_splat),
                                               E));
            b.Return(apply_gamma_transfer_function,
                     b.Call(vec3f, core::BuiltinFn::kSelect, f, t, cond));
        });

        return apply_gamma_transfer_function;
    }

    /// Gets or creates the hlg transfer function application helper function.
    /// @returns the function
    /// Reference:
    ///   * https://www.khronos.org/registry/DataFormat/specs/1.3/dataformat.1.3.inline.html#TRANSFER_HLG
    ///   * Specifically HLG OETF^-1 (normalized)
    Function* ApplyHLGTransferFunction() {
        if (apply_hlg_transfer_function) {
            return apply_hlg_transfer_function;
        }

        // The helper function implements the following:
        //   note:      cutoff = params.D
        //         lower_scale = params.E
        //         upper_scale = params.F

        //   fn tint_ApplyHLGSingleChannel(v : f32, params: TransferFunctionParams -> f32 {
        //     if (c <= params.cutoff) {
        //       return (v * v) / params.lower_scale
        //     } else {
        //       return (params.B + exp((v - params.C) / params.A)) / params.upper_scale;
        //     }
        //     unreachable();
        //   }
        //
        //   fn tint_ApplyHLGTransferFunction(v : vec3f, params : TransferFunctionParams) -> vec3f {
        //     let r = tintApplyHLGSingleChannel(v.r, params);
        //     let g = tintApplyHLGSingleChannel(v.g, params);
        //     let b = tintApplyHLGSingleChannel(v.b, params);
        //     return vec3f(r, g, b);
        //   }

        Function* apply_hlg_single_channel = nullptr;
        {
            apply_hlg_single_channel = b.Function("tint_ApplyHLGSingleChannel", ty.f32());
            auto* v = b.FunctionParam("v", ty.f32());
            auto* params = b.FunctionParam("params", TransferFunctionParams());
            apply_hlg_single_channel->SetParams({v, params});
            b.Append(apply_hlg_single_channel->Block(), [&] {
                auto* A = b.Access(ty.f32(), params, 1_u);
                auto* B = b.Access(ty.f32(), params, 2_u);
                auto* C = b.Access(ty.f32(), params, 3_u);
                auto* cutoff = b.Access(ty.f32(), params, 4_u);
                auto* lower_scale = b.Access(ty.f32(), params, 5_u);
                auto* upper_scale = b.Access(ty.f32(), params, 6_u);

                b.Name("A", A);
                b.Name("B", B);
                b.Name("C", C);
                b.Name("cutoff", cutoff);
                b.Name("lower_scale", lower_scale);
                b.Name("upper_scale", upper_scale);

                auto* if_ = b.If(b.LessThanEqual(v, cutoff));
                b.Append(if_->True(), [&] {
                    auto* vv = b.Multiply(v, v);
                    b.Return(apply_hlg_single_channel, b.Divide(vv, lower_scale));
                });
                b.Append(if_->False(), [&] {
                    auto* exp =
                        b.Call(ty.f32(), core::BuiltinFn::kExp, b.Divide(b.Subtract(v, C), A));
                    auto* num = b.Add(B, exp);
                    b.Return(apply_hlg_single_channel, b.Divide(num, upper_scale));
                });
                b.Unreachable();
            });
        }

        {
            apply_hlg_transfer_function = b.Function("tint_ApplyHLGTransferFunction", ty.vec3f());
            auto* v = b.FunctionParam("v", ty.vec3f());
            auto* params = b.FunctionParam("params", TransferFunctionParams());
            apply_hlg_transfer_function->SetParams({v, params});
            b.Append(apply_hlg_transfer_function->Block(), [&] {
                auto* red = b.Call(ty.f32(), apply_hlg_single_channel,
                                   b.Swizzle(ty.f32(), v, {0_u}), params);
                auto* green = b.Call(ty.f32(), apply_hlg_single_channel,
                                     b.Swizzle(ty.f32(), v, {1_u}), params);
                auto* blue = b.Call(ty.f32(), apply_hlg_single_channel,
                                    b.Swizzle(ty.f32(), v, {2_u}), params);

                b.Return(apply_hlg_transfer_function, b.Construct(ty.vec3f(), red, green, blue));
            });
        }

        return apply_hlg_transfer_function;
    }

    /// Gets or creates the pq transfer function application helper function.
    /// @returns the function
    // Reference:
    //   * https://registry.khronos.org/DataFormat/specs/1.3/dataformat.1.3.inline.html#TRANSFER_PQ
    //   * Specifically the PQ EOTF
    Function* ApplyPQTransferFunction() {
        if (apply_pq_transfer_function) {
            return apply_pq_transfer_function;
        }

        //
        // The helper function implements the following:
        //   note: m1 = params.A
        //         m2 = params.B
        //         c1 = params.C
        //         c2 = params.D
        //         c3 = params.E
        //   fn tint_ApplyPQTransferFunction(v : vec3f, params : TransferFunctionParams) -> vec3f {
        //     let clamped = clamp(v, vec3f(0), vec3f(1));
        //     let pow = pow(clamped, 1.f / vec3f(m2));
        //     let num = max(pow - c1, 0.f);
        //     let denom = c2 - (c3 * pow);
        //     let res = num / denom
        //     return pow(res, 1.f / vec3f(m1));
        //   }
        apply_pq_transfer_function = b.Function("tint_ApplyPQTransferFunction", ty.vec3f());
        auto* v = b.FunctionParam("v", ty.vec3f());
        auto* params = b.FunctionParam("params", TransferFunctionParams());
        apply_pq_transfer_function->SetParams({v, params});
        b.Append(apply_pq_transfer_function->Block(), [&] {
            auto* m1 = b.Access(ty.f32(), params, 1_u);
            auto* m2 = b.Access(ty.f32(), params, 2_u);
            auto* c1 = b.Access(ty.f32(), params, 3_u);
            auto* c2 = b.Access(ty.f32(), params, 4_u);
            auto* c3 = b.Access(ty.f32(), params, 5_u);

            b.Name("m1", m1);
            b.Name("m2", m2);
            b.Name("c1", c1);
            b.Name("c2", c2);
            b.Name("c3", c3);

            auto* c1_splat = b.Construct(ty.vec3f(), c1);
            auto* c2_splat = b.Construct(ty.vec3f(), c2);
            auto* c3_splat = b.Construct(ty.vec3f(), c3);
            auto* m1_splat = b.Construct(ty.vec3f(), m1);
            auto* m2_splat = b.Construct(ty.vec3f(), m2);

            auto* one = b.Splat(ty.vec3f(), 1_f);
            auto* zero = b.Splat(ty.vec3f(), 0_f);

            auto* clamped = b.Call(ty.vec3f(), core::BuiltinFn::kClamp, v, zero, one);
            auto* pow = b.Call(ty.vec3f(), core::BuiltinFn::kPow, clamped, b.Divide(one, m2_splat));
            auto* num = b.Call(ty.vec3f(), core::BuiltinFn::kMax, b.Subtract(pow, c1_splat), zero);
            auto* denom = b.Subtract(c2_splat, b.Multiply(c3_splat, pow));
            auto* res = b.Divide(num, denom);
            b.Return(apply_pq_transfer_function,
                     b.Call(ty.vec3f(), core::BuiltinFn::kPow, res, b.Divide(one, m1_splat)));
        });

        return apply_pq_transfer_function;
    }

    // The src transfer function is based on the mode param
    Function* ApplySrcTransferFunction() {
        if (apply_src_transfer_function) {
            return apply_src_transfer_function;
        }

        // The helper function implements the following:
        //   fn tint_ApplySrcTransferFunction(v : vec3f, params : TransferFunctionParams) -> vec3f {
        //     let mode = params.mode;
        //     if (mode == 0) {
        //       return ApplyGammaTransferFunction(v, params);
        //     } else {
        //       if (mode == 1) {
        //         return ApplyHLGTransferFunction(v, params);
        //       } else {
        //         return ApplyPQTransferFunction(v, params);
        //       }
        //       unreachable();
        //     }
        //     unreachable();
        //   }

        apply_src_transfer_function = b.Function("tint_ApplySrcTransferFunction", ty.vec3f());
        auto* v = b.FunctionParam("v", ty.vec3f());
        auto* params = b.FunctionParam("params", TransferFunctionParams());
        apply_src_transfer_function->SetParams({v, params});
        b.Append(apply_src_transfer_function->Block(), [&] {
            auto* mode = b.Access(ty.u32(), params, 0_u);
            b.Name("mode", mode);

            auto* outer_if = b.If(b.Equal(mode, 0_u));
            b.Append(outer_if->True(), [&] {
                b.Return(apply_src_transfer_function,
                         b.Call(ty.vec3f(), ApplyGammaTransferFunction(), v, params));
            });
            b.Append(outer_if->False(), [&] {
                auto* inner_if = b.If(b.Equal(mode, 1_u));
                b.Append(inner_if->True(), [&] {
                    b.Return(apply_src_transfer_function,
                             b.Call(ty.vec3f(), ApplyHLGTransferFunction(), v, params));
                });

                b.Append(inner_if->False(), [&] {
                    b.Return(apply_src_transfer_function,
                             b.Call(ty.vec3f(), ApplyPQTransferFunction(), v, params));
                });
                b.Unreachable();
            });
            b.Unreachable();
        });

        return apply_src_transfer_function;
    }

    // The destination transfer is always gamma
    Function* ApplyDstTransferFunction() { return ApplyGammaTransferFunction(); }

    /// Gets or creates the multiplanar texture load helper function.
    /// @returns the function
    Function* TextureLoadMultiplanarExternal() {
        if (texture_load_multiplanar_external) {
            return texture_load_multiplanar_external;
        }

        // The helper function implements the following:
        // fn tint_TextureLoadMultiplanarExternal(plane0: texture_2d<f32>,
        //                                        plane1: texture_2d<f32>,
        //                                        params: ExternalTextureParams,
        //                                        coords: vec2<u32>) ->vec4f {
        //     let clampedCoords = min(coords, params.apparentSize);
        //     let loadCoords = vec2<u32>(
        //         round(params.loadTransform * vec3<f32>(vec2<f32>(clampedCoords), 1)));
        //     var color: vec3<f32>;
        //     var alpha: f32;
        //     if ((params.numPlanes == 1)) {
        //         let val = textureLoad(plane0, loadCoords, 0).rgba;
        //          color = val.xyz;
        //          alpha = val.w;
        //     } else {
        //         let plane1_clamped = vec2<u32>(vec2<f32>(loadCoords) *
        //                                params.plane1CoordFactor);
        //
        //         color = (vec4<f32>(textureLoad(plane0, loadCoords, 0).r,
        //                                      textureLoad(plane1, plane1_clamped, 0).rg, 1) *
        //                            params.yuvToRgbConversionMatrix));
        //         alpha = 1.f;
        //     }
        //     if ((params.doYuvToRgbConversionOnly == 0)) {
        //         color = applyTransferFunction(color, params.srcTransferFunction);
        //         color = params.gamutConversionMatrix * color;
        //         color = applyTransferFunction(color, params.dstTransferFunction);
        //     }
        //     return vec4f<f32>(color, alpha);
        // }
        texture_load_multiplanar_external =
            b.Function("tint_TextureLoadMultiplanarExternal", ty.vec4f());
        auto* plane_0 = b.FunctionParam("plane_0", SampledTexture());
        auto* plane_1 = b.FunctionParam("plane_1", SampledTexture());
        auto* params = b.FunctionParam("params", ExternalTextureParams());
        auto* coords = b.FunctionParam("coords", ty.vec2u());
        texture_load_multiplanar_external->SetParams({plane_0, plane_1, params, coords});

        b.Append(texture_load_multiplanar_external->Block(), [&] {
            auto* yuv_to_rgb_conversion_only = b.Access(ty.u32(), params, 1_u);
            auto* yuv_to_rgb_conversion = b.Access(ty.mat3x4<f32>(), params, 2_u);
            auto* load_transform_matrix = b.Access(ty.mat3x2<f32>(), params, 7_u);
            auto* apparent_size = b.Access(ty.vec2u(), params, 12_u);
            auto* plane1_coord_factor = b.Access(ty.vec2f(), params, 13_u);

            auto* clamped_coords = b.Min(coords, apparent_size);
            auto* clamped_coords_f = b.Convert(ty.vec2f(), clamped_coords);
            auto* modified_coords =
                b.Multiply(load_transform_matrix, b.Construct(ty.vec3f(), clamped_coords_f, 1_f));
            auto* loadCoords_f = b.Call(ty.vec2f(), core::BuiltinFn::kRound, modified_coords);
            auto* loadCoords = b.Convert(ty.vec2u(), loadCoords_f);

            auto* rgb_result = b.InstructionResult(ty.vec3f());
            auto* alpha_result = b.InstructionResult(ty.f32());
            auto* num_planes = b.Access(ty.u32(), params, 0_u);
            auto* if_planes_eq_1 = b.If(b.Equal(num_planes, 1_u));
            if_planes_eq_1->SetResults(rgb_result, alpha_result);
            b.Append(if_planes_eq_1->True(), [&] {
                // Load the texel from the first plane and split into separate rgb and a values.
                auto* texel =
                    b.Call(ty.vec4f(), core::BuiltinFn::kTextureLoad, plane_0, loadCoords, 0_u);
                auto* rgb = b.Swizzle(ty.vec3f(), texel, {0u, 1u, 2u});
                auto* a = b.Access(ty.f32(), texel, 3_u);
                b.ExitIf(if_planes_eq_1, rgb, a);
            });
            b.Append(if_planes_eq_1->False(), [&] {
                // Load the y value from the first plane.
                auto* y = b.Access(
                    ty.f32(),
                    b.Call(ty.vec4f(), core::BuiltinFn::kTextureLoad, plane_0, loadCoords, 0_u),
                    0_u);

                // Load the uv value from the second plane.
                auto* plane1_clamped_f = b.Multiply(loadCoords_f, plane1_coord_factor);

                auto* plane1_clamped = b.Convert(ty.vec2u(), plane1_clamped_f);
                auto* uv = b.Swizzle(
                    ty.vec2f(),
                    b.Call(ty.vec4f(), core::BuiltinFn::kTextureLoad, plane_1, plane1_clamped, 0_u),
                    {0u, 1u});

                // Convert the combined yuv value into rgb and set the alpha to 1.0.
                b.ExitIf(if_planes_eq_1,
                         b.Multiply(b.Construct(ty.vec4f(), y, uv, 1_f), yuv_to_rgb_conversion),
                         1_f);
            });

            // Apply gamma correction if needed.
            auto* final_result = b.InstructionResult(ty.vec3f());
            auto* if_gamma_correct = b.If(b.Equal(yuv_to_rgb_conversion_only, 0_u));
            if_gamma_correct->SetResult(final_result);
            b.Append(if_gamma_correct->True(), [&] {
                auto* src_transfer_function = b.Access(TransferFunctionParams(), params, 3_u);
                auto* dst_transfer_function = b.Access(TransferFunctionParams(), params, 4_u);
                auto* gamut_conversion_matrix = b.Access(ty.mat3x3<f32>(), params, 5_u);
                auto* decoded = b.Call(ty.vec3f(), ApplySrcTransferFunction(), rgb_result,
                                       src_transfer_function);
                auto* converted = b.Multiply(gamut_conversion_matrix, decoded);
                auto* encoded = b.Call(ty.vec3f(), ApplyDstTransferFunction(), converted,
                                       dst_transfer_function);
                b.ExitIf(if_gamma_correct, encoded);
            });
            b.Append(if_gamma_correct->False(), [&] {  //
                b.ExitIf(if_gamma_correct, rgb_result);
            });

            b.Return(texture_load_multiplanar_external,
                     b.Construct(ty.vec4f(), final_result, alpha_result));
        });

        return texture_load_multiplanar_external;
    }

    /// Gets or creates the YCBCR texture load helper function.
    /// @returns the function
    Function* TextureLoadYCBCRExternal() {
        if (texture_load_ycbcr_external) {
            return texture_load_ycbcr_external;
        }

        // The helper function implements the following:
        // fn tint_TextureLoadYcbcrExternal(texture: texture_2d<f32>,
        //                                  params: ExternalTextureParams,
        //                                  coords: vec2<u32>) ->vec4f {
        //   let clampedCoords = min(coords, params.apparentSize);
        //   let loadCoords = vec2<u32>(
        //       round(params.loadTransform * vec3<f32>(vec2<f32>(clampedCoords), 1)));
        //   var color = textureLoad(texture, loadCoords, 0).rgb, 1) *
        //                   params.yuvToRgbConversionMatrix;
        //   if ((params.doYuvToRgbConversionOnly == 0)) {
        //       color = applyTransferFunction(color.rgb, params.srcTransferFunction);
        //       color = (params.gamutConversionMatrix * color.rgb);
        //       color = applyTransferFunction(color.rgb, params.dstTransferFunction);
        //   }
        //   return vec4f(color, 1);
        // }
        texture_load_ycbcr_external = b.Function("tint_TextureLoadYcbcrExternal", ty.vec4f());
        auto* texture = b.FunctionParam("texture", SampledTexture());
        auto* params = b.FunctionParam("params", ExternalTextureParams());
        auto* coords = b.FunctionParam("coords", ty.vec2u());
        texture_load_ycbcr_external->SetParams({texture, params, coords});

        b.Append(texture_load_ycbcr_external->Block(), [&] {
            auto* yuv_to_rgb_conversion_only = b.Access(ty.u32(), params, 1_u);
            auto* yuv_to_rgb_conversion = b.Access(ty.mat3x4<f32>(), params, 2_u);
            auto* load_transform_matrix = b.Access(ty.mat3x2<f32>(), params, 7_u);
            auto* apparent_size = b.Access(ty.vec2u(), params, 12_u);

            auto* clamped_coords = b.Min(coords, apparent_size);
            auto* clamped_coords_f = b.Convert(ty.vec2f(), clamped_coords);

            auto* modified_coords =
                b.Multiply(load_transform_matrix, b.Construct(ty.vec3f(), clamped_coords_f, 1_f));
            auto* load_coords_f = b.Call(ty.vec2f(), core::BuiltinFn::kRound, modified_coords);
            auto* load_coords = b.Convert(ty.vec2u(), load_coords_f);

            auto* load =
                b.Call(ty.vec4f(), core::BuiltinFn::kTextureLoad, texture, load_coords, 0_u);

            auto* extract_rgb =
                b.Construct(ty.vec4f(), b.Swizzle(ty.vec3f(), load, {0u, 1u, 2u}), 1_f);
            auto* rgb_result = b.Multiply(extract_rgb, yuv_to_rgb_conversion);

            // Apply gamma correction if needed.
            auto* final_result = b.InstructionResult(ty.vec3f());
            auto* if_gamma_correct = b.If(b.Equal(yuv_to_rgb_conversion_only, 0_u));
            if_gamma_correct->SetResult(final_result);
            b.Append(if_gamma_correct->True(), [&] {
                auto* src_transfer_function = b.Access(TransferFunctionParams(), params, 3_u);
                auto* dst_transfer_function = b.Access(TransferFunctionParams(), params, 4_u);
                auto* gamut_conversion_matrix = b.Access(ty.mat3x3<f32>(), params, 5_u);
                auto* decoded = b.Call(ty.vec3f(), ApplySrcTransferFunction(), rgb_result,
                                       src_transfer_function);
                auto* converted = b.Multiply(gamut_conversion_matrix, decoded);
                auto* encoded = b.Call(ty.vec3f(), ApplyDstTransferFunction(), converted,
                                       dst_transfer_function);
                b.ExitIf(if_gamma_correct, encoded);
            });
            b.Append(if_gamma_correct->False(), [&] {  //
                b.ExitIf(if_gamma_correct, rgb_result);
            });

            b.Return(texture_load_ycbcr_external, b.Construct(ty.vec4f(), final_result, 1_f));
        });

        return texture_load_ycbcr_external;
    }

    /// Gets or creates the texture sample helper function.
    /// @returns the function
    Function* TextureSampleClampToEdgeMultiplanarExternal() {
        if (texture_sample_clamp_to_edge_multiplanar_external) {
            return texture_sample_clamp_to_edge_multiplanar_external;
        }

        // The helper function implements the following:
        // fn textureSampleClampToEdgeMultiplanarExternal(plane0 : texture_2d<f32>,
        //                          plane1 : texture_2d<f32>,
        //                          params : ExternalTextureParams,
        //                          tint_sampler : sampler,
        //                          coord  : vec2f) ->vec4f {
        //     let modifiedCoords = params.sampleTransform * vec3<f32>(coord, 1);
        //     let loadCoords =
        //         clamp(modifiedCoords, params.samplePlane0RectMin, params.samplePlane0RectMax);
        //
        //     var color: vec3<f32>;
        //     var alpha: f32
        //
        //     if ((params.numPlanes == 1)) {
        //         let val = textureSampleLevel(plane0, tint_sampler, loadCoords, 0);
        //         color = val.rgb;
        //         alpha = val.a;
        //     } else {
        //         let plane1_clamped =
        //             clamp(modifiedCoords, params.samplePlane1RectMin,
        //             params.samplePlane1RectMax);
        //        color = vec4<f32>(textureSampleLevel(plane0, tint_sampler, loadCoords, 0).r,
        //                             textureSampleLevel(plane1, tint_sampler, plane1_clamped,
        //                             0).rg, 1) *
        //                   params.yuvToRgbConversionMatrix)
        //        alpha = 1.f;
        //     }
        //
        //     if ((params.doYuvToRgbConversionOnly == 0)) {
        //         color = applyTransferFunction(color.rgb, params.srcTransferFunction);
        //         color = (params.gamutConversionMatrix * color.rgb);
        //         color = applyTransferFunction(color.rgb, params.dstTransferFunction);
        //     }
        //
        //     return vec4f(color, a);
        // }
        texture_sample_clamp_to_edge_multiplanar_external =
            b.Function("tint_TextureSampleClampToEdgeMultiplanarExternal", ty.vec4f());
        auto* plane_0 = b.FunctionParam("plane_0", SampledTexture());
        auto* plane_1 = b.FunctionParam("plane_1", SampledTexture());
        auto* params = b.FunctionParam("params", ExternalTextureParams());
        auto* sampler = b.FunctionParam("tint_sampler", ty.sampler());
        auto* coords = b.FunctionParam("coords", ty.vec2f());
        texture_sample_clamp_to_edge_multiplanar_external->SetParams(
            {plane_0, plane_1, params, sampler, coords});
        b.Append(texture_sample_clamp_to_edge_multiplanar_external->Block(), [&] {
            auto* yuv_to_rgb_conversion_only = b.Access(ty.u32(), params, 1_u);
            auto* yuv_to_rgb_conversion = b.Access(ty.mat3x4<f32>(), params, 2_u);
            auto* transformation_matrix = b.Access(ty.mat3x2<f32>(), params, 6_u);
            auto* sample_plane0_rect_min = b.Access(ty.vec2f(), params, 8_u);
            auto* sample_plane0_rect_max = b.Access(ty.vec2f(), params, 9_u);
            auto* sample_plane1_rect_min = b.Access(ty.vec2f(), params, 10_u);
            auto* sample_plane1_rect_max = b.Access(ty.vec2f(), params, 11_u);

            auto* modified_coords =
                b.Multiply(transformation_matrix, b.Construct(ty.vec3f(), coords, 1_f));
            auto* loadCoords =
                b.Clamp(modified_coords, sample_plane0_rect_min, sample_plane0_rect_max);

            auto* rgb_result = b.InstructionResult(ty.vec3f());
            auto* alpha_result = b.InstructionResult(ty.f32());
            auto* num_planes = b.Access(ty.u32(), params, 0_u);
            auto* if_planes_eq_1 = b.If(b.Equal(num_planes, 1_u));
            if_planes_eq_1->SetResults(rgb_result, alpha_result);
            b.Append(if_planes_eq_1->True(), [&] {
                // Sample the texel from the first plane and split into separate rgb and a values.
                auto* texel = b.Call(ty.vec4f(), core::BuiltinFn::kTextureSampleLevel, plane_0,
                                     sampler, loadCoords, 0_f);
                auto* rgb = b.Swizzle(ty.vec3f(), texel, {0u, 1u, 2u});
                auto* a = b.Access(ty.f32(), texel, 3_u);
                b.ExitIf(if_planes_eq_1, rgb, a);
            });
            b.Append(if_planes_eq_1->False(), [&] {
                // Sample the y value from the first plane.
                auto* y = b.Access(ty.f32(),
                                   b.Call(ty.vec4f(), core::BuiltinFn::kTextureSampleLevel, plane_0,
                                          sampler, loadCoords, 0_f),
                                   0_u);
                auto* plane1_clamped =
                    b.Clamp(modified_coords, sample_plane1_rect_min, sample_plane1_rect_max);

                // Sample the uv value from the second plane.
                auto* uv = b.Swizzle(ty.vec2f(),
                                     b.Call(ty.vec4f(), core::BuiltinFn::kTextureSampleLevel,
                                            plane_1, sampler, plane1_clamped, 0_f),
                                     {0u, 1u});

                // Convert the combined yuv value into rgb and set the alpha to 1.0.
                b.ExitIf(if_planes_eq_1,
                         b.Multiply(b.Construct(ty.vec4f(), y, uv, 1_f), yuv_to_rgb_conversion),
                         1_f);
            });

            // Apply gamma correction if needed.
            auto* final_result = b.InstructionResult(ty.vec3f());
            auto* if_gamma_correct = b.If(b.Equal(yuv_to_rgb_conversion_only, 0_u));
            if_gamma_correct->SetResult(final_result);
            b.Append(if_gamma_correct->True(), [&] {
                auto* src_transfer_function = b.Access(TransferFunctionParams(), params, 3_u);
                auto* dst_transfer_function = b.Access(TransferFunctionParams(), params, 4_u);
                auto* gamut_conversion_matrix = b.Access(ty.mat3x3<f32>(), params, 5_u);
                auto* decoded = b.Call(ty.vec3f(), ApplySrcTransferFunction(), rgb_result,
                                       src_transfer_function);
                auto* converted = b.Multiply(gamut_conversion_matrix, decoded);
                auto* encoded = b.Call(ty.vec3f(), ApplyDstTransferFunction(), converted,
                                       dst_transfer_function);
                b.ExitIf(if_gamma_correct, encoded);
            });
            b.Append(if_gamma_correct->False(), [&] {  //
                b.ExitIf(if_gamma_correct, rgb_result);
            });

            b.Return(texture_sample_clamp_to_edge_multiplanar_external,
                     b.Construct(ty.vec4f(), final_result, alpha_result));
        });

        return texture_sample_clamp_to_edge_multiplanar_external;
    }

    /// Gets or creates the texture sample helper function.
    /// @returns the function
    Function* TextureSampleClampToEdgeYCBCRExternal() {
        if (texture_sample_clamp_to_edge_ycbcr_external) {
            return texture_sample_clamp_to_edge_ycbcr_external;
        }

        // The helper function implements the following:
        // fn textureSampleClampToEdgeYcbcrExternal(texture : texture_2d<f32>,
        //                          ycbcr_sampler : sampler,
        //                          coord  : vec2f,
        //                          params : ExternalTextureParams) ->vec4f {
        //     let modifiedCoords = params.sampleTransform * vec3<f32>(coord, 1);
        //     let loadCoords =
        //         clamp(modifiedCoords, params.samplePlane0RectMin, params.samplePlane0RectMax);
        //
        //     let val = textureSampleLevel(texture, ycbcr_sampler, loadCoords, 0);
        //     var color = val.rgb * params.yuvToRgbConversionMatrix
        //     let a = val.a;
        //     if ((params.doYuvToRgbConversionOnly == 0)) {
        //         color = applyTransferFunction(color.rgb, params.srcTransferFunction);
        //         color = (params.gamutConversionMatrix * color.rgb);
        //         color = applyTransferFunction(color.rgb, params.dstTransferFunction);
        //     }
        //
        //     return vec4f(color, a);
        // }
        texture_sample_clamp_to_edge_ycbcr_external =
            b.Function("tint_TextureSampleClampToEdgeYcbcrExternal", ty.vec4f());
        auto* texture = b.FunctionParam("texture", SampledTexture());
        auto* ycbcr_sampler = b.FunctionParam("ycbcr_sampler", ty.sampler());
        auto* params = b.FunctionParam("params", ExternalTextureParams());
        auto* coords = b.FunctionParam("coords", ty.vec2f());
        texture_sample_clamp_to_edge_ycbcr_external->SetParams(
            {texture, ycbcr_sampler, params, coords});

        b.Append(texture_sample_clamp_to_edge_ycbcr_external->Block(), [&] {
            auto* yuv_to_rgb_conversion_only = b.Access(ty.u32(), params, 1_u);
            auto* yuv_to_rgb_conversion = b.Access(ty.mat3x4<f32>(), params, 2_u);
            auto* transformation_matrix = b.Access(ty.mat3x2<f32>(), params, 6_u);
            auto* sample_rect_min = b.Access(ty.vec2f(), params, 8_u);
            auto* sample_rect_max = b.Access(ty.vec2f(), params, 9_u);

            auto* modified_coords =
                b.Multiply(transformation_matrix, b.Construct(ty.vec3f(), coords, 1_f));
            auto* loadCoords = b.Clamp(modified_coords, sample_rect_min, sample_rect_max);

            auto* texel = b.Call(ty.vec4f(), core::BuiltinFn::kTextureSampleLevel, texture,
                                 ycbcr_sampler, loadCoords, 0_f);
            auto* rgb = b.Swizzle(ty.vec3f(), texel, {0u, 1u, 2u});
            auto* colour = b.Multiply(b.Construct(ty.vec4f(), rgb, 1_f), yuv_to_rgb_conversion);

            auto* alpha = b.Swizzle(ty.f32(), texel, {3u});

            // Apply gamma correction if needed.
            auto* final_result = b.InstructionResult(ty.vec3f());
            auto* do_yuv_to_rgb = b.If(b.Equal(yuv_to_rgb_conversion_only, 0_u));
            do_yuv_to_rgb->SetResult(final_result);
            b.Append(do_yuv_to_rgb->True(), [&] {
                auto* src_transfer_function = b.Access(TransferFunctionParams(), params, 3_u);
                auto* dst_transfer_function = b.Access(TransferFunctionParams(), params, 4_u);
                auto* gamut_conversion_matrix = b.Access(ty.mat3x3<f32>(), params, 5_u);
                auto* decoded =
                    b.Call(ty.vec3f(), ApplySrcTransferFunction(), colour, src_transfer_function);
                auto* converted = b.Multiply(gamut_conversion_matrix, decoded);
                auto* encoded = b.Call(ty.vec3f(), ApplyDstTransferFunction(), converted,
                                       dst_transfer_function);
                b.ExitIf(do_yuv_to_rgb, encoded);
            });
            b.Append(do_yuv_to_rgb->False(), [&] {  //
                b.ExitIf(do_yuv_to_rgb, colour);
            });

            b.Return(texture_sample_clamp_to_edge_ycbcr_external,
                     b.Construct(ty.vec4f(), final_result, alpha));
        });

        return texture_sample_clamp_to_edge_ycbcr_external;
    }
};

}  // namespace

Result<SuccessType> MultiplanarExternalTexture(
    Module& ir,
    const tint::transform::multiplanar::BindingsMap& multiplanar_map) {
    core::ir::AssertValid(ir, kMultiplanarExternalTextureCapabilities,
                          "before core.MultiplanarExternalTexture");

    return State{multiplanar_map, ir}.Process();
}

}  // namespace tint::core::ir::transform
