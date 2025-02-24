//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RecordUniformBlocksWithLargeArrayMember.h:
// Collect all uniform blocks which have one or more large array members,
// and the array sizes are greater than or equal to 50. If some of them
// satify some conditions, we will translate them to StructuredBuffers
// on Direct3D backend.
//

#include "compiler/translator/tree_ops/hlsl/RecordUniformBlocksWithLargeArrayMember.h"

#include "compiler/translator/Compiler.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{

namespace
{
// Only when a uniform block member's array size is greater than or equal to
// kMinArraySizeUseStructuredBuffer, then we may translate the uniform block
// to a StructuredBuffer on Direct3D backend.
const unsigned int kMinArraySizeUseStructuredBuffer = 50u;

// There is a maximum of D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT(128) slots that are
// available for shader resources on Direct3D 11. When shader version is 300, we only use
// D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT(16) slots for texture units. We allow StructuredBuffer
// to use the maximum of 60 slots, that is enough here.
const unsigned int kMaxAllowToUseRegisterCount = 60u;

// Traverser that all uniform blocks which have one or more large array members, and the array
// sizes are greater than or equal to 50.
class UniformBlocksWithLargeArrayMemberTraverser : public TIntermTraverser
{
  public:
    UniformBlocksWithLargeArrayMemberTraverser();

    void visitSymbol(TIntermSymbol *node) override;
    bool visitBinary(Visit visit, TIntermBinary *node) override;
    std::map<int, const TInterfaceBlock *> &getUniformBlockMayTranslation()
    {
        return mUniformBlockMayTranslation;
    }
    std::map<int, const TInterfaceBlock *> &getUniformBlockNotAllowTranslation()
    {
        return mUniformBlockNotAllowTranslation;
    }
    std::map<int, unsigned int> &getUniformBlockUsedRegisterCount()
    {
        return mUniformBlockUsedRegisterCount;
    }
    std::map<int, const TInterfaceBlock *> &getUniformBlockWithLargeArrayMember()
    {
        return mUniformBlockWithLargeArrayMember;
    }

  private:
    std::map<int, const TInterfaceBlock *> mUniformBlockMayTranslation;
    std::map<int, const TInterfaceBlock *> mUniformBlockNotAllowTranslation;
    std::map<int, unsigned int> mUniformBlockUsedRegisterCount;
    std::map<int, const TInterfaceBlock *> mUniformBlockWithLargeArrayMember;
};

UniformBlocksWithLargeArrayMemberTraverser::UniformBlocksWithLargeArrayMemberTraverser()
    : TIntermTraverser(true, true, false)
{}

static bool IsSupportedTypeForStructuredBuffer(const TType &type)
{
    const TStructure *structure              = type.getStruct();
    const TLayoutMatrixPacking matrixPacking = type.getLayoutQualifier().matrixPacking;
    if (structure)
    {
        const TFieldList &fields = structure->fields();
        for (size_t i = 0; i < fields.size(); i++)
        {
            const TType &fieldType = *fields[i]->type();
            // Do not allow the structure's member is array or structure.
            if (!fieldType.isArray() && !fieldType.getStruct() &&
                (fieldType.isScalar() || fieldType.isVector() ||
                 (fieldType.isMatrix() &&
                  ((matrixPacking != EmpRowMajor && fieldType.getRows() == 4) ||
                   (matrixPacking == EmpRowMajor && fieldType.getCols() == 4)))))
            {
                return true;
            }
        }
        return false;
    }
    else if (type.isMatrix())
    {
        // Only supports the matrix types that we do not need to pad in a structure or an array
        // explicitly.
        return (matrixPacking != EmpRowMajor && type.getRows() == 4) ||
               (matrixPacking == EmpRowMajor && type.getCols() == 4);
    }
    else
    {
        // Supports vector and scalar types in a structure or an array.
        return true;
    }
}

static bool CanTranslateUniformBlockToStructuredBuffer(const TInterfaceBlock &interfaceBlock)
{
    const TLayoutBlockStorage blockStorage = interfaceBlock.blockStorage();

    if (blockStorage == EbsStd140 && interfaceBlock.fields().size() == 1u)
    {
        const TType &fieldType = *interfaceBlock.fields()[0]->type();
        if (fieldType.getNumArraySizes() == 1u &&
            fieldType.getOutermostArraySize() >= kMinArraySizeUseStructuredBuffer)
        {
            return IsSupportedTypeForStructuredBuffer(fieldType);
        }
    }

    return false;
}

static bool FieldIsOrHasLargeArrayField(const TField &field)
{
    const TType *type = field.type();
    if (type->getArraySizeProduct() >= kMinArraySizeUseStructuredBuffer)
    {
        return true;
    }

    const TStructure *structure = type->getStruct();
    if (structure)
    {
        const TFieldList &fields = structure->fields();
        bool hasLargeArrayField  = false;
        for (size_t i = 0; i < fields.size(); i++)
        {
            hasLargeArrayField = FieldIsOrHasLargeArrayField(*fields[i]);
            if (hasLargeArrayField)
            {
                break;
            }
        }
        return hasLargeArrayField;
    }

    return false;
}

static bool IsInterfaceBlockWithLargeArrayField(const TInterfaceBlock &interfaceBlock)
{
    const TFieldList &fields = interfaceBlock.fields();
    bool isLargeArrayField   = false;
    for (size_t i = 0; i < fields.size(); i++)
    {
        isLargeArrayField = FieldIsOrHasLargeArrayField(*fields[i]);
        if (isLargeArrayField)
        {
            break;
        }
    }

    return isLargeArrayField;
}

void UniformBlocksWithLargeArrayMemberTraverser::visitSymbol(TIntermSymbol *node)
{
    const TVariable &variable = node->variable();
    const TType &variableType = variable.getType();
    TQualifier qualifier      = variable.getType().getQualifier();

    if (qualifier == EvqUniform)
    {
        const TInterfaceBlock *interfaceBlock = variableType.getInterfaceBlock();
        if (interfaceBlock)
        {
            if (CanTranslateUniformBlockToStructuredBuffer(*interfaceBlock))
            {
                if (mUniformBlockMayTranslation.count(interfaceBlock->uniqueId().get()) == 0)
                {
                    mUniformBlockMayTranslation[interfaceBlock->uniqueId().get()] = interfaceBlock;
                }

                if (!variableType.isInterfaceBlock())
                {
                    TIntermNode *accessor           = getAncestorNode(0);
                    TIntermBinary *accessorAsBinary = accessor->getAsBinaryNode();
                    // The uniform block variable is array type, only indexing operator is allowed
                    // to operate on the variable, otherwise do not translate the uniform block to
                    // HLSL StructuredBuffer.
                    if (!accessorAsBinary ||
                        !(accessorAsBinary && (accessorAsBinary->getOp() == EOpIndexDirect ||
                                               accessorAsBinary->getOp() == EOpIndexIndirect)))
                    {
                        if (mUniformBlockNotAllowTranslation.count(
                                interfaceBlock->uniqueId().get()) == 0)
                        {
                            mUniformBlockNotAllowTranslation[interfaceBlock->uniqueId().get()] =
                                interfaceBlock;
                        }
                    }
                    else
                    {
                        if (mUniformBlockUsedRegisterCount.count(
                                interfaceBlock->uniqueId().get()) == 0)
                        {
                            // The uniform block is not an instanced one, so it only uses one
                            // register.
                            mUniformBlockUsedRegisterCount[interfaceBlock->uniqueId().get()] = 1;
                        }
                    }
                }
                else
                {
                    if (mUniformBlockUsedRegisterCount.count(interfaceBlock->uniqueId().get()) == 0)
                    {
                        // The uniform block is an instanced one, the count of used registers
                        // depends on the array size of variable.
                        mUniformBlockUsedRegisterCount[interfaceBlock->uniqueId().get()] =
                            variableType.isArray() ? variableType.getOutermostArraySize() : 1;
                    }
                }
            }

            if (interfaceBlock->blockStorage() == EbsStd140 &&
                IsInterfaceBlockWithLargeArrayField(*interfaceBlock))
            {
                if (!variableType.isInterfaceBlock())
                {
                    TIntermNode *accessor           = getAncestorNode(0);
                    TIntermBinary *accessorAsBinary = accessor->getAsBinaryNode();
                    if (accessorAsBinary && (accessorAsBinary->getOp() == EOpIndexDirect ||
                                             accessorAsBinary->getOp() == EOpIndexIndirect))
                    {
                        if (mUniformBlockWithLargeArrayMember.count(
                                interfaceBlock->uniqueId().get()) == 0)
                        {
                            mUniformBlockWithLargeArrayMember[interfaceBlock->uniqueId().get()] =
                                interfaceBlock;
                        }
                    }
                }
            }
        }
    }
}

bool UniformBlocksWithLargeArrayMemberTraverser::visitBinary(Visit visit, TIntermBinary *node)
{
    switch (node->getOp())
    {
        case EOpIndexDirect:
        {
            if (visit == PreVisit)
            {
                const TType &leftType = node->getLeft()->getType();
                if (leftType.isInterfaceBlock())
                {
                    const TInterfaceBlock *interfaceBlock = leftType.getInterfaceBlock();
                    if (CanTranslateUniformBlockToStructuredBuffer(*interfaceBlock) &&
                        mUniformBlockMayTranslation.count(interfaceBlock->uniqueId().get()) == 0)
                    {
                        mUniformBlockMayTranslation[interfaceBlock->uniqueId().get()] =
                            interfaceBlock;
                        if (mUniformBlockUsedRegisterCount.count(
                                interfaceBlock->uniqueId().get()) == 0)
                        {
                            // The uniform block is an instanced one, the count of used registers
                            // depends on the array size of variable.
                            mUniformBlockUsedRegisterCount[interfaceBlock->uniqueId().get()] =
                                leftType.isArray() ? leftType.getOutermostArraySize() : 1;
                        }
                        return false;
                    }

                    if (interfaceBlock->blockStorage() == EbsStd140 &&
                        IsInterfaceBlockWithLargeArrayField(*interfaceBlock))
                    {
                        if (mUniformBlockWithLargeArrayMember.count(
                                interfaceBlock->uniqueId().get()) == 0)
                        {
                            mUniformBlockWithLargeArrayMember[interfaceBlock->uniqueId().get()] =
                                interfaceBlock;
                        }
                    }
                }
            }
            break;
        }
        case EOpIndexDirectInterfaceBlock:
        {
            if (visit == InVisit)
            {
                const TInterfaceBlock *interfaceBlock =
                    node->getLeft()->getType().getInterfaceBlock();
                if (CanTranslateUniformBlockToStructuredBuffer(*interfaceBlock))
                {
                    TIntermNode *accessor           = getAncestorNode(0);
                    TIntermBinary *accessorAsBinary = accessor->getAsBinaryNode();
                    // The uniform block variable is array type, only indexing operator is allowed
                    // to operate on the variable, otherwise do not translate the uniform block to
                    // HLSL StructuredBuffer.
                    if ((!accessorAsBinary ||
                         !(accessorAsBinary && (accessorAsBinary->getOp() == EOpIndexDirect ||
                                                accessorAsBinary->getOp() == EOpIndexIndirect))) &&
                        mUniformBlockNotAllowTranslation.count(interfaceBlock->uniqueId().get()) ==
                            0)
                    {
                        mUniformBlockNotAllowTranslation[interfaceBlock->uniqueId().get()] =
                            interfaceBlock;
                        return false;
                    }
                }

                if (interfaceBlock->blockStorage() == EbsStd140 &&
                    IsInterfaceBlockWithLargeArrayField(*interfaceBlock))
                {
                    TIntermNode *accessor           = getAncestorNode(0);
                    TIntermBinary *accessorAsBinary = accessor->getAsBinaryNode();
                    if (accessorAsBinary && (accessorAsBinary->getOp() == EOpIndexDirect ||
                                             accessorAsBinary->getOp() == EOpIndexIndirect))
                    {
                        if (mUniformBlockWithLargeArrayMember.count(
                                interfaceBlock->uniqueId().get()) == 0)
                        {
                            mUniformBlockWithLargeArrayMember[interfaceBlock->uniqueId().get()] =
                                interfaceBlock;
                        }
                    }
                }
            }
            break;
        }
        default:
            break;
    }

    return true;
}
}  // namespace

bool RecordUniformBlocksWithLargeArrayMember(
    TIntermNode *root,
    std::map<int, const TInterfaceBlock *> &uniformBlockOptimizedMap,
    std::set<std::string> &slowCompilingUniformBlockSet)
{
    UniformBlocksWithLargeArrayMemberTraverser traverser;
    root->traverse(&traverser);
    std::map<int, const TInterfaceBlock *> &uniformBlockMayTranslation =
        traverser.getUniformBlockMayTranslation();
    std::map<int, const TInterfaceBlock *> &uniformBlockNotAllowTranslation =
        traverser.getUniformBlockNotAllowTranslation();
    std::map<int, unsigned int> &uniformBlockUsedRegisterCount =
        traverser.getUniformBlockUsedRegisterCount();
    std::map<int, const TInterfaceBlock *> &uniformBlockWithLargeArrayMember =
        traverser.getUniformBlockWithLargeArrayMember();

    unsigned int usedRegisterCount = 0;
    for (auto &uniformBlock : uniformBlockMayTranslation)
    {
        if (uniformBlockNotAllowTranslation.count(uniformBlock.first) == 0)
        {
            usedRegisterCount += uniformBlockUsedRegisterCount[uniformBlock.first];
            if (usedRegisterCount > kMaxAllowToUseRegisterCount)
            {
                break;
            }
            uniformBlockOptimizedMap[uniformBlock.first] = uniformBlock.second;
        }
    }

    for (auto &uniformBlock : uniformBlockWithLargeArrayMember)
    {
        if (uniformBlockOptimizedMap.count(uniformBlock.first) == 0)
        {
            slowCompilingUniformBlockSet.insert(uniformBlock.second->name().data());
        }
    }

    return true;
}

}  // namespace sh
