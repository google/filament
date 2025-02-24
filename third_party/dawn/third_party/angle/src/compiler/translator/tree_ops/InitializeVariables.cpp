//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/tree_ops/InitializeVariables.h"

#include "angle_gl.h"
#include "common/debug.h"
#include "common/hash_containers.h"
#include "compiler/translator/Compiler.h"
#include "compiler/translator/StaticType.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_util/FindMain.h"
#include "compiler/translator/tree_util/FindSymbolNode.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"
#include "compiler/translator/util.h"

namespace sh
{

namespace
{

void AddArrayZeroInitSequence(const TIntermTyped *initializedNode,
                              bool canUseLoopsToInitialize,
                              bool highPrecisionSupported,
                              TIntermSequence *initSequenceOut,
                              TSymbolTable *symbolTable);

void AddStructZeroInitSequence(const TIntermTyped *initializedNode,
                               bool canUseLoopsToInitialize,
                               bool highPrecisionSupported,
                               TIntermSequence *initSequenceOut,
                               TSymbolTable *symbolTable);

TIntermBinary *CreateZeroInitAssignment(const TIntermTyped *initializedNode)
{
    TIntermTyped *zero = CreateZeroNode(initializedNode->getType());
    return new TIntermBinary(EOpAssign, initializedNode->deepCopy(), zero);
}

void AddZeroInitSequence(const TIntermTyped *initializedNode,
                         bool canUseLoopsToInitialize,
                         bool highPrecisionSupported,
                         TIntermSequence *initSequenceOut,
                         TSymbolTable *symbolTable)
{
    if (initializedNode->isArray())
    {
        AddArrayZeroInitSequence(initializedNode, canUseLoopsToInitialize, highPrecisionSupported,
                                 initSequenceOut, symbolTable);
    }
    else if (initializedNode->getType().isStructureContainingArrays() ||
             initializedNode->getType().isNamelessStruct())
    {
        AddStructZeroInitSequence(initializedNode, canUseLoopsToInitialize, highPrecisionSupported,
                                  initSequenceOut, symbolTable);
    }
    else if (initializedNode->getType().isInterfaceBlock())
    {
        const TType &type                     = initializedNode->getType();
        const TInterfaceBlock &interfaceBlock = *type.getInterfaceBlock();
        const TFieldList &fieldList           = interfaceBlock.fields();
        for (size_t fieldIndex = 0; fieldIndex < fieldList.size(); ++fieldIndex)
        {
            const TField &field         = *fieldList[fieldIndex];
            TIntermTyped *fieldIndexRef = CreateIndexNode(static_cast<int>(fieldIndex));
            TIntermTyped *fieldReference =
                new TIntermBinary(TOperator::EOpIndexDirectInterfaceBlock,
                                  initializedNode->deepCopy(), fieldIndexRef);
            TIntermTyped *fieldZero = CreateZeroNode(*field.type());
            TIntermTyped *assignment =
                new TIntermBinary(TOperator::EOpAssign, fieldReference, fieldZero);
            initSequenceOut->push_back(assignment);
        }
    }
    else
    {
        initSequenceOut->push_back(CreateZeroInitAssignment(initializedNode));
    }
}

void AddStructZeroInitSequence(const TIntermTyped *initializedNode,
                               bool canUseLoopsToInitialize,
                               bool highPrecisionSupported,
                               TIntermSequence *initSequenceOut,
                               TSymbolTable *symbolTable)
{
    ASSERT(initializedNode->getBasicType() == EbtStruct);
    const TStructure *structType = initializedNode->getType().getStruct();
    for (int i = 0; i < static_cast<int>(structType->fields().size()); ++i)
    {
        TIntermBinary *element = new TIntermBinary(EOpIndexDirectStruct,
                                                   initializedNode->deepCopy(), CreateIndexNode(i));
        // Structs can't be defined inside structs, so the type of a struct field can't be a
        // nameless struct.
        ASSERT(!element->getType().isNamelessStruct());
        AddZeroInitSequence(element, canUseLoopsToInitialize, highPrecisionSupported,
                            initSequenceOut, symbolTable);
    }
}

void AddArrayZeroInitStatementList(const TIntermTyped *initializedNode,
                                   bool canUseLoopsToInitialize,
                                   bool highPrecisionSupported,
                                   TIntermSequence *initSequenceOut,
                                   TSymbolTable *symbolTable)
{
    for (unsigned int i = 0; i < initializedNode->getOutermostArraySize(); ++i)
    {
        TIntermBinary *element =
            new TIntermBinary(EOpIndexDirect, initializedNode->deepCopy(), CreateIndexNode(i));
        AddZeroInitSequence(element, canUseLoopsToInitialize, highPrecisionSupported,
                            initSequenceOut, symbolTable);
    }
}

void AddArrayZeroInitForLoop(const TIntermTyped *initializedNode,
                             bool highPrecisionSupported,
                             TIntermSequence *initSequenceOut,
                             TSymbolTable *symbolTable)
{
    ASSERT(initializedNode->isArray());
    const TType *mediumpIndexType = StaticType::Get<EbtInt, EbpMedium, EvqTemporary, 1, 1>();
    const TType *highpIndexType   = StaticType::Get<EbtInt, EbpHigh, EvqTemporary, 1, 1>();
    TVariable *indexVariable =
        CreateTempVariable(symbolTable, highPrecisionSupported ? highpIndexType : mediumpIndexType);

    TIntermSymbol *indexSymbolNode = CreateTempSymbolNode(indexVariable);
    TIntermDeclaration *indexInit =
        CreateTempInitDeclarationNode(indexVariable, CreateZeroNode(indexVariable->getType()));
    TIntermConstantUnion *arraySizeNode = CreateIndexNode(initializedNode->getOutermostArraySize());
    TIntermBinary *indexSmallerThanSize =
        new TIntermBinary(EOpLessThan, indexSymbolNode->deepCopy(), arraySizeNode);
    TIntermUnary *indexIncrement =
        new TIntermUnary(EOpPreIncrement, indexSymbolNode->deepCopy(), nullptr);

    TIntermBlock *forLoopBody       = new TIntermBlock();
    TIntermSequence *forLoopBodySeq = forLoopBody->getSequence();

    TIntermBinary *element = new TIntermBinary(EOpIndexIndirect, initializedNode->deepCopy(),
                                               indexSymbolNode->deepCopy());
    AddZeroInitSequence(element, true, highPrecisionSupported, forLoopBodySeq, symbolTable);

    TIntermLoop *forLoop =
        new TIntermLoop(ELoopFor, indexInit, indexSmallerThanSize, indexIncrement, forLoopBody);
    initSequenceOut->push_back(forLoop);
}

void AddArrayZeroInitSequence(const TIntermTyped *initializedNode,
                              bool canUseLoopsToInitialize,
                              bool highPrecisionSupported,
                              TIntermSequence *initSequenceOut,
                              TSymbolTable *symbolTable)
{
    // The array elements are assigned one by one to keep the AST compatible with ESSL 1.00 which
    // doesn't have array assignment. We'll do this either with a for loop or just a list of
    // statements assigning to each array index. Note that it is important to have the array init in
    // the right order to workaround http://crbug.com/709317
    bool isSmallArray = initializedNode->getOutermostArraySize() <= 1u ||
                        (initializedNode->getBasicType() != EbtStruct &&
                         !initializedNode->getType().isArrayOfArrays() &&
                         initializedNode->getOutermostArraySize() <= 3u);
    if (initializedNode->getQualifier() == EvqFragData ||
        initializedNode->getQualifier() == EvqFragmentOut || isSmallArray ||
        !canUseLoopsToInitialize)
    {
        // Fragment outputs should not be indexed by non-constant indices.
        // Also it doesn't make sense to use loops to initialize very small arrays.
        AddArrayZeroInitStatementList(initializedNode, canUseLoopsToInitialize,
                                      highPrecisionSupported, initSequenceOut, symbolTable);
    }
    else
    {
        AddArrayZeroInitForLoop(initializedNode, highPrecisionSupported, initSequenceOut,
                                symbolTable);
    }
}

void InsertInitCode(TCompiler *compiler,
                    TIntermBlock *root,
                    const InitVariableList &variables,
                    TSymbolTable *symbolTable,
                    int shaderVersion,
                    const TExtensionBehavior &extensionBehavior,
                    bool canUseLoopsToInitialize,
                    bool highPrecisionSupported)
{
    TIntermSequence *mainBody = FindMainBody(root)->getSequence();
    for (const TVariable *var : variables)
    {
        TIntermTyped *initializedSymbol = nullptr;

        if (var->symbolType() == SymbolType::Empty)
        {
            // Must be a nameless interface block.
            ASSERT(var->getType().getInterfaceBlock() != nullptr);
            ASSERT(!var->getType().getInterfaceBlock()->name().empty());

            const TInterfaceBlock *block = var->getType().getInterfaceBlock();
            for (const TField *field : block->fields())
            {
                initializedSymbol = ReferenceGlobalVariable(field->name(), *symbolTable);

                TIntermSequence initCode;
                CreateInitCode(initializedSymbol, canUseLoopsToInitialize, highPrecisionSupported,
                               &initCode, symbolTable);
                mainBody->insert(mainBody->begin(), initCode.begin(), initCode.end());
            }

            // All done with the interface block
            continue;
        }

        const TQualifier qualifier = var->getType().getQualifier();

        initializedSymbol = new TIntermSymbol(var);
        if (qualifier == EvqFragData &&
            !IsExtensionEnabled(extensionBehavior, TExtension::EXT_draw_buffers))
        {
            // If GL_EXT_draw_buffers is disabled, only the 0th index of gl_FragData can be
            // written to.
            initializedSymbol =
                new TIntermBinary(EOpIndexDirect, initializedSymbol, CreateIndexNode(0));
        }

        TIntermSequence initCode;
        CreateInitCode(initializedSymbol, canUseLoopsToInitialize, highPrecisionSupported,
                       &initCode, symbolTable);
        mainBody->insert(mainBody->begin(), initCode.begin(), initCode.end());
    }
}

TFunction *CloneFunctionHeader(TSymbolTable *symbolTable, const TFunction *function)
{
    TFunction *newFunction =
        new TFunction(symbolTable, function->name(), function->symbolType(),
                      &function->getReturnType(), function->isKnownToNotHaveSideEffects());

    if (function->isDefined())
    {
        newFunction->setDefined();
    }
    if (function->hasPrototypeDeclaration())
    {
        newFunction->setHasPrototypeDeclaration();
    }
    return newFunction;
}

class InitializeLocalsTraverser final : public TIntermTraverser
{
  public:
    InitializeLocalsTraverser(int shaderVersion,
                              TSymbolTable *symbolTable,
                              bool canUseLoopsToInitialize,
                              bool highPrecisionSupported)
        : TIntermTraverser(true, false, false, symbolTable),
          mShaderVersion(shaderVersion),
          mCanUseLoopsToInitialize(canUseLoopsToInitialize),
          mHighPrecisionSupported(highPrecisionSupported)
    {}

    void collectUnnamedOutFunctions(TIntermBlock &root)
    {
        TIntermSequence &sequence = *root.getSequence();
        const size_t count        = sequence.size();
        for (size_t i = 0; i < count; ++i)
        {
            const TIntermFunctionDefinition *functionDefinition =
                sequence[i]->getAsFunctionDefinition();
            if (!functionDefinition)
            {
                continue;
            }
            const TFunction *function = functionDefinition->getFunction();
            TFunction *newFunction    = nullptr;
            for (size_t p = 0; p < function->getParamCount(); ++p)
            {
                const TVariable *param = function->getParam(p);
                const TType &type      = param->getType();
                if (param->symbolType() == SymbolType::Empty)
                {
                    if (!newFunction)
                    {
                        newFunction                   = CloneFunctionHeader(mSymbolTable, function);
                        mFunctionsToReplace[function] = newFunction;
                        for (size_t z = 0; z < p; ++z)
                        {
                            newFunction->addParameter(function->getParam(z));
                        }
                    }
                    param = new TVariable(mSymbolTable, kEmptyImmutableString, &type,
                                          SymbolType::AngleInternal, param->extensions());
                }
                if (newFunction)
                {
                    newFunction->addParameter(param);
                }
            }
        }
    }

  protected:
    bool visitDeclaration(Visit visit, TIntermDeclaration *node) override
    {
        for (TIntermNode *declarator : *node->getSequence())
        {
            if (!mInGlobalScope && !declarator->getAsBinaryNode())
            {
                TIntermSymbol *symbol = declarator->getAsSymbolNode();
                ASSERT(symbol);
                if (symbol->variable().symbolType() == SymbolType::Empty)
                {
                    continue;
                }

                // Arrays may need to be initialized one element at a time, since ESSL 1.00 does not
                // support array constructors or assigning arrays.
                bool arrayConstructorUnavailable =
                    (symbol->isArray() || symbol->getType().isStructureContainingArrays()) &&
                    mShaderVersion == 100;
                // Nameless struct constructors can't be referred to, so they also need to be
                // initialized one element at a time.
                // TODO(oetuaho): Check if it makes sense to initialize using a loop, even if we
                // could use an initializer. It could at least reduce code size for very large
                // arrays, but could hurt runtime performance.
                if (arrayConstructorUnavailable || symbol->getType().isNamelessStruct())
                {
                    // SimplifyLoopConditions should have been run so the parent node of this node
                    // should not be a loop.
                    ASSERT(getParentNode()->getAsLoopNode() == nullptr);
                    // SeparateDeclarations should have already been run, so we don't need to worry
                    // about further declarators in this declaration depending on the effects of
                    // this declarator.
                    ASSERT(node->getSequence()->size() == 1);
                    TIntermSequence initCode;
                    CreateInitCode(symbol, mCanUseLoopsToInitialize, mHighPrecisionSupported,
                                   &initCode, mSymbolTable);
                    insertStatementsInParentBlock(TIntermSequence(), initCode);
                }
                else
                {
                    TIntermBinary *init =
                        new TIntermBinary(EOpInitialize, symbol, CreateZeroNode(symbol->getType()));
                    queueReplacementWithParent(node, symbol, init, OriginalNode::BECOMES_CHILD);
                }
            }
        }
        // Must recurse in the cases which had initializers, because the initializiers might
        // call the function that was rewritten.
        return true;
    }

    void visitFunctionPrototype(TIntermFunctionPrototype *node) override
    {
        if (getParentNode()->getAsFunctionDefinition() != nullptr)
        {
            return;
        }
        auto it = mFunctionsToReplace.find(node->getFunction());
        if (it != mFunctionsToReplace.end())
        {
            queueReplacement(new TIntermFunctionPrototype(it->second), OriginalNode::IS_DROPPED);
        }
    }

    bool visitFunctionDefinition(Visit visit, TIntermFunctionDefinition *node) override
    {
        // Initialize output function arguments as well, the parameter passed in at call time may be
        // clobbered if the function doesn't fully write to the argument.

        TIntermSequence initCode;

        const TFunction *function = node->getFunction();
        auto it                   = mFunctionsToReplace.find(function);
        if (it != mFunctionsToReplace.end())
        {
            function                                   = it->second;
            TIntermFunctionPrototype *newPrototypeNode = new TIntermFunctionPrototype(function);
            TIntermFunctionDefinition *newNode =
                new TIntermFunctionDefinition(newPrototypeNode, node->getBody());
            queueReplacement(newNode, OriginalNode::IS_DROPPED);
        }

        for (size_t paramIndex = 0; paramIndex < function->getParamCount(); ++paramIndex)
        {
            const TVariable *paramVariable = function->getParam(paramIndex);
            const TType &paramType         = paramVariable->getType();

            if (paramType.getQualifier() != EvqParamOut)
            {
                continue;
            }

            CreateInitCode(new TIntermSymbol(paramVariable), mCanUseLoopsToInitialize,
                           mHighPrecisionSupported, &initCode, mSymbolTable);
        }

        if (!initCode.empty())
        {
            TIntermSequence *body = node->getBody()->getSequence();
            body->insert(body->begin(), initCode.begin(), initCode.end());
        }

        return true;
    }

    bool visitAggregate(Visit visit, TIntermAggregate *node) override
    {
        const TFunction *function = node->getFunction();
        if (function != nullptr)
        {
            auto it = mFunctionsToReplace.find(function);
            if (it != mFunctionsToReplace.end())
            {
                const TFunction *target = it->second;
                TIntermAggregate *newNode =
                    TIntermAggregate::CreateFunctionCall(*target, node->getSequence());
                queueReplacement(newNode, OriginalNode::IS_DROPPED);
            }
        }
        return true;
    }

  private:
    int mShaderVersion;
    bool mCanUseLoopsToInitialize;
    bool mHighPrecisionSupported;
    angle::HashMap<const TFunction *, TFunction *> mFunctionsToReplace;
};

}  // namespace

void CreateInitCode(const TIntermTyped *initializedSymbol,
                    bool canUseLoopsToInitialize,
                    bool highPrecisionSupported,
                    TIntermSequence *initCode,
                    TSymbolTable *symbolTable)
{
    AddZeroInitSequence(initializedSymbol, canUseLoopsToInitialize, highPrecisionSupported,
                        initCode, symbolTable);
}

bool InitializeUninitializedLocals(TCompiler *compiler,
                                   TIntermBlock *root,
                                   int shaderVersion,
                                   bool canUseLoopsToInitialize,
                                   bool highPrecisionSupported,
                                   TSymbolTable *symbolTable)
{
    InitializeLocalsTraverser traverser(shaderVersion, symbolTable, canUseLoopsToInitialize,
                                        highPrecisionSupported);
    traverser.collectUnnamedOutFunctions(*root);
    root->traverse(&traverser);
    return traverser.updateTree(compiler, root);
}

bool InitializeVariables(TCompiler *compiler,
                         TIntermBlock *root,
                         const InitVariableList &vars,
                         TSymbolTable *symbolTable,
                         int shaderVersion,
                         const TExtensionBehavior &extensionBehavior,
                         bool canUseLoopsToInitialize,
                         bool highPrecisionSupported)
{
    InsertInitCode(compiler, root, vars, symbolTable, shaderVersion, extensionBehavior,
                   canUseLoopsToInitialize, highPrecisionSupported);

    return compiler->validateAST(root);
}

}  // namespace sh
