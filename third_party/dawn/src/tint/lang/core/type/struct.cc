// Copyright 2022 The Dawn & Tint Authors
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

#include "src/tint/lang/core/type/struct.h"

#include <cmath>
#include <iomanip>
#include <string>
#include <string_view>
#include <utility>

#include "src/tint/lang/core/type/manager.h"
#include "src/tint/utils/math/hash.h"
#include "src/tint/utils/symbol/symbol_table.h"
#include "src/tint/utils/text/string_stream.h"
#include "src/tint/utils/text/styled_text.h"
#include "src/tint/utils/text/text_style.h"

TINT_INSTANTIATE_TYPEINFO(tint::core::type::Struct);
TINT_INSTANTIATE_TYPEINFO(tint::core::type::StructMember);

namespace tint::core::type {
namespace {

Flags FlagsFrom(VectorRef<const StructMember*> members) {
    Flags flags{
        Flag::kConstructable,
        Flag::kCreationFixedFootprint,
        Flag::kFixedFootprint,
    };
    for (auto* member : members) {
        if (!member->Type()->IsConstructible()) {
            flags.Remove(Flag::kConstructable);
        }
        if (!member->Type()->HasFixedFootprint()) {
            flags.Remove(Flag::kFixedFootprint);
        }
        if (!member->Type()->HasCreationFixedFootprint()) {
            flags.Remove(Flag::kCreationFixedFootprint);
        }
    }
    return flags;
}

}  // namespace

Struct::Struct(Symbol name, bool is_wgsl_internal)
    : Base(Hash(tint::TypeCode::Of<Struct>().bits, name, is_wgsl_internal), type::Flags{}),
      name_(name),
      members_{},
      align_(0),
      size_(0),
      size_no_padding_(0),
      is_wgsl_internal_(is_wgsl_internal) {}

Struct::Struct(Symbol name,
               VectorRef<const StructMember*> members,
               uint32_t align,
               uint32_t size,
               uint32_t size_no_padding,
               bool is_wgsl_internal)
    : Base(Hash(tint::TypeCode::Of<Struct>().bits, name, is_wgsl_internal), FlagsFrom(members)),
      name_(name),
      members_(std::move(members)),
      align_(align),
      size_(size),
      size_no_padding_(size_no_padding),
      is_wgsl_internal_(is_wgsl_internal) {}

Struct::~Struct() = default;

bool Struct::Equals(const UniqueNode& other) const {
    if (auto* o = other.As<Struct>()) {
        return o->name_ == name_;
    }
    return false;
}

const StructMember* Struct::FindMember(Symbol name) const {
    for (auto* member : members_) {
        if (member->Name() == name) {
            return member;
        }
    }
    return nullptr;
}

uint32_t Struct::Align() const {
    return align_;
}

uint32_t Struct::Size() const {
    return size_;
}

std::string Struct::FriendlyName() const {
    return name_.Name();
}

StyledText Struct::Layout() const {
    static constexpr auto Code = style::Code + style::NoQuote;
    static constexpr auto Comment = style::Comment + style::NoQuote;
    static constexpr auto Keyword = style::Keyword + style::NoQuote;
    static constexpr auto Type = style::Type + style::NoQuote;
    static constexpr auto Variable = style::Variable + style::NoQuote;
    static constexpr auto Plain = style::Plain;

    StyledText out;

    auto member_name_of = [&](const StructMember* sm) { return sm->Name().Name(); };

    if (Members().IsEmpty()) {
        return {};
    }
    const auto* const last_member = Members().Back();
    const uint32_t last_member_struct_padding_offset = last_member->Offset() + last_member->Size();

    // Compute max widths to align output
    const auto offset_w = static_cast<int>(::log10(last_member_struct_padding_offset)) + 1;
    const auto size_w = static_cast<int>(::log10(Size())) + 1;
    const auto align_w = static_cast<int>(::log10(Align())) + 1;

    auto pad = [](int n) { return std::string(static_cast<size_t>(n), ' '); };

    auto print_struct_begin_line = [&](size_t align, size_t size, std::string struct_name) {
        out << Comment("/*          ", pad(offset_w), "align(", std::setw(align_w), align,
                       ") size(", std::setw(size_w), size, ") */")
            << Keyword(" struct ") << Type(struct_name) << Code(" {") << Plain("\n");
    };

    auto print_struct_end_line = [&] {
        out << Comment("/*                         ", pad(offset_w + size_w + align_w), "*/")
            << Code(" };");
    };

    auto print_member_line = [&](size_t offset, size_t align, size_t size, StyledText s) {
        out << Comment("/* offset(", std::setw(offset_w), offset, ") align(", std::setw(align_w),
                       align, ") size(", std::setw(size_w), size, ") */ ")
            << s << Plain("\n");
    };

    print_struct_begin_line(Align(), Size(), UnwrapRef()->FriendlyName());

    for (size_t i = 0; i < Members().Length(); ++i) {
        auto* const m = Members()[i];

        // Output field alignment padding, if any
        auto* const prev_member = (i == 0) ? nullptr : Members()[i - 1];
        if (prev_member) {
            uint32_t padding = m->Offset() - (prev_member->Offset() + prev_member->Size());
            if (padding > 0) {
                size_t padding_offset = m->Offset() - padding;
                print_member_line(padding_offset, 1, padding,
                                  StyledText{}
                                      << Code("  ")
                                      << Comment("// -- implicit field alignment padding --"));
            }
        }

        // Output member
        std::string member_name = member_name_of(m);
        print_member_line(m->Offset(), m->Align(), m->Size(),
                          StyledText{} << Code("  ") << Variable(member_name) << Code(" : ")
                                       << Type(m->Type()->UnwrapRef()->FriendlyName())
                                       << Code(","));
    }

    // Output struct size padding, if any
    uint32_t struct_padding = Size() - last_member_struct_padding_offset;
    if (struct_padding > 0) {
        print_member_line(
            last_member_struct_padding_offset, 1, struct_padding,
            StyledText{} << Code("  ") << Comment("// -- implicit struct size padding --"));
    }

    print_struct_end_line();

    return out;
}

TypeAndCount Struct::Elements(const Type* type_if_invalid /* = nullptr */,
                              uint32_t /* count_if_invalid = 0 */) const {
    return {type_if_invalid, static_cast<uint32_t>(members_.Length())};
}

const Type* Struct::Element(uint32_t index) const {
    return index < members_.Length() ? members_[index]->Type() : nullptr;
}

Struct* Struct::Clone(CloneContext& ctx) const {
    auto sym = ctx.dst.st->Register(name_.Name());

    tint::Vector<const StructMember*, 4> members;
    for (const auto& mem : members_) {
        members.Push(mem->Clone(ctx));
    }
    return ctx.dst.mgr->Get<Struct>(sym, members, align_, size_, size_no_padding_,
                                    is_wgsl_internal_);
}

StructMember::StructMember(Symbol name,
                           const core::type::Type* type,
                           uint32_t index,
                           uint32_t offset,
                           uint32_t align,
                           uint32_t size,
                           const IOAttributes& attributes)
    : name_(name),
      type_(type),
      index_(index),
      offset_(offset),
      align_(align),
      size_(size),
      attributes_(attributes) {}

StructMember::~StructMember() = default;

StructMember* StructMember::Clone(CloneContext& ctx) const {
    auto sym = ctx.dst.st->Register(name_.Name());
    auto* ty = type_->Clone(ctx);
    return ctx.dst.mgr->Get<StructMember>(sym, ty, index_, offset_, align_, size_, attributes_);
}

}  // namespace tint::core::type
