//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RewriteStructSamplers: Extract samplers from structs.
//

#include "compiler/translator/tree_ops/RewriteStructSamplers.h"

#include "common/hash_containers.h"
#include "common/span.h"
#include "compiler/translator/ImmutableStringBuilder.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{
namespace
{

// Used to map one structure type to another (one where the samplers are removed).
struct StructureData
{
    // The structure this was replaced with.  If nullptr, it means the structure is removed (because
    // it had all samplers).
    //
    // ParseContext reorders the samplers to the end of the struct, so the EOpIndexDirectStruct
    // expressions that select non-sampler members don't have to change when they are moved out of
    // the struct.
    const TStructure *modified;
};

using StructureMap        = angle::HashMap<const TStructure *, StructureData>;
using StructureUniformMap = angle::HashMap<const TVariable *, const TVariable *>;
using ExtractedSamplerMap = angle::HashMap<std::string, const TVariable *>;

TIntermTyped *RewriteModifiedStructFieldSelectionExpression(
    TCompiler *compiler,
    TIntermBinary *node,
    const StructureMap &structureMap,
    const StructureUniformMap &structureUniformMap,
    const ExtractedSamplerMap &extractedSamplers);

TIntermTyped *RewriteExpressionVisitBinaryHelper(TCompiler *compiler,
                                                 TIntermBinary *node,
                                                 const StructureMap &structureMap,
                                                 const StructureUniformMap &structureUniformMap,
                                                 const ExtractedSamplerMap &extractedSamplers)
{
    // Only interested in EOpIndexDirectStruct binary nodes.
    if (node->getOp() != EOpIndexDirectStruct)
    {
        return nullptr;
    }

    const TStructure *structure = node->getLeft()->getType().getStruct();
    ASSERT(structure);

    // If the result of the index is not a sampler and the struct is not replaced, there's nothing
    // to do.
    if (!node->getType().isSampler() && structureMap.find(structure) == structureMap.end())
    {
        return nullptr;
    }

    // Otherwise, replace the whole expression such that:
    //
    // - if sampler, it's indexed with whatever indices the parent structs were indexed with,
    // - otherwise, the chain of field selections is rewritten by modifying the base uniform so all
    //   the intermediate nodes would have the correct type (and therefore fields).
    ASSERT(structureMap.find(structure) != structureMap.end());

    return RewriteModifiedStructFieldSelectionExpression(compiler, node, structureMap,
                                                         structureUniformMap, extractedSamplers);
}

// Given an expression, this traverser calculates a new expression where sampler-in-structs are
// replaced with their extracted ones, and field indices are adjusted for the rest of the fields.
// In particular, this is run on the right node of EOpIndexIndirect binary nodes, so that the
// expression in the index gets a chance to go through this transformation.
class RewriteExpressionTraverser final : public TIntermTraverser
{
  public:
    explicit RewriteExpressionTraverser(TCompiler *compiler,
                                        const StructureMap &structureMap,
                                        const StructureUniformMap &structureUniformMap,
                                        const ExtractedSamplerMap &extractedSamplers)
        : TIntermTraverser(true, false, false),
          mCompiler(compiler),
          mStructureMap(structureMap),
          mStructureUniformMap(structureUniformMap),
          mExtractedSamplers(extractedSamplers)
    {}

    bool visitBinary(Visit visit, TIntermBinary *node) override
    {
        TIntermTyped *rewritten = RewriteExpressionVisitBinaryHelper(
            mCompiler, node, mStructureMap, mStructureUniformMap, mExtractedSamplers);

        if (rewritten == nullptr)
        {
            return true;
        }

        queueReplacement(rewritten, OriginalNode::IS_DROPPED);

        // Don't iterate as the expression is rewritten.
        return false;
    }

    void visitSymbol(TIntermSymbol *node) override
    {
        // It's impossible to reach here with a symbol that needs replacement.
        // MonomorphizeUnsupportedFunctions makes sure that whole structs containing
        // samplers are not passed to functions, so any instance of the struct uniform is
        // necessarily indexed right away.  visitBinary should have already taken care of it.
        ASSERT(mStructureUniformMap.find(&node->variable()) == mStructureUniformMap.end());
    }

  private:
    TCompiler *mCompiler;

    // See RewriteStructSamplersTraverser.
    const StructureMap &mStructureMap;
    const StructureUniformMap &mStructureUniformMap;
    const ExtractedSamplerMap &mExtractedSamplers;
};

// Rewrite the index of an EOpIndexIndirect expression.  The root can never need replacing, because
// it cannot be a sampler itself or of a struct type.
void RewriteIndexExpression(TCompiler *compiler,
                            TIntermTyped *expression,
                            const StructureMap &structureMap,
                            const StructureUniformMap &structureUniformMap,
                            const ExtractedSamplerMap &extractedSamplers)
{
    RewriteExpressionTraverser traverser(compiler, structureMap, structureUniformMap,
                                         extractedSamplers);
    expression->traverse(&traverser);
    bool valid = traverser.updateTree(compiler, expression);
    ASSERT(valid);
}

// Given an expression such as the following:
//
//                                                    EOpIndexDirectStruct (sampler)
//                                                    /                  \
//                                               EOpIndex*           field index
//                                              /        \
//                                EOpIndexDirectStruct   index 2
//                                /                  \
//                           EOpIndex*           field index
//                          /        \
//            EOpIndexDirectStruct   index 1
//            /                  \
//     Uniform Struct           field index
//
// produces:
//
//                                EOpIndex*
//                                /      \
//                           EOpIndex*  index 2
//                          /        \
//                      sampler    index 1
//
// If the expression is not a sampler, it only replaces the struct with the modified one, while
// still processing the EOpIndexIndirect expressions (which may contain more structs to map).
TIntermTyped *RewriteModifiedStructFieldSelectionExpression(
    TCompiler *compiler,
    TIntermBinary *node,
    const StructureMap &structureMap,
    const StructureUniformMap &structureUniformMap,
    const ExtractedSamplerMap &extractedSamplers)
{
    ASSERT(node->getOp() == EOpIndexDirectStruct);

    const bool isSampler = node->getType().isSampler();

    TIntermSymbol *baseUniform = nullptr;
    std::string samplerName;

    TVector<TIntermBinary *> indexNodeStack;

    // Iterate once and build the name of the sampler.
    TIntermBinary *iter = node;
    while (baseUniform == nullptr)
    {
        indexNodeStack.push_back(iter);
        baseUniform = iter->getLeft()->getAsSymbolNode();

        if (isSampler)
        {
            if (iter->getOp() == EOpIndexDirectStruct)
            {
                // When indexed into a struct, get the field name instead and construct the sampler
                // name.
                samplerName.insert(0, iter->getIndexStructFieldName().data());
                samplerName.insert(0, "_");
            }

            if (baseUniform)
            {
                // If left is a symbol, we have reached the end of the chain.  Use the struct name
                // to finish building the name of the sampler.
                samplerName.insert(0, baseUniform->variable().name().data());
            }
        }

        iter = iter->getLeft()->getAsBinaryNode();
    }

    TIntermTyped *rewritten = nullptr;

    if (isSampler)
    {
        ASSERT(extractedSamplers.find(samplerName) != extractedSamplers.end());
        rewritten = new TIntermSymbol(extractedSamplers.at(samplerName));
    }
    else
    {
        const TVariable *baseUniformVar = &baseUniform->variable();
        ASSERT(structureUniformMap.find(baseUniformVar) != structureUniformMap.end());
        rewritten = new TIntermSymbol(structureUniformMap.at(baseUniformVar));
    }

    // Iterate again and build the expression from bottom up.
    for (auto it = indexNodeStack.rbegin(); it != indexNodeStack.rend(); ++it)
    {
        TIntermBinary *indexNode = *it;

        switch (indexNode->getOp())
        {
            case EOpIndexDirectStruct:
                if (!isSampler)
                {
                    rewritten =
                        new TIntermBinary(EOpIndexDirectStruct, rewritten, indexNode->getRight());
                }
                break;

            case EOpIndexDirect:
                rewritten = new TIntermBinary(EOpIndexDirect, rewritten, indexNode->getRight());
                break;

            case EOpIndexIndirect:
            {
                // Run RewriteExpressionTraverser on the right node.  It may itself be an expression
                // with a sampler inside that needs to be rewritten, or simply use a field of a
                // struct that's remapped.
                TIntermTyped *indexExpression = indexNode->getRight();
                RewriteIndexExpression(compiler, indexExpression, structureMap, structureUniformMap,
                                       extractedSamplers);
                rewritten = new TIntermBinary(EOpIndexIndirect, rewritten, indexExpression);
                break;
            }

            default:
                UNREACHABLE();
                break;
        }
    }

    return rewritten;
}

class RewriteStructSamplersTraverser final : public TIntermTraverser
{
  public:
    explicit RewriteStructSamplersTraverser(TCompiler *compiler, TSymbolTable *symbolTable)
        : TIntermTraverser(true, false, false, symbolTable),
          mCompiler(compiler),
          mRemovedUniformsCount(0)
    {}

    int removedUniformsCount() const { return mRemovedUniformsCount; }

    // Each struct sampler declaration is stripped of its samplers. New uniforms are added for each
    // stripped struct sampler.
    bool visitDeclaration(Visit visit, TIntermDeclaration *decl) override
    {
        if (!mInGlobalScope)
        {
            return true;
        }

        const TIntermSequence &sequence = *(decl->getSequence());
        TIntermTyped *declarator        = sequence.front()->getAsTyped();
        const TType &type               = declarator->getType();

        if (!type.isStructureContainingSamplers())
        {
            return false;
        }

        TIntermSequence newSequence;

        if (type.isStructSpecifier())
        {
            // If this is just a struct definition (not a uniform variable declaration of a
            // struct type), just remove the samplers.  They are not instantiated yet.
            const TStructure *structure = type.getStruct();
            ASSERT(structure && mStructureMap.find(structure) == mStructureMap.end());

            stripStructSpecifierSamplers(structure, &newSequence);
        }
        else
        {
            const TStructure *structure = type.getStruct();

            // If the structure is defined at the same time, create the mapping to the stripped
            // version first.
            if (mStructureMap.find(structure) == mStructureMap.end())
            {
                stripStructSpecifierSamplers(structure, &newSequence);
            }

            // Then, extract the samplers from the struct and create global-scope variables instead.
            TIntermSymbol *asSymbol = declarator->getAsSymbolNode();
            ASSERT(asSymbol);
            const TVariable &variable = asSymbol->variable();
            ASSERT(variable.symbolType() != SymbolType::Empty);

            extractStructSamplerUniforms(variable, structure, &newSequence);
        }

        mMultiReplacements.emplace_back(getParentNode()->getAsBlock(), decl,
                                        std::move(newSequence));

        return false;
    }

    // Same implementation as in RewriteExpressionTraverser.  That traverser cannot replace root.
    bool visitBinary(Visit visit, TIntermBinary *node) override
    {
        TIntermTyped *rewritten = RewriteExpressionVisitBinaryHelper(
            mCompiler, node, mStructureMap, mStructureUniformMap, mExtractedSamplers);

        if (rewritten == nullptr)
        {
            return true;
        }

        queueReplacement(rewritten, OriginalNode::IS_DROPPED);

        // Don't iterate as the expression is rewritten.
        return false;
    }

    // Same implementation as in RewriteExpressionTraverser.  That traverser cannot replace root.
    void visitSymbol(TIntermSymbol *node) override
    {
        auto replacement = mStructureUniformMap.find(&node->variable());
        if (replacement != mStructureUniformMap.end())
        {
            // This is a reference to the whole struct, just replace it with its replacement.
            queueReplacement(new TIntermSymbol(replacement->second), OriginalNode::IS_DROPPED);
        }
    }

  private:
    // Removes all samplers from a struct specifier.
    void stripStructSpecifierSamplers(const TStructure *structure, TIntermSequence *newSequence)
    {
        TFieldList *newFieldList = new TFieldList;
        ASSERT(structure->containsSamplers());

        // Add this struct to the struct map
        ASSERT(mStructureMap.find(structure) == mStructureMap.end());
        StructureData *modifiedData = &mStructureMap[structure];

        modifiedData->modified = nullptr;

        for (size_t fieldIndex = 0; fieldIndex < structure->fields().size(); ++fieldIndex)
        {
            const TField *field    = structure->fields()[fieldIndex];
            const TType &fieldType = *field->type();

            // If the field is a sampler, or a struct that's entirely removed, skip it.
            if (!fieldType.isSampler() && !isRemovedStructType(fieldType))
            {
                TType *newType = nullptr;

                // Otherwise, if it's a struct that's replaced, create a new field of the replaced
                // type.
                if (fieldType.isStructureContainingSamplers())
                {
                    const TStructure *fieldStruct = fieldType.getStruct();
                    ASSERT(mStructureMap.find(fieldStruct) != mStructureMap.end());

                    const TStructure *modifiedStruct = mStructureMap[fieldStruct].modified;
                    ASSERT(modifiedStruct);

                    newType = new TType(modifiedStruct, true);
                    if (fieldType.isArray())
                    {
                        newType->makeArrays(fieldType.getArraySizes());
                    }
                }
                else
                {
                    // If not, duplicate the field as is.
                    newType = new TType(fieldType);
                }

                TField *newField =
                    new TField(newType, field->name(), field->line(), field->symbolType());
                newFieldList->push_back(newField);
            }
        }

        // Prune empty structs.
        if (newFieldList->empty())
        {
            return;
        }

        // Declare a new struct with the same name and the new fields.
        modifiedData->modified =
            new TStructure(mSymbolTable,
                           structure->symbolType() == SymbolType::Empty ? kEmptyImmutableString
                                                                        : structure->name(),
                           newFieldList, structure->symbolType());
        TType *newStructType = new TType(modifiedData->modified, true);
        TVariable *newStructVar =
            new TVariable(mSymbolTable, kEmptyImmutableString, newStructType, SymbolType::Empty);
        TIntermSymbol *newStructRef = new TIntermSymbol(newStructVar);

        TIntermDeclaration *structDecl = new TIntermDeclaration;
        structDecl->appendDeclarator(newStructRef);

        newSequence->push_back(structDecl);
    }

    // Returns true if the type is a struct that was removed because we extracted all the members.
    bool isRemovedStructType(const TType &type) const
    {
        const TStructure *structure = type.getStruct();
        if (structure == nullptr)
        {
            // Not a struct
            return false;
        }

        // A struct is removed if it is in the map, but doesn't have a replacement struct.
        auto iter = mStructureMap.find(structure);
        return iter != mStructureMap.end() && iter->second.modified == nullptr;
    }

    // Removes samplers from struct uniforms. For each sampler removed also adds a new globally
    // defined sampler uniform.
    void extractStructSamplerUniforms(const TVariable &variable,
                                      const TStructure *structure,
                                      TIntermSequence *newSequence)
    {
        ASSERT(structure->containsSamplers());
        ASSERT(mStructureMap.find(structure) != mStructureMap.end());

        const TType &type = variable.getType();
        enterArray(type);

        for (const TField *field : structure->fields())
        {
            extractFieldSamplers(variable.name().data(), field, newSequence);
        }

        // If there's a replacement structure (because there are non-sampler fields in the struct),
        // add a declaration with that type.
        const TStructure *modified = mStructureMap[structure].modified;
        if (modified != nullptr)
        {
            TType *newType = new TType(modified, false);
            if (type.isArray())
            {
                newType->makeArrays(type.getArraySizes());
            }
            newType->setQualifier(EvqUniform);
            const TVariable *newVariable =
                new TVariable(mSymbolTable, variable.name(), newType, variable.symbolType());

            TIntermDeclaration *newDecl = new TIntermDeclaration();
            newDecl->appendDeclarator(new TIntermSymbol(newVariable));

            newSequence->push_back(newDecl);

            ASSERT(mStructureUniformMap.find(&variable) == mStructureUniformMap.end());
            mStructureUniformMap[&variable] = newVariable;
        }
        else
        {
            mRemovedUniformsCount++;
        }

        exitArray(type);
    }

    // Extracts samplers from a field of a struct. Works with nested structs and arrays.
    void extractFieldSamplers(const std::string &prefix,
                              const TField *field,
                              TIntermSequence *newSequence)
    {
        const TType &fieldType = *field->type();
        if (fieldType.isSampler() || fieldType.isStructureContainingSamplers())
        {
            std::string newPrefix = prefix + "_" + field->name().data();

            if (fieldType.isSampler())
            {
                extractSampler(newPrefix, fieldType, newSequence);
            }
            else
            {
                enterArray(fieldType);
                const TStructure *structure = fieldType.getStruct();
                for (const TField *nestedField : structure->fields())
                {
                    extractFieldSamplers(newPrefix, nestedField, newSequence);
                }
                exitArray(fieldType);
            }
        }
    }

    void GenerateArraySizesFromStack(TVector<unsigned int> *sizesOut)
    {
        sizesOut->reserve(mArraySizeStack.size());

        for (auto it = mArraySizeStack.rbegin(); it != mArraySizeStack.rend(); ++it)
        {
            sizesOut->push_back(*it);
        }
    }

    // Extracts a sampler from a struct. Declares the new extracted sampler.
    void extractSampler(const std::string &newName,
                        const TType &fieldType,
                        TIntermSequence *newSequence)
    {
        ASSERT(fieldType.isSampler());

        TType *newType = new TType(fieldType);

        // Add array dimensions accumulated so far due to struct arrays.  Note that to support
        // nested arrays, mArraySizeStack has the outermost size in the front.  |makeArrays| thus
        // expects this in reverse order.
        TVector<unsigned int> parentArraySizes;
        GenerateArraySizesFromStack(&parentArraySizes);
        newType->makeArrays(parentArraySizes);

        ImmutableStringBuilder nameBuilder(newName.size() + 1);
        nameBuilder << newName;

        newType->setQualifier(EvqUniform);
        TVariable *newVariable =
            new TVariable(mSymbolTable, nameBuilder, newType, SymbolType::AngleInternal);
        TIntermSymbol *newSymbol = new TIntermSymbol(newVariable);

        TIntermDeclaration *samplerDecl = new TIntermDeclaration;
        samplerDecl->appendDeclarator(newSymbol);

        newSequence->push_back(samplerDecl);

        // TODO: Use a temp name instead of generating a name as currently done.  There is no
        // guarantee that these generated names cannot clash.  Create a mapping from the previous
        // name to the name assigned to the temp variable so ShaderVariable::mappedName can be
        // updated post-transformation.  http://anglebug.com/42262930
        ASSERT(mExtractedSamplers.find(newName) == mExtractedSamplers.end());
        mExtractedSamplers[newName] = newVariable;
    }

    void enterArray(const TType &arrayType)
    {
        const angle::Span<const unsigned int> &arraySizes = arrayType.getArraySizes();
        for (auto it = arraySizes.rbegin(); it != arraySizes.rend(); ++it)
        {
            unsigned int arraySize = *it;
            mArraySizeStack.push_back(arraySize);
        }
    }

    void exitArray(const TType &arrayType)
    {
        mArraySizeStack.resize(mArraySizeStack.size() - arrayType.getNumArraySizes());
    }

    TCompiler *mCompiler;
    int mRemovedUniformsCount;

    // Map structures with samplers to ones that have their samplers removed.
    StructureMap mStructureMap;

    // Map uniform variables of structure type that are replaced with another variable.
    StructureUniformMap mStructureUniformMap;

    // Map a constructed sampler name to its variable.  Used to replace an expression that uses this
    // sampler with the extracted one.
    ExtractedSamplerMap mExtractedSamplers;

    // A stack of array sizes.  Used to figure out the array dimensions of the extracted sampler,
    // for example when it's nested in an array of structs in an array of structs.
    TVector<unsigned int> mArraySizeStack;
};
}  // anonymous namespace

bool RewriteStructSamplers(TCompiler *compiler,
                           TIntermBlock *root,
                           TSymbolTable *symbolTable,
                           int *removedUniformsCountOut)
{
    RewriteStructSamplersTraverser traverser(compiler, symbolTable);
    root->traverse(&traverser);
    *removedUniformsCountOut = traverser.removedUniformsCount();
    return traverser.updateTree(compiler, root);
}
}  // namespace sh
