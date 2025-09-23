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

#ifndef SRC_TINT_LANG_SPIRV_READER_AST_PARSER_ATTRIBUTES_H_
#define SRC_TINT_LANG_SPIRV_READER_AST_PARSER_ATTRIBUTES_H_

#include "src/tint/lang/core/enums.h"
#include "src/tint/lang/wgsl/ast/attribute.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/utils/containers/enum_set.h"
#include "src/tint/utils/containers/vector.h"

namespace tint::spirv::reader::ast_parser {

/// Attributes holds a vector of ast::Attribute pointers, and a enum-set of flags used to hold
/// additional metadata.
struct Attributes {
    /// Flags used by #flags.
    enum class Flags {
        kHasBuiltinSampleMask,
    };

    /// Adds the attributes and flags of @p other to this.
    /// @param other the other Attributes to combine into this
    void Add(const Attributes& other) {
        for (auto* attr : other.list) {
            list.Push(attr);
        }
        for (auto flag : other.flags) {
            flags.Add(flag);
        }
    }

    /// Adds the attribute @p attr to the list of attributes
    /// @param attr the attribute to add to this
    void Add(const ast::Attribute* attr) { list.Push(attr); }

    /// Adds the builtin to the attribute list, also marking any necessary flags
    /// @param builder the program builder
    /// @param source the source of the builtin attribute
    /// @param builtin the builtin attribute to add
    void Add(ProgramBuilder& builder, const Source& source, core::BuiltinValue builtin) {
        Add(builder.Builtin(source, builtin));
        if (builtin == core::BuiltinValue::kSampleMask) {
            flags.Add(Flags::kHasBuiltinSampleMask);
        }
    }

    /// @returns true if the attribute list contains an attribute with the type `T`.
    template <typename T>
    bool Has() const {
        return ast::HasAttribute<T>(list);
    }

    /// @returns the attribute with type `T` in the list, or nullptr if no attribute of the given
    /// type exists in list.
    template <typename T>
    const T* Get() const {
        return ast::GetAttribute<T>(list);
    }

    /// The attributes
    tint::Vector<const ast::Attribute*, 8> list;
    /// The additional metadata flags
    tint::EnumSet<Flags> flags;
};

}  // namespace tint::spirv::reader::ast_parser

#endif  // SRC_TINT_LANG_SPIRV_READER_AST_PARSER_ATTRIBUTES_H_
