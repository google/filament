//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include <cstring>
#include <numeric>
#include <unordered_map>
#include <unordered_set>

#include "compiler/translator/msl/AstHelpers.h"

using namespace sh;

////////////////////////////////////////////////////////////////////////////////

const TVariable &sh::CreateStructTypeVariable(TSymbolTable &symbolTable,
                                              const TStructure &structure)
{
    TType *type    = new TType(&structure, true);
    TVariable *var = new TVariable(&symbolTable, ImmutableString(""), type, SymbolType::Empty);
    return *var;
}

const TVariable &sh::CreateInstanceVariable(TSymbolTable &symbolTable,
                                            const TStructure &structure,
                                            const Name &name,
                                            TQualifier qualifier,
                                            const angle::Span<const unsigned int> *arraySizes)
{
    TType *type = new TType(&structure, false);
    type->setQualifier(qualifier);
    if (arraySizes)
    {
        type->makeArrays(*arraySizes);
    }
    TVariable *var = new TVariable(&symbolTable, name.rawName(), type, name.symbolType());
    return *var;
}

static void AcquireFunctionExtras(TFunction &dest, const TFunction &src)
{
    if (src.isDefined())
    {
        dest.setDefined();
    }

    if (src.hasPrototypeDeclaration())
    {
        dest.setHasPrototypeDeclaration();
    }
}

TIntermSequence &sh::CloneSequenceAndPrepend(const TIntermSequence &seq, TIntermNode &node)
{
    TIntermSequence *newSeq = new TIntermSequence();
    newSeq->push_back(&node);

    for (TIntermNode *oldNode : seq)
    {
        newSeq->push_back(oldNode);
    }

    return *newSeq;
}

void sh::AddParametersFrom(TFunction &dest, const TFunction &src)
{
    const size_t paramCount = src.getParamCount();
    for (size_t i = 0; i < paramCount; ++i)
    {
        const TVariable *var = src.getParam(i);
        dest.addParameter(var);
    }
}

const TFunction &sh::CloneFunction(TSymbolTable &symbolTable,
                                   IdGen &idGen,
                                   const TFunction &oldFunc)
{
    ASSERT(oldFunc.symbolType() == SymbolType::UserDefined);

    Name newName = idGen.createNewName(Name(oldFunc));

    TFunction &newFunc =
        *new TFunction(&symbolTable, newName.rawName(), newName.symbolType(),
                       &oldFunc.getReturnType(), oldFunc.isKnownToNotHaveSideEffects());

    AcquireFunctionExtras(newFunc, oldFunc);
    AddParametersFrom(newFunc, oldFunc);

    return newFunc;
}

const TFunction &sh::CloneFunctionAndPrependParam(TSymbolTable &symbolTable,
                                                  IdGen *idGen,
                                                  const TFunction &oldFunc,
                                                  const TVariable &newParam)
{
    ASSERT(oldFunc.symbolType() == SymbolType::UserDefined ||
           oldFunc.symbolType() == SymbolType::AngleInternal);

    Name newName = idGen ? idGen->createNewName(Name(oldFunc)) : Name(oldFunc);

    TFunction &newFunc =
        *new TFunction(&symbolTable, newName.rawName(), newName.symbolType(),
                       &oldFunc.getReturnType(), oldFunc.isKnownToNotHaveSideEffects());

    AcquireFunctionExtras(newFunc, oldFunc);
    newFunc.addParameter(&newParam);
    AddParametersFrom(newFunc, oldFunc);

    return newFunc;
}

const TFunction &sh::CloneFunctionAndPrependTwoParams(TSymbolTable &symbolTable,
                                                      IdGen *idGen,
                                                      const TFunction &oldFunc,
                                                      const TVariable &newParam1,
                                                      const TVariable &newParam2)
{
    ASSERT(oldFunc.symbolType() == SymbolType::UserDefined ||
           oldFunc.symbolType() == SymbolType::AngleInternal);

    Name newName = idGen ? idGen->createNewName(Name(oldFunc)) : Name(oldFunc);

    TFunction &newFunc =
        *new TFunction(&symbolTable, newName.rawName(), newName.symbolType(),
                       &oldFunc.getReturnType(), oldFunc.isKnownToNotHaveSideEffects());

    AcquireFunctionExtras(newFunc, oldFunc);
    newFunc.addParameter(&newParam1);
    newFunc.addParameter(&newParam2);
    AddParametersFrom(newFunc, oldFunc);

    return newFunc;
}

const TFunction &sh::CloneFunctionAndAppendParams(TSymbolTable &symbolTable,
                                                  IdGen *idGen,
                                                  const TFunction &oldFunc,
                                                  const std::vector<const TVariable *> &newParams)
{
    ASSERT(oldFunc.symbolType() == SymbolType::UserDefined ||
           oldFunc.symbolType() == SymbolType::AngleInternal);

    Name newName = idGen ? idGen->createNewName(Name(oldFunc)) : Name(oldFunc);

    TFunction &newFunc =
        *new TFunction(&symbolTable, newName.rawName(), newName.symbolType(),
                       &oldFunc.getReturnType(), oldFunc.isKnownToNotHaveSideEffects());

    AcquireFunctionExtras(newFunc, oldFunc);
    AddParametersFrom(newFunc, oldFunc);
    for (const TVariable *param : newParams)
    {
        newFunc.addParameter(param);
    }

    return newFunc;
}

const TFunction &sh::CloneFunctionAndChangeReturnType(TSymbolTable &symbolTable,
                                                      IdGen *idGen,
                                                      const TFunction &oldFunc,
                                                      const TStructure &newReturn)
{
    ASSERT(oldFunc.symbolType() == SymbolType::UserDefined);

    Name newName = idGen ? idGen->createNewName(Name(oldFunc)) : Name(oldFunc);

    TType *newReturnType = new TType(&newReturn, true);
    TFunction &newFunc   = *new TFunction(&symbolTable, newName.rawName(), newName.symbolType(),
                                          newReturnType, oldFunc.isKnownToNotHaveSideEffects());

    AcquireFunctionExtras(newFunc, oldFunc);
    AddParametersFrom(newFunc, oldFunc);

    return newFunc;
}

TIntermTyped &sh::GetArg(const TIntermAggregate &call, size_t index)
{
    ASSERT(index < call.getChildCount());
    TIntermNode *arg = call.getChildNode(index);
    ASSERT(arg);
    TIntermTyped *targ = arg->getAsTyped();
    ASSERT(targ);
    return *targ;
}

void sh::SetArg(TIntermAggregate &call, size_t index, TIntermTyped &arg)
{
    ASSERT(index < call.getChildCount());
    (*call.getSequence())[index] = &arg;
}

TIntermBinary &sh::AccessField(const TVariable &structInstanceVar, const Name &name)
{
    return AccessField(*new TIntermSymbol(&structInstanceVar), name);
}

TIntermBinary &sh::AccessField(TIntermTyped &object, const Name &name)
{
    const TStructure *structure = object.getType().getStruct();
    ASSERT(structure);
    const TFieldList &fieldList = structure->fields();
    for (int i = 0; i < static_cast<int>(fieldList.size()); ++i)
    {
        TField *current = fieldList[i];
        if (Name(*current) == name)
        {
            return AccessFieldByIndex(object, i);
        }
    }
    UNREACHABLE();
    return AccessFieldByIndex(object, -1);
}

TIntermBinary &sh::AccessFieldByIndex(TIntermTyped &object, int index)
{
#if defined(ANGLE_ENABLE_ASSERTS)
    const TType &type = object.getType();
    ASSERT(!type.isArray());
    const TStructure *structure = type.getStruct();
    ASSERT(structure);
    ASSERT(0 <= index);
    ASSERT(static_cast<size_t>(index) < structure->fields().size());
#endif

    return *new TIntermBinary(
        TOperator::EOpIndexDirectStruct, &object,
        new TIntermConstantUnion(new TConstantUnion(index), *new TType(TBasicType::EbtInt)));
}

TIntermBinary &sh::AccessIndex(TIntermTyped &indexableNode, int index)
{
#if defined(ANGLE_ENABLE_ASSERTS)
    const TType &type = indexableNode.getType();
    ASSERT(type.isArray() || type.isVector() || type.isMatrix());
#endif

    TIntermBinary *accessNode = new TIntermBinary(
        TOperator::EOpIndexDirect, &indexableNode,
        new TIntermConstantUnion(new TConstantUnion(index), *new TType(TBasicType::EbtInt)));
    return *accessNode;
}

TIntermTyped &sh::AccessIndex(TIntermTyped &node, const int *index)
{
    if (index)
    {
        return AccessIndex(node, *index);
    }
    return node;
}

TIntermTyped &sh::SubVector(TIntermTyped &vectorNode, int begin, int end)
{
    ASSERT(vectorNode.getType().isVector());
    ASSERT(0 <= begin);
    ASSERT(end <= 4);
    ASSERT(begin <= end);
    if (begin == 0 && end == vectorNode.getType().getNominalSize())
    {
        return vectorNode;
    }
    TVector<int> offsets(static_cast<size_t>(end - begin));
    std::iota(offsets.begin(), offsets.end(), begin);
    TIntermSwizzle *swizzle = new TIntermSwizzle(vectorNode.deepCopy(), offsets);
    return *swizzle->fold(nullptr);  // Swizzles must always be folded to prevent double swizzles.
}

bool sh::IsScalarBasicType(const TType &type)
{
    if (!type.isScalar())
    {
        return false;
    }
    return HasScalarBasicType(type);
}

bool sh::IsVectorBasicType(const TType &type)
{
    if (!type.isVector())
    {
        return false;
    }
    return HasScalarBasicType(type);
}

bool sh::HasScalarBasicType(TBasicType type)
{
    switch (type)
    {
        case TBasicType::EbtFloat:
        case TBasicType::EbtInt:
        case TBasicType::EbtUInt:
        case TBasicType::EbtBool:
            return true;

        default:
            return false;
    }
}

bool sh::HasScalarBasicType(const TType &type)
{
    return HasScalarBasicType(type.getBasicType());
}

TType &sh::CloneType(const TType &type)
{
    TType &clone = *new TType(type);
    return clone;
}

TType &sh::InnermostType(const TType &type)
{
    TType &inner = *new TType(type);
    inner.toArrayBaseType();
    return inner;
}

TType &sh::DropColumns(const TType &matrixType)
{
    ASSERT(matrixType.isMatrix());
    ASSERT(HasScalarBasicType(matrixType));

    TType &vectorType = *new TType(matrixType);
    vectorType.toMatrixColumnType();
    return vectorType;
}

TType &sh::DropOuterDimension(const TType &arrayType)
{
    ASSERT(arrayType.isArray());

    TType &innerType = *new TType(arrayType);
    innerType.toArrayElementType();
    return innerType;
}

static TType &SetTypeDimsImpl(const TType &type, int primary, int secondary)
{
    ASSERT(1 < primary && primary <= 4);
    ASSERT(1 <= secondary && secondary <= 4);
    ASSERT(HasScalarBasicType(type));

    TType &newType = *new TType(type);
    newType.setPrimarySize(primary);
    newType.setSecondarySize(secondary);
    return newType;
}

TType &sh::SetVectorDim(const TType &type, int newDim)
{
    ASSERT(type.isRank0() || type.isVector());
    return SetTypeDimsImpl(type, newDim, 1);
}

TType &sh::SetMatrixRowDim(const TType &matrixType, int newDim)
{
    ASSERT(matrixType.isMatrix());
    ASSERT(1 < newDim && newDim <= 4);
    return SetTypeDimsImpl(matrixType, matrixType.getCols(), newDim);
}

bool sh::HasMatrixField(const TStructure &structure)
{
    for (const TField *field : structure.fields())
    {
        const TType &type = *field->type();
        if (type.isMatrix())
        {
            return true;
        }
    }
    return false;
}

bool sh::HasArrayField(const TStructure &structure)
{
    for (const TField *field : structure.fields())
    {
        const TType &type = *field->type();
        if (type.isArray())
        {
            return true;
        }
    }
    return false;
}

TIntermTyped &sh::CoerceSimple(TBasicType toBasicType,
                               TIntermTyped &fromNode,
                               bool needsExplicitBoolCast)
{
    const TType &fromType = fromNode.getType();

    ASSERT(HasScalarBasicType(toBasicType));
    ASSERT(HasScalarBasicType(fromType));
    ASSERT(!fromType.isArray());

    const TBasicType fromBasicType = fromType.getBasicType();

    if (toBasicType != fromBasicType)
    {
        if (toBasicType == TBasicType::EbtBool && fromNode.isVector() && needsExplicitBoolCast)
        {
            switch (fromBasicType)
            {
                case TBasicType::EbtFloat:
                case TBasicType::EbtInt:
                case TBasicType::EbtUInt:
                {
                    TIntermSequence *argsSequence = new TIntermSequence();
                    for (uint8_t i = 0; i < fromType.getNominalSize(); i++)
                    {
                        TIntermTyped &fromTypeSwizzle     = SubVector(fromNode, i, i + 1);
                        TIntermAggregate *boolConstructor = TIntermAggregate::CreateConstructor(
                            *new TType(toBasicType, 1, 1), new TIntermSequence{&fromTypeSwizzle});
                        argsSequence->push_back(boolConstructor);
                    }
                    return *TIntermAggregate::CreateConstructor(
                        *new TType(toBasicType, fromType.getNominalSize(),
                                   fromType.getSecondarySize()),
                        argsSequence);
                }

                default:
                    break;  // No explicit conversion needed
            }
        }

        return *TIntermAggregate::CreateConstructor(
            *new TType(toBasicType, fromType.getNominalSize(), fromType.getSecondarySize()),
            new TIntermSequence{&fromNode});
    }
    return fromNode;
}

TIntermTyped &sh::CoerceSimple(const TType &toType,
                               TIntermTyped &fromNode,
                               bool needsExplicitBoolCast)
{
    const TType &fromType = fromNode.getType();

    ASSERT(HasScalarBasicType(toType));
    ASSERT(HasScalarBasicType(fromType));
    ASSERT(toType.getNominalSize() == fromType.getNominalSize());
    ASSERT(toType.getSecondarySize() == fromType.getSecondarySize());
    ASSERT(!toType.isArray());
    ASSERT(!fromType.isArray());

    const TBasicType toBasicType   = toType.getBasicType();
    const TBasicType fromBasicType = fromType.getBasicType();

    if (toBasicType != fromBasicType)
    {
        if (toBasicType == TBasicType::EbtBool && fromNode.isVector() && needsExplicitBoolCast)
        {
            switch (fromBasicType)
            {
                case TBasicType::EbtFloat:
                case TBasicType::EbtInt:
                case TBasicType::EbtUInt:
                {
                    TIntermSequence *argsSequence = new TIntermSequence();
                    for (uint8_t i = 0; i < fromType.getNominalSize(); i++)
                    {
                        TIntermTyped &fromTypeSwizzle     = SubVector(fromNode, i, i + 1);
                        TIntermAggregate *boolConstructor = TIntermAggregate::CreateConstructor(
                            *new TType(toBasicType, 1, 1), new TIntermSequence{&fromTypeSwizzle});
                        argsSequence->push_back(boolConstructor);
                    }
                    return *TIntermAggregate::CreateConstructor(
                        *new TType(toBasicType, fromType.getNominalSize(),
                                   fromType.getSecondarySize()),
                        new TIntermSequence{*argsSequence});
                }

                default:
                    break;  // No explicit conversion needed
            }
        }

        return *TIntermAggregate::CreateConstructor(toType, new TIntermSequence{&fromNode});
    }
    return fromNode;
}

TIntermTyped &sh::AsType(SymbolEnv &symbolEnv, const TType &toType, TIntermTyped &fromNode)
{
    const TType &fromType = fromNode.getType();

    ASSERT(HasScalarBasicType(toType));
    ASSERT(HasScalarBasicType(fromType));
    ASSERT(!toType.isArray());
    ASSERT(!fromType.isArray());

    if (toType == fromType)
    {
        return fromNode;
    }
    TemplateArg targ(toType);
    return symbolEnv.callFunctionOverload(Name("as_type", SymbolType::BuiltIn), toType,
                                          *new TIntermSequence{&fromNode}, 1, &targ);
}
