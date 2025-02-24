//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// IntermNode_util.h: High-level utilities for creating AST nodes and node hierarchies. Mostly meant
// to be used in AST transforms.

#ifndef COMPILER_TRANSLATOR_INTERMNODEUTIL_H_
#define COMPILER_TRANSLATOR_INTERMNODEUTIL_H_

#include "compiler/translator/IntermNode.h"
#include "compiler/translator/tree_util/FindFunction.h"

namespace sh
{

class TSymbolTable;
class TVariable;

TIntermFunctionPrototype *CreateInternalFunctionPrototypeNode(const TFunction &func);
TIntermFunctionDefinition *CreateInternalFunctionDefinitionNode(const TFunction &func,
                                                                TIntermBlock *functionBody);

TIntermTyped *CreateZeroNode(const TType &type);
TIntermConstantUnion *CreateFloatNode(float value, TPrecision precision);
TIntermConstantUnion *CreateVecNode(const float values[],
                                    unsigned int vecSize,
                                    TPrecision precision);
TIntermConstantUnion *CreateUVecNode(const unsigned int values[],
                                     unsigned int vecSize,
                                     TPrecision precision);
TIntermConstantUnion *CreateIndexNode(int index);
TIntermConstantUnion *CreateUIntNode(unsigned int value);
TIntermConstantUnion *CreateBoolNode(bool value);

// Create temporary variable of known type |type|.
TVariable *CreateTempVariable(TSymbolTable *symbolTable, const TType *type);

// Create temporary variable compatible with user-provide type |type|.
// Possibly creates a new type. The qualifer of the new type of the new variable is |qualifier|.
TVariable *CreateTempVariable(TSymbolTable *symbolTable, const TType *type, TQualifier qualifier);

TIntermSymbol *CreateTempSymbolNode(const TVariable *tempVariable);
TIntermDeclaration *CreateTempDeclarationNode(const TVariable *tempVariable);
TIntermDeclaration *CreateTempInitDeclarationNode(const TVariable *tempVariable,
                                                  TIntermTyped *initializer);
TIntermBinary *CreateTempAssignmentNode(const TVariable *tempVariable, TIntermTyped *rightNode);

TVariable *DeclareTempVariable(TSymbolTable *symbolTable,
                               const TType *type,
                               TQualifier qualifier,
                               TIntermDeclaration **declarationOut);
TVariable *DeclareTempVariable(TSymbolTable *symbolTable,
                               TIntermTyped *initializer,
                               TQualifier qualifier,
                               TIntermDeclaration **declarationOut);
std::pair<const TVariable *, const TVariable *> DeclareStructure(
    TIntermBlock *root,
    TSymbolTable *symbolTable,
    TFieldList *fieldList,
    TQualifier qualifier,
    const TMemoryQualifier &memoryQualifier,
    uint32_t arraySize,
    const ImmutableString &structTypeName,
    const ImmutableString *structInstanceName);
const TVariable *DeclareInterfaceBlock(TIntermBlock *root,
                                       TSymbolTable *symbolTable,
                                       TFieldList *fieldList,
                                       TQualifier qualifier,
                                       const TLayoutQualifier &layoutQualifier,
                                       const TMemoryQualifier &memoryQualifier,
                                       uint32_t arraySize,
                                       const ImmutableString &blockTypeName,
                                       const ImmutableString &blockVariableName);

// If the input node is nullptr, return nullptr.
// If the input node is a block node, return it.
// If the input node is not a block node, put it inside a block node and return that.
TIntermBlock *EnsureBlock(TIntermNode *node);

// If the input node is nullptr, return a new block.
// If the input node is a block node, return it.
// If the input node is not a block node, put it inside a block node and return that.
TIntermBlock *EnsureLoopBodyBlock(TIntermNode *node);

// Should be called from inside Compiler::compileTreeImpl() where the global level is in scope.
TIntermSymbol *ReferenceGlobalVariable(const ImmutableString &name,
                                       const TSymbolTable &symbolTable);

TIntermSymbol *ReferenceBuiltInVariable(const ImmutableString &name,
                                        const TSymbolTable &symbolTable,
                                        int shaderVersion);

TIntermTyped *CreateBuiltInFunctionCallNode(const char *name,
                                            TIntermSequence *arguments,
                                            const TSymbolTable &symbolTable,
                                            int shaderVersion);
TIntermTyped *CreateBuiltInFunctionCallNode(const char *name,
                                            const std::initializer_list<TIntermNode *> &arguments,
                                            const TSymbolTable &symbolTable,
                                            int shaderVersion);
TIntermTyped *CreateBuiltInUnaryFunctionCallNode(const char *name,
                                                 TIntermTyped *argument,
                                                 const TSymbolTable &symbolTable,
                                                 int shaderVersion);

inline void GetSwizzleIndex(TVector<int> *indexOut) {}

template <typename T, typename... ArgsT>
void GetSwizzleIndex(TVector<int> *indexOut, T arg, ArgsT... args)
{
    indexOut->push_back(arg);
    GetSwizzleIndex(indexOut, args...);
}

template <typename... ArgsT>
TIntermSwizzle *CreateSwizzle(TIntermTyped *reference, ArgsT... args)
{
    TVector<int> swizzleIndex;
    GetSwizzleIndex(&swizzleIndex, args...);
    return new TIntermSwizzle(reference, swizzleIndex);
}

// Returns true if a block ends in a branch (break, continue, return, etc).  This is only correct
// after PruneNoOps, because it expects empty blocks after a branch to have been already pruned,
// i.e. a block can only end in a branch if its last statement is a branch or is a block ending in
// branch.
bool EndsInBranch(TIntermBlock *block);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_INTERMNODEUTIL_H_
