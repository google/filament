// Copyright 2024 The Dawn & Tint Authors
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

#include "src/tint/lang/spirv/reader/lower/builtins.h"

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/builtin_structs.h"
#include "src/tint/lang/spirv/ir/builtin_call.h"

namespace tint::spirv::reader::lower {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    core::ir::Module& ir;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// Process the module.
    void Process() {
        Vector<spirv::ir::BuiltinCall*, 4> builtin_worklist;
        for (auto* inst : ir.Instructions()) {
            if (auto* builtin = inst->As<spirv::ir::BuiltinCall>()) {
                builtin_worklist.Push(builtin);
            }
        }

        // Replace the builtins that we found.
        for (auto* builtin : builtin_worklist) {
            switch (builtin->Func()) {
                case spirv::BuiltinFn::kNormalize:
                    Normalize(builtin);
                    break;
                case spirv::BuiltinFn::kInverse:
                    Inverse(builtin);
                    break;
                case spirv::BuiltinFn::kSign:
                    Sign(builtin);
                    break;
                case spirv::BuiltinFn::kAbs:
                    Abs(builtin);
                    break;
                case spirv::BuiltinFn::kSmax:
                    SMax(builtin);
                    break;
                case spirv::BuiltinFn::kSmin:
                    SMin(builtin);
                    break;
                case spirv::BuiltinFn::kSclamp:
                    SClamp(builtin);
                    break;
                case spirv::BuiltinFn::kUmax:
                    UMax(builtin);
                    break;
                case spirv::BuiltinFn::kUmin:
                    UMin(builtin);
                    break;
                case spirv::BuiltinFn::kUclamp:
                    UClamp(builtin);
                    break;
                case spirv::BuiltinFn::kFindILsb:
                    FindILsb(builtin);
                    break;
                case spirv::BuiltinFn::kFindSMsb:
                    FindSMsb(builtin);
                    break;
                case spirv::BuiltinFn::kFindUMsb:
                    FindUMsb(builtin);
                    break;
                case spirv::BuiltinFn::kRefract:
                    Refract(builtin);
                    break;
                case spirv::BuiltinFn::kReflect:
                    Reflect(builtin);
                    break;
                case spirv::BuiltinFn::kFaceForward:
                    FaceForward(builtin);
                    break;
                case spirv::BuiltinFn::kLdexp:
                    Ldexp(builtin);
                    break;
                case spirv::BuiltinFn::kModf:
                    Modf(builtin);
                    break;
                case spirv::BuiltinFn::kFrexp:
                    Frexp(builtin);
                    break;
                default:
                    TINT_UNREACHABLE() << "unknown spirv builtin: " << builtin->Func();
            }
        }
    }

    // The SPIR-V Signed methods all interpret their arguments as signed (regardless of the type of
    // the argument). In order to satisfy this, we must bitcast any unsigned argument to a signed
    // type before calling the WGSL equivalent method.
    //
    // The result of the WGSL method will match the arguments, or in this case a signed value. If
    // the SPIR-V instruction expected an unsigned result we must bitcast the WGSL result to the
    // corrrect unsigned type.
    void WrapSignedSpirvMethods(spirv::ir::BuiltinCall* call, core::BuiltinFn func) {
        auto args = call->Args();

        b.InsertBefore(call, [&] {
            auto* result_ty = call->Result(0)->Type();
            Vector<core::ir::Value*, 2> new_args;

            for (auto* arg : args) {
                if (arg->Type()->IsUnsignedIntegerScalarOrVector()) {
                    arg = b.Bitcast(ty.MatchWidth(ty.i32(), result_ty), arg)->Result(0);
                }
                new_args.Push(arg);
            }

            auto* new_call = b.Call(result_ty, func, new_args);

            core::ir::Value* replacement = new_call->Result(0);
            if (result_ty->DeepestElement() == ty.u32()) {
                new_call->Result(0)->SetType(ty.MatchWidth(ty.i32(), result_ty));
                replacement = b.Bitcast(result_ty, replacement)->Result(0);
            }
            call->Result(0)->ReplaceAllUsesWith(replacement);
        });
        call->Destroy();
    }

    void Sign(spirv::ir::BuiltinCall* call) {
        WrapSignedSpirvMethods(call, core::BuiltinFn::kSign);
    }
    void Abs(spirv::ir::BuiltinCall* call) { WrapSignedSpirvMethods(call, core::BuiltinFn::kAbs); }
    void FindSMsb(spirv::ir::BuiltinCall* call) {
        WrapSignedSpirvMethods(call, core::BuiltinFn::kFirstLeadingBit);
    }
    void SMax(spirv::ir::BuiltinCall* call) { WrapSignedSpirvMethods(call, core::BuiltinFn::kMax); }
    void SMin(spirv::ir::BuiltinCall* call) { WrapSignedSpirvMethods(call, core::BuiltinFn::kMin); }
    void SClamp(spirv::ir::BuiltinCall* call) {
        WrapSignedSpirvMethods(call, core::BuiltinFn::kClamp);
    }

    void Ldexp(spirv::ir::BuiltinCall* call) {
        WrapSignedSpirvMethods(call, core::BuiltinFn::kLdexp);
    }

    // The SPIR-V Unsigned methods all interpret their arguments as unsigned (regardless of the type
    // of the argument). In order to satisfy this, we must bitcast any signed argument to an
    // unsigned type before calling the WGSL equivalent method.
    //
    // The result of the WGSL method will match the arguments, or in this case an unsigned value. If
    // the SPIR-V instruction expected a signed result we must bitcast the WGSL result to the
    // correct signed type.
    void WrapUnsignedSpirvMethods(spirv::ir::BuiltinCall* call, core::BuiltinFn func) {
        auto args = call->Args();

        b.InsertBefore(call, [&] {
            auto* result_ty = call->Result(0)->Type();
            Vector<core::ir::Value*, 2> new_args;

            for (auto* arg : args) {
                if (arg->Type()->IsSignedIntegerScalarOrVector()) {
                    arg = b.Bitcast(ty.MatchWidth(ty.u32(), result_ty), arg)->Result(0);
                }
                new_args.Push(arg);
            }

            auto* new_call = b.Call(result_ty, func, new_args);

            core::ir::Value* replacement = new_call->Result(0);
            if (result_ty->DeepestElement() == ty.i32()) {
                new_call->Result(0)->SetType(ty.MatchWidth(ty.u32(), result_ty));
                replacement = b.Bitcast(result_ty, replacement)->Result(0);
            }
            call->Result(0)->ReplaceAllUsesWith(replacement);
        });
        call->Destroy();
    }

    void UMax(spirv::ir::BuiltinCall* call) {
        WrapUnsignedSpirvMethods(call, core::BuiltinFn::kMax);
    }
    void UMin(spirv::ir::BuiltinCall* call) {
        WrapUnsignedSpirvMethods(call, core::BuiltinFn::kMin);
    }
    void UClamp(spirv::ir::BuiltinCall* call) {
        WrapUnsignedSpirvMethods(call, core::BuiltinFn::kClamp);
    }
    void FindUMsb(spirv::ir::BuiltinCall* call) {
        WrapUnsignedSpirvMethods(call, core::BuiltinFn::kFirstLeadingBit);
    }

    void Normalize(spirv::ir::BuiltinCall* call) {
        auto* arg = call->Args()[0];

        b.InsertBefore(call, [&] {
            core::BuiltinFn fn = core::BuiltinFn::kNormalize;
            if (arg->Type()->IsScalar()) {
                fn = core::BuiltinFn::kSign;
            }
            b.CallWithResult(call->DetachResult(), fn, Vector<core::ir::Value*, 1>{arg});
        });
        call->Destroy();
    }

    void FindILsb(spirv::ir::BuiltinCall* call) {
        auto* arg = call->Args()[0];

        b.InsertBefore(call, [&] {
            auto* arg_ty = arg->Type();
            auto* ret_ty = call->Result(0)->Type();

            auto* v =
                b.Call(arg_ty, core::BuiltinFn::kFirstTrailingBit, Vector<core::ir::Value*, 1>{arg})
                    ->Result(0);
            if (arg_ty != ret_ty) {
                v = b.Bitcast(ret_ty, v)->Result(0);
            }
            call->Result(0)->ReplaceAllUsesWith(v);
        });
        call->Destroy();
    }

    void Refract(spirv::ir::BuiltinCall* call) {
        auto args = call->Args();

        auto* I = args[0];
        auto* N = args[1];
        auto* eta = args[2];

        b.InsertBefore(call, [&] {
            if (I->Type()->IsFloatScalar()) {
                auto* src_ty = I->Type();
                auto* vec_ty = ty.vec(src_ty, 2);
                auto* zero = b.Zero(src_ty);
                I = b.Construct(vec_ty, I, zero)->Result(0);
                N = b.Construct(vec_ty, N, zero)->Result(0);

                auto* c = b.Call(vec_ty, core::BuiltinFn::kRefract,
                                 Vector<core::ir::Value*, 3>{I, N, eta});
                auto* s = b.Swizzle(src_ty, c, {0});
                call->Result(0)->ReplaceAllUsesWith(s->Result(0));
            } else {
                b.CallWithResult(call->DetachResult(), core::BuiltinFn::kRefract,
                                 Vector<core::ir::Value*, 3>{I, N, eta});
            }
        });
        call->Destroy();
    }

    void Reflect(spirv::ir::BuiltinCall* call) {
        auto args = call->Args();

        auto* I = args[0];
        auto* N = args[1];

        b.InsertBefore(call, [&] {
            if (I->Type()->IsFloatScalar()) {
                auto* v = b.Multiply(I->Type(), I, N)->Result(0);
                v = b.Multiply(I->Type(), v, N)->Result(0);
                v = b.Multiply(I->Type(), v, 2.0_f)->Result(0);
                v = b.Subtract(I->Type(), I, v)->Result(0);
                call->Result(0)->ReplaceAllUsesWith(v);
            } else {
                b.CallWithResult(call->DetachResult(), core::BuiltinFn::kReflect,
                                 Vector<core::ir::Value*, 2>{I, N});
            }
        });
        call->Destroy();
    }

    void FaceForward(spirv::ir::BuiltinCall* call) {
        auto args = call->Args();
        auto* N = args[0];
        auto* I = args[1];
        auto* Nref = args[2];

        b.InsertBefore(call, [&] {
            if (I->Type()->IsFloatScalar()) {
                auto* neg = b.Negation(N->Type(), N);
                auto* sel = b.Multiply(I->Type(), I, Nref)->Result(0);
                sel = b.LessThan(ty.bool_(), sel, b.Zero(sel->Type()))->Result(0);
                b.CallWithResult(call->DetachResult(), core::BuiltinFn::kSelect, neg, N, sel);
            } else {
                b.CallWithResult(call->DetachResult(), core::BuiltinFn::kFaceForward, N, I, Nref);
            }
        });
        call->Destroy();
    }

    void Modf(spirv::ir::BuiltinCall* call) {
        auto* x = call->Args()[0];
        auto* i = call->Args()[1];
        auto* result_ty = call->Result(0)->Type();
        auto* modf_result_ty = core::type::CreateModfResult(ty, ir.symbols, result_ty);

        b.InsertBefore(call, [&] {
            auto* c = b.Call(modf_result_ty, core::BuiltinFn::kModf, x);
            auto* whole = b.Access(result_ty, c, 1_u);
            b.Store(i, whole);

            b.AccessWithResult(call->DetachResult(), c, 0_u);
        });
        call->Destroy();
    }

    void Frexp(spirv::ir::BuiltinCall* call) {
        auto* x = call->Args()[0];
        auto* i = call->Args()[1];
        auto* result_ty = call->Result(0)->Type();
        auto* frexp_result_ty = core::type::CreateFrexpResult(ty, ir.symbols, result_ty);

        b.InsertBefore(call, [&] {
            auto* c = b.Call(frexp_result_ty, core::BuiltinFn::kFrexp, x);
            auto* exp = b.Access(ty.MatchWidth(ty.i32(), result_ty), c, 1_u)->Result(0);

            if (i->Type()->UnwrapPtr()->DeepestElement()->IsUnsignedIntegerScalar()) {
                exp = b.Bitcast(i->Type()->UnwrapPtr(), exp)->Result(0);
            }
            b.Store(i, exp);

            b.AccessWithResult(call->DetachResult(), c, 0_u);
        });
        call->Destroy();
    }

    void Inverse(spirv::ir::BuiltinCall* call) {
        auto* arg = call->Args()[0];
        auto* mat_ty = arg->Type()->As<core::type::Matrix>();
        TINT_ASSERT(mat_ty);
        TINT_ASSERT(mat_ty->Columns() == mat_ty->Rows());

        auto* elem_ty = mat_ty->Type();

        b.InsertBefore(call, [&] {
            auto* det =
                b.Call(elem_ty, core::BuiltinFn::kDeterminant, Vector<core::ir::Value*, 1>{arg});
            core::ir::Value* one = nullptr;
            if (elem_ty->Is<core::type::F32>()) {
                one = b.Constant(1.0_f);
            } else if (elem_ty->Is<core::type::F16>()) {
                one = b.Constant(1.0_h);
            } else {
                TINT_UNREACHABLE();
            }
            auto* inv_det = b.Divide(elem_ty, one, det);

            // Returns (m * n) - (o * p)
            auto sub_mul2 = [&](auto* m, auto* n, auto* o, auto* p) {
                auto* x = b.Multiply(elem_ty, m, n);
                auto* y = b.Multiply(elem_ty, o, p);
                return b.Subtract(elem_ty, x, y);
            };

            // Returns (m * n) - (o * p) + (q * r)
            auto sub_add_mul3 = [&](auto* m, auto* n, auto* o, auto* p, auto* q, auto* r) {
                auto* w = b.Multiply(elem_ty, m, n);
                auto* x = b.Multiply(elem_ty, o, p);
                auto* y = b.Multiply(elem_ty, q, r);

                auto* z = b.Subtract(elem_ty, w, x);
                return b.Add(elem_ty, z, y);
            };

            // Returns (m * n) + (o * p) - (q * r)
            auto add_sub_mul3 = [&](auto* m, auto* n, auto* o, auto* p, auto* q, auto* r) {
                auto* w = b.Multiply(elem_ty, m, n);
                auto* x = b.Multiply(elem_ty, o, p);
                auto* y = b.Multiply(elem_ty, q, r);

                auto* z = b.Add(elem_ty, w, x);
                return b.Subtract(elem_ty, z, y);
            };

            switch (mat_ty->Columns()) {
                case 2: {
                    auto* neg_inv_det = b.Negation(elem_ty, inv_det);

                    auto* ma = b.Access(elem_ty, arg, 0_u, 0_u);
                    auto* mb = b.Access(elem_ty, arg, 0_u, 1_u);
                    auto* mc = b.Access(elem_ty, arg, 1_u, 0_u);
                    auto* md = b.Access(elem_ty, arg, 1_u, 1_u);

                    auto* r_00 = b.Multiply(elem_ty, inv_det, md);
                    auto* r_01 = b.Multiply(elem_ty, neg_inv_det, mb);
                    auto* r_10 = b.Multiply(elem_ty, neg_inv_det, mc);
                    auto* r_11 = b.Multiply(elem_ty, inv_det, ma);

                    auto* r1 = b.Construct(ty.vec2(elem_ty), r_00, r_01);
                    auto* r2 = b.Construct(ty.vec2(elem_ty), r_10, r_11);
                    b.ConstructWithResult(call->DetachResult(), r1, r2);
                    break;
                }
                case 3: {
                    auto* ma = b.Access(elem_ty, arg, 0_u, 0_u);
                    auto* mb = b.Access(elem_ty, arg, 0_u, 1_u);
                    auto* mc = b.Access(elem_ty, arg, 0_u, 2_u);
                    auto* md = b.Access(elem_ty, arg, 1_u, 0_u);
                    auto* me = b.Access(elem_ty, arg, 1_u, 1_u);
                    auto* mf = b.Access(elem_ty, arg, 1_u, 2_u);
                    auto* mg = b.Access(elem_ty, arg, 2_u, 0_u);
                    auto* mh = b.Access(elem_ty, arg, 2_u, 1_u);
                    auto* mi = b.Access(elem_ty, arg, 2_u, 2_u);

                    // e * i - f * h
                    auto* r_00 = sub_mul2(me, mi, mf, mh);
                    // c * h - b * i
                    auto* r_01 = sub_mul2(mc, mh, mb, mi);
                    // b * f - c * e
                    auto* r_02 = sub_mul2(mb, mf, mc, me);

                    // f * g - d * i
                    auto* r_10 = sub_mul2(mf, mg, md, mi);
                    // a * i - c * g
                    auto* r_11 = sub_mul2(ma, mi, mc, mg);
                    // c * d - a * f
                    auto* r_12 = sub_mul2(mc, md, ma, mf);

                    // d * h - e * g
                    auto* r_20 = sub_mul2(md, mh, me, mg);
                    // b * g - a * h
                    auto* r_21 = sub_mul2(mb, mg, ma, mh);
                    // a * e - b * d
                    auto* r_22 = sub_mul2(ma, me, mb, md);

                    auto* r1 = b.Construct(ty.vec3(elem_ty), r_00, r_01, r_02);
                    auto* r2 = b.Construct(ty.vec3(elem_ty), r_10, r_11, r_12);
                    auto* r3 = b.Construct(ty.vec3(elem_ty), r_20, r_21, r_22);

                    auto* m = b.Construct(mat_ty, r1, r2, r3);
                    auto* inv = b.Multiply(mat_ty, inv_det, m);
                    call->Result(0)->ReplaceAllUsesWith(inv->Result(0));
                    break;
                }
                case 4: {
                    auto* ma = b.Access(elem_ty, arg, 0_u, 0_u);
                    auto* mb = b.Access(elem_ty, arg, 0_u, 1_u);
                    auto* mc = b.Access(elem_ty, arg, 0_u, 2_u);
                    auto* md = b.Access(elem_ty, arg, 0_u, 3_u);
                    auto* me = b.Access(elem_ty, arg, 1_u, 0_u);
                    auto* mf = b.Access(elem_ty, arg, 1_u, 1_u);
                    auto* mg = b.Access(elem_ty, arg, 1_u, 2_u);
                    auto* mh = b.Access(elem_ty, arg, 1_u, 3_u);
                    auto* mi = b.Access(elem_ty, arg, 2_u, 0_u);
                    auto* mj = b.Access(elem_ty, arg, 2_u, 1_u);
                    auto* mk = b.Access(elem_ty, arg, 2_u, 2_u);
                    auto* ml = b.Access(elem_ty, arg, 2_u, 3_u);
                    auto* mm = b.Access(elem_ty, arg, 3_u, 0_u);
                    auto* mn = b.Access(elem_ty, arg, 3_u, 1_u);
                    auto* mo = b.Access(elem_ty, arg, 3_u, 2_u);
                    auto* mp = b.Access(elem_ty, arg, 3_u, 3_u);

                    // kplo = k * p - l * o
                    auto* kplo = sub_mul2(mk, mp, ml, mo);
                    // jpln = j * p - l * n
                    auto* jpln = sub_mul2(mj, mp, ml, mn);
                    // jokn = j * o - k * n;
                    auto* jokn = sub_mul2(mj, mo, mk, mn);
                    // gpho = g * p - h * o
                    auto* gpho = sub_mul2(mg, mp, mh, mo);
                    // fphn = f * p - h * n
                    auto* fphn = sub_mul2(mf, mp, mh, mn);
                    // fogn = f * o - g * n;
                    auto* fogn = sub_mul2(mf, mo, mg, mn);
                    // glhk = g * l - h * k
                    auto* glhk = sub_mul2(mg, ml, mh, mk);
                    // flhj = f * l - h * j
                    auto* flhj = sub_mul2(mf, ml, mh, mj);
                    // fkgj = f * k - g * j;
                    auto* fkgj = sub_mul2(mf, mk, mg, mj);
                    // iplm = i * p - l * m
                    auto* iplm = sub_mul2(mi, mp, ml, mm);
                    // iokm = i * o - k * m
                    auto* iokm = sub_mul2(mi, mo, mk, mm);
                    // ephm = e * p - h * m;
                    auto* ephm = sub_mul2(me, mp, mh, mm);
                    // eogm = e * o - g * m
                    auto* eogm = sub_mul2(me, mo, mg, mm);
                    // elhi = e * l - h * i
                    auto* elhi = sub_mul2(me, ml, mh, mi);
                    // ekgi = e * k - g * i;
                    auto* ekgi = sub_mul2(me, mk, mg, mi);
                    // injm = i * n - j * m
                    auto* injm = sub_mul2(mi, mn, mj, mm);
                    // enfm = e * n - f * m
                    auto* enfm = sub_mul2(me, mn, mf, mm);
                    // ejfi = e * j - f * i;
                    auto* ejfi = sub_mul2(me, mj, mf, mi);

                    auto* neg_b = b.Negation(elem_ty, mb);
                    // f * kplo - g * jpln + h * jokn
                    auto* r_00 = sub_add_mul3(mf, kplo, mg, jpln, mh, jokn);
                    // -b * kplo + c * jpln - d * jokn
                    auto* r_01 = add_sub_mul3(neg_b, kplo, mc, jpln, md, jokn);
                    // b * gpho - c * fphn + d * fogn
                    auto* r_02 = sub_add_mul3(mb, gpho, mc, fphn, md, fogn);
                    // -b * glhk + c * flhj - d * fkgj
                    auto* r_03 = add_sub_mul3(neg_b, glhk, mc, flhj, md, fkgj);

                    auto* neg_e = b.Negation(elem_ty, me);
                    auto* neg_a = b.Negation(elem_ty, ma);
                    // -e * kplo + g * iplm - h * iokm
                    auto* r_10 = add_sub_mul3(neg_e, kplo, mg, iplm, mh, iokm);
                    // a * kplo - c * iplm + d * iokm
                    auto* r_11 = sub_add_mul3(ma, kplo, mc, iplm, md, iokm);
                    // -a * gpho + c * ephm - d * eogm
                    auto* r_12 = add_sub_mul3(neg_a, gpho, mc, ephm, md, eogm);
                    // a * glhk - c * elhi + d * ekgi
                    auto* r_13 = sub_add_mul3(ma, glhk, mc, elhi, md, ekgi);

                    // e * jpln - f * iplm + h * injm
                    auto* r_20 = sub_add_mul3(me, jpln, mf, iplm, mh, injm);
                    // -a * jpln + b * iplm - d * injm
                    auto* r_21 = add_sub_mul3(neg_a, jpln, mb, iplm, md, injm);
                    // a * fphn - b * ephm + d * enfm
                    auto* r_22 = sub_add_mul3(ma, fphn, mb, ephm, md, enfm);
                    // -a * flhj + b * elhi - d * ejfi
                    auto* r_23 = add_sub_mul3(neg_a, flhj, mb, elhi, md, ejfi);

                    // -e * jokn + f * iokm - g * injm
                    auto* r_30 = add_sub_mul3(neg_e, jokn, mf, iokm, mg, injm);
                    // a * jokn - b * iokm + c * injm
                    auto* r_31 = sub_add_mul3(ma, jokn, mb, iokm, mc, injm);
                    // -a * fogn + b * eogm - c * enfm
                    auto* r_32 = add_sub_mul3(neg_a, fogn, mb, eogm, mc, enfm);
                    // a * fkgj - b * ekgi + c * ejfi
                    auto* r_33 = sub_add_mul3(ma, fkgj, mb, ekgi, mc, ejfi);

                    auto* r1 = b.Construct(ty.vec3(elem_ty), r_00, r_01, r_02, r_03);
                    auto* r2 = b.Construct(ty.vec3(elem_ty), r_10, r_11, r_12, r_13);
                    auto* r3 = b.Construct(ty.vec3(elem_ty), r_20, r_21, r_22, r_23);
                    auto* r4 = b.Construct(ty.vec3(elem_ty), r_30, r_31, r_32, r_33);

                    auto* m = b.Construct(mat_ty, r1, r2, r3, r4);
                    auto* inv = b.Multiply(mat_ty, inv_det, m);
                    call->Result(0)->ReplaceAllUsesWith(inv->Result(0));
                    break;
                }
                default: {
                    TINT_UNREACHABLE();
                }
            }
        });
        call->Destroy();
    }
};

}  // namespace

Result<SuccessType> Builtins(core::ir::Module& ir) {
    auto result = ValidateAndDumpIfNeeded(ir, "spirv.Builtins");
    if (result != Success) {
        return result.Failure();
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::spirv::reader::lower
