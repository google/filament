//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// VaryingPacking:
//   Class which describes a mapping from varyings to registers, according
//   to the spec, or using custom packing algorithms. We also keep a register
//   allocation list for the D3D renderer.
//

#ifndef LIBANGLE_VARYINGPACKING_H_
#define LIBANGLE_VARYINGPACKING_H_

#include <GLSLANG/ShaderVars.h>

#include "angle_gl.h"
#include "common/angleutils.h"
#include "libANGLE/angletypes.h"

#include <map>

namespace gl
{
class InfoLog;
class ProgramExecutable;
struct Caps;
struct LinkingVariables;
struct ProgramVaryingRef;

using ProgramMergedVaryings = std::vector<ProgramVaryingRef>;

// A varying can have different names between stages if matched by the location layout qualifier.
// Additionally, same name varyings could still be of two identical struct types with different
// names.  This struct contains information on the varying in one of the two stages.  PackedVarying
// will thus contain two copies of this along with common information, such as interpolation or
// field index.
struct VaryingInShaderRef : angle::NonCopyable
{
    VaryingInShaderRef(ShaderType stageIn, const sh::ShaderVariable *varyingIn);
    VaryingInShaderRef(VaryingInShaderRef &&other);
    ~VaryingInShaderRef();

    VaryingInShaderRef &operator=(VaryingInShaderRef &&other);

    const sh::ShaderVariable *varying;

    ShaderType stage;

    // Struct name
    std::string parentStructName;
};

struct PackedVarying : angle::NonCopyable
{
    // Throughout this file, the "front" stage refers to the stage that outputs the varying, and the
    // "back" stage refers to the stage that takes the varying as input.  Note that this struct
    // contains linked varyings, which means both front and back stage varyings are valid, except
    // for the following which may have only one valid stage.
    //
    //  - transform-feedback-captured varyings
    //  - builtins
    //  - separable program stages,
    //
    PackedVarying(VaryingInShaderRef &&frontVaryingIn,
                  VaryingInShaderRef &&backVaryingIn,
                  sh::InterpolationType interpolationIn);
    PackedVarying(VaryingInShaderRef &&frontVaryingIn,
                  VaryingInShaderRef &&backVaryingIn,
                  sh::InterpolationType interpolationIn,
                  GLuint arrayIndexIn,
                  GLuint fieldIndexIn,
                  GLuint secondaryFieldIndexIn);
    PackedVarying(PackedVarying &&other);
    ~PackedVarying();

    PackedVarying &operator=(PackedVarying &&other);

    bool isStructField() const
    {
        return frontVarying.varying ? !frontVarying.parentStructName.empty()
                                    : !backVarying.parentStructName.empty();
    }

    bool isTransformFeedbackArrayElement() const
    {
        return isTransformFeedback && arrayIndex != GL_INVALID_INDEX;
    }

    // Return either front or back varying, whichever is available.  Only used when the name of the
    // varying is not important, but only the type is interesting.
    const sh::ShaderVariable &varying() const
    {
        return frontVarying.varying ? *frontVarying.varying : *backVarying.varying;
    }

    const std::string &getParentStructName() const
    {
        ASSERT(isStructField());
        return frontVarying.varying ? frontVarying.parentStructName : backVarying.parentStructName;
    }

    std::string fullName(ShaderType stage) const
    {
        ASSERT(stage == frontVarying.stage || stage == backVarying.stage);
        const VaryingInShaderRef &varying =
            stage == frontVarying.stage ? frontVarying : backVarying;

        std::stringstream fullNameStr;
        if (isStructField())
        {
            fullNameStr << varying.parentStructName << ".";
        }

        fullNameStr << varying.varying->name;
        if (arrayIndex != GL_INVALID_INDEX)
        {
            fullNameStr << "[" << arrayIndex << "]";
        }
        return fullNameStr.str();
    }

    // Transform feedback varyings can be only referenced in the VS.
    bool vertexOnly() const
    {
        return frontVarying.stage == ShaderType::Vertex && backVarying.varying == nullptr;
    }

    // Special handling for GS/TS array inputs.
    unsigned int getBasicTypeElementCount() const;

    VaryingInShaderRef frontVarying;
    VaryingInShaderRef backVarying;

    // Cached so we can store sh::ShaderVariable to point to varying fields.
    sh::InterpolationType interpolation;

    // Used by varyings that are captured with transform feedback, xor arrays of shader I/O blocks,
    // distinguished by isTransformFeedback;
    GLuint arrayIndex;
    bool isTransformFeedback;

    // Field index in the struct.  In Vulkan, this is used to assign a
    // struct-typed varying location to the location of its first field.
    GLuint fieldIndex;
    GLuint secondaryFieldIndex;
};

struct PackedVaryingRegister final
{
    PackedVaryingRegister()
        : packedVarying(nullptr),
          varyingArrayIndex(0),
          varyingRowIndex(0),
          registerRow(0),
          registerColumn(0)
    {}

    PackedVaryingRegister(const PackedVaryingRegister &)            = default;
    PackedVaryingRegister &operator=(const PackedVaryingRegister &) = default;

    bool operator<(const PackedVaryingRegister &other) const
    {
        return sortOrder() < other.sortOrder();
    }

    unsigned int sortOrder() const
    {
        // TODO(jmadill): Handle interpolation types
        return registerRow * 4 + registerColumn;
    }

    std::string tfVaryingName() const
    {
        return packedVarying->fullName(packedVarying->frontVarying.stage);
    }

    // Index to the array of varyings.
    const PackedVarying *packedVarying;

    // The array element of the packed varying.
    unsigned int varyingArrayIndex;

    // The row of the array element of the packed varying.
    unsigned int varyingRowIndex;

    // The register row to which we've assigned this packed varying.
    unsigned int registerRow;

    // The column of the register row into which we've packed this varying.
    unsigned int registerColumn;
};

// Supported packing modes:
enum class PackMode
{
    // We treat mat2 arrays as taking two full rows.
    WEBGL_STRICT,

    // We allow mat2 to take a 2x2 chunk.
    ANGLE_RELAXED,

    // Each varying takes a separate register. No register sharing.
    ANGLE_NON_CONFORMANT_D3D9,
};

enum class PerVertexMember
{
    // The gl_Pervertex struct is defined as:
    //
    //     out gl_PerVertex
    //     {
    //         vec4 gl_Position;
    //         float gl_PointSize;
    //         float gl_ClipDistance[];
    //         float gl_CullDistance[];
    //     };
    Position,
    PointSize,
    ClipDistance,
    CullDistance,

    EnumCount,
    InvalidEnum = EnumCount,
};
using PerVertexMemberBitSet = angle::PackedEnumBitSet<PerVertexMember, uint8_t>;

class VaryingPacking final : angle::NonCopyable
{
  public:
    VaryingPacking();
    ~VaryingPacking();

    [[nodiscard]] bool collectAndPackUserVaryings(InfoLog &infoLog,
                                                  GLint maxVaryingVectors,
                                                  PackMode packMode,
                                                  ShaderType frontShaderStage,
                                                  ShaderType backShaderStage,
                                                  const ProgramMergedVaryings &mergedVaryings,
                                                  const std::vector<std::string> &tfVaryings,
                                                  const bool isSeparableProgram);

    struct Register
    {
        Register() { data[0] = data[1] = data[2] = data[3] = false; }

        bool &operator[](unsigned int index) { return data[index]; }
        bool operator[](unsigned int index) const { return data[index]; }

        bool data[4];
    };

    Register &operator[](unsigned int index) { return mRegisterMap[index]; }
    const Register &operator[](unsigned int index) const { return mRegisterMap[index]; }

    const std::vector<PackedVaryingRegister> &getRegisterList() const { return mRegisterList; }
    unsigned int getMaxSemanticIndex() const
    {
        return static_cast<unsigned int>(mRegisterList.size());
    }

    const ShaderMap<std::vector<uint32_t>> &getInactiveVaryingIds() const
    {
        return mInactiveVaryingIds;
    }

    const ShaderMap<PerVertexMemberBitSet> &getOutputPerVertexActiveMembers() const
    {
        return mOutputPerVertexActiveMembers;
    }

    void reset();

  private:
    using VaryingUniqueFullNames = ShaderMap<std::set<std::string>>;

    // Register map functions.
    bool packUserVaryings(InfoLog &infoLog,
                          GLint maxVaryingVectors,
                          PackMode packMode,
                          const std::vector<PackedVarying> &packedVaryings);
    bool packVaryingIntoRegisterMap(PackMode packMode, const PackedVarying &packedVarying);
    bool isRegisterRangeFree(unsigned int registerRow,
                             unsigned int registerColumn,
                             unsigned int varyingRows,
                             unsigned int varyingColumns) const;
    void insertVaryingIntoRegisterMap(unsigned int registerRow,
                                      unsigned int registerColumn,
                                      unsigned int varyingColumns,
                                      const PackedVarying &packedVarying);
    void clearRegisterMap();

    // Collection functions.
    void collectUserVarying(const ProgramVaryingRef &ref, VaryingUniqueFullNames *uniqueFullNames);
    void collectUserVaryingField(const ProgramVaryingRef &ref,
                                 GLuint arrayIndex,
                                 GLuint fieldIndex,
                                 GLuint secondaryFieldIndex,
                                 VaryingUniqueFullNames *uniqueFullNames);
    void collectUserVaryingTF(const ProgramVaryingRef &ref, size_t subscript);
    void collectUserVaryingFieldTF(const ProgramVaryingRef &ref,
                                   const sh::ShaderVariable &field,
                                   GLuint fieldIndex,
                                   GLuint secondaryFieldIndex);
    void collectVarying(const sh::ShaderVariable &varying,
                        const ProgramVaryingRef &ref,
                        PackMode packMode,
                        VaryingUniqueFullNames *uniqueFullNames);
    void collectTFVarying(const std::string &tfVarying,
                          const ProgramVaryingRef &ref,
                          VaryingUniqueFullNames *uniqueFullNames);

    std::vector<Register> mRegisterMap;
    std::vector<PackedVaryingRegister> mRegisterList;
    std::vector<PackedVarying> mPackedVaryings;
    ShaderMap<std::vector<uint32_t>> mInactiveVaryingIds;
    ShaderMap<PerVertexMemberBitSet> mOutputPerVertexActiveMembers;
};

class ProgramVaryingPacking final : angle::NonCopyable
{
  public:
    ProgramVaryingPacking();
    ~ProgramVaryingPacking();

    const VaryingPacking &getInputPacking(ShaderType backShaderStage) const;
    const VaryingPacking &getOutputPacking(ShaderType frontShaderStage) const;

    [[nodiscard]] bool collectAndPackUserVaryings(InfoLog &infoLog,
                                                  const Caps &caps,
                                                  PackMode packMode,
                                                  const ShaderBitSet &activeShadersMask,
                                                  const ProgramMergedVaryings &mergedVaryings,
                                                  const std::vector<std::string> &tfVaryings,
                                                  bool isSeparableProgram);

  private:
    // Indexed by the front shader.
    ShaderMap<VaryingPacking> mVaryingPackings;

    // Looks up the front stage from the back stage.
    ShaderMap<ShaderType> mBackToFrontStageMap;
};

ProgramMergedVaryings GetMergedVaryingsFromLinkingVariables(
    const LinkingVariables &linkingVariables);
}  // namespace gl

#endif  // LIBANGLE_VARYINGPACKING_H_
