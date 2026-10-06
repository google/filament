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

#include "WgslExternalTextures.h"

#include <utils/CString.h>

#include <src/tint/api/common/binding_point.h>
#include <src/tint/lang/core/enums.h>
#include <src/tint/lang/core/ir/builder.h>
#include <src/tint/lang/core/ir/call.h>
#include <src/tint/lang/core/ir/clone_context.h>
#include <src/tint/lang/core/ir/core_builtin_call.h>
#include <src/tint/lang/core/ir/function.h>
#include <src/tint/lang/core/ir/function_param.h>
#include <src/tint/lang/core/ir/instruction.h>
#include <src/tint/lang/core/ir/instruction_result.h>
#include <src/tint/lang/core/ir/let.h>
#include <src/tint/lang/core/ir/load.h>
#include <src/tint/lang/core/ir/module.h>
#include <src/tint/lang/core/ir/user_call.h>
#include <src/tint/lang/core/ir/validator.h>
#include <src/tint/lang/core/ir/value.h>
#include <src/tint/lang/core/ir/var.h>
#include <src/tint/lang/core/type/external_texture.h>
#include <src/tint/lang/core/type/manager.h>
#include <src/tint/lang/core/type/pointer.h>
#include <src/tint/lang/core/type/sampled_texture.h>
#include <src/tint/lang/core/type/texture_dimension.h>
#include <src/tint/utils/symbol/symbol.h>

#include <algorithm>
#include <map>
#include <string_view>
#include <utility>
#include <vector>

#include <stddef.h>
#include <stdint.h>

// Note: the src/tint/ headers above are Tint's internal IR headers. They are not part of Tint's
// public API and may change when Dawn is updated.

namespace filamat {

namespace {

namespace ir = tint::core::ir;
namespace type = tint::core::type;
using tint::core::BuiltinFn;

class ExternalTextureRetyper {
public:
    ExternalTextureRetyper(ir::Module& module, std::vector<WgslBindingPoint> const& bindings)
            : mModule(module),
              mBuilder(module),
              mExternalType(module.Types().external_texture()),
              mSampledType(module.Types().sampled_texture(type::TextureDimension::k2d,
                      module.Types().f32())),
              mBindings(bindings) {}

    bool run(utils::CString* error) {
        bool found = false;
        if (!retypeVariables(&found, error)) {
            return false;
        }
        if (!found) {
            // None of the external samplers are used by this shader.
            return true;
        }
        propagateTypes();
        removeUncalledOriginals();
        if (!rewriteBuiltins(error)) {
            return false;
        }
        auto const result = ir::Validate(mModule, "after filamat::retypeExternalTextures");
        if (result != tint::Success) {
            *error = utils::CString(std::string_view(result.Failure().reason));
            return false;
        }
        return true;
    }

private:
    bool isExternalBinding(tint::BindingPoint const& bp) const noexcept {
        return std::any_of(mBindings.begin(), mBindings.end(), [&](WgslBindingPoint const& b) {
            return b.group == bp.group && b.binding == bp.binding;
        });
    }

    // Returns the name of a texture value for error messages, looking through loads so that the
    // variable or the function parameter is named.
    utils::CString nameOf(ir::Value const* value) const {
        if (auto const* result = value->As<ir::InstructionResult>()) {
            if (auto const* load = result->Instruction()->As<ir::Load>()) {
                value = load->From();
            }
        }
        auto const symbol = mModule.NameOf(value);
        return symbol ? utils::CString(symbol.NameView()) : utils::CString("<unnamed>");
    }

    // Returns an error message about the external sampler of the given texture value.
    utils::CString samplerError(ir::Value const* texture, char const* reason) const {
        utils::CString message("external sampler '");
        message += nameOf(texture);
        message += "' ";
        message += reason;
        return message;
    }

    // Retypes the module-scope variables bound to an external sampler from texture_2d<f32> to
    // texture_external.
    bool retypeVariables(bool* found, utils::CString* error) {
        for (auto* inst: *mModule.root_block) {
            auto* var = inst->As<ir::Var>();
            if (!var) {
                continue;
            }
            auto const bp = var->BindingPoint();
            if (!bp || !isExternalBinding(*bp)) {
                continue;
            }
            auto const* ptr = var->Result()->Type()->As<type::Pointer>();
            if (!ptr || ptr->StoreType() != mSampledType) {
                *error = samplerError(var->Result(), "is not declared as a 2D float texture");
                return false;
            }
            var->Result()->SetType(
                    mModule.Types().ptr(ptr->AddressSpace(), mExternalType, ptr->Access()));
            *found = true;
        }
        return true;
    }

    // Propagates the texture_external type from the variables to every value derived from them,
    // until a fixed point is reached. Each iteration scans all the instructions of the module,
    // which keeps the logic simple: forked functions are cloned in whatever state they are in, and
    // are simply picked up by the next iteration.
    void propagateTypes() {
        bool changed = true;
        while (changed) {
            changed = false;
            // Snapshot the instructions, since forking functions creates new ones.
            std::vector<ir::Instruction*> instructions;
            for (auto* inst: mModule.Instructions()) {
                instructions.push_back(inst);
            }
            for (auto* inst: instructions) {
                if (!inst->Alive()) {
                    continue;
                }
                if (auto* load = inst->As<ir::Load>()) {
                    if (load->From()->Type()->UnwrapPtr() == mExternalType &&
                            load->Result()->Type() != mExternalType) {
                        load->Result()->SetType(mExternalType);
                        changed = true;
                    }
                } else if (auto* let = inst->As<ir::Let>()) {
                    // WGSL doesn't allow texture-typed lets, so forward the value to the users.
                    if (let->Value()->Type() == mExternalType) {
                        let->Result()->ReplaceAllUsesWith(let->Value());
                        let->Destroy();
                        changed = true;
                    }
                } else if (auto* call = inst->As<ir::UserCall>()) {
                    changed |= forkCallee(call);
                }
            }
        }
    }

    // If the call passes an external texture to a texture_2d<f32> parameter, retargets it to a
    // clone of the callee whose matching parameters are texture_external. Clones are shared by
    // all calls that pass external textures at the same parameter positions.
    bool forkCallee(ir::UserCall* call) {
        ir::Function* const target = call->Target();
        auto const params = target->Params();
        auto const args = call->Args();

        std::vector<uint32_t> externalParams;
        bool needsFork = false;
        for (uint32_t i = 0; i < args.size(); i++) {
            if (args[i]->Type() == mExternalType) {
                externalParams.push_back(i);
                if (params[i]->Type() != mExternalType) {
                    needsFork = true;
                }
            }
        }
        if (!needsFork) {
            return false;
        }

        // Always clone the original function, so the non-external parameters keep their type.
        ir::Function* original = target;
        if (auto const pos = mOriginals.find(target); pos != mOriginals.end()) {
            original = pos->second;
        }

        ir::Function*& fork = mForks[{ original, externalParams }];
        if (!fork) {
            ir::CloneContext ctx{ mModule };
            fork = original->Clone(ctx);
            for (uint32_t const i: externalParams) {
                fork->Params()[i]->SetType(mExternalType);
            }
            // Insert the clone right after the original, so that the generated code keeps the
            // same function order when the original is removed.
            auto& functions = mModule.functions;
            for (size_t i = 0; i < functions.Length(); i++) {
                if (functions[i] == original) {
                    functions.Insert(i + 1, fork);
                    break;
                }
            }
            mOriginals[fork] = original;
            if (std::find(mForkedFunctions.begin(), mForkedFunctions.end(), original) ==
                    mForkedFunctions.end()) {
                mForkedFunctions.push_back(original);
            }
        }
        call->SetTarget(fork);
        return true;
    }

    // Removes the functions that were forked and are no longer called. Removing a function can
    // leave the functions it calls without callers, hence the loop.
    void removeUncalledOriginals() {
        bool changed = true;
        while (changed) {
            changed = false;
            for (ir::Function* fn: mForkedFunctions) {
                if (!fn->Alive() || fn->IsEntryPoint()) {
                    continue;
                }
                bool called = false;
                for (auto& usage: fn->UsagesUnsorted()) {
                    if (usage->instruction->Is<ir::Call>()) {
                        called = true;
                        break;
                    }
                }
                if (!called) {
                    mModule.Destroy(fn);
                    changed = true;
                }
            }
        }
    }

    bool rewriteBuiltins(utils::CString* error) {
        std::vector<ir::CoreBuiltinCall*> calls;
        for (auto* inst: mModule.Instructions()) {
            if (auto* call = inst->As<ir::CoreBuiltinCall>()) {
                calls.push_back(call);
            }
        }
        for (auto* call: calls) {
            if (!rewriteBuiltin(call, error)) {
                return false;
            }
        }
        return true;
    }

    // Rewrites a texture builtin call that takes a texture_2d<f32> into its texture_external
    // equivalent.
    bool rewriteBuiltin(ir::CoreBuiltinCall* call, utils::CString* error) {
        auto const args = call->Args();
        auto const pos = std::find_if(args.begin(), args.end(),
                [this](ir::Value const* arg) { return arg->Type() == mExternalType; });
        if (pos == args.end()) {
            return true;
        }

        BuiltinFn const fn = call->Func();
        auto fail = [&](char const* reason) {
            *error = samplerError(*pos, reason);
            return false;
        };

        // The number of arguments of each sampling function, without the optional offset. All the
        // functions handled below take the texture as their first argument.
        size_t sampleArgCount = 0;
        switch (fn) {
            case BuiltinFn::kTextureSample:
                sampleArgCount = 3; // texture, sampler, coords
                break;
            case BuiltinFn::kTextureSampleBias:
            case BuiltinFn::kTextureSampleLevel:
                sampleArgCount = 4; // texture, sampler, coords, bias or level
                break;
            case BuiltinFn::kTextureSampleGrad:
                sampleArgCount = 5; // texture, sampler, coords, ddx, ddy
                break;
            case BuiltinFn::kTextureLoad: {
                // textureLoad(texture, coords, level) -> textureLoad(texture, coords)
                ir::Value* const texture = args[0];
                ir::Value* const coords = args[1];
                mBuilder.InsertBefore(call, [&] {
                    mBuilder.CallWithResult(call->DetachResult(), fn, texture, coords);
                });
                call->Destroy();
                return true;
            }
            case BuiltinFn::kTextureDimensions: {
                // textureDimensions(texture[, level]) -> textureDimensions(texture)
                ir::Value* const texture = args[0];
                mBuilder.InsertBefore(call,
                        [&] { mBuilder.CallWithResult(call->DetachResult(), fn, texture); });
                call->Destroy();
                return true;
            }
            default: {
                utils::CString reason("cannot be used with ");
                reason += tint::core::str(fn);
                return fail(reason.c_str());
            }
        }

        if (args.size() > sampleArgCount) {
            return fail("cannot be sampled with an offset");
        }

        // External textures have a single mip level, so the bias, level or gradients can be
        // dropped.
        ir::Value* const texture = args[0];
        ir::Value* const sampler = args[1];
        ir::Value* const coords = args[2];
        mBuilder.InsertBefore(call, [&] {
            mBuilder.CallWithResult(call->DetachResult(), BuiltinFn::kTextureSampleBaseClampToEdge,
                    texture, sampler, coords);
        });
        call->Destroy();
        return true;
    }

    ir::Module& mModule;
    ir::Builder mBuilder;
    type::Type const* const mExternalType;
    type::Type const* const mSampledType;
    std::vector<WgslBindingPoint> const& mBindings;

    // Clones of functions, keyed by the original function and the indices of the parameters that
    // are retyped to texture_external.
    std::map<std::pair<ir::Function*, std::vector<uint32_t>>, ir::Function*> mForks;
    // The original function of each clone.
    std::map<ir::Function*, ir::Function*> mOriginals;
    // The functions that were cloned, in the order they were first cloned.
    std::vector<ir::Function*> mForkedFunctions;
};

} // anonymous namespace

bool retypeExternalTextures(tint::core::ir::Module& module,
        std::vector<WgslBindingPoint> const& bindings, utils::CString* error) {
    if (bindings.empty()) {
        return true;
    }
    return ExternalTextureRetyper(module, bindings).run(error);
}

} // namespace filamat
