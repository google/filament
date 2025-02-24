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

#include "libANGLE/VaryingPacking.h"

#include "common/CompiledShaderState.h"
#include "common/utilities.h"
#include "libANGLE/Program.h"
#include "libANGLE/ProgramExecutable.h"
#include "libANGLE/Shader.h"

namespace gl
{

namespace
{
// true if varying x has a higher priority in packing than y
bool ComparePackedVarying(const PackedVarying &x, const PackedVarying &y)
{
    // If the PackedVarying 'x' or 'y' to be compared is an array element for transform feedback,
    // this clones an equivalent non-array shader variable 'vx' or 'vy' for actual comparison
    // instead.  For I/O block arrays, the array index is used in the comparison.
    sh::ShaderVariable vx, vy;
    const sh::ShaderVariable *px, *py;

    px = &x.varying();
    py = &y.varying();

    if (x.isTransformFeedbackArrayElement())
    {
        vx = *px;
        vx.arraySizes.clear();
        px = &vx;
    }

    if (y.isTransformFeedbackArrayElement())
    {
        vy = *py;
        vy.arraySizes.clear();
        py = &vy;
    }

    // Make sure struct fields end up together.
    if (x.isStructField() != y.isStructField())
    {
        return x.isStructField();
    }

    if (x.isStructField())
    {
        ASSERT(y.isStructField());

        if (x.getParentStructName() != y.getParentStructName())
        {
            return x.getParentStructName() < y.getParentStructName();
        }
    }

    // For I/O block fields, order first by array index:
    if (!x.isTransformFeedbackArrayElement() && !y.isTransformFeedbackArrayElement())
    {
        if (x.arrayIndex != y.arrayIndex)
        {
            return x.arrayIndex < y.arrayIndex;
        }
    }

    // Then order by field index
    if (x.fieldIndex != y.fieldIndex)
    {
        return x.fieldIndex < y.fieldIndex;
    }

    // Then order by secondary field index
    if (x.secondaryFieldIndex != y.secondaryFieldIndex)
    {
        return x.secondaryFieldIndex < y.secondaryFieldIndex;
    }

    // Otherwise order by variable
    return gl::CompareShaderVar(*px, *py);
}

bool InterfaceVariablesMatch(const sh::ShaderVariable &front, const sh::ShaderVariable &back)
{
    // Matching ruels from 7.4.1 Shader Interface Matching from the GLES 3.2 spec:
    // - the two variables match in name, type, and qualification; or
    // - the two variables are declared with the same location qualifier and match in type and
    // qualification. Note that we use a more permissive check here thanks to front-end validation.
    if (back.location != -1 && back.location == front.location)
    {
        return true;
    }

    if (front.isShaderIOBlock != back.isShaderIOBlock)
    {
        return false;
    }

    // Compare names, or if shader I/O blocks, block names.
    const std::string &backName  = back.isShaderIOBlock ? back.structOrBlockName : back.name;
    const std::string &frontName = front.isShaderIOBlock ? front.structOrBlockName : front.name;
    return backName == frontName;
}

GLint GetMaxShaderInputVectors(const Caps &caps, ShaderType shaderStage)
{
    switch (shaderStage)
    {
        case ShaderType::TessControl:
            return caps.maxTessControlInputComponents / 4;
        case ShaderType::TessEvaluation:
            return caps.maxTessEvaluationInputComponents / 4;
        case ShaderType::Geometry:
            return caps.maxGeometryInputComponents / 4;
        case ShaderType::Fragment:
            return caps.maxFragmentInputComponents / 4;
        default:
            return std::numeric_limits<GLint>::max();
    }
}

GLint GetMaxShaderOutputVectors(const Caps &caps, ShaderType shaderStage)
{
    switch (shaderStage)
    {
        case ShaderType::Vertex:
            return caps.maxVertexOutputComponents / 4;
        case ShaderType::TessControl:
            return caps.maxTessControlOutputComponents / 4;
        case ShaderType::TessEvaluation:
            return caps.maxTessEvaluationOutputComponents / 4;
        case ShaderType::Geometry:
            return caps.maxGeometryOutputComponents / 4;
        default:
            return std::numeric_limits<GLint>::max();
    }
}

bool ShouldSkipPackedVarying(const sh::ShaderVariable &varying, PackMode packMode)
{
    // Don't pack gl_Position. Also don't count gl_PointSize for D3D9.
    // Additionally, gl_TessLevelInner and gl_TessLevelOuter should not be packed.
    return varying.name == "gl_Position" ||
           (varying.name == "gl_PointSize" && packMode == PackMode::ANGLE_NON_CONFORMANT_D3D9) ||
           varying.name == "gl_TessLevelInner" || varying.name == "gl_TessLevelOuter";
}

std::vector<unsigned int> StripVaryingArrayDimension(const sh::ShaderVariable *frontVarying,
                                                     ShaderType frontShaderStage,
                                                     const sh::ShaderVariable *backVarying,
                                                     ShaderType backShaderStage,
                                                     bool isStructField)
{
    // "Geometry shader inputs, tessellation control shader inputs and outputs, and tessellation
    // evaluation inputs all have an additional level of arrayness relative to other shader inputs
    // and outputs. This outer array level is removed from the type before considering how many
    // locations the type consumes."

    if (backVarying && backVarying->isArray() && !backVarying->isPatch && !isStructField &&
        (backShaderStage == ShaderType::Geometry || backShaderStage == ShaderType::TessEvaluation ||
         backShaderStage == ShaderType::TessControl))
    {
        std::vector<unsigned int> arr = backVarying->arraySizes;
        arr.pop_back();
        return arr;
    }

    if (frontVarying && frontVarying->isArray() && !frontVarying->isPatch && !isStructField &&
        frontShaderStage == ShaderType::TessControl)
    {
        std::vector<unsigned int> arr = frontVarying->arraySizes;
        arr.pop_back();
        return arr;
    }

    return frontVarying ? frontVarying->arraySizes : backVarying->arraySizes;
}

PerVertexMember GetPerVertexMember(const std::string &name)
{
    if (name == "gl_Position")
    {
        return PerVertexMember::Position;
    }
    if (name == "gl_PointSize")
    {
        return PerVertexMember::PointSize;
    }
    if (name == "gl_ClipDistance")
    {
        return PerVertexMember::ClipDistance;
    }
    if (name == "gl_CullDistance")
    {
        return PerVertexMember::CullDistance;
    }
    return PerVertexMember::InvalidEnum;
}

void SetActivePerVertexMembers(const sh::ShaderVariable *var, PerVertexMemberBitSet *bitset)
{
    ASSERT(var->isBuiltIn() && var->active);

    // Only process gl_Position, gl_PointSize, gl_ClipDistance, gl_CullDistance and the fields of
    // gl_in/out.
    if (var->fields.empty())
    {
        PerVertexMember member = GetPerVertexMember(var->name);
        // Skip gl_TessLevelInner/Outer etc.
        if (member != PerVertexMember::InvalidEnum)
        {
            bitset->set(member);
        }
        return;
    }

    // This must be gl_out.  Note that only `out gl_PerVertex` is processed; the input of the
    // next stage is implicitly identically active.
    ASSERT(var->name == "gl_out");
    for (const sh::ShaderVariable &field : var->fields)
    {
        bitset->set(GetPerVertexMember(field.name));
    }
}
}  // anonymous namespace

// Implementation of VaryingInShaderRef
VaryingInShaderRef::VaryingInShaderRef(ShaderType stageIn, const sh::ShaderVariable *varyingIn)
    : varying(varyingIn), stage(stageIn)
{}

VaryingInShaderRef::~VaryingInShaderRef() = default;

VaryingInShaderRef::VaryingInShaderRef(VaryingInShaderRef &&other)
    : varying(other.varying),
      stage(other.stage),
      parentStructName(std::move(other.parentStructName))
{}

VaryingInShaderRef &VaryingInShaderRef::operator=(VaryingInShaderRef &&other)
{
    std::swap(varying, other.varying);
    std::swap(stage, other.stage);
    std::swap(parentStructName, other.parentStructName);

    return *this;
}

// Implementation of PackedVarying
PackedVarying::PackedVarying(VaryingInShaderRef &&frontVaryingIn,
                             VaryingInShaderRef &&backVaryingIn,
                             sh::InterpolationType interpolationIn)
    : PackedVarying(std::move(frontVaryingIn),
                    std::move(backVaryingIn),
                    interpolationIn,
                    GL_INVALID_INDEX,
                    0,
                    0)
{}

PackedVarying::PackedVarying(VaryingInShaderRef &&frontVaryingIn,
                             VaryingInShaderRef &&backVaryingIn,
                             sh::InterpolationType interpolationIn,
                             GLuint arrayIndexIn,
                             GLuint fieldIndexIn,
                             GLuint secondaryFieldIndexIn)
    : frontVarying(std::move(frontVaryingIn)),
      backVarying(std::move(backVaryingIn)),
      interpolation(interpolationIn),
      arrayIndex(arrayIndexIn),
      isTransformFeedback(false),
      fieldIndex(fieldIndexIn),
      secondaryFieldIndex(secondaryFieldIndexIn)
{}

PackedVarying::~PackedVarying() = default;

PackedVarying::PackedVarying(PackedVarying &&other)
    : frontVarying(std::move(other.frontVarying)),
      backVarying(std::move(other.backVarying)),
      interpolation(other.interpolation),
      arrayIndex(other.arrayIndex),
      isTransformFeedback(other.isTransformFeedback),
      fieldIndex(other.fieldIndex),
      secondaryFieldIndex(other.secondaryFieldIndex)
{}

PackedVarying &PackedVarying::operator=(PackedVarying &&other)
{
    std::swap(frontVarying, other.frontVarying);
    std::swap(backVarying, other.backVarying);
    std::swap(interpolation, other.interpolation);
    std::swap(arrayIndex, other.arrayIndex);
    std::swap(isTransformFeedback, other.isTransformFeedback);
    std::swap(fieldIndex, other.fieldIndex);
    std::swap(secondaryFieldIndex, other.secondaryFieldIndex);

    return *this;
}

unsigned int PackedVarying::getBasicTypeElementCount() const
{
    // "Geometry shader inputs, tessellation control shader inputs and outputs, and tessellation
    // evaluation inputs all have an additional level of arrayness relative to other shader inputs
    // and outputs. This outer array level is removed from the type before considering how many
    // locations the type consumes."
    std::vector<unsigned int> arr =
        StripVaryingArrayDimension(frontVarying.varying, frontVarying.stage, backVarying.varying,
                                   backVarying.stage, isStructField());
    return arr.empty() ? 1u : arr.back();
}

// Implementation of VaryingPacking
VaryingPacking::VaryingPacking() = default;

VaryingPacking::~VaryingPacking() = default;

void VaryingPacking::reset()
{
    clearRegisterMap();
    mRegisterList.clear();
    mPackedVaryings.clear();

    for (std::vector<uint32_t> &inactiveVaryingIds : mInactiveVaryingIds)
    {
        inactiveVaryingIds.clear();
    }

    std::fill(mOutputPerVertexActiveMembers.begin(), mOutputPerVertexActiveMembers.end(),
              gl::PerVertexMemberBitSet{});
}

void VaryingPacking::clearRegisterMap()
{
    std::fill(mRegisterMap.begin(), mRegisterMap.end(), Register());
}

// Packs varyings into generic varying registers, using the algorithm from
// See [OpenGL ES Shading Language 1.00 rev. 17] appendix A section 7 page 111
// Also [OpenGL ES Shading Language 3.00 rev. 4] Section 11 page 119
// Returns false if unsuccessful.
bool VaryingPacking::packVaryingIntoRegisterMap(PackMode packMode,
                                                const PackedVarying &packedVarying)
{
    const sh::ShaderVariable &varying = packedVarying.varying();

    // "Non - square matrices of type matCxR consume the same space as a square matrix of type matN
    // where N is the greater of C and R."
    // Here we are a bit more conservative and allow packing non-square matrices more tightly.
    // Make sure we use transposed matrix types to count registers correctly.
    ASSERT(!varying.isStruct());
    GLenum transposedType       = gl::TransposeMatrixType(varying.type);
    unsigned int varyingRows    = gl::VariableRowCount(transposedType);
    unsigned int varyingColumns = gl::VariableColumnCount(transposedType);

    // Special pack mode for D3D9. Each varying takes a full register, no sharing.
    // TODO(jmadill): Implement more sophisticated component packing in D3D9.
    if (packMode == PackMode::ANGLE_NON_CONFORMANT_D3D9)
    {
        varyingColumns = 4;
    }

    // "Variables of type mat2 occupies 2 complete rows."
    // For non-WebGL contexts, we allow mat2 to occupy only two columns per row.
    else if (packMode == PackMode::WEBGL_STRICT && varying.type == GL_FLOAT_MAT2)
    {
        varyingColumns = 4;
    }

    // "Arrays of size N are assumed to take N times the size of the base type"
    // GLSL ES 3.10 section 4.3.6: Output variables cannot be arrays of arrays or arrays of
    // structures, so we may use getBasicTypeElementCount().
    const unsigned int elementCount = packedVarying.getBasicTypeElementCount();
    varyingRows *= (packedVarying.isTransformFeedbackArrayElement() ? 1 : elementCount);

    unsigned int maxVaryingVectors = static_cast<unsigned int>(mRegisterMap.size());

    // Fail if we are packing a single over-large varying.
    if (varyingRows > maxVaryingVectors)
    {
        return false;
    }

    // "For 2, 3 and 4 component variables packing is started using the 1st column of the 1st row.
    // Variables are then allocated to successive rows, aligning them to the 1st column."
    if (varyingColumns >= 2 && varyingColumns <= 4)
    {
        for (unsigned int row = 0; row <= maxVaryingVectors - varyingRows; ++row)
        {
            if (isRegisterRangeFree(row, 0, varyingRows, varyingColumns))
            {
                insertVaryingIntoRegisterMap(row, 0, varyingColumns, packedVarying);
                return true;
            }
        }

        // "For 2 component variables, when there are no spare rows, the strategy is switched to
        // using the highest numbered row and the lowest numbered column where the variable will
        // fit."
        if (varyingColumns == 2)
        {
            for (unsigned int r = maxVaryingVectors - varyingRows + 1; r-- >= 1;)
            {
                if (isRegisterRangeFree(r, 2, varyingRows, 2))
                {
                    insertVaryingIntoRegisterMap(r, 2, varyingColumns, packedVarying);
                    return true;
                }
            }
        }

        return false;
    }

    // "1 component variables have their own packing rule. They are packed in order of size, largest
    // first. Each variable is placed in the column that leaves the least amount of space in the
    // column and aligned to the lowest available rows within that column."
    ASSERT(varyingColumns == 1);
    unsigned int contiguousSpace[4]     = {0};
    unsigned int bestContiguousSpace[4] = {0};
    unsigned int totalSpace[4]          = {0};

    for (unsigned int row = 0; row < maxVaryingVectors; ++row)
    {
        for (unsigned int column = 0; column < 4; ++column)
        {
            if (mRegisterMap[row][column])
            {
                contiguousSpace[column] = 0;
            }
            else
            {
                contiguousSpace[column]++;
                totalSpace[column]++;

                if (contiguousSpace[column] > bestContiguousSpace[column])
                {
                    bestContiguousSpace[column] = contiguousSpace[column];
                }
            }
        }
    }

    unsigned int bestColumn = 0;
    for (unsigned int column = 1; column < 4; ++column)
    {
        if (bestContiguousSpace[column] >= varyingRows &&
            (bestContiguousSpace[bestColumn] < varyingRows ||
             totalSpace[column] < totalSpace[bestColumn]))
        {
            bestColumn = column;
        }
    }

    if (bestContiguousSpace[bestColumn] >= varyingRows)
    {
        for (unsigned int row = 0; row < maxVaryingVectors; row++)
        {
            if (isRegisterRangeFree(row, bestColumn, varyingRows, 1))
            {
                for (unsigned int arrayIndex = 0; arrayIndex < varyingRows; ++arrayIndex)
                {
                    // If varyingRows > 1, it must be an array.
                    PackedVaryingRegister registerInfo;
                    registerInfo.packedVarying  = &packedVarying;
                    registerInfo.registerRow    = row + arrayIndex;
                    registerInfo.registerColumn = bestColumn;
                    registerInfo.varyingArrayIndex =
                        (packedVarying.isTransformFeedbackArrayElement() ? packedVarying.arrayIndex
                                                                         : arrayIndex);
                    registerInfo.varyingRowIndex = 0;
                    // Do not record register info for builtins.
                    // TODO(jmadill): Clean this up.
                    if (!varying.isBuiltIn())
                    {
                        mRegisterList.push_back(registerInfo);
                    }
                    mRegisterMap[row + arrayIndex][bestColumn] = true;
                }
                break;
            }
        }
        return true;
    }

    return false;
}

bool VaryingPacking::isRegisterRangeFree(unsigned int registerRow,
                                         unsigned int registerColumn,
                                         unsigned int varyingRows,
                                         unsigned int varyingColumns) const
{
    for (unsigned int row = 0; row < varyingRows; ++row)
    {
        ASSERT(registerRow + row < mRegisterMap.size());
        for (unsigned int column = 0; column < varyingColumns; ++column)
        {
            ASSERT(registerColumn + column < 4);
            if (mRegisterMap[registerRow + row][registerColumn + column])
            {
                return false;
            }
        }
    }

    return true;
}

void VaryingPacking::insertVaryingIntoRegisterMap(unsigned int registerRow,
                                                  unsigned int registerColumn,
                                                  unsigned int varyingColumns,
                                                  const PackedVarying &packedVarying)
{
    unsigned int varyingRows = 0;

    const sh::ShaderVariable &varying = packedVarying.varying();
    ASSERT(!varying.isStruct());
    GLenum transposedType = gl::TransposeMatrixType(varying.type);
    varyingRows           = gl::VariableRowCount(transposedType);

    PackedVaryingRegister registerInfo;
    registerInfo.packedVarying  = &packedVarying;
    registerInfo.registerColumn = registerColumn;

    // GLSL ES 3.10 section 4.3.6: Output variables cannot be arrays of arrays or arrays of
    // structures, so we may use getBasicTypeElementCount().
    const unsigned int arrayElementCount = packedVarying.getBasicTypeElementCount();
    for (unsigned int arrayElement = 0; arrayElement < arrayElementCount; ++arrayElement)
    {
        if (packedVarying.isTransformFeedbackArrayElement() &&
            arrayElement != packedVarying.arrayIndex)
        {
            continue;
        }
        for (unsigned int varyingRow = 0; varyingRow < varyingRows; ++varyingRow)
        {
            registerInfo.registerRow     = registerRow + (arrayElement * varyingRows) + varyingRow;
            registerInfo.varyingRowIndex = varyingRow;
            registerInfo.varyingArrayIndex = arrayElement;
            // Do not record register info for builtins.
            // TODO(jmadill): Clean this up.
            if (!varying.isBuiltIn())
            {
                mRegisterList.push_back(registerInfo);
            }

            for (unsigned int columnIndex = 0; columnIndex < varyingColumns; ++columnIndex)
            {
                mRegisterMap[registerInfo.registerRow][registerColumn + columnIndex] = true;
            }
        }
    }
}

void VaryingPacking::collectUserVarying(const ProgramVaryingRef &ref,
                                        VaryingUniqueFullNames *uniqueFullNames)
{
    const sh::ShaderVariable *input  = ref.frontShader;
    const sh::ShaderVariable *output = ref.backShader;

    // Will get the vertex shader interpolation by default.
    sh::InterpolationType interpolation = input ? input->interpolation : output->interpolation;

    VaryingInShaderRef frontVarying(ref.frontShaderStage, input);
    VaryingInShaderRef backVarying(ref.backShaderStage, output);

    mPackedVaryings.emplace_back(std::move(frontVarying), std::move(backVarying), interpolation);
    if (input && !input->isBuiltIn())
    {
        (*uniqueFullNames)[ref.frontShaderStage].insert(
            mPackedVaryings.back().fullName(ref.frontShaderStage));
    }
    if (output && !output->isBuiltIn())
    {
        (*uniqueFullNames)[ref.backShaderStage].insert(
            mPackedVaryings.back().fullName(ref.backShaderStage));
    }
}

void VaryingPacking::collectUserVaryingField(const ProgramVaryingRef &ref,
                                             GLuint arrayIndex,
                                             GLuint fieldIndex,
                                             GLuint secondaryFieldIndex,
                                             VaryingUniqueFullNames *uniqueFullNames)
{
    const sh::ShaderVariable *input  = ref.frontShader;
    const sh::ShaderVariable *output = ref.backShader;

    // Will get the vertex shader interpolation by default.
    sh::InterpolationType interpolation = input ? input->interpolation : output->interpolation;

    const sh::ShaderVariable *frontField = input ? &input->fields[fieldIndex] : nullptr;
    const sh::ShaderVariable *backField  = output ? &output->fields[fieldIndex] : nullptr;

    if (secondaryFieldIndex != GL_INVALID_INDEX)
    {
        frontField = frontField ? &frontField->fields[secondaryFieldIndex] : nullptr;
        backField  = backField ? &backField->fields[secondaryFieldIndex] : nullptr;
    }

    VaryingInShaderRef frontVarying(ref.frontShaderStage, frontField);
    VaryingInShaderRef backVarying(ref.backShaderStage, backField);

    if (input)
    {
        if (frontField->isShaderIOBlock)
        {
            frontVarying.parentStructName = input->structOrBlockName;
        }
        else
        {
            ASSERT(!frontField->isStruct() && !frontField->isArray());
            frontVarying.parentStructName = input->name;
        }
    }
    if (output)
    {
        if (backField->isShaderIOBlock)
        {
            backVarying.parentStructName = output->structOrBlockName;
        }
        else
        {
            ASSERT(!backField->isStruct() && !backField->isArray());
            backVarying.parentStructName = output->name;
        }
    }

    mPackedVaryings.emplace_back(std::move(frontVarying), std::move(backVarying), interpolation,
                                 arrayIndex, fieldIndex,
                                 secondaryFieldIndex == GL_INVALID_INDEX ? 0 : secondaryFieldIndex);

    if (input)
    {
        (*uniqueFullNames)[ref.frontShaderStage].insert(
            mPackedVaryings.back().fullName(ref.frontShaderStage));
    }
    if (output)
    {
        (*uniqueFullNames)[ref.backShaderStage].insert(
            mPackedVaryings.back().fullName(ref.backShaderStage));
    }
}

void VaryingPacking::collectUserVaryingTF(const ProgramVaryingRef &ref, size_t subscript)
{
    const sh::ShaderVariable *input = ref.frontShader;

    VaryingInShaderRef frontVarying(ref.frontShaderStage, input);
    VaryingInShaderRef backVarying(ref.backShaderStage, nullptr);

    mPackedVaryings.emplace_back(std::move(frontVarying), std::move(backVarying),
                                 input->interpolation);
    mPackedVaryings.back().arrayIndex          = static_cast<GLuint>(subscript);
    mPackedVaryings.back().isTransformFeedback = true;
}

void VaryingPacking::collectUserVaryingFieldTF(const ProgramVaryingRef &ref,
                                               const sh::ShaderVariable &field,
                                               GLuint fieldIndex,
                                               GLuint secondaryFieldIndex)
{
    const sh::ShaderVariable *input = ref.frontShader;

    const sh::ShaderVariable *frontField = &field;
    if (secondaryFieldIndex != GL_INVALID_INDEX)
    {
        frontField = &frontField->fields[secondaryFieldIndex];
    }

    VaryingInShaderRef frontVarying(ref.frontShaderStage, frontField);
    VaryingInShaderRef backVarying(ref.backShaderStage, nullptr);

    if (frontField->isShaderIOBlock)
    {
        frontVarying.parentStructName = input->structOrBlockName;
    }
    else
    {
        ASSERT(!frontField->isStruct() && !frontField->isArray());
        frontVarying.parentStructName = input->name;
    }

    mPackedVaryings.emplace_back(std::move(frontVarying), std::move(backVarying),
                                 input->interpolation, GL_INVALID_INDEX, fieldIndex,
                                 secondaryFieldIndex == GL_INVALID_INDEX ? 0 : secondaryFieldIndex);
}

void VaryingPacking::collectVarying(const sh::ShaderVariable &varying,
                                    const ProgramVaryingRef &ref,
                                    PackMode packMode,
                                    VaryingUniqueFullNames *uniqueFullNames)
{
    const sh::ShaderVariable *input  = ref.frontShader;
    const sh::ShaderVariable *output = ref.backShader;

    if (varying.isStruct())
    {
        std::vector<unsigned int> arraySizes = StripVaryingArrayDimension(
            ref.frontShader, ref.frontShaderStage, ref.backShader, ref.backShaderStage, false);
        const bool isArray     = !arraySizes.empty();
        const GLuint arraySize = isArray ? arraySizes[0] : 1;

        for (GLuint arrayIndex = 0; arrayIndex < arraySize; ++arrayIndex)
        {
            const GLuint effectiveArrayIndex = isArray ? arrayIndex : GL_INVALID_INDEX;
            for (GLuint fieldIndex = 0; fieldIndex < varying.fields.size(); ++fieldIndex)
            {
                const sh::ShaderVariable &fieldVarying = varying.fields[fieldIndex];
                if (ShouldSkipPackedVarying(fieldVarying, packMode))
                {
                    continue;
                }

                if (fieldVarying.isStruct())
                {
                    if (fieldVarying.isArray())
                    {
                        unsigned int structFieldArraySize = fieldVarying.arraySizes[0];
                        for (unsigned int fieldArrayIndex = 0;
                             fieldArrayIndex < structFieldArraySize; ++fieldArrayIndex)
                        {
                            for (GLuint nestedIndex = 0; nestedIndex < fieldVarying.fields.size();
                                 nestedIndex++)
                            {
                                collectUserVaryingField(ref, effectiveArrayIndex, fieldIndex,
                                                        nestedIndex, uniqueFullNames);
                            }
                        }
                    }
                    else
                    {
                        for (GLuint nestedIndex = 0; nestedIndex < fieldVarying.fields.size();
                             nestedIndex++)
                        {
                            collectUserVaryingField(ref, effectiveArrayIndex, fieldIndex,
                                                    nestedIndex, uniqueFullNames);
                        }
                    }
                }
                else
                {
                    collectUserVaryingField(ref, effectiveArrayIndex, fieldIndex, GL_INVALID_INDEX,
                                            uniqueFullNames);
                }
            }
        }
        if (input)
        {
            (*uniqueFullNames)[ref.frontShaderStage].insert(input->name);
            if (input->isShaderIOBlock)
            {
                (*uniqueFullNames)[ref.frontShaderStage].insert(input->structOrBlockName);
            }
        }
        if (output)
        {
            (*uniqueFullNames)[ref.backShaderStage].insert(output->name);
        }
    }
    else
    {
        collectUserVarying(ref, uniqueFullNames);
    }
}

void VaryingPacking::collectTFVarying(const std::string &tfVarying,
                                      const ProgramVaryingRef &ref,
                                      VaryingUniqueFullNames *uniqueFullNames)
{
    const sh::ShaderVariable *input = ref.frontShader;

    std::vector<unsigned int> subscripts;
    std::string baseName = ParseResourceName(tfVarying, &subscripts);

    // Already packed as active varying.
    if ((*uniqueFullNames)[ref.frontShaderStage].count(tfVarying) > 0 ||
        (*uniqueFullNames)[ref.frontShaderStage].count(baseName) > 0 ||
        (input->isShaderIOBlock &&
         (*uniqueFullNames)[ref.frontShaderStage].count(input->structOrBlockName) > 0))
    {
        return;
    }

    if (input->isStruct())
    {
        GLuint fieldIndex               = 0;
        const sh::ShaderVariable *field = input->findField(tfVarying, &fieldIndex);
        if (field != nullptr)
        {
            ASSERT(input->isShaderIOBlock || (!field->isStruct() && !field->isArray()));

            // If it's an I/O block whose member is being captured, pack every member of the
            // block.  Currently, we pack either all or none of an I/O block.
            if (input->isShaderIOBlock)
            {
                for (fieldIndex = 0; fieldIndex < input->fields.size(); ++fieldIndex)
                {
                    if (input->fields[fieldIndex].isStruct())
                    {

                        for (GLuint nestedIndex = 0;
                             nestedIndex < input->fields[fieldIndex].fields.size(); nestedIndex++)
                        {
                            collectUserVaryingFieldTF(ref, input->fields[fieldIndex], fieldIndex,
                                                      nestedIndex);
                        }
                    }
                    else
                    {
                        collectUserVaryingFieldTF(ref, input->fields[fieldIndex], fieldIndex,
                                                  GL_INVALID_INDEX);
                    }
                }

                (*uniqueFullNames)[ref.frontShaderStage].insert(input->structOrBlockName);
            }
            else
            {
                collectUserVaryingFieldTF(ref, *field, fieldIndex, GL_INVALID_INDEX);
            }
            (*uniqueFullNames)[ref.frontShaderStage].insert(tfVarying);
            (*uniqueFullNames)[ref.frontShaderStage].insert(input->name);
        }
    }
    // Array as a whole and array element conflict has already been checked in
    // linkValidateTransformFeedback.
    else if (baseName == input->name)
    {
        size_t subscript = GL_INVALID_INDEX;
        if (!subscripts.empty())
        {
            subscript = subscripts.back();
        }

        // only pack varyings that are not builtins.
        if (tfVarying.compare(0, 3, "gl_") != 0)
        {
            collectUserVaryingTF(ref, subscript);
            (*uniqueFullNames)[ref.frontShaderStage].insert(tfVarying);
        }
    }
}

bool VaryingPacking::collectAndPackUserVaryings(gl::InfoLog &infoLog,
                                                GLint maxVaryingVectors,
                                                PackMode packMode,
                                                ShaderType frontShaderStage,
                                                ShaderType backShaderStage,
                                                const ProgramMergedVaryings &mergedVaryings,
                                                const std::vector<std::string> &tfVaryings,
                                                const bool isSeparableProgram)
{
    VaryingUniqueFullNames uniqueFullNames;

    reset();

    for (const ProgramVaryingRef &ref : mergedVaryings)
    {
        const sh::ShaderVariable *input  = ref.frontShader;
        const sh::ShaderVariable *output = ref.backShader;

        if ((input && ref.frontShaderStage != frontShaderStage) ||
            (output && ref.backShaderStage != backShaderStage))
        {
            continue;
        }

        const bool isActiveBuiltInInput  = input && input->isBuiltIn() && input->active;
        const bool isActiveBuiltInOutput = output && output->isBuiltIn() && output->active;

        if (isActiveBuiltInInput)
        {
            SetActivePerVertexMembers(input, &mOutputPerVertexActiveMembers[frontShaderStage]);
        }

        // Only pack statically used varyings that have a matched input or output, plus special
        // builtins. Note that we pack all statically used user-defined varyings even if they are
        // not active. GLES specs are a bit vague on whether it's allowed to only pack active
        // varyings, though GLES 3.1 spec section 11.1.2.1 says that "device-dependent
        // optimizations" may be used to make vertex shader outputs fit.
        //
        // When separable programs are linked, varyings at the separable program's boundary are
        // treated as active. See section 7.4.1 in
        // https://www.khronos.org/registry/OpenGL/specs/es/3.2/es_spec_3.2.pdf
        bool matchedInputOutputStaticUse = (input && output && output->staticUse);
        bool activeBuiltIn               = (isActiveBuiltInInput || isActiveBuiltInOutput);

        // Output variable in TCS can be read as input in another invocation by barrier.
        // See section 11.2.1.2.4 Tessellation Control Shader Execution Order in OpenGL ES 3.2.
        bool staticUseInTCS =
            (input && input->staticUse && ref.frontShaderStage == ShaderType::TessControl);

        // Separable program requirements
        bool separableActiveInput  = (input && (input->active || !output));
        bool separableActiveOutput = (output && (output->active || !input));
        bool separableActiveVarying =
            (isSeparableProgram && (separableActiveInput || separableActiveOutput));

        if (matchedInputOutputStaticUse || activeBuiltIn || separableActiveVarying ||
            staticUseInTCS)
        {
            const sh::ShaderVariable *varying = output ? output : input;

            if (!ShouldSkipPackedVarying(*varying, packMode))
            {
                collectVarying(*varying, ref, packMode, &uniqueFullNames);
                continue;
            }
        }

        // If the varying is not used in the input, we know it is inactive, unless it's a separable
        // program, in which case the input shader may not exist in this program.
        if (!input && !isSeparableProgram)
        {
            if (!output->isBuiltIn() && output->id != 0)
            {
                mInactiveVaryingIds[ref.backShaderStage].push_back(output->id);
            }
            continue;
        }

        // Keep Transform FB varyings in the merged list always.
        for (const std::string &tfVarying : tfVaryings)
        {
            collectTFVarying(tfVarying, ref, &uniqueFullNames);
        }

        if (input && !input->isBuiltIn())
        {
            const std::string &name =
                input->isShaderIOBlock ? input->structOrBlockName : input->name;
            if (uniqueFullNames[ref.frontShaderStage].count(name) == 0 && input->id != 0)
            {
                mInactiveVaryingIds[ref.frontShaderStage].push_back(input->id);
            }
        }
        if (output && !output->isBuiltIn())
        {
            const std::string &name =
                output->isShaderIOBlock ? output->structOrBlockName : output->name;
            if (uniqueFullNames[ref.backShaderStage].count(name) == 0 && output->id != 0)
            {
                mInactiveVaryingIds[ref.backShaderStage].push_back(output->id);
            }
        }
    }

    std::sort(mPackedVaryings.begin(), mPackedVaryings.end(), ComparePackedVarying);

    return packUserVaryings(infoLog, maxVaryingVectors, packMode, mPackedVaryings);
}

// See comment on packVarying.
bool VaryingPacking::packUserVaryings(gl::InfoLog &infoLog,
                                      GLint maxVaryingVectors,
                                      PackMode packMode,
                                      const std::vector<PackedVarying> &packedVaryings)
{
    clearRegisterMap();
    mRegisterMap.resize(maxVaryingVectors);

    // "Variables are packed into the registers one at a time so that they each occupy a contiguous
    // subrectangle. No splitting of variables is permitted."
    for (const PackedVarying &packedVarying : packedVaryings)
    {
        if (!packVaryingIntoRegisterMap(packMode, packedVarying))
        {
            ShaderType eitherStage = packedVarying.frontVarying.varying
                                         ? packedVarying.frontVarying.stage
                                         : packedVarying.backVarying.stage;
            infoLog << "Could not pack varying " << packedVarying.fullName(eitherStage);

            // TODO(jmadill): Implement more sophisticated component packing in D3D9.
            if (packMode == PackMode::ANGLE_NON_CONFORMANT_D3D9)
            {
                infoLog << "Note: Additional non-conformant packing restrictions are enforced on "
                           "D3D9.";
            }

            return false;
        }
    }

    // Sort the packed register list
    std::sort(mRegisterList.begin(), mRegisterList.end());

    return true;
}

// ProgramVaryingPacking implementation.
ProgramVaryingPacking::ProgramVaryingPacking() = default;

ProgramVaryingPacking::~ProgramVaryingPacking() = default;

const VaryingPacking &ProgramVaryingPacking::getInputPacking(ShaderType backShaderStage) const
{
    ShaderType frontShaderStage = mBackToFrontStageMap[backShaderStage];

    // If there's a missing shader stage, return the compute shader packing which is always empty.
    if (frontShaderStage == ShaderType::InvalidEnum)
    {
        ASSERT(mVaryingPackings[ShaderType::Compute].getMaxSemanticIndex() == 0);
        return mVaryingPackings[ShaderType::Compute];
    }

    return mVaryingPackings[frontShaderStage];
}

const VaryingPacking &ProgramVaryingPacking::getOutputPacking(ShaderType frontShaderStage) const
{
    return mVaryingPackings[frontShaderStage];
}

bool ProgramVaryingPacking::collectAndPackUserVaryings(InfoLog &infoLog,
                                                       const Caps &caps,
                                                       PackMode packMode,
                                                       const ShaderBitSet &activeShadersMask,
                                                       const ProgramMergedVaryings &mergedVaryings,
                                                       const std::vector<std::string> &tfVaryings,
                                                       bool isSeparableProgram)
{
    mBackToFrontStageMap.fill(ShaderType::InvalidEnum);

    ShaderBitSet activeShaders = activeShadersMask;

    ASSERT(activeShaders.any());
    ShaderType frontShaderStage     = activeShaders.first();
    activeShaders[frontShaderStage] = false;

    // Special case for start-after-vertex.
    if (frontShaderStage != ShaderType::Vertex)
    {
        ShaderType emulatedFrontShaderStage = ShaderType::Vertex;
        ShaderType backShaderStage          = frontShaderStage;

        if (!mVaryingPackings[emulatedFrontShaderStage].collectAndPackUserVaryings(
                infoLog, GetMaxShaderInputVectors(caps, backShaderStage), packMode,
                ShaderType::InvalidEnum, backShaderStage, mergedVaryings, tfVaryings,
                isSeparableProgram))
        {
            return false;
        }
        mBackToFrontStageMap[backShaderStage] = emulatedFrontShaderStage;
    }

    // Process input/output shader pairs.
    for (ShaderType backShaderStage : activeShaders)
    {
        GLint maxVaryingVectors;
        if (frontShaderStage == ShaderType::Vertex && backShaderStage == ShaderType::Fragment)
        {
            maxVaryingVectors = caps.maxVaryingVectors;
        }
        else
        {
            GLint outputVaryingsMax = GetMaxShaderOutputVectors(caps, frontShaderStage);
            GLint inputVaryingsMax  = GetMaxShaderInputVectors(caps, backShaderStage);
            maxVaryingVectors       = std::min(inputVaryingsMax, outputVaryingsMax);
        }

        ASSERT(maxVaryingVectors > 0 && maxVaryingVectors < std::numeric_limits<GLint>::max());

        if (!mVaryingPackings[frontShaderStage].collectAndPackUserVaryings(
                infoLog, maxVaryingVectors, packMode, frontShaderStage, backShaderStage,
                mergedVaryings, tfVaryings, isSeparableProgram))
        {
            return false;
        }

        mBackToFrontStageMap[backShaderStage] = frontShaderStage;
        frontShaderStage                      = backShaderStage;
    }

    // Special case for stop-before-fragment.
    if (frontShaderStage != ShaderType::Fragment)
    {
        if (!mVaryingPackings[frontShaderStage].collectAndPackUserVaryings(
                infoLog, GetMaxShaderOutputVectors(caps, frontShaderStage), packMode,
                frontShaderStage, ShaderType::InvalidEnum, mergedVaryings, tfVaryings,
                isSeparableProgram))
        {
            return false;
        }

        ShaderType emulatedBackShaderStage            = ShaderType::Fragment;
        mBackToFrontStageMap[emulatedBackShaderStage] = frontShaderStage;
    }

    return true;
}

ProgramMergedVaryings GetMergedVaryingsFromLinkingVariables(
    const LinkingVariables &linkingVariables)
{
    ShaderType frontShaderType = ShaderType::InvalidEnum;
    ProgramMergedVaryings merged;

    for (ShaderType backShaderType : kAllGraphicsShaderTypes)
    {
        if (!linkingVariables.isShaderStageUsedBitset[backShaderType])
        {
            continue;
        }
        const std::vector<sh::ShaderVariable> &backShaderOutputVaryings =
            linkingVariables.outputVaryings[backShaderType];
        const std::vector<sh::ShaderVariable> &backShaderInputVaryings =
            linkingVariables.inputVaryings[backShaderType];

        // Add outputs. These are always unmatched since we walk shader stages sequentially.
        for (const sh::ShaderVariable &frontVarying : backShaderOutputVaryings)
        {
            ProgramVaryingRef ref;
            ref.frontShader      = &frontVarying;
            ref.frontShaderStage = backShaderType;
            merged.push_back(ref);
        }

        if (frontShaderType == ShaderType::InvalidEnum)
        {
            // If this is our first shader stage, and not a VS, we might have unmatched inputs.
            for (const sh::ShaderVariable &backVarying : backShaderInputVaryings)
            {
                ProgramVaryingRef ref;
                ref.backShader      = &backVarying;
                ref.backShaderStage = backShaderType;
                merged.push_back(ref);
            }
        }
        else
        {
            // Match inputs with the prior shader stage outputs.
            for (const sh::ShaderVariable &backVarying : backShaderInputVaryings)
            {
                bool found = false;
                for (ProgramVaryingRef &ref : merged)
                {
                    if (ref.frontShader && ref.frontShaderStage == frontShaderType &&
                        InterfaceVariablesMatch(*ref.frontShader, backVarying))
                    {
                        ASSERT(ref.backShader == nullptr);

                        ref.backShader      = &backVarying;
                        ref.backShaderStage = backShaderType;
                        found               = true;
                        break;
                    }
                }

                // Some outputs are never matched, e.g. some builtin variables.
                if (!found)
                {
                    ProgramVaryingRef ref;
                    ref.backShader      = &backVarying;
                    ref.backShaderStage = backShaderType;
                    merged.push_back(ref);
                }
            }
        }

        // Save the current back shader to use as the next front shader.
        frontShaderType = backShaderType;
    }

    return merged;
}
}  // namespace gl
