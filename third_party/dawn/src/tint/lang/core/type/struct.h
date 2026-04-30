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

#ifndef SRC_TINT_LANG_CORE_TYPE_STRUCT_H_
#define SRC_TINT_LANG_CORE_TYPE_STRUCT_H_

#include <cstdint>
#include <optional>
#include <string>
#include <utility>

#include "src/tint/lang/core/enums.h"
#include "src/tint/lang/core/interpolation.h"
#include "src/tint/lang/core/io_attributes.h"
#include "src/tint/lang/core/type/node.h"
#include "src/tint/lang/core/type/type.h"
#include "src/tint/utils/containers/hashset.h"
#include "src/tint/utils/containers/vector.h"
#include "src/tint/utils/symbol/symbol.h"
#include "src/tint/utils/text/styled_text.h"

// Forward declarations
namespace tint::core::type {
class StructMember;
}  // namespace tint::core::type

namespace tint::core::type {

/// Metadata to capture how a structure is used in a shader module.
enum class PipelineStageUsage {
    kVertexInput,
    kVertexOutput,
    kFragmentInput,
    kFragmentOutput,
    kComputeInput,
    kComputeOutput,
};

/// StructFlag is an enumerator of struct flag bits, used by StructFlags.
enum StructFlag {
    /// The structure is a block-decorated structure (for SPIR-V or GLSL).
    kBlock,
    /// The structure requires explicit layout decorations
    kExplicitLayout,
};

/// An alias to tint::EnumSet<StructFlag>
using StructFlags = tint::EnumSet<StructFlag>;

/// Struct holds the Type information for structures.
class Struct : public Castable<Struct, Type> {
  public:
    /// Constructor
    /// Note: this constructs an empty structure, which should only be used find a struct with the
    /// same name in a type::Manager.
    /// @param name the name of the structure
    /// @param is_wgsl_internal `true` if the structure is an internally defined structure in WGSL
    Struct(Symbol name, bool is_wgsl_internal);

    /// Constructor
    /// @param name the name of the structure
    /// @param members the structure members
    /// @param size the byte size of the structure
    /// @param is_wgsl_internal `true` if the structure is an internally defined structure in WGSL
    /// alignment padding
    Struct(Symbol name,
           VectorRef<const StructMember*> members,
           uint32_t size,
           bool is_wgsl_internal = false);

    /// Destructor
    ~Struct() override;

    /// @param other the other node to compare against
    /// @returns true if the this type is equal to @p other
    bool Equals(const UniqueNode& other) const override;

    /// @returns the name of the structure
    Symbol Name() const { return name_; }

    /// Renames the structure
    /// @param name the new name for the structure
    void SetName(Symbol name) { name_ = name; }

    /// @returns the members of the structure
    VectorRef<const StructMember*> Members() const { return members_; }

    /// @param name the member name to look for
    /// @returns the member with the given name, or nullptr if it was not found.
    const StructMember* FindMember(Symbol name) const;

    /// @returns the byte alignment of the structure
    /// @note this may differ from the alignment of a structure member of this
    /// structure type, if the member is annotated with the `@align(n)`
    /// attribute.
    uint32_t Align() const override;

    /// @returns the byte size of the structure
    /// @note this may differ from the size of a structure member of this
    /// structure type, if the member is annotated with the `@size(n)`
    /// attribute.
    uint32_t Size() const override;

    /// @returns the byte size of the members without the end of structure
    /// alignment padding
    uint32_t SizeNoPadding() const;

    /// @returns the structure flags
    core::type::StructFlags StructFlags() const { return struct_flags_; }

    /// Set a structure flag.
    /// @param flag the flag to set
    void SetStructFlag(StructFlag flag) { struct_flags_.Add(flag); }

    /// Adds the AddressSpace usage to the structure.
    /// @param usage the storage usage
    void AddUsage(core::AddressSpace usage) { address_space_usage_.Add(usage); }

    /// @returns the set of address space uses of this structure
    const Hashset<core::AddressSpace, 1>& AddressSpaceUsage() const { return address_space_usage_; }

    /// @param usage the AddressSpace usage type to query
    /// @returns true iff this structure has been used as the given address space
    bool UsedAs(core::AddressSpace usage) const { return address_space_usage_.Contains(usage); }

    /// Adds the pipeline stage usage to the structure.
    /// @param usage the storage usage
    void AddUsage(PipelineStageUsage usage) { pipeline_stage_uses_.Add(usage); }

    /// @returns the set of entry point uses of this structure
    const Hashset<PipelineStageUsage, 1>& PipelineStageUses() const { return pipeline_stage_uses_; }

    /// @returns the name for this type that closely resembles how it would be
    /// declared in WGSL.
    std::string FriendlyName() const override;

    /// @returns a multiline string that describes the layout of this struct,
    /// including size and alignment information.
    StyledText Layout() const;

    /// @param concrete the conversion-rank ordered concrete versions of this abstract structure.
    void SetConcreteTypes(VectorRef<const Struct*> concrete) { concrete_types_ = concrete; }

    /// @returns the conversion-rank ordered concrete versions of this abstract structure, or an
    /// empty vector if this structure is not abstract.
    /// @note only structures returned by builtins may be abstract (e.g. modf, frexp)
    VectorRef<const Struct*> ConcreteTypes() const { return concrete_types_; }

    /// @copydoc Type::Elements
    TypeAndCount Elements(const Type* type_if_invalid = nullptr,
                          uint32_t count_if_invalid = 0) const override;

    /// @copydoc Type::Element
    const Type* Element(uint32_t index) const override;

    /// @returns `true` if this struct is internally defined in WGSL
    bool IsWgslInternal() const { return is_wgsl_internal_; }

    /// @param ctx the clone context
    /// @returns a clone of this type
    Struct* Clone(CloneContext& ctx) const override;

  private:
    Symbol name_;
    const tint::Vector<const StructMember*, 4> members_;
    const uint32_t size_;
    const bool is_wgsl_internal_;
    core::type::StructFlags struct_flags_;
    Hashset<core::AddressSpace, 1> address_space_usage_;
    Hashset<PipelineStageUsage, 1> pipeline_stage_uses_;
    tint::Vector<const Struct*, 2> concrete_types_;
};

/// StructMember holds the type information for structure members.
class StructMember : public Castable<StructMember, Node> {
  public:
    /// Constructor
    /// @param name the name of the structure member
    /// @param type the type of the member
    /// @param index the index of the member in the structure
    /// @param offset the byte offset from the base of the structure
    /// @param align the byte alignment of the member
    /// @param size the byte size of the member
    /// @param attributes the optional attributes
    StructMember(Symbol name,
                 const core::type::Type* type,
                 uint32_t index,
                 uint32_t offset,
                 uint32_t align,
                 uint32_t size,
                 const IOAttributes& attributes);

    /// Destructor
    ~StructMember() override;

    /// @returns the name of the structure member
    Symbol Name() const { return name_; }

    /// Sets the owning structure to `s`
    /// @param s the new structure owner
    void SetStruct(const Struct* s) { struct_ = s; }

    /// @returns the structure that owns this member
    const core::type::Struct* Struct() const { return struct_; }

    /// @returns the type of the member
    const core::type::Type* Type() const { return type_; }

    /// @returns the member index
    uint32_t Index() const { return index_; }

    /// @returns byte offset from base of structure
    uint32_t Offset() const { return offset_; }

    /// @returns the alignment of the member in bytes
    uint32_t Align() const { return align_; }

    /// @returns byte size
    uint32_t Size() const { return size_; }

    /// @returns the minimum size required for this struct member.
    uint32_t MinimumRequiredSize() const;

    /// @returns the optional attributes
    const IOAttributes& Attributes() const { return attributes_; }

    /// Sets the interpolation.
    /// @param interpolation the optional location interpolation settings
    void SetInterpolation(std::optional<core::Interpolation> interpolation) {
        attributes_.interpolation = interpolation;
    }

    /// Sets the location.
    /// @param loc the optional location value
    void SetLocation(std::optional<uint32_t> loc) { attributes_.location = loc; }

    /// Resets the attributes to empty
    void ResetAttributes() { attributes_ = {}; }

    /// Sets this member to be row major
    void SetRowMajor() { is_row_major_ = true; }
    /// Returns if this member is row major
    bool RowMajor() const { return is_row_major_; }

    /// Sets the matrix stride for the member
    void SetMatrixStride(uint32_t matrix_stride) { matrix_stride_ = matrix_stride; }
    /// Returns true if a matrix stride is set
    bool HasMatrixStride() const { return matrix_stride_ > 0; }
    /// Returns the matrix stride
    uint32_t MatrixStride() const { return matrix_stride_; }

    /// @param ctx the clone context
    /// @returns a clone of this struct member
    StructMember* Clone(CloneContext& ctx) const;

  private:
    const Symbol name_;
    const core::type::Struct* struct_;
    const core::type::Type* type_;
    const uint32_t index_;
    const uint32_t offset_;
    const uint32_t align_;
    const uint32_t size_;
    bool is_row_major_ = false;
    uint32_t matrix_stride_ = 0;
    IOAttributes attributes_;
};

}  // namespace tint::core::type

#endif  // SRC_TINT_LANG_CORE_TYPE_STRUCT_H_
