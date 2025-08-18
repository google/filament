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

#include "src/tint/lang/core/constant/manager.h"

#include "src/tint/lang/core/constant/composite.h"
#include "src/tint/lang/core/constant/invalid.h"
#include "src/tint/lang/core/constant/scalar.h"
#include "src/tint/lang/core/constant/splat.h"
#include "src/tint/lang/core/constant/string.h"
#include "src/tint/lang/core/type/abstract_float.h"
#include "src/tint/lang/core/type/abstract_int.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/bool.h"
#include "src/tint/lang/core/type/f16.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/i8.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/u64.h"
#include "src/tint/lang/core/type/u8.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/utils/containers/predicates.h"
#include "src/tint/utils/rtti/switch.h"

namespace tint::core::constant {

Manager::Manager() = default;

Manager::Manager(Manager&&) = default;

Manager& Manager::operator=(Manager&& rhs) = default;

Manager::~Manager() = default;

const constant::Value* Manager::Composite(const core::type::Type* type,
                                          VectorRef<const constant::Value*> elements) {
    if (elements.IsEmpty()) {
        return nullptr;
    }

    bool any_zero = false;
    bool all_zero = true;
    bool all_equal = true;
    auto* first = elements.Front();
    for (auto* el : elements) {
        if (DAWN_UNLIKELY(!el)) {
            return nullptr;
        }
        if (!any_zero && el->AnyZero()) {
            any_zero = true;
        }
        if (all_zero && !el->AllZero()) {
            all_zero = false;
        }
        if (all_equal && el != first) {
            all_equal = false;
        }
    }
    if (all_equal) {
        return Splat(type, elements.Front());
    }

    return Get<constant::Composite>(type, std::move(elements), all_zero, any_zero);
}

const constant::Splat* Manager::Splat(const core::type::Type* type,
                                      const constant::Value* element) {
    return Get<constant::Splat>(type, element);
}

const Scalar<i32>* Manager::Get(i32 value) {
    return Get<Scalar<i32>>(types.i32(), value);
}

const Scalar<u32>* Manager::Get(u32 value) {
    return Get<Scalar<u32>>(types.u32(), value);
}

const Scalar<u64>* Manager::Get(u64 value) {
    return Get<Scalar<u64>>(types.u64(), value);
}

const Scalar<i8>* Manager::Get(i8 value) {
    return Get<Scalar<i8>>(types.i8(), value);
}

const Scalar<u8>* Manager::Get(u8 value) {
    return Get<Scalar<u8>>(types.u8(), value);
}

const Scalar<f32>* Manager::Get(f32 value) {
    return Get<Scalar<f32>>(types.f32(), value);
}

const Scalar<f16>* Manager::Get(f16 value) {
    return Get<Scalar<f16>>(types.f16(), value);
}

const Scalar<bool>* Manager::Get(bool value) {
    return Get<Scalar<bool>>(types.bool_(), value);
}

const Scalar<AFloat>* Manager::Get(AFloat value) {
    return Get<Scalar<AFloat>>(types.AFloat(), value);
}

const Scalar<AInt>* Manager::Get(AInt value) {
    return Get<Scalar<AInt>>(types.AInt(), value);
}

const constant::String* Manager::Get(std::string_view value) {
    return Get<String>(types.String(), value);
}

const Value* Manager::Zero(const core::type::Type* type) {
    return Switch(
        type,  //
        [&](const core::type::Vector* v) -> const Value* {
            auto* zero_el = Zero(v->Type());
            return Splat(type, zero_el);
        },
        [&](const core::type::Matrix* m) -> const Value* {
            auto* zero_el = Zero(m->ColumnType());
            return Splat(type, zero_el);
        },
        [&](const core::type::Array* a) -> const Value* {
            if (a->ConstantCount()) {
                if (auto* zero_el = Zero(a->ElemType())) {
                    return Splat(type, zero_el);
                }
            }
            return nullptr;
        },
        [&](const core::type::Struct* s) -> const Value* {
            Hashmap<const core::type::Type*, const Value*, 8> zero_by_type;
            Vector<const Value*, 4> zeros;
            zeros.Reserve(s->Members().Length());
            for (auto* member : s->Members()) {
                auto* zero =
                    zero_by_type.GetOrAdd(member->Type(), [&] { return Zero(member->Type()); });
                if (!zero) {
                    return nullptr;
                }
                zeros.Push(zero);
            }
            if (zero_by_type.Count() == 1) {
                // All members were of the same type, so the zero value is the same for all members.
                return Splat(type, zeros[0]);
            }
            return Composite(s, std::move(zeros));
        },
        [&](const core::type::AbstractInt*) { return Get(AInt(0)); },      //
        [&](const core::type::AbstractFloat*) { return Get(AFloat(0)); },  //
        [&](const core::type::I32*) { return Get(i32(0)); },               //
        [&](const core::type::U32*) { return Get(u32(0)); },               //
        [&](const core::type::I8*) { return Get(i8(0)); },                 //
        [&](const core::type::U8*) { return Get(u8(0)); },                 //
        [&](const core::type::U64*) { return Get(u64(0)); },               //
        [&](const core::type::F32*) { return Get(f32(0)); },               //
        [&](const core::type::F16*) { return Get(f16(0)); },               //
        [&](const core::type::Bool*) { return Get(false); },               //
        [&](const core::type::Invalid*) { return Invalid(); },             //
        TINT_ICE_ON_NO_MATCH);
}

const constant::Invalid* Manager::Invalid() {
    return values_.Get<constant::Invalid>(types.invalid());
}

}  // namespace tint::core::constant
