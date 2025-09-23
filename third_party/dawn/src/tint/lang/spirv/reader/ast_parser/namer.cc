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

#include "src/tint/lang/spirv/reader/ast_parser/namer.h"

#include <algorithm>
#include <unordered_set>

#include "src/tint/lang/core/enums.h"
#include "src/tint/utils/ice/ice.h"
#include "src/tint/utils/text/string_stream.h"

namespace tint::spirv::reader::ast_parser {

namespace {

// The reserved words in WGSL.
// TODO(crbug.com/368114894) update this list to match the WGSL spec.
const char* kWGSLReservedWords[] = {
    // Please keep this list sorted
    "array",
    "as",
    "asm",
    "atomic",
    "bf16",
    "binding",
    "block",
    "bool",
    "break",
    "builtin",
    "case",
    "cast",
    "compute",
    "const",
    "continue",
    "default",
    "discard",
    "do",
    "else",
    "elseif",
    "entry_point",
    "enum",
    "f16",
    "f32",
    "fallthrough",
    "false",
    "fn",
    "for",
    "frag_depth",
    "fragment",
    "front_facing",
    "global_invocation_id",
    "i16",
    "i32",
    "i64",
    "i8",
    "if",
    "image",
    "import",
    "in",
    "instance_index",
    "let",
    "local_invocation_id",
    "local_invocation_index",
    "location",
    "loop",
    "mat2x2",
    "mat2x2f",
    "mat2x2h",
    "mat2x3",
    "mat2x3f",
    "mat2x3h",
    "mat2x4",
    "mat2x4f",
    "mat2x4h",
    "mat3x2",
    "mat3x2f",
    "mat3x2h",
    "mat3x3",
    "mat3x3f",
    "mat3x3h",
    "mat3x4",
    "mat3x4f",
    "mat3x4h",
    "mat4x2",
    "mat4x2f",
    "mat4x2h",
    "mat4x3",
    "mat4x3f",
    "mat4x3h",
    "mat4x4",
    "mat4x4f",
    "mat4x4h",
    "non_coherent",
    "noncoherent",
    "num_workgroups",
    "offset",
    "out",
    "override",
    "position",
    "premerge",
    "private",
    "ptr",
    "regardless",
    "return",
    "sample_index",
    "sample_mask",
    "sampler_comparison",
    "sampler",
    "set",
    "storage",
    "struct",
    "switch",
    "texture_1d",
    "texture_2d_array",
    "texture_2d",
    "texture_3d",
    "texture_cube_array",
    "texture_cube",
    "texture_depth_2d_array",
    "texture_depth_2d",
    "texture_depth_cube_array",
    "texture_depth_cube",
    "texture_depth_multisampled_2d",
    "texture_external",
    "texture_multisampled_2d",
    "texture_storage_1d",
    "texture_storage_2d_array",
    "texture_storage_2d",
    "texture_storage_3d",
    "true",
    "type",
    "typedef",
    "u16",
    "u32",
    "u64",
    "u8",
    "uniform_constant",
    "uniform",
    "unless",
    "using",
    "var",
    "vec2",
    "vec2f",
    "vec2h",
    "vec2i",
    "vec2u",
    "vec3",
    "vec3f",
    "vec3h",
    "vec3i",
    "vec3u",
    "vec4",
    "vec4f",
    "vec4h",
    "vec4i",
    "vec4u",
    "vertex_index",
    "vertex",
    "void",
    "while",
    "workgroup_id",
    "workgroup",
};

}  // namespace

Namer::Namer(const FailStream& fail_stream) : fail_stream_(fail_stream) {
    for (const auto* reserved : kWGSLReservedWords) {
        name_to_id_[std::string(reserved)] = 0;
    }
    for (const auto* builtin_function : core::kBuiltinFnStrings) {
        name_to_id_[std::string(builtin_function)] = 0;
    }
}

Namer::~Namer() = default;

std::string Namer::Sanitize(const std::string& suggested_name) {
    if (suggested_name.empty()) {
        return "empty";
    }
    // Otherwise, replace invalid characters by '_'.
    std::string result;
    std::string invalid_as_first_char = "_0123456789";
    std::string valid =
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "_0123456789";
    // If the first character is invalid for starting a WGSL identifier, then
    // prefix the result with "x".
    if ((std::string::npos != invalid_as_first_char.find(suggested_name[0])) ||
        (std::string::npos == valid.find(suggested_name[0]))) {
        result = "x";
    }
    std::transform(
        suggested_name.begin(), suggested_name.end(), std::back_inserter(result),
        [&valid](const char c) { return (std::string::npos == valid.find(c)) ? '_' : c; });
    return result;
}

std::string Namer::GetMemberName(uint32_t struct_id, uint32_t member_index) const {
    std::string result;
    auto where = struct_member_names_.find(struct_id);
    if (where != struct_member_names_.end()) {
        auto& member_names = where->second;
        if (member_index < member_names.size()) {
            result = member_names[member_index];
        }
    }
    return result;
}

std::string Namer::FindUnusedDerivedName(const std::string& base_name) {
    // Ensure uniqueness among names.
    std::string derived_name;
    uint32_t& i = next_unusued_derived_name_id_[base_name];
    while (i != 0xffffffff) {
        StringStream new_name_stream;
        new_name_stream << base_name;
        if (i > 0) {
            new_name_stream << "_" << i;
        }
        derived_name = new_name_stream.str();
        if (!IsRegistered(derived_name)) {
            return derived_name;
        }
        i++;
    }
    TINT_UNREACHABLE() << "FindUnusedDerivedName() overflowed u32";
}

std::string Namer::MakeDerivedName(const std::string& base_name) {
    auto result = FindUnusedDerivedName(base_name);
    const bool registered = RegisterWithoutId(result);
    TINT_ASSERT(registered);
    return result;
}

bool Namer::Register(uint32_t id, const std::string& name) {
    if (HasName(id)) {
        return Fail() << "internal error: ID " << id
                      << " already has registered name: " << id_to_name_[id];
    }
    if (!RegisterWithoutId(name)) {
        return false;
    }
    id_to_name_[id] = name;
    name_to_id_[name] = id;
    return true;
}

bool Namer::RegisterWithoutId(const std::string& name) {
    if (IsRegistered(name)) {
        return Fail() << "internal error: name already registered: " << name;
    }
    name_to_id_[name] = 0;
    return true;
}

bool Namer::SuggestSanitizedName(uint32_t id, const std::string& suggested_name) {
    if (HasName(id)) {
        return false;
    }

    return Register(id, FindUnusedDerivedName(Sanitize(suggested_name)));
}

bool Namer::SuggestSanitizedMemberName(uint32_t struct_id,
                                       uint32_t member_index,
                                       const std::string& suggested_name) {
    // Creates an empty vector the first time we visit this struct.
    auto& name_vector = struct_member_names_[struct_id];
    // Resizing will set new entries to the empty string.
    name_vector.resize(std::max(name_vector.size(), size_t(member_index + 1)));
    auto& entry = name_vector[member_index];
    if (entry.empty()) {
        entry = Sanitize(suggested_name);
        return true;
    }
    return false;
}

void Namer::ResolveMemberNamesForStruct(uint32_t struct_id, uint32_t num_members) {
    auto& name_vector = struct_member_names_[struct_id];
    // Resizing will set new entries to the empty string.
    // It would have been an error if the client had registered a name for
    // an out-of-bounds member index, so toss those away.
    name_vector.resize(num_members);

    std::unordered_set<std::string> used_names;

    // Returns a name, based on the suggestion, which does not equal
    // any name in the used_names set.
    auto disambiguate_name = [&used_names](const std::string& suggestion) -> std::string {
        if (used_names.find(suggestion) == used_names.end()) {
            // There is no collision.
            return suggestion;
        }

        uint32_t i = 1;
        std::string new_name;
        do {
            StringStream new_name_stream;
            new_name_stream << suggestion << "_" << i;
            new_name = new_name_stream.str();
            ++i;
        } while (used_names.find(new_name) != used_names.end());
        return new_name;
    };

    // First ensure uniqueness among names for which we have already taken
    // suggestions.
    for (auto& name : name_vector) {
        if (!name.empty()) {
            // This modifies the names in-place, i.e. update the name_vector
            // entries.
            name = disambiguate_name(name);
            used_names.insert(name);
        }
    }

    // Now ensure uniqueness among the rest.  Doing this in a second pass
    // allows us to preserve suggestions as much as possible.  Otherwise
    // a generated name such as 'field1' might collide with a user-suggested
    // name of 'field1' attached to a later member.
    uint32_t index = 0;
    for (auto& name : name_vector) {
        if (name.empty()) {
            StringStream suggestion;
            suggestion << "field" << index;
            // Again, modify the name-vector in-place.
            name = disambiguate_name(suggestion.str());
            used_names.insert(name);
        }
        index++;
    }
}

}  // namespace tint::spirv::reader::ast_parser
