//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CollectVariables.cpp: Collect lists of shader interface variables based on the AST.

#include "compiler/translator/CollectVariables.h"

#include "angle_gl.h"
#include "common/utilities.h"
#include "compiler/translator/HashNames.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_util/IntermTraverse.h"
#include "compiler/translator/util.h"

namespace sh
{

namespace
{

BlockLayoutType GetBlockLayoutType(TLayoutBlockStorage blockStorage)
{
    switch (blockStorage)
    {
        case EbsPacked:
            return BLOCKLAYOUT_PACKED;
        case EbsShared:
            return BLOCKLAYOUT_SHARED;
        case EbsStd140:
            return BLOCKLAYOUT_STD140;
        case EbsStd430:
            return BLOCKLAYOUT_STD430;
        default:
            UNREACHABLE();
            return BLOCKLAYOUT_SHARED;
    }
}

BlockType GetBlockType(TQualifier qualifier)
{
    switch (qualifier)
    {
        case EvqUniform:
            return BlockType::kBlockUniform;
        case EvqBuffer:
            return BlockType::kBlockBuffer;
        case EvqPixelLocalEXT:
            return BlockType::kPixelLocalExt;
        default:
            UNREACHABLE();
            return BlockType::kBlockUniform;
    }
}

template <class VarT>
VarT *FindVariable(const ImmutableString &name, std::vector<VarT> *infoList)
{
    // TODO(zmo): optimize this function.
    for (size_t ii = 0; ii < infoList->size(); ++ii)
    {
        if (name == (*infoList)[ii].name)
            return &((*infoList)[ii]);
    }

    return nullptr;
}

void MarkActive(ShaderVariable *variable)
{
    if (!variable->active)
    {
        if (variable->isStruct())
        {
            // Conservatively assume all fields are statically used as well.
            for (auto &field : variable->fields)
            {
                MarkActive(&field);
            }
        }
        variable->staticUse = true;
        variable->active    = true;
    }
}

ShaderVariable *FindVariableInInterfaceBlock(const ImmutableString &name,
                                             const TInterfaceBlock *interfaceBlock,
                                             std::vector<InterfaceBlock> *infoList)
{
    ASSERT(interfaceBlock);
    InterfaceBlock *namedBlock = FindVariable(interfaceBlock->name(), infoList);
    ASSERT(namedBlock);

    // Set static use on the parent interface block here
    namedBlock->staticUse = true;
    namedBlock->active    = true;
    return FindVariable(name, &namedBlock->fields);
}

ShaderVariable *FindShaderIOBlockVariable(const ImmutableString &blockName,
                                          std::vector<ShaderVariable> *infoList)
{
    for (size_t index = 0; index < infoList->size(); ++index)
    {
        if (blockName == (*infoList)[index].structOrBlockName)
            return &(*infoList)[index];
    }

    return nullptr;
}

// Traverses the intermediate tree to collect all attributes, uniforms, varyings, fragment outputs,
// shared data and interface blocks.
class CollectVariablesTraverser : public TIntermTraverser
{
  public:
    CollectVariablesTraverser(std::vector<ShaderVariable> *attribs,
                              std::vector<ShaderVariable> *outputVariables,
                              std::vector<ShaderVariable> *uniforms,
                              std::vector<ShaderVariable> *inputVaryings,
                              std::vector<ShaderVariable> *outputVaryings,
                              std::vector<ShaderVariable> *sharedVariables,
                              std::vector<InterfaceBlock> *uniformBlocks,
                              std::vector<InterfaceBlock> *shaderStorageBlocks,
                              ShHashFunction64 hashFunction,
                              TSymbolTable *symbolTable,
                              GLenum shaderType,
                              const TExtensionBehavior &extensionBehavior,
                              const ShBuiltInResources &resources,
                              int tessControlShaderOutputVertices);

    bool visitGlobalQualifierDeclaration(Visit visit,
                                         TIntermGlobalQualifierDeclaration *node) override;
    void visitSymbol(TIntermSymbol *symbol) override;
    bool visitDeclaration(Visit, TIntermDeclaration *node) override;
    bool visitBinary(Visit visit, TIntermBinary *binaryNode) override;

  private:
    std::string getMappedName(const TSymbol *symbol) const;

    void setFieldOrVariableProperties(const TType &type,
                                      bool staticUse,
                                      bool isShaderIOBlock,
                                      bool isPatch,
                                      ShaderVariable *variableOut) const;
    void setFieldProperties(const TType &type,
                            const ImmutableString &name,
                            bool staticUse,
                            bool isShaderIOBlock,
                            bool isPatch,
                            SymbolType symbolType,
                            ShaderVariable *variableOut) const;
    void setCommonVariableProperties(const TType &type,
                                     const TVariable &variable,
                                     ShaderVariable *variableOut) const;

    ShaderVariable recordAttribute(const TIntermSymbol &variable) const;
    ShaderVariable recordOutputVariable(const TIntermSymbol &variable) const;
    ShaderVariable recordVarying(const TIntermSymbol &variable) const;
    void recordInterfaceBlock(const char *instanceName,
                              const TType &interfaceBlockType,
                              InterfaceBlock *interfaceBlock) const;
    ShaderVariable recordUniform(const TIntermSymbol &variable) const;

    void setBuiltInInfoFromSymbol(const TVariable &variable, ShaderVariable *info);

    void recordBuiltInVaryingUsed(const TVariable &variable,
                                  bool *addedFlag,
                                  std::vector<ShaderVariable> *varyings);
    void recordBuiltInFragmentOutputUsed(const TVariable &variable, bool *addedFlag);
    void recordBuiltInAttributeUsed(const TVariable &variable, bool *addedFlag);
    InterfaceBlock *findNamedInterfaceBlock(const ImmutableString &name) const;

    std::vector<ShaderVariable> *mAttribs;
    std::vector<ShaderVariable> *mOutputVariables;
    std::vector<ShaderVariable> *mUniforms;
    std::vector<ShaderVariable> *mInputVaryings;
    std::vector<ShaderVariable> *mOutputVaryings;
    std::vector<ShaderVariable> *mSharedVariables;
    std::vector<InterfaceBlock> *mUniformBlocks;
    std::vector<InterfaceBlock> *mShaderStorageBlocks;

    std::map<std::string, ShaderVariable *> mInterfaceBlockFields;

    // Shader uniforms
    bool mDepthRangeAdded;
    bool mNumSamplesAdded;

    // Compute Shader builtins
    bool mNumWorkGroupsAdded;
    bool mWorkGroupIDAdded;
    bool mLocalInvocationIDAdded;
    bool mGlobalInvocationIDAdded;
    bool mLocalInvocationIndexAdded;

    // Vertex Shader builtins
    bool mInstanceIDAdded;
    bool mVertexIDAdded;
    bool mPointSizeAdded;
    bool mDrawIDAdded;

    // Vertex Shader and Geometry Shader builtins
    bool mPositionAdded;
    bool mClipDistanceAdded;
    bool mCullDistanceAdded;

    // Fragment Shader builtins
    bool mPointCoordAdded;
    bool mFrontFacingAdded;
    bool mHelperInvocationAdded;
    bool mFragCoordAdded;
    bool mLastFragDataAdded;
    bool mLastFragColorAdded;
    bool mFragColorAdded;
    bool mFragDataAdded;
    bool mFragDepthAdded;
    bool mSecondaryFragColorEXTAdded;
    bool mSecondaryFragDataEXTAdded;
    bool mSampleIDAdded;
    bool mSamplePositionAdded;
    bool mSampleMaskAdded;
    bool mSampleMaskInAdded;

    // Geometry and Tessellation Shader builtins
    bool mPerVertexInAdded;
    bool mPerVertexOutAdded;

    // Geometry Shader builtins
    bool mPrimitiveIDInAdded;
    bool mInvocationIDAdded;

    // Geometry Shader and Fragment Shader builtins
    bool mPrimitiveIDAdded;
    bool mLayerAdded;

    // Shared memory variables
    bool mSharedVariableAdded;

    // Tessellation Shader builtins
    bool mPatchVerticesInAdded;
    bool mTessLevelOuterAdded;
    bool mTessLevelInnerAdded;
    bool mBoundingBoxAdded;
    bool mTessCoordAdded;
    const int mTessControlShaderOutputVertices;

    ShHashFunction64 mHashFunction;

    GLenum mShaderType;
    const TExtensionBehavior &mExtensionBehavior;
    const ShBuiltInResources &mResources;
};

CollectVariablesTraverser::CollectVariablesTraverser(
    std::vector<sh::ShaderVariable> *attribs,
    std::vector<sh::ShaderVariable> *outputVariables,
    std::vector<sh::ShaderVariable> *uniforms,
    std::vector<sh::ShaderVariable> *inputVaryings,
    std::vector<sh::ShaderVariable> *outputVaryings,
    std::vector<sh::ShaderVariable> *sharedVariables,
    std::vector<sh::InterfaceBlock> *uniformBlocks,
    std::vector<sh::InterfaceBlock> *shaderStorageBlocks,
    ShHashFunction64 hashFunction,
    TSymbolTable *symbolTable,
    GLenum shaderType,
    const TExtensionBehavior &extensionBehavior,
    const ShBuiltInResources &resources,
    int tessControlShaderOutputVertices)
    : TIntermTraverser(true, false, false, symbolTable),
      mAttribs(attribs),
      mOutputVariables(outputVariables),
      mUniforms(uniforms),
      mInputVaryings(inputVaryings),
      mOutputVaryings(outputVaryings),
      mSharedVariables(sharedVariables),
      mUniformBlocks(uniformBlocks),
      mShaderStorageBlocks(shaderStorageBlocks),
      mDepthRangeAdded(false),
      mNumSamplesAdded(false),
      mNumWorkGroupsAdded(false),
      mWorkGroupIDAdded(false),
      mLocalInvocationIDAdded(false),
      mGlobalInvocationIDAdded(false),
      mLocalInvocationIndexAdded(false),
      mInstanceIDAdded(false),
      mVertexIDAdded(false),
      mPointSizeAdded(false),
      mDrawIDAdded(false),
      mPositionAdded(false),
      mClipDistanceAdded(false),
      mCullDistanceAdded(false),
      mPointCoordAdded(false),
      mFrontFacingAdded(false),
      mHelperInvocationAdded(false),
      mFragCoordAdded(false),
      mLastFragDataAdded(false),
      mLastFragColorAdded(false),
      mFragColorAdded(false),
      mFragDataAdded(false),
      mFragDepthAdded(false),
      mSecondaryFragColorEXTAdded(false),
      mSecondaryFragDataEXTAdded(false),
      mSampleIDAdded(false),
      mSamplePositionAdded(false),
      mSampleMaskAdded(false),
      mSampleMaskInAdded(false),
      mPerVertexInAdded(false),
      mPerVertexOutAdded(false),
      mPrimitiveIDInAdded(false),
      mInvocationIDAdded(false),
      mPrimitiveIDAdded(false),
      mLayerAdded(false),
      mSharedVariableAdded(false),
      mPatchVerticesInAdded(false),
      mTessLevelOuterAdded(false),
      mTessLevelInnerAdded(false),
      mBoundingBoxAdded(false),
      mTessCoordAdded(false),
      mTessControlShaderOutputVertices(tessControlShaderOutputVertices),
      mHashFunction(hashFunction),
      mShaderType(shaderType),
      mExtensionBehavior(extensionBehavior),
      mResources(resources)
{}

std::string CollectVariablesTraverser::getMappedName(const TSymbol *symbol) const
{
    return HashName(symbol, mHashFunction, nullptr).data();
}

void CollectVariablesTraverser::setBuiltInInfoFromSymbol(const TVariable &variable,
                                                         ShaderVariable *info)
{
    const TType &type = variable.getType();

    info->name       = variable.name().data();
    info->mappedName = variable.name().data();

    bool isShaderIOBlock =
        IsShaderIoBlock(type.getQualifier()) && type.getInterfaceBlock() != nullptr;
    bool isPatch = type.getQualifier() == EvqTessLevelInner ||
                   type.getQualifier() == EvqTessLevelOuter ||
                   type.getQualifier() == EvqBoundingBox;

    setFieldOrVariableProperties(type, true, isShaderIOBlock, isPatch, info);
}

void CollectVariablesTraverser::recordBuiltInVaryingUsed(const TVariable &variable,
                                                         bool *addedFlag,
                                                         std::vector<ShaderVariable> *varyings)
{
    ASSERT(varyings);
    if (!(*addedFlag))
    {
        ShaderVariable info;
        setBuiltInInfoFromSymbol(variable, &info);
        info.active      = true;
        info.isInvariant = mSymbolTable->isVaryingInvariant(variable);

        varyings->push_back(info);
        (*addedFlag) = true;
    }
}

void CollectVariablesTraverser::recordBuiltInFragmentOutputUsed(const TVariable &variable,
                                                                bool *addedFlag)
{
    if (!(*addedFlag))
    {
        ShaderVariable info;
        setBuiltInInfoFromSymbol(variable, &info);
        info.active = true;
        mOutputVariables->push_back(info);
        (*addedFlag) = true;
    }
}

void CollectVariablesTraverser::recordBuiltInAttributeUsed(const TVariable &variable,
                                                           bool *addedFlag)
{
    if (!(*addedFlag))
    {
        ShaderVariable info;
        setBuiltInInfoFromSymbol(variable, &info);
        info.active   = true;
        info.location = -1;
        mAttribs->push_back(info);
        (*addedFlag) = true;
    }
}

bool CollectVariablesTraverser::visitGlobalQualifierDeclaration(
    Visit visit,
    TIntermGlobalQualifierDeclaration *node)
{
    // We should not mark variables as active just based on an invariant/precise declaration, so we
    // don't traverse the symbols declared invariant.
    return false;
}

// We want to check whether a uniform/varying is active because we need to skip updating inactive
// ones. We also only count the active ones in packing computing. Also, gl_FragCoord, gl_PointCoord,
// and gl_FrontFacing count toward varying counting if they are active in a fragment shader.
void CollectVariablesTraverser::visitSymbol(TIntermSymbol *symbol)
{
    ASSERT(symbol != nullptr);

    if (symbol->variable().symbolType() == SymbolType::AngleInternal ||
        symbol->variable().symbolType() == SymbolType::Empty)
    {
        // Internal variables or nameless variables are not collected.
        return;
    }

    ShaderVariable *var = nullptr;

    const ImmutableString &symbolName = symbol->getName();

    // Check the qualifier from the variable, not from the symbol node. The node may have a
    // different qualifier if it's the result of a folded ternary node.
    TQualifier qualifier                  = symbol->variable().getType().getQualifier();
    const TInterfaceBlock *interfaceBlock = symbol->getType().getInterfaceBlock();

    if (IsVaryingIn(qualifier))
    {
        if (interfaceBlock)
        {
            var = FindShaderIOBlockVariable(interfaceBlock->name(), mInputVaryings);
        }
        else
        {
            var = FindVariable(symbolName, mInputVaryings);
        }
    }
    else if (IsVaryingOut(qualifier))
    {
        if (interfaceBlock)
        {
            var = FindShaderIOBlockVariable(interfaceBlock->name(), mOutputVaryings);
        }
        else
        {
            var = FindVariable(symbolName, mOutputVaryings);
        }
    }
    else if (symbol->getType().getBasicType() == EbtInterfaceBlock)
    {
        UNREACHABLE();
    }
    else if (symbolName == "gl_DepthRange")
    {
        ASSERT(qualifier == EvqUniform);

        if (!mDepthRangeAdded)
        {
            ShaderVariable info;
            const char kName[] = "gl_DepthRange";
            info.name          = kName;
            info.mappedName    = kName;
            info.type          = GL_NONE;
            info.precision     = GL_NONE;
            info.staticUse     = true;
            info.active        = true;

            ShaderVariable nearInfo(GL_FLOAT);
            const char kNearName[] = "near";
            nearInfo.name          = kNearName;
            nearInfo.mappedName    = kNearName;
            nearInfo.precision     = GL_HIGH_FLOAT;
            nearInfo.staticUse     = true;
            nearInfo.active        = true;

            ShaderVariable farInfo(GL_FLOAT);
            const char kFarName[] = "far";
            farInfo.name          = kFarName;
            farInfo.mappedName    = kFarName;
            farInfo.precision     = GL_HIGH_FLOAT;
            farInfo.staticUse     = true;
            farInfo.active        = true;

            ShaderVariable diffInfo(GL_FLOAT);
            const char kDiffName[] = "diff";
            diffInfo.name          = kDiffName;
            diffInfo.mappedName    = kDiffName;
            diffInfo.precision     = GL_HIGH_FLOAT;
            diffInfo.staticUse     = true;
            diffInfo.active        = true;

            info.fields.push_back(nearInfo);
            info.fields.push_back(farInfo);
            info.fields.push_back(diffInfo);

            mUniforms->push_back(info);
            mDepthRangeAdded = true;
        }
    }
    else if (symbolName == "gl_NumSamples")
    {
        ASSERT(qualifier == EvqUniform);

        if (!mNumSamplesAdded)
        {
            ShaderVariable info;
            const char kName[] = "gl_NumSamples";
            info.name          = kName;
            info.mappedName    = kName;
            info.type          = GL_INT;
            info.precision     = GL_LOW_INT;
            info.staticUse     = true;
            info.active        = true;

            mUniforms->push_back(info);
            mNumSamplesAdded = true;
        }
    }
    else
    {
        switch (qualifier)
        {
            case EvqAttribute:
            case EvqVertexIn:
                var = FindVariable(symbolName, mAttribs);
                break;
            case EvqFragmentOut:
            case EvqFragmentInOut:
                var                  = FindVariable(symbolName, mOutputVariables);
                var->isFragmentInOut = qualifier == EvqFragmentInOut;
                break;
            case EvqUniform:
            {
                if (interfaceBlock)
                {
                    var = FindVariableInInterfaceBlock(symbolName, interfaceBlock, mUniformBlocks);
                }
                else
                {
                    var = FindVariable(symbolName, mUniforms);
                }

                // It's an internal error to reference an undefined user uniform
                ASSERT(!gl::IsBuiltInName(symbolName.data()) || var);
            }
            break;
            case EvqBuffer:
            {
                var =
                    FindVariableInInterfaceBlock(symbolName, interfaceBlock, mShaderStorageBlocks);
            }
            break;
            case EvqFragCoord:
                recordBuiltInVaryingUsed(symbol->variable(), &mFragCoordAdded, mInputVaryings);
                return;
            case EvqFrontFacing:
                recordBuiltInVaryingUsed(symbol->variable(), &mFrontFacingAdded, mInputVaryings);
                return;
            case EvqHelperInvocation:
                recordBuiltInVaryingUsed(symbol->variable(), &mHelperInvocationAdded,
                                         mInputVaryings);
                return;
            case EvqPointCoord:
                recordBuiltInVaryingUsed(symbol->variable(), &mPointCoordAdded, mInputVaryings);
                return;
            case EvqNumWorkGroups:
                recordBuiltInAttributeUsed(symbol->variable(), &mNumWorkGroupsAdded);
                return;
            case EvqWorkGroupID:
                recordBuiltInAttributeUsed(symbol->variable(), &mWorkGroupIDAdded);
                return;
            case EvqLocalInvocationID:
                recordBuiltInAttributeUsed(symbol->variable(), &mLocalInvocationIDAdded);
                return;
            case EvqGlobalInvocationID:
                recordBuiltInAttributeUsed(symbol->variable(), &mGlobalInvocationIDAdded);
                return;
            case EvqLocalInvocationIndex:
                recordBuiltInAttributeUsed(symbol->variable(), &mLocalInvocationIndexAdded);
                return;
            case EvqInstanceID:
                // Whenever the initializeBuiltinsForInstancedMultiview option is set,
                // gl_InstanceID is added inside expressions to initialize ViewID_OVR and
                // InstanceID. Note that gl_InstanceID is not added to the symbol table for ESSL1
                // shaders.
                recordBuiltInAttributeUsed(symbol->variable(), &mInstanceIDAdded);
                return;
            case EvqVertexID:
                recordBuiltInAttributeUsed(symbol->variable(), &mVertexIDAdded);
                return;
            case EvqPosition:
                recordBuiltInVaryingUsed(symbol->variable(), &mPositionAdded, mOutputVaryings);
                return;
            case EvqPointSize:
                recordBuiltInVaryingUsed(symbol->variable(), &mPointSizeAdded, mOutputVaryings);
                return;
            case EvqDrawID:
                recordBuiltInAttributeUsed(symbol->variable(), &mDrawIDAdded);
                return;
            case EvqLastFragData:
                recordBuiltInVaryingUsed(symbol->variable(), &mLastFragDataAdded, mInputVaryings);
                return;
            case EvqLastFragColor:
                recordBuiltInVaryingUsed(symbol->variable(), &mLastFragColorAdded, mInputVaryings);
                return;
            case EvqFragColor:
                recordBuiltInFragmentOutputUsed(symbol->variable(), &mFragColorAdded);
                return;
            case EvqFragData:
                recordBuiltInFragmentOutputUsed(symbol->variable(), &mFragDataAdded);
                return;
            case EvqFragDepth:
                recordBuiltInFragmentOutputUsed(symbol->variable(), &mFragDepthAdded);
                return;
            case EvqSecondaryFragColorEXT:
                recordBuiltInFragmentOutputUsed(symbol->variable(), &mSecondaryFragColorEXTAdded);
                return;
            case EvqSecondaryFragDataEXT:
                recordBuiltInFragmentOutputUsed(symbol->variable(), &mSecondaryFragDataEXTAdded);
                return;
            case EvqInvocationID:
                recordBuiltInVaryingUsed(symbol->variable(), &mInvocationIDAdded, mInputVaryings);
                break;
            case EvqPrimitiveIDIn:
                recordBuiltInVaryingUsed(symbol->variable(), &mPrimitiveIDInAdded, mInputVaryings);
                break;
            case EvqPrimitiveID:
                if (mShaderType == GL_GEOMETRY_SHADER_EXT)
                {
                    recordBuiltInVaryingUsed(symbol->variable(), &mPrimitiveIDAdded,
                                             mOutputVaryings);
                }
                else
                {
                    ASSERT(mShaderType == GL_FRAGMENT_SHADER ||
                           mShaderType == GL_TESS_CONTROL_SHADER ||
                           mShaderType == GL_TESS_EVALUATION_SHADER);
                    recordBuiltInVaryingUsed(symbol->variable(), &mPrimitiveIDAdded,
                                             mInputVaryings);
                }
                break;
            case EvqLayerOut:
                if (mShaderType == GL_GEOMETRY_SHADER_EXT)
                {
                    recordBuiltInVaryingUsed(symbol->variable(), &mLayerAdded, mOutputVaryings);
                }
                else
                {
                    ASSERT(mShaderType == GL_VERTEX_SHADER &&
                           (IsExtensionEnabled(mExtensionBehavior, TExtension::OVR_multiview2) ||
                            IsExtensionEnabled(mExtensionBehavior, TExtension::OVR_multiview)));
                }
                break;
            case EvqLayerIn:
                ASSERT(mShaderType == GL_FRAGMENT_SHADER);
                recordBuiltInVaryingUsed(symbol->variable(), &mLayerAdded, mInputVaryings);
                break;
            case EvqShared:
                if (mShaderType == GL_COMPUTE_SHADER)
                {
                    recordBuiltInVaryingUsed(symbol->variable(), &mSharedVariableAdded,
                                             mSharedVariables);
                }
                break;
            case EvqClipDistance:
                recordBuiltInVaryingUsed(
                    symbol->variable(), &mClipDistanceAdded,
                    mShaderType == GL_FRAGMENT_SHADER ? mInputVaryings : mOutputVaryings);
                return;
            case EvqCullDistance:
                recordBuiltInVaryingUsed(
                    symbol->variable(), &mCullDistanceAdded,
                    mShaderType == GL_FRAGMENT_SHADER ? mInputVaryings : mOutputVaryings);
                return;
            case EvqSampleID:
                recordBuiltInVaryingUsed(symbol->variable(), &mSampleIDAdded, mInputVaryings);
                return;
            case EvqSamplePosition:
                recordBuiltInVaryingUsed(symbol->variable(), &mSamplePositionAdded, mInputVaryings);
                return;
            case EvqSampleMaskIn:
                recordBuiltInVaryingUsed(symbol->variable(), &mSampleMaskInAdded, mInputVaryings);
                return;
            case EvqSampleMask:
                recordBuiltInFragmentOutputUsed(symbol->variable(), &mSampleMaskAdded);
                return;
            case EvqPatchVerticesIn:
                recordBuiltInVaryingUsed(symbol->variable(), &mPatchVerticesInAdded,
                                         mInputVaryings);
                break;
            case EvqTessCoord:
                recordBuiltInVaryingUsed(symbol->variable(), &mTessCoordAdded, mInputVaryings);
                break;
            case EvqTessLevelOuter:
                if (mShaderType == GL_TESS_CONTROL_SHADER)
                {
                    recordBuiltInVaryingUsed(symbol->variable(), &mTessLevelOuterAdded,
                                             mOutputVaryings);
                }
                else
                {
                    ASSERT(mShaderType == GL_TESS_EVALUATION_SHADER);
                    recordBuiltInVaryingUsed(symbol->variable(), &mTessLevelOuterAdded,
                                             mInputVaryings);
                }
                break;
            case EvqTessLevelInner:
                if (mShaderType == GL_TESS_CONTROL_SHADER)
                {
                    recordBuiltInVaryingUsed(symbol->variable(), &mTessLevelInnerAdded,
                                             mOutputVaryings);
                }
                else
                {
                    ASSERT(mShaderType == GL_TESS_EVALUATION_SHADER);
                    recordBuiltInVaryingUsed(symbol->variable(), &mTessLevelInnerAdded,
                                             mInputVaryings);
                }
                break;
            case EvqBoundingBox:
                recordBuiltInVaryingUsed(symbol->variable(), &mBoundingBoxAdded, mOutputVaryings);
                break;
            default:
                break;
        }
    }
    if (var)
    {
        MarkActive(var);
    }
}

void CollectVariablesTraverser::setFieldOrVariableProperties(const TType &type,
                                                             bool staticUse,
                                                             bool isShaderIOBlock,
                                                             bool isPatch,
                                                             ShaderVariable *variableOut) const
{
    ASSERT(variableOut);

    variableOut->staticUse       = staticUse;
    variableOut->isShaderIOBlock = isShaderIOBlock;
    variableOut->isPatch         = isPatch;

    const TStructure *structure           = type.getStruct();
    const TInterfaceBlock *interfaceBlock = type.getInterfaceBlock();
    if (structure)
    {
        // Structures use a NONE type that isn't exposed outside ANGLE.
        variableOut->type = GL_NONE;
        // Anonymous structs are given name from AngleInternal namespace.
        if (structure->symbolType() != SymbolType::Empty &&
            structure->symbolType() != SymbolType::AngleInternal)
        {
            variableOut->structOrBlockName = structure->name().data();
        }

        const TFieldList &fields = structure->fields();

        for (const TField *field : fields)
        {
            // Regardless of the variable type (uniform, in/out etc.) its fields are always plain
            // ShaderVariable objects.
            ShaderVariable fieldVariable;
            setFieldProperties(*field->type(), field->name(), staticUse, isShaderIOBlock, isPatch,
                               field->symbolType(), &fieldVariable);
            variableOut->fields.push_back(fieldVariable);
        }
    }
    else if (interfaceBlock && isShaderIOBlock)
    {
        const bool isPerVertex = (interfaceBlock->name() == "gl_PerVertex");
        variableOut->type      = GL_NONE;
        if (interfaceBlock->symbolType() != SymbolType::Empty)
        {
            variableOut->structOrBlockName = interfaceBlock->name().data();
            variableOut->mappedStructOrBlockName =
                isPerVertex ? interfaceBlock->name().data()
                            : HashName(interfaceBlock->name(), mHashFunction, nullptr).data();
        }
        const TFieldList &fields = interfaceBlock->fields();
        for (const TField *field : fields)
        {
            ShaderVariable fieldVariable;

            setFieldProperties(*field->type(), field->name(), staticUse, true, isPatch,
                               field->symbolType(), &fieldVariable);
            fieldVariable.isShaderIOBlock = true;
            variableOut->fields.push_back(fieldVariable);
        }
    }
    else
    {
        variableOut->type      = GLVariableType(type);
        variableOut->precision = GLVariablePrecision(type);
    }

    const angle::Span<const unsigned int> &arraySizes = type.getArraySizes();
    if (!arraySizes.empty())
    {
        variableOut->arraySizes.assign(arraySizes.begin(), arraySizes.end());

        if (arraySizes[0] == 0)
        {
            // Tessellation Control & Evaluation shader inputs:
            // Declaring an array size is optional. If no size is specified, it will be taken from
            // the implementation-dependent maximum patch size (gl_MaxPatchVertices).
            if (type.getQualifier() == EvqTessControlIn ||
                type.getQualifier() == EvqTessEvaluationIn)
            {
                variableOut->arraySizes[0] = mResources.MaxPatchVertices;
            }

            // Tessellation Control shader outputs:
            // Declaring an array size is optional. If no size is specified, it will be taken from
            // output patch size declared in the shader.
            if (type.getQualifier() == EvqTessControlOut)
            {
                ASSERT(mTessControlShaderOutputVertices > 0);
                variableOut->arraySizes[0] = mTessControlShaderOutputVertices;
            }
        }
    }
}

void CollectVariablesTraverser::setFieldProperties(const TType &type,
                                                   const ImmutableString &name,
                                                   bool staticUse,
                                                   bool isShaderIOBlock,
                                                   bool isPatch,
                                                   SymbolType symbolType,
                                                   ShaderVariable *variableOut) const
{
    ASSERT(variableOut);
    setFieldOrVariableProperties(type, staticUse, isShaderIOBlock, isPatch, variableOut);
    variableOut->name.assign(name.data(), name.length());
    variableOut->mappedName = (symbolType == SymbolType::BuiltIn)
                                  ? name.data()
                                  : HashName(name, mHashFunction, nullptr).data();
}

void CollectVariablesTraverser::setCommonVariableProperties(const TType &type,
                                                            const TVariable &variable,
                                                            ShaderVariable *variableOut) const
{
    ASSERT(variableOut);
    ASSERT(type.getInterfaceBlock() == nullptr || IsShaderIoBlock(type.getQualifier()) ||
           type.getQualifier() == EvqPatchIn || type.getQualifier() == EvqPatchOut);

    const bool staticUse       = mSymbolTable->isStaticallyUsed(variable);
    const bool isShaderIOBlock = type.getInterfaceBlock() != nullptr;
    const bool isPatch = type.getQualifier() == EvqPatchIn || type.getQualifier() == EvqPatchOut;

    setFieldOrVariableProperties(type, staticUse, isShaderIOBlock, isPatch, variableOut);

    const bool isNamed = variable.symbolType() != SymbolType::Empty;

    ASSERT(isNamed || isShaderIOBlock);
    if (isNamed)
    {
        variableOut->name.assign(variable.name().data(), variable.name().length());
        variableOut->mappedName = getMappedName(&variable);
    }

    // For I/O blocks, additionally store the name of the block as blockName.  If the variable is
    // unnamed, this name will be used instead for the purpose of interface matching.
    if (isShaderIOBlock)
    {
        const TInterfaceBlock *interfaceBlock = type.getInterfaceBlock();
        ASSERT(interfaceBlock);

        variableOut->structOrBlockName.assign(interfaceBlock->name().data(),
                                              interfaceBlock->name().length());
        variableOut->mappedStructOrBlockName =
            HashName(interfaceBlock->name(), mHashFunction, nullptr).data();
        variableOut->isShaderIOBlock = true;
    }
}

ShaderVariable CollectVariablesTraverser::recordAttribute(const TIntermSymbol &variable) const
{
    const TType &type = variable.getType();
    ASSERT(!type.getStruct());

    ShaderVariable attribute;
    setCommonVariableProperties(type, variable.variable(), &attribute);

    attribute.location = type.getLayoutQualifier().location;
    return attribute;
}

ShaderVariable CollectVariablesTraverser::recordOutputVariable(const TIntermSymbol &variable) const
{
    const TType &type = variable.getType();
    ASSERT(!type.getStruct());

    ShaderVariable outputVariable;
    setCommonVariableProperties(type, variable.variable(), &outputVariable);

    outputVariable.location = type.getLayoutQualifier().location;
    outputVariable.index    = type.getLayoutQualifier().index;
    outputVariable.yuv      = type.getLayoutQualifier().yuv;
    return outputVariable;
}

ShaderVariable CollectVariablesTraverser::recordVarying(const TIntermSymbol &variable) const
{
    const TType &type = variable.getType();

    ShaderVariable varying;
    setCommonVariableProperties(type, variable.variable(), &varying);
    varying.location = type.getLayoutQualifier().location;

    switch (type.getQualifier())
    {
        case EvqVaryingIn:
        case EvqVaryingOut:
        case EvqVertexOut:
        case EvqSmoothOut:
        case EvqFlatOut:
        case EvqNoPerspectiveOut:
        case EvqCentroidOut:
        case EvqSampleOut:
        case EvqNoPerspectiveCentroidOut:
        case EvqNoPerspectiveSampleOut:
        case EvqGeometryOut:
            if (mSymbolTable->isVaryingInvariant(variable.variable()) || type.isInvariant())
            {
                varying.isInvariant = true;
            }
            break;
        case EvqPatchIn:
        case EvqPatchOut:
            varying.isPatch = true;
            break;
        default:
            break;
    }

    varying.interpolation = GetInterpolationType(type.getQualifier());

    // Shader I/O block properties
    if (type.getBasicType() == EbtInterfaceBlock)
    {
        bool isBlockImplicitLocation = false;
        int location                 = type.getLayoutQualifier().location;

        // when a interface has not location in layout, assign to the zero.
        if (location < 0)
        {
            location                = 0;
            isBlockImplicitLocation = true;
        }

        const TInterfaceBlock *blockType = type.getInterfaceBlock();
        ASSERT(blockType->fields().size() == varying.fields.size());

        for (size_t fieldIndex = 0; fieldIndex < varying.fields.size(); ++fieldIndex)
        {
            const TField *blockField      = blockType->fields()[fieldIndex];
            ShaderVariable &fieldVariable = varying.fields[fieldIndex];
            const TType &fieldType        = *blockField->type();

            fieldVariable.hasImplicitLocation = isBlockImplicitLocation;
            fieldVariable.isPatch             = varying.isPatch;

            int fieldLocation = fieldType.getLayoutQualifier().location;
            if (fieldLocation >= 0)
            {
                fieldVariable.hasImplicitLocation = false;
                fieldVariable.location            = fieldLocation;
                location                          = fieldLocation;
            }
            else
            {
                fieldVariable.location = location;
                location += fieldType.getLocationCount();
            }

            if (fieldType.getQualifier() != EvqGlobal)
            {
                fieldVariable.interpolation = GetFieldInterpolationType(fieldType.getQualifier());
            }
        }
    }

    return varying;
}

void CollectVariablesTraverser::recordInterfaceBlock(const char *instanceName,
                                                     const TType &interfaceBlockType,
                                                     InterfaceBlock *interfaceBlock) const
{
    ASSERT(interfaceBlockType.getBasicType() == EbtInterfaceBlock);
    ASSERT(interfaceBlock);

    const TInterfaceBlock *blockType = interfaceBlockType.getInterfaceBlock();
    ASSERT(blockType);

    interfaceBlock->name       = blockType->name().data();
    interfaceBlock->mappedName = getMappedName(blockType);

    const bool isGLInBuiltin = (instanceName != nullptr) && strncmp(instanceName, "gl_in", 5u) == 0;
    if (instanceName != nullptr)
    {
        interfaceBlock->instanceName = instanceName;
        const TSymbol *blockSymbol   = nullptr;
        if (isGLInBuiltin)
        {
            blockSymbol = mSymbolTable->getGlInVariableWithArraySize();
        }
        else
        {
            blockSymbol = mSymbolTable->findGlobal(ImmutableString(instanceName));
        }
        ASSERT(blockSymbol && blockSymbol->isVariable());
        interfaceBlock->staticUse =
            mSymbolTable->isStaticallyUsed(*static_cast<const TVariable *>(blockSymbol));
    }

    ASSERT(!interfaceBlockType.isArrayOfArrays());  // Disallowed by GLSL ES 3.10 section 4.3.9
    interfaceBlock->arraySize =
        interfaceBlockType.isArray() ? interfaceBlockType.getOutermostArraySize() : 0;

    interfaceBlock->blockType = GetBlockType(interfaceBlockType.getQualifier());
    if (interfaceBlock->blockType == BlockType::kBlockUniform ||
        interfaceBlock->blockType == BlockType::kBlockBuffer)
    {
        // TODO(oetuaho): Remove setting isRowMajorLayout.
        interfaceBlock->isRowMajorLayout = false;
        interfaceBlock->binding          = blockType->blockBinding();
        interfaceBlock->layout           = GetBlockLayoutType(blockType->blockStorage());
    }

    // Consider an SSBO readonly if all its fields are readonly.  Note that ANGLE doesn't keep the
    // readonly qualifier applied to the interface block itself, but rather applies it to the
    // fields.
    ASSERT(!interfaceBlockType.getMemoryQualifier().readonly);
    bool isReadOnly = true;

    // Gather field information
    bool anyFieldStaticallyUsed = false;

    for (const TField *field : blockType->fields())
    {
        const TType &fieldType = *field->type();

        bool staticUse = false;
        if (instanceName == nullptr)
        {
            // Static use of individual fields has been recorded, since they are present in the
            // symbol table as variables.
            const TSymbol *fieldSymbol = mSymbolTable->findGlobal(field->name());
            ASSERT(fieldSymbol && fieldSymbol->isVariable());
            staticUse =
                mSymbolTable->isStaticallyUsed(*static_cast<const TVariable *>(fieldSymbol));
            if (staticUse)
            {
                anyFieldStaticallyUsed = true;
            }
        }

        ShaderVariable fieldVariable;
        setFieldProperties(fieldType, field->name(), staticUse, false, false, field->symbolType(),
                           &fieldVariable);
        fieldVariable.isRowMajorLayout =
            (fieldType.getLayoutQualifier().matrixPacking == EmpRowMajor);
        interfaceBlock->fields.push_back(fieldVariable);

        // The SSBO is not readonly if any field is not readonly.
        if (!fieldType.getMemoryQualifier().readonly)
        {
            isReadOnly = false;
        }
    }
    if (anyFieldStaticallyUsed)
    {
        interfaceBlock->staticUse = true;
    }
    if (interfaceBlock->blockType == BlockType::kBlockBuffer)
    {
        interfaceBlock->isReadOnly = isReadOnly;
    }
}

ShaderVariable CollectVariablesTraverser::recordUniform(const TIntermSymbol &variable) const
{
    ShaderVariable uniform;
    setCommonVariableProperties(variable.getType(), variable.variable(), &uniform);
    uniform.binding = variable.getType().getLayoutQualifier().binding;
    uniform.imageUnitFormat =
        GetImageInternalFormatType(variable.getType().getLayoutQualifier().imageInternalFormat);
    uniform.location      = variable.getType().getLayoutQualifier().location;
    uniform.offset        = variable.getType().getLayoutQualifier().offset;
    uniform.rasterOrdered = variable.getType().getLayoutQualifier().rasterOrdered;
    uniform.readonly      = variable.getType().getMemoryQualifier().readonly;
    uniform.writeonly     = variable.getType().getMemoryQualifier().writeonly;
    return uniform;
}

bool CollectVariablesTraverser::visitDeclaration(Visit, TIntermDeclaration *node)
{
    const TIntermSequence &sequence = *(node->getSequence());
    ASSERT(!sequence.empty());

    const TIntermTyped &typedNode = *(sequence.front()->getAsTyped());
    TQualifier qualifier          = typedNode.getQualifier();

    bool isShaderVariable = qualifier == EvqAttribute || qualifier == EvqVertexIn ||
                            qualifier == EvqFragmentOut || qualifier == EvqFragmentInOut ||
                            qualifier == EvqUniform || IsVarying(qualifier);

    if (typedNode.getBasicType() != EbtInterfaceBlock && !isShaderVariable)
    {
        return true;
    }

    for (TIntermNode *variableNode : sequence)
    {
        // The only case in which the sequence will not contain a TIntermSymbol node is
        // initialization. It will contain a TInterBinary node in that case. Since attributes,
        // uniforms, varyings, outputs and interface blocks cannot be initialized in a shader, we
        // must have only TIntermSymbol nodes in the sequence in the cases we are interested in.
        const TIntermSymbol &variable = *variableNode->getAsSymbolNode();
        if (variable.variable().symbolType() == SymbolType::AngleInternal)
        {
            // Internal variables are not collected.
            continue;
        }

        // SpirvTransformer::transform uses a map of ShaderVariables, it needs member variables and
        // (named or unnamed) structure as ShaderVariable. at link between two shaders, validation
        // between of named and unnamed, needs the same structure, its members, and members order
        // except instance name.
        if (typedNode.getBasicType() == EbtInterfaceBlock && !IsShaderIoBlock(qualifier) &&
            qualifier != EvqPatchIn && qualifier != EvqPatchOut)
        {
            InterfaceBlock interfaceBlock;
            bool isUnnamed    = variable.variable().symbolType() == SymbolType::Empty;
            const TType &type = variable.getType();
            recordInterfaceBlock(isUnnamed ? nullptr : variable.getName().data(), type,
                                 &interfaceBlock);

            // all fields in interface block will be added for updating interface variables because
            // the temporal structure variable will be ignored.
            switch (qualifier)
            {
                case EvqUniform:
                    mUniformBlocks->push_back(interfaceBlock);
                    break;
                case EvqBuffer:
                    mShaderStorageBlocks->push_back(interfaceBlock);
                    break;
                case EvqPixelLocalEXT:
                    // EXT_shader_pixel_local_storage is completely self-contained within the
                    // shader, so we don't need to gather any info on it.
                    break;
                default:
                    UNREACHABLE();
            }
        }
        else
        {
            ASSERT(variable.variable().symbolType() != SymbolType::Empty ||
                   IsShaderIoBlock(qualifier) || qualifier == EvqPatchIn ||
                   qualifier == EvqPatchOut);
            switch (qualifier)
            {
                case EvqAttribute:
                case EvqVertexIn:
                    mAttribs->push_back(recordAttribute(variable));
                    break;
                case EvqFragmentOut:
                case EvqFragmentInOut:
                    mOutputVariables->push_back(recordOutputVariable(variable));
                    break;
                case EvqUniform:
                    mUniforms->push_back(recordUniform(variable));
                    break;
                default:
                    if (IsVaryingIn(qualifier))
                    {
                        mInputVaryings->push_back(recordVarying(variable));
                    }
                    else
                    {
                        ASSERT(IsVaryingOut(qualifier));
                        mOutputVaryings->push_back(recordVarying(variable));
                    }
                    break;
            }
        }
    }

    // None of the recorded variables can have initializers, so we don't need to traverse the
    // declarators.
    return false;
}

InterfaceBlock *CollectVariablesTraverser::findNamedInterfaceBlock(
    const ImmutableString &blockName) const
{
    InterfaceBlock *namedBlock = FindVariable(blockName, mUniformBlocks);
    if (!namedBlock)
    {
        namedBlock = FindVariable(blockName, mShaderStorageBlocks);
    }
    return namedBlock;
}

bool CollectVariablesTraverser::visitBinary(Visit, TIntermBinary *binaryNode)
{
    if (binaryNode->getOp() == EOpIndexDirectInterfaceBlock)
    {
        // NOTE: we do not determine static use / activeness for individual blocks of an array.
        TIntermTyped *blockNode = binaryNode->getLeft()->getAsTyped();
        ASSERT(blockNode);

        TIntermConstantUnion *constantUnion = binaryNode->getRight()->getAsConstantUnion();
        ASSERT(constantUnion);

        InterfaceBlock *namedBlock = nullptr;

        bool traverseIndexExpression         = false;
        TIntermBinary *interfaceIndexingNode = blockNode->getAsBinaryNode();
        if (interfaceIndexingNode)
        {
            ASSERT(interfaceIndexingNode->getOp() == EOpIndexDirect ||
                   interfaceIndexingNode->getOp() == EOpIndexIndirect);
            traverseIndexExpression = true;
            blockNode               = interfaceIndexingNode->getLeft();
        }

        const TType &interfaceNodeType        = blockNode->getType();
        const TInterfaceBlock *interfaceBlock = interfaceNodeType.getInterfaceBlock();
        const TQualifier qualifier            = interfaceNodeType.getQualifier();

        // If it's a shader I/O block, look in varyings
        ShaderVariable *ioBlockVar = nullptr;
        if (qualifier == EvqPerVertexIn)
        {
            TIntermSymbol *symbolNode = blockNode->getAsSymbolNode();
            ASSERT(symbolNode);
            recordBuiltInVaryingUsed(symbolNode->variable(), &mPerVertexInAdded, mInputVaryings);
            ioBlockVar = FindShaderIOBlockVariable(interfaceBlock->name(), mInputVaryings);
        }
        else if (IsVaryingIn(qualifier))
        {
            ioBlockVar = FindShaderIOBlockVariable(interfaceBlock->name(), mInputVaryings);
        }
        else if (qualifier == EvqPerVertexOut)
        {
            TIntermSymbol *symbolNode = blockNode->getAsSymbolNode();
            ASSERT(symbolNode);
            recordBuiltInVaryingUsed(symbolNode->variable(), &mPerVertexOutAdded, mOutputVaryings);
            ioBlockVar = FindShaderIOBlockVariable(interfaceBlock->name(), mOutputVaryings);
        }
        else if (IsVaryingOut(qualifier))
        {
            ioBlockVar = FindShaderIOBlockVariable(interfaceBlock->name(), mOutputVaryings);
        }

        if (ioBlockVar)
        {
            MarkActive(ioBlockVar);
        }
        else if (qualifier != EvqPixelLocalEXT)
        {

            if (!namedBlock)
            {
                namedBlock = findNamedInterfaceBlock(interfaceBlock->name());
            }
            ASSERT(namedBlock);
            ASSERT(namedBlock->staticUse);
            namedBlock->active      = true;
            unsigned int fieldIndex = static_cast<unsigned int>(constantUnion->getIConst(0));
            ASSERT(fieldIndex < namedBlock->fields.size());
            // TODO(oetuaho): Would be nicer to record static use of fields of named interface
            // blocks more accurately at parse time - now we only mark the fields statically used if
            // they are active. http://anglebug.com/42261150 We need to mark this field and all of
            // its sub-fields, as static/active
            MarkActive(&namedBlock->fields[fieldIndex]);
        }

        if (traverseIndexExpression)
        {
            ASSERT(interfaceIndexingNode);
            interfaceIndexingNode->getRight()->traverse(this);
        }
        return false;
    }

    return true;
}

}  // anonymous namespace

void CollectVariables(TIntermBlock *root,
                      std::vector<ShaderVariable> *attributes,
                      std::vector<ShaderVariable> *outputVariables,
                      std::vector<ShaderVariable> *uniforms,
                      std::vector<ShaderVariable> *inputVaryings,
                      std::vector<ShaderVariable> *outputVaryings,
                      std::vector<ShaderVariable> *sharedVariables,
                      std::vector<InterfaceBlock> *uniformBlocks,
                      std::vector<InterfaceBlock> *shaderStorageBlocks,
                      ShHashFunction64 hashFunction,
                      TSymbolTable *symbolTable,
                      GLenum shaderType,
                      const TExtensionBehavior &extensionBehavior,
                      const ShBuiltInResources &resources,
                      int tessControlShaderOutputVertices)
{
    CollectVariablesTraverser collect(
        attributes, outputVariables, uniforms, inputVaryings, outputVaryings, sharedVariables,
        uniformBlocks, shaderStorageBlocks, hashFunction, symbolTable, shaderType,
        extensionBehavior, resources, tessControlShaderOutputVertices);
    root->traverse(&collect);
}

}  // namespace sh
