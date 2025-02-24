//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include <cstring>
#include <unordered_map>
#include <unordered_set>

#include "compiler/translator/IntermRebuild.h"
#include "compiler/translator/msl/AstHelpers.h"
#include "compiler/translator/msl/DiscoverDependentFunctions.h"
#include "compiler/translator/msl/IdGen.h"
#include "compiler/translator/msl/MapSymbols.h"
#include "compiler/translator/msl/Pipeline.h"
#include "compiler/translator/msl/RewritePipelines.h"
#include "compiler/translator/msl/SymbolEnv.h"
#include "compiler/translator/msl/TranslatorMSL.h"
#include "compiler/translator/tree_ops/PruneNoOps.h"
#include "compiler/translator/tree_util/DriverUniform.h"
#include "compiler/translator/tree_util/FindMain.h"
#include "compiler/translator/tree_util/IntermTraverse.h"
#include "compiler/translator/util.h"

using namespace sh;

////////////////////////////////////////////////////////////////////////////////

namespace
{

bool IsVariableInvariant(const std::vector<sh::ShaderVariable> &mVars, const ImmutableString &name)
{
    for (const auto &var : mVars)
    {
        if (name == var.name)
        {
            return var.isInvariant;
        }
    }
    // TODO(kpidington): this should be UNREACHABLE() but isn't because the translator generates
    // declarations to unused built-in variables.
    return false;
}

using VariableSet  = std::unordered_set<const TVariable *>;
using VariableList = std::vector<const TVariable *>;

////////////////////////////////////////////////////////////////////////////////

struct PipelineStructInfo
{
    VariableSet pipelineVariables;
    PipelineScoped<TStructure> pipelineStruct;
    const TFunction *funcOriginalToModified = nullptr;
    const TFunction *funcModifiedToOriginal = nullptr;

    bool isEmpty() const
    {
        if (pipelineStruct.isTotallyEmpty())
        {
            ASSERT(pipelineVariables.empty());
            return true;
        }
        else
        {
            ASSERT(pipelineStruct.isTotallyFull());
            ASSERT(!pipelineVariables.empty());
            return false;
        }
    }
};

class GeneratePipelineStruct : private TIntermRebuild
{
  private:
    const Pipeline &mPipeline;
    SymbolEnv &mSymbolEnv;
    const std::vector<sh::ShaderVariable> *mVariableInfos;
    VariableList mPipelineVariableList;
    IdGen &mIdGen;
    PipelineStructInfo mInfo;

  public:
    static bool Exec(PipelineStructInfo &out,
                     TCompiler &compiler,
                     TIntermBlock &root,
                     IdGen &idGen,
                     const Pipeline &pipeline,
                     SymbolEnv &symbolEnv,
                     const std::vector<sh::ShaderVariable> *variableInfos)
    {
        GeneratePipelineStruct self(compiler, idGen, pipeline, symbolEnv, variableInfos);
        if (!self.exec(root))
        {
            return false;
        }
        out = self.mInfo;
        return true;
    }

  private:
    GeneratePipelineStruct(TCompiler &compiler,
                           IdGen &idGen,
                           const Pipeline &pipeline,
                           SymbolEnv &symbolEnv,
                           const std::vector<sh::ShaderVariable> *variableInfos)
        : TIntermRebuild(compiler, true, true),
          mPipeline(pipeline),
          mSymbolEnv(symbolEnv),
          mVariableInfos(variableInfos),
          mIdGen(idGen)
    {}

    bool exec(TIntermBlock &root)
    {
        if (!rebuildRoot(root))
        {
            return false;
        }

        if (mInfo.pipelineVariables.empty())
        {
            return true;
        }

        TIntermSequence seq;

        const TStructure &pipelineStruct = [&]() -> const TStructure & {
            if (mPipeline.globalInstanceVar)
            {
                return *mPipeline.globalInstanceVar->getType().getStruct();
            }
            else
            {
                return createInternalPipelineStruct(root, seq);
            }
        }();

        ModifiedStructMachineries modifiedMachineries;
        const bool isUBO     = mPipeline.type == Pipeline::Type::UniformBuffer;
        const bool isUniform = mPipeline.type == Pipeline::Type::UniformBuffer ||
                               mPipeline.type == Pipeline::Type::UserUniforms;
        const bool useAttributeAliasing =
            mPipeline.type == Pipeline::Type::VertexIn && mCompiler.supportsAttributeAliasing();
        const bool modified = TryCreateModifiedStruct(
            mCompiler, mSymbolEnv, mIdGen, mPipeline.externalStructModifyConfig(), pipelineStruct,
            mPipeline.getStructTypeName(Pipeline::Variant::Modified), modifiedMachineries, isUBO,
            !isUniform, useAttributeAliasing);

        if (modified)
        {
            ASSERT(mPipeline.type != Pipeline::Type::Texture);
            ASSERT(mPipeline.type == Pipeline::Type::AngleUniforms ||
                   !mPipeline.globalInstanceVar);  // This shouldn't happen by construction.

            auto getFunction = [](sh::TIntermFunctionDefinition *funcDecl) {
                return funcDecl ? funcDecl->getFunction() : nullptr;
            };

            const size_t size = modifiedMachineries.size();
            ASSERT(size > 0);
            for (size_t i = 0; i < size; ++i)
            {
                const ModifiedStructMachinery &machinery = modifiedMachineries.at(i);
                ASSERT(machinery.modifiedStruct);

                seq.push_back(new TIntermDeclaration{
                    &CreateStructTypeVariable(mSymbolTable, *machinery.modifiedStruct)});

                if (mPipeline.isPipelineOut())
                {
                    ASSERT(machinery.funcOriginalToModified);
                    ASSERT(!machinery.funcModifiedToOriginal);
                    seq.push_back(machinery.funcOriginalToModified);
                }
                else
                {
                    ASSERT(machinery.funcModifiedToOriginal);
                    ASSERT(!machinery.funcOriginalToModified);
                    seq.push_back(machinery.funcModifiedToOriginal);
                }

                if (i == size - 1)
                {
                    mInfo.funcOriginalToModified = getFunction(machinery.funcOriginalToModified);
                    mInfo.funcModifiedToOriginal = getFunction(machinery.funcModifiedToOriginal);

                    mInfo.pipelineStruct.internal = &pipelineStruct;
                    mInfo.pipelineStruct.external =
                        modified ? machinery.modifiedStruct : &pipelineStruct;
                }
            }
        }
        else
        {
            mInfo.pipelineStruct.internal = &pipelineStruct;
            mInfo.pipelineStruct.external = &pipelineStruct;
        }

        if (mPipeline.type == Pipeline::Type::FragmentOut &&
            mCompiler.hasPixelLocalStorageUniforms() &&
            mCompiler.getPixelLocalStorageType() == ShPixelLocalStorageType::FramebufferFetch)
        {
            auto &fields = *new TFieldList();
            for (const TField *field : mInfo.pipelineStruct.external->fields())
            {
                if (field->type()->getQualifier() == EvqFragmentInOut)
                {
                    fields.push_back(new TField(&CloneType(*field->type()), field->name(),
                                                kNoSourceLoc, field->symbolType()));
                }
            }
            TStructure *extraStruct =
                new TStructure(&mSymbolTable, ImmutableString("LastFragmentOut"), &fields,
                               SymbolType::AngleInternal);
            seq.push_back(
                new TIntermDeclaration{&CreateStructTypeVariable(mSymbolTable, *extraStruct)});
            mInfo.pipelineStruct.externalExtra = extraStruct;
        }

        root.insertChildNodes(FindMainIndex(&root), seq);

        return true;
    }

  private:
    PreResult visitFunctionDefinitionPre(TIntermFunctionDefinition &node) override
    {
        return {node, VisitBits::Neither};
    }
    PostResult visitDeclarationPost(TIntermDeclaration &declNode) override
    {
        Declaration decl     = ViewDeclaration(declNode);
        const TVariable &var = decl.symbol.variable();
        if (mPipeline.uses(var))
        {
            ASSERT(mInfo.pipelineVariables.find(&var) == mInfo.pipelineVariables.end());
            mInfo.pipelineVariables.insert(&var);
            mPipelineVariableList.push_back(&var);
            return nullptr;
        }

        return declNode;
    }

    const TStructure &createInternalPipelineStruct(TIntermBlock &root, TIntermSequence &outDeclSeq)
    {
        auto &fields = *new TFieldList();

        switch (mPipeline.type)
        {
            case Pipeline::Type::Texture:
            {
                for (const TVariable *var : mPipelineVariableList)
                {
                    const TType &varType         = var->getType();
                    const TBasicType samplerType = varType.getBasicType();

                    const TStructure &textureEnv = mSymbolEnv.getTextureEnv(samplerType);
                    auto *textureEnvType         = new TType(&textureEnv, false);
                    if (varType.isArray())
                    {
                        textureEnvType->makeArrays(varType.getArraySizes());
                    }

                    fields.push_back(
                        new TField(textureEnvType, var->name(), kNoSourceLoc, var->symbolType()));
                }
            }
            break;

            case Pipeline::Type::Image:
            {
                for (const TVariable *var : mPipelineVariableList)
                {
                    auto &type  = CloneType(var->getType());
                    auto *field = new TField(&type, var->name(), kNoSourceLoc, var->symbolType());
                    fields.push_back(field);
                }
            }
            break;

            case Pipeline::Type::UniformBuffer:
            {
                for (const TVariable *var : mPipelineVariableList)
                {
                    auto &type  = CloneType(var->getType());
                    auto *field = new TField(&type, var->name(), kNoSourceLoc, var->symbolType());
                    mSymbolEnv.markAsPointer(*field, AddressSpace::Constant);
                    mSymbolEnv.markAsUBO(*field);
                    mSymbolEnv.markAsPointer(*var, AddressSpace::Constant);
                    fields.push_back(field);
                }
            }
            break;
            default:
            {
                for (const TVariable *var : mPipelineVariableList)
                {
                    auto &type = CloneType(var->getType());
                    if (mVariableInfos && IsVariableInvariant(*mVariableInfos, var->name()))
                    {
                        type.setInvariant(true);
                    }
                    auto *field = new TField(&type, var->name(), kNoSourceLoc, var->symbolType());
                    fields.push_back(field);
                }
            }
            break;
        }

        Name pipelineStructName = mPipeline.getStructTypeName(Pipeline::Variant::Original);
        auto &s = *new TStructure(&mSymbolTable, pipelineStructName.rawName(), &fields,
                                  pipelineStructName.symbolType());

        outDeclSeq.push_back(new TIntermDeclaration{&CreateStructTypeVariable(mSymbolTable, s)});

        return s;
    }
};

////////////////////////////////////////////////////////////////////////////////

PipelineScoped<TVariable> CreatePipelineMainLocalVar(TSymbolTable &symbolTable,
                                                     const Pipeline &pipeline,
                                                     PipelineScoped<TStructure> pipelineStruct)
{
    ASSERT(pipelineStruct.isTotallyFull());

    PipelineScoped<TVariable> pipelineMainLocalVar;

    auto populateExternalMainLocalVar = [&]() {
        ASSERT(!pipelineMainLocalVar.external);
        pipelineMainLocalVar.external = &CreateInstanceVariable(
            symbolTable, *pipelineStruct.external,
            pipeline.getStructInstanceName(pipelineStruct.isUniform()
                                               ? Pipeline::Variant::Original
                                               : Pipeline::Variant::Modified));
    };

    auto populateDistinctInternalMainLocalVar = [&]() {
        ASSERT(!pipelineMainLocalVar.internal);
        pipelineMainLocalVar.internal =
            &CreateInstanceVariable(symbolTable, *pipelineStruct.internal,
                                    pipeline.getStructInstanceName(Pipeline::Variant::Original));
    };

    if (pipeline.type == Pipeline::Type::InstanceId)
    {
        populateDistinctInternalMainLocalVar();
    }
    else if (pipeline.alwaysRequiresLocalVariableDeclarationInMain())
    {
        populateExternalMainLocalVar();

        if (pipelineStruct.isUniform())
        {
            pipelineMainLocalVar.internal = pipelineMainLocalVar.external;
        }
        else
        {
            populateDistinctInternalMainLocalVar();
        }
    }
    else if (!pipelineStruct.isUniform())
    {
        populateDistinctInternalMainLocalVar();
    }

    return pipelineMainLocalVar;
}

class PipelineFunctionEnv
{
  private:
    TCompiler &mCompiler;
    SymbolEnv &mSymbolEnv;
    TSymbolTable &mSymbolTable;
    IdGen &mIdGen;
    const Pipeline &mPipeline;
    const std::unordered_set<const TFunction *> &mPipelineFunctions;
    const PipelineScoped<TStructure> mPipelineStruct;
    PipelineScoped<TVariable> &mPipelineMainLocalVar;
    size_t mFirstParamIdxInMainFn = 0;

    std::unordered_map<const TFunction *, const TFunction *> mFuncMap;

  public:
    PipelineFunctionEnv(TCompiler &compiler,
                        SymbolEnv &symbolEnv,
                        IdGen &idGen,
                        const Pipeline &pipeline,
                        const std::unordered_set<const TFunction *> &pipelineFunctions,
                        PipelineScoped<TStructure> pipelineStruct,
                        PipelineScoped<TVariable> &pipelineMainLocalVar)
        : mCompiler(compiler),
          mSymbolEnv(symbolEnv),
          mSymbolTable(symbolEnv.symbolTable()),
          mIdGen(idGen),
          mPipeline(pipeline),
          mPipelineFunctions(pipelineFunctions),
          mPipelineStruct(pipelineStruct),
          mPipelineMainLocalVar(pipelineMainLocalVar)
    {}

    bool isOriginalPipelineFunction(const TFunction &func) const
    {
        return mPipelineFunctions.find(&func) != mPipelineFunctions.end();
    }

    bool isUpdatedPipelineFunction(const TFunction &func) const
    {
        auto it = mFuncMap.find(&func);
        if (it == mFuncMap.end())
        {
            return false;
        }
        return &func == it->second;
    }

    const TFunction &getUpdatedFunction(const TFunction &func)
    {
        ASSERT(isOriginalPipelineFunction(func) || isUpdatedPipelineFunction(func));

        const TFunction *newFunc;

        auto it = mFuncMap.find(&func);
        if (it == mFuncMap.end())
        {
            const bool isMain = func.isMain();
            if (isMain)
            {
                mFirstParamIdxInMainFn = func.getParamCount();
            }

            if (isMain && mPipeline.isPipelineOut())
            {
                ASSERT(func.getReturnType().getBasicType() == TBasicType::EbtVoid);
                newFunc = &CloneFunctionAndChangeReturnType(mSymbolTable, nullptr, func,
                                                            *mPipelineStruct.external);
                if (mPipeline.type == Pipeline::Type::FragmentOut &&
                    mCompiler.hasPixelLocalStorageUniforms() &&
                    mCompiler.getPixelLocalStorageType() ==
                        ShPixelLocalStorageType::FramebufferFetch)
                {
                    // Add an input argument to main() that contains the current framebuffer
                    // attachment values, for loading pixel local storage.
                    TType *type = new TType(mPipelineStruct.externalExtra, true);
                    TVariable *lastFragmentOut =
                        new TVariable(&mSymbolTable, ImmutableString("lastFragmentOut"), type,
                                      SymbolType::AngleInternal);
                    newFunc = &CloneFunctionAndPrependParam(mSymbolTable, nullptr, *newFunc,
                                                            *lastFragmentOut);
                    // Initialize the main local variable with the current framebuffer contents.
                    mPipelineMainLocalVar.externalExtra = lastFragmentOut;
                }
            }
            else if (isMain && (mPipeline.type == Pipeline::Type::InvocationVertexGlobals))
            {
                ASSERT(mPipelineStruct.external->fields().size() == 1);
                ASSERT(mPipelineStruct.external->fields()[0]->type()->getQualifier() ==
                       TQualifier::EvqVertexID);
                auto *vertexIDMetalVar =
                    new TVariable(&mSymbolTable, ImmutableString("vertexIDMetal"),
                                  new TType(TBasicType::EbtUInt), SymbolType::AngleInternal);
                newFunc                        = &func;
                mPipelineMainLocalVar.external = vertexIDMetalVar;
            }
            else if (isMain && (mPipeline.type == Pipeline::Type::InvocationFragmentGlobals))
            {
                std::vector<const TVariable *> variables;
                for (const TField *field : mPipelineStruct.external->fields())
                {
                    variables.push_back(new TVariable(&mSymbolTable, field->name(), field->type(),
                                                      field->symbolType()));
                }
                newFunc = &CloneFunctionAndAppendParams(mSymbolTable, nullptr, func, variables);
            }
            else if (isMain && mPipeline.type == Pipeline::Type::Texture)
            {
                std::vector<const TVariable *> variables;
                TranslatorMetalReflection *reflection =
                    mtl::getTranslatorMetalReflection(&mCompiler);
                for (const TField *field : mPipelineStruct.external->fields())
                {
                    const TStructure *textureEnv = field->type()->getStruct();
                    ASSERT(textureEnv && textureEnv->fields().size() == 2);
                    for (const TField *subfield : textureEnv->fields())
                    {
                        const Name name = mIdGen.createNewName({field->name(), subfield->name()});
                        TType &type     = *new TType(*subfield->type());
                        ASSERT(!type.isArray());
                        type.makeArrays(field->type()->getArraySizes());
                        auto *var =
                            new TVariable(&mSymbolTable, name.rawName(), &type, name.symbolType());
                        variables.push_back(var);
                        reflection->addOriginalName(var->uniqueId().get(), field->name().data());
                    }
                }
                newFunc = &CloneFunctionAndAppendParams(mSymbolTable, nullptr, func, variables);
            }
            else if (isMain && mPipeline.type == Pipeline::Type::InstanceId)
            {
                Name instanceIdName = mPipeline.getStructInstanceName(Pipeline::Variant::Modified);
                auto *instanceIdVar =
                    new TVariable(&mSymbolTable, instanceIdName.rawName(),
                                  new TType(TBasicType::EbtUInt), instanceIdName.symbolType());

                auto *baseInstanceVar =
                    new TVariable(&mSymbolTable, kBaseInstanceName.rawName(),
                                  new TType(TBasicType::EbtUInt), kBaseInstanceName.symbolType());

                newFunc = &CloneFunctionAndPrependTwoParams(mSymbolTable, nullptr, func,
                                                            *instanceIdVar, *baseInstanceVar);
                mPipelineMainLocalVar.external      = instanceIdVar;
                mPipelineMainLocalVar.externalExtra = baseInstanceVar;
            }
            else if (isMain && mPipeline.alwaysRequiresLocalVariableDeclarationInMain())
            {
                ASSERT(mPipelineMainLocalVar.isTotallyFull());
                newFunc = &func;
            }
            else
            {
                const TVariable *var;
                AddressSpace addressSpace;

                if (isMain && !mPipelineMainLocalVar.isUniform())
                {
                    var = &CreateInstanceVariable(
                        mSymbolTable, *mPipelineStruct.external,
                        mPipeline.getStructInstanceName(Pipeline::Variant::Modified));
                    addressSpace = mPipeline.externalAddressSpace();
                }
                else
                {
                    var = &CreateInstanceVariable(
                        mSymbolTable, *mPipelineStruct.internal,
                        mPipeline.getStructInstanceName(Pipeline::Variant::Original));
                    addressSpace = mPipelineMainLocalVar.isUniform()
                                       ? mPipeline.externalAddressSpace()
                                       : AddressSpace::Thread;
                }

                bool markAsReference = true;
                if (isMain)
                {
                    switch (mPipeline.type)
                    {
                        case Pipeline::Type::VertexIn:
                        case Pipeline::Type::FragmentIn:
                        case Pipeline::Type::Image:
                            markAsReference = false;
                            break;

                        default:
                            break;
                    }
                }

                if (markAsReference)
                {
                    mSymbolEnv.markAsReference(*var, addressSpace);
                }

                newFunc = &CloneFunctionAndPrependParam(mSymbolTable, nullptr, func, *var);
            }

            mFuncMap[&func]   = newFunc;
            mFuncMap[newFunc] = newFunc;
        }
        else
        {
            newFunc = it->second;
        }

        return *newFunc;
    }

    TIntermFunctionPrototype *createUpdatedFunctionPrototype(
        TIntermFunctionPrototype &funcProtoNode)
    {
        const TFunction &func = *funcProtoNode.getFunction();
        if (!isOriginalPipelineFunction(func) && !isUpdatedPipelineFunction(func))
        {
            return nullptr;
        }
        const TFunction &newFunc = getUpdatedFunction(func);
        return new TIntermFunctionPrototype(&newFunc);
    }

    size_t getFirstParamIdxInMainFn() const { return mFirstParamIdxInMainFn; }
};

class UpdatePipelineFunctions : private TIntermRebuild
{
  private:
    const Pipeline &mPipeline;
    const PipelineScoped<TStructure> mPipelineStruct;
    PipelineScoped<TVariable> &mPipelineMainLocalVar;
    SymbolEnv &mSymbolEnv;
    PipelineFunctionEnv mEnv;
    const TFunction *mFuncOriginalToModified;
    const TFunction *mFuncModifiedToOriginal;

  public:
    static bool ThreadPipeline(TCompiler &compiler,
                               TIntermBlock &root,
                               const Pipeline &pipeline,
                               const std::unordered_set<const TFunction *> &pipelineFunctions,
                               PipelineScoped<TStructure> pipelineStruct,
                               PipelineScoped<TVariable> &pipelineMainLocalVar,
                               IdGen &idGen,
                               SymbolEnv &symbolEnv,
                               const TFunction *funcOriginalToModified,
                               const TFunction *funcModifiedToOriginal)
    {
        UpdatePipelineFunctions self(compiler, pipeline, pipelineFunctions, pipelineStruct,
                                     pipelineMainLocalVar, idGen, symbolEnv, funcOriginalToModified,
                                     funcModifiedToOriginal);
        if (!self.rebuildRoot(root))
        {
            return false;
        }
        return true;
    }

  private:
    UpdatePipelineFunctions(TCompiler &compiler,
                            const Pipeline &pipeline,
                            const std::unordered_set<const TFunction *> &pipelineFunctions,
                            PipelineScoped<TStructure> pipelineStruct,
                            PipelineScoped<TVariable> &pipelineMainLocalVar,
                            IdGen &idGen,
                            SymbolEnv &symbolEnv,
                            const TFunction *funcOriginalToModified,
                            const TFunction *funcModifiedToOriginal)
        : TIntermRebuild(compiler, false, true),
          mPipeline(pipeline),
          mPipelineStruct(pipelineStruct),
          mPipelineMainLocalVar(pipelineMainLocalVar),
          mSymbolEnv(symbolEnv),
          mEnv(compiler,
               symbolEnv,
               idGen,
               pipeline,
               pipelineFunctions,
               pipelineStruct,
               mPipelineMainLocalVar),
          mFuncOriginalToModified(funcOriginalToModified),
          mFuncModifiedToOriginal(funcModifiedToOriginal)
    {
        ASSERT(mPipelineStruct.isTotallyFull());
    }

    const TVariable &getInternalPipelineVariable(const TFunction &pipelineFunc)
    {
        if (pipelineFunc.isMain() && (mPipeline.alwaysRequiresLocalVariableDeclarationInMain() ||
                                      !mPipelineMainLocalVar.isUniform()))
        {
            ASSERT(mPipelineMainLocalVar.internal);
            return *mPipelineMainLocalVar.internal;
        }
        else
        {
            ASSERT(pipelineFunc.getParamCount() > 0);
            return *pipelineFunc.getParam(0);
        }
    }

    const TVariable &getExternalPipelineVariable(const TFunction &mainFunc)
    {
        ASSERT(mainFunc.isMain());
        if (mPipelineMainLocalVar.external)
        {
            return *mPipelineMainLocalVar.external;
        }
        else
        {
            ASSERT(mainFunc.getParamCount() > 0);
            return *mainFunc.getParam(0);
        }
    }

    const TVariable &getExternalExtraPipelineVariable(const TFunction &mainFunc)
    {
        ASSERT(mainFunc.isMain());
        if (mPipelineMainLocalVar.externalExtra)
        {
            return *mPipelineMainLocalVar.externalExtra;
        }
        else
        {
            ASSERT(mainFunc.getParamCount() > 1);
            return *mainFunc.getParam(1);
        }
    }

    PostResult visitAggregatePost(TIntermAggregate &callNode) override
    {
        if (callNode.isConstructor())
        {
            return callNode;
        }
        else
        {
            const TFunction &oldCalledFunc = *callNode.getFunction();
            if (!mEnv.isOriginalPipelineFunction(oldCalledFunc))
            {
                return callNode;
            }
            const TFunction &newCalledFunc = mEnv.getUpdatedFunction(oldCalledFunc);

            const TFunction *oldOwnerFunc = getParentFunction();
            ASSERT(oldOwnerFunc);
            const TFunction &newOwnerFunc = mEnv.getUpdatedFunction(*oldOwnerFunc);

            return *TIntermAggregate::CreateFunctionCall(
                newCalledFunc, &CloneSequenceAndPrepend(
                                   *callNode.getSequence(),
                                   *new TIntermSymbol(&getInternalPipelineVariable(newOwnerFunc))));
        }
    }

    PostResult visitFunctionPrototypePost(TIntermFunctionPrototype &funcProtoNode) override
    {
        TIntermFunctionPrototype *newFuncProtoNode =
            mEnv.createUpdatedFunctionPrototype(funcProtoNode);
        if (newFuncProtoNode == nullptr)
        {
            return funcProtoNode;
        }
        return *newFuncProtoNode;
    }

    PostResult visitFunctionDefinitionPost(TIntermFunctionDefinition &funcDefNode) override
    {
        if (funcDefNode.getFunction()->isMain())
        {
            return visitMain(funcDefNode);
        }
        else
        {
            return visitNonMain(funcDefNode);
        }
    }

    TIntermNode &visitNonMain(TIntermFunctionDefinition &funcDefNode)
    {
        TIntermFunctionPrototype &funcProtoNode = *funcDefNode.getFunctionPrototype();
        ASSERT(!funcProtoNode.getFunction()->isMain());

        TIntermFunctionPrototype *newFuncProtoNode =
            mEnv.createUpdatedFunctionPrototype(funcProtoNode);
        if (newFuncProtoNode == nullptr)
        {
            return funcDefNode;
        }

        const TFunction &func = *newFuncProtoNode->getFunction();
        ASSERT(!func.isMain());

        TIntermBlock *body = funcDefNode.getBody();

        return *new TIntermFunctionDefinition(newFuncProtoNode, body);
    }

    TIntermNode &visitMain(TIntermFunctionDefinition &funcDefNode)
    {
        TIntermFunctionPrototype &funcProtoNode = *funcDefNode.getFunctionPrototype();
        ASSERT(funcProtoNode.getFunction()->isMain());

        TIntermFunctionPrototype *newFuncProtoNode =
            mEnv.createUpdatedFunctionPrototype(funcProtoNode);
        if (newFuncProtoNode == nullptr)
        {
            return funcDefNode;
        }

        const TFunction &func = *newFuncProtoNode->getFunction();
        ASSERT(func.isMain());

        auto callModifiedToOriginal = [&](TIntermBlock &body) {
            ASSERT(mPipelineMainLocalVar.internal);
            if (!mPipeline.isPipelineOut())
            {
                ASSERT(mFuncModifiedToOriginal);
                auto *m = new TIntermSymbol(&getExternalPipelineVariable(func));
                auto *o = new TIntermSymbol(mPipelineMainLocalVar.internal);
                body.appendStatement(TIntermAggregate::CreateFunctionCall(
                    *mFuncModifiedToOriginal, new TIntermSequence{m, o}));
            }
        };

        auto callOriginalToModified = [&](TIntermBlock &body) {
            ASSERT(mPipelineMainLocalVar.internal);
            if (mPipeline.isPipelineOut())
            {
                ASSERT(mFuncOriginalToModified);
                auto *o = new TIntermSymbol(mPipelineMainLocalVar.internal);
                auto *m = new TIntermSymbol(&getExternalPipelineVariable(func));
                body.appendStatement(TIntermAggregate::CreateFunctionCall(
                    *mFuncOriginalToModified, new TIntermSequence{o, m}));
            }
        };

        TIntermBlock *body = funcDefNode.getBody();

        if (mPipeline.alwaysRequiresLocalVariableDeclarationInMain())
        {
            ASSERT(mPipelineMainLocalVar.isTotallyFull());

            auto *newBody = new TIntermBlock();
            newBody->appendStatement(new TIntermDeclaration{mPipelineMainLocalVar.internal});

            if (mPipeline.type == Pipeline::Type::InvocationVertexGlobals)
            {
                ASSERT(mPipelineStruct.external->fields().size() == 1);
                ASSERT(mPipelineStruct.external->fields()[0]->type()->getQualifier() ==
                       TQualifier::EvqVertexID);
                const TField *field = mPipelineStruct.external->fields()[0];
                auto *var =
                    new TVariable(&mSymbolTable, field->name(), field->type(), field->symbolType());
                auto &accessNode   = AccessField(*mPipelineMainLocalVar.internal, Name(*var));
                auto vertexIDMetal = new TIntermSymbol(&getExternalPipelineVariable(func));
                auto *assignNode   = new TIntermBinary(
                    TOperator::EOpAssign, &accessNode,
                    &AsType(mSymbolEnv, *new TType(TBasicType::EbtInt), *vertexIDMetal));
                newBody->appendStatement(assignNode);
            }
            else if (mPipeline.type == Pipeline::Type::InvocationFragmentGlobals)
            {
                // Populate struct instance with references to global pipeline variables.
                for (const TField *field : mPipelineStruct.external->fields())
                {
                    auto *var        = new TVariable(&mSymbolTable, field->name(), field->type(),
                                                     field->symbolType());
                    auto *symbol     = new TIntermSymbol(var);
                    auto &accessNode = AccessField(*mPipelineMainLocalVar.internal, Name(*var));
                    auto *assignNode = new TIntermBinary(TOperator::EOpAssign, &accessNode, symbol);
                    newBody->appendStatement(assignNode);
                }
            }
            else if (mPipeline.type == Pipeline::Type::FragmentOut &&
                     mCompiler.hasPixelLocalStorageUniforms() &&
                     mCompiler.getPixelLocalStorageType() ==
                         ShPixelLocalStorageType::FramebufferFetch)
            {
                ASSERT(mPipelineMainLocalVar.externalExtra);
                auto &lastFragmentOut = *mPipelineMainLocalVar.externalExtra;
                for (const TField *field : lastFragmentOut.getType().getStruct()->fields())
                {
                    auto &accessNode = AccessField(*mPipelineMainLocalVar.internal, Name(*field));
                    auto &sourceNode = AccessField(lastFragmentOut, Name(*field));
                    auto *assignNode =
                        new TIntermBinary(TOperator::EOpAssign, &accessNode, &sourceNode);
                    newBody->appendStatement(assignNode);
                }
            }
            else if (mPipeline.type == Pipeline::Type::Texture)
            {
                const TFieldList &fields = mPipelineStruct.external->fields();

                ASSERT(func.getParamCount() >= mEnv.getFirstParamIdxInMainFn() + 2 * fields.size());
                size_t paramIndex = mEnv.getFirstParamIdxInMainFn();

                for (const TField *field : fields)
                {
                    const TVariable &textureParam = *func.getParam(paramIndex++);
                    const TVariable &samplerParam = *func.getParam(paramIndex++);

                    auto go = [&](TIntermTyped &env, const int *index) {
                        TIntermTyped &textureField =
                            AccessField(AccessIndex(*env.deepCopy(), index),
                                        Name("texture", SymbolType::BuiltIn));
                        TIntermTyped &samplerField =
                            AccessField(AccessIndex(*env.deepCopy(), index),
                                        Name("sampler", SymbolType::BuiltIn));

                        auto mkAssign = [&](TIntermTyped &field, const TVariable &param) {
                            return new TIntermBinary(TOperator::EOpAssign, &field,
                                                     &mSymbolEnv.callFunctionOverload(
                                                         Name("addressof"), field.getType(),
                                                         *new TIntermSequence{&AccessIndex(
                                                             *new TIntermSymbol(&param), index)}));
                        };

                        newBody->appendStatement(mkAssign(textureField, textureParam));
                        newBody->appendStatement(mkAssign(samplerField, samplerParam));
                    };

                    TIntermTyped &env = AccessField(*mPipelineMainLocalVar.internal, Name(*field));
                    const TType &envType = env.getType();

                    if (envType.isArray())
                    {
                        ASSERT(!envType.isArrayOfArrays());
                        const auto n = static_cast<int>(envType.getArraySizeProduct());
                        for (int i = 0; i < n; ++i)
                        {
                            go(env, &i);
                        }
                    }
                    else
                    {
                        go(env, nullptr);
                    }
                }
            }
            else if (mPipeline.type == Pipeline::Type::InstanceId)
            {
                auto varInstanceId   = new TIntermSymbol(&getExternalPipelineVariable(func));
                auto varBaseInstance = new TIntermSymbol(&getExternalExtraPipelineVariable(func));

                newBody->appendStatement(new TIntermBinary(
                    TOperator::EOpAssign,
                    &AccessFieldByIndex(*new TIntermSymbol(&getInternalPipelineVariable(func)), 0),
                    &AsType(
                        mSymbolEnv, *new TType(TBasicType::EbtInt),
                        *new TIntermBinary(TOperator::EOpSub, varInstanceId, varBaseInstance))));
            }
            else if (!mPipelineMainLocalVar.isUniform())
            {
                newBody->appendStatement(new TIntermDeclaration{mPipelineMainLocalVar.external});
                callModifiedToOriginal(*newBody);
            }

            newBody->appendStatement(body);

            if (!mPipelineMainLocalVar.isUniform())
            {
                callOriginalToModified(*newBody);
            }

            if (mPipeline.isPipelineOut())
            {
                newBody->appendStatement(new TIntermBranch(
                    TOperator::EOpReturn, new TIntermSymbol(mPipelineMainLocalVar.external)));
            }

            body = newBody;
        }
        else if (!mPipelineMainLocalVar.isUniform())
        {
            ASSERT(!mPipelineMainLocalVar.external);
            ASSERT(mPipelineMainLocalVar.internal);

            auto *newBody = new TIntermBlock();
            newBody->appendStatement(new TIntermDeclaration{mPipelineMainLocalVar.internal});
            callModifiedToOriginal(*newBody);
            newBody->appendStatement(body);
            callOriginalToModified(*newBody);
            body = newBody;
        }

        return *new TIntermFunctionDefinition(newFuncProtoNode, body);
    }
};

////////////////////////////////////////////////////////////////////////////////

bool UpdatePipelineSymbols(Pipeline::Type pipelineType,
                           TCompiler &compiler,
                           TIntermBlock &root,
                           SymbolEnv &symbolEnv,
                           const VariableSet &pipelineVariables,
                           PipelineScoped<TVariable> pipelineMainLocalVar)
{
    auto map = [&](const TFunction *owner, TIntermSymbol &symbol) -> TIntermNode & {
        if (!owner)
            return symbol;
        const TVariable &var = symbol.variable();
        if (pipelineVariables.find(&var) == pipelineVariables.end())
        {
            return symbol;
        }
        const TVariable *structInstanceVar;
        if (owner->isMain() && pipelineType != Pipeline::Type::FragmentIn)
        {
            ASSERT(pipelineMainLocalVar.internal);
            structInstanceVar = pipelineMainLocalVar.internal;
        }
        else
        {
            ASSERT(owner->getParamCount() > 0);
            structInstanceVar = owner->getParam(0);
        }
        ASSERT(structInstanceVar);
        return AccessField(*structInstanceVar, Name(var));
    };
    return MapSymbols(compiler, root, map);
}

////////////////////////////////////////////////////////////////////////////////

bool RewritePipeline(TCompiler &compiler,
                     TIntermBlock &root,
                     IdGen &idGen,
                     const Pipeline &pipeline,
                     SymbolEnv &symbolEnv,
                     const std::vector<sh::ShaderVariable> *variableInfo,
                     PipelineScoped<TStructure> &outStruct)
{
    ASSERT(outStruct.isTotallyEmpty());

    TSymbolTable &symbolTable = compiler.getSymbolTable();

    PipelineStructInfo psi;
    if (!GeneratePipelineStruct::Exec(psi, compiler, root, idGen, pipeline, symbolEnv,
                                      variableInfo))
    {
        return false;
    }

    if (psi.isEmpty())
    {
        return true;
    }

    const auto pipelineFunctions = DiscoverDependentFunctions(root, [&](const TVariable &var) {
        return psi.pipelineVariables.find(&var) != psi.pipelineVariables.end();
    });

    auto pipelineMainLocalVar =
        CreatePipelineMainLocalVar(symbolTable, pipeline, psi.pipelineStruct);

    if (!UpdatePipelineFunctions::ThreadPipeline(
            compiler, root, pipeline, pipelineFunctions, psi.pipelineStruct, pipelineMainLocalVar,
            idGen, symbolEnv, psi.funcOriginalToModified, psi.funcModifiedToOriginal))
    {
        return false;
    }

    if (!pipeline.globalInstanceVar)
    {
        if (!UpdatePipelineSymbols(pipeline.type, compiler, root, symbolEnv, psi.pipelineVariables,
                                   pipelineMainLocalVar))
        {
            return false;
        }
    }

    if (!PruneNoOps(&compiler, &root, &compiler.getSymbolTable()))
    {
        return false;
    }

    outStruct = psi.pipelineStruct;
    return true;
}

}  // anonymous namespace

bool sh::RewritePipelines(TCompiler &compiler,
                          TIntermBlock &root,
                          const std::vector<sh::ShaderVariable> &inputVaryings,
                          const std::vector<sh::ShaderVariable> &outputVaryings,
                          IdGen &idGen,
                          DriverUniform &angleUniformsGlobalInstanceVar,
                          SymbolEnv &symbolEnv,
                          PipelineStructs &outStructs)
{
    struct Info
    {
        Pipeline::Type pipelineType;
        PipelineScoped<TStructure> &outStruct;
        const TVariable *globalInstanceVar;
        const std::vector<sh::ShaderVariable> *variableInfo;
    };

    Info infos[] = {
        {Pipeline::Type::InstanceId, outStructs.instanceId, nullptr, nullptr},
        {Pipeline::Type::Texture, outStructs.image, nullptr, nullptr},
        {Pipeline::Type::Image, outStructs.texture, nullptr, nullptr},
        {Pipeline::Type::NonConstantGlobals, outStructs.nonConstantGlobals, nullptr, nullptr},
        {Pipeline::Type::AngleUniforms, outStructs.angleUniforms,
         angleUniformsGlobalInstanceVar.getDriverUniformsVariable(), nullptr},
        {Pipeline::Type::UserUniforms, outStructs.userUniforms, nullptr, nullptr},
        {Pipeline::Type::VertexIn, outStructs.vertexIn, nullptr, &inputVaryings},
        {Pipeline::Type::VertexOut, outStructs.vertexOut, nullptr, &outputVaryings},
        {Pipeline::Type::FragmentIn, outStructs.fragmentIn, nullptr, &inputVaryings},
        {Pipeline::Type::FragmentOut, outStructs.fragmentOut, nullptr, &outputVaryings},
        {Pipeline::Type::InvocationVertexGlobals, outStructs.invocationVertexGlobals, nullptr,
         nullptr},
        {Pipeline::Type::InvocationFragmentGlobals, outStructs.invocationFragmentGlobals, nullptr,
         &inputVaryings},
        {Pipeline::Type::UniformBuffer, outStructs.uniformBuffers, nullptr, nullptr},
    };

    for (Info &info : infos)
    {
        if ((compiler.getShaderType() != GL_VERTEX_SHADER &&
             (info.pipelineType == Pipeline::Type::VertexIn ||
              info.pipelineType == Pipeline::Type::VertexOut ||
              info.pipelineType == Pipeline::Type::InvocationVertexGlobals)) ||
            (compiler.getShaderType() != GL_FRAGMENT_SHADER &&
             (info.pipelineType == Pipeline::Type::FragmentIn ||
              info.pipelineType == Pipeline::Type::FragmentOut ||
              info.pipelineType == Pipeline::Type::InvocationFragmentGlobals)))
            continue;

        Pipeline pipeline{info.pipelineType, info.globalInstanceVar};
        if (!RewritePipeline(compiler, root, idGen, pipeline, symbolEnv, info.variableInfo,
                             info.outStruct))
        {
            return false;
        }
    }

    return true;
}
