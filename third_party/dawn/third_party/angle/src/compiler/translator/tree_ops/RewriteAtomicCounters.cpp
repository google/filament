//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RewriteAtomicCounters: Emulate atomic counter buffers with storage buffers.
//

#include "compiler/translator/tree_ops/RewriteAtomicCounters.h"

#include "compiler/translator/Compiler.h"
#include "compiler/translator/ImmutableStringBuilder.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"
#include "compiler/translator/tree_util/ReplaceVariable.h"

namespace sh
{
namespace
{
constexpr ImmutableString kAtomicCountersVarName   = ImmutableString("atomicCounters");
constexpr ImmutableString kAtomicCountersBlockName = ImmutableString("ANGLEAtomicCounters");
constexpr ImmutableString kAtomicCounterFieldName  = ImmutableString("counters");

// DeclareAtomicCountersBuffer adds a storage buffer array that's used with atomic counters.
const TVariable *DeclareAtomicCountersBuffers(TIntermBlock *root, TSymbolTable *symbolTable)
{
    // Define `uint counters[];` as the only field in the interface block.
    TFieldList *fieldList = new TFieldList;
    TType *counterType    = new TType(EbtUInt, EbpHigh, EvqGlobal);
    counterType->makeArray(0);

    TField *countersField =
        new TField(counterType, kAtomicCounterFieldName, TSourceLoc(), SymbolType::AngleInternal);

    fieldList->push_back(countersField);

    TMemoryQualifier coherentMemory = TMemoryQualifier::Create();
    coherentMemory.coherent         = true;

    // There are a maximum of 8 atomic counter buffers per IMPLEMENTATION_MAX_ATOMIC_COUNTER_BUFFERS
    // in libANGLE/Constants.h.
    constexpr uint32_t kMaxAtomicCounterBuffers = 8;

    // Define a storage block "ANGLEAtomicCounters" with instance name "atomicCounters".
    TLayoutQualifier layoutQualifier = TLayoutQualifier::Create();
    layoutQualifier.blockStorage     = EbsStd430;

    return DeclareInterfaceBlock(root, symbolTable, fieldList, EvqBuffer, layoutQualifier,
                                 coherentMemory, kMaxAtomicCounterBuffers, kAtomicCountersBlockName,
                                 kAtomicCountersVarName);
}

TIntermTyped *CreateUniformBufferOffset(const TIntermTyped *uniformBufferOffsets, int binding)
{
    // Each uint in the |acbBufferOffsets| uniform contains offsets for 4 bindings.  Therefore, the
    // expression to get the uniform offset for the binding is:
    //
    //     acbBufferOffsets[binding / 4] >> ((binding % 4) * 8) & 0xFF

    // acbBufferOffsets[binding / 4]
    TIntermBinary *uniformBufferOffsetUint = new TIntermBinary(
        EOpIndexDirect, uniformBufferOffsets->deepCopy(), CreateIndexNode(binding / 4));

    // acbBufferOffsets[binding / 4] >> ((binding % 4) * 8)
    TIntermBinary *uniformBufferOffsetShifted = uniformBufferOffsetUint;
    if (binding % 4 != 0)
    {
        uniformBufferOffsetShifted = new TIntermBinary(EOpBitShiftRight, uniformBufferOffsetUint,
                                                       CreateUIntNode((binding % 4) * 8));
    }

    // acbBufferOffsets[binding / 4] >> ((binding % 4) * 8) & 0xFF
    return new TIntermBinary(EOpBitwiseAnd, uniformBufferOffsetShifted, CreateUIntNode(0xFF));
}

TIntermBinary *CreateAtomicCounterRef(TIntermTyped *atomicCounterExpression,
                                      const TVariable *atomicCounters,
                                      const TIntermTyped *uniformBufferOffsets)
{
    // The atomic counters storage buffer declaration looks as such:
    //
    // layout(...) buffer ANGLEAtomicCounters
    // {
    //     uint counters[];
    // } atomicCounters[N];
    //
    // Where N is large enough to accommodate atomic counter buffer bindings used in the shader.
    //
    // This function takes an expression that uses an atomic counter, which can either be:
    //
    //  - ac
    //  - acArray[index]
    //
    // Note that RewriteArrayOfArrayOfOpaqueUniforms has already flattened array of array of atomic
    // counters.
    //
    // For the first case (ac), the following code is generated:
    //
    //     atomicCounters[binding].counters[offset]
    //
    // For the second case (acArray[index]), the following code is generated:
    //
    //     atomicCounters[binding].counters[offset + index]
    //
    // In either case, an offset given through uniforms is also added to |offset|.  The binding is
    // necessarily a constant thanks to MonomorphizeUnsupportedFunctions.

    // First determine if there's an index, and extract the atomic counter symbol out of the
    // expression.
    TIntermSymbol *atomicCounterSymbol = atomicCounterExpression->getAsSymbolNode();
    TIntermTyped *atomicCounterIndex   = nullptr;
    int atomicCounterConstIndex        = 0;
    TIntermBinary *asBinary            = atomicCounterExpression->getAsBinaryNode();
    if (asBinary != nullptr)
    {
        atomicCounterSymbol = asBinary->getLeft()->getAsSymbolNode();

        switch (asBinary->getOp())
        {
            case EOpIndexDirect:
                atomicCounterConstIndex = asBinary->getRight()->getAsConstantUnion()->getIConst(0);
                break;
            case EOpIndexIndirect:
                atomicCounterIndex = asBinary->getRight();
                break;
            default:
                UNREACHABLE();
        }
    }

    // Extract binding and offset information out of the atomic counter symbol.
    ASSERT(atomicCounterSymbol);
    const TVariable *atomicCounterVar = &atomicCounterSymbol->variable();
    const TType &atomicCounterType    = atomicCounterVar->getType();

    const int binding = atomicCounterType.getLayoutQualifier().binding;
    int offset        = atomicCounterType.getLayoutQualifier().offset / 4;

    // Create the expression:
    //
    //     offset + arrayIndex + uniformOffset
    //
    // If arrayIndex is a constant, it's added with offset right here.

    offset += atomicCounterConstIndex;

    TIntermTyped *index = CreateUniformBufferOffset(uniformBufferOffsets, binding);
    if (atomicCounterIndex != nullptr)
    {
        index = new TIntermBinary(EOpAdd, index, atomicCounterIndex);
    }
    if (offset != 0)
    {
        index = new TIntermBinary(EOpAdd, index, CreateIndexNode(offset));
    }

    // Finally, create the complete expression:
    //
    //     atomicCounters[binding].counters[index]

    TIntermSymbol *atomicCountersRef = new TIntermSymbol(atomicCounters);

    // atomicCounters[binding]
    TIntermBinary *countersBlock =
        new TIntermBinary(EOpIndexDirect, atomicCountersRef, CreateIndexNode(binding));

    // atomicCounters[binding].counters
    TIntermBinary *counters =
        new TIntermBinary(EOpIndexDirectInterfaceBlock, countersBlock, CreateIndexNode(0));

    return new TIntermBinary(EOpIndexIndirect, counters, index);
}

// Traverser that:
//
// 1. Removes the |uniform atomic_uint| declarations and remembers the binding and offset.
// 2. Substitutes |atomicVar[n]| with |buffer[binding].counters[offset + n]|.
class RewriteAtomicCountersTraverser : public TIntermTraverser
{
  public:
    RewriteAtomicCountersTraverser(TSymbolTable *symbolTable,
                                   const TVariable *atomicCounters,
                                   const TIntermTyped *acbBufferOffsets)
        : TIntermTraverser(true, false, false, symbolTable),
          mAtomicCounters(atomicCounters),
          mAcbBufferOffsets(acbBufferOffsets)
    {}

    bool visitDeclaration(Visit visit, TIntermDeclaration *node) override
    {
        if (!mInGlobalScope)
        {
            return true;
        }

        const TIntermSequence &sequence = *(node->getSequence());

        TIntermTyped *variable = sequence.front()->getAsTyped();
        const TType &type      = variable->getType();
        bool isAtomicCounter   = type.isAtomicCounter();

        if (isAtomicCounter)
        {
            ASSERT(type.getQualifier() == EvqUniform);
            TIntermSequence emptySequence;
            mMultiReplacements.emplace_back(getParentNode()->getAsBlock(), node,
                                            std::move(emptySequence));

            return false;
        }

        return true;
    }

    bool visitAggregate(Visit visit, TIntermAggregate *node) override
    {
        if (BuiltInGroup::IsBuiltIn(node->getOp()))
        {
            bool converted = convertBuiltinFunction(node);
            return !converted;
        }

        // AST functions don't require modification as atomic counter function parameters are
        // removed by MonomorphizeUnsupportedFunctions.
        return true;
    }

    void visitSymbol(TIntermSymbol *symbol) override
    {
        // Cannot encounter the atomic counter symbol directly.  It can only be used with functions,
        // and therefore it's handled by visitAggregate.
        ASSERT(!symbol->getType().isAtomicCounter());
    }

    bool visitBinary(Visit visit, TIntermBinary *node) override
    {
        // Cannot encounter an atomic counter expression directly.  It can only be used with
        // functions, and therefore it's handled by visitAggregate.
        ASSERT(!node->getType().isAtomicCounter());
        return true;
    }

  private:
    bool convertBuiltinFunction(TIntermAggregate *node)
    {
        const TOperator op = node->getOp();

        // If the function is |memoryBarrierAtomicCounter|, simply replace it with
        // |memoryBarrierBuffer|.
        if (op == EOpMemoryBarrierAtomicCounter)
        {
            TIntermSequence emptySequence;
            TIntermTyped *substituteCall = CreateBuiltInFunctionCallNode(
                "memoryBarrierBuffer", &emptySequence, *mSymbolTable, 310);
            queueReplacement(substituteCall, OriginalNode::IS_DROPPED);
            return true;
        }

        // If it's an |atomicCounter*| function, replace the function with an |atomic*| equivalent.
        if (!node->getFunction()->isAtomicCounterFunction())
        {
            return false;
        }

        // Note: atomicAdd(0) is used for atomic reads.
        uint32_t valueChange                = 0;
        constexpr char kAtomicAddFunction[] = "atomicAdd";
        bool isDecrement                    = false;

        if (op == EOpAtomicCounterIncrement)
        {
            valueChange = 1;
        }
        else if (op == EOpAtomicCounterDecrement)
        {
            // uint values are required to wrap around, so 0xFFFFFFFFu is used as -1.
            valueChange = std::numeric_limits<uint32_t>::max();
            static_assert(static_cast<uint32_t>(-1) == std::numeric_limits<uint32_t>::max(),
                          "uint32_t max is not -1");

            isDecrement = true;
        }
        else
        {
            ASSERT(op == EOpAtomicCounter);
        }

        TIntermTyped *param = (*node->getSequence())[0]->getAsTyped();

        TIntermSequence substituteArguments;
        substituteArguments.push_back(
            CreateAtomicCounterRef(param, mAtomicCounters, mAcbBufferOffsets));
        substituteArguments.push_back(CreateUIntNode(valueChange));

        TIntermTyped *substituteCall = CreateBuiltInFunctionCallNode(
            kAtomicAddFunction, &substituteArguments, *mSymbolTable, 310);

        // Note that atomicCounterDecrement returns the *new* value instead of the prior value,
        // unlike atomicAdd.  So we need to do a -1 on the result as well.
        if (isDecrement)
        {
            substituteCall = new TIntermBinary(EOpSub, substituteCall, CreateUIntNode(1));
        }

        queueReplacement(substituteCall, OriginalNode::IS_DROPPED);
        return true;
    }

    const TVariable *mAtomicCounters;
    const TIntermTyped *mAcbBufferOffsets;
};

}  // anonymous namespace

bool RewriteAtomicCounters(TCompiler *compiler,
                           TIntermBlock *root,
                           TSymbolTable *symbolTable,
                           const TIntermTyped *acbBufferOffsets,
                           const TVariable **atomicCountersOut)
{
    const TVariable *atomicCounters = DeclareAtomicCountersBuffers(root, symbolTable);
    if (atomicCountersOut)
    {
        *atomicCountersOut = atomicCounters;
    }

    RewriteAtomicCountersTraverser traverser(symbolTable, atomicCounters, acbBufferOffsets);
    root->traverse(&traverser);
    return traverser.updateTree(compiler, root);
}
}  // namespace sh
