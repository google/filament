//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// IntermNode_test.cpp:
//   Unit tests for the AST node classes.
//

#include "compiler/translator/IntermNode.h"
#include "angle_gl.h"
#include "compiler/translator/InfoSink.h"
#include "compiler/translator/PoolAlloc.h"
#include "compiler/translator/StaticType.h"
#include "compiler/translator/SymbolTable.h"
#include "gtest/gtest.h"

using namespace sh;

class IntermNodeTest : public testing::Test
{
  public:
    IntermNodeTest() : mUniqueIndex(0) {}

  protected:
    void SetUp() override
    {
        allocator.push();
        SetGlobalPoolAllocator(&allocator);
    }

    void TearDown() override
    {
        SetGlobalPoolAllocator(nullptr);
        allocator.pop();
    }

    TIntermSymbol *createTestSymbol(const TType &type)
    {
        std::stringstream symbolNameOut;
        symbolNameOut << "test" << mUniqueIndex;
        ImmutableString symbolName(symbolNameOut.str());
        ++mUniqueIndex;

        // We're using a mock symbol table here, don't need to assign proper symbol ids to these
        // nodes.
        TSymbolTable symbolTable;
        TType *variableType = new TType(type);
        variableType->setQualifier(EvqTemporary);
        TVariable *variable =
            new TVariable(&symbolTable, symbolName, variableType, SymbolType::AngleInternal);
        TIntermSymbol *node = new TIntermSymbol(variable);
        node->setLine(createUniqueSourceLoc());
        return node;
    }

    TIntermSymbol *createTestSymbol()
    {
        TType type(EbtFloat, EbpHigh);
        return createTestSymbol(type);
    }

    TFunction *createTestFunction(const TType &returnType, const TIntermSequence &args)
    {
        // We're using a mock symbol table similarly as for creating symbol nodes.
        const ImmutableString name("testFunc");
        TSymbolTable symbolTable;
        TFunction *func = new TFunction(&symbolTable, name, SymbolType::UserDefined,
                                        new TType(returnType), false);
        for (TIntermNode *arg : args)
        {
            const TType *type = new TType(arg->getAsTyped()->getType());
            func->addParameter(new TVariable(&symbolTable, ImmutableString("param"), type,
                                             SymbolType::UserDefined));
        }
        return func;
    }

    void checkTypeEqualWithQualifiers(const TType &original, const TType &copy)
    {
        ASSERT_EQ(original, copy);
        ASSERT_EQ(original.getPrecision(), copy.getPrecision());
        ASSERT_EQ(original.getQualifier(), copy.getQualifier());
    }

    void checkSymbolCopy(TIntermNode *aOriginal, TIntermNode *aCopy)
    {
        ASSERT_NE(aOriginal, aCopy);
        TIntermSymbol *copy     = aCopy->getAsSymbolNode();
        TIntermSymbol *original = aOriginal->getAsSymbolNode();
        ASSERT_NE(nullptr, copy);
        ASSERT_NE(nullptr, original);
        ASSERT_NE(original, copy);
        ASSERT_EQ(&original->variable(), &copy->variable());
        ASSERT_EQ(original->uniqueId(), copy->uniqueId());
        ASSERT_EQ(original->getName(), copy->getName());
        checkTypeEqualWithQualifiers(original->getType(), copy->getType());
        ASSERT_EQ(original->getLine().first_file, copy->getLine().first_file);
        ASSERT_EQ(original->getLine().first_line, copy->getLine().first_line);
        ASSERT_EQ(original->getLine().last_file, copy->getLine().last_file);
        ASSERT_EQ(original->getLine().last_line, copy->getLine().last_line);
    }

    TSourceLoc createUniqueSourceLoc()
    {
        TSourceLoc loc;
        loc.first_file = mUniqueIndex;
        loc.first_line = mUniqueIndex + 1;
        loc.last_file  = mUniqueIndex + 2;
        loc.last_line  = mUniqueIndex + 3;
        ++mUniqueIndex;
        return loc;
    }

    static TSourceLoc getTestSourceLoc()
    {
        TSourceLoc loc;
        loc.first_file = 1;
        loc.first_line = 2;
        loc.last_file  = 3;
        loc.last_line  = 4;
        return loc;
    }

    static void checkTestSourceLoc(const TSourceLoc &loc)
    {
        ASSERT_EQ(1, loc.first_file);
        ASSERT_EQ(2, loc.first_line);
        ASSERT_EQ(3, loc.last_file);
        ASSERT_EQ(4, loc.last_line);
    }

  private:
    angle::PoolAllocator allocator;
    int mUniqueIndex;
};

// Check that the deep copy of a symbol node is an actual copy with the same attributes as the
// original.
TEST_F(IntermNodeTest, DeepCopySymbolNode)
{
    const TType *type = StaticType::Get<EbtInt, EbpHigh, EvqTemporary, 1, 1>();

    // We're using a mock symbol table here, don't need to assign proper symbol ids to these nodes.
    TSymbolTable symbolTable;

    TVariable *variable =
        new TVariable(&symbolTable, ImmutableString("name"), type, SymbolType::AngleInternal);
    TIntermSymbol *original = new TIntermSymbol(variable);
    original->setLine(getTestSourceLoc());
    TIntermTyped *copy = original->deepCopy();
    checkSymbolCopy(original, copy);
    checkTestSourceLoc(copy->getLine());
}

// Check that the deep copy of a constant union node is an actual copy with the same attributes as
// the original.
TEST_F(IntermNodeTest, DeepCopyConstantUnionNode)
{
    TType type(EbtInt, EbpHigh);
    TConstantUnion *constValue = new TConstantUnion[1];
    constValue[0].setIConst(101);
    TIntermConstantUnion *original = new TIntermConstantUnion(constValue, type);
    original->setLine(getTestSourceLoc());
    TIntermTyped *copyTyped    = original->deepCopy();
    TIntermConstantUnion *copy = copyTyped->getAsConstantUnion();
    ASSERT_NE(nullptr, copy);
    ASSERT_NE(original, copy);
    checkTestSourceLoc(copy->getLine());
    checkTypeEqualWithQualifiers(original->getType(), copy->getType());
    ASSERT_EQ(101, copy->getIConst(0));
}

// Check that the deep copy of a binary node is an actual copy with the same attributes as the
// original. Child nodes also need to be copies with the same attributes as the original children.
TEST_F(IntermNodeTest, DeepCopyBinaryNode)
{
    TType type(EbtFloat, EbpHigh);

    TIntermBinary *original = new TIntermBinary(EOpAdd, createTestSymbol(), createTestSymbol());
    original->setLine(getTestSourceLoc());
    TIntermTyped *copyTyped = original->deepCopy();
    TIntermBinary *copy     = copyTyped->getAsBinaryNode();
    ASSERT_NE(nullptr, copy);
    ASSERT_NE(original, copy);
    checkTestSourceLoc(copy->getLine());
    checkTypeEqualWithQualifiers(original->getType(), copy->getType());

    checkSymbolCopy(original->getLeft(), copy->getLeft());
    checkSymbolCopy(original->getRight(), copy->getRight());
}

// Check that the deep copy of a unary node is an actual copy with the same attributes as the
// original. The child node also needs to be a copy with the same attributes as the original child.
TEST_F(IntermNodeTest, DeepCopyUnaryNode)
{
    TType type(EbtFloat, EbpHigh);

    TIntermUnary *original = new TIntermUnary(EOpPreIncrement, createTestSymbol(), nullptr);
    original->setLine(getTestSourceLoc());
    TIntermTyped *copyTyped = original->deepCopy();
    TIntermUnary *copy      = copyTyped->getAsUnaryNode();
    ASSERT_NE(nullptr, copy);
    ASSERT_NE(original, copy);
    checkTestSourceLoc(copy->getLine());
    checkTypeEqualWithQualifiers(original->getType(), copy->getType());

    checkSymbolCopy(original->getOperand(), copy->getOperand());
}

// Check that the deep copy of an aggregate node is an actual copy with the same attributes as the
// original. Child nodes also need to be copies with the same attributes as the original children.
TEST_F(IntermNodeTest, DeepCopyAggregateNode)
{
    TIntermSequence *originalSeq = new TIntermSequence();
    originalSeq->push_back(createTestSymbol());
    originalSeq->push_back(createTestSymbol());
    originalSeq->push_back(createTestSymbol());

    TFunction *testFunc =
        createTestFunction(originalSeq->back()->getAsTyped()->getType(), *originalSeq);

    TIntermAggregate *original = TIntermAggregate::CreateFunctionCall(*testFunc, originalSeq);
    original->setLine(getTestSourceLoc());

    TIntermTyped *copyTyped = original->deepCopy();
    TIntermAggregate *copy  = copyTyped->getAsAggregate();
    ASSERT_NE(nullptr, copy);
    ASSERT_NE(original, copy);
    checkTestSourceLoc(copy->getLine());
    checkTypeEqualWithQualifiers(original->getType(), copy->getType());

    ASSERT_EQ(original->getSequence()->size(), copy->getSequence()->size());
    TIntermSequence::size_type i = 0;
    for (auto *copyChild : *copy->getSequence())
    {
        TIntermNode *originalChild = original->getSequence()->at(i);
        checkSymbolCopy(originalChild, copyChild);
        ++i;
    }
}

// Check that the deep copy of a ternary node is an actual copy with the same attributes as the
// original. Child nodes also need to be copies with the same attributes as the original children.
TEST_F(IntermNodeTest, DeepCopyTernaryNode)
{
    TType type(EbtFloat, EbpHigh);

    TIntermTernary *original = new TIntermTernary(createTestSymbol(TType(EbtBool, EbpUndefined)),
                                                  createTestSymbol(), createTestSymbol());
    original->setLine(getTestSourceLoc());
    TIntermTyped *copyTyped = original->deepCopy();
    TIntermTernary *copy    = copyTyped->getAsTernaryNode();
    ASSERT_NE(nullptr, copy);
    ASSERT_NE(original, copy);
    checkTestSourceLoc(copy->getLine());
    checkTypeEqualWithQualifiers(original->getType(), copy->getType());

    checkSymbolCopy(original->getCondition(), copy->getCondition());
    checkSymbolCopy(original->getTrueExpression(), copy->getTrueExpression());
    checkSymbolCopy(original->getFalseExpression(), copy->getFalseExpression());
}
