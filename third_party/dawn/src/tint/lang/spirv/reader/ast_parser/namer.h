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

#ifndef SRC_TINT_LANG_SPIRV_READER_AST_PARSER_NAMER_H_
#define SRC_TINT_LANG_SPIRV_READER_AST_PARSER_NAMER_H_

#include <string>
#include <unordered_map>
#include <vector>

#include "src/tint/lang/spirv/reader/ast_parser/fail_stream.h"

namespace tint::spirv::reader::ast_parser {

/// A Namer maps SPIR-V IDs to strings.
///
/// Sanitization:
/// Some names are user-suggested, but "sanitized" in the sense that an
/// unusual character (e.g. invalid for use in WGSL identifiers) is remapped
/// to a safer character such as an underscore.  Also, sanitized names
/// never start with an underscore.
class Namer {
  public:
    /// Creates a new namer
    /// @param fail_stream the error reporting stream
    explicit Namer(const FailStream& fail_stream);
    /// Destructor
    ~Namer();

    /// Sanitizes the given string, to replace unusual characters with
    /// obviously-valid idenfier characters. An empy string yields "empty".
    /// A sanitized name never starts with an underscore.
    /// @param suggested_name input string
    /// @returns sanitized name, suitable for use as an identifier
    static std::string Sanitize(const std::string& suggested_name);

    /// Registers a failure.
    /// @returns a fail stream to accumulate diagnostics.
    FailStream& Fail() { return fail_stream_.Fail(); }

    /// @param id the SPIR-V ID
    /// @returns true if we the given ID already has a registered name.
    bool HasName(uint32_t id) { return id_to_name_.find(id) != id_to_name_.end(); }

    /// @param name a string
    /// @returns true if the string has been registered as a name.
    bool IsRegistered(const std::string& name) const {
        return name_to_id_.find(name) != name_to_id_.end();
    }

    /// @param id the SPIR-V ID
    /// @returns the name for the ID. It must have been registered.
    const std::string& GetName(uint32_t id) const { return id_to_name_.find(id)->second; }

    /// Gets a unique name for the ID. If one already exists, then return
    /// that, otherwise synthesize a name and remember it for later.
    /// @param id the SPIR-V ID
    /// @returns a name for the given ID. Generates a name if non exists.
    const std::string& Name(uint32_t id) {
        if (!HasName(id)) {
            SuggestSanitizedName(id, "x_" + std::to_string(id));
        }
        return GetName(id);
    }

    /// Gets the registered name for a struct member. If no name has
    /// been registered for this member, then returns the empty string.
    /// member index is in bounds.
    /// @param id the SPIR-V ID of the struct type
    /// @param member_index the index of the member, counting from 0
    /// @returns the registered name for the ID, or an empty string if
    /// nothing has been registered.
    std::string GetMemberName(uint32_t id, uint32_t member_index) const;

    /// Returns an unregistered name based on a given base name.
    /// @param base_name the base name
    /// @returns a new name
    std::string FindUnusedDerivedName(const std::string& base_name);

    /// Returns a newly registered name based on a given base name.
    /// In the internal table `name_to_id_`, it is mapped to the invalid
    /// SPIR-V ID 0.  It does not have an entry in `id_to_name_`.
    /// @param base_name the base name
    /// @returns a new name
    std::string MakeDerivedName(const std::string& base_name);

    /// Records a mapping from the given ID to a name. Emits a failure
    /// if the ID already has a registered name.
    /// @param id the SPIR-V ID
    /// @param name the name to map to the ID
    /// @returns true if the ID did not have a previously registered name.
    bool Register(uint32_t id, const std::string& name);

    /// Registers a name, but not associated to any ID. Fails and emits
    /// a diagnostic if the name was already registered.
    /// @param name the name to register
    /// @returns true if the name was not already reegistered.
    bool RegisterWithoutId(const std::string& name);

    /// Saves a sanitized name for the given ID, if that ID does not yet
    /// have a registered name, and if the sanitized name has not already
    /// been registered to a different ID.
    /// @param id the SPIR-V ID
    /// @param suggested_name the suggested name
    /// @returns true if a name was newly registered for the ID
    bool SuggestSanitizedName(uint32_t id, const std::string& suggested_name);

    /// Saves a sanitized name for a member of a struct, if that member
    /// does not yet have a registered name.
    /// @param struct_id the SPIR-V ID for the struct
    /// @param member_index the index of the member inside the struct
    /// @param suggested_name the suggested name
    /// @returns true if a name was newly registered
    bool SuggestSanitizedMemberName(uint32_t struct_id,
                                    uint32_t member_index,
                                    const std::string& suggested_name);

    /// Ensure there are member names registered for members of the given struct
    /// such that:
    /// - Each member has a non-empty sanitized name.
    /// - No two members in the struct have the same name.
    /// @param struct_id the SPIR-V ID for the struct
    /// @param num_members the number of members in the struct
    void ResolveMemberNamesForStruct(uint32_t struct_id, uint32_t num_members);

  private:
    FailStream fail_stream_;

    // Maps an ID to its registered name.
    std::unordered_map<uint32_t, std::string> id_to_name_;
    // Maps a name to a SPIR-V ID, or 0 (the case for derived names).
    std::unordered_map<std::string, uint32_t> name_to_id_;

    // Maps a struct id and member index to a suggested sanitized name.
    // If entry k in the vector is an empty string, then a suggestion
    // was recorded for a higher-numbered index, but not for index k.
    std::unordered_map<uint32_t, std::vector<std::string>> struct_member_names_;

    // Saved search id suffix for a given base name. Used by
    // FindUnusedDerivedName().
    std::unordered_map<std::string, uint32_t> next_unusued_derived_name_id_;
};

}  // namespace tint::spirv::reader::ast_parser

#endif  // SRC_TINT_LANG_SPIRV_READER_AST_PARSER_NAMER_H_
