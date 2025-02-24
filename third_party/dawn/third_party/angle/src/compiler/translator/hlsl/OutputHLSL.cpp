//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/hlsl/OutputHLSL.h"

#include <stdio.h>
#include <algorithm>
#include <cfloat>

#include "common/angleutils.h"
#include "common/debug.h"
#include "common/utilities.h"
#include "compiler/translator/BuiltInFunctionEmulator.h"
#include "compiler/translator/InfoSink.h"
#include "compiler/translator/StaticType.h"
#include "compiler/translator/blocklayout.h"
#include "compiler/translator/hlsl/AtomicCounterFunctionHLSL.h"
#include "compiler/translator/hlsl/BuiltInFunctionEmulatorHLSL.h"
#include "compiler/translator/hlsl/ImageFunctionHLSL.h"
#include "compiler/translator/hlsl/ResourcesHLSL.h"
#include "compiler/translator/hlsl/StructureHLSL.h"
#include "compiler/translator/hlsl/TextureFunctionHLSL.h"
#include "compiler/translator/hlsl/TranslatorHLSL.h"
#include "compiler/translator/hlsl/UtilsHLSL.h"
#include "compiler/translator/tree_ops/hlsl/RemoveSwitchFallThrough.h"
#include "compiler/translator/tree_util/FindSymbolNode.h"
#include "compiler/translator/tree_util/NodeSearch.h"
#include "compiler/translator/util.h"

namespace sh
{

namespace
{

constexpr const char kImage2DFunctionString[] = "// @@ IMAGE2D DECLARATION FUNCTION STRING @@";

TString ArrayHelperFunctionName(const char *prefix, const TType &type)
{
    TStringStream fnName = sh::InitializeStream<TStringStream>();
    fnName << prefix << "_";
    if (type.isArray())
    {
        for (unsigned int arraySize : type.getArraySizes())
        {
            fnName << arraySize << "_";
        }
    }
    fnName << TypeString(type);
    return fnName.str();
}

bool IsDeclarationWrittenOut(TIntermDeclaration *node)
{
    TIntermSequence *sequence = node->getSequence();
    TIntermTyped *variable    = (*sequence)[0]->getAsTyped();
    ASSERT(sequence->size() == 1);
    ASSERT(variable);
    return (variable->getQualifier() == EvqTemporary || variable->getQualifier() == EvqGlobal ||
            variable->getQualifier() == EvqConst || variable->getQualifier() == EvqShared);
}

bool IsInStd140UniformBlock(TIntermTyped *node)
{
    TIntermBinary *binaryNode = node->getAsBinaryNode();

    if (binaryNode)
    {
        return IsInStd140UniformBlock(binaryNode->getLeft());
    }

    const TType &type = node->getType();

    if (type.getQualifier() == EvqUniform)
    {
        // determine if we are in the standard layout
        const TInterfaceBlock *interfaceBlock = type.getInterfaceBlock();
        if (interfaceBlock)
        {
            return (interfaceBlock->blockStorage() == EbsStd140);
        }
    }

    return false;
}

const TInterfaceBlock *GetInterfaceBlockOfUniformBlockNearestIndexOperator(TIntermTyped *node)
{
    const TIntermBinary *binaryNode = node->getAsBinaryNode();
    if (binaryNode)
    {
        if (binaryNode->getOp() == EOpIndexDirectInterfaceBlock)
        {
            return binaryNode->getLeft()->getType().getInterfaceBlock();
        }
    }

    const TIntermSymbol *symbolNode = node->getAsSymbolNode();
    if (symbolNode)
    {
        const TVariable &variable = symbolNode->variable();
        const TType &variableType = variable.getType();

        if (variableType.getQualifier() == EvqUniform &&
            variable.symbolType() == SymbolType::UserDefined)
        {
            return variableType.getInterfaceBlock();
        }
    }

    return nullptr;
}

const char *GetHLSLAtomicFunctionStringAndLeftParenthesis(TOperator op)
{
    switch (op)
    {
        case EOpAtomicAdd:
            return "InterlockedAdd(";
        case EOpAtomicMin:
            return "InterlockedMin(";
        case EOpAtomicMax:
            return "InterlockedMax(";
        case EOpAtomicAnd:
            return "InterlockedAnd(";
        case EOpAtomicOr:
            return "InterlockedOr(";
        case EOpAtomicXor:
            return "InterlockedXor(";
        case EOpAtomicExchange:
            return "InterlockedExchange(";
        case EOpAtomicCompSwap:
            return "InterlockedCompareExchange(";
        default:
            UNREACHABLE();
            return "";
    }
}

bool IsAtomicFunctionForSharedVariableDirectAssign(const TIntermBinary &node)
{
    TIntermAggregate *aggregateNode = node.getRight()->getAsAggregate();
    if (aggregateNode == nullptr)
    {
        return false;
    }

    if (node.getOp() == EOpAssign && BuiltInGroup::IsAtomicMemory(aggregateNode->getOp()))
    {
        return !IsInShaderStorageBlock((*aggregateNode->getSequence())[0]->getAsTyped()) &&
               !IsInShaderStorageBlock(node.getLeft());
    }

    return false;
}

const char *kZeros       = "_ANGLE_ZEROS_";
constexpr int kZeroCount = 256;
std::string DefineZeroArray()
{
    std::stringstream ss = sh::InitializeStream<std::stringstream>();
    // For 'static', if the declaration does not include an initializer, the value is set to zero.
    // https://docs.microsoft.com/en-us/windows/desktop/direct3dhlsl/dx-graphics-hlsl-variable-syntax
    ss << "static uint " << kZeros << "[" << kZeroCount << "];\n";
    return ss.str();
}

std::string GetZeroInitializer(size_t size)
{
    std::stringstream ss = sh::InitializeStream<std::stringstream>();
    size_t quotient      = size / kZeroCount;
    size_t reminder      = size % kZeroCount;

    for (size_t i = 0; i < quotient; ++i)
    {
        if (i != 0)
        {
            ss << ", ";
        }
        ss << kZeros;
    }

    for (size_t i = 0; i < reminder; ++i)
    {
        if (quotient != 0 || i != 0)
        {
            ss << ", ";
        }
        ss << "0";
    }

    return ss.str();
}

bool IsFlatInterpolant(TIntermTyped *node)
{
    TIntermTyped *interpolant = node->getAsBinaryNode() ? node->getAsBinaryNode()->getLeft() : node;
    return interpolant->getType().getQualifier() == EvqFlatIn;
}

}  // anonymous namespace

TReferencedBlock::TReferencedBlock(const TInterfaceBlock *aBlock,
                                   const TVariable *aInstanceVariable)
    : block(aBlock), instanceVariable(aInstanceVariable)
{}

bool OutputHLSL::needStructMapping(TIntermTyped *node)
{
    ASSERT(node->getBasicType() == EbtStruct);
    for (unsigned int n = 0u; getAncestorNode(n) != nullptr; ++n)
    {
        TIntermNode *ancestor               = getAncestorNode(n);
        const TIntermBinary *ancestorBinary = ancestor->getAsBinaryNode();
        if (ancestorBinary)
        {
            switch (ancestorBinary->getOp())
            {
                case EOpIndexDirectStruct:
                {
                    const TStructure *structure = ancestorBinary->getLeft()->getType().getStruct();
                    const TIntermConstantUnion *index =
                        ancestorBinary->getRight()->getAsConstantUnion();
                    const TField *field = structure->fields()[index->getIConst(0)];
                    if (field->type()->getStruct() == nullptr)
                    {
                        return false;
                    }
                    break;
                }
                case EOpIndexDirect:
                case EOpIndexIndirect:
                    break;
                default:
                    return true;
            }
        }
        else
        {
            const TIntermAggregate *ancestorAggregate = ancestor->getAsAggregate();
            if (ancestorAggregate)
            {
                return true;
            }
            return false;
        }
    }
    return true;
}

void OutputHLSL::writeFloat(TInfoSinkBase &out, float f)
{
    // This is known not to work for NaN on all drivers but make the best effort to output NaNs
    // regardless.
    if ((gl::isInf(f) || gl::isNaN(f)) && mShaderVersion >= 300 &&
        mOutputType == SH_HLSL_4_1_OUTPUT)
    {
        out << "asfloat(" << gl::bitCast<uint32_t>(f) << "u)";
    }
    else
    {
        out << std::min(FLT_MAX, std::max(-FLT_MAX, f));
    }
}

void OutputHLSL::writeSingleConstant(TInfoSinkBase &out, const TConstantUnion *const constUnion)
{
    ASSERT(constUnion != nullptr);
    switch (constUnion->getType())
    {
        case EbtFloat:
            writeFloat(out, constUnion->getFConst());
            break;
        case EbtInt:
            out << constUnion->getIConst();
            break;
        case EbtUInt:
            out << constUnion->getUConst();
            break;
        case EbtBool:
            out << constUnion->getBConst();
            break;
        default:
            UNREACHABLE();
    }
}

const TConstantUnion *OutputHLSL::writeConstantUnionArray(TInfoSinkBase &out,
                                                          const TConstantUnion *const constUnion,
                                                          const size_t size)
{
    const TConstantUnion *constUnionIterated = constUnion;
    for (size_t i = 0; i < size; i++, constUnionIterated++)
    {
        writeSingleConstant(out, constUnionIterated);

        if (i != size - 1)
        {
            out << ", ";
        }
    }
    return constUnionIterated;
}

OutputHLSL::OutputHLSL(sh::GLenum shaderType,
                       ShShaderSpec shaderSpec,
                       int shaderVersion,
                       const TExtensionBehavior &extensionBehavior,
                       const char *sourcePath,
                       ShShaderOutput outputType,
                       int numRenderTargets,
                       int maxDualSourceDrawBuffers,
                       const std::vector<ShaderVariable> &uniforms,
                       const ShCompileOptions &compileOptions,
                       sh::WorkGroupSize workGroupSize,
                       TSymbolTable *symbolTable,
                       PerformanceDiagnostics *perfDiagnostics,
                       const std::map<int, const TInterfaceBlock *> &uniformBlockOptimizedMap,
                       const std::vector<InterfaceBlock> &shaderStorageBlocks,
                       uint8_t clipDistanceSize,
                       uint8_t cullDistanceSize,
                       bool isEarlyFragmentTestsSpecified)
    : TIntermTraverser(true, true, true, symbolTable),
      mShaderType(shaderType),
      mShaderSpec(shaderSpec),
      mShaderVersion(shaderVersion),
      mExtensionBehavior(extensionBehavior),
      mSourcePath(sourcePath),
      mOutputType(outputType),
      mCompileOptions(compileOptions),
      mInsideFunction(false),
      mInsideMain(false),
      mUniformBlockOptimizedMap(uniformBlockOptimizedMap),
      mNumRenderTargets(numRenderTargets),
      mMaxDualSourceDrawBuffers(maxDualSourceDrawBuffers),
      mCurrentFunctionMetadata(nullptr),
      mWorkGroupSize(workGroupSize),
      mPerfDiagnostics(perfDiagnostics),
      mClipDistanceSize(clipDistanceSize),
      mCullDistanceSize(cullDistanceSize),
      mIsEarlyFragmentTestsSpecified(isEarlyFragmentTestsSpecified),
      mNeedStructMapping(false)
{
    mUsesFragColor        = false;
    mUsesFragData         = false;
    mUsesDepthRange       = false;
    mUsesFragCoord        = false;
    mUsesPointCoord       = false;
    mUsesFrontFacing      = false;
    mUsesHelperInvocation = false;
    mUsesPointSize        = false;
    mUsesInstanceID       = false;
    mHasMultiviewExtensionEnabled =
        IsExtensionEnabled(mExtensionBehavior, TExtension::OVR_multiview) ||
        IsExtensionEnabled(mExtensionBehavior, TExtension::OVR_multiview2);
    mUsesViewID                  = false;
    mUsesVertexID                = false;
    mUsesFragDepth               = false;
    mUsesSampleID                = false;
    mUsesSamplePosition          = false;
    mUsesSampleMaskIn            = false;
    mUsesSampleMask              = false;
    mUsesNumSamples              = false;
    mUsesNumWorkGroups           = false;
    mUsesWorkGroupID             = false;
    mUsesLocalInvocationID       = false;
    mUsesGlobalInvocationID      = false;
    mUsesLocalInvocationIndex    = false;
    mUsesXor                     = false;
    mUsesDiscardRewriting        = false;
    mUsesNestedBreak             = false;
    mRequiresIEEEStrictCompiling = false;
    mUseZeroArray                = false;
    mUsesSecondaryColor          = false;

    mDepthLayout = EdUnspecified;

    mUniqueIndex = 0;

    mOutputLod0Function      = false;
    mInsideDiscontinuousLoop = false;
    mNestedLoopDepth         = 0;

    mExcessiveLoopIndex = nullptr;

    mStructureHLSL       = new StructureHLSL;
    mTextureFunctionHLSL = new TextureFunctionHLSL;
    mImageFunctionHLSL   = new ImageFunctionHLSL;
    mAtomicCounterFunctionHLSL =
        new AtomicCounterFunctionHLSL(compileOptions.forceAtomicValueResolution);

    unsigned int firstUniformRegister = compileOptions.skipD3DConstantRegisterZero ? 1u : 0u;
    mResourcesHLSL = new ResourcesHLSL(mStructureHLSL, outputType, uniforms, firstUniformRegister);

    if (mOutputType == SH_HLSL_3_0_OUTPUT)
    {
        // Fragment shaders need dx_DepthRange, dx_ViewCoords, dx_DepthFront,
        // and dx_FragCoordOffset.
        // Vertex shaders need a slightly different set: dx_DepthRange, dx_ViewCoords and
        // dx_ViewAdjust.
        if (mShaderType == GL_VERTEX_SHADER)
        {
            mResourcesHLSL->reserveUniformRegisters(3);
        }
        else
        {
            mResourcesHLSL->reserveUniformRegisters(4);
        }
    }

    // Reserve registers for the default uniform block and driver constants
    mResourcesHLSL->reserveUniformBlockRegisters(2);

    mSSBOOutputHLSL = new ShaderStorageBlockOutputHLSL(this, mResourcesHLSL, shaderStorageBlocks);
}

OutputHLSL::~OutputHLSL()
{
    SafeDelete(mSSBOOutputHLSL);
    SafeDelete(mStructureHLSL);
    SafeDelete(mResourcesHLSL);
    SafeDelete(mTextureFunctionHLSL);
    SafeDelete(mImageFunctionHLSL);
    SafeDelete(mAtomicCounterFunctionHLSL);
    for (auto &eqFunction : mStructEqualityFunctions)
    {
        SafeDelete(eqFunction);
    }
    for (auto &eqFunction : mArrayEqualityFunctions)
    {
        SafeDelete(eqFunction);
    }
}

void OutputHLSL::output(TIntermNode *treeRoot, TInfoSinkBase &objSink)
{
    BuiltInFunctionEmulator builtInFunctionEmulator;
    InitBuiltInFunctionEmulatorForHLSL(&builtInFunctionEmulator);
    if (mCompileOptions.emulateIsnanFloatFunction)
    {
        InitBuiltInIsnanFunctionEmulatorForHLSLWorkarounds(&builtInFunctionEmulator,
                                                           mShaderVersion);
    }

    builtInFunctionEmulator.markBuiltInFunctionsForEmulation(treeRoot);

    // Now that we are done changing the AST, do the analyses need for HLSL generation
    CallDAG::InitResult success = mCallDag.init(treeRoot, nullptr);
    ASSERT(success == CallDAG::INITDAG_SUCCESS);
    mASTMetadataList = CreateASTMetadataHLSL(treeRoot, mCallDag);

    const std::vector<MappedStruct> std140Structs = FlagStd140Structs(treeRoot);
    // TODO(oetuaho): The std140Structs could be filtered based on which ones actually get used in
    // the shader code. When we add shader storage blocks we might also consider an alternative
    // solution, since the struct mapping won't work very well for shader storage blocks.

    // Output the body and footer first to determine what has to go in the header
    mInfoSinkStack.push(&mBody);
    treeRoot->traverse(this);
    mInfoSinkStack.pop();

    mInfoSinkStack.push(&mFooter);
    mInfoSinkStack.pop();

    mInfoSinkStack.push(&mHeader);
    header(mHeader, std140Structs, &builtInFunctionEmulator);
    mInfoSinkStack.pop();

    objSink << mHeader.c_str();
    objSink << mBody.c_str();
    objSink << mFooter.c_str();

    builtInFunctionEmulator.cleanup();
}

const std::map<std::string, unsigned int> &OutputHLSL::getShaderStorageBlockRegisterMap() const
{
    return mResourcesHLSL->getShaderStorageBlockRegisterMap();
}

const std::map<std::string, unsigned int> &OutputHLSL::getUniformBlockRegisterMap() const
{
    return mResourcesHLSL->getUniformBlockRegisterMap();
}

const std::map<std::string, bool> &OutputHLSL::getUniformBlockUseStructuredBufferMap() const
{
    return mResourcesHLSL->getUniformBlockUseStructuredBufferMap();
}

const std::map<std::string, unsigned int> &OutputHLSL::getUniformRegisterMap() const
{
    return mResourcesHLSL->getUniformRegisterMap();
}

unsigned int OutputHLSL::getReadonlyImage2DRegisterIndex() const
{
    return mResourcesHLSL->getReadonlyImage2DRegisterIndex();
}

unsigned int OutputHLSL::getImage2DRegisterIndex() const
{
    return mResourcesHLSL->getImage2DRegisterIndex();
}

const std::set<std::string> &OutputHLSL::getUsedImage2DFunctionNames() const
{
    return mImageFunctionHLSL->getUsedImage2DFunctionNames();
}

TString OutputHLSL::structInitializerString(int indent,
                                            const TType &type,
                                            const TString &name) const
{
    TString init;

    TString indentString;
    for (int spaces = 0; spaces < indent; spaces++)
    {
        indentString += "    ";
    }

    if (type.isArray())
    {
        init += indentString + "{\n";
        for (unsigned int arrayIndex = 0u; arrayIndex < type.getOutermostArraySize(); ++arrayIndex)
        {
            TStringStream indexedString = sh::InitializeStream<TStringStream>();
            indexedString << name << "[" << arrayIndex << "]";
            TType elementType = type;
            elementType.toArrayElementType();
            init += structInitializerString(indent + 1, elementType, indexedString.str());
            if (arrayIndex < type.getOutermostArraySize() - 1)
            {
                init += ",";
            }
            init += "\n";
        }
        init += indentString + "}";
    }
    else if (type.getBasicType() == EbtStruct)
    {
        init += indentString + "{\n";
        const TStructure &structure = *type.getStruct();
        const TFieldList &fields    = structure.fields();
        for (unsigned int fieldIndex = 0; fieldIndex < fields.size(); fieldIndex++)
        {
            const TField &field      = *fields[fieldIndex];
            const TString &fieldName = name + "." + Decorate(field.name());
            const TType &fieldType   = *field.type();

            init += structInitializerString(indent + 1, fieldType, fieldName);
            if (fieldIndex < fields.size() - 1)
            {
                init += ",";
            }
            init += "\n";
        }
        init += indentString + "}";
    }
    else
    {
        init += indentString + name;
    }

    return init;
}

TString OutputHLSL::generateStructMapping(const std::vector<MappedStruct> &std140Structs) const
{
    TString mappedStructs;

    for (auto &mappedStruct : std140Structs)
    {
        const TInterfaceBlock *interfaceBlock =
            mappedStruct.blockDeclarator->getType().getInterfaceBlock();
        TQualifier qualifier = mappedStruct.blockDeclarator->getType().getQualifier();
        switch (qualifier)
        {
            case EvqUniform:
                if (mReferencedUniformBlocks.count(interfaceBlock->uniqueId().get()) == 0)
                {
                    continue;
                }
                break;
            case EvqBuffer:
                continue;
            default:
                UNREACHABLE();
                return mappedStructs;
        }

        unsigned int instanceCount = 1u;
        bool isInstanceArray       = mappedStruct.blockDeclarator->isArray();
        if (isInstanceArray)
        {
            instanceCount = mappedStruct.blockDeclarator->getOutermostArraySize();
        }

        for (unsigned int instanceArrayIndex = 0; instanceArrayIndex < instanceCount;
             ++instanceArrayIndex)
        {
            TString originalName;
            TString mappedName("map");

            if (mappedStruct.blockDeclarator->variable().symbolType() != SymbolType::Empty)
            {
                const ImmutableString &instanceName =
                    mappedStruct.blockDeclarator->variable().name();
                unsigned int instanceStringArrayIndex = GL_INVALID_INDEX;
                if (isInstanceArray)
                    instanceStringArrayIndex = instanceArrayIndex;
                TString instanceString = mResourcesHLSL->InterfaceBlockInstanceString(
                    instanceName, instanceStringArrayIndex);
                originalName += instanceString;
                mappedName += instanceString;
                originalName += ".";
                mappedName += "_";
            }

            TString fieldName = Decorate(mappedStruct.field->name());
            originalName += fieldName;
            mappedName += fieldName;

            TType *structType = mappedStruct.field->type();
            mappedStructs +=
                "static " + Decorate(structType->getStruct()->name()) + " " + mappedName;

            if (structType->isArray())
            {
                mappedStructs += ArrayString(*mappedStruct.field->type()).data();
            }

            mappedStructs += " =\n";
            mappedStructs += structInitializerString(0, *structType, originalName);
            mappedStructs += ";\n";
        }
    }
    return mappedStructs;
}

void OutputHLSL::writeReferencedAttributes(TInfoSinkBase &out) const
{
    for (const auto &attribute : mReferencedAttributes)
    {
        const TType &type           = attribute.second->getType();
        const ImmutableString &name = attribute.second->name();

        out << "static " << TypeString(type) << " " << Decorate(name) << ArrayString(type) << " = "
            << zeroInitializer(type) << ";\n";
    }
}

void OutputHLSL::writeReferencedVaryings(TInfoSinkBase &out) const
{
    for (const auto &varying : mReferencedVaryings)
    {
        const TType &type = varying.second->getType();

        // Program linking depends on this exact format
        out << "static " << InterpolationString(type.getQualifier()) << " " << TypeString(type)
            << " " << DecorateVariableIfNeeded(*varying.second) << ArrayString(type) << " = "
            << zeroInitializer(type) << ";\n";
    }
}

void OutputHLSL::header(TInfoSinkBase &out,
                        const std::vector<MappedStruct> &std140Structs,
                        const BuiltInFunctionEmulator *builtInFunctionEmulator) const
{
    TString mappedStructs;
    if (mNeedStructMapping)
    {
        mappedStructs = generateStructMapping(std140Structs);
    }

    // Suppress some common warnings:
    // 3556 : Integer divides might be much slower, try using uints if possible.
    // 3571 : The pow(f, e) intrinsic function won't work for negative f, use abs(f) or
    //        conditionally handle negative values if you expect them.
    out << "#pragma warning( disable: 3556 3571 )\n";

    out << mStructureHLSL->structsHeader();

    mResourcesHLSL->uniformsHeader(out, mOutputType, mReferencedUniforms, mSymbolTable);
    out << mResourcesHLSL->uniformBlocksHeader(mReferencedUniformBlocks, mUniformBlockOptimizedMap);
    mSSBOOutputHLSL->writeShaderStorageBlocksHeader(mShaderType, out);

    if (!mEqualityFunctions.empty())
    {
        out << "\n// Equality functions\n\n";
        for (const auto &eqFunction : mEqualityFunctions)
        {
            out << eqFunction->functionDefinition << "\n";
        }
    }
    if (!mArrayAssignmentFunctions.empty())
    {
        out << "\n// Assignment functions\n\n";
        for (const auto &assignmentFunction : mArrayAssignmentFunctions)
        {
            out << assignmentFunction.functionDefinition << "\n";
        }
    }
    if (!mArrayConstructIntoFunctions.empty())
    {
        out << "\n// Array constructor functions\n\n";
        for (const auto &constructIntoFunction : mArrayConstructIntoFunctions)
        {
            out << constructIntoFunction.functionDefinition << "\n";
        }
    }
    if (!mFlatEvaluateFunctions.empty())
    {
        out << "\n// Evaluate* functions for flat inputs\n\n";
        for (const auto &flatEvaluateFunction : mFlatEvaluateFunctions)
        {
            out << flatEvaluateFunction.functionDefinition << "\n";
        }
    }

    if (mUsesDiscardRewriting)
    {
        out << "#define ANGLE_USES_DISCARD_REWRITING\n";
    }

    if (mUsesNestedBreak)
    {
        out << "#define ANGLE_USES_NESTED_BREAK\n";
    }

    if (mRequiresIEEEStrictCompiling)
    {
        out << "#define ANGLE_REQUIRES_IEEE_STRICT_COMPILING\n";
    }

    out << "#ifdef ANGLE_ENABLE_LOOP_FLATTEN\n"
           "#define LOOP [loop]\n"
           "#define FLATTEN [flatten]\n"
           "#else\n"
           "#define LOOP\n"
           "#define FLATTEN\n"
           "#endif\n";

    // array stride for atomic counter buffers is always 4 per original extension
    // ARB_shader_atomic_counters and discussion on
    // https://github.com/KhronosGroup/OpenGL-API/issues/5
    out << "\n#define ATOMIC_COUNTER_ARRAY_STRIDE 4\n\n";

    if (mUseZeroArray)
    {
        out << DefineZeroArray() << "\n";
    }

    if (mShaderType == GL_FRAGMENT_SHADER)
    {
        const bool usingMRTExtension =
            IsExtensionEnabled(mExtensionBehavior, TExtension::EXT_draw_buffers);
        const bool usingBFEExtension =
            IsExtensionEnabled(mExtensionBehavior, TExtension::EXT_blend_func_extended);

        out << "// Varyings\n";
        writeReferencedVaryings(out);
        out << "\n";

        if (mShaderVersion >= 300)
        {
            for (const auto &outputVariable : mReferencedOutputVariables)
            {
                const ImmutableString &variableName = outputVariable.second->name();
                const TType &variableType           = outputVariable.second->getType();

                out << "static " << TypeString(variableType) << " out_" << variableName
                    << ArrayString(variableType) << " = " << zeroInitializer(variableType) << ";\n";
            }
        }
        else
        {
            const unsigned int numColorValues = usingMRTExtension ? mNumRenderTargets : 1;

            out << "static float4 gl_Color[" << numColorValues
                << "] =\n"
                   "{\n";
            for (unsigned int i = 0; i < numColorValues; i++)
            {
                out << "    float4(0, 0, 0, 0)";
                if (i + 1 != numColorValues)
                {
                    out << ",";
                }
                out << "\n";
            }

            out << "};\n";

            if (usingBFEExtension && mUsesSecondaryColor)
            {
                out << "static float4 gl_SecondaryColor[" << mMaxDualSourceDrawBuffers
                    << "] = \n"
                       "{\n";
                for (int i = 0; i < mMaxDualSourceDrawBuffers; i++)
                {
                    out << "    float4(0, 0, 0, 0)";
                    if (i + 1 != mMaxDualSourceDrawBuffers)
                    {
                        out << ",";
                    }
                    out << "\n";
                }
                out << "};\n";
            }
        }

        if (mUsesViewID)
        {
            out << "static uint ViewID_OVR = 0;\n";
        }

        if (mUsesFragDepth)
        {
            out << "static float gl_Depth = 0.0;\n";
        }

        if (mUsesSampleID)
        {
            out << "static int gl_SampleID = 0;\n";
        }

        if (mUsesSamplePosition)
        {
            out << "static float2 gl_SamplePosition = float2(0.0, 0.0);\n";
        }

        if (mUsesSampleMaskIn)
        {
            out << "static int gl_SampleMaskIn[1] = {0};\n";
        }

        if (mUsesSampleMask)
        {
            out << "static int gl_SampleMask[1] = {0};\n";
        }

        if (mUsesNumSamples)
        {
            out << "static int gl_NumSamples = GetRenderTargetSampleCount();\n";
        }

        if (mUsesFragCoord)
        {
            out << "static float4 gl_FragCoord = float4(0, 0, 0, 0);\n";
        }

        if (mUsesPointCoord)
        {
            out << "static float2 gl_PointCoord = float2(0.5, 0.5);\n";
        }

        if (mUsesFrontFacing)
        {
            out << "static bool gl_FrontFacing = false;\n";
        }

        if (mUsesHelperInvocation)
        {
            out << "static bool gl_HelperInvocation = false;\n";
        }

        out << "\n";

        if (mUsesDepthRange)
        {
            out << "struct gl_DepthRangeParameters\n"
                   "{\n"
                   "    float near;\n"
                   "    float far;\n"
                   "    float diff;\n"
                   "};\n"
                   "\n";
        }

        if (mOutputType == SH_HLSL_4_1_OUTPUT)
        {
            out << "cbuffer DriverConstants : register(b1)\n"
                   "{\n";

            if (mUsesDepthRange)
            {
                out << "    float3 dx_DepthRange : packoffset(c0);\n";
            }

            if (mUsesFragCoord)
            {
                out << "    float4 dx_ViewCoords : packoffset(c1);\n";
                out << "    float2 dx_FragCoordOffset : packoffset(c3);\n";
            }

            if (mUsesFragCoord || mUsesFrontFacing)
            {
                out << "    float3 dx_DepthFront : packoffset(c2);\n";
            }

            if (mUsesFragCoord)
            {
                // dx_ViewScale is only used in the fragment shader to correct
                // the value for glFragCoord if necessary
                out << "    float2 dx_ViewScale : packoffset(c3.z);\n";
            }

            if (mOutputType == SH_HLSL_4_1_OUTPUT)
            {
                out << "    uint dx_Misc : packoffset(c2.w);\n";
                unsigned int registerIndex = 4;
                mResourcesHLSL->samplerMetadataUniforms(out, registerIndex);
                // Sampler metadata struct must be two 4-vec, 32 bytes.
                registerIndex += mResourcesHLSL->getSamplerCount() * 2;
                mResourcesHLSL->imageMetadataUniforms(out, registerIndex);
            }

            out << "};\n";

            if (mOutputType == SH_HLSL_4_1_OUTPUT && mResourcesHLSL->hasImages())
            {
                out << kImage2DFunctionString << "\n";
            }
        }
        else
        {
            if (mUsesDepthRange)
            {
                out << "uniform float3 dx_DepthRange : register(c0);";
            }

            if (mUsesFragCoord)
            {
                out << "uniform float4 dx_ViewCoords : register(c1);\n";
            }

            if (mUsesFragCoord || mUsesFrontFacing)
            {
                out << "uniform float3 dx_DepthFront : register(c2);\n";
                out << "uniform float2 dx_FragCoordOffset : register(c3);\n";
            }
        }

        out << "\n";

        if (mUsesDepthRange)
        {
            out << "static gl_DepthRangeParameters gl_DepthRange = {dx_DepthRange.x, "
                   "dx_DepthRange.y, dx_DepthRange.z};\n"
                   "\n";
        }

        if (mClipDistanceSize)
        {
            out << "static float gl_ClipDistance[" << static_cast<int>(mClipDistanceSize)
                << "] = {0";
            for (unsigned int i = 1; i < mClipDistanceSize; i++)
            {
                out << ", 0";
            }
            out << "};\n";
        }

        if (mCullDistanceSize)
        {
            out << "static float gl_CullDistance[" << static_cast<int>(mCullDistanceSize)
                << "] = {0";
            for (unsigned int i = 1; i < mCullDistanceSize; i++)
            {
                out << ", 0";
            }
            out << "};\n";
        }

        if (usingMRTExtension && mNumRenderTargets > 1)
        {
            out << "#define GL_USES_MRT\n";
        }

        if (mUsesFragColor)
        {
            out << "#define GL_USES_FRAG_COLOR\n";
        }

        if (mUsesFragData)
        {
            out << "#define GL_USES_FRAG_DATA\n";
        }

        if (mShaderVersion < 300 && usingBFEExtension && mUsesSecondaryColor)
        {
            out << "#define GL_USES_SECONDARY_COLOR\n";
        }
    }
    else if (mShaderType == GL_VERTEX_SHADER)
    {
        out << "// Attributes\n";
        writeReferencedAttributes(out);
        out << "\n"
               "static float4 gl_Position = float4(0, 0, 0, 0);\n";

        if (mClipDistanceSize)
        {
            out << "static float gl_ClipDistance[" << static_cast<int>(mClipDistanceSize)
                << "] = {0";
            for (size_t i = 1; i < mClipDistanceSize; i++)
            {
                out << ", 0";
            }
            out << "};\n";
        }

        if (mCullDistanceSize)
        {
            out << "static float gl_CullDistance[" << static_cast<int>(mCullDistanceSize)
                << "] = {0";
            for (size_t i = 1; i < mCullDistanceSize; i++)
            {
                out << ", 0";
            }
            out << "};\n";
        }

        if (mUsesPointSize)
        {
            out << "static float gl_PointSize = float(1);\n";
        }

        if (mUsesInstanceID)
        {
            out << "static int gl_InstanceID;\n";
        }

        if (mUsesViewID)
        {
            out << "static uint ViewID_OVR;\n";
        }

        if (mUsesVertexID)
        {
            out << "static int gl_VertexID;\n";
        }

        out << "\n"
               "// Varyings\n";
        writeReferencedVaryings(out);
        out << "\n";

        if (mUsesDepthRange)
        {
            out << "struct gl_DepthRangeParameters\n"
                   "{\n"
                   "    float near;\n"
                   "    float far;\n"
                   "    float diff;\n"
                   "};\n"
                   "\n";
        }

        if (mOutputType == SH_HLSL_4_1_OUTPUT)
        {
            out << "cbuffer DriverConstants : register(b1)\n"
                   "{\n";

            if (mUsesDepthRange)
            {
                out << "    float3 dx_DepthRange : packoffset(c0);\n";
            }

            // dx_ViewAdjust and dx_ViewCoords will only be used in Feature Level 9
            // shaders. However, we declare it for all shaders (including Feature Level 10+).
            // The bytecode is the same whether we declare it or not, since D3DCompiler removes it
            // if it's unused.
            out << "    float4 dx_ViewAdjust : packoffset(c1);\n";
            out << "    float2 dx_ViewCoords : packoffset(c2);\n";
            out << "    float2 dx_ViewScale  : packoffset(c3);\n";

            out << "    float clipControlOrigin : packoffset(c3.z);\n";
            out << "    float clipControlZeroToOne : packoffset(c3.w);\n";

            if (mOutputType == SH_HLSL_4_1_OUTPUT)
            {
                mResourcesHLSL->samplerMetadataUniforms(out, 5);
            }

            if (mUsesVertexID)
            {
                out << "    uint dx_VertexID : packoffset(c4.x);\n";
            }

            if (mClipDistanceSize)
            {
                out << "    uint clipDistancesEnabled : packoffset(c4.y);\n";
            }

            out << "};\n"
                   "\n";
        }
        else
        {
            if (mUsesDepthRange)
            {
                out << "uniform float3 dx_DepthRange : register(c0);\n";
            }

            out << "uniform float4 dx_ViewAdjust : register(c1);\n";
            out << "uniform float2 dx_ViewCoords : register(c2);\n";

            out << "static const float clipControlOrigin = -1.0f;\n";
            out << "static const float clipControlZeroToOne = 0.0f;\n";

            out << "\n";
        }

        if (mUsesDepthRange)
        {
            out << "static gl_DepthRangeParameters gl_DepthRange = {dx_DepthRange.x, "
                   "dx_DepthRange.y, dx_DepthRange.z};\n"
                   "\n";
        }

        if (mOutputType == SH_HLSL_4_1_OUTPUT && mResourcesHLSL->hasImages())
        {
            out << kImage2DFunctionString << "\n";
        }
    }
    else  // Compute shader
    {
        ASSERT(mShaderType == GL_COMPUTE_SHADER);

        out << "cbuffer DriverConstants : register(b1)\n"
               "{\n";
        if (mUsesNumWorkGroups)
        {
            out << "    uint3 gl_NumWorkGroups : packoffset(c0);\n";
        }
        ASSERT(mOutputType == SH_HLSL_4_1_OUTPUT);
        unsigned int registerIndex = 1;
        mResourcesHLSL->samplerMetadataUniforms(out, registerIndex);
        // Sampler metadata struct must be two 4-vec, 32 bytes.
        registerIndex += mResourcesHLSL->getSamplerCount() * 2;
        mResourcesHLSL->imageMetadataUniforms(out, registerIndex);
        out << "};\n";

        out << kImage2DFunctionString << "\n";

        std::ostringstream systemValueDeclaration  = sh::InitializeStream<std::ostringstream>();
        std::ostringstream glBuiltinInitialization = sh::InitializeStream<std::ostringstream>();

        systemValueDeclaration << "\nstruct CS_INPUT\n{\n";
        glBuiltinInitialization << "\nvoid initGLBuiltins(CS_INPUT input)\n" << "{\n";

        if (mUsesWorkGroupID)
        {
            out << "static uint3 gl_WorkGroupID = uint3(0, 0, 0);\n";
            systemValueDeclaration << "    uint3 dx_WorkGroupID : " << "SV_GroupID;\n";
            glBuiltinInitialization << "    gl_WorkGroupID = input.dx_WorkGroupID;\n";
        }

        if (mUsesLocalInvocationID)
        {
            out << "static uint3 gl_LocalInvocationID = uint3(0, 0, 0);\n";
            systemValueDeclaration << "    uint3 dx_LocalInvocationID : " << "SV_GroupThreadID;\n";
            glBuiltinInitialization << "    gl_LocalInvocationID = input.dx_LocalInvocationID;\n";
        }

        if (mUsesGlobalInvocationID)
        {
            out << "static uint3 gl_GlobalInvocationID = uint3(0, 0, 0);\n";
            systemValueDeclaration << "    uint3 dx_GlobalInvocationID : "
                                   << "SV_DispatchThreadID;\n";
            glBuiltinInitialization << "    gl_GlobalInvocationID = input.dx_GlobalInvocationID;\n";
        }

        if (mUsesLocalInvocationIndex)
        {
            out << "static uint gl_LocalInvocationIndex = uint(0);\n";
            systemValueDeclaration << "    uint dx_LocalInvocationIndex : " << "SV_GroupIndex;\n";
            glBuiltinInitialization
                << "    gl_LocalInvocationIndex = input.dx_LocalInvocationIndex;\n";
        }

        systemValueDeclaration << "};\n\n";
        glBuiltinInitialization << "};\n\n";

        out << systemValueDeclaration.str();
        out << glBuiltinInitialization.str();
    }

    if (!mappedStructs.empty())
    {
        out << "// Structures from std140 blocks with padding removed\n";
        out << "\n";
        out << mappedStructs;
        out << "\n";
    }

    bool getDimensionsIgnoresBaseLevel = mCompileOptions.HLSLGetDimensionsIgnoresBaseLevel;
    mTextureFunctionHLSL->textureFunctionHeader(out, mOutputType, getDimensionsIgnoresBaseLevel);
    mImageFunctionHLSL->imageFunctionHeader(out);
    mAtomicCounterFunctionHLSL->atomicCounterFunctionHeader(out);

    if (mUsesFragCoord)
    {
        out << "#define GL_USES_FRAG_COORD\n";
    }

    if (mUsesPointCoord)
    {
        out << "#define GL_USES_POINT_COORD\n";
    }

    if (mUsesFrontFacing)
    {
        out << "#define GL_USES_FRONT_FACING\n";
    }

    if (mUsesHelperInvocation)
    {
        out << "#define GL_USES_HELPER_INVOCATION\n";
    }

    if (mUsesPointSize)
    {
        out << "#define GL_USES_POINT_SIZE\n";
    }

    if (mHasMultiviewExtensionEnabled)
    {
        out << "#define GL_MULTIVIEW_ENABLED\n";
    }

    if (mUsesVertexID)
    {
        out << "#define GL_USES_VERTEX_ID\n";
    }

    if (mUsesViewID)
    {
        out << "#define GL_USES_VIEW_ID\n";
    }

    if (mUsesSampleID)
    {
        out << "#define GL_USES_SAMPLE_ID\n";
    }

    if (mUsesSamplePosition)
    {
        out << "#define GL_USES_SAMPLE_POSITION\n";
    }

    if (mUsesSampleMaskIn)
    {
        out << "#define GL_USES_SAMPLE_MASK_IN\n";
    }

    if (mUsesSampleMask)
    {
        out << "#define GL_USES_SAMPLE_MASK_OUT\n";
    }

    if (mUsesFragDepth)
    {
        switch (mDepthLayout)
        {
            case EdGreater:
                out << "#define GL_USES_FRAG_DEPTH_GREATER\n";
                break;
            case EdLess:
                out << "#define GL_USES_FRAG_DEPTH_LESS\n";
                break;
            default:
                out << "#define GL_USES_FRAG_DEPTH\n";
                break;
        }
    }

    if (mUsesDepthRange)
    {
        out << "#define GL_USES_DEPTH_RANGE\n";
    }

    if (mUsesXor)
    {
        out << "bool xor(bool p, bool q)\n"
               "{\n"
               "    return (p || q) && !(p && q);\n"
               "}\n"
               "\n";
    }

    builtInFunctionEmulator->outputEmulatedFunctions(out);
}

void OutputHLSL::visitSymbol(TIntermSymbol *node)
{
    const TVariable &variable = node->variable();

    // Empty symbols can only appear in declarations and function arguments, and in either of those
    // cases the symbol nodes are not visited.
    ASSERT(variable.symbolType() != SymbolType::Empty);

    TInfoSinkBase &out = getInfoSink();

    // Handle accessing std140 structs by value
    if (IsInStd140UniformBlock(node) && node->getBasicType() == EbtStruct &&
        needStructMapping(node))
    {
        mNeedStructMapping = true;
        out << "map";
    }

    const ImmutableString &name     = variable.name();
    const TSymbolUniqueId &uniqueId = variable.uniqueId();

    if (name == "gl_DepthRange")
    {
        mUsesDepthRange = true;
        out << name;
    }
    else if (name == "gl_NumSamples")
    {
        mUsesNumSamples = true;
        out << name;
    }
    else if (IsAtomicCounter(variable.getType().getBasicType()))
    {
        const TType &variableType = variable.getType();
        if (variableType.getQualifier() == EvqUniform)
        {
            TLayoutQualifier layout             = variableType.getLayoutQualifier();
            mReferencedUniforms[uniqueId.get()] = &variable;
            out << getAtomicCounterNameForBinding(layout.binding) << ", " << layout.offset;
        }
        else
        {
            TString varName = DecorateVariableIfNeeded(variable);
            out << varName << ", " << varName << "_offset";
        }
    }
    else
    {
        const TType &variableType = variable.getType();
        TQualifier qualifier      = variable.getType().getQualifier();

        ensureStructDefined(variableType);

        if (qualifier == EvqUniform)
        {
            const TInterfaceBlock *interfaceBlock = variableType.getInterfaceBlock();

            if (interfaceBlock)
            {
                if (mReferencedUniformBlocks.count(interfaceBlock->uniqueId().get()) == 0)
                {
                    const TVariable *instanceVariable = nullptr;
                    if (variableType.isInterfaceBlock())
                    {
                        instanceVariable = &variable;
                    }
                    mReferencedUniformBlocks[interfaceBlock->uniqueId().get()] =
                        new TReferencedBlock(interfaceBlock, instanceVariable);
                }
            }
            else
            {
                mReferencedUniforms[uniqueId.get()] = &variable;
            }

            out << DecorateVariableIfNeeded(variable);
        }
        else if (qualifier == EvqBuffer)
        {
            UNREACHABLE();
        }
        else if (qualifier == EvqAttribute || qualifier == EvqVertexIn)
        {
            mReferencedAttributes[uniqueId.get()] = &variable;
            out << Decorate(name);
        }
        else if (IsVarying(qualifier))
        {
            mReferencedVaryings[uniqueId.get()] = &variable;
            out << DecorateVariableIfNeeded(variable);
        }
        else if (qualifier == EvqFragmentOut)
        {
            mReferencedOutputVariables[uniqueId.get()] = &variable;
            out << "out_" << name;
        }
        else if (qualifier == EvqViewIDOVR)
        {
            out << name;
            mUsesViewID = true;
        }
        else if (qualifier == EvqClipDistance)
        {
            out << name;
        }
        else if (qualifier == EvqCullDistance)
        {
            out << name;
        }
        else if (qualifier == EvqFragColor)
        {
            out << "gl_Color[0]";
            mUsesFragColor = true;
        }
        else if (qualifier == EvqFragData)
        {
            out << "gl_Color";
            mUsesFragData = true;
        }
        else if (qualifier == EvqSecondaryFragColorEXT)
        {
            out << "gl_SecondaryColor[0]";
            mUsesSecondaryColor = true;
        }
        else if (qualifier == EvqSecondaryFragDataEXT)
        {
            out << "gl_SecondaryColor";
            mUsesSecondaryColor = true;
        }
        else if (qualifier == EvqFragCoord)
        {
            mUsesFragCoord = true;
            out << name;
        }
        else if (qualifier == EvqPointCoord)
        {
            mUsesPointCoord = true;
            out << name;
        }
        else if (qualifier == EvqFrontFacing)
        {
            mUsesFrontFacing = true;
            out << name;
        }
        else if (qualifier == EvqHelperInvocation)
        {
            mUsesHelperInvocation = true;
            out << name;
        }
        else if (qualifier == EvqPointSize)
        {
            mUsesPointSize = true;
            out << name;
        }
        else if (qualifier == EvqInstanceID)
        {
            mUsesInstanceID = true;
            out << name;
        }
        else if (qualifier == EvqVertexID)
        {
            mUsesVertexID = true;
            out << name;
        }
        else if (name == "gl_FragDepthEXT" || name == "gl_FragDepth")
        {
            mUsesFragDepth = true;
            mDepthLayout   = variableType.getLayoutQualifier().depth;
            out << "gl_Depth";
        }
        else if (qualifier == EvqSampleID)
        {
            mUsesSampleID = true;
            out << name;
        }
        else if (qualifier == EvqSamplePosition)
        {
            mUsesSamplePosition = true;
            out << name;
        }
        else if (qualifier == EvqSampleMaskIn)
        {
            mUsesSampleMaskIn = true;
            out << name;
        }
        else if (qualifier == EvqSampleMask)
        {
            mUsesSampleMask = true;
            out << name;
        }
        else if (qualifier == EvqNumWorkGroups)
        {
            mUsesNumWorkGroups = true;
            out << name;
        }
        else if (qualifier == EvqWorkGroupID)
        {
            mUsesWorkGroupID = true;
            out << name;
        }
        else if (qualifier == EvqLocalInvocationID)
        {
            mUsesLocalInvocationID = true;
            out << name;
        }
        else if (qualifier == EvqGlobalInvocationID)
        {
            mUsesGlobalInvocationID = true;
            out << name;
        }
        else if (qualifier == EvqLocalInvocationIndex)
        {
            mUsesLocalInvocationIndex = true;
            out << name;
        }
        else
        {
            out << DecorateVariableIfNeeded(variable);
        }
    }
}

void OutputHLSL::outputEqual(Visit visit, const TType &type, TOperator op, TInfoSinkBase &out)
{
    if (type.isScalar() && !type.isArray())
    {
        if (op == EOpEqual)
        {
            outputTriplet(out, visit, "(", " == ", ")");
        }
        else
        {
            outputTriplet(out, visit, "(", " != ", ")");
        }
    }
    else
    {
        if (visit == PreVisit && op == EOpNotEqual)
        {
            out << "!";
        }

        if (type.isArray())
        {
            const TString &functionName = addArrayEqualityFunction(type);
            outputTriplet(out, visit, (functionName + "(").c_str(), ", ", ")");
        }
        else if (type.getBasicType() == EbtStruct)
        {
            const TStructure &structure = *type.getStruct();
            const TString &functionName = addStructEqualityFunction(structure);
            outputTriplet(out, visit, (functionName + "(").c_str(), ", ", ")");
        }
        else
        {
            ASSERT(type.isMatrix() || type.isVector());
            outputTriplet(out, visit, "all(", " == ", ")");
        }
    }
}

void OutputHLSL::outputAssign(Visit visit, const TType &type, TInfoSinkBase &out)
{
    if (type.isArray())
    {
        const TString &functionName = addArrayAssignmentFunction(type);
        outputTriplet(out, visit, (functionName + "(").c_str(), ", ", ")");
    }
    else
    {
        outputTriplet(out, visit, "(", " = ", ")");
    }
}

bool OutputHLSL::ancestorEvaluatesToSamplerInStruct()
{
    for (unsigned int n = 0u; getAncestorNode(n) != nullptr; ++n)
    {
        TIntermNode *ancestor               = getAncestorNode(n);
        const TIntermBinary *ancestorBinary = ancestor->getAsBinaryNode();
        if (ancestorBinary == nullptr)
        {
            return false;
        }
        switch (ancestorBinary->getOp())
        {
            case EOpIndexDirectStruct:
            {
                const TStructure *structure = ancestorBinary->getLeft()->getType().getStruct();
                const TIntermConstantUnion *index =
                    ancestorBinary->getRight()->getAsConstantUnion();
                const TField *field = structure->fields()[index->getIConst(0)];
                if (IsSampler(field->type()->getBasicType()))
                {
                    return true;
                }
                break;
            }
            case EOpIndexDirect:
                break;
            default:
                // Returning a sampler from indirect indexing is not supported.
                return false;
        }
    }
    return false;
}

bool OutputHLSL::visitSwizzle(Visit visit, TIntermSwizzle *node)
{
    TInfoSinkBase &out = getInfoSink();
    if (visit == PostVisit)
    {
        out << ".";
        node->writeOffsetsAsXYZW(&out);
    }
    return true;
}

bool OutputHLSL::visitBinary(Visit visit, TIntermBinary *node)
{
    TInfoSinkBase &out = getInfoSink();

    switch (node->getOp())
    {
        case EOpComma:
            outputTriplet(out, visit, "(", ", ", ")");
            break;
        case EOpAssign:
            if (node->isArray())
            {
                TIntermAggregate *rightAgg = node->getRight()->getAsAggregate();
                if (rightAgg != nullptr && rightAgg->isConstructor())
                {
                    const TString &functionName = addArrayConstructIntoFunction(node->getType());
                    out << functionName << "(";
                    node->getLeft()->traverse(this);
                    TIntermSequence *seq = rightAgg->getSequence();
                    for (auto &arrayElement : *seq)
                    {
                        out << ", ";
                        arrayElement->traverse(this);
                    }
                    out << ")";
                    return false;
                }
                // ArrayReturnValueToOutParameter should have eliminated expressions where a
                // function call is assigned.
                ASSERT(rightAgg == nullptr);
            }
            // Assignment expressions with atomic functions should be transformed into atomic
            // function calls in HLSL.
            // e.g. original_value = atomicAdd(dest, value) should be translated into
            //      InterlockedAdd(dest, value, original_value);
            else if (IsAtomicFunctionForSharedVariableDirectAssign(*node))
            {
                TIntermAggregate *atomicFunctionNode = node->getRight()->getAsAggregate();
                TOperator atomicFunctionOp           = atomicFunctionNode->getOp();
                out << GetHLSLAtomicFunctionStringAndLeftParenthesis(atomicFunctionOp);
                TIntermSequence *argumentSeq = atomicFunctionNode->getSequence();
                ASSERT(argumentSeq->size() >= 2u);
                for (auto &argument : *argumentSeq)
                {
                    argument->traverse(this);
                    out << ", ";
                }
                node->getLeft()->traverse(this);
                out << ")";
                return false;
            }
            else if (IsInShaderStorageBlock(node->getLeft()))
            {
                mSSBOOutputHLSL->outputStoreFunctionCallPrefix(node->getLeft());
                out << ", ";
                if (IsInShaderStorageBlock(node->getRight()))
                {
                    mSSBOOutputHLSL->outputLoadFunctionCall(node->getRight());
                }
                else
                {
                    node->getRight()->traverse(this);
                }

                out << ")";
                return false;
            }
            else if (IsInShaderStorageBlock(node->getRight()))
            {
                node->getLeft()->traverse(this);
                out << " = ";
                mSSBOOutputHLSL->outputLoadFunctionCall(node->getRight());
                return false;
            }

            outputAssign(visit, node->getType(), out);
            break;
        case EOpInitialize:
            if (visit == PreVisit)
            {
                TIntermSymbol *symbolNode = node->getLeft()->getAsSymbolNode();
                ASSERT(symbolNode);
                TIntermTyped *initializer = node->getRight();

                // Global initializers must be constant at this point.
                ASSERT(symbolNode->getQualifier() != EvqGlobal || initializer->hasConstantValue());

                // GLSL allows to write things like "float x = x;" where a new variable x is defined
                // and the value of an existing variable x is assigned. HLSL uses C semantics (the
                // new variable is created before the assignment is evaluated), so we need to
                // convert
                // this to "float t = x, x = t;".
                if (writeSameSymbolInitializer(out, symbolNode, initializer))
                {
                    // Skip initializing the rest of the expression
                    return false;
                }
                else if (writeConstantInitialization(out, symbolNode, initializer))
                {
                    return false;
                }
            }
            else if (visit == InVisit)
            {
                out << " = ";
                if (IsInShaderStorageBlock(node->getRight()))
                {
                    mSSBOOutputHLSL->outputLoadFunctionCall(node->getRight());
                    return false;
                }
            }
            break;
        case EOpAddAssign:
            outputTriplet(out, visit, "(", " += ", ")");
            break;
        case EOpSubAssign:
            outputTriplet(out, visit, "(", " -= ", ")");
            break;
        case EOpMulAssign:
            outputTriplet(out, visit, "(", " *= ", ")");
            break;
        case EOpVectorTimesScalarAssign:
            outputTriplet(out, visit, "(", " *= ", ")");
            break;
        case EOpMatrixTimesScalarAssign:
            outputTriplet(out, visit, "(", " *= ", ")");
            break;
        case EOpVectorTimesMatrixAssign:
            if (visit == PreVisit)
            {
                out << "(";
            }
            else if (visit == InVisit)
            {
                out << " = mul(";
                node->getLeft()->traverse(this);
                out << ", transpose(";
            }
            else
            {
                out << ")))";
            }
            break;
        case EOpMatrixTimesMatrixAssign:
            if (visit == PreVisit)
            {
                out << "(";
            }
            else if (visit == InVisit)
            {
                out << " = transpose(mul(transpose(";
                node->getLeft()->traverse(this);
                out << "), transpose(";
            }
            else
            {
                out << "))))";
            }
            break;
        case EOpDivAssign:
            outputTriplet(out, visit, "(", " /= ", ")");
            break;
        case EOpIModAssign:
            outputTriplet(out, visit, "(", " %= ", ")");
            break;
        case EOpBitShiftLeftAssign:
            outputTriplet(out, visit, "(", " <<= ", ")");
            break;
        case EOpBitShiftRightAssign:
            outputTriplet(out, visit, "(", " >>= ", ")");
            break;
        case EOpBitwiseAndAssign:
            outputTriplet(out, visit, "(", " &= ", ")");
            break;
        case EOpBitwiseXorAssign:
            outputTriplet(out, visit, "(", " ^= ", ")");
            break;
        case EOpBitwiseOrAssign:
            outputTriplet(out, visit, "(", " |= ", ")");
            break;
        case EOpIndexDirect:
        {
            const TType &leftType = node->getLeft()->getType();
            if (leftType.isInterfaceBlock())
            {
                if (visit == PreVisit)
                {
                    TIntermSymbol *instanceArraySymbol    = node->getLeft()->getAsSymbolNode();
                    const TInterfaceBlock *interfaceBlock = leftType.getInterfaceBlock();

                    ASSERT(leftType.getQualifier() == EvqUniform);
                    if (mReferencedUniformBlocks.count(interfaceBlock->uniqueId().get()) == 0)
                    {
                        mReferencedUniformBlocks[interfaceBlock->uniqueId().get()] =
                            new TReferencedBlock(interfaceBlock, &instanceArraySymbol->variable());
                    }
                    const int arrayIndex = node->getRight()->getAsConstantUnion()->getIConst(0);
                    out << mResourcesHLSL->InterfaceBlockInstanceString(
                        instanceArraySymbol->getName(), arrayIndex);
                    return false;
                }
            }
            else if (ancestorEvaluatesToSamplerInStruct())
            {
                // All parts of an expression that access a sampler in a struct need to use _ as
                // separator to access the sampler variable that has been moved out of the struct.
                outputTriplet(out, visit, "", "_", "");
            }
            else if (IsAtomicCounter(leftType.getBasicType()))
            {
                outputTriplet(out, visit, "", " + (", ") * ATOMIC_COUNTER_ARRAY_STRIDE");
            }
            else
            {
                outputTriplet(out, visit, "", "[", "]");
                if (visit == PostVisit)
                {
                    const TInterfaceBlock *interfaceBlock =
                        GetInterfaceBlockOfUniformBlockNearestIndexOperator(node->getLeft());
                    if (interfaceBlock &&
                        mUniformBlockOptimizedMap.count(interfaceBlock->uniqueId().get()) != 0)
                    {
                        // If the uniform block member's type is not structure, we had explicitly
                        // packed the member into a structure, so need to add an operator of field
                        // slection.
                        const TField *field    = interfaceBlock->fields()[0];
                        const TType *fieldType = field->type();
                        if (fieldType->isMatrix() || fieldType->isVectorArray() ||
                            fieldType->isScalarArray())
                        {
                            out << "." << Decorate(field->name());
                        }
                    }
                }
            }
        }
        break;
        case EOpIndexIndirect:
        {
            // We do not currently support indirect references to interface blocks
            ASSERT(node->getLeft()->getBasicType() != EbtInterfaceBlock);

            const TType &leftType = node->getLeft()->getType();
            if (IsAtomicCounter(leftType.getBasicType()))
            {
                outputTriplet(out, visit, "", " + (", ") * ATOMIC_COUNTER_ARRAY_STRIDE");
            }
            else
            {
                outputTriplet(out, visit, "", "[", "]");
                if (visit == PostVisit)
                {
                    const TInterfaceBlock *interfaceBlock =
                        GetInterfaceBlockOfUniformBlockNearestIndexOperator(node->getLeft());
                    if (interfaceBlock &&
                        mUniformBlockOptimizedMap.count(interfaceBlock->uniqueId().get()) != 0)
                    {
                        // If the uniform block member's type is not structure, we had explicitly
                        // packed the member into a structure, so need to add an operator of field
                        // slection.
                        const TField *field    = interfaceBlock->fields()[0];
                        const TType *fieldType = field->type();
                        if (fieldType->isMatrix() || fieldType->isVectorArray() ||
                            fieldType->isScalarArray())
                        {
                            out << "." << Decorate(field->name());
                        }
                    }
                }
            }
            break;
        }
        case EOpIndexDirectStruct:
        {
            const TStructure *structure       = node->getLeft()->getType().getStruct();
            const TIntermConstantUnion *index = node->getRight()->getAsConstantUnion();
            const TField *field               = structure->fields()[index->getIConst(0)];

            // In cases where indexing returns a sampler, we need to access the sampler variable
            // that has been moved out of the struct.
            bool indexingReturnsSampler = IsSampler(field->type()->getBasicType());
            if (visit == PreVisit && indexingReturnsSampler)
            {
                // Samplers extracted from structs have "angle" prefix to avoid name conflicts.
                // This prefix is only output at the beginning of the indexing expression, which
                // may have multiple parts.
                out << "angle";
            }
            if (!indexingReturnsSampler)
            {
                // All parts of an expression that access a sampler in a struct need to use _ as
                // separator to access the sampler variable that has been moved out of the struct.
                indexingReturnsSampler = ancestorEvaluatesToSamplerInStruct();
            }
            if (visit == InVisit)
            {
                if (indexingReturnsSampler)
                {
                    out << "_" << field->name();
                }
                else
                {
                    out << "." << DecorateField(field->name(), *structure);
                }

                return false;
            }
        }
        break;
        case EOpIndexDirectInterfaceBlock:
        {
            ASSERT(!IsInShaderStorageBlock(node->getLeft()));
            bool structInStd140UniformBlock = node->getBasicType() == EbtStruct &&
                                              IsInStd140UniformBlock(node->getLeft()) &&
                                              needStructMapping(node);
            if (visit == PreVisit && structInStd140UniformBlock)
            {
                mNeedStructMapping = true;
                out << "map";
            }
            if (visit == InVisit)
            {
                const TInterfaceBlock *interfaceBlock =
                    node->getLeft()->getType().getInterfaceBlock();
                const TIntermConstantUnion *index = node->getRight()->getAsConstantUnion();
                const TField *field               = interfaceBlock->fields()[index->getIConst(0)];
                if (structInStd140UniformBlock ||
                    mUniformBlockOptimizedMap.count(interfaceBlock->uniqueId().get()) != 0)
                {
                    out << "_";
                }
                else
                {
                    out << ".";
                }
                out << Decorate(field->name());

                return false;
            }
            break;
        }
        case EOpAdd:
            outputTriplet(out, visit, "(", " + ", ")");
            break;
        case EOpSub:
            outputTriplet(out, visit, "(", " - ", ")");
            break;
        case EOpMul:
            outputTriplet(out, visit, "(", " * ", ")");
            break;
        case EOpDiv:
            outputTriplet(out, visit, "(", " / ", ")");
            break;
        case EOpIMod:
            outputTriplet(out, visit, "(", " % ", ")");
            break;
        case EOpBitShiftLeft:
            outputTriplet(out, visit, "(", " << ", ")");
            break;
        case EOpBitShiftRight:
            outputTriplet(out, visit, "(", " >> ", ")");
            break;
        case EOpBitwiseAnd:
            outputTriplet(out, visit, "(", " & ", ")");
            break;
        case EOpBitwiseXor:
            outputTriplet(out, visit, "(", " ^ ", ")");
            break;
        case EOpBitwiseOr:
            outputTriplet(out, visit, "(", " | ", ")");
            break;
        case EOpEqual:
        case EOpNotEqual:
            outputEqual(visit, node->getLeft()->getType(), node->getOp(), out);
            break;
        case EOpLessThan:
            outputTriplet(out, visit, "(", " < ", ")");
            break;
        case EOpGreaterThan:
            outputTriplet(out, visit, "(", " > ", ")");
            break;
        case EOpLessThanEqual:
            outputTriplet(out, visit, "(", " <= ", ")");
            break;
        case EOpGreaterThanEqual:
            outputTriplet(out, visit, "(", " >= ", ")");
            break;
        case EOpVectorTimesScalar:
            outputTriplet(out, visit, "(", " * ", ")");
            break;
        case EOpMatrixTimesScalar:
            outputTriplet(out, visit, "(", " * ", ")");
            break;
        case EOpVectorTimesMatrix:
            outputTriplet(out, visit, "mul(", ", transpose(", "))");
            break;
        case EOpMatrixTimesVector:
            outputTriplet(out, visit, "mul(transpose(", "), ", ")");
            break;
        case EOpMatrixTimesMatrix:
            outputTriplet(out, visit, "transpose(mul(transpose(", "), transpose(", ")))");
            break;
        case EOpLogicalOr:
            // HLSL doesn't short-circuit ||, so we assume that || affected by short-circuiting have
            // been unfolded.
            ASSERT(!node->getRight()->hasSideEffects());
            outputTriplet(out, visit, "(", " || ", ")");
            return true;
        case EOpLogicalXor:
            mUsesXor = true;
            outputTriplet(out, visit, "xor(", ", ", ")");
            break;
        case EOpLogicalAnd:
            // HLSL doesn't short-circuit &&, so we assume that && affected by short-circuiting have
            // been unfolded.
            ASSERT(!node->getRight()->hasSideEffects());
            outputTriplet(out, visit, "(", " && ", ")");
            return true;
        default:
            UNREACHABLE();
    }

    return true;
}

bool OutputHLSL::visitUnary(Visit visit, TIntermUnary *node)
{
    TInfoSinkBase &out = getInfoSink();

    switch (node->getOp())
    {
        case EOpNegative:
            outputTriplet(out, visit, "(-", "", ")");
            break;
        case EOpPositive:
            outputTriplet(out, visit, "(+", "", ")");
            break;
        case EOpLogicalNot:
            outputTriplet(out, visit, "(!", "", ")");
            break;
        case EOpBitwiseNot:
            outputTriplet(out, visit, "(~", "", ")");
            break;
        case EOpPostIncrement:
            outputTriplet(out, visit, "(", "", "++)");
            break;
        case EOpPostDecrement:
            outputTriplet(out, visit, "(", "", "--)");
            break;
        case EOpPreIncrement:
            outputTriplet(out, visit, "(++", "", ")");
            break;
        case EOpPreDecrement:
            outputTriplet(out, visit, "(--", "", ")");
            break;
        case EOpRadians:
            outputTriplet(out, visit, "radians(", "", ")");
            break;
        case EOpDegrees:
            outputTriplet(out, visit, "degrees(", "", ")");
            break;
        case EOpSin:
            outputTriplet(out, visit, "sin(", "", ")");
            break;
        case EOpCos:
            outputTriplet(out, visit, "cos(", "", ")");
            break;
        case EOpTan:
            outputTriplet(out, visit, "tan(", "", ")");
            break;
        case EOpAsin:
            outputTriplet(out, visit, "asin(", "", ")");
            break;
        case EOpAcos:
            outputTriplet(out, visit, "acos(", "", ")");
            break;
        case EOpAtan:
            outputTriplet(out, visit, "atan(", "", ")");
            break;
        case EOpSinh:
            outputTriplet(out, visit, "sinh(", "", ")");
            break;
        case EOpCosh:
            outputTriplet(out, visit, "cosh(", "", ")");
            break;
        case EOpTanh:
        case EOpAsinh:
        case EOpAcosh:
        case EOpAtanh:
            ASSERT(node->getUseEmulatedFunction());
            writeEmulatedFunctionTriplet(out, visit, node->getFunction());
            break;
        case EOpExp:
            outputTriplet(out, visit, "exp(", "", ")");
            break;
        case EOpLog:
            outputTriplet(out, visit, "log(", "", ")");
            break;
        case EOpExp2:
            outputTriplet(out, visit, "exp2(", "", ")");
            break;
        case EOpLog2:
            outputTriplet(out, visit, "log2(", "", ")");
            break;
        case EOpSqrt:
            outputTriplet(out, visit, "sqrt(", "", ")");
            break;
        case EOpInversesqrt:
            outputTriplet(out, visit, "rsqrt(", "", ")");
            break;
        case EOpAbs:
            outputTriplet(out, visit, "abs(", "", ")");
            break;
        case EOpSign:
            outputTriplet(out, visit, "sign(", "", ")");
            break;
        case EOpFloor:
            outputTriplet(out, visit, "floor(", "", ")");
            break;
        case EOpTrunc:
            outputTriplet(out, visit, "trunc(", "", ")");
            break;
        case EOpRound:
            outputTriplet(out, visit, "round(", "", ")");
            break;
        case EOpRoundEven:
            ASSERT(node->getUseEmulatedFunction());
            writeEmulatedFunctionTriplet(out, visit, node->getFunction());
            break;
        case EOpCeil:
            outputTriplet(out, visit, "ceil(", "", ")");
            break;
        case EOpFract:
            outputTriplet(out, visit, "frac(", "", ")");
            break;
        case EOpIsnan:
            if (node->getUseEmulatedFunction())
                writeEmulatedFunctionTriplet(out, visit, node->getFunction());
            else
                outputTriplet(out, visit, "isnan(", "", ")");
            mRequiresIEEEStrictCompiling = true;
            break;
        case EOpIsinf:
            outputTriplet(out, visit, "isinf(", "", ")");
            break;
        case EOpFloatBitsToInt:
            outputTriplet(out, visit, "asint(", "", ")");
            break;
        case EOpFloatBitsToUint:
            outputTriplet(out, visit, "asuint(", "", ")");
            break;
        case EOpIntBitsToFloat:
            outputTriplet(out, visit, "asfloat(", "", ")");
            break;
        case EOpUintBitsToFloat:
            outputTriplet(out, visit, "asfloat(", "", ")");
            break;
        case EOpPackSnorm2x16:
        case EOpPackUnorm2x16:
        case EOpPackHalf2x16:
        case EOpUnpackSnorm2x16:
        case EOpUnpackUnorm2x16:
        case EOpUnpackHalf2x16:
        case EOpPackUnorm4x8:
        case EOpPackSnorm4x8:
        case EOpUnpackUnorm4x8:
        case EOpUnpackSnorm4x8:
            ASSERT(node->getUseEmulatedFunction());
            writeEmulatedFunctionTriplet(out, visit, node->getFunction());
            break;
        case EOpLength:
            outputTriplet(out, visit, "length(", "", ")");
            break;
        case EOpNormalize:
            outputTriplet(out, visit, "normalize(", "", ")");
            break;
        case EOpTranspose:
            outputTriplet(out, visit, "transpose(", "", ")");
            break;
        case EOpDeterminant:
            outputTriplet(out, visit, "determinant(transpose(", "", "))");
            break;
        case EOpInverse:
            ASSERT(node->getUseEmulatedFunction());
            writeEmulatedFunctionTriplet(out, visit, node->getFunction());
            break;

        case EOpAny:
            outputTriplet(out, visit, "any(", "", ")");
            break;
        case EOpAll:
            outputTriplet(out, visit, "all(", "", ")");
            break;
        case EOpNotComponentWise:
            outputTriplet(out, visit, "(!", "", ")");
            break;
        case EOpBitfieldReverse:
            outputTriplet(out, visit, "reversebits(", "", ")");
            break;
        case EOpBitCount:
            outputTriplet(out, visit, "countbits(", "", ")");
            break;
        case EOpFindLSB:
            // Note that it's unclear from the HLSL docs what this returns for 0, but this is tested
            // in GLSLTest and results are consistent with GL.
            outputTriplet(out, visit, "firstbitlow(", "", ")");
            break;
        case EOpFindMSB:
            // Note that it's unclear from the HLSL docs what this returns for 0 or -1, but this is
            // tested in GLSLTest and results are consistent with GL.
            outputTriplet(out, visit, "firstbithigh(", "", ")");
            break;
        case EOpArrayLength:
        {
            TIntermTyped *operand = node->getOperand();
            ASSERT(IsInShaderStorageBlock(operand));
            mSSBOOutputHLSL->outputLengthFunctionCall(operand);
            return false;
        }
        default:
            UNREACHABLE();
    }

    return true;
}

ImmutableString OutputHLSL::samplerNamePrefixFromStruct(TIntermTyped *node)
{
    if (node->getAsSymbolNode())
    {
        ASSERT(node->getAsSymbolNode()->variable().symbolType() != SymbolType::Empty);
        return node->getAsSymbolNode()->getName();
    }
    TIntermBinary *nodeBinary = node->getAsBinaryNode();
    switch (nodeBinary->getOp())
    {
        case EOpIndexDirect:
        {
            int index = nodeBinary->getRight()->getAsConstantUnion()->getIConst(0);

            std::stringstream prefixSink = sh::InitializeStream<std::stringstream>();
            prefixSink << samplerNamePrefixFromStruct(nodeBinary->getLeft()) << "_" << index;
            return ImmutableString(prefixSink.str());
        }
        case EOpIndexDirectStruct:
        {
            const TStructure *s = nodeBinary->getLeft()->getAsTyped()->getType().getStruct();
            int index           = nodeBinary->getRight()->getAsConstantUnion()->getIConst(0);
            const TField *field = s->fields()[index];

            std::stringstream prefixSink = sh::InitializeStream<std::stringstream>();
            prefixSink << samplerNamePrefixFromStruct(nodeBinary->getLeft()) << "_"
                       << field->name();
            return ImmutableString(prefixSink.str());
        }
        default:
            UNREACHABLE();
            return kEmptyImmutableString;
    }
}

bool OutputHLSL::visitBlock(Visit visit, TIntermBlock *node)
{
    TInfoSinkBase &out = getInfoSink();

    bool isMainBlock = mInsideMain && getParentNode()->getAsFunctionDefinition();

    if (mInsideFunction)
    {
        outputLineDirective(out, node->getLine().first_line);
        out << "{\n";
        if (isMainBlock)
        {
            if (mShaderType == GL_COMPUTE_SHADER)
            {
                out << "initGLBuiltins(input);\n";
            }
            else
            {
                out << "@@ MAIN PROLOGUE @@\n";
            }
        }
    }

    for (TIntermNode *statement : *node->getSequence())
    {
        outputLineDirective(out, statement->getLine().first_line);

        statement->traverse(this);

        // Don't output ; after case labels, they're terminated by :
        // This is needed especially since outputting a ; after a case statement would turn empty
        // case statements into non-empty case statements, disallowing fall-through from them.
        // Also the output code is clearer if we don't output ; after statements where it is not
        // needed:
        //  * if statements
        //  * switch statements
        //  * blocks
        //  * function definitions
        //  * loops (do-while loops output the semicolon in VisitLoop)
        //  * declarations that don't generate output.
        if (statement->getAsCaseNode() == nullptr && statement->getAsIfElseNode() == nullptr &&
            statement->getAsBlock() == nullptr && statement->getAsLoopNode() == nullptr &&
            statement->getAsSwitchNode() == nullptr &&
            statement->getAsFunctionDefinition() == nullptr &&
            (statement->getAsDeclarationNode() == nullptr ||
             IsDeclarationWrittenOut(statement->getAsDeclarationNode())) &&
            statement->getAsGlobalQualifierDeclarationNode() == nullptr)
        {
            out << ";\n";
        }
    }

    if (mInsideFunction)
    {
        outputLineDirective(out, node->getLine().last_line);
        if (isMainBlock && shaderNeedsGenerateOutput())
        {
            // We could have an empty main, a main function without a branch at the end, or a main
            // function with a discard statement at the end. In these cases we need to add a return
            // statement.
            bool needReturnStatement =
                node->getSequence()->empty() || !node->getSequence()->back()->getAsBranchNode() ||
                node->getSequence()->back()->getAsBranchNode()->getFlowOp() != EOpReturn;
            if (needReturnStatement)
            {
                out << "return " << generateOutputCall() << ";\n";
            }
        }
        out << "}\n";
    }

    return false;
}

bool OutputHLSL::visitFunctionDefinition(Visit visit, TIntermFunctionDefinition *node)
{
    TInfoSinkBase &out = getInfoSink();

    ASSERT(mCurrentFunctionMetadata == nullptr);

    size_t index = mCallDag.findIndex(node->getFunction()->uniqueId());
    ASSERT(index != CallDAG::InvalidIndex);
    mCurrentFunctionMetadata = &mASTMetadataList[index];

    const TFunction *func = node->getFunction();

    if (func->isMain())
    {
        // The stub strings below are replaced when shader is dynamically defined by its layout:
        switch (mShaderType)
        {
            case GL_VERTEX_SHADER:
                out << "@@ VERTEX ATTRIBUTES @@\n\n"
                    << "@@ VERTEX OUTPUT @@\n\n"
                    << "VS_OUTPUT main(VS_INPUT input)";
                break;
            case GL_FRAGMENT_SHADER:
                out << "@@ PIXEL OUTPUT @@\n\n";
                if (mIsEarlyFragmentTestsSpecified)
                {
                    out << "[earlydepthstencil]\n";
                }
                out << "PS_OUTPUT main(@@ PIXEL MAIN PARAMETERS @@)";
                break;
            case GL_COMPUTE_SHADER:
                out << "[numthreads(" << mWorkGroupSize[0] << ", " << mWorkGroupSize[1] << ", "
                    << mWorkGroupSize[2] << ")]\n";
                out << "void main(CS_INPUT input)";
                break;
            default:
                UNREACHABLE();
                break;
        }
    }
    else
    {
        out << TypeString(node->getFunctionPrototype()->getType()) << " ";
        out << DecorateFunctionIfNeeded(func) << DisambiguateFunctionName(func)
            << (mOutputLod0Function ? "Lod0(" : "(");

        size_t paramCount = func->getParamCount();
        for (unsigned int i = 0; i < paramCount; i++)
        {
            const TVariable *param = func->getParam(i);
            ensureStructDefined(param->getType());

            writeParameter(param, out);

            if (i < paramCount - 1)
            {
                out << ", ";
            }
        }

        out << ")\n";
    }

    mInsideFunction = true;
    if (func->isMain())
    {
        mInsideMain = true;
    }
    // The function body node will output braces.
    node->getBody()->traverse(this);
    mInsideFunction = false;
    mInsideMain     = false;

    mCurrentFunctionMetadata = nullptr;

    bool needsLod0 = mASTMetadataList[index].mNeedsLod0;
    if (needsLod0 && !mOutputLod0Function && mShaderType == GL_FRAGMENT_SHADER)
    {
        ASSERT(!node->getFunction()->isMain());
        mOutputLod0Function = true;
        node->traverse(this);
        mOutputLod0Function = false;
    }

    return false;
}

bool OutputHLSL::visitDeclaration(Visit visit, TIntermDeclaration *node)
{
    if (visit == PreVisit)
    {
        TIntermSequence *sequence = node->getSequence();
        TIntermTyped *declarator  = (*sequence)[0]->getAsTyped();
        ASSERT(sequence->size() == 1);
        ASSERT(declarator);

        if (IsDeclarationWrittenOut(node))
        {
            TInfoSinkBase &out = getInfoSink();
            ensureStructDefined(declarator->getType());

            if (!declarator->getAsSymbolNode() ||
                declarator->getAsSymbolNode()->variable().symbolType() !=
                    SymbolType::Empty)  // Variable declaration
            {
                if (declarator->getQualifier() == EvqShared)
                {
                    out << "groupshared ";
                }
                else if (!mInsideFunction)
                {
                    out << "static ";
                }

                out << TypeString(declarator->getType()) + " ";

                TIntermSymbol *symbol = declarator->getAsSymbolNode();

                if (symbol)
                {
                    symbol->traverse(this);
                    out << ArrayString(symbol->getType());
                    // Temporarily disable shadred memory initialization. It is very slow for D3D11
                    // drivers to compile a compute shader if we add code to initialize a
                    // groupshared array variable with a large array size. And maybe produce
                    // incorrect result. See http://anglebug.com/40644676.
                    if (declarator->getQualifier() != EvqShared)
                    {
                        out << " = " + zeroInitializer(symbol->getType());
                    }
                }
                else
                {
                    declarator->traverse(this);
                }
            }
        }
        else if (IsVaryingOut(declarator->getQualifier()))
        {
            TIntermSymbol *symbol = declarator->getAsSymbolNode();
            ASSERT(symbol);  // Varying declarations can't have initializers.

            const TVariable &variable = symbol->variable();

            if (variable.symbolType() != SymbolType::Empty)
            {
                // Vertex outputs which are declared but not written to should still be declared to
                // allow successful linking.
                mReferencedVaryings[symbol->uniqueId().get()] = &variable;
            }
        }
    }
    return false;
}

bool OutputHLSL::visitGlobalQualifierDeclaration(Visit visit,
                                                 TIntermGlobalQualifierDeclaration *node)
{
    // Do not do any translation
    return false;
}

void OutputHLSL::visitFunctionPrototype(TIntermFunctionPrototype *node)
{
    TInfoSinkBase &out = getInfoSink();

    size_t index = mCallDag.findIndex(node->getFunction()->uniqueId());
    // Skip the prototype if it is not implemented (and thus not used)
    if (index == CallDAG::InvalidIndex)
    {
        return;
    }

    const TFunction *func = node->getFunction();

    TString name = DecorateFunctionIfNeeded(func);
    out << TypeString(node->getType()) << " " << name << DisambiguateFunctionName(func)
        << (mOutputLod0Function ? "Lod0(" : "(");

    size_t paramCount = func->getParamCount();
    for (unsigned int i = 0; i < paramCount; i++)
    {
        writeParameter(func->getParam(i), out);

        if (i < paramCount - 1)
        {
            out << ", ";
        }
    }

    out << ");\n";

    // Also prototype the Lod0 variant if needed
    bool needsLod0 = mASTMetadataList[index].mNeedsLod0;
    if (needsLod0 && !mOutputLod0Function && mShaderType == GL_FRAGMENT_SHADER)
    {
        mOutputLod0Function = true;
        node->traverse(this);
        mOutputLod0Function = false;
    }
}

bool OutputHLSL::visitAggregate(Visit visit, TIntermAggregate *node)
{
    TInfoSinkBase &out = getInfoSink();

    switch (node->getOp())
    {
        case EOpCallFunctionInAST:
        case EOpCallInternalRawFunction:
        default:
        {
            TIntermSequence *arguments = node->getSequence();

            bool lod0 = (mInsideDiscontinuousLoop || mOutputLod0Function) &&
                        mShaderType == GL_FRAGMENT_SHADER;

            // No raw function is expected.
            ASSERT(node->getOp() != EOpCallInternalRawFunction);

            if (node->getOp() == EOpCallFunctionInAST)
            {
                if (node->isArray())
                {
                    UNIMPLEMENTED();
                }
                size_t index = mCallDag.findIndex(node->getFunction()->uniqueId());
                ASSERT(index != CallDAG::InvalidIndex);
                lod0 &= mASTMetadataList[index].mNeedsLod0;

                out << DecorateFunctionIfNeeded(node->getFunction());
                out << DisambiguateFunctionName(node->getSequence());
                out << (lod0 ? "Lod0(" : "(");
            }
            else if (node->getFunction()->isImageFunction())
            {
                const ImmutableString &name              = node->getFunction()->name();
                TType type                               = (*arguments)[0]->getAsTyped()->getType();
                const ImmutableString &imageFunctionName = mImageFunctionHLSL->useImageFunction(
                    name, type.getBasicType(), type.getLayoutQualifier().imageInternalFormat,
                    type.getMemoryQualifier().readonly);
                out << imageFunctionName << "(";
            }
            else if (node->getFunction()->isAtomicCounterFunction())
            {
                const ImmutableString &name = node->getFunction()->name();
                ImmutableString atomicFunctionName =
                    mAtomicCounterFunctionHLSL->useAtomicCounterFunction(name);
                out << atomicFunctionName << "(";
            }
            else
            {
                const ImmutableString &name = node->getFunction()->name();
                TBasicType samplerType = (*arguments)[0]->getAsTyped()->getType().getBasicType();
                int coords = 0;  // textureSize(gsampler2DMS) doesn't have a second argument.
                if (arguments->size() > 1)
                {
                    coords = (*arguments)[1]->getAsTyped()->getNominalSize();
                }
                const ImmutableString &textureFunctionName =
                    mTextureFunctionHLSL->useTextureFunction(name, samplerType, coords,
                                                             arguments->size(), lod0, mShaderType);
                out << textureFunctionName << "(";
            }

            for (TIntermSequence::iterator arg = arguments->begin(); arg != arguments->end(); arg++)
            {
                TIntermTyped *typedArg = (*arg)->getAsTyped();

                (*arg)->traverse(this);

                if (typedArg->getType().isStructureContainingSamplers())
                {
                    const TType &argType = typedArg->getType();
                    TVector<const TVariable *> samplerSymbols;
                    ImmutableString structName = samplerNamePrefixFromStruct(typedArg);
                    std::string namePrefix     = "angle_";
                    namePrefix += structName.data();
                    argType.createSamplerSymbols(ImmutableString(namePrefix), "", &samplerSymbols,
                                                 nullptr, mSymbolTable);
                    for (const TVariable *sampler : samplerSymbols)
                    {
                        // In case of HLSL 4.1+, this symbol is the sampler index, and in case
                        // of D3D9, it's the sampler variable.
                        out << ", " << sampler->name();
                    }
                }

                if (arg < arguments->end() - 1)
                {
                    out << ", ";
                }
            }

            out << ")";

            return false;
        }
        case EOpConstruct:
            outputConstructor(out, visit, node);
            break;
        case EOpEqualComponentWise:
            outputTriplet(out, visit, "(", " == ", ")");
            break;
        case EOpNotEqualComponentWise:
            outputTriplet(out, visit, "(", " != ", ")");
            break;
        case EOpLessThanComponentWise:
            outputTriplet(out, visit, "(", " < ", ")");
            break;
        case EOpGreaterThanComponentWise:
            outputTriplet(out, visit, "(", " > ", ")");
            break;
        case EOpLessThanEqualComponentWise:
            outputTriplet(out, visit, "(", " <= ", ")");
            break;
        case EOpGreaterThanEqualComponentWise:
            outputTriplet(out, visit, "(", " >= ", ")");
            break;
        case EOpMod:
            ASSERT(node->getUseEmulatedFunction());
            writeEmulatedFunctionTriplet(out, visit, node->getFunction());
            break;
        case EOpModf:
            outputTriplet(out, visit, "modf(", ", ", ")");
            break;
        case EOpPow:
            outputTriplet(out, visit, "pow(", ", ", ")");
            break;
        case EOpAtan:
            ASSERT(node->getSequence()->size() == 2);  // atan(x) is a unary operator
            ASSERT(node->getUseEmulatedFunction());
            writeEmulatedFunctionTriplet(out, visit, node->getFunction());
            break;
        case EOpMin:
            outputTriplet(out, visit, "min(", ", ", ")");
            break;
        case EOpMax:
            outputTriplet(out, visit, "max(", ", ", ")");
            break;
        case EOpClamp:
            outputTriplet(out, visit, "clamp(", ", ", ")");
            break;
        case EOpMix:
        {
            TIntermTyped *lastParamNode = (*(node->getSequence()))[2]->getAsTyped();
            if (lastParamNode->getType().getBasicType() == EbtBool)
            {
                // There is no HLSL equivalent for ESSL3 built-in "genType mix (genType x, genType
                // y, genBType a)",
                // so use emulated version.
                ASSERT(node->getUseEmulatedFunction());
                writeEmulatedFunctionTriplet(out, visit, node->getFunction());
            }
            else
            {
                outputTriplet(out, visit, "lerp(", ", ", ")");
            }
            break;
        }
        case EOpStep:
            outputTriplet(out, visit, "step(", ", ", ")");
            break;
        case EOpSmoothstep:
            outputTriplet(out, visit, "smoothstep(", ", ", ")");
            break;
        case EOpFma:
            outputTriplet(out, visit, "mad(", ", ", ")");
            break;
        case EOpFrexp:
        case EOpLdexp:
            ASSERT(node->getUseEmulatedFunction());
            writeEmulatedFunctionTriplet(out, visit, node->getFunction());
            break;
        case EOpDistance:
            outputTriplet(out, visit, "distance(", ", ", ")");
            break;
        case EOpDot:
            outputTriplet(out, visit, "dot(", ", ", ")");
            break;
        case EOpCross:
            outputTriplet(out, visit, "cross(", ", ", ")");
            break;
        case EOpFaceforward:
            ASSERT(node->getUseEmulatedFunction());
            writeEmulatedFunctionTriplet(out, visit, node->getFunction());
            break;
        case EOpReflect:
            outputTriplet(out, visit, "reflect(", ", ", ")");
            break;
        case EOpRefract:
            outputTriplet(out, visit, "refract(", ", ", ")");
            break;
        case EOpOuterProduct:
            ASSERT(node->getUseEmulatedFunction());
            writeEmulatedFunctionTriplet(out, visit, node->getFunction());
            break;
        case EOpMatrixCompMult:
            outputTriplet(out, visit, "(", " * ", ")");
            break;
        case EOpBitfieldExtract:
        case EOpBitfieldInsert:
        case EOpUaddCarry:
        case EOpUsubBorrow:
        case EOpUmulExtended:
        case EOpImulExtended:
            ASSERT(node->getUseEmulatedFunction());
            writeEmulatedFunctionTriplet(out, visit, node->getFunction());
            break;
        case EOpDFdx:
            if (mInsideDiscontinuousLoop || mOutputLod0Function)
            {
                outputTriplet(out, visit, "(", "", ", 0.0)");
            }
            else
            {
                outputTriplet(out, visit, "ddx(", "", ")");
            }
            break;
        case EOpDFdy:
            if (mInsideDiscontinuousLoop || mOutputLod0Function)
            {
                outputTriplet(out, visit, "(", "", ", 0.0)");
            }
            else
            {
                outputTriplet(out, visit, "ddy(", "", ")");
            }
            break;
        case EOpFwidth:
            if (mInsideDiscontinuousLoop || mOutputLod0Function)
            {
                outputTriplet(out, visit, "(", "", ", 0.0)");
            }
            else
            {
                outputTriplet(out, visit, "fwidth(", "", ")");
            }
            break;
        case EOpInterpolateAtCentroid:
        {
            TIntermTyped *interpolantNode = (*(node->getSequence()))[0]->getAsTyped();
            if (!IsFlatInterpolant(interpolantNode))
            {
                outputTriplet(out, visit, "EvaluateAttributeCentroid(", "", ")");
            }
            break;
        }
        case EOpInterpolateAtSample:
        {
            TIntermTyped *interpolantNode = (*(node->getSequence()))[0]->getAsTyped();
            if (!IsFlatInterpolant(interpolantNode))
            {
                mUsesNumSamples = true;
                outputTriplet(out, visit, "EvaluateAttributeAtSample(", ", clamp(",
                              ", 0, gl_NumSamples - 1))");
            }
            else
            {
                const TString &functionName = addFlatEvaluateFunction(
                    interpolantNode->getType(), *StaticType::GetBasic<EbtInt, EbpUndefined, 1>());
                outputTriplet(out, visit, (functionName + "(").c_str(), ", ", ")");
            }
            break;
        }
        case EOpInterpolateAtOffset:
        {
            TIntermTyped *interpolantNode = (*(node->getSequence()))[0]->getAsTyped();
            if (!IsFlatInterpolant(interpolantNode))
            {
                outputTriplet(out, visit, "EvaluateAttributeSnapped(", ", int2((", ") * 16.0))");
            }
            else
            {
                const TString &functionName = addFlatEvaluateFunction(
                    interpolantNode->getType(), *StaticType::GetBasic<EbtFloat, EbpUndefined, 2>());
                outputTriplet(out, visit, (functionName + "(").c_str(), ", ", ")");
            }
            break;
        }
        case EOpBarrier:
            // barrier() is translated to GroupMemoryBarrierWithGroupSync(), which is the
            // cheapest *WithGroupSync() function, without any functionality loss, but
            // with the potential for severe performance loss.
            outputTriplet(out, visit, "GroupMemoryBarrierWithGroupSync(", "", ")");
            break;
        case EOpMemoryBarrierShared:
            outputTriplet(out, visit, "GroupMemoryBarrier(", "", ")");
            break;
        case EOpMemoryBarrierAtomicCounter:
        case EOpMemoryBarrierBuffer:
        case EOpMemoryBarrierImage:
            outputTriplet(out, visit, "DeviceMemoryBarrier(", "", ")");
            break;
        case EOpGroupMemoryBarrier:
        case EOpMemoryBarrier:
            outputTriplet(out, visit, "AllMemoryBarrier(", "", ")");
            break;

        // Single atomic function calls without return value.
        // e.g. atomicAdd(dest, value) should be translated into InterlockedAdd(dest, value).
        case EOpAtomicAdd:
        case EOpAtomicMin:
        case EOpAtomicMax:
        case EOpAtomicAnd:
        case EOpAtomicOr:
        case EOpAtomicXor:
        // The parameter 'original_value' of InterlockedExchange(dest, value, original_value)
        // and InterlockedCompareExchange(dest, compare_value, value, original_value) is not
        // optional.
        // https://docs.microsoft.com/en-us/windows/desktop/direct3dhlsl/interlockedexchange
        // https://docs.microsoft.com/en-us/windows/desktop/direct3dhlsl/interlockedcompareexchange
        // So all the call of atomicExchange(dest, value) and atomicCompSwap(dest,
        // compare_value, value) should all be modified into the form of "int temp; temp =
        // atomicExchange(dest, value);" and "int temp; temp = atomicCompSwap(dest,
        // compare_value, value);" in the intermediate tree before traversing outputHLSL.
        case EOpAtomicExchange:
        case EOpAtomicCompSwap:
        {
            ASSERT(node->getChildCount() > 1);
            TIntermTyped *memNode = (*node->getSequence())[0]->getAsTyped();
            if (IsInShaderStorageBlock(memNode))
            {
                // Atomic memory functions for SSBO.
                // "_ssbo_atomicXXX_TYPE(RWByteAddressBuffer buffer, uint loc" is written to |out|.
                mSSBOOutputHLSL->outputAtomicMemoryFunctionCallPrefix(memNode, node->getOp());
                // Write the rest argument list to |out|.
                for (size_t i = 1; i < node->getChildCount(); i++)
                {
                    out << ", ";
                    TIntermTyped *argument = (*node->getSequence())[i]->getAsTyped();
                    if (IsInShaderStorageBlock(argument))
                    {
                        mSSBOOutputHLSL->outputLoadFunctionCall(argument);
                    }
                    else
                    {
                        argument->traverse(this);
                    }
                }

                out << ")";
                return false;
            }
            else
            {
                // Atomic memory functions for shared variable.
                if (node->getOp() != EOpAtomicExchange && node->getOp() != EOpAtomicCompSwap)
                {
                    outputTriplet(out, visit,
                                  GetHLSLAtomicFunctionStringAndLeftParenthesis(node->getOp()), ",",
                                  ")");
                }
                else
                {
                    UNREACHABLE();
                }
            }

            break;
        }
    }

    return true;
}

void OutputHLSL::writeIfElse(TInfoSinkBase &out, TIntermIfElse *node)
{
    out << "if (";

    node->getCondition()->traverse(this);

    out << ")\n";

    outputLineDirective(out, node->getLine().first_line);

    bool discard = false;

    if (node->getTrueBlock())
    {
        // The trueBlock child node will output braces.
        node->getTrueBlock()->traverse(this);

        // Detect true discard
        discard = (discard || FindDiscard::search(node->getTrueBlock()));
    }
    else
    {
        // TODO(oetuaho): Check if the semicolon inside is necessary.
        // It's there as a result of conservative refactoring of the output.
        out << "{;}\n";
    }

    outputLineDirective(out, node->getLine().first_line);

    if (node->getFalseBlock())
    {
        out << "else\n";

        outputLineDirective(out, node->getFalseBlock()->getLine().first_line);

        // The falseBlock child node will output braces.
        node->getFalseBlock()->traverse(this);

        outputLineDirective(out, node->getFalseBlock()->getLine().first_line);

        // Detect false discard
        discard = (discard || FindDiscard::search(node->getFalseBlock()));
    }

    // ANGLE issue 486: Detect problematic conditional discard
    if (discard)
    {
        mUsesDiscardRewriting = true;
    }
}

bool OutputHLSL::visitTernary(Visit, TIntermTernary *)
{
    // Ternary ops should have been already converted to something else in the AST. HLSL ternary
    // operator doesn't short-circuit, so it's not the same as the GLSL ternary operator.
    UNREACHABLE();
    return false;
}

bool OutputHLSL::visitIfElse(Visit visit, TIntermIfElse *node)
{
    TInfoSinkBase &out = getInfoSink();

    ASSERT(mInsideFunction);

    // D3D errors when there is a gradient operation in a loop in an unflattened if.
    if (mShaderType == GL_FRAGMENT_SHADER && mCurrentFunctionMetadata->hasGradientLoop(node))
    {
        out << "FLATTEN ";
    }

    writeIfElse(out, node);

    return false;
}

bool OutputHLSL::visitSwitch(Visit visit, TIntermSwitch *node)
{
    TInfoSinkBase &out = getInfoSink();

    ASSERT(node->getStatementList());
    if (visit == PreVisit)
    {
        node->setStatementList(RemoveSwitchFallThrough(node->getStatementList(), mPerfDiagnostics));
    }
    outputTriplet(out, visit, "switch (", ") ", "");
    // The curly braces get written when visiting the statementList block.
    return true;
}

bool OutputHLSL::visitCase(Visit visit, TIntermCase *node)
{
    TInfoSinkBase &out = getInfoSink();

    if (node->hasCondition())
    {
        outputTriplet(out, visit, "case (", "", "):\n");
        return true;
    }
    else
    {
        out << "default:\n";
        return false;
    }
}

void OutputHLSL::visitConstantUnion(TIntermConstantUnion *node)
{
    TInfoSinkBase &out = getInfoSink();
    writeConstantUnion(out, node->getType(), node->getConstantValue());
}

bool OutputHLSL::visitLoop(Visit visit, TIntermLoop *node)
{
    mNestedLoopDepth++;

    bool wasDiscontinuous = mInsideDiscontinuousLoop;
    mInsideDiscontinuousLoop =
        mInsideDiscontinuousLoop || mCurrentFunctionMetadata->mDiscontinuousLoops.count(node) > 0;

    TInfoSinkBase &out = getInfoSink();

    if (mOutputType == SH_HLSL_3_0_OUTPUT)
    {
        if (handleExcessiveLoop(out, node))
        {
            mInsideDiscontinuousLoop = wasDiscontinuous;
            mNestedLoopDepth--;

            return false;
        }
    }

    const char *unroll = mCurrentFunctionMetadata->hasGradientInCallGraph(node) ? "LOOP" : "";
    if (node->getType() == ELoopDoWhile)
    {
        out << "{" << unroll << " do\n";

        outputLineDirective(out, node->getLine().first_line);
    }
    else
    {
        out << "{" << unroll << " for(";

        if (node->getInit())
        {
            node->getInit()->traverse(this);
        }

        out << "; ";

        if (node->getCondition())
        {
            node->getCondition()->traverse(this);
        }

        out << "; ";

        if (node->getExpression())
        {
            node->getExpression()->traverse(this);
        }

        out << ")\n";

        outputLineDirective(out, node->getLine().first_line);
    }

    // The loop body node will output braces.
    node->getBody()->traverse(this);

    outputLineDirective(out, node->getLine().first_line);

    if (node->getType() == ELoopDoWhile)
    {
        outputLineDirective(out, node->getCondition()->getLine().first_line);
        out << "while (";

        node->getCondition()->traverse(this);

        out << ");\n";
    }

    out << "}\n";

    mInsideDiscontinuousLoop = wasDiscontinuous;
    mNestedLoopDepth--;

    return false;
}

bool OutputHLSL::visitBranch(Visit visit, TIntermBranch *node)
{
    if (visit == PreVisit)
    {
        TInfoSinkBase &out = getInfoSink();

        switch (node->getFlowOp())
        {
            case EOpKill:
                out << "discard";
                break;
            case EOpBreak:
                if (mNestedLoopDepth > 1)
                {
                    mUsesNestedBreak = true;
                }

                if (mExcessiveLoopIndex)
                {
                    out << "{Break";
                    mExcessiveLoopIndex->traverse(this);
                    out << " = true; break;}\n";
                }
                else
                {
                    out << "break";
                }
                break;
            case EOpContinue:
                out << "continue";
                break;
            case EOpReturn:
                if (node->getExpression())
                {
                    ASSERT(!mInsideMain);
                    out << "return ";
                    if (IsInShaderStorageBlock(node->getExpression()))
                    {
                        mSSBOOutputHLSL->outputLoadFunctionCall(node->getExpression());
                        return false;
                    }
                }
                else
                {
                    if (mInsideMain && shaderNeedsGenerateOutput())
                    {
                        out << "return " << generateOutputCall();
                    }
                    else
                    {
                        out << "return";
                    }
                }
                break;
            default:
                UNREACHABLE();
        }
    }

    return true;
}

// Handle loops with more than 254 iterations (unsupported by D3D9) by splitting them
// (The D3D documentation says 255 iterations, but the compiler complains at anything more than
// 254).
bool OutputHLSL::handleExcessiveLoop(TInfoSinkBase &out, TIntermLoop *node)
{
    const int MAX_LOOP_ITERATIONS = 254;

    // Parse loops of the form:
    // for(int index = initial; index [comparator] limit; index += increment)
    TIntermSymbol *index = nullptr;
    TOperator comparator = EOpNull;
    int initial          = 0;
    int limit            = 0;
    int increment        = 0;

    // Parse index name and intial value
    if (node->getInit())
    {
        TIntermDeclaration *init = node->getInit()->getAsDeclarationNode();

        if (init)
        {
            TIntermSequence *sequence = init->getSequence();
            TIntermTyped *variable    = (*sequence)[0]->getAsTyped();

            if (variable && variable->getQualifier() == EvqTemporary)
            {
                TIntermBinary *assign = variable->getAsBinaryNode();

                if (assign != nullptr && assign->getOp() == EOpInitialize)
                {
                    TIntermSymbol *symbol          = assign->getLeft()->getAsSymbolNode();
                    TIntermConstantUnion *constant = assign->getRight()->getAsConstantUnion();

                    if (symbol && constant)
                    {
                        if (constant->getBasicType() == EbtInt && constant->isScalar())
                        {
                            index   = symbol;
                            initial = constant->getIConst(0);
                        }
                    }
                }
            }
        }
    }

    // Parse comparator and limit value
    if (index != nullptr && node->getCondition())
    {
        TIntermBinary *test = node->getCondition()->getAsBinaryNode();

        if (test && test->getLeft()->getAsSymbolNode()->uniqueId() == index->uniqueId())
        {
            TIntermConstantUnion *constant = test->getRight()->getAsConstantUnion();

            if (constant)
            {
                if (constant->getBasicType() == EbtInt && constant->isScalar())
                {
                    comparator = test->getOp();
                    limit      = constant->getIConst(0);
                }
            }
        }
    }

    // Parse increment
    if (index != nullptr && comparator != EOpNull && node->getExpression())
    {
        TIntermBinary *binaryTerminal = node->getExpression()->getAsBinaryNode();
        TIntermUnary *unaryTerminal   = node->getExpression()->getAsUnaryNode();

        if (binaryTerminal)
        {
            TOperator op                   = binaryTerminal->getOp();
            TIntermConstantUnion *constant = binaryTerminal->getRight()->getAsConstantUnion();

            if (constant)
            {
                if (constant->getBasicType() == EbtInt && constant->isScalar())
                {
                    int value = constant->getIConst(0);

                    switch (op)
                    {
                        case EOpAddAssign:
                            increment = value;
                            break;
                        case EOpSubAssign:
                            increment = -value;
                            break;
                        default:
                            UNIMPLEMENTED();
                    }
                }
            }
        }
        else if (unaryTerminal)
        {
            TOperator op = unaryTerminal->getOp();

            switch (op)
            {
                case EOpPostIncrement:
                    increment = 1;
                    break;
                case EOpPostDecrement:
                    increment = -1;
                    break;
                case EOpPreIncrement:
                    increment = 1;
                    break;
                case EOpPreDecrement:
                    increment = -1;
                    break;
                default:
                    UNIMPLEMENTED();
            }
        }
    }

    if (index != nullptr && comparator != EOpNull && increment != 0)
    {
        if (comparator == EOpLessThanEqual)
        {
            comparator = EOpLessThan;
            limit += 1;
        }

        if (comparator == EOpLessThan)
        {
            int iterations = (limit - initial) / increment;

            if (iterations <= MAX_LOOP_ITERATIONS)
            {
                return false;  // Not an excessive loop
            }

            TIntermSymbol *restoreIndex = mExcessiveLoopIndex;
            mExcessiveLoopIndex         = index;

            out << "{int ";
            index->traverse(this);
            out << ";\n"
                   "bool Break";
            index->traverse(this);
            out << " = false;\n";

            bool firstLoopFragment = true;

            while (iterations > 0)
            {
                int clampedLimit = initial + increment * std::min(MAX_LOOP_ITERATIONS, iterations);

                if (!firstLoopFragment)
                {
                    out << "if (!Break";
                    index->traverse(this);
                    out << ") {\n";
                }

                if (iterations <= MAX_LOOP_ITERATIONS)  // Last loop fragment
                {
                    mExcessiveLoopIndex = nullptr;  // Stops setting the Break flag
                }

                // for(int index = initial; index < clampedLimit; index += increment)
                const char *unroll =
                    mCurrentFunctionMetadata->hasGradientInCallGraph(node) ? "LOOP" : "";

                out << unroll << " for(";
                index->traverse(this);
                out << " = ";
                out << initial;

                out << "; ";
                index->traverse(this);
                out << " < ";
                out << clampedLimit;

                out << "; ";
                index->traverse(this);
                out << " += ";
                out << increment;
                out << ")\n";

                outputLineDirective(out, node->getLine().first_line);
                out << "{\n";

                node->getBody()->traverse(this);

                outputLineDirective(out, node->getLine().first_line);
                out << ";}\n";

                if (!firstLoopFragment)
                {
                    out << "}\n";
                }

                firstLoopFragment = false;

                initial += MAX_LOOP_ITERATIONS * increment;
                iterations -= MAX_LOOP_ITERATIONS;
            }

            out << "}";

            mExcessiveLoopIndex = restoreIndex;

            return true;
        }
        else
            UNIMPLEMENTED();
    }

    return false;  // Not handled as an excessive loop
}

void OutputHLSL::outputTriplet(TInfoSinkBase &out,
                               Visit visit,
                               const char *preString,
                               const char *inString,
                               const char *postString)
{
    if (visit == PreVisit)
    {
        out << preString;
    }
    else if (visit == InVisit)
    {
        out << inString;
    }
    else if (visit == PostVisit)
    {
        out << postString;
    }
}

void OutputHLSL::outputLineDirective(TInfoSinkBase &out, int line)
{
    if (mCompileOptions.lineDirectives && line > 0)
    {
        out << "\n";
        out << "#line " << line;

        if (mSourcePath)
        {
            out << " \"" << mSourcePath << "\"";
        }

        out << "\n";
    }
}

void OutputHLSL::writeParameter(const TVariable *param, TInfoSinkBase &out)
{
    const TType &type    = param->getType();
    TQualifier qualifier = type.getQualifier();

    TString nameStr = DecorateVariableIfNeeded(*param);
    ASSERT(nameStr != "");  // HLSL demands named arguments, also for prototypes

    if (IsSampler(type.getBasicType()))
    {
        if (mOutputType == SH_HLSL_4_1_OUTPUT)
        {
            // Samplers are passed as indices to the sampler array.
            ASSERT(qualifier != EvqParamOut && qualifier != EvqParamInOut);
            out << "const uint " << nameStr << ArrayString(type);
            return;
        }
    }

    // If the parameter is an atomic counter, we need to add an extra parameter to keep track of the
    // buffer offset.
    if (IsAtomicCounter(type.getBasicType()))
    {
        out << QualifierString(qualifier) << " " << TypeString(type) << " " << nameStr << ", int "
            << nameStr << "_offset";
    }
    else
    {
        out << QualifierString(qualifier) << " " << TypeString(type) << " " << nameStr
            << ArrayString(type);
    }

    // If the structure parameter contains samplers, they need to be passed into the function as
    // separate parameters. HLSL doesn't natively support samplers in structs.
    if (type.isStructureContainingSamplers())
    {
        ASSERT(qualifier != EvqParamOut && qualifier != EvqParamInOut);
        TVector<const TVariable *> samplerSymbols;
        std::string namePrefix = "angle";
        namePrefix += nameStr.c_str();
        type.createSamplerSymbols(ImmutableString(namePrefix), "", &samplerSymbols, nullptr,
                                  mSymbolTable);
        for (const TVariable *sampler : samplerSymbols)
        {
            const TType &samplerType = sampler->getType();
            if (mOutputType == SH_HLSL_4_1_OUTPUT)
            {
                out << ", const uint " << sampler->name() << ArrayString(samplerType);
            }
            else
            {
                ASSERT(IsSampler(samplerType.getBasicType()));
                out << ", " << QualifierString(qualifier) << " " << TypeString(samplerType) << " "
                    << sampler->name() << ArrayString(samplerType);
            }
        }
    }
}

TString OutputHLSL::zeroInitializer(const TType &type) const
{
    TString string;

    size_t size = type.getObjectSize();
    if (size >= kZeroCount)
    {
        mUseZeroArray = true;
    }
    string = GetZeroInitializer(size).c_str();

    return "{" + string + "}";
}

void OutputHLSL::outputConstructor(TInfoSinkBase &out, Visit visit, TIntermAggregate *node)
{
    // Array constructors should have been already pruned from the code.
    ASSERT(!node->getType().isArray());

    if (visit == PreVisit)
    {
        TString constructorName;
        if (node->getBasicType() == EbtStruct)
        {
            constructorName = mStructureHLSL->addStructConstructor(*node->getType().getStruct());
        }
        else
        {
            constructorName =
                mStructureHLSL->addBuiltInConstructor(node->getType(), node->getSequence());
        }
        out << constructorName << "(";
    }
    else if (visit == InVisit)
    {
        out << ", ";
    }
    else if (visit == PostVisit)
    {
        out << ")";
    }
}

const TConstantUnion *OutputHLSL::writeConstantUnion(TInfoSinkBase &out,
                                                     const TType &type,
                                                     const TConstantUnion *const constUnion)
{
    ASSERT(!type.isArray());

    const TConstantUnion *constUnionIterated = constUnion;

    const TStructure *structure = type.getStruct();
    if (structure)
    {
        out << mStructureHLSL->addStructConstructor(*structure) << "(";

        const TFieldList &fields = structure->fields();

        for (size_t i = 0; i < fields.size(); i++)
        {
            const TType *fieldType = fields[i]->type();
            constUnionIterated     = writeConstantUnion(out, *fieldType, constUnionIterated);

            if (i != fields.size() - 1)
            {
                out << ", ";
            }
        }

        out << ")";
    }
    else
    {
        size_t size    = type.getObjectSize();
        bool writeType = size > 1;

        if (writeType)
        {
            out << TypeString(type) << "(";
        }
        constUnionIterated = writeConstantUnionArray(out, constUnionIterated, size);
        if (writeType)
        {
            out << ")";
        }
    }

    return constUnionIterated;
}

void OutputHLSL::writeEmulatedFunctionTriplet(TInfoSinkBase &out,
                                              Visit visit,
                                              const TFunction *function)
{
    if (visit == PreVisit)
    {
        ASSERT(function != nullptr);
        BuiltInFunctionEmulator::WriteEmulatedFunctionName(out, function->name().data());
        out << "(";
    }
    else
    {
        outputTriplet(out, visit, nullptr, ", ", ")");
    }
}

bool OutputHLSL::writeSameSymbolInitializer(TInfoSinkBase &out,
                                            TIntermSymbol *symbolNode,
                                            TIntermTyped *expression)
{
    ASSERT(symbolNode->variable().symbolType() != SymbolType::Empty);
    const TIntermSymbol *symbolInInitializer = FindSymbolNode(expression, symbolNode->getName());

    if (symbolInInitializer)
    {
        // Type already printed
        out << "t" + str(mUniqueIndex) + " = ";
        expression->traverse(this);
        out << ", ";
        symbolNode->traverse(this);
        out << " = t" + str(mUniqueIndex);

        mUniqueIndex++;
        return true;
    }

    return false;
}

bool OutputHLSL::writeConstantInitialization(TInfoSinkBase &out,
                                             TIntermSymbol *symbolNode,
                                             TIntermTyped *initializer)
{
    if (initializer->hasConstantValue())
    {
        symbolNode->traverse(this);
        out << ArrayString(symbolNode->getType());
        out << " = {";
        writeConstantUnionArray(out, initializer->getConstantValue(),
                                initializer->getType().getObjectSize());
        out << "}";
        return true;
    }
    return false;
}

TString OutputHLSL::addStructEqualityFunction(const TStructure &structure)
{
    const TFieldList &fields = structure.fields();

    for (const auto &eqFunction : mStructEqualityFunctions)
    {
        if (eqFunction->structure == &structure)
        {
            return eqFunction->functionName;
        }
    }

    const TString &structNameString = StructNameString(structure);

    StructEqualityFunction *function = new StructEqualityFunction();
    function->structure              = &structure;
    function->functionName           = "angle_eq_" + structNameString;

    TInfoSinkBase fnOut;

    fnOut << "bool " << function->functionName << "(" << structNameString << " a, "
          << structNameString + " b)\n"
          << "{\n"
             "    return ";

    for (size_t i = 0; i < fields.size(); i++)
    {
        const TField *field    = fields[i];
        const TType *fieldType = field->type();

        const TString &fieldNameA = "a." + Decorate(field->name());
        const TString &fieldNameB = "b." + Decorate(field->name());

        if (i > 0)
        {
            fnOut << " && ";
        }

        fnOut << "(";
        outputEqual(PreVisit, *fieldType, EOpEqual, fnOut);
        fnOut << fieldNameA;
        outputEqual(InVisit, *fieldType, EOpEqual, fnOut);
        fnOut << fieldNameB;
        outputEqual(PostVisit, *fieldType, EOpEqual, fnOut);
        fnOut << ")";
    }

    fnOut << ";\n" << "}\n";

    function->functionDefinition = fnOut.c_str();

    mStructEqualityFunctions.push_back(function);
    mEqualityFunctions.push_back(function);

    return function->functionName;
}

TString OutputHLSL::addArrayEqualityFunction(const TType &type)
{
    for (const auto &eqFunction : mArrayEqualityFunctions)
    {
        if (eqFunction->type == type)
        {
            return eqFunction->functionName;
        }
    }

    TType elementType(type);
    elementType.toArrayElementType();

    ArrayHelperFunction *function = new ArrayHelperFunction();
    function->type                = type;

    function->functionName = ArrayHelperFunctionName("angle_eq", type);

    TInfoSinkBase fnOut;

    const TString &typeName = TypeString(type);
    fnOut << "bool " << function->functionName << "(" << typeName << " a" << ArrayString(type)
          << ", " << typeName << " b" << ArrayString(type) << ")\n"
          << "{\n"
             "    for (int i = 0; i < "
          << type.getOutermostArraySize()
          << "; ++i)\n"
             "    {\n"
             "        if (";

    outputEqual(PreVisit, elementType, EOpNotEqual, fnOut);
    fnOut << "a[i]";
    outputEqual(InVisit, elementType, EOpNotEqual, fnOut);
    fnOut << "b[i]";
    outputEqual(PostVisit, elementType, EOpNotEqual, fnOut);

    fnOut << ") { return false; }\n"
             "    }\n"
             "    return true;\n"
             "}\n";

    function->functionDefinition = fnOut.c_str();

    mArrayEqualityFunctions.push_back(function);
    mEqualityFunctions.push_back(function);

    return function->functionName;
}

TString OutputHLSL::addArrayAssignmentFunction(const TType &type)
{
    for (const auto &assignFunction : mArrayAssignmentFunctions)
    {
        if (assignFunction.type == type)
        {
            return assignFunction.functionName;
        }
    }

    TType elementType(type);
    elementType.toArrayElementType();

    ArrayHelperFunction function;
    function.type = type;

    function.functionName = ArrayHelperFunctionName("angle_assign", type);

    TInfoSinkBase fnOut;

    const TString &typeName = TypeString(type);
    fnOut << "void " << function.functionName << "(out " << typeName << " a" << ArrayString(type)
          << ", " << typeName << " b" << ArrayString(type) << ")\n"
          << "{\n"
             "    for (int i = 0; i < "
          << type.getOutermostArraySize()
          << "; ++i)\n"
             "    {\n"
             "        ";

    outputAssign(PreVisit, elementType, fnOut);
    fnOut << "a[i]";
    outputAssign(InVisit, elementType, fnOut);
    fnOut << "b[i]";
    outputAssign(PostVisit, elementType, fnOut);

    fnOut << ";\n"
             "    }\n"
             "}\n";

    function.functionDefinition = fnOut.c_str();

    mArrayAssignmentFunctions.push_back(function);

    return function.functionName;
}

TString OutputHLSL::addArrayConstructIntoFunction(const TType &type)
{
    for (const auto &constructIntoFunction : mArrayConstructIntoFunctions)
    {
        if (constructIntoFunction.type == type)
        {
            return constructIntoFunction.functionName;
        }
    }

    TType elementType(type);
    elementType.toArrayElementType();

    ArrayHelperFunction function;
    function.type = type;

    function.functionName = ArrayHelperFunctionName("angle_construct_into", type);

    TInfoSinkBase fnOut;

    const TString &typeName = TypeString(type);
    fnOut << "void " << function.functionName << "(out " << typeName << " a" << ArrayString(type);
    for (unsigned int i = 0u; i < type.getOutermostArraySize(); ++i)
    {
        fnOut << ", " << typeName << " b" << i << ArrayString(elementType);
    }
    fnOut << ")\n"
             "{\n";

    for (unsigned int i = 0u; i < type.getOutermostArraySize(); ++i)
    {
        fnOut << "    ";
        outputAssign(PreVisit, elementType, fnOut);
        fnOut << "a[" << i << "]";
        outputAssign(InVisit, elementType, fnOut);
        fnOut << "b" << i;
        outputAssign(PostVisit, elementType, fnOut);
        fnOut << ";\n";
    }
    fnOut << "}\n";

    function.functionDefinition = fnOut.c_str();

    mArrayConstructIntoFunctions.push_back(function);

    return function.functionName;
}

TString OutputHLSL::addFlatEvaluateFunction(const TType &type, const TType &parameterType)
{
    for (const auto &flatEvaluateFunction : mFlatEvaluateFunctions)
    {
        if (flatEvaluateFunction.type == type &&
            flatEvaluateFunction.parameterType == parameterType)
        {
            return flatEvaluateFunction.functionName;
        }
    }

    FlatEvaluateFunction function;
    function.type          = type;
    function.parameterType = parameterType;

    const TString &typeName          = TypeString(type);
    const TString &parameterTypeName = TypeString(parameterType);

    function.functionName = "angle_eval_flat_" + typeName + "_" + parameterTypeName;

    // If <interpolant> is declared with a "flat" qualifier, the interpolated
    // value will have the same value everywhere for a single primitive, so
    // the location used for the interpolation has no effect and the functions
    // just return that same value.
    TInfoSinkBase fnOut;
    fnOut << typeName << " " << function.functionName << "(" << typeName << " i, "
          << parameterTypeName << " p)\n";
    fnOut << "{\n" << "    return i;\n" << "}\n";
    function.functionDefinition = fnOut.c_str();

    mFlatEvaluateFunctions.push_back(function);

    return function.functionName;
}

void OutputHLSL::ensureStructDefined(const TType &type)
{
    const TStructure *structure = type.getStruct();
    if (structure)
    {
        ASSERT(type.getBasicType() == EbtStruct);
        mStructureHLSL->ensureStructDefined(*structure);
    }
}

bool OutputHLSL::shaderNeedsGenerateOutput() const
{
    return mShaderType == GL_VERTEX_SHADER || mShaderType == GL_FRAGMENT_SHADER;
}

const char *OutputHLSL::generateOutputCall() const
{
    if (mShaderType == GL_VERTEX_SHADER)
    {
        return "generateOutput(input)";
    }
    else
    {
        return "generateOutput()";
    }
}
}  // namespace sh
