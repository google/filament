//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/Compiler.h"

#include <sstream>

#include "angle_gl.h"

#include "common/BinaryStream.h"
#include "common/CompiledShaderState.h"
#include "common/PackedEnums.h"
#include "common/angle_version_info.h"

#include "compiler/translator/CallDAG.h"
#include "compiler/translator/CollectVariables.h"
#include "compiler/translator/Initialize.h"
#include "compiler/translator/IsASTDepthBelowLimit.h"
#include "compiler/translator/OutputTree.h"
#include "compiler/translator/ParseContext.h"
#include "compiler/translator/ValidateBarrierFunctionCall.h"
#include "compiler/translator/ValidateClipCullDistance.h"
#include "compiler/translator/ValidateLimitations.h"
#include "compiler/translator/ValidateMaxParameters.h"
#include "compiler/translator/ValidateOutputs.h"
#include "compiler/translator/ValidateTypeSizeLimitations.h"
#include "compiler/translator/ValidateVaryingLocations.h"
#include "compiler/translator/VariablePacker.h"
#include "compiler/translator/tree_ops/ClampFragDepth.h"
#include "compiler/translator/tree_ops/ClampIndirectIndices.h"
#include "compiler/translator/tree_ops/ClampPointSize.h"
#include "compiler/translator/tree_ops/DeclareAndInitBuiltinsForInstancedMultiview.h"
#include "compiler/translator/tree_ops/DeferGlobalInitializers.h"
#include "compiler/translator/tree_ops/EmulateGLFragColorBroadcast.h"
#include "compiler/translator/tree_ops/EmulateMultiDrawShaderBuiltins.h"
#include "compiler/translator/tree_ops/FoldExpressions.h"
#include "compiler/translator/tree_ops/ForcePrecisionQualifier.h"
#include "compiler/translator/tree_ops/InitializeVariables.h"
#include "compiler/translator/tree_ops/MonomorphizeUnsupportedFunctions.h"
#include "compiler/translator/tree_ops/PruneEmptyCases.h"
#include "compiler/translator/tree_ops/PruneInfiniteLoops.h"
#include "compiler/translator/tree_ops/PruneNoOps.h"
#include "compiler/translator/tree_ops/RemoveArrayLengthMethod.h"
#include "compiler/translator/tree_ops/RemoveDynamicIndexing.h"
#include "compiler/translator/tree_ops/RemoveInvariantDeclaration.h"
#include "compiler/translator/tree_ops/RemoveUnreferencedVariables.h"
#include "compiler/translator/tree_ops/RemoveUnusedFramebufferFetch.h"
#include "compiler/translator/tree_ops/RescopeGlobalVariables.h"
#include "compiler/translator/tree_ops/RewritePixelLocalStorage.h"
#include "compiler/translator/tree_ops/SeparateDeclarations.h"
#include "compiler/translator/tree_ops/SimplifyLoopConditions.h"
#include "compiler/translator/tree_ops/SplitSequenceOperator.h"
#include "compiler/translator/tree_ops/glsl/RegenerateStructNames.h"
#include "compiler/translator/tree_ops/glsl/RewriteRepeatedAssignToSwizzled.h"
#include "compiler/translator/tree_ops/glsl/ScalarizeVecAndMatConstructorArgs.h"
#include "compiler/translator/tree_ops/glsl/UseInterfaceBlockFields.h"
#include "compiler/translator/tree_ops/glsl/apple/AddAndTrueToLoopCondition.h"
#include "compiler/translator/tree_ops/glsl/apple/RewriteDoWhile.h"
#include "compiler/translator/tree_ops/glsl/apple/UnfoldShortCircuitAST.h"
#include "compiler/translator/tree_util/BuiltIn.h"
#include "compiler/translator/tree_util/FindSymbolNode.h"
#include "compiler/translator/tree_util/IntermNodePatternMatcher.h"
#include "compiler/translator/tree_util/ReplaceShadowingVariables.h"
#include "compiler/translator/tree_util/ReplaceVariable.h"
#include "compiler/translator/util.h"

// #define ANGLE_FUZZER_CORPUS_OUTPUT_DIR "corpus/"

#if defined(ANGLE_FUZZER_CORPUS_OUTPUT_DIR)
#    include "common/hash_utils.h"
#    include "common/mathutil.h"
#endif

namespace sh
{

namespace
{
// Helper that returns if a top-level node is unused.  If it's a function, the function prototype is
// returned as well.
bool IsTopLevelNodeUnusedFunction(const CallDAG &callDag,
                                  const std::vector<TFunctionMetadata> &metadata,
                                  TIntermNode *node,
                                  const TFunction **functionOut)
{
    const TIntermFunctionPrototype *asFunctionPrototype   = node->getAsFunctionPrototypeNode();
    const TIntermFunctionDefinition *asFunctionDefinition = node->getAsFunctionDefinition();

    *functionOut = nullptr;

    if (asFunctionDefinition)
    {
        *functionOut = asFunctionDefinition->getFunction();
    }
    else if (asFunctionPrototype)
    {
        *functionOut = asFunctionPrototype->getFunction();
    }
    if (*functionOut == nullptr)
    {
        return false;
    }

    size_t callDagIndex = callDag.findIndex((*functionOut)->uniqueId());
    if (callDagIndex == CallDAG::InvalidIndex)
    {
        // This happens only for unimplemented prototypes which are thus unused
        ASSERT(asFunctionPrototype);
        return true;
    }

    ASSERT(callDagIndex < metadata.size());
    return !metadata[callDagIndex].used;
}

void AddBuiltInToInitList(TSymbolTable *symbolTable,
                          int shaderVersion,
                          TIntermBlock *root,
                          const char *name,
                          InitVariableList *list)
{
    const TIntermSymbol *builtin = FindSymbolNode(root, ImmutableString(name));
    const TVariable *builtinVar  = nullptr;
    if (builtin != nullptr)
    {
        builtinVar = &builtin->variable();
    }
    else
    {
        builtinVar = static_cast<const TVariable *>(
            symbolTable->findBuiltIn(ImmutableString(name), shaderVersion));
    }

    if (builtinVar != nullptr)
    {
        list->push_back(builtinVar);
    }
}

#if defined(ANGLE_FUZZER_CORPUS_OUTPUT_DIR)
void DumpFuzzerCase(char const *const *shaderStrings,
                    size_t numStrings,
                    uint32_t type,
                    uint32_t spec,
                    uint32_t output,
                    const ShCompileOptions &options)
{
    ShaderDumpHeader header{};
    header.type   = type;
    header.spec   = spec;
    header.output = output;
    memcpy(&header.basicCompileOptions, &options, offsetof(ShCompileOptions, metal));
    static_assert(offsetof(ShCompileOptions, metal) <= sizeof(header.basicCompileOptions));
    memcpy(&header.metalCompileOptions, &options.metal, sizeof(options.metal));
    static_assert(sizeof(options.metal) <= sizeof(header.metalCompileOptions));
    memcpy(&header.plsCompileOptions, &options.pls, sizeof(options.pls));
    static_assert(sizeof(options.pls) <= sizeof(header.plsCompileOptions));
    size_t contentsLength = sizeof(header) + 1;  // Extra: header + nul terminator.
    for (size_t i = 0; i < numStrings; i++)
    {
        contentsLength += strlen(shaderStrings[i]);
    }
    std::vector<uint8_t> contents(rx::roundUp<size_t>(contentsLength, 4), 0);
    memcpy(&contents[0], &header, sizeof(header));
    uint8_t *data = &contents[sizeof(header)];
    for (size_t i = 0; i < numStrings; i++)
    {
        auto length = strlen(shaderStrings[i]);
        memcpy(data, shaderStrings[i], length);
        data += length;
    }
    auto hash = angle::ComputeGenericHash(contents.data(), contents.size());

    std::ostringstream o = sh::InitializeStream<std::ostringstream>();
    o << ANGLE_FUZZER_CORPUS_OUTPUT_DIR << std::hex << std::setw(16) << std::setfill('0') << hash
      << ".sample";
    std::string s = o.str();

    // Must match the input format of the fuzzer
    FILE *f = fopen(s.c_str(), "w");
    fwrite(contents.data(), sizeof(char), contentsLength, f);
    fclose(f);
}
#endif  // defined(ANGLE_FUZZER_CORPUS_OUTPUT_DIR)
}  // anonymous namespace

bool IsGLSL130OrNewer(ShShaderOutput output)
{
    return (output == SH_GLSL_130_OUTPUT || output == SH_GLSL_140_OUTPUT ||
            output == SH_GLSL_150_CORE_OUTPUT || output == SH_GLSL_330_CORE_OUTPUT ||
            output == SH_GLSL_400_CORE_OUTPUT || output == SH_GLSL_410_CORE_OUTPUT ||
            output == SH_GLSL_420_CORE_OUTPUT || output == SH_GLSL_430_CORE_OUTPUT ||
            output == SH_GLSL_440_CORE_OUTPUT || output == SH_GLSL_450_CORE_OUTPUT);
}

bool IsGLSL420OrNewer(ShShaderOutput output)
{
    return (output == SH_GLSL_420_CORE_OUTPUT || output == SH_GLSL_430_CORE_OUTPUT ||
            output == SH_GLSL_440_CORE_OUTPUT || output == SH_GLSL_450_CORE_OUTPUT);
}

bool IsGLSL410OrOlder(ShShaderOutput output)
{
    return (output == SH_GLSL_130_OUTPUT || output == SH_GLSL_140_OUTPUT ||
            output == SH_GLSL_150_CORE_OUTPUT || output == SH_GLSL_330_CORE_OUTPUT ||
            output == SH_GLSL_400_CORE_OUTPUT || output == SH_GLSL_410_CORE_OUTPUT);
}

bool RemoveInvariant(sh::GLenum shaderType,
                     int shaderVersion,
                     ShShaderOutput outputType,
                     const ShCompileOptions &compileOptions)
{
    if (shaderType == GL_FRAGMENT_SHADER &&
        (IsGLSL420OrNewer(outputType) || IsOutputSPIRV(outputType)))
        return true;

    if (compileOptions.removeInvariantAndCentroidForESSL3 && shaderVersion >= 300 &&
        shaderType == GL_VERTEX_SHADER)
        return true;

    return false;
}

size_t GetGlobalMaxTokenSize(ShShaderSpec spec)
{
    // WebGL defines a max token length of 256, while ES2 leaves max token
    // size undefined. ES3 defines a max size of 1024 characters.
    switch (spec)
    {
        case SH_WEBGL_SPEC:
            return 256;
        default:
            return 1024;
    }
}

int GetMaxUniformVectorsForShaderType(GLenum shaderType, const ShBuiltInResources &resources)
{
    switch (shaderType)
    {
        case GL_VERTEX_SHADER:
            return resources.MaxVertexUniformVectors;
        case GL_FRAGMENT_SHADER:
            return resources.MaxFragmentUniformVectors;

        // TODO (jiawei.shao@intel.com): check if we need finer-grained component counting
        case GL_COMPUTE_SHADER:
            return resources.MaxComputeUniformComponents / 4;
        case GL_GEOMETRY_SHADER_EXT:
            return resources.MaxGeometryUniformComponents / 4;
        default:
            UNREACHABLE();
            return -1;
    }
}

namespace
{

class [[nodiscard]] TScopedPoolAllocator
{
  public:
    TScopedPoolAllocator(angle::PoolAllocator *allocator) : mAllocator(allocator)
    {
        mAllocator->push();
        SetGlobalPoolAllocator(mAllocator);
    }
    ~TScopedPoolAllocator()
    {
        SetGlobalPoolAllocator(nullptr);
        mAllocator->pop(angle::PoolAllocator::ReleaseStrategy::All);
    }

  private:
    angle::PoolAllocator *mAllocator;
};

class [[nodiscard]] TScopedSymbolTableLevel
{
  public:
    TScopedSymbolTableLevel(TSymbolTable *table) : mTable(table)
    {
        ASSERT(mTable->isEmpty());
        mTable->push();
    }
    ~TScopedSymbolTableLevel()
    {
        while (!mTable->isEmpty())
            mTable->pop();
    }

  private:
    TSymbolTable *mTable;
};

int GetMaxShaderVersionForSpec(ShShaderSpec spec)
{
    switch (spec)
    {
        case SH_GLES2_SPEC:
        case SH_WEBGL_SPEC:
            return 100;
        case SH_GLES3_SPEC:
        case SH_WEBGL2_SPEC:
            return 300;
        case SH_GLES3_1_SPEC:
        case SH_WEBGL3_SPEC:
            return 310;
        case SH_GLES3_2_SPEC:
            return 320;
        default:
            UNREACHABLE();
            return 0;
    }
}

bool ValidateFragColorAndFragData(GLenum shaderType,
                                  int shaderVersion,
                                  const TSymbolTable &symbolTable,
                                  TDiagnostics *diagnostics)
{
    if (shaderVersion > 100 || shaderType != GL_FRAGMENT_SHADER)
    {
        return true;
    }

    bool usesFragColor = false;
    bool usesFragData  = false;
    // This validation is a bit stricter than the spec - it's only an error to write to
    // both FragData and FragColor. But because it's better not to have reads from undefined
    // variables, we always return an error if they are both referenced, rather than only if they
    // are written.
    if (symbolTable.isStaticallyUsed(*BuiltInVariable::gl_FragColor()) ||
        symbolTable.isStaticallyUsed(*BuiltInVariable::gl_SecondaryFragColorEXT()))
    {
        usesFragColor = true;
    }
    // Extension variables may not always be initialized (saves some time at symbol table init).
    bool secondaryFragDataUsed =
        symbolTable.gl_SecondaryFragDataEXT() != nullptr &&
        symbolTable.isStaticallyUsed(*symbolTable.gl_SecondaryFragDataEXT());
    if (symbolTable.isStaticallyUsed(*symbolTable.gl_FragData()) || secondaryFragDataUsed)
    {
        usesFragData = true;
    }
    if (usesFragColor && usesFragData)
    {
        const char *errorMessage = "cannot use both gl_FragData and gl_FragColor";
        if (symbolTable.isStaticallyUsed(*BuiltInVariable::gl_SecondaryFragColorEXT()) ||
            secondaryFragDataUsed)
        {
            errorMessage =
                "cannot use both output variable sets (gl_FragData, gl_SecondaryFragDataEXT)"
                " and (gl_FragColor, gl_SecondaryFragColorEXT)";
        }
        diagnostics->globalError(errorMessage);
        return false;
    }
    return true;
}

}  // namespace

TShHandleBase::TShHandleBase()
{
    allocator.push();
    SetGlobalPoolAllocator(&allocator);
}

TShHandleBase::~TShHandleBase()
{
    SetGlobalPoolAllocator(nullptr);
    allocator.popAll();
}

TCompiler::TCompiler(sh::GLenum type, ShShaderSpec spec, ShShaderOutput output)
    : mVariablesCollected(false),
      mGLPositionInitialized(false),
      mShaderType(type),
      mShaderSpec(spec),
      mOutputType(output),
      mBuiltInFunctionEmulator(),
      mDiagnostics(mInfoSink.info),
      mSourcePath(nullptr),
      mComputeShaderLocalSizeDeclared(false),
      mComputeShaderLocalSize(1),
      mGeometryShaderMaxVertices(-1),
      mGeometryShaderInvocations(0),
      mGeometryShaderInputPrimitiveType(EptUndefined),
      mGeometryShaderOutputPrimitiveType(EptUndefined),
      mTessControlShaderOutputVertices(0),
      mTessEvaluationShaderInputPrimitiveType(EtetUndefined),
      mTessEvaluationShaderInputVertexSpacingType(EtetUndefined),
      mTessEvaluationShaderInputOrderingType(EtetUndefined),
      mTessEvaluationShaderInputPointType(EtetUndefined),
      mHasAnyPreciseType(false),
      mAdvancedBlendEquations(0),
      mUsesDerivatives(false),
      mCompileOptions{}
{}

TCompiler::~TCompiler() {}

bool TCompiler::isHighPrecisionSupported() const
{
    return mShaderVersion > 100 || mShaderType != GL_FRAGMENT_SHADER ||
           mResources.FragmentPrecisionHigh == 1;
}

bool TCompiler::shouldRunLoopAndIndexingValidation(const ShCompileOptions &compileOptions) const
{
    // If compiling an ESSL 1.00 shader for WebGL, or if its been requested through the API,
    // validate loop and indexing as well (to verify that the shader only uses minimal functionality
    // of ESSL 1.00 as in Appendix A of the spec).
    return (IsWebGLBasedSpec(mShaderSpec) && mShaderVersion == 100) ||
           compileOptions.validateLoopIndexing;
}

bool TCompiler::shouldLimitTypeSizes() const
{
    // Prevent unrealistically large variable sizes in shaders.  This works around driver bugs
    // around int-size limits (such as 2GB).  The limits are generously large enough that no real
    // shader should ever hit it.
    //
    // The size check does not take std430 into account, so this is limited to WebGL and shaders
    // up to ES3.
    return mShaderVersion <= 300;
}

bool TCompiler::Init(const ShBuiltInResources &resources)
{
    SetGlobalPoolAllocator(&allocator);

    // Generate built-in symbol table.
    if (!initBuiltInSymbolTable(resources))
        return false;

    mResources = resources;
    setResourceString();

    InitExtensionBehavior(resources, mExtensionBehavior);
    return true;
}

TIntermBlock *TCompiler::compileTreeForTesting(const char *const shaderStrings[],
                                               size_t numStrings,
                                               const ShCompileOptions &compileOptions)
{
    return compileTreeImpl(shaderStrings, numStrings, compileOptions);
}

TIntermBlock *TCompiler::compileTreeImpl(const char *const shaderStrings[],
                                         size_t numStrings,
                                         const ShCompileOptions &compileOptions)
{
    // Remember the compile options for helper functions such as validateAST.
    mCompileOptions = compileOptions;

    clearResults();

    ASSERT(numStrings > 0);
    ASSERT(GetGlobalPoolAllocator());

    // Reset the extension behavior for each compilation unit.
    ResetExtensionBehavior(mResources, mExtensionBehavior, compileOptions);

    // If gl_DrawID is not supported, remove it from the available extensions
    // Currently we only allow emulation of gl_DrawID
    const bool glDrawIDSupported = compileOptions.emulateGLDrawID;
    if (!glDrawIDSupported)
    {
        auto it = mExtensionBehavior.find(TExtension::ANGLE_multi_draw);
        if (it != mExtensionBehavior.end())
        {
            mExtensionBehavior.erase(it);
        }
    }

    const bool glBaseVertexBaseInstanceSupported = compileOptions.emulateGLBaseVertexBaseInstance;
    if (!glBaseVertexBaseInstanceSupported)
    {
        auto it =
            mExtensionBehavior.find(TExtension::ANGLE_base_vertex_base_instance_shader_builtin);
        if (it != mExtensionBehavior.end())
        {
            mExtensionBehavior.erase(it);
        }
    }

    // First string is path of source file if flag is set. The actual source follows.
    size_t firstSource = 0;
    if (compileOptions.sourcePath)
    {
        mSourcePath = shaderStrings[0];
        ++firstSource;
    }

    TParseContext parseContext(mSymbolTable, mExtensionBehavior, mShaderType, mShaderSpec,
                               compileOptions, &mDiagnostics, getResources(), getOutputType());

    parseContext.setFragmentPrecisionHighOnESSL1(mResources.FragmentPrecisionHigh == 1);

    // We preserve symbols at the built-in level from compile-to-compile.
    // Start pushing the user-defined symbols at global level.
    TScopedSymbolTableLevel globalLevel(&mSymbolTable);
    ASSERT(mSymbolTable.atGlobalLevel());

    // Parse shader.
    if (PaParseStrings(numStrings - firstSource, &shaderStrings[firstSource], nullptr,
                       &parseContext) != 0)
    {
        return nullptr;
    }

    if (!postParseChecks(parseContext))
    {
        return nullptr;
    }

    setASTMetadata(parseContext);

    if (!checkShaderVersion(&parseContext))
    {
        return nullptr;
    }

    TIntermBlock *root = parseContext.getTreeRoot();
    if (!checkAndSimplifyAST(root, parseContext, compileOptions))
    {
        return nullptr;
    }

    return root;
}

bool TCompiler::checkShaderVersion(TParseContext *parseContext)
{
    if (GetMaxShaderVersionForSpec(mShaderSpec) < mShaderVersion)
    {
        mDiagnostics.globalError("unsupported shader version");
        return false;
    }

    ASSERT(parseContext);
    switch (mShaderType)
    {
        case GL_COMPUTE_SHADER:
            if (mShaderVersion < 310)
            {
                mDiagnostics.globalError("Compute shader is not supported in this shader version.");
                return false;
            }
            break;

        case GL_GEOMETRY_SHADER_EXT:
            if (mShaderVersion < 310)
            {
                mDiagnostics.globalError(
                    "Geometry shader is not supported in this shader version.");
                return false;
            }
            else if (mShaderVersion == 310)
            {
                if (!parseContext->checkCanUseOneOfExtensions(
                        sh::TSourceLoc(),
                        std::array<TExtension, 2u>{
                            {TExtension::EXT_geometry_shader, TExtension::OES_geometry_shader}}))
                {
                    return false;
                }
            }
            break;

        case GL_TESS_CONTROL_SHADER_EXT:
        case GL_TESS_EVALUATION_SHADER_EXT:
            if (mShaderVersion < 310)
            {
                mDiagnostics.globalError(
                    "Tessellation shaders are not supported in this shader version.");
                return false;
            }
            else if (mShaderVersion == 310)
            {
                if (!parseContext->checkCanUseOneOfExtensions(
                        sh::TSourceLoc(),
                        std::array<TExtension, 2u>{{TExtension::EXT_tessellation_shader,
                                                    TExtension::OES_tessellation_shader}}))
                {
                    return false;
                }
            }
            break;

        default:
            break;
    }

    return true;
}

void TCompiler::setASTMetadata(const TParseContext &parseContext)
{
    mShaderVersion = parseContext.getShaderVersion();

    mPragma = parseContext.pragma();
    mSymbolTable.setGlobalInvariant(mPragma.stdgl.invariantAll);

    mEarlyFragmentTestsSpecified = parseContext.isEarlyFragmentTestsSpecified();

    mMetadataFlags[MetadataFlags::HasDiscard] = parseContext.hasDiscard();
    mMetadataFlags[MetadataFlags::EnablesPerSampleShading] =
        parseContext.isSampleQualifierSpecified();

    mComputeShaderLocalSizeDeclared = parseContext.isComputeShaderLocalSizeDeclared();
    mComputeShaderLocalSize         = parseContext.getComputeShaderLocalSize();

    mNumViews = parseContext.getNumViews();

    mHasAnyPreciseType = parseContext.hasAnyPreciseType();

    mUsesDerivatives = parseContext.usesDerivatives();

    if (mShaderType == GL_FRAGMENT_SHADER)
    {
        mAdvancedBlendEquations = parseContext.getAdvancedBlendEquations();
        const std::map<int, ShPixelLocalStorageFormat> &plsFormats =
            parseContext.pixelLocalStorageFormats();
        // std::map keys are in sorted order, so the PLS uniform with the largest binding will be at
        // rbegin().
        mPixelLocalStorageFormats.resize(plsFormats.empty() ? 0 : plsFormats.rbegin()->first + 1,
                                         ShPixelLocalStorageFormat::NotPLS);
        for (auto [binding, format] : parseContext.pixelLocalStorageFormats())
        {
            mPixelLocalStorageFormats[binding] = format;
        }
    }
    if (mShaderType == GL_GEOMETRY_SHADER_EXT)
    {
        mGeometryShaderInputPrimitiveType  = parseContext.getGeometryShaderInputPrimitiveType();
        mGeometryShaderOutputPrimitiveType = parseContext.getGeometryShaderOutputPrimitiveType();
        mGeometryShaderMaxVertices         = parseContext.getGeometryShaderMaxVertices();
        mGeometryShaderInvocations         = parseContext.getGeometryShaderInvocations();

        mMetadataFlags[MetadataFlags::HasValidGeometryShaderInputPrimitiveType] =
            mGeometryShaderInputPrimitiveType != EptUndefined;
        mMetadataFlags[MetadataFlags::HasValidGeometryShaderOutputPrimitiveType] =
            mGeometryShaderOutputPrimitiveType != EptUndefined;
        mMetadataFlags[MetadataFlags::HasValidGeometryShaderMaxVertices] =
            mGeometryShaderMaxVertices >= 0;
    }
    if (mShaderType == GL_TESS_CONTROL_SHADER_EXT)
    {
        mTessControlShaderOutputVertices = parseContext.getTessControlShaderOutputVertices();
    }
    if (mShaderType == GL_TESS_EVALUATION_SHADER_EXT)
    {
        mTessEvaluationShaderInputPrimitiveType =
            parseContext.getTessEvaluationShaderInputPrimitiveType();
        mTessEvaluationShaderInputVertexSpacingType =
            parseContext.getTessEvaluationShaderInputVertexSpacingType();
        mTessEvaluationShaderInputOrderingType =
            parseContext.getTessEvaluationShaderInputOrderingType();
        mTessEvaluationShaderInputPointType = parseContext.getTessEvaluationShaderInputPointType();

        mMetadataFlags[MetadataFlags::HasValidTessGenMode] =
            mTessEvaluationShaderInputPrimitiveType != EtetUndefined;
        mMetadataFlags[MetadataFlags::HasValidTessGenSpacing] =
            mTessEvaluationShaderInputVertexSpacingType != EtetUndefined;
        mMetadataFlags[MetadataFlags::HasValidTessGenVertexOrder] =
            mTessEvaluationShaderInputOrderingType != EtetUndefined;
        mMetadataFlags[MetadataFlags::HasValidTessGenPointMode] =
            mTessEvaluationShaderInputPointType != EtetUndefined;
    }
}

unsigned int TCompiler::getSharedMemorySize() const
{
    unsigned int sharedMemSize = 0;
    for (const sh::ShaderVariable &var : mSharedVariables)
    {
        sharedMemSize += var.getExternalSize();
    }

    return sharedMemSize;
}

bool TCompiler::getShaderBinary(const ShHandle compilerHandle,
                                const char *const shaderStrings[],
                                size_t numStrings,
                                const ShCompileOptions &compileOptions,
                                ShaderBinaryBlob *const binaryOut)
{
    if (!compile(shaderStrings, numStrings, compileOptions))
    {
        return false;
    }

    gl::BinaryOutputStream stream;
    gl::ShaderType shaderType = gl::FromGLenum<gl::ShaderType>(mShaderType);
    gl::CompiledShaderState state(shaderType);
    state.buildCompiledShaderState(compilerHandle, IsOutputSPIRV(mOutputType));

    stream.writeBytes(
        reinterpret_cast<const unsigned char *>(angle::GetANGLEShaderProgramVersion()),
        angle::GetANGLEShaderProgramVersionHashSize());
    stream.writeEnum(shaderType);
    stream.writeEnum(mOutputType);

    // Serialize the full source string for the shader. Ignore the source path if it is provided.
    std::string sourceString;
    size_t startingIndex = compileOptions.sourcePath ? 1 : 0;
    for (size_t i = startingIndex; i < numStrings; ++i)
    {
        sourceString.append(shaderStrings[i]);
    }
    stream.writeString(sourceString);

    stream.writeBytes(reinterpret_cast<const uint8_t *>(&compileOptions), sizeof(compileOptions));
    stream.writeBytes(reinterpret_cast<const uint8_t *>(&mResources), sizeof(mResources));

    state.serialize(stream);

    ASSERT(binaryOut);
    *binaryOut = std::move(stream.getData());
    return true;
}

bool TCompiler::validateAST(TIntermNode *root)
{
    if (mCompileOptions.validateAST)
    {
        bool valid = ValidateAST(root, &mDiagnostics, mValidateASTOptions);

#if defined(ANGLE_ENABLE_ASSERTS)
        if (!valid)
        {
            OutputTree(root, mInfoSink.info);
            fprintf(stderr, "AST validation error(s):\n%s\n", mInfoSink.info.c_str());
        }
#endif
        // In debug, assert validation.  In release, validation errors will be returned back to the
        // application as internal ANGLE errors.
        ASSERT(valid);

        return valid;
    }
    return true;
}

bool TCompiler::disableValidateFunctionCall()
{
    bool wasEnabled                          = mValidateASTOptions.validateFunctionCall;
    mValidateASTOptions.validateFunctionCall = false;
    return wasEnabled;
}

void TCompiler::restoreValidateFunctionCall(bool enable)
{
    ASSERT(!mValidateASTOptions.validateFunctionCall);
    mValidateASTOptions.validateFunctionCall = enable;
}

bool TCompiler::disableValidateVariableReferences()
{
    bool wasEnabled                                = mValidateASTOptions.validateVariableReferences;
    mValidateASTOptions.validateVariableReferences = false;
    return wasEnabled;
}

void TCompiler::restoreValidateVariableReferences(bool enable)
{
    ASSERT(!mValidateASTOptions.validateVariableReferences);
    mValidateASTOptions.validateVariableReferences = enable;
}

void TCompiler::enableValidateNoMoreTransformations()
{
    mValidateASTOptions.validateNoMoreTransformations = true;
}

bool TCompiler::checkAndSimplifyAST(TIntermBlock *root,
                                    const TParseContext &parseContext,
                                    const ShCompileOptions &compileOptions)
{
    mValidateASTOptions = {};

    // Disallow expressions deemed too complex.
    // This needs to be checked before other functions that will traverse the AST
    // to prevent potential stack overflow crashes.
    if (compileOptions.limitExpressionComplexity && !limitExpressionComplexity(root))
    {
        return false;
    }

    if (!validateAST(root))
    {
        return false;
    }

    // Turn |inout| variables that are never read from into |out| before collecting variables and
    // before PLS uses them.
    if (mShaderVersion >= 300 &&
        (IsExtensionEnabled(mExtensionBehavior, TExtension::EXT_shader_framebuffer_fetch) ||
         IsExtensionEnabled(mExtensionBehavior,
                            TExtension::EXT_shader_framebuffer_fetch_non_coherent)))
    {
        if (!RemoveUnusedFramebufferFetch(this, root, &mSymbolTable))
        {
            return false;
        }
    }

    // For now, rewrite pixel local storage before collecting variables or any operations on images.
    //
    // TODO(anglebug.com/40096838):
    //   Should this actually run after collecting variables?
    //   Do we need more introspection?
    //   Do we want to hide rewritten shader image uniforms from glGetActiveUniform?
    if (hasPixelLocalStorageUniforms())
    {
        ASSERT(
            IsExtensionEnabled(mExtensionBehavior, TExtension::ANGLE_shader_pixel_local_storage));
        if (!RewritePixelLocalStorage(this, root, getSymbolTable(), compileOptions,
                                      getShaderVersion()))
        {
            mDiagnostics.globalError("internal compiler error translating pixel local storage");
            return false;
        }
    }

    if (shouldRunLoopAndIndexingValidation(compileOptions) &&
        !ValidateLimitations(root, mShaderType, &mSymbolTable, &mDiagnostics))
    {
        return false;
    }

    if (!ValidateFragColorAndFragData(mShaderType, mShaderVersion, mSymbolTable, &mDiagnostics))
    {
        return false;
    }

    // Fold expressions that could not be folded before validation that was done as a part of
    // parsing.
    if (!FoldExpressions(this, root, &mDiagnostics))
    {
        return false;
    }
    // Folding should only be able to generate warnings.
    ASSERT(mDiagnostics.numErrors() == 0);

    // gl_ClipDistance and gl_CullDistance built-in arrays have unique semantics.
    // They are pre-declared as unsized and must be sized by the shader either
    // redeclaring them or indexing them only with integral constant expressions.
    // The translator treats them as having the maximum allowed size and this pass
    // detects the actual sizes resizing the variables if needed.
    if (parseContext.isExtensionEnabled(TExtension::ANGLE_clip_cull_distance) ||
        parseContext.isExtensionEnabled(TExtension::EXT_clip_cull_distance) ||
        parseContext.isExtensionEnabled(TExtension::APPLE_clip_distance))
    {
        bool isClipDistanceUsed = false;
        if (!ValidateClipCullDistance(this, root, &mDiagnostics,
                                      mResources.MaxCombinedClipAndCullDistances,
                                      &mClipDistanceSize, &mCullDistanceSize, &isClipDistanceUsed))
        {
            return false;
        }
        mMetadataFlags[MetadataFlags::HasClipDistance] = isClipDistanceUsed;
    }

    // Validate no barrier() after return before prunning it in |PruneNoOps()| below.
    if (mShaderType == GL_TESS_CONTROL_SHADER && !ValidateBarrierFunctionCall(root, &mDiagnostics))
    {
        return false;
    }

    // We prune no-ops to work around driver bugs and to keep AST processing and output simple.
    // The following kinds of no-ops are pruned:
    //   1. Empty declarations "int;".
    //   2. Literal statements: "1.0;". The ESSL output doesn't define a default precision
    //      for float, so float literal statements would end up with no precision which is
    //      invalid ESSL.
    //   3. Any unreachable statement after a discard, return, break or continue.
    // After this empty declarations are not allowed in the AST.
    if (!PruneNoOps(this, root, &mSymbolTable))
    {
        return false;
    }
    mValidateASTOptions.validateNoStatementsAfterBranch = true;

    // We need to generate globals early if we have non constant initializers enabled
    bool initializeLocalsAndGlobals =
        compileOptions.initializeUninitializedLocals && !IsOutputHLSL(getOutputType());
    bool canUseLoopsToInitialize       = !compileOptions.dontUseLoopsToInitializeVariables;
    bool highPrecisionSupported        = isHighPrecisionSupported();
    bool enableNonConstantInitializers = IsExtensionEnabled(
        mExtensionBehavior, TExtension::EXT_shader_non_constant_global_initializers);
    // forceDeferNonConstGlobalInitializers is needed for MSL
    // to convert a non-const global. For example:
    //
    //    int someGlobal = 123;
    //
    // to
    //
    //    int someGlobal;
    //    void main() {
    //        someGlobal = 123;
    //
    // This is because MSL doesn't allow statically initialized non-const globals.
    bool forceDeferNonConstGlobalInitializers = getOutputType() == SH_MSL_METAL_OUTPUT;

    if (enableNonConstantInitializers &&
        !DeferGlobalInitializers(this, root, initializeLocalsAndGlobals, canUseLoopsToInitialize,
                                 highPrecisionSupported, forceDeferNonConstGlobalInitializers,
                                 &mSymbolTable))
    {
        return false;
    }

    // Create the function DAG and check there is no recursion
    if (!initCallDag(root))
    {
        return false;
    }

    if (compileOptions.limitCallStackDepth && !checkCallDepth())
    {
        return false;
    }

    // Checks which functions are used and if "main" exists
    mFunctionMetadata.clear();
    mFunctionMetadata.resize(mCallDag.size());
    if (!tagUsedFunctions())
    {
        return false;
    }

    if (!pruneUnusedFunctions(root))
    {
        return false;
    }

    if (IsSpecWithFunctionBodyNewScope(mShaderSpec, mShaderVersion))
    {
        if (!ReplaceShadowingVariables(this, root, &mSymbolTable))
        {
            return false;
        }
    }

    if (mShaderVersion >= 310 && !ValidateVaryingLocations(root, &mDiagnostics, mShaderType))
    {
        return false;
    }

    // anglebug.com/42265954: The ESSL spec has a bug with images as function arguments. The
    // recommended workaround is to inline functions that accept image arguments.
    if (mShaderVersion >= 310 && !MonomorphizeUnsupportedFunctions(
                                     this, root, &mSymbolTable,
                                     UnsupportedFunctionArgsBitSet{UnsupportedFunctionArgs::Image}))
    {
        return false;
    }

    if (mShaderVersion >= 300 && mShaderType == GL_FRAGMENT_SHADER &&
        !ValidateOutputs(root, getExtensionBehavior(), mResources, hasPixelLocalStorageUniforms(),
                         IsWebGLBasedSpec(mShaderSpec), &mDiagnostics))
    {
        return false;
    }

    // Clamping uniform array bounds needs to happen after validateLimitations pass.
    if (compileOptions.clampIndirectArrayBounds)
    {
        if (!ClampIndirectIndices(this, root, &mSymbolTable))
        {
            return false;
        }
    }

    if (compileOptions.initializeBuiltinsForInstancedMultiview &&
        (parseContext.isExtensionEnabled(TExtension::OVR_multiview2) ||
         parseContext.isExtensionEnabled(TExtension::OVR_multiview)) &&
        getShaderType() != GL_COMPUTE_SHADER)
    {
        if (!DeclareAndInitBuiltinsForInstancedMultiview(
                this, root, mNumViews, mShaderType, compileOptions, mOutputType, &mSymbolTable))
        {
            return false;
        }
    }

    // This pass might emit short circuits so keep it before the short circuit unfolding
    if (compileOptions.rewriteDoWhileLoops)
    {
        if (!RewriteDoWhile(this, root, &mSymbolTable))
        {
            return false;
        }
    }

    if (compileOptions.addAndTrueToLoopCondition)
    {
        if (!AddAndTrueToLoopCondition(this, root))
        {
            return false;
        }
    }

    if (compileOptions.unfoldShortCircuit)
    {
        if (!UnfoldShortCircuitAST(this, root))
        {
            return false;
        }
    }

    if (compileOptions.regenerateStructNames)
    {
        if (!RegenerateStructNames(this, root, &mSymbolTable))
        {
            return false;
        }
    }

    if (mShaderType == GL_VERTEX_SHADER &&
        IsExtensionEnabled(mExtensionBehavior, TExtension::ANGLE_multi_draw))
    {
        if (compileOptions.emulateGLDrawID)
        {
            if (!EmulateGLDrawID(this, root, &mSymbolTable, &mUniforms))
            {
                return false;
            }
        }
    }

    if (mShaderType == GL_VERTEX_SHADER &&
        IsExtensionEnabled(mExtensionBehavior,
                           TExtension::ANGLE_base_vertex_base_instance_shader_builtin))
    {
        if (compileOptions.emulateGLBaseVertexBaseInstance)
        {
            if (!EmulateGLBaseVertexBaseInstance(this, root, &mSymbolTable, &mUniforms,
                                                 compileOptions.addBaseVertexToVertexID))
            {
                return false;
            }
        }
    }

    if (mShaderType == GL_FRAGMENT_SHADER && mShaderVersion == 100 && mResources.EXT_draw_buffers &&
        mResources.MaxDrawBuffers > 1 &&
        IsExtensionEnabled(mExtensionBehavior, TExtension::EXT_draw_buffers))
    {
        if (!EmulateGLFragColorBroadcast(this, root, mResources.MaxDrawBuffers,
                                         mResources.MaxDualSourceDrawBuffers, &mOutputVariables,
                                         &mSymbolTable, mShaderVersion))
        {
            return false;
        }
    }

    if (compileOptions.simplifyLoopConditions)
    {
        if (!SimplifyLoopConditions(this, root, &getSymbolTable()))
        {
            return false;
        }
    }
    else
    {
        // Split multi declarations and remove calls to array length().
        // Note that SimplifyLoopConditions needs to be run before any other AST transformations
        // that may need to generate new statements from loop conditions or loop expressions.
        if (!SimplifyLoopConditions(this, root,
                                    IntermNodePatternMatcher::kMultiDeclaration |
                                        IntermNodePatternMatcher::kArrayLengthMethod,
                                    &getSymbolTable()))
        {
            return false;
        }
    }

    // Note that separate declarations need to be run before other AST transformations that
    // generate new statements from expressions.
    if (!SeparateDeclarations(*this, *root, mCompileOptions.separateCompoundStructDeclarations))
    {
        return false;
    }

    if (IsWebGLBasedSpec(mShaderSpec))
    {
        // Remove infinite loops, they are not supposed to exist in shaders.
        bool anyInfiniteLoops = false;
        if (!PruneInfiniteLoops(this, root, &mSymbolTable, &anyInfiniteLoops))
        {
            return false;
        }

        // If requested, reject shaders with infinite loops.  If not requested, the same loops are
        // removed from the shader as a fallback.
        if (anyInfiniteLoops && mCompileOptions.rejectWebglShadersWithUndefinedBehavior)
        {
            mDiagnostics.globalError("Infinite loop detected in the shader");
            return false;
        }
    }

    if (compileOptions.rescopeGlobalVariables)
    {
        if (!RescopeGlobalVariables(*this, *root))
        {
            return false;
        }
    }

    mValidateASTOptions.validateMultiDeclarations = true;

    if (!SplitSequenceOperator(this, root, IntermNodePatternMatcher::kArrayLengthMethod,
                               &getSymbolTable()))
    {
        return false;
    }

    if (!RemoveArrayLengthMethod(this, root))
    {
        return false;
    }
    // Fold the expressions again, because |RemoveArrayLengthMethod| can introduce new constants.
    if (!FoldExpressions(this, root, &mDiagnostics))
    {
        return false;
    }

    if (!RemoveUnreferencedVariables(this, root, &mSymbolTable))
    {
        return false;
    }

    // In case the last case inside a switch statement is a certain type of no-op, GLSL compilers in
    // drivers may not accept it. In this case we clean up the dead code from the end of switch
    // statements. This is also required because PruneNoOps or RemoveUnreferencedVariables may have
    // left switch statements that only contained an empty declaration inside the final case in an
    // invalid state. Relies on that PruneNoOps and RemoveUnreferencedVariables have already been
    // run.
    if (!PruneEmptyCases(this, root))
    {
        return false;
    }

    // Run after RemoveUnreferencedVariables, validate that the shader does not have excessively
    // large variables.
    if (shouldLimitTypeSizes() && !ValidateTypeSizeLimitations(root, &mSymbolTable, &mDiagnostics))
    {
        return false;
    }

    // Built-in function emulation needs to happen after validateLimitations pass.
    GetGlobalPoolAllocator()->lock();
    initBuiltInFunctionEmulator(&mBuiltInFunctionEmulator, compileOptions);
    GetGlobalPoolAllocator()->unlock();
    mBuiltInFunctionEmulator.markBuiltInFunctionsForEmulation(root);

    if (compileOptions.scalarizeVecAndMatConstructorArgs)
    {
        if (!ScalarizeVecAndMatConstructorArgs(this, root, &mSymbolTable))
        {
            return false;
        }
    }

    if (compileOptions.forceShaderPrecisionHighpToMediump)
    {
        if (!ForceShaderPrecisionToMediump(root, &mSymbolTable, mShaderType))
        {
            return false;
        }
    }

    ASSERT(!mVariablesCollected);
    CollectVariables(root, &mAttributes, &mOutputVariables, &mUniforms, &mInputVaryings,
                     &mOutputVaryings, &mSharedVariables, &mUniformBlocks, &mShaderStorageBlocks,
                     mResources.HashFunction, &mSymbolTable, mShaderType, mExtensionBehavior,
                     mResources, mTessControlShaderOutputVertices);
    collectInterfaceBlocks();
    mVariablesCollected = true;
    if (compileOptions.useUnusedStandardSharedBlocks)
    {
        if (!useAllMembersInUnusedStandardAndSharedBlocks(root))
        {
            return false;
        }
    }
    if (compileOptions.enforcePackingRestrictions)
    {
        int maxUniformVectors = GetMaxUniformVectorsForShaderType(mShaderType, mResources);
        if (mShaderType == GL_VERTEX_SHADER && compileOptions.emulateClipOrigin)
        {
            --maxUniformVectors;
        }
        // Returns true if, after applying the packing rules in the GLSL ES 1.00.17 spec
        // Appendix A, section 7, the shader does not use too many uniforms.
        if (!CheckVariablesInPackingLimits(maxUniformVectors, mUniforms))
        {
            mDiagnostics.globalError("too many uniforms");
            return false;
        }
    }
    bool needInitializeOutputVariables =
        compileOptions.initOutputVariables && mShaderType != GL_COMPUTE_SHADER;
    needInitializeOutputVariables |=
        compileOptions.initFragmentOutputVariables && mShaderType == GL_FRAGMENT_SHADER;
    if (needInitializeOutputVariables)
    {
        if (!initializeOutputVariables(root))
        {
            return false;
        }
    }

    // Removing invariant declarations must be done after collecting variables.
    // Otherwise, built-in invariant declarations don't apply.
    if (RemoveInvariant(mShaderType, mShaderVersion, mOutputType, compileOptions))
    {
        if (!RemoveInvariantDeclaration(this, root))
        {
            return false;
        }
    }

    // gl_Position is always written in compatibility output mode.
    // It may have been already initialized among other output variables, in that case we don't
    // need to initialize it twice.
    if (mShaderType == GL_VERTEX_SHADER && !mGLPositionInitialized &&
        (compileOptions.initGLPosition || mOutputType == SH_GLSL_COMPATIBILITY_OUTPUT))
    {
        if (!initializeGLPosition(root))
        {
            return false;
        }
        mGLPositionInitialized = true;
    }

    // DeferGlobalInitializers needs to be run before other AST transformations that generate new
    // statements from expressions. But it's fine to run DeferGlobalInitializers after the above
    // SplitSequenceOperator and RemoveArrayLengthMethod since they only have an effect on the AST
    // on ESSL >= 3.00, and the initializers that need to be deferred can only exist in ESSL < 3.00.
    // Exception: if EXT_shader_non_constant_global_initializers is enabled, we must generate global
    // initializers before we generate the DAG, since initializers may call functions which must not
    // be optimized out
    if (!enableNonConstantInitializers &&
        !DeferGlobalInitializers(this, root, initializeLocalsAndGlobals, canUseLoopsToInitialize,
                                 highPrecisionSupported, forceDeferNonConstGlobalInitializers,
                                 &mSymbolTable))
    {
        return false;
    }

    if (initializeLocalsAndGlobals)
    {
        // Initialize uninitialized local variables.
        // In some cases initializing can generate extra statements in the parent block, such as
        // when initializing nameless structs or initializing arrays in ESSL 1.00. In that case
        // we need to first simplify loop conditions. We've already separated declarations
        // earlier, which is also required. If we don't follow the Appendix A limitations, loop
        // init statements can declare arrays or nameless structs and have multiple
        // declarations.

        if (!shouldRunLoopAndIndexingValidation(compileOptions))
        {
            if (!SimplifyLoopConditions(this, root,
                                        IntermNodePatternMatcher::kArrayDeclaration |
                                            IntermNodePatternMatcher::kNamelessStructDeclaration,
                                        &getSymbolTable()))
            {
                return false;
            }
        }

        if (!InitializeUninitializedLocals(this, root, getShaderVersion(), canUseLoopsToInitialize,
                                           highPrecisionSupported, &getSymbolTable()))
        {
            return false;
        }
    }

    if (getShaderType() == GL_VERTEX_SHADER && compileOptions.clampPointSize)
    {
        if (!ClampPointSize(this, root, mResources.MinPointSize, mResources.MaxPointSize,
                            &getSymbolTable()))
        {
            return false;
        }
    }

    if (getShaderType() == GL_FRAGMENT_SHADER && compileOptions.clampFragDepth)
    {
        if (!ClampFragDepth(this, root, &getSymbolTable()))
        {
            return false;
        }
    }

    if (compileOptions.rewriteRepeatedAssignToSwizzled)
    {
        if (!sh::RewriteRepeatedAssignToSwizzled(this, root))
        {
            return false;
        }
    }

    if (compileOptions.removeDynamicIndexingOfSwizzledVector)
    {
        if (!sh::RemoveDynamicIndexingOfSwizzledVector(this, root, &getSymbolTable(), nullptr))
        {
            return false;
        }
    }

    return true;
}

bool TCompiler::postParseChecks(const TParseContext &parseContext)
{
    std::stringstream errorMessage;

    if (parseContext.getTreeRoot() == nullptr)
    {
        errorMessage << "Shader parsing failed (mTreeRoot == nullptr)";
    }

    for (TType *type : parseContext.getDeferredArrayTypesToSize())
    {
        errorMessage << "Unsized global array type: " << type->getBasicString();
    }

    if (!errorMessage.str().empty())
    {
        mDiagnostics.globalError(errorMessage.str().c_str());
        return false;
    }

    return true;
}

bool TCompiler::compile(const char *const shaderStrings[],
                        size_t numStrings,
                        const ShCompileOptions &compileOptionsIn)
{
#if defined(ANGLE_FUZZER_CORPUS_OUTPUT_DIR)
    DumpFuzzerCase(shaderStrings, numStrings, mShaderType, mShaderSpec, mOutputType,
                   compileOptionsIn);
#endif  // defined(ANGLE_FUZZER_CORPUS_OUTPUT_DIR)

    if (numStrings == 0)
        return true;

    ShCompileOptions compileOptions = compileOptionsIn;

    // Apply key workarounds.
    if (shouldFlattenPragmaStdglInvariantAll())
    {
        // This should be harmless to do in all cases, but for the moment, do it only conditionally.
        compileOptions.flattenPragmaSTDGLInvariantAll = true;
    }

    TScopedPoolAllocator scopedAlloc(&allocator);
    TIntermBlock *root = compileTreeImpl(shaderStrings, numStrings, compileOptions);

    if (root)
    {
        if (compileOptions.intermediateTree)
        {
            OutputTree(root, mInfoSink.info);
        }

        if (compileOptions.objectCode)
        {
            PerformanceDiagnostics perfDiagnostics(&mDiagnostics);
            if (!translate(root, compileOptions, &perfDiagnostics))
            {
                return false;
            }
        }

        if (mShaderType == GL_VERTEX_SHADER)
        {
            bool lookForDrawID =
                IsExtensionEnabled(mExtensionBehavior, TExtension::ANGLE_multi_draw) &&
                compileOptions.emulateGLDrawID;
            bool lookForBaseVertexBaseInstance =
                IsExtensionEnabled(mExtensionBehavior,
                                   TExtension::ANGLE_base_vertex_base_instance_shader_builtin) &&
                compileOptions.emulateGLBaseVertexBaseInstance;

            if (lookForDrawID || lookForBaseVertexBaseInstance)
            {
                for (auto &uniform : mUniforms)
                {
                    if (lookForDrawID && uniform.name == "angle_DrawID" &&
                        uniform.mappedName == "angle_DrawID")
                    {
                        uniform.name = "gl_DrawID";
                    }
                    else if (lookForBaseVertexBaseInstance && uniform.name == "angle_BaseVertex" &&
                             uniform.mappedName == "angle_BaseVertex")
                    {
                        uniform.name = "gl_BaseVertex";
                    }
                    else if (lookForBaseVertexBaseInstance &&
                             uniform.name == "angle_BaseInstance" &&
                             uniform.mappedName == "angle_BaseInstance")
                    {
                        uniform.name = "gl_BaseInstance";
                    }
                }
            }
        }

        // The IntermNode tree doesn't need to be deleted here, since the
        // memory will be freed in a big chunk by the PoolAllocator.
        return true;
    }
    return false;
}

bool TCompiler::initBuiltInSymbolTable(const ShBuiltInResources &resources)
{
    if (resources.MaxDrawBuffers < 1)
    {
        return false;
    }
    if (resources.EXT_blend_func_extended && resources.MaxDualSourceDrawBuffers < 1)
    {
        return false;
    }

    mSymbolTable.initializeBuiltIns(mShaderType, mShaderSpec, resources);

    return true;
}

void TCompiler::setResourceString()
{
    std::ostringstream strstream = sh::InitializeStream<std::ostringstream>();

    // clang-format off
    strstream << ":MaxVertexAttribs:" << mResources.MaxVertexAttribs
        << ":MaxVertexUniformVectors:" << mResources.MaxVertexUniformVectors
        << ":MaxVaryingVectors:" << mResources.MaxVaryingVectors
        << ":MaxVertexTextureImageUnits:" << mResources.MaxVertexTextureImageUnits
        << ":MaxCombinedTextureImageUnits:" << mResources.MaxCombinedTextureImageUnits
        << ":MaxTextureImageUnits:" << mResources.MaxTextureImageUnits
        << ":MaxFragmentUniformVectors:" << mResources.MaxFragmentUniformVectors
        << ":MaxDrawBuffers:" << mResources.MaxDrawBuffers
        << ":OES_standard_derivatives:" << mResources.OES_standard_derivatives
        << ":OES_EGL_image_external:" << mResources.OES_EGL_image_external
        << ":OES_EGL_image_external_essl3:" << mResources.OES_EGL_image_external_essl3
        << ":NV_EGL_stream_consumer_external:" << mResources.NV_EGL_stream_consumer_external
        << ":ARB_texture_rectangle:" << mResources.ARB_texture_rectangle
        << ":EXT_draw_buffers:" << mResources.EXT_draw_buffers
        << ":FragmentPrecisionHigh:" << mResources.FragmentPrecisionHigh
        << ":MaxExpressionComplexity:" << mResources.MaxExpressionComplexity
        << ":MaxStatementDepth:" << mResources.MaxStatementDepth
        << ":MaxCallStackDepth:" << mResources.MaxCallStackDepth
        << ":MaxFunctionParameters:" << mResources.MaxFunctionParameters
        << ":EXT_blend_func_extended:" << mResources.EXT_blend_func_extended
        << ":EXT_conservative_depth:" << mResources.EXT_conservative_depth
        << ":EXT_frag_depth:" << mResources.EXT_frag_depth
        << ":EXT_primitive_bounding_box:" << mResources.EXT_primitive_bounding_box
        << ":OES_primitive_bounding_box:" << mResources.OES_primitive_bounding_box
        << ":EXT_separate_shader_objects:" << mResources.EXT_separate_shader_objects
        << ":EXT_shader_texture_lod:" << mResources.EXT_shader_texture_lod
        << ":EXT_shader_framebuffer_fetch:" << mResources.EXT_shader_framebuffer_fetch
        << ":EXT_shader_framebuffer_fetch_non_coherent:" << mResources.EXT_shader_framebuffer_fetch_non_coherent
        << ":NV_shader_framebuffer_fetch:" << mResources.NV_shader_framebuffer_fetch
        << ":ARM_shader_framebuffer_fetch:" << mResources.ARM_shader_framebuffer_fetch
        << ":ARM_shader_framebuffer_fetch_depth_stencil:" << mResources.ARM_shader_framebuffer_fetch_depth_stencil
        << ":OVR_multiview2:" << mResources.OVR_multiview2
        << ":OVR_multiview:" << mResources.OVR_multiview
        << ":EXT_YUV_target:" << mResources.EXT_YUV_target
        << ":EXT_geometry_shader:" << mResources.EXT_geometry_shader
        << ":OES_geometry_shader:" << mResources.OES_geometry_shader
        << ":OES_shader_io_blocks:" << mResources.OES_shader_io_blocks
        << ":EXT_shader_io_blocks:" << mResources.EXT_shader_io_blocks
        << ":EXT_gpu_shader5:" << mResources.EXT_gpu_shader5
        << ":OES_texture_3D:" << mResources.OES_texture_3D
        << ":MaxVertexOutputVectors:" << mResources.MaxVertexOutputVectors
        << ":MaxFragmentInputVectors:" << mResources.MaxFragmentInputVectors
        << ":MinProgramTexelOffset:" << mResources.MinProgramTexelOffset
        << ":MaxProgramTexelOffset:" << mResources.MaxProgramTexelOffset
        << ":MaxDualSourceDrawBuffers:" << mResources.MaxDualSourceDrawBuffers
        << ":MaxViewsOVR:" << mResources.MaxViewsOVR
        << ":NV_draw_buffers:" << mResources.NV_draw_buffers
        << ":ANGLE_multi_draw:" << mResources.ANGLE_multi_draw
        << ":ANGLE_base_vertex_base_instance_shader_builtin:" << mResources.ANGLE_base_vertex_base_instance_shader_builtin
        << ":APPLE_clip_distance:" << mResources.APPLE_clip_distance
        << ":OES_texture_cube_map_array:" << mResources.OES_texture_cube_map_array
        << ":EXT_texture_cube_map_array:" << mResources.EXT_texture_cube_map_array
        << ":EXT_texture_query_lod:" << mResources.EXT_texture_query_lod
        << ":EXT_texture_shadow_lod:" << mResources.EXT_texture_shadow_lod
        << ":EXT_shadow_samplers:" << mResources.EXT_shadow_samplers
        << ":OES_shader_multisample_interpolation:" << mResources.OES_shader_multisample_interpolation
        << ":OES_shader_image_atomic:" << mResources.OES_shader_image_atomic
        << ":EXT_tessellation_shader:" << mResources.EXT_tessellation_shader
        << ":OES_tessellation_shader:" << mResources.OES_tessellation_shader
        << ":OES_texture_buffer:" << mResources.OES_texture_buffer
        << ":EXT_texture_buffer:" << mResources.EXT_texture_buffer
        << ":OES_sample_variables:" << mResources.OES_sample_variables
        << ":EXT_clip_cull_distance:" << mResources.EXT_clip_cull_distance
        << ":ANGLE_clip_cull_distance:" << mResources.ANGLE_clip_cull_distance
        << ":MinProgramTextureGatherOffset:" << mResources.MinProgramTextureGatherOffset
        << ":MaxProgramTextureGatherOffset:" << mResources.MaxProgramTextureGatherOffset
        << ":MaxImageUnits:" << mResources.MaxImageUnits
        << ":MaxSamples:" << mResources.MaxSamples
        << ":MaxVertexImageUniforms:" << mResources.MaxVertexImageUniforms
        << ":MaxFragmentImageUniforms:" << mResources.MaxFragmentImageUniforms
        << ":MaxComputeImageUniforms:" << mResources.MaxComputeImageUniforms
        << ":MaxCombinedImageUniforms:" << mResources.MaxCombinedImageUniforms
        << ":MaxCombinedShaderOutputResources:" << mResources.MaxCombinedShaderOutputResources
        << ":MaxComputeWorkGroupCountX:" << mResources.MaxComputeWorkGroupCount[0]
        << ":MaxComputeWorkGroupCountY:" << mResources.MaxComputeWorkGroupCount[1]
        << ":MaxComputeWorkGroupCountZ:" << mResources.MaxComputeWorkGroupCount[2]
        << ":MaxComputeWorkGroupSizeX:" << mResources.MaxComputeWorkGroupSize[0]
        << ":MaxComputeWorkGroupSizeY:" << mResources.MaxComputeWorkGroupSize[1]
        << ":MaxComputeWorkGroupSizeZ:" << mResources.MaxComputeWorkGroupSize[2]
        << ":MaxComputeUniformComponents:" << mResources.MaxComputeUniformComponents
        << ":MaxComputeTextureImageUnits:" << mResources.MaxComputeTextureImageUnits
        << ":MaxComputeAtomicCounters:" << mResources.MaxComputeAtomicCounters
        << ":MaxComputeAtomicCounterBuffers:" << mResources.MaxComputeAtomicCounterBuffers
        << ":MaxVertexAtomicCounters:" << mResources.MaxVertexAtomicCounters
        << ":MaxFragmentAtomicCounters:" << mResources.MaxFragmentAtomicCounters
        << ":MaxCombinedAtomicCounters:" << mResources.MaxCombinedAtomicCounters
        << ":MaxAtomicCounterBindings:" << mResources.MaxAtomicCounterBindings
        << ":MaxVertexAtomicCounterBuffers:" << mResources.MaxVertexAtomicCounterBuffers
        << ":MaxFragmentAtomicCounterBuffers:" << mResources.MaxFragmentAtomicCounterBuffers
        << ":MaxCombinedAtomicCounterBuffers:" << mResources.MaxCombinedAtomicCounterBuffers
        << ":MaxAtomicCounterBufferSize:" << mResources.MaxAtomicCounterBufferSize
        << ":MaxGeometryUniformComponents:" << mResources.MaxGeometryUniformComponents
        << ":MaxGeometryUniformBlocks:" << mResources.MaxGeometryUniformBlocks
        << ":MaxGeometryInputComponents:" << mResources.MaxGeometryInputComponents
        << ":MaxGeometryOutputComponents:" << mResources.MaxGeometryOutputComponents
        << ":MaxGeometryOutputVertices:" << mResources.MaxGeometryOutputVertices
        << ":MaxGeometryTotalOutputComponents:" << mResources.MaxGeometryTotalOutputComponents
        << ":MaxGeometryTextureImageUnits:" << mResources.MaxGeometryTextureImageUnits
        << ":MaxGeometryAtomicCounterBuffers:" << mResources.MaxGeometryAtomicCounterBuffers
        << ":MaxGeometryAtomicCounters:" << mResources.MaxGeometryAtomicCounters
        << ":MaxGeometryShaderStorageBlocks:" << mResources.MaxGeometryShaderStorageBlocks
        << ":MaxGeometryShaderInvocations:" << mResources.MaxGeometryShaderInvocations
        << ":MaxGeometryImageUniforms:" << mResources.MaxGeometryImageUniforms
        << ":MaxClipDistances" << mResources.MaxClipDistances
        << ":MaxCullDistances" << mResources.MaxCullDistances
        << ":MaxCombinedClipAndCullDistances" << mResources.MaxCombinedClipAndCullDistances
        << ":MaxTessControlInputComponents:" << mResources.MaxTessControlInputComponents
        << ":MaxTessControlOutputComponents:" << mResources.MaxTessControlOutputComponents
        << ":MaxTessControlTextureImageUnits:" << mResources.MaxTessControlTextureImageUnits
        << ":MaxTessControlUniformComponents:" << mResources.MaxTessControlUniformComponents
        << ":MaxTessControlTotalOutputComponents:" << mResources.MaxTessControlTotalOutputComponents
        << ":MaxTessControlImageUniforms:" << mResources.MaxTessControlImageUniforms
        << ":MaxTessControlAtomicCounters:" << mResources.MaxTessControlAtomicCounters
        << ":MaxTessControlAtomicCounterBuffers:" << mResources.MaxTessControlAtomicCounterBuffers
        << ":MaxTessPatchComponents:" << mResources.MaxTessPatchComponents
        << ":MaxPatchVertices:" << mResources.MaxPatchVertices
        << ":MaxTessGenLevel:" << mResources.MaxTessGenLevel
        << ":MaxTessEvaluationInputComponents:" << mResources.MaxTessEvaluationInputComponents
        << ":MaxTessEvaluationOutputComponents:" << mResources.MaxTessEvaluationOutputComponents
        << ":MaxTessEvaluationTextureImageUnits:" << mResources.MaxTessEvaluationTextureImageUnits
        << ":MaxTessEvaluationUniformComponents:" << mResources.MaxTessEvaluationUniformComponents
        << ":MaxTessEvaluationImageUniforms:" << mResources.MaxTessEvaluationImageUniforms
        << ":MaxTessEvaluationAtomicCounters:" << mResources.MaxTessEvaluationAtomicCounters
        << ":MaxTessEvaluationAtomicCounterBuffers:" << mResources.MaxTessEvaluationAtomicCounterBuffers;
    // clang-format on

    mBuiltInResourcesString = strstream.str();
}

void TCompiler::collectInterfaceBlocks()
{
    ASSERT(mInterfaceBlocks.empty());
    mInterfaceBlocks.reserve(mUniformBlocks.size() + mShaderStorageBlocks.size());
    mInterfaceBlocks.insert(mInterfaceBlocks.end(), mUniformBlocks.begin(), mUniformBlocks.end());
    mInterfaceBlocks.insert(mInterfaceBlocks.end(), mShaderStorageBlocks.begin(),
                            mShaderStorageBlocks.end());
}

void TCompiler::clearResults()
{
    mInfoSink.info.erase();
    mInfoSink.obj.erase();
    mInfoSink.debug.erase();
    mDiagnostics.resetErrorCount();

    mMetadataFlags.reset();
    mSpecConstUsageBits.reset();

    mAttributes.clear();
    mOutputVariables.clear();
    mUniforms.clear();
    mInputVaryings.clear();
    mOutputVaryings.clear();
    mSharedVariables.clear();
    mInterfaceBlocks.clear();
    mUniformBlocks.clear();
    mShaderStorageBlocks.clear();
    mVariablesCollected    = false;
    mGLPositionInitialized = false;

    mNumViews = -1;

    mClipDistanceSize = 0;
    mCullDistanceSize = 0;

    mGeometryShaderInputPrimitiveType  = EptUndefined;
    mGeometryShaderOutputPrimitiveType = EptUndefined;
    mGeometryShaderInvocations         = 0;
    mGeometryShaderMaxVertices         = -1;

    mTessControlShaderOutputVertices            = 0;
    mTessEvaluationShaderInputPrimitiveType     = EtetUndefined;
    mTessEvaluationShaderInputVertexSpacingType = EtetUndefined;
    mTessEvaluationShaderInputOrderingType      = EtetUndefined;
    mTessEvaluationShaderInputPointType         = EtetUndefined;

    mBuiltInFunctionEmulator.cleanup();

    mNameMap.clear();

    mSourcePath = nullptr;

    mSymbolTable.clearCompilationResults();
}

bool TCompiler::initCallDag(TIntermNode *root)
{
    mCallDag.clear();

    switch (mCallDag.init(root, &mDiagnostics))
    {
        case CallDAG::INITDAG_SUCCESS:
            return true;
        case CallDAG::INITDAG_RECURSION:
        case CallDAG::INITDAG_UNDEFINED:
            // Error message has already been written out.
            ASSERT(mDiagnostics.numErrors() > 0);
            return false;
    }

    UNREACHABLE();
    return true;
}

bool TCompiler::checkCallDepth()
{
    std::vector<int> depths(mCallDag.size());

    for (size_t i = 0; i < mCallDag.size(); i++)
    {
        int depth                     = 0;
        const CallDAG::Record &record = mCallDag.getRecordFromIndex(i);

        for (int calleeIndex : record.callees)
        {
            depth = std::max(depth, depths[calleeIndex] + 1);
        }

        depths[i] = depth;

        if (depth >= mResources.MaxCallStackDepth)
        {
            // Trace back the function chain to have a meaningful info log.
            std::stringstream errorStream = sh::InitializeStream<std::stringstream>();
            errorStream << "Call stack too deep (larger than " << mResources.MaxCallStackDepth
                        << ") with the following call chain: "
                        << record.node->getFunction()->name();

            int currentFunction = static_cast<int>(i);
            int currentDepth    = depth;

            while (currentFunction != -1)
            {
                errorStream
                    << " -> "
                    << mCallDag.getRecordFromIndex(currentFunction).node->getFunction()->name();

                int nextFunction = -1;
                for (const int &calleeIndex : mCallDag.getRecordFromIndex(currentFunction).callees)
                {
                    if (depths[calleeIndex] == currentDepth - 1)
                    {
                        currentDepth--;
                        nextFunction = calleeIndex;
                    }
                }

                currentFunction = nextFunction;
            }

            std::string errorStr = errorStream.str();
            mDiagnostics.globalError(errorStr.c_str());

            return false;
        }
    }

    return true;
}

bool TCompiler::tagUsedFunctions()
{
    // Search from main, starting from the end of the DAG as it usually is the root.
    for (size_t i = mCallDag.size(); i-- > 0;)
    {
        if (mCallDag.getRecordFromIndex(i).node->getFunction()->isMain())
        {
            internalTagUsedFunction(i);
            return true;
        }
    }

    mDiagnostics.globalError("Missing main()");
    return false;
}

void TCompiler::internalTagUsedFunction(size_t index)
{
    if (mFunctionMetadata[index].used)
    {
        return;
    }

    mFunctionMetadata[index].used = true;

    for (int calleeIndex : mCallDag.getRecordFromIndex(index).callees)
    {
        internalTagUsedFunction(calleeIndex);
    }
}

bool TCompiler::pruneUnusedFunctions(TIntermBlock *root)
{
    TIntermSequence *sequence = root->getSequence();

    size_t writeIndex = 0;
    for (size_t readIndex = 0; readIndex < sequence->size(); ++readIndex)
    {
        TIntermNode *node = sequence->at(readIndex);

        // Keep anything that's not unused.
        const TFunction *function = nullptr;
        const bool shouldPrune =
            IsTopLevelNodeUnusedFunction(mCallDag, mFunctionMetadata, node, &function);
        if (!shouldPrune)
        {
            (*sequence)[writeIndex++] = node;
            continue;
        }

        // If a function is unused, it may have a struct declaration in its return value which
        // shouldn't be pruned.  In that case, replace the function definition with the struct
        // definition.
        ASSERT(function != nullptr);
        const TType &returnType = function->getReturnType();
        if (!returnType.isStructSpecifier())
        {
            continue;
        }

        TVariable *structVariable =
            new TVariable(&mSymbolTable, kEmptyImmutableString, &returnType, SymbolType::Empty);
        TIntermSymbol *structSymbol           = new TIntermSymbol(structVariable);
        TIntermDeclaration *structDeclaration = new TIntermDeclaration;
        structDeclaration->appendDeclarator(structSymbol);

        structSymbol->setLine(node->getLine());
        structDeclaration->setLine(node->getLine());

        (*sequence)[writeIndex++] = structDeclaration;
    }

    sequence->resize(writeIndex);

    return validateAST(root);
}

bool TCompiler::limitExpressionComplexity(TIntermBlock *root)
{
    if (!IsASTDepthBelowLimit(root, mResources.MaxExpressionComplexity))
    {
        mDiagnostics.globalError("Expression too complex.");
        return false;
    }

    if (!ValidateMaxParameters(root, mResources.MaxFunctionParameters))
    {
        mDiagnostics.globalError("Function has too many parameters.");
        return false;
    }

    return true;
}

bool TCompiler::initializeGLPosition(TIntermBlock *root)
{
    InitVariableList list;
    AddBuiltInToInitList(&mSymbolTable, mShaderVersion, root, "gl_Position", &list);

    if (!list.empty())
    {
        return InitializeVariables(this, root, list, &mSymbolTable, mShaderVersion,
                                   mExtensionBehavior, false, false);
    }

    return true;
}

bool TCompiler::useAllMembersInUnusedStandardAndSharedBlocks(TIntermBlock *root)
{
    sh::InterfaceBlockList list;

    for (const sh::InterfaceBlock &block : mUniformBlocks)
    {
        if (!block.staticUse &&
            (block.layout == sh::BLOCKLAYOUT_STD140 || block.layout == sh::BLOCKLAYOUT_SHARED))
        {
            list.push_back(block);
        }
    }

    return sh::UseInterfaceBlockFields(this, root, list, mSymbolTable);
}

bool TCompiler::initializeOutputVariables(TIntermBlock *root)
{
    InitVariableList list;

    for (TIntermNode *node : *root->getSequence())
    {
        TIntermDeclaration *asDecl = node->getAsDeclarationNode();
        if (asDecl == nullptr)
        {
            continue;
        }

        TIntermSymbol *symbol = asDecl->getSequence()->front()->getAsSymbolNode();
        if (symbol == nullptr)
        {
            TIntermBinary *initNode = asDecl->getSequence()->front()->getAsBinaryNode();
            ASSERT(initNode->getOp() == EOpInitialize);
            symbol = initNode->getLeft()->getAsSymbolNode();
        }
        ASSERT(symbol);

        // inout variables represent the context of the framebuffer when the draw call starts, so
        // they have to be considered as already initialized.
        const TQualifier qualifier = symbol->getType().getQualifier();
        if (qualifier != EvqFragmentInOut && IsShaderOut(symbol->getType().getQualifier()))
        {
            list.push_back(&symbol->variable());
        }
    }

    // Initialize built-in outputs as well.
    const std::vector<ShaderVariable> &outputVariables =
        mShaderType == GL_FRAGMENT_SHADER ? mOutputVariables : mOutputVaryings;

    for (const ShaderVariable &var : outputVariables)
    {
        if (var.isFragmentInOut || !var.isBuiltIn())
        {
            continue;
        }

        AddBuiltInToInitList(&mSymbolTable, mShaderVersion, root, var.name.c_str(), &list);
        if (var.name == "gl_Position")
        {
            ASSERT(!mGLPositionInitialized);
            mGLPositionInitialized = true;
        }
    }

    return InitializeVariables(this, root, list, &mSymbolTable, mShaderVersion, mExtensionBehavior,
                               false, false);
}

const TExtensionBehavior &TCompiler::getExtensionBehavior() const
{
    return mExtensionBehavior;
}

const char *TCompiler::getSourcePath() const
{
    return mSourcePath;
}

const ShBuiltInResources &TCompiler::getResources() const
{
    return mResources;
}

const BuiltInFunctionEmulator &TCompiler::getBuiltInFunctionEmulator() const
{
    return mBuiltInFunctionEmulator;
}

bool TCompiler::isVaryingDefined(const char *varyingName)
{
    ASSERT(mVariablesCollected);
    for (size_t ii = 0; ii < mInputVaryings.size(); ++ii)
    {
        if (mInputVaryings[ii].name == varyingName)
        {
            return true;
        }
    }
    for (size_t ii = 0; ii < mOutputVaryings.size(); ++ii)
    {
        if (mOutputVaryings[ii].name == varyingName)
        {
            return true;
        }
    }

    return false;
}

}  // namespace sh
