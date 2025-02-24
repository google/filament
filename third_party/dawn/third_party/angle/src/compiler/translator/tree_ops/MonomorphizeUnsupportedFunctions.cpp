//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// MonomorphizeUnsupportedFunctions: Monomorphize functions that are called with
// parameters that are incompatible with both Vulkan GLSL and Metal.
//

#include "compiler/translator/tree_ops/MonomorphizeUnsupportedFunctions.h"

#include "compiler/translator/ImmutableStringBuilder.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"
#include "compiler/translator/tree_util/ReplaceVariable.h"

namespace sh
{
namespace
{
struct Argument
{
    size_t argumentIndex;
    TIntermTyped *argument;
};

struct FunctionData
{
    // Whether the original function is used.  If this is false, the function can be removed because
    // all callers have been modified.
    bool isOriginalUsed;
    // The original definition of the function, used to create the monomorphized version.
    TIntermFunctionDefinition *originalDefinition;
    // List of monomorphized versions of this function.  They will be added next to the original
    // version (or replace it).
    TVector<TIntermFunctionDefinition *> monomorphizedDefinitions;
};

using FunctionMap = angle::HashMap<const TFunction *, FunctionData>;

// Traverse the function definitions and initialize the map.  Allows visitAggregate to have access
// to TIntermFunctionDefinition even when the function is only forward declared at that point.
void InitializeFunctionMap(TIntermBlock *root, FunctionMap *functionMapOut)
{
    TIntermSequence &sequence = *root->getSequence();

    for (TIntermNode *node : sequence)
    {
        TIntermFunctionDefinition *asFuncDef = node->getAsFunctionDefinition();
        if (asFuncDef != nullptr)
        {
            const TFunction *function = asFuncDef->getFunction();
            ASSERT(function && functionMapOut->find(function) == functionMapOut->end());
            (*functionMapOut)[function] = FunctionData{false, asFuncDef, {}};
        }
    }
}

const TVariable *GetBaseUniform(TIntermTyped *node, bool *isSamplerInStructOut)
{
    *isSamplerInStructOut = false;

    while (node->getAsBinaryNode())
    {
        TIntermBinary *asBinary = node->getAsBinaryNode();

        TOperator op = asBinary->getOp();

        // No opaque uniform can be inside an interface block.
        if (op == EOpIndexDirectInterfaceBlock)
        {
            return nullptr;
        }

        if (op == EOpIndexDirectStruct)
        {
            *isSamplerInStructOut = true;
        }

        node = asBinary->getLeft();
    }

    // Only interested in uniform opaque types.  If a function call within another function uses
    // opaque uniforms in an unsupported way, it will be replaced in a follow up pass after the
    // calling function is monomorphized.
    if (node->getType().getQualifier() != EvqUniform)
    {
        return nullptr;
    }

    ASSERT(IsOpaqueType(node->getType().getBasicType()) ||
           node->getType().isStructureContainingSamplers());

    TIntermSymbol *asSymbol = node->getAsSymbolNode();
    ASSERT(asSymbol);

    return &asSymbol->variable();
}

TIntermTyped *ExtractSideEffects(TSymbolTable *symbolTable,
                                 TIntermTyped *node,
                                 TIntermSequence *replacementIndices)
{
    TIntermTyped *withoutSideEffects = node->deepCopy();

    for (TIntermBinary *asBinary = withoutSideEffects->getAsBinaryNode(); asBinary;
         asBinary                = asBinary->getLeft()->getAsBinaryNode())
    {
        TOperator op        = asBinary->getOp();
        TIntermTyped *index = asBinary->getRight();

        if (op == EOpIndexDirectStruct)
        {
            break;
        }

        // No side effects with constant expressions.
        if (op == EOpIndexDirect)
        {
            ASSERT(index->getAsConstantUnion());
            continue;
        }

        ASSERT(op == EOpIndexIndirect);

        // If the index is a symbol, there's no side effect, so leave it as-is.
        if (index->getAsSymbolNode())
        {
            continue;
        }

        // Otherwise create a temp variable initialized with the index and use that temp variable as
        // the index.
        TIntermDeclaration *tempDecl = nullptr;
        TVariable *tempVar = DeclareTempVariable(symbolTable, index, EvqTemporary, &tempDecl);

        replacementIndices->push_back(tempDecl);
        asBinary->replaceChildNode(index, new TIntermSymbol(tempVar));
    }

    return withoutSideEffects;
}

void CreateMonomorphizedFunctionCallArgs(const TIntermSequence &originalCallArguments,
                                         const TVector<Argument> &replacedArguments,
                                         TIntermSequence *substituteArgsOut)
{
    size_t nextReplacedArg = 0;
    for (size_t argIndex = 0; argIndex < originalCallArguments.size(); ++argIndex)
    {
        if (nextReplacedArg >= replacedArguments.size() ||
            argIndex != replacedArguments[nextReplacedArg].argumentIndex)
        {
            // Not replaced, keep argument as is.
            substituteArgsOut->push_back(originalCallArguments[argIndex]);
        }
        else
        {
            TIntermTyped *argument = replacedArguments[nextReplacedArg].argument;

            // Iterate over indices of the argument and create a new arg for every non-const
            // index.  Note that the index itself may be an expression, and it may require further
            // substitution in the next pass.
            while (argument->getAsBinaryNode())
            {
                TIntermBinary *asBinary = argument->getAsBinaryNode();
                if (asBinary->getOp() == EOpIndexIndirect)
                {
                    TIntermTyped *index = asBinary->getRight();
                    substituteArgsOut->push_back(index->deepCopy());
                }
                argument = asBinary->getLeft();
            }

            ++nextReplacedArg;
        }
    }
}

const TFunction *MonomorphizeFunction(TSymbolTable *symbolTable,
                                      const TFunction *original,
                                      TVector<Argument> *replacedArguments,
                                      VariableReplacementMap *argumentMapOut)
{
    TFunction *substituteFunction =
        new TFunction(symbolTable, kEmptyImmutableString, SymbolType::AngleInternal,
                      &original->getReturnType(), original->isKnownToNotHaveSideEffects());

    size_t nextReplacedArg = 0;
    for (size_t paramIndex = 0; paramIndex < original->getParamCount(); ++paramIndex)
    {
        const TVariable *originalParam = original->getParam(paramIndex);

        if (nextReplacedArg >= replacedArguments->size() ||
            paramIndex != (*replacedArguments)[nextReplacedArg].argumentIndex)
        {
            TVariable *substituteArgument =
                new TVariable(symbolTable, originalParam->name(), &originalParam->getType(),
                              originalParam->symbolType());
            // Not replaced, add an identical parameter.
            substituteFunction->addParameter(substituteArgument);
            (*argumentMapOut)[originalParam] = new TIntermSymbol(substituteArgument);
        }
        else
        {
            TIntermTyped *substituteArgument = (*replacedArguments)[nextReplacedArg].argument;
            (*argumentMapOut)[originalParam] = substituteArgument;

            // Iterate over indices of the argument and create a new parameter for every non-const
            // index (which may be an expression).  Replace the symbol in the argument with a
            // variable of the index type.  This is later used to replace the parameter in the
            // function body.
            while (substituteArgument->getAsBinaryNode())
            {
                TIntermBinary *asBinary = substituteArgument->getAsBinaryNode();
                if (asBinary->getOp() == EOpIndexIndirect)
                {
                    TIntermTyped *index = asBinary->getRight();
                    TType *indexType    = new TType(index->getType());
                    indexType->setQualifier(EvqParamIn);

                    TVariable *param = new TVariable(symbolTable, kEmptyImmutableString, indexType,
                                                     SymbolType::AngleInternal);
                    substituteFunction->addParameter(param);

                    // The argument now uses the function parameters as indices.
                    asBinary->replaceChildNode(asBinary->getRight(), new TIntermSymbol(param));
                }
                substituteArgument = asBinary->getLeft();
            }

            ++nextReplacedArg;
        }
    }

    return substituteFunction;
}

class MonomorphizeTraverser final : public TIntermTraverser
{
  public:
    explicit MonomorphizeTraverser(TCompiler *compiler,
                                   TSymbolTable *symbolTable,
                                   UnsupportedFunctionArgsBitSet unsupportedFunctionArgs,
                                   FunctionMap *functionMap)
        : TIntermTraverser(true, false, false, symbolTable),
          mCompiler(compiler),
          mUnsupportedFunctionArgs(unsupportedFunctionArgs),
          mFunctionMap(functionMap)
    {}

    bool visitAggregate(Visit visit, TIntermAggregate *node) override
    {
        if (node->getOp() != EOpCallFunctionInAST)
        {
            return true;
        }

        const TFunction *function = node->getFunction();
        ASSERT(function && mFunctionMap->find(function) != mFunctionMap->end());

        FunctionData &data = (*mFunctionMap)[function];

        TIntermFunctionDefinition *monomorphized =
            processFunctionCall(node, data.originalDefinition, &data.isOriginalUsed);
        if (monomorphized)
        {
            data.monomorphizedDefinitions.push_back(monomorphized);
        }

        return true;
    }

    bool getAnyMonomorphized() const { return mAnyMonomorphized; }

  private:
    bool isUnsupportedArgument(TIntermTyped *callArgument, const TVariable *funcArgument) const
    {
        // Only interested in opaque uniforms and structs that contain samplers.
        const bool isOpaqueType = IsOpaqueType(funcArgument->getType().getBasicType());
        const bool isStructContainingSamplers =
            funcArgument->getType().isStructureContainingSamplers();
        if (!isOpaqueType && !isStructContainingSamplers)
        {
            return false;
        }

        // If not uniform (the variable was itself a function parameter), don't process it in
        // this pass, as we don't know which actual uniform it corresponds to.
        bool isSamplerInStruct   = false;
        const TVariable *uniform = GetBaseUniform(callArgument, &isSamplerInStruct);
        if (uniform == nullptr)
        {
            return false;
        }

        const TType &type = uniform->getType();

        if (mUnsupportedFunctionArgs[UnsupportedFunctionArgs::StructContainingSamplers])
        {
            // Monomorphize if the parameter is a structure that contains samplers (so in
            // RewriteStructSamplers we don't need to rewrite the functions to accept multiple
            // parameters split from the struct).
            if (isStructContainingSamplers)
            {
                return true;
            }
        }

        if (mUnsupportedFunctionArgs[UnsupportedFunctionArgs::ArrayOfArrayOfSamplerOrImage])
        {
            // Monomorphize if:
            //
            // - The opaque uniform is a sampler in a struct (which can create an array-of-array
            //   situation), and the function expects an array of samplers, or
            //
            // - The opaque uniform is an array of array of sampler or image, and it's partially
            //   subscripted (i.e. the function itself expects an array)
            //
            const bool isParameterArrayOfOpaqueType = funcArgument->getType().isArray();
            const bool isArrayOfArrayOfSamplerOrImage =
                (type.isSampler() || type.isImage()) && type.isArrayOfArrays();
            if (isSamplerInStruct && isParameterArrayOfOpaqueType)
            {
                return true;
            }
            if (isArrayOfArrayOfSamplerOrImage && isParameterArrayOfOpaqueType)
            {
                return true;
            }
        }

        if (mUnsupportedFunctionArgs[UnsupportedFunctionArgs::AtomicCounter])
        {
            if (type.isAtomicCounter())
            {
                return true;
            }
        }

        if (mUnsupportedFunctionArgs[UnsupportedFunctionArgs::Image])
        {
            if (type.isImage())
            {
                return true;
            }
        }

        if (mUnsupportedFunctionArgs[UnsupportedFunctionArgs::PixelLocalStorage])
        {
            if (type.isPixelLocal())
            {
                return true;
            }
        }

        return false;
    }

    TIntermFunctionDefinition *processFunctionCall(TIntermAggregate *functionCall,
                                                   TIntermFunctionDefinition *originalDefinition,
                                                   bool *isOriginalUsedOut)
    {
        const TFunction *function            = functionCall->getFunction();
        const TIntermSequence &callArguments = *functionCall->getSequence();

        TVector<Argument> replacedArguments;
        TIntermSequence replacementIndices;

        // Go through function call arguments, and see if any is used in an unsupported way.
        for (size_t argIndex = 0; argIndex < callArguments.size(); ++argIndex)
        {
            TIntermTyped *callArgument    = callArguments[argIndex]->getAsTyped();
            const TVariable *funcArgument = function->getParam(argIndex);
            if (isUnsupportedArgument(callArgument, funcArgument))
            {
                // Copy the argument and extract the side effects.
                TIntermTyped *argument =
                    ExtractSideEffects(mSymbolTable, callArgument, &replacementIndices);

                replacedArguments.push_back({argIndex, argument});
            }
        }

        if (replacedArguments.empty())
        {
            *isOriginalUsedOut = true;
            return nullptr;
        }

        mAnyMonomorphized = true;

        insertStatementsInParentBlock(replacementIndices);

        // Create the arguments for the substitute function call.  Done before monomorphizing the
        // function, which transforms the arguments to what needs to be replaced in the function
        // body.
        TIntermSequence newCallArgs;
        CreateMonomorphizedFunctionCallArgs(callArguments, replacedArguments, &newCallArgs);

        // Duplicate the function and substitute the replaced arguments with only the non-const
        // indices.  Additionally, substitute the non-const indices of arguments with the new
        // function parameters.
        VariableReplacementMap argumentMap;
        const TFunction *monomorphized =
            MonomorphizeFunction(mSymbolTable, function, &replacedArguments, &argumentMap);

        // Replace this function call with a call to the new one.
        queueReplacement(TIntermAggregate::CreateFunctionCall(*monomorphized, &newCallArgs),
                         OriginalNode::IS_DROPPED);

        // Create a new function definition, with the body of the old function but with the replaced
        // parameters substituted with the calling expressions.
        TIntermFunctionPrototype *substitutePrototype = new TIntermFunctionPrototype(monomorphized);
        TIntermBlock *substituteBlock                 = originalDefinition->getBody()->deepCopy();
        GetDeclaratorReplacements(mSymbolTable, substituteBlock, &argumentMap);
        bool valid = ReplaceVariables(mCompiler, substituteBlock, argumentMap);
        ASSERT(valid);

        return new TIntermFunctionDefinition(substitutePrototype, substituteBlock);
    }

    TCompiler *mCompiler;
    UnsupportedFunctionArgsBitSet mUnsupportedFunctionArgs;
    bool mAnyMonomorphized = false;

    // Map of original to monomorphized functions.
    FunctionMap *mFunctionMap;
};

class UpdateFunctionsDefinitionsTraverser final : public TIntermTraverser
{
  public:
    explicit UpdateFunctionsDefinitionsTraverser(TSymbolTable *symbolTable,
                                                 const FunctionMap &functionMap)
        : TIntermTraverser(true, false, false, symbolTable), mFunctionMap(functionMap)
    {}

    void visitFunctionPrototype(TIntermFunctionPrototype *node) override
    {
        const bool isInFunctionDefinition = getParentNode()->getAsFunctionDefinition() != nullptr;
        if (isInFunctionDefinition)
        {
            return;
        }

        // Add to and possibly replace the function prototype with replacement prototypes.
        const TFunction *function = node->getFunction();
        ASSERT(function && mFunctionMap.find(function) != mFunctionMap.end());

        const FunctionData &data = mFunctionMap.at(function);

        // If nothing to do, leave it be.
        if (data.monomorphizedDefinitions.empty())
        {
            ASSERT(data.isOriginalUsed || function->isMain());
            return;
        }

        // Replace the prototype with itself (if function is still used) as well as any
        // monomorphized versions.
        TIntermSequence replacement;
        if (data.isOriginalUsed)
        {
            replacement.push_back(node);
        }
        for (TIntermFunctionDefinition *monomorphizedDefinition : data.monomorphizedDefinitions)
        {
            replacement.push_back(new TIntermFunctionPrototype(
                monomorphizedDefinition->getFunctionPrototype()->getFunction()));
        }
        mMultiReplacements.emplace_back(getParentNode()->getAsBlock(), node,
                                        std::move(replacement));
    }

    bool visitFunctionDefinition(Visit visit, TIntermFunctionDefinition *node) override
    {
        // Add to and possibly replace the function definition with replacement definitions.
        const TFunction *function = node->getFunction();
        ASSERT(function && mFunctionMap.find(function) != mFunctionMap.end());

        const FunctionData &data = mFunctionMap.at(function);

        // If nothing to do, leave it be.
        if (data.monomorphizedDefinitions.empty())
        {
            ASSERT(data.isOriginalUsed || function->isMain());
            return false;
        }

        // Replace the definition with itself (if function is still used) as well as any
        // monomorphized versions.
        TIntermSequence replacement;
        if (data.isOriginalUsed)
        {
            replacement.push_back(node);
        }
        for (TIntermFunctionDefinition *monomorphizedDefinition : data.monomorphizedDefinitions)
        {
            replacement.push_back(monomorphizedDefinition);
        }
        mMultiReplacements.emplace_back(getParentNode()->getAsBlock(), node,
                                        std::move(replacement));

        return false;
    }

  private:
    const FunctionMap &mFunctionMap;
};

void SortDeclarations(TIntermBlock *root)
{
    TIntermSequence *original = root->getSequence();

    TIntermSequence replacement;
    TIntermSequence functionDefs;

    // Accumulate non-function-definition declarations in |replacement| and function definitions in
    // |functionDefs|.
    for (TIntermNode *node : *original)
    {
        if (node->getAsFunctionDefinition() || node->getAsFunctionPrototypeNode())
        {
            functionDefs.push_back(node);
        }
        else
        {
            replacement.push_back(node);
        }
    }

    // Append function definitions to |replacement|.
    replacement.insert(replacement.end(), functionDefs.begin(), functionDefs.end());

    // Replace root's sequence with |replacement|.
    root->replaceAllChildren(replacement);
}

bool MonomorphizeUnsupportedFunctionsImpl(TCompiler *compiler,
                                          TIntermBlock *root,
                                          TSymbolTable *symbolTable,
                                          UnsupportedFunctionArgsBitSet unsupportedFunctionArgs)
{
    // First, sort out the declarations such that all non-function declarations are placed before
    // function definitions.  This way when the function is replaced with one that references said
    // declarations (i.e. uniforms), the uniform declaration is already present above it.
    SortDeclarations(root);

    while (true)
    {
        FunctionMap functionMap;
        InitializeFunctionMap(root, &functionMap);

        MonomorphizeTraverser monomorphizer(compiler, symbolTable, unsupportedFunctionArgs,
                                            &functionMap);
        root->traverse(&monomorphizer);

        if (!monomorphizer.getAnyMonomorphized())
        {
            break;
        }

        if (!monomorphizer.updateTree(compiler, root))
        {
            return false;
        }

        UpdateFunctionsDefinitionsTraverser functionUpdater(symbolTable, functionMap);
        root->traverse(&functionUpdater);

        if (!functionUpdater.updateTree(compiler, root))
        {
            return false;
        }
    }

    return true;
}
}  // anonymous namespace

bool MonomorphizeUnsupportedFunctions(TCompiler *compiler,
                                      TIntermBlock *root,
                                      TSymbolTable *symbolTable,
                                      UnsupportedFunctionArgsBitSet unsupportedFunctionArgs)
{
    // This function actually applies multiple transformation, and the AST may not be valid until
    // the transformations are entirely done.  Some validation is momentarily disabled.
    bool enableValidateFunctionCall = compiler->disableValidateFunctionCall();

    bool result =
        MonomorphizeUnsupportedFunctionsImpl(compiler, root, symbolTable, unsupportedFunctionArgs);

    compiler->restoreValidateFunctionCall(enableValidateFunctionCall);
    return result && compiler->validateAST(root);
}
}  // namespace sh
