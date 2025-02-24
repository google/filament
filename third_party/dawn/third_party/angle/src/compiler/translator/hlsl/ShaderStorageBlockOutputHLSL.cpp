//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ShaderStorageBlockOutputHLSL: A traverser to translate a ssbo_access_chain to an offset of
// RWByteAddressBuffer.
//     //EOpIndexDirectInterfaceBlock
//     ssbo_variable :=
//       | the name of the SSBO
//       | the name of a variable in an SSBO backed interface block

//     // EOpIndexInDirect
//     // EOpIndexDirect
//     ssbo_array_indexing := ssbo_access_chain[expr_no_ssbo]

//     // EOpIndexDirectStruct
//     ssbo_structure_access := ssbo_access_chain.identifier

//     ssbo_access_chain :=
//       | ssbo_variable
//       | ssbo_array_indexing
//       | ssbo_structure_access
//

#include "compiler/translator/hlsl/ShaderStorageBlockOutputHLSL.h"

#include "common/span.h"
#include "compiler/translator/hlsl/ResourcesHLSL.h"
#include "compiler/translator/hlsl/blocklayoutHLSL.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/util.h"

namespace sh
{

namespace
{

constexpr const char kShaderStorageDeclarationString[] =
    "// @@ SHADER STORAGE DECLARATION STRING @@";

void GetBlockLayoutInfo(TIntermTyped *node,
                        bool rowMajorAlreadyAssigned,
                        TLayoutBlockStorage *storage,
                        bool *rowMajor)
{
    TIntermSwizzle *swizzleNode = node->getAsSwizzleNode();
    if (swizzleNode)
    {
        return GetBlockLayoutInfo(swizzleNode->getOperand(), rowMajorAlreadyAssigned, storage,
                                  rowMajor);
    }

    TIntermBinary *binaryNode = node->getAsBinaryNode();
    if (binaryNode)
    {
        switch (binaryNode->getOp())
        {
            case EOpIndexDirectInterfaceBlock:
            {
                // The column_major/row_major qualifier of a field member overrides the interface
                // block's row_major/column_major. So we can assign rowMajor here and don't need to
                // assign it again. But we still need to call recursively to get the storage's
                // value.
                const TType &type = node->getType();
                *rowMajor         = type.getLayoutQualifier().matrixPacking == EmpRowMajor;
                return GetBlockLayoutInfo(binaryNode->getLeft(), true, storage, rowMajor);
            }
            case EOpIndexIndirect:
            case EOpIndexDirect:
            case EOpIndexDirectStruct:
                return GetBlockLayoutInfo(binaryNode->getLeft(), rowMajorAlreadyAssigned, storage,
                                          rowMajor);
            default:
                UNREACHABLE();
                return;
        }
    }

    const TType &type = node->getType();
    ASSERT(type.getQualifier() == EvqBuffer);
    const TInterfaceBlock *interfaceBlock = type.getInterfaceBlock();
    ASSERT(interfaceBlock);
    *storage = interfaceBlock->blockStorage();
    // If the block doesn't have an instance name, rowMajorAlreadyAssigned will be false. In
    // this situation, we still need to set rowMajor's value.
    if (!rowMajorAlreadyAssigned)
    {
        *rowMajor = type.getLayoutQualifier().matrixPacking == EmpRowMajor;
    }
}

// It's possible that the current type has lost the original layout information. So we should pass
// the right layout information to GetBlockMemberInfoByType.
const BlockMemberInfo GetBlockMemberInfoByType(const TType &type,
                                               TLayoutBlockStorage storage,
                                               bool rowMajor)
{
    sh::Std140BlockEncoder std140Encoder;
    sh::Std430BlockEncoder std430Encoder;
    sh::HLSLBlockEncoder hlslEncoder(sh::HLSLBlockEncoder::ENCODE_PACKED, false);
    sh::BlockLayoutEncoder *encoder = nullptr;

    if (storage == EbsStd140)
    {
        encoder = &std140Encoder;
    }
    else if (storage == EbsStd430)
    {
        encoder = &std430Encoder;
    }
    else
    {
        encoder = &hlslEncoder;
    }

    std::vector<unsigned int> arraySizes;
    const angle::Span<const unsigned int> &typeArraySizes = type.getArraySizes();
    if (!typeArraySizes.empty())
    {
        arraySizes.assign(typeArraySizes.begin(), typeArraySizes.end());
    }
    return encoder->encodeType(GLVariableType(type), arraySizes, rowMajor);
}

const TField *GetFieldMemberInShaderStorageBlock(const TInterfaceBlock *interfaceBlock,
                                                 const ImmutableString &variableName)
{
    for (const TField *field : interfaceBlock->fields())
    {
        if (field->name() == variableName)
        {
            return field;
        }
    }
    return nullptr;
}

const InterfaceBlock *FindInterfaceBlock(const TInterfaceBlock *needle,
                                         const std::vector<InterfaceBlock> &haystack)
{
    for (const InterfaceBlock &block : haystack)
    {
        if (strcmp(block.name.c_str(), needle->name().data()) == 0)
        {
            ASSERT(block.fields.size() == needle->fields().size());
            return &block;
        }
    }

    UNREACHABLE();
    return nullptr;
}

std::string StripArrayIndices(const std::string &nameIn)
{
    std::string name = nameIn;
    size_t pos       = name.find('[');
    while (pos != std::string::npos)
    {
        size_t closePos = name.find(']', pos);
        ASSERT(closePos != std::string::npos);
        name.erase(pos, closePos - pos + 1);
        pos = name.find('[', pos);
    }
    ASSERT(name.find(']') == std::string::npos);
    return name;
}

// Does not include any array indices.
void MapVariableToField(const ShaderVariable &variable,
                        const TField *field,
                        std::string currentName,
                        ShaderVarToFieldMap *shaderVarToFieldMap)
{
    ASSERT((field->type()->getStruct() == nullptr) == variable.fields.empty());
    (*shaderVarToFieldMap)[currentName] = field;

    if (!variable.fields.empty())
    {
        const TStructure *subStruct = field->type()->getStruct();
        ASSERT(variable.fields.size() == subStruct->fields().size());

        for (size_t index = 0; index < variable.fields.size(); ++index)
        {
            const TField *subField            = subStruct->fields()[index];
            const ShaderVariable &subVariable = variable.fields[index];
            std::string subName               = currentName + "." + subVariable.name;
            MapVariableToField(subVariable, subField, subName, shaderVarToFieldMap);
        }
    }
}

class BlockInfoVisitor final : public BlockEncoderVisitor
{
  public:
    BlockInfoVisitor(const std::string &prefix,
                     TLayoutBlockStorage storage,
                     const ShaderVarToFieldMap &shaderVarToFieldMap,
                     BlockMemberInfoMap *blockInfoOut)
        : BlockEncoderVisitor(prefix, "", getEncoder(storage)),
          mShaderVarToFieldMap(shaderVarToFieldMap),
          mBlockInfoOut(blockInfoOut),
          mHLSLEncoder(HLSLBlockEncoder::ENCODE_PACKED, false),
          mStorage(storage)
    {}

    BlockLayoutEncoder *getEncoder(TLayoutBlockStorage storage)
    {
        switch (storage)
        {
            case EbsStd140:
                return &mStd140Encoder;
            case EbsStd430:
                return &mStd430Encoder;
            default:
                return &mHLSLEncoder;
        }
    }

    void enterStructAccess(const ShaderVariable &structVar, bool isRowMajor) override
    {
        BlockEncoderVisitor::enterStructAccess(structVar, isRowMajor);

        std::string variableName = StripArrayIndices(collapseNameStack());

        // Remove the trailing "."
        variableName.pop_back();

        BlockInfoVisitor childVisitor(variableName, mStorage, mShaderVarToFieldMap, mBlockInfoOut);
        childVisitor.getEncoder(mStorage)->enterAggregateType(structVar);
        TraverseShaderVariables(structVar.fields, isRowMajor, &childVisitor);
        childVisitor.getEncoder(mStorage)->exitAggregateType(structVar);

        int offset      = static_cast<int>(getEncoder(mStorage)->getCurrentOffset());
        int arrayStride = static_cast<int>(childVisitor.getEncoder(mStorage)->getCurrentOffset());

        auto iter = mShaderVarToFieldMap.find(variableName);
        if (iter == mShaderVarToFieldMap.end())
            return;

        const TField *structField = iter->second;
        if (mBlockInfoOut->count(structField) == 0)
        {
            mBlockInfoOut->emplace(structField, BlockMemberInfo(offset, arrayStride, -1, false));
        }
    }

    void encodeVariable(const ShaderVariable &variable,
                        const BlockMemberInfo &variableInfo,
                        const std::string &name,
                        const std::string &mappedName) override
    {
        auto iter = mShaderVarToFieldMap.find(StripArrayIndices(name));
        if (iter == mShaderVarToFieldMap.end())
            return;

        const TField *field = iter->second;
        if (mBlockInfoOut->count(field) == 0)
        {
            mBlockInfoOut->emplace(field, variableInfo);
        }
    }

  private:
    const ShaderVarToFieldMap &mShaderVarToFieldMap;
    BlockMemberInfoMap *mBlockInfoOut;
    Std140BlockEncoder mStd140Encoder;
    Std430BlockEncoder mStd430Encoder;
    HLSLBlockEncoder mHLSLEncoder;
    TLayoutBlockStorage mStorage;
};

void GetShaderStorageBlockMembersInfo(const TInterfaceBlock *interfaceBlock,
                                      const std::vector<InterfaceBlock> &shaderStorageBlocks,
                                      BlockMemberInfoMap *blockInfoOut)
{
    // Find the sh::InterfaceBlock.
    const InterfaceBlock *block = FindInterfaceBlock(interfaceBlock, shaderStorageBlocks);
    ASSERT(block);

    // Map ShaderVariable to TField.
    ShaderVarToFieldMap shaderVarToFieldMap;
    for (size_t index = 0; index < block->fields.size(); ++index)
    {
        const TField *field            = interfaceBlock->fields()[index];
        const ShaderVariable &variable = block->fields[index];
        MapVariableToField(variable, field, variable.name, &shaderVarToFieldMap);
    }

    BlockInfoVisitor visitor("", interfaceBlock->blockStorage(), shaderVarToFieldMap, blockInfoOut);
    TraverseShaderVariables(block->fields, false, &visitor);
}

TIntermTyped *Mul(TIntermTyped *left, TIntermTyped *right)
{
    return left && right ? new TIntermBinary(EOpMul, left, right) : nullptr;
}

TIntermTyped *Add(TIntermTyped *left, TIntermTyped *right)
{
    return left ? right ? new TIntermBinary(EOpAdd, left, right) : left : right;
}

}  // anonymous namespace

ShaderStorageBlockOutputHLSL::ShaderStorageBlockOutputHLSL(
    OutputHLSL *outputHLSL,
    ResourcesHLSL *resourcesHLSL,
    const std::vector<InterfaceBlock> &shaderStorageBlocks)
    : mOutputHLSL(outputHLSL),
      mResourcesHLSL(resourcesHLSL),
      mShaderStorageBlocks(shaderStorageBlocks)
{
    mSSBOFunctionHLSL = new ShaderStorageBlockFunctionHLSL;
}

ShaderStorageBlockOutputHLSL::~ShaderStorageBlockOutputHLSL()
{
    SafeDelete(mSSBOFunctionHLSL);
}

void ShaderStorageBlockOutputHLSL::outputStoreFunctionCallPrefix(TIntermTyped *node)
{
    traverseSSBOAccess(node, SSBOMethod::STORE);
}

void ShaderStorageBlockOutputHLSL::outputLoadFunctionCall(TIntermTyped *node)
{
    traverseSSBOAccess(node, SSBOMethod::LOAD);
    mOutputHLSL->getInfoSink() << ")";
}

void ShaderStorageBlockOutputHLSL::outputLengthFunctionCall(TIntermTyped *node)
{
    traverseSSBOAccess(node, SSBOMethod::LENGTH);
    mOutputHLSL->getInfoSink() << ")";
}

void ShaderStorageBlockOutputHLSL::outputAtomicMemoryFunctionCallPrefix(TIntermTyped *node,
                                                                        TOperator op)
{
    switch (op)
    {
        case EOpAtomicAdd:
            traverseSSBOAccess(node, SSBOMethod::ATOMIC_ADD);
            break;
        case EOpAtomicMin:
            traverseSSBOAccess(node, SSBOMethod::ATOMIC_MIN);
            break;
        case EOpAtomicMax:
            traverseSSBOAccess(node, SSBOMethod::ATOMIC_MAX);
            break;
        case EOpAtomicAnd:
            traverseSSBOAccess(node, SSBOMethod::ATOMIC_AND);
            break;
        case EOpAtomicOr:
            traverseSSBOAccess(node, SSBOMethod::ATOMIC_OR);
            break;
        case EOpAtomicXor:
            traverseSSBOAccess(node, SSBOMethod::ATOMIC_XOR);
            break;
        case EOpAtomicExchange:
            traverseSSBOAccess(node, SSBOMethod::ATOMIC_EXCHANGE);
            break;
        case EOpAtomicCompSwap:
            traverseSSBOAccess(node, SSBOMethod::ATOMIC_COMPSWAP);
            break;
        default:
            UNREACHABLE();
            break;
    }
}

// Note that we must calculate the matrix stride here instead of ShaderStorageBlockFunctionHLSL.
// It's because that if the current node's type is a vector which comes from a matrix, we will
// lose the matrix type info once we enter ShaderStorageBlockFunctionHLSL.
int ShaderStorageBlockOutputHLSL::getMatrixStride(TIntermTyped *node,
                                                  TLayoutBlockStorage storage,
                                                  bool rowMajor,
                                                  bool *isRowMajorMatrix) const
{
    if (node->getType().isMatrix())
    {
        *isRowMajorMatrix = rowMajor;
        return GetBlockMemberInfoByType(node->getType(), storage, rowMajor).matrixStride;
    }

    if (node->getType().isVector())
    {
        TIntermBinary *binaryNode = node->getAsBinaryNode();
        if (binaryNode)
        {
            return getMatrixStride(binaryNode->getLeft(), storage, rowMajor, isRowMajorMatrix);
        }
        else
        {
            TIntermSwizzle *swizzleNode = node->getAsSwizzleNode();
            if (swizzleNode)
            {
                return getMatrixStride(swizzleNode->getOperand(), storage, rowMajor,
                                       isRowMajorMatrix);
            }
        }
    }
    return 0;
}

void ShaderStorageBlockOutputHLSL::collectShaderStorageBlocks(TIntermTyped *node)
{
    TIntermSwizzle *swizzleNode = node->getAsSwizzleNode();
    if (swizzleNode)
    {
        return collectShaderStorageBlocks(swizzleNode->getOperand());
    }

    TIntermBinary *binaryNode = node->getAsBinaryNode();
    if (binaryNode)
    {
        switch (binaryNode->getOp())
        {
            case EOpIndexDirectInterfaceBlock:
            case EOpIndexIndirect:
            case EOpIndexDirect:
            case EOpIndexDirectStruct:
                return collectShaderStorageBlocks(binaryNode->getLeft());
            default:
                UNREACHABLE();
                return;
        }
    }

    const TIntermSymbol *symbolNode = node->getAsSymbolNode();
    const TType &type               = symbolNode->getType();
    ASSERT(type.getQualifier() == EvqBuffer);
    const TVariable &variable = symbolNode->variable();

    const TInterfaceBlock *interfaceBlock = type.getInterfaceBlock();
    ASSERT(interfaceBlock);
    if (mReferencedShaderStorageBlocks.count(interfaceBlock->uniqueId().get()) == 0)
    {
        const TVariable *instanceVariable = nullptr;
        if (type.isInterfaceBlock())
        {
            instanceVariable = &variable;
        }
        mReferencedShaderStorageBlocks[interfaceBlock->uniqueId().get()] =
            new TReferencedBlock(interfaceBlock, instanceVariable);
        GetShaderStorageBlockMembersInfo(interfaceBlock, mShaderStorageBlocks,
                                         &mBlockMemberInfoMap);
    }
}

void ShaderStorageBlockOutputHLSL::traverseSSBOAccess(TIntermTyped *node, SSBOMethod method)
{
    // TODO: Merge collectShaderStorageBlocks and GetBlockLayoutInfo to simplify the code.
    collectShaderStorageBlocks(node);

    // Note that we don't have correct BlockMemberInfo from mBlockMemberInfoMap at the current
    // point. But we must use those information to generate the right function name. So here we have
    // to calculate them again.
    TLayoutBlockStorage storage;
    bool rowMajor;
    GetBlockLayoutInfo(node, false, &storage, &rowMajor);
    int unsizedArrayStride = 0;
    if (node->getType().isUnsizedArray())
    {
        // The unsized array member must be the last member of a shader storage block.
        TIntermBinary *binaryNode = node->getAsBinaryNode();
        if (binaryNode)
        {
            const TInterfaceBlock *interfaceBlock =
                binaryNode->getLeft()->getType().getInterfaceBlock();
            ASSERT(interfaceBlock);
            const TIntermConstantUnion *index = binaryNode->getRight()->getAsConstantUnion();
            const TField *field               = interfaceBlock->fields()[index->getIConst(0)];
            auto fieldInfoIter                = mBlockMemberInfoMap.find(field);
            ASSERT(fieldInfoIter != mBlockMemberInfoMap.end());
            unsizedArrayStride = fieldInfoIter->second.arrayStride;
        }
        else
        {
            const TIntermSymbol *symbolNode       = node->getAsSymbolNode();
            const TVariable &variable             = symbolNode->variable();
            const TInterfaceBlock *interfaceBlock = symbolNode->getType().getInterfaceBlock();
            ASSERT(interfaceBlock);
            const TField *field =
                GetFieldMemberInShaderStorageBlock(interfaceBlock, variable.name());
            auto fieldInfoIter = mBlockMemberInfoMap.find(field);
            ASSERT(fieldInfoIter != mBlockMemberInfoMap.end());
            unsizedArrayStride = fieldInfoIter->second.arrayStride;
        }
    }
    bool isRowMajorMatrix = false;
    int matrixStride      = getMatrixStride(node, storage, rowMajor, &isRowMajorMatrix);

    const TString &functionName = mSSBOFunctionHLSL->registerShaderStorageBlockFunction(
        node->getType(), method, storage, isRowMajorMatrix, matrixStride, unsizedArrayStride,
        node->getAsSwizzleNode());
    TInfoSinkBase &out = mOutputHLSL->getInfoSink();
    out << functionName;
    out << "(";
    BlockMemberInfo blockMemberInfo;
    TIntermNode *loc = traverseNode(out, node, &blockMemberInfo);
    out << ", ";
    loc->traverse(mOutputHLSL);
}

void ShaderStorageBlockOutputHLSL::writeShaderStorageBlocksHeader(GLenum shaderType,
                                                                  TInfoSinkBase &out) const
{
    if (mReferencedShaderStorageBlocks.empty())
    {
        return;
    }

    mResourcesHLSL->allocateShaderStorageBlockRegisters(mReferencedShaderStorageBlocks);
    out << "// Shader Storage Blocks\n\n";
    if (shaderType == GL_COMPUTE_SHADER)
    {
        out << mResourcesHLSL->shaderStorageBlocksHeader(mReferencedShaderStorageBlocks);
    }
    else
    {
        out << kShaderStorageDeclarationString << "\n";
    }
    mSSBOFunctionHLSL->shaderStorageBlockFunctionHeader(out);
}

TIntermTyped *ShaderStorageBlockOutputHLSL::traverseNode(TInfoSinkBase &out,
                                                         TIntermTyped *node,
                                                         BlockMemberInfo *blockMemberInfo)
{
    if (TIntermSymbol *symbolNode = node->getAsSymbolNode())
    {
        const TVariable &variable = symbolNode->variable();
        const TType &type         = variable.getType();
        if (type.isInterfaceBlock())
        {
            out << DecorateVariableIfNeeded(variable);
        }
        else
        {
            const TInterfaceBlock *interfaceBlock = type.getInterfaceBlock();
            out << Decorate(interfaceBlock->name());
            const TField *field =
                GetFieldMemberInShaderStorageBlock(interfaceBlock, variable.name());
            return createFieldOffset(field, blockMemberInfo);
        }
    }
    else if (TIntermSwizzle *swizzleNode = node->getAsSwizzleNode())
    {
        return traverseNode(out, swizzleNode->getOperand(), blockMemberInfo);
    }
    else if (TIntermBinary *binaryNode = node->getAsBinaryNode())
    {
        switch (binaryNode->getOp())
        {
            case EOpIndexDirect:
            {
                const TType &leftType = binaryNode->getLeft()->getType();
                if (leftType.isInterfaceBlock())
                {
                    ASSERT(leftType.getQualifier() == EvqBuffer);
                    TIntermSymbol *instanceArraySymbol = binaryNode->getLeft()->getAsSymbolNode();

                    const int arrayIndex =
                        binaryNode->getRight()->getAsConstantUnion()->getIConst(0);
                    out << mResourcesHLSL->InterfaceBlockInstanceString(
                        instanceArraySymbol->getName(), arrayIndex);
                }
                else
                {
                    return writeEOpIndexDirectOrIndirectOutput(out, binaryNode, blockMemberInfo);
                }
                break;
            }
            case EOpIndexIndirect:
            {
                // We do not currently support indirect references to interface blocks
                ASSERT(binaryNode->getLeft()->getBasicType() != EbtInterfaceBlock);
                return writeEOpIndexDirectOrIndirectOutput(out, binaryNode, blockMemberInfo);
            }
            case EOpIndexDirectStruct:
            {
                // We do not currently support direct references to interface blocks
                ASSERT(binaryNode->getLeft()->getBasicType() != EbtInterfaceBlock);
                TIntermTyped *left = traverseNode(out, binaryNode->getLeft(), blockMemberInfo);
                const TStructure *structure       = binaryNode->getLeft()->getType().getStruct();
                const TIntermConstantUnion *index = binaryNode->getRight()->getAsConstantUnion();
                const TField *field               = structure->fields()[index->getIConst(0)];
                return Add(createFieldOffset(field, blockMemberInfo), left);
            }
            case EOpIndexDirectInterfaceBlock:
            {
                ASSERT(IsInShaderStorageBlock(binaryNode->getLeft()));
                traverseNode(out, binaryNode->getLeft(), blockMemberInfo);
                const TInterfaceBlock *interfaceBlock =
                    binaryNode->getLeft()->getType().getInterfaceBlock();
                const TIntermConstantUnion *index = binaryNode->getRight()->getAsConstantUnion();
                const TField *field               = interfaceBlock->fields()[index->getIConst(0)];
                return createFieldOffset(field, blockMemberInfo);
            }
            default:
                return nullptr;
        }
    }
    return nullptr;
}

TIntermTyped *ShaderStorageBlockOutputHLSL::writeEOpIndexDirectOrIndirectOutput(
    TInfoSinkBase &out,
    TIntermBinary *node,
    BlockMemberInfo *blockMemberInfo)
{
    ASSERT(IsInShaderStorageBlock(node->getLeft()));
    TIntermTyped *left  = traverseNode(out, node->getLeft(), blockMemberInfo);
    TIntermTyped *right = node->getRight()->deepCopy();
    const TType &type   = node->getLeft()->getType();
    TLayoutBlockStorage storage;
    bool rowMajor;
    GetBlockLayoutInfo(node, false, &storage, &rowMajor);

    if (type.isArray())
    {
        const angle::Span<const unsigned int> &arraySizes = type.getArraySizes();
        for (unsigned int i = 0; i < arraySizes.size() - 1; i++)
        {
            right = Mul(CreateUIntNode(arraySizes[i]), right);
        }
        right = Mul(CreateUIntNode(blockMemberInfo->arrayStride), right);
    }
    else if (type.isMatrix())
    {
        if (rowMajor)
        {
            right = Mul(CreateUIntNode(BlockLayoutEncoder::kBytesPerComponent), right);
        }
        else
        {
            right = Mul(CreateUIntNode(blockMemberInfo->matrixStride), right);
        }
    }
    else if (type.isVector())
    {
        if (blockMemberInfo->isRowMajorMatrix)
        {
            right = Mul(CreateUIntNode(blockMemberInfo->matrixStride), right);
        }
        else
        {
            right = Mul(CreateUIntNode(BlockLayoutEncoder::kBytesPerComponent), right);
        }
    }
    return Add(left, right);
}

TIntermTyped *ShaderStorageBlockOutputHLSL::createFieldOffset(const TField *field,
                                                              BlockMemberInfo *blockMemberInfo)
{
    auto fieldInfoIter = mBlockMemberInfoMap.find(field);
    ASSERT(fieldInfoIter != mBlockMemberInfoMap.end());
    *blockMemberInfo = fieldInfoIter->second;
    return CreateUIntNode(blockMemberInfo->offset);
}

}  // namespace sh
