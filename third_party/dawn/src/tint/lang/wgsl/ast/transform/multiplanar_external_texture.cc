// Copyright 2021 The Dawn & Tint Authors
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

#include "src/tint/lang/wgsl/ast/transform/multiplanar_external_texture.h"

#include <string>
#include <vector>

#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/wgsl/ast/function.h"
#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"
#include "src/tint/lang/wgsl/sem/call.h"
#include "src/tint/lang/wgsl/sem/function.h"
#include "src/tint/lang/wgsl/sem/variable.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::transform::MultiplanarExternalTexture);
TINT_INSTANTIATE_TYPEINFO(tint::ast::transform::MultiplanarExternalTexture::NewBindingPoints);

namespace tint::ast::transform {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

bool ShouldRun(const Program& program) {
    auto ext = program.Types().Find<core::type::ExternalTexture>();
    return ext != nullptr;
}

/// This struct stores symbols for new bindings created as a result of transforming a
/// texture_external instance.
struct NewBindingSymbols {
    Symbol params;
    Symbol plane_0;
    Symbol plane_1;
};
}  // namespace

/// PIMPL state for the transform
struct MultiplanarExternalTexture::State {
    /// The clone context.
    program::CloneContext& ctx;

    /// Alias to `*ctx.dst`
    ast::Builder& b;

    /// Destination binding locations for the expanded texture_external provided
    /// as input into the transform.
    const NewBindingPoints* new_binding_points;

    /// Symbol for the GammaTransferParams
    Symbol gamma_transfer_struct_sym;

    /// Symbol for the ExternalTextureParams struct
    Symbol params_struct_sym;

    /// Symbol for the textureLoadExternal functions
    Hashmap<const sem::CallTarget*, Symbol, 2> texture_load_external_fns;

    /// Symbol for the textureSampleExternal function
    Symbol texture_sample_external_sym;

    /// Symbol for the textureSampleExternalDEPRECATED function
    Symbol texture_sample_external_deprecated_sym;

    /// Symbol for the gammaCorrection function
    Symbol gamma_correction_sym;

    /// Storage for new bindings that have been created corresponding to an original
    /// texture_external binding.
    std::unordered_map<const sem::Variable*, NewBindingSymbols> new_binding_symbols;

    /// Constructor
    /// @param context the clone
    /// @param newBindingPoints the input destination binding locations for the
    /// expanded texture_external
    State(program::CloneContext& context, const NewBindingPoints* newBindingPoints)
        : ctx(context), b(*context.dst), new_binding_points(newBindingPoints) {}

    /// Processes the module
    void Process() {
        auto& sem = ctx.src->Sem();

        // For each texture_external binding, we replace it with a texture_2d<f32> binding and
        // create two additional bindings (one texture_2d<f32> to represent the secondary plane and
        // one uniform buffer for the ExternalTextureParams struct).
        for (auto* global : ctx.src->AST().GlobalVariables()) {
            auto* sem_var = sem.Get<sem::GlobalVariable>(global);
            if (!sem_var->Type()->UnwrapRef()->Is<core::type::ExternalTexture>()) {
                continue;
            }

            // If the attributes are empty, then this must be a texture_external passed as a
            // function parameter. These variables are transformed elsewhere.
            if (global->attributes.IsEmpty()) {
                continue;
            }

            // If we find a texture_external binding, we know we must emit the ExternalTextureParams
            // struct.
            if (!params_struct_sym.IsValid()) {
                createExtTexParamsStructs();
            }

            // The binding points for the newly introduced bindings must have been provided to this
            // transform. We fetch the new binding points by providing the original texture_external
            // binding points into the passed map.
            BindingPoint bp = *sem_var->Attributes().binding_point;

            const tint::transform::multiplanar::BindingsMap::const_iterator it =
                new_binding_points->bindings_map.find(bp);
            if (it == new_binding_points->bindings_map.end()) {
                b.Diagnostics().AddError(Source{})
                    << "missing new binding points for texture_external at binding {" << bp.group
                    << "," << bp.binding << "}";
                continue;
            }

            const tint::transform::multiplanar::BindingPoints bps = it->second;

            // Symbols for the newly created bindings must be saved so they can be passed as
            // parameters later. These are placed in a map and keyed by the source symbol associated
            // with the texture_external binding that corresponds with the new destination bindings.
            // NewBindingSymbols new_binding_syms;
            auto& syms = new_binding_symbols[sem_var];
            syms.plane_0 = ctx.Clone(global->name->symbol);
            syms.plane_1 = b.Symbols().New("ext_tex_plane_1");
            if (new_binding_points->allow_collisions) {
                b.GlobalVar(syms.plane_1,
                            b.ty.sampled_texture(core::type::TextureDimension::k2d, b.ty.f32()),
                            b.Disable(DisabledValidation::kBindingPointCollision),
                            b.Group(AInt(bps.plane_1.group)), b.Binding(AInt(bps.plane_1.binding)));
            } else {
                b.GlobalVar(syms.plane_1,
                            b.ty.sampled_texture(core::type::TextureDimension::k2d, b.ty.f32()),
                            b.Group(AInt(bps.plane_1.group)), b.Binding(AInt(bps.plane_1.binding)));
            }
            syms.params = b.Symbols().New("ext_tex_params");
            if (new_binding_points->allow_collisions) {
                b.GlobalVar(syms.params, b.ty("ExternalTextureParams"),
                            core::AddressSpace::kUniform,
                            b.Disable(DisabledValidation::kBindingPointCollision),
                            b.Group(AInt(bps.params.group)), b.Binding(AInt(bps.params.binding)));
            } else {
                b.GlobalVar(syms.params, b.ty("ExternalTextureParams"),
                            core::AddressSpace::kUniform, b.Group(AInt(bps.params.group)),
                            b.Binding(AInt(bps.params.binding)));
            }

            // Replace the original texture_external binding with a texture_2d<f32> binding.
            auto cloned_attributes = ctx.Clone(global->attributes);

            // Allow the originating binding to have collisions.
            if (new_binding_points->allow_collisions) {
                cloned_attributes.Push(b.Disable(DisabledValidation::kBindingPointCollision));
            }

            const Expression* cloned_initializer = ctx.Clone(global->initializer);

            auto* replacement = b.Var(
                syms.plane_0, b.ty.sampled_texture(core::type::TextureDimension::k2d, b.ty.f32()),
                cloned_initializer, cloned_attributes);
            ctx.Replace(global, replacement);
        }

        // We must update all the texture_external parameters for user declared functions.
        for (auto* fn : ctx.src->AST().Functions()) {
            for (const Variable* param : fn->params) {
                if (auto* sem_var = sem.Get(param)) {
                    if (!sem_var->Type()->UnwrapRef()->Is<core::type::ExternalTexture>()) {
                        continue;
                    }
                    // If we find a texture_external, we must ensure the ExternalTextureParams
                    // struct exists.
                    if (!params_struct_sym.IsValid()) {
                        createExtTexParamsStructs();
                    }
                    // When a texture_external is found, we insert all components the
                    // texture_external into the parameter list. We must also place the new symbols
                    // into the transform state so they can be used when transforming function
                    // calls.
                    auto& syms = new_binding_symbols[sem_var];
                    syms.plane_0 = ctx.Clone(param->name->symbol);
                    syms.plane_1 = b.Symbols().New("ext_tex_plane_1");
                    syms.params = b.Symbols().New("ext_tex_params");
                    auto tex2d_f32 = [&] {
                        return b.ty.sampled_texture(core::type::TextureDimension::k2d, b.ty.f32());
                    };
                    ctx.Replace(param, b.Param(syms.plane_0, tex2d_f32()));
                    ctx.InsertAfter(fn->params, param, b.Param(syms.plane_1, tex2d_f32()));
                    ctx.InsertAfter(fn->params, param,
                                    b.Param(syms.params, b.ty(params_struct_sym)));
                }
            }
        }

        // Transform the external texture builtin calls into calls to the external texture
        // functions.
        ctx.ReplaceAll([&](const CallExpression* expr) -> const Expression* {
            auto* call = sem.Get(expr)->UnwrapMaterialize()->As<sem::Call>();
            auto* builtin = call->Target()->As<sem::BuiltinFn>();

            if (builtin && !builtin->Parameters().IsEmpty() &&
                builtin->Parameters()[0]->Type()->Is<core::type::ExternalTexture>()) {
                if (auto* var_user =
                        sem.GetVal(expr->args[0])->UnwrapLoad()->As<sem::VariableUser>()) {
                    auto it = new_binding_symbols.find(var_user->Variable());
                    if (it == new_binding_symbols.end()) {
                        // If valid new binding locations were not provided earlier, we would have
                        // been unable to create these symbols. An error message was emitted
                        // earlier, so just return early to avoid internal compiler errors and
                        // retain a clean error message.
                        return nullptr;
                    }
                    auto& syms = it->second;

                    switch (builtin->Fn()) {
                        case wgsl::BuiltinFn::kTextureLoad:
                            return createTextureLoad(call, syms);
                        case wgsl::BuiltinFn::kTextureSampleBaseClampToEdge:
                            return createTextureSampleBaseClampToEdge(expr, syms);
                        case wgsl::BuiltinFn::kTextureDimensions:
                            return createTextureDimensions(call, syms);
                        default:
                            break;
                    }
                }
            } else if (call->Target()->Is<sem::Function>()) {
                // The call expression may be to a user-defined function that contains a
                // texture_external parameter. These need to be expanded out to multiple plane
                // textures and the texture parameters structure.
                for (auto* arg : expr->args) {
                    if (auto* var_user = sem.GetVal(arg)->UnwrapLoad()->As<sem::VariableUser>()) {
                        // Check if a parameter is a texture_external by trying to find
                        // it in the transform state.
                        auto it = new_binding_symbols.find(var_user->Variable());
                        if (it != new_binding_symbols.end()) {
                            auto& syms = it->second;
                            // When we find a texture_external, we must unpack it into its
                            // components.
                            ctx.Replace(arg, b.Expr(syms.plane_0));
                            ctx.InsertAfter(expr->args, arg, b.Expr(syms.plane_1));
                            ctx.InsertAfter(expr->args, arg, b.Expr(syms.params));
                        }
                    }
                }
            }

            return nullptr;
        });
    }

    /// Creates the parameter structs associated with the transform.
    void createExtTexParamsStructs() {
        // Create GammaTransferParams struct.
        tint::Vector gamma_transfer_member_list{
            b.Member("G", b.ty.f32()), b.Member("A", b.ty.f32()),      b.Member("B", b.ty.f32()),
            b.Member("C", b.ty.f32()), b.Member("D", b.ty.f32()),      b.Member("E", b.ty.f32()),
            b.Member("F", b.ty.f32()), b.Member("padding", b.ty.u32())};

        gamma_transfer_struct_sym = b.Symbols().New("GammaTransferParams");

        b.Structure(gamma_transfer_struct_sym, gamma_transfer_member_list);

        // Create ExternalTextureParams struct.
        tint::Vector ext_tex_params_member_list{
            b.Member("numPlanes", b.ty.u32()),
            b.Member("doYuvToRgbConversionOnly", b.ty.u32()),
            b.Member("yuvToRgbConversionMatrix", b.ty.mat3x4<f32>()),
            b.Member("gammaDecodeParams", b.ty("GammaTransferParams")),
            b.Member("gammaEncodeParams", b.ty("GammaTransferParams")),
            b.Member("gamutConversionMatrix", b.ty.mat3x3<f32>()),
            b.Member("sampleTransform", b.ty.mat3x2<f32>()),
            b.Member("loadTransform", b.ty.mat3x2<f32>()),
            b.Member("samplePlane0RectMin", b.ty.vec2<f32>()),
            b.Member("samplePlane0RectMax", b.ty.vec2<f32>()),
            b.Member("samplePlane1RectMin", b.ty.vec2<f32>()),
            b.Member("samplePlane1RectMax", b.ty.vec2<f32>()),
            b.Member("apparentSize", b.ty.vec2<u32>()),
            b.Member("plane1CoordFactor", b.ty.vec2<f32>())};

        params_struct_sym = b.Symbols().New("ExternalTextureParams");

        b.Structure(params_struct_sym, ext_tex_params_member_list);
    }

    /// Creates the gammaCorrection function if needed and returns a call
    /// expression to it.
    void createGammaCorrectionFn() {
        gamma_correction_sym = b.Symbols().New("gammaCorrection");

        b.Func(gamma_correction_sym,
               tint::Vector{
                   b.Param("v", b.ty.vec3<f32>()),
                   b.Param("params", b.ty(gamma_transfer_struct_sym)),
               },
               b.ty.vec3<f32>(),
               tint::Vector{
                   // let cond = abs(v) < vec3(params.D);
                   b.Decl(b.Let("cond",
                                b.LessThan(b.Call("abs", "v"),
                                           b.Call<vec3<f32>>(b.MemberAccessor("params", "D"))))),
                   // let t = sign(v) * ((params.C * abs(v)) + params.F);
                   b.Decl(b.Let(
                       "t", b.Mul(b.Call("sign", "v"),
                                  b.Add(b.Mul(b.MemberAccessor("params", "C"), b.Call("abs", "v")),
                                        b.MemberAccessor("params", "F"))))),
                   // let f = (sign(v) * pow(((params.A * abs(v)) + params.B),
                   // vec3(params.G))) + params.E;
                   b.Decl(b.Let(
                       "f", b.Mul(b.Call("sign", "v"),
                                  b.Add(b.Call("pow",
                                               b.Add(b.Mul(b.MemberAccessor("params", "A"),
                                                           b.Call("abs", "v")),
                                                     b.MemberAccessor("params", "B")),
                                               b.Call<vec3<f32>>(b.MemberAccessor("params", "G"))),
                                        b.MemberAccessor("params", "E"))))),
                   // return select(f, t, cond);
                   b.Return(b.Call("select", "f", "t", "cond")),
               });
    }

    /// Constructs a StatementList containing all the statements making up the body of the texture
    /// builtin function.
    /// @param call_type determines which function body to generate
    /// @returns a statement list that makes of the body of the chosen function
    auto buildTextureBuiltinBody(wgsl::BuiltinFn call_type) {
        tint::Vector<const Statement*, 16> stmts;
        const BlockStatement* single_plane_block = nullptr;
        const BlockStatement* multi_plane_block = nullptr;
        switch (call_type) {
            case wgsl::BuiltinFn::kTextureSampleBaseClampToEdge:
                stmts.Push(b.Decl(
                    b.Let("modifiedCoords", b.Mul(b.MemberAccessor("params", "sampleTransform"),
                                                  b.Call<vec3<f32>>("coord", 1_a)))));

                stmts.Push(b.Decl(b.Let(
                    "plane0_clamped", b.Call("clamp", "modifiedCoords",
                                             b.MemberAccessor("params", "samplePlane0RectMin"),
                                             b.MemberAccessor("params", "samplePlane0RectMax")))));

                // var color: vec4<f32>;
                stmts.Push(b.Decl(b.Var("color", b.ty.vec4(b.ty.f32()))));

                single_plane_block = b.Block(
                    b.Assign("color", b.MemberAccessor(b.Call("textureSampleLevel", "plane0", "smp",
                                                              "plane0_clamped", 0_a),
                                                       "rgba")));

                multi_plane_block = b.Block(
                    b.Decl(b.Let("plane1_clamped",
                                 b.Call("clamp", "modifiedCoords",
                                        b.MemberAccessor("params", "samplePlane1RectMin"),
                                        b.MemberAccessor("params", "samplePlane1RectMax")))),

                    b.Assign("color",
                             b.Call<vec4<f32>>(
                                 b.Mul(b.Call<vec4<f32>>(
                                           b.MemberAccessor(b.Call("textureSampleLevel", "plane0",
                                                                   "smp", "plane0_clamped", 0_a),
                                                            "r"),
                                           b.MemberAccessor(b.Call("textureSampleLevel", "plane1",
                                                                   "smp", "plane1_clamped", 0_a),
                                                            "rg"),
                                           1_a),
                                       b.MemberAccessor("params", "yuvToRgbConversionMatrix")),
                                 1_a)));
                break;
            case wgsl::BuiltinFn::kTextureLoad:
                stmts.Push(b.Decl(
                    b.Let("clampedCoords", b.Call("min", b.Call<vec2<u32>>("coord"),
                                                  b.MemberAccessor("params", "apparentSize")))));
                stmts.Push(b.Decl(b.Let(
                    "plane0_clamped",
                    b.Call<vec2<u32>>(b.Call(
                        "round",
                        b.Mul(b.MemberAccessor("params", "loadTransform"),
                              b.Call<vec3<f32>>(b.Call<vec2<f32>>("clampedCoords"), 1_a)))))));

                // var color: vec4<f32>;
                stmts.Push(b.Decl(b.Var("color", b.ty.vec4(b.ty.f32()))));

                single_plane_block = b.Block(b.Assign(
                    "color", b.MemberAccessor(
                                 b.Call("textureLoad", "plane0", "plane0_clamped", 0_a), "rgba")));

                multi_plane_block = b.Block(
                    b.Decl(b.Let(
                        "plane1_clamped",
                        b.Call<vec2<u32>>(b.Mul(b.Call<vec2<f32>>("plane0_clamped"),
                                                b.MemberAccessor("params", "plane1CoordFactor"))))),

                    b.Assign("color",
                             b.Call<vec4<f32>>(
                                 b.Mul(b.Call<vec4<f32>>(
                                           b.MemberAccessor(b.Call("textureLoad", "plane0",
                                                                   "plane0_clamped", 0_a),
                                                            "r"),
                                           b.MemberAccessor(b.Call("textureLoad", "plane1",
                                                                   "plane1_clamped", 0_a),
                                                            "rg"),
                                           1_a),
                                       b.MemberAccessor("params", "yuvToRgbConversionMatrix")),
                                 1_a)));
                break;
            default:
                TINT_ICE() << "unhandled builtin: " << call_type;
        }

        // if ((params.numPlanes == 1u))
        stmts.Push(b.If(b.Equal(b.MemberAccessor("params", "numPlanes"), b.Expr(1_a)),
                        single_plane_block, b.Else(multi_plane_block)));

        // if (params.doYuvToRgbConversionOnly == 0u)
        stmts.Push(b.If(
            b.Equal(b.MemberAccessor("params", "doYuvToRgbConversionOnly"), b.Expr(0_a)),
            b.Block(
                // color = vec4<f32>(gammaConversion(color.rgb, gammaDecodeParams), color.a);
                b.Assign("color", b.Call<vec4<f32>>(
                                      b.Call("gammaCorrection", b.MemberAccessor("color", "rgb"),
                                             b.MemberAccessor("params", "gammaDecodeParams")),
                                      b.MemberAccessor("color", "a"))),
                // color = vec4<f32>(params.gamutConversionMatrix * color.rgb), color.a);
                b.Assign("color", b.Call<vec4<f32>>(
                                      b.Mul(b.MemberAccessor("params", "gamutConversionMatrix"),
                                            b.MemberAccessor("color", "rgb")),
                                      b.MemberAccessor("color", "a"))),
                // color = vec4<f32>(gammaConversion(color.rgb, gammaEncodeParams), color.a);
                b.Assign("color", b.Call<vec4<f32>>(
                                      b.Call("gammaCorrection", b.MemberAccessor("color", "rgb"),
                                             b.MemberAccessor("params", "gammaEncodeParams")),
                                      b.MemberAccessor("color", "a"))))));

        // return color;
        stmts.Push(b.Return("color"));

        return stmts;
    }

    /// Creates the textureSampleExternal function if needed and returns a call expression to it.
    /// @param expr the call expression being transformed
    /// @param syms the expanded symbols to be used in the new call
    /// @returns a call expression to textureSampleExternal
    const CallExpression* createTextureSampleBaseClampToEdge(const CallExpression* expr,
                                                             NewBindingSymbols syms) {
        const Expression* plane_0_binding_param = ctx.Clone(expr->args[0]);

        if (DAWN_UNLIKELY(expr->args.Length() != 3)) {
            TINT_ICE() << "expected textureSampleBaseClampToEdge call with a "
                          "texture_external to have 3 parameters, found "
                       << expr->args.Length() << " parameters";
        }

        // TextureSampleExternal calls the gammaCorrection function, so ensure it
        // exists.
        if (!gamma_correction_sym.IsValid()) {
            createGammaCorrectionFn();
        }

        if (!texture_sample_external_sym.IsValid()) {
            texture_sample_external_sym = b.Symbols().New("textureSampleExternal");

            // Emit the textureSampleExternal function.
            b.Func(texture_sample_external_sym,
                   tint::Vector{
                       b.Param("plane0",
                               b.ty.sampled_texture(core::type::TextureDimension::k2d, b.ty.f32())),
                       b.Param("plane1",
                               b.ty.sampled_texture(core::type::TextureDimension::k2d, b.ty.f32())),
                       b.Param("smp", b.ty.sampler(core::type::SamplerKind::kSampler)),
                       b.Param("coord", b.ty.vec2(b.ty.f32())),
                       b.Param("params", b.ty(params_struct_sym)),
                   },
                   b.ty.vec4(b.ty.f32()),
                   buildTextureBuiltinBody(wgsl::BuiltinFn::kTextureSampleBaseClampToEdge));
        }

        return b.Call(texture_sample_external_sym, tint::Vector{
                                                       plane_0_binding_param,
                                                       b.Expr(syms.plane_1),
                                                       ctx.Clone(expr->args[1]),
                                                       ctx.Clone(expr->args[2]),
                                                       b.Expr(syms.params),
                                                   });
    }

    /// Creates the textureLoadExternal function if needed and returns a call expression to it.
    /// @param call the call expression being transformed
    /// @param syms the expanded symbols to be used in the new call
    /// @returns a call expression to textureLoadExternal
    const CallExpression* createTextureLoad(const sem::Call* call, NewBindingSymbols syms) {
        if (DAWN_UNLIKELY(call->Arguments().Length() != 2)) {
            TINT_ICE()
                << "expected textureLoad call with a texture_external to have 2 arguments, found "
                << call->Arguments().Length() << " arguments";
        }

        auto& args = call->Arguments();

        // TextureLoadExternal calls the gammaCorrection function, so ensure it exists.
        if (!gamma_correction_sym.IsValid()) {
            createGammaCorrectionFn();
        }

        auto texture_load_external_sym = texture_load_external_fns.GetOrAdd(call->Target(), [&] {
            auto& sig = call->Target()->Signature();
            auto* coord_ty = sig.Parameter(core::ParameterUsage::kCoords)->Type();

            auto name = b.Symbols().New("textureLoadExternal");

            // Emit the textureLoadExternal() function.
            b.Func(name,
                   tint::Vector{
                       b.Param("plane0",
                               b.ty.sampled_texture(core::type::TextureDimension::k2d, b.ty.f32())),
                       b.Param("plane1",
                               b.ty.sampled_texture(core::type::TextureDimension::k2d, b.ty.f32())),
                       b.Param("coord", CreateASTTypeFor(ctx, coord_ty)),
                       b.Param("params", b.ty(params_struct_sym)),
                   },
                   b.ty.vec4(b.ty.f32()),  //
                   buildTextureBuiltinBody(wgsl::BuiltinFn::kTextureLoad));

            return name;
        });

        auto plane_0_binding_arg = ctx.Clone(args[0]->Declaration());

        return b.Call(texture_load_external_sym, plane_0_binding_arg, syms.plane_1,
                      ctx.Clone(args[1]->Declaration()), syms.params);
    }

    /// Returns the expression used to replace a textureDimensions call.
    /// @param call the call expression being transformed
    /// @param syms the expanded symbols to be used in the new call
    /// @returns a load of params.apparentSize
    const Expression* createTextureDimensions(const sem::Call* call, NewBindingSymbols syms) {
        if (DAWN_UNLIKELY(call->Arguments().Length() != 1)) {
            TINT_ICE() << "expected textureDimensions call with a texture_external to have 1 "
                          "arguments, found "
                       << call->Arguments().Length() << " arguments";
        }
        return b.Add(b.MemberAccessor(syms.params, "apparentSize"), b.Call<vec2<u32>>(1_a));
    }
};

MultiplanarExternalTexture::NewBindingPoints::NewBindingPoints() = default;
MultiplanarExternalTexture::NewBindingPoints::NewBindingPoints(
    tint::transform::multiplanar::BindingsMap inputBindingsMap,
    bool may_collide)
    : bindings_map(std::move(inputBindingsMap)), allow_collisions(may_collide) {}

MultiplanarExternalTexture::NewBindingPoints::~NewBindingPoints() = default;

MultiplanarExternalTexture::MultiplanarExternalTexture() = default;
MultiplanarExternalTexture::~MultiplanarExternalTexture() = default;

// Within this transform, an instance of a texture_external binding is unpacked into two
// texture_2d<f32> bindings representing two possible planes of a single texture and a uniform
// buffer binding representing a struct of parameters. Calls to texture builtins that contain a
// texture_external parameter will be transformed into a newly generated version of the function,
// which can perform the desired operation on a single RGBA plane or on separate Y and UV planes.
Transform::ApplyResult MultiplanarExternalTexture::Apply(const Program& src,
                                                         const DataMap& inputs,
                                                         DataMap&) const {
    auto* new_binding_points = inputs.Get<NewBindingPoints>();

    if (!ShouldRun(src)) {
        return SkipTransform;
    }

    ProgramBuilder b;
    program::CloneContext ctx{&b, &src, /* auto_clone_symbols */ true};
    if (!new_binding_points) {
        b.Diagnostics().AddError(Source{})
            << "missing new binding point data for " << TypeInfo().name;
        return resolver::Resolve(b);
    }

    State state(ctx, new_binding_points);

    state.Process();

    ctx.Clone();
    return resolver::Resolve(b);
}

}  // namespace tint::ast::transform
