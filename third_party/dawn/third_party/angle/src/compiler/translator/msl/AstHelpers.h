//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_MSL_ASTHELPERS_H_
#define COMPILER_TRANSLATOR_MSL_ASTHELPERS_H_

#include <cstring>
#include <unordered_map>
#include <unordered_set>

#include "common/span.h"
#include "compiler/translator/Name.h"
#include "compiler/translator/msl/IdGen.h"
#include "compiler/translator/msl/SymbolEnv.h"

namespace sh
{

// Creates a variable for a struct type.
const TVariable &CreateStructTypeVariable(TSymbolTable &symbolTable, const TStructure &structure);

// Creates a variable for a struct instance.
const TVariable &CreateInstanceVariable(
    TSymbolTable &symbolTable,
    const TStructure &structure,
    const Name &name,
    TQualifier qualifier                              = TQualifier::EvqTemporary,
    const angle::Span<const unsigned int> *arraySizes = nullptr);

// The input sequence should be discarded from AST after this is called.
TIntermSequence &CloneSequenceAndPrepend(const TIntermSequence &seq, TIntermNode &node);

// Appends parameters from `src` function to `dest` function.
void AddParametersFrom(TFunction &dest, const TFunction &src);

// Clones a function.
const TFunction &CloneFunction(TSymbolTable &symbolTable, IdGen &idGen, const TFunction &oldFunc);

// Clones a function and prepends the provided extr parameter.
// If `idGen` is null, the original function must be discarded from the AST.
const TFunction &CloneFunctionAndPrependParam(TSymbolTable &symbolTable,
                                              IdGen *idGen,
                                              const TFunction &oldFunc,
                                              const TVariable &newParam);

// Clones a function and prepends the provided two parameters.
// If `idGen` is null, the original function must be discarded from the AST.
const TFunction &CloneFunctionAndPrependTwoParams(TSymbolTable &symbolTable,
                                                  IdGen *idGen,
                                                  const TFunction &oldFunc,
                                                  const TVariable &newParam1,
                                                  const TVariable &newParam2);

// Clones a function and appends the provided extra parameters.
// If `idGen` is null, the original function must be discarded from the AST.
const TFunction &CloneFunctionAndAppendParams(TSymbolTable &symbolTable,
                                              IdGen *idGen,
                                              const TFunction &oldFunc,
                                              const std::vector<const TVariable *> &newParam);

// Clones a function and changes its return type.
// If `idGen` is null, the original function must be discarded from the AST.
const TFunction &CloneFunctionAndChangeReturnType(TSymbolTable &symbolTable,
                                                  IdGen *idGen,
                                                  const TFunction &oldFunc,
                                                  const TStructure &newReturn);

// Gets the argument of a function call at the given index.
TIntermTyped &GetArg(const TIntermAggregate &call, size_t index);

// Sets the argument of a function call at the given index.
void SetArg(TIntermAggregate &call, size_t index, TIntermTyped &arg);

// Accesses a field for the given node with the given field name.
// The node must be a struct instance.
TIntermBinary &AccessField(const TVariable &structInstanceVar, const Name &field);

// Accesses a field for the given node with the given field name.
// The node must be a struct instance.
TIntermBinary &AccessField(TIntermTyped &object, const Name &field);

// Accesses a field for the given node by its field index.
// The node must be a struct instance.
TIntermBinary &AccessFieldByIndex(TIntermTyped &object, int index);

// Accesses an element by index for the given node.
// The node must be an array, vector, or matrix.
TIntermBinary &AccessIndex(TIntermTyped &indexableNode, int index);

// Accesses an element by index for the given node if `index` is non-null.
// Returns the original node if `index` is null.
// The node must be an array, vector, or matrix if `index` is non-null.
TIntermTyped &AccessIndex(TIntermTyped &node, const int *index);

// Returns a subvector based on the input slice range.
// This returns the original node if the slice is an identity for the node.
TIntermTyped &SubVector(TIntermTyped &vectorNode, int begin, int end);

// Matches scalar bool, int, uint32_t, float.
bool IsScalarBasicType(const TType &type);

// Matches vector bool, int, uint32_t, float.
bool IsVectorBasicType(const TType &type);

// Matches bool, int, uint32_t, float.
// Type does not need to be a scalar.
bool HasScalarBasicType(const TType &type);

// Matches bool, int, uint32_t, float.
bool HasScalarBasicType(TBasicType type);

// Clones a type.
TType &CloneType(const TType &type);

// Clones a type and drops all array dimensions.
TType &InnermostType(const TType &type);

// Creates a vector type by dropping the columns off of a matrix type.
TType &DropColumns(const TType &matrixType);

// Creates a type by dropping the outer dimension off of an array type.
TType &DropOuterDimension(const TType &arrayType);

// Creates a scalar or vector type by changing the dimensions of a vector type.
TType &SetVectorDim(const TType &type, int newDim);

// Creates a matrix type by changing the row dimensions of a matrix type.
TType &SetMatrixRowDim(const TType &matrixType, int newDim);

// Returns true iff the structure directly contains a field with matrix type.
bool HasMatrixField(const TStructure &structure);

// Returns true iff the structure directly contains a field with array type.
bool HasArrayField(const TStructure &structure);

// Coerces `fromNode` to `toType` by a constructor call of `toType` if their types differ.
// Vector and matrix dimensions are retained.
// Array types are not allowed.
TIntermTyped &CoerceSimple(TBasicType toBasicType,
                           TIntermTyped &fromNode,
                           bool needsExplicitBoolCast);

// Coerces `fromNode` to `toType` by a constructor call of `toType` if their types differ.
// Vector and matrix dimensions must coincide between to and from.
// Array types are not allowed.
TIntermTyped &CoerceSimple(const TType &toType, TIntermTyped &fromNode, bool needsExplicitBoolCast);

TIntermTyped &AsType(SymbolEnv &symbolEnv, const TType &toType, TIntermTyped &fromNode);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_MSL_ASTHELPERS_H_
