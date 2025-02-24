//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// IntermNode_util.cpp: High-level utilities for creating AST nodes and node hierarchies. Mostly
// meant to be used in AST transforms.

#include "compiler/translator/tree_util/IntermNode_util.h"

#include "compiler/translator/FunctionLookup.h"
#include "compiler/translator/SymbolTable.h"

namespace sh
{

namespace
{

const TFunction *LookUpBuiltInFunction(const char *name,
                                       const TIntermSequence *arguments,
                                       const TSymbolTable &symbolTable,
                                       int shaderVersion)
{
    const ImmutableString &mangledName = TFunctionLookup::GetMangledName(name, *arguments);
    const TSymbol *symbol              = symbolTable.findBuiltIn(mangledName, shaderVersion);
    if (symbol)
    {
        ASSERT(symbol->isFunction());
        return static_cast<const TFunction *>(symbol);
    }
    return nullptr;
}

}  // anonymous namespace

TIntermFunctionPrototype *CreateInternalFunctionPrototypeNode(const TFunction &func)
{
    return new TIntermFunctionPrototype(&func);
}

TIntermFunctionDefinition *CreateInternalFunctionDefinitionNode(const TFunction &func,
                                                                TIntermBlock *functionBody)
{
    return new TIntermFunctionDefinition(new TIntermFunctionPrototype(&func), functionBody);
}

TIntermTyped *CreateZeroNode(const TType &type)
{
    TType constType(type);
    constType.setQualifier(EvqConst);

    // Make sure as a constructor, the type does not inherit qualifiers that are otherwise specified
    // on interface blocks and varyings.
    constType.setInvariant(false);
    constType.setPrecise(false);
    constType.setInterpolant(false);
    constType.setMemoryQualifier(TMemoryQualifier::Create());
    constType.setLayoutQualifier(TLayoutQualifier::Create());
    constType.setInterfaceBlock(nullptr);

    if (!type.isArray() && type.getBasicType() != EbtStruct)
    {
        size_t size       = constType.getObjectSize();
        TConstantUnion *u = new TConstantUnion[size];
        for (size_t i = 0; i < size; ++i)
        {
            switch (type.getBasicType())
            {
                case EbtFloat:
                    u[i].setFConst(0.0f);
                    break;
                case EbtInt:
                    u[i].setIConst(0);
                    break;
                case EbtUInt:
                    u[i].setUConst(0u);
                    break;
                case EbtBool:
                    u[i].setBConst(false);
                    break;
                default:
                    // CreateZeroNode is called by ParseContext that keeps parsing even when an
                    // error occurs, so it is possible for CreateZeroNode to be called with
                    // non-basic types. This happens only on error condition but CreateZeroNode
                    // needs to return a value with the correct type to continue the type check.
                    // That's why we handle non-basic type by setting whatever value, we just need
                    // the type to be right.
                    u[i].setIConst(42);
                    break;
            }
        }

        TIntermConstantUnion *node = new TIntermConstantUnion(u, constType);
        return node;
    }

    TIntermSequence arguments;

    if (type.isArray())
    {
        TType elementType(type);
        elementType.toArrayElementType();

        size_t arraySize = type.getOutermostArraySize();
        for (size_t i = 0; i < arraySize; ++i)
        {
            arguments.push_back(CreateZeroNode(elementType));
        }
    }
    else
    {
        ASSERT(type.getBasicType() == EbtStruct);

        const TStructure *structure = type.getStruct();
        for (const auto &field : structure->fields())
        {
            arguments.push_back(CreateZeroNode(*field->type()));
        }
    }

    return TIntermAggregate::CreateConstructor(constType, &arguments);
}

TIntermConstantUnion *CreateFloatNode(float value, TPrecision precision)
{
    TConstantUnion *u = new TConstantUnion[1];
    u[0].setFConst(value);

    TType type(EbtFloat, precision, EvqConst, 1);
    return new TIntermConstantUnion(u, type);
}

TIntermConstantUnion *CreateVecNode(const float values[],
                                    unsigned int vecSize,
                                    TPrecision precision)
{
    TConstantUnion *u = new TConstantUnion[vecSize];
    for (unsigned int channel = 0; channel < vecSize; ++channel)
    {
        u[channel].setFConst(values[channel]);
    }

    TType type(EbtFloat, precision, EvqConst, static_cast<uint8_t>(vecSize));
    return new TIntermConstantUnion(u, type);
}

TIntermConstantUnion *CreateUVecNode(const unsigned int values[],
                                     unsigned int vecSize,
                                     TPrecision precision)
{
    TConstantUnion *u = new TConstantUnion[vecSize];
    for (unsigned int channel = 0; channel < vecSize; ++channel)
    {
        u[channel].setUConst(values[channel]);
    }

    TType type(EbtUInt, precision, EvqConst, static_cast<uint8_t>(vecSize));
    return new TIntermConstantUnion(u, type);
}

TIntermConstantUnion *CreateIndexNode(int index)
{
    TConstantUnion *u = new TConstantUnion[1];
    u[0].setIConst(index);

    TType type(EbtInt, EbpHigh, EvqConst, 1);
    return new TIntermConstantUnion(u, type);
}

TIntermConstantUnion *CreateUIntNode(unsigned int value)
{
    TConstantUnion *u = new TConstantUnion[1];
    u[0].setUConst(value);

    TType type(EbtUInt, EbpHigh, EvqConst, 1);
    return new TIntermConstantUnion(u, type);
}

TIntermConstantUnion *CreateBoolNode(bool value)
{
    TConstantUnion *u = new TConstantUnion[1];
    u[0].setBConst(value);

    TType type(EbtBool, EbpUndefined, EvqConst, 1);
    return new TIntermConstantUnion(u, type);
}

TVariable *CreateTempVariable(TSymbolTable *symbolTable, const TType *type)
{
    ASSERT(symbolTable != nullptr);
    return new TVariable(symbolTable, kEmptyImmutableString, type, SymbolType::AngleInternal);
}

TVariable *CreateTempVariable(TSymbolTable *symbolTable, const TType *type, TQualifier qualifier)
{
    ASSERT(symbolTable != nullptr);
    if (type->getQualifier() != qualifier || type->getInterfaceBlock() != nullptr)
    {
        TType *newType = new TType(*type);
        newType->setQualifier(qualifier);
        newType->setInterfaceBlock(nullptr);
        type = newType;
    }
    return new TVariable(symbolTable, kEmptyImmutableString, type, SymbolType::AngleInternal);
}

TIntermSymbol *CreateTempSymbolNode(const TVariable *tempVariable)
{
    ASSERT(tempVariable->symbolType() == SymbolType::AngleInternal);
    ASSERT(tempVariable->getType().getQualifier() == EvqTemporary ||
           tempVariable->getType().getQualifier() == EvqConst ||
           tempVariable->getType().getQualifier() == EvqGlobal);
    return new TIntermSymbol(tempVariable);
}

TIntermDeclaration *CreateTempDeclarationNode(const TVariable *tempVariable)
{
    TIntermDeclaration *tempDeclaration = new TIntermDeclaration();
    tempDeclaration->appendDeclarator(CreateTempSymbolNode(tempVariable));
    return tempDeclaration;
}

TIntermDeclaration *CreateTempInitDeclarationNode(const TVariable *tempVariable,
                                                  TIntermTyped *initializer)
{
    ASSERT(initializer != nullptr);
    TIntermSymbol *tempSymbol           = CreateTempSymbolNode(tempVariable);
    TIntermDeclaration *tempDeclaration = new TIntermDeclaration();
    TIntermBinary *tempInit             = new TIntermBinary(EOpInitialize, tempSymbol, initializer);
    tempDeclaration->appendDeclarator(tempInit);
    return tempDeclaration;
}

TIntermBinary *CreateTempAssignmentNode(const TVariable *tempVariable, TIntermTyped *rightNode)
{
    ASSERT(rightNode != nullptr);
    TIntermSymbol *tempSymbol = CreateTempSymbolNode(tempVariable);
    return new TIntermBinary(EOpAssign, tempSymbol, rightNode);
}

TVariable *DeclareTempVariable(TSymbolTable *symbolTable,
                               const TType *type,
                               TQualifier qualifier,
                               TIntermDeclaration **declarationOut)
{
    TVariable *variable = CreateTempVariable(symbolTable, type, qualifier);
    *declarationOut     = CreateTempDeclarationNode(variable);
    return variable;
}

TVariable *DeclareTempVariable(TSymbolTable *symbolTable,
                               TIntermTyped *initializer,
                               TQualifier qualifier,
                               TIntermDeclaration **declarationOut)
{
    TVariable *variable =
        CreateTempVariable(symbolTable, new TType(initializer->getType()), qualifier);
    *declarationOut = CreateTempInitDeclarationNode(variable, initializer);
    return variable;
}

std::pair<const TVariable *, const TVariable *> DeclareStructure(
    TIntermBlock *root,
    TSymbolTable *symbolTable,
    TFieldList *fieldList,
    TQualifier qualifier,
    const TMemoryQualifier &memoryQualifier,
    uint32_t arraySize,
    const ImmutableString &structTypeName,
    const ImmutableString *structInstanceName)
{
    TStructure *structure =
        new TStructure(symbolTable, structTypeName, fieldList, SymbolType::AngleInternal);

    auto makeStructureType = [&](bool isStructSpecifier) {
        TType *structureType = new TType(structure, isStructSpecifier);
        structureType->setQualifier(qualifier);
        structureType->setMemoryQualifier(memoryQualifier);
        if (arraySize > 0)
        {
            structureType->makeArray(arraySize);
        }
        return structureType;
    };

    TIntermSequence insertSequence;

    TVariable *typeVar = new TVariable(symbolTable, kEmptyImmutableString, makeStructureType(true),
                                       SymbolType::Empty);
    insertSequence.push_back(new TIntermDeclaration{typeVar});

    TVariable *instanceVar = nullptr;
    if (structInstanceName)
    {
        instanceVar = new TVariable(symbolTable, *structInstanceName, makeStructureType(false),
                                    SymbolType::AngleInternal);
        insertSequence.push_back(new TIntermDeclaration{instanceVar});
    }

    size_t firstFunctionIndex = FindFirstFunctionDefinitionIndex(root);
    root->insertChildNodes(firstFunctionIndex, insertSequence);

    return {typeVar, instanceVar};
}

const TVariable *DeclareInterfaceBlock(TIntermBlock *root,
                                       TSymbolTable *symbolTable,
                                       TFieldList *fieldList,
                                       TQualifier qualifier,
                                       const TLayoutQualifier &layoutQualifier,
                                       const TMemoryQualifier &memoryQualifier,
                                       uint32_t arraySize,
                                       const ImmutableString &blockTypeName,
                                       const ImmutableString &blockVariableName)
{
    // Define an interface block.
    TInterfaceBlock *interfaceBlock = new TInterfaceBlock(
        symbolTable, blockTypeName, fieldList, layoutQualifier, SymbolType::AngleInternal);

    // Turn the inteface block into a declaration.
    TType *interfaceBlockType = new TType(interfaceBlock, qualifier, layoutQualifier);
    interfaceBlockType->setMemoryQualifier(memoryQualifier);
    if (arraySize > 0)
    {
        interfaceBlockType->makeArray(arraySize);
    }

    TIntermDeclaration *interfaceBlockDecl = new TIntermDeclaration;
    TVariable *interfaceBlockVar =
        new TVariable(symbolTable, blockVariableName, interfaceBlockType,
                      blockVariableName.empty() ? SymbolType::Empty : SymbolType::AngleInternal);
    TIntermSymbol *interfaceBlockDeclarator = new TIntermSymbol(interfaceBlockVar);
    interfaceBlockDecl->appendDeclarator(interfaceBlockDeclarator);

    // Insert the declarations before the first function.
    TIntermSequence insertSequence;
    insertSequence.push_back(interfaceBlockDecl);

    size_t firstFunctionIndex = FindFirstFunctionDefinitionIndex(root);
    root->insertChildNodes(firstFunctionIndex, insertSequence);

    return interfaceBlockVar;
}

TIntermBlock *EnsureBlock(TIntermNode *node)
{
    if (node == nullptr)
        return nullptr;
    TIntermBlock *blockNode = node->getAsBlock();
    if (blockNode != nullptr)
    {
        return blockNode;
    }
    blockNode = new TIntermBlock();
    blockNode->setLine(node->getLine());
    blockNode->appendStatement(node);
    return blockNode;
}

TIntermBlock *EnsureLoopBodyBlock(TIntermNode *node)
{
    if (node == nullptr)
    {
        return new TIntermBlock();
    }
    return EnsureBlock(node);
}

TIntermSymbol *ReferenceGlobalVariable(const ImmutableString &name, const TSymbolTable &symbolTable)
{
    const TSymbol *symbol = symbolTable.findGlobal(name);
    ASSERT(symbol && symbol->isVariable());
    return new TIntermSymbol(static_cast<const TVariable *>(symbol));
}

TIntermSymbol *ReferenceBuiltInVariable(const ImmutableString &name,
                                        const TSymbolTable &symbolTable,
                                        int shaderVersion)
{
    const TVariable *var =
        static_cast<const TVariable *>(symbolTable.findBuiltIn(name, shaderVersion));
    ASSERT(var);
    return new TIntermSymbol(var);
}

TIntermTyped *CreateBuiltInFunctionCallNode(const char *name,
                                            TIntermSequence *arguments,
                                            const TSymbolTable &symbolTable,
                                            int shaderVersion)
{
    const TFunction *fn = LookUpBuiltInFunction(name, arguments, symbolTable, shaderVersion);
    ASSERT(fn);
    TOperator op = fn->getBuiltInOp();
    if (BuiltInGroup::IsMath(op) && arguments->size() == 1)
    {
        return new TIntermUnary(op, arguments->at(0)->getAsTyped(), fn);
    }
    return TIntermAggregate::CreateBuiltInFunctionCall(*fn, arguments);
}

TIntermTyped *CreateBuiltInFunctionCallNode(const char *name,
                                            const std::initializer_list<TIntermNode *> &arguments,
                                            const TSymbolTable &symbolTable,
                                            int shaderVersion)
{
    TIntermSequence argSequence(arguments);
    return CreateBuiltInFunctionCallNode(name, &argSequence, symbolTable, shaderVersion);
}

TIntermTyped *CreateBuiltInUnaryFunctionCallNode(const char *name,
                                                 TIntermTyped *argument,
                                                 const TSymbolTable &symbolTable,
                                                 int shaderVersion)
{
    return CreateBuiltInFunctionCallNode(name, {argument}, symbolTable, shaderVersion);
}

// Returns true if a block ends in a branch (break, continue, return, etc).  This is only correct
// after PruneNoOps, because it expects empty blocks after a branch to have been already pruned,
// i.e. a block can only end in a branch if its last statement is a branch or is a block ending in
// branch.
bool EndsInBranch(TIntermBlock *block)
{
    while (block != nullptr)
    {
        // Get the last statement of the block.
        TIntermSequence &statements = *block->getSequence();
        if (statements.empty())
        {
            return false;
        }

        TIntermNode *lastStatement = statements.back();

        // If it's a branch itself, we have the answer.
        if (lastStatement->getAsBranchNode())
        {
            return true;
        }

        // Otherwise, see if it's a block that ends in a branch
        block = lastStatement->getAsBlock();
    }

    return false;
}

}  // namespace sh
