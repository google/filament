//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// OutputSPIRV: Generate SPIR-V from the AST.
//

#include "compiler/translator/spirv/OutputSPIRV.h"

#include "angle_gl.h"
#include "common/debug.h"
#include "common/mathutil.h"
#include "common/spirv/spirv_instruction_builder_autogen.h"
#include "compiler/translator/Compiler.h"
#include "compiler/translator/StaticType.h"
#include "compiler/translator/spirv/BuildSPIRV.h"
#include "compiler/translator/tree_util/FindPreciseNodes.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

#include <cfloat>

// Extended instructions
namespace spv
{
#include <spirv/unified1/GLSL.std.450.h>
}

// SPIR-V tools include for disassembly
#include <spirv-tools/libspirv.hpp>

// Enable this for debug logging of pre-transform SPIR-V:
#if !defined(ANGLE_DEBUG_SPIRV_GENERATION)
#    define ANGLE_DEBUG_SPIRV_GENERATION 0
#endif  // !defined(ANGLE_DEBUG_SPIRV_GENERATION)

namespace sh
{
namespace
{
// A struct to hold either SPIR-V ids or literal constants.   If id is not valid, a literal is
// assumed.
struct SpirvIdOrLiteral
{
    SpirvIdOrLiteral() = default;
    SpirvIdOrLiteral(const spirv::IdRef idIn) : id(idIn) {}
    SpirvIdOrLiteral(const spirv::LiteralInteger literalIn) : literal(literalIn) {}

    spirv::IdRef id;
    spirv::LiteralInteger literal;
};

// A data structure to facilitate generating array indexing, block field selection, swizzle and
// such.  Used in conjunction with NodeData which includes the access chain's baseId and idList.
//
// - rvalue[literal].field[literal] generates OpCompositeExtract
// - rvalue.x generates OpCompositeExtract
// - rvalue.xyz generates OpVectorShuffle
// - rvalue.xyz[i] generates OpVectorExtractDynamic (xyz[i] itself generates an
//   OpVectorExtractDynamic as well)
// - rvalue[i].field[j] generates a temp variable OpStore'ing rvalue and then generating an
//   OpAccessChain and OpLoad
//
// - lvalue[i].field[j].x generates OpAccessChain and OpStore
// - lvalue.xyz generates an OpLoad followed by OpVectorShuffle and OpStore
// - lvalue.xyz[i] generates OpAccessChain and OpStore (xyz[i] itself generates an
//   OpVectorExtractDynamic as well)
//
// storageClass == Max implies an rvalue.
//
struct AccessChain
{
    // The storage class for lvalues.  If Max, it's an rvalue.
    spv::StorageClass storageClass = spv::StorageClassMax;
    // If the access chain ends in swizzle, the swizzle components are specified here.  Swizzles
    // select multiple components so need special treatment when used as lvalue.
    std::vector<uint32_t> swizzles;
    // If a vector component is selected dynamically (i.e. indexed with a non-literal index),
    // dynamicComponent will contain the id of the index.
    spirv::IdRef dynamicComponent;

    // Type of base expression, before swizzle is applied, after swizzle is applied and after
    // dynamic component is applied.
    spirv::IdRef baseTypeId;
    spirv::IdRef preSwizzleTypeId;
    spirv::IdRef postSwizzleTypeId;
    spirv::IdRef postDynamicComponentTypeId;

    // If the OpAccessChain is already generated (done by accessChainCollapse()), this caches the
    // id.
    spirv::IdRef accessChainId;

    // Whether all indices are literal.  Avoids looping through indices to determine this
    // information.
    bool areAllIndicesLiteral = true;
    // The number of components in the vector, if vector and swizzle is used.  This is cached to
    // avoid a type look up when handling swizzles.
    uint8_t swizzledVectorComponentCount = 0;

    // SPIR-V type specialization due to the base type.  Used to correctly select the SPIR-V type
    // id when visiting EOpIndex* binary nodes (i.e. reading from or writing to an access chain).
    // This always corresponds to the specialization specific to the end result of the access chain,
    // not the base or any intermediary types.  For example, a struct nested in a column-major
    // interface block, with a parent block qualified as row-major would specify row-major here.
    SpirvTypeSpec typeSpec;
};

// As each node is traversed, it produces data.  When visiting back the parent, this data is used to
// complete the data of the parent.  For example, the children of a function call (i.e. the
// arguments) each produce a SPIR-V id corresponding to the result of their expression.  The
// function call node itself in PostVisit uses those ids to generate the function call instruction.
struct NodeData
{
    // An id whose meaning depends on the node.  It could be a temporary id holding the result of an
    // expression, a reference to a variable etc.
    spirv::IdRef baseId;

    // List of relevant SPIR-V ids accumulated while traversing the children.  Meaning depends on
    // the node, for example a list of parameters to be passed to a function, a set of ids used to
    // construct an access chain etc.
    std::vector<SpirvIdOrLiteral> idList;

    // For constructing access chains.
    AccessChain accessChain;
};

struct FunctionIds
{
    // Id of the function type, return type and parameter types.
    spirv::IdRef functionTypeId;
    spirv::IdRef returnTypeId;
    spirv::IdRefList parameterTypeIds;

    // Id of the function itself.
    spirv::IdRef functionId;
};

struct BuiltInResultStruct
{
    // Some builtins require a struct result.  The struct always has two fields of a scalar or
    // vector type.
    TBasicType lsbType;
    TBasicType msbType;
    uint32_t lsbPrimarySize;
    uint32_t msbPrimarySize;
};

struct BuiltInResultStructHash
{
    size_t operator()(const BuiltInResultStruct &key) const
    {
        static_assert(sh::EbtLast < 256, "Basic type doesn't fit in uint8_t");
        ASSERT(key.lsbPrimarySize > 0 && key.lsbPrimarySize <= 4);
        ASSERT(key.msbPrimarySize > 0 && key.msbPrimarySize <= 4);

        const uint8_t properties[4] = {
            static_cast<uint8_t>(key.lsbType),
            static_cast<uint8_t>(key.msbType),
            static_cast<uint8_t>(key.lsbPrimarySize),
            static_cast<uint8_t>(key.msbPrimarySize),
        };

        return angle::ComputeGenericHash(properties, sizeof(properties));
    }
};

bool operator==(const BuiltInResultStruct &a, const BuiltInResultStruct &b)
{
    return a.lsbType == b.lsbType && a.msbType == b.msbType &&
           a.lsbPrimarySize == b.lsbPrimarySize && a.msbPrimarySize == b.msbPrimarySize;
}

bool IsAccessChainRValue(const AccessChain &accessChain)
{
    return accessChain.storageClass == spv::StorageClassMax;
}

// A traverser that generates SPIR-V as it walks the AST.
class OutputSPIRVTraverser : public TIntermTraverser
{
  public:
    OutputSPIRVTraverser(TCompiler *compiler,
                         const ShCompileOptions &compileOptions,
                         const angle::HashMap<int, uint32_t> &uniqueToSpirvIdMap,
                         uint32_t firstUnusedSpirvId);
    ~OutputSPIRVTraverser() override;

    spirv::Blob getSpirv();

  protected:
    void visitSymbol(TIntermSymbol *node) override;
    void visitConstantUnion(TIntermConstantUnion *node) override;
    bool visitSwizzle(Visit visit, TIntermSwizzle *node) override;
    bool visitBinary(Visit visit, TIntermBinary *node) override;
    bool visitUnary(Visit visit, TIntermUnary *node) override;
    bool visitTernary(Visit visit, TIntermTernary *node) override;
    bool visitIfElse(Visit visit, TIntermIfElse *node) override;
    bool visitSwitch(Visit visit, TIntermSwitch *node) override;
    bool visitCase(Visit visit, TIntermCase *node) override;
    void visitFunctionPrototype(TIntermFunctionPrototype *node) override;
    bool visitFunctionDefinition(Visit visit, TIntermFunctionDefinition *node) override;
    bool visitAggregate(Visit visit, TIntermAggregate *node) override;
    bool visitBlock(Visit visit, TIntermBlock *node) override;
    bool visitGlobalQualifierDeclaration(Visit visit,
                                         TIntermGlobalQualifierDeclaration *node) override;
    bool visitDeclaration(Visit visit, TIntermDeclaration *node) override;
    bool visitLoop(Visit visit, TIntermLoop *node) override;
    bool visitBranch(Visit visit, TIntermBranch *node) override;
    void visitPreprocessorDirective(TIntermPreprocessorDirective *node) override;

  private:
    spirv::IdRef getSymbolIdAndStorageClass(const TSymbol *symbol,
                                            const TType &type,
                                            spv::StorageClass *storageClass);

    // Access chain handling.

    // Called before pushing indices to access chain to adjust |typeSpec| (which is then used to
    // determine the typeId passed to |accessChainPush*|).
    void accessChainOnPush(NodeData *data, const TType &parentType, size_t index);
    void accessChainPush(NodeData *data, spirv::IdRef index, spirv::IdRef typeId) const;
    void accessChainPushLiteral(NodeData *data,
                                spirv::LiteralInteger index,
                                spirv::IdRef typeId) const;
    void accessChainPushSwizzle(NodeData *data,
                                const TVector<int> &swizzle,
                                spirv::IdRef typeId,
                                uint8_t componentCount) const;
    void accessChainPushDynamicComponent(NodeData *data, spirv::IdRef index, spirv::IdRef typeId);
    spirv::IdRef accessChainCollapse(NodeData *data);
    spirv::IdRef accessChainLoad(NodeData *data,
                                 const TType &valueType,
                                 spirv::IdRef *resultTypeIdOut);
    void accessChainStore(NodeData *data, spirv::IdRef value, const TType &valueType);

    // Access chain helpers.
    void makeAccessChainIdList(NodeData *data, spirv::IdRefList *idsOut);
    void makeAccessChainLiteralList(NodeData *data, spirv::LiteralIntegerList *literalsOut);
    spirv::IdRef getAccessChainTypeId(NodeData *data);

    // Node data handling.
    void nodeDataInitLValue(NodeData *data,
                            spirv::IdRef baseId,
                            spirv::IdRef typeId,
                            spv::StorageClass storageClass,
                            const SpirvTypeSpec &typeSpec) const;
    void nodeDataInitRValue(NodeData *data, spirv::IdRef baseId, spirv::IdRef typeId) const;

    void declareConst(TIntermDeclaration *decl);
    void declareSpecConst(TIntermDeclaration *decl);
    spirv::IdRef createConstant(const TType &type,
                                TBasicType expectedBasicType,
                                const TConstantUnion *constUnion,
                                bool isConstantNullValue);
    spirv::IdRef createComplexConstant(const TType &type,
                                       spirv::IdRef typeId,
                                       const spirv::IdRefList &parameters);
    spirv::IdRef createConstructor(TIntermAggregate *node, spirv::IdRef typeId);
    spirv::IdRef createArrayOrStructConstructor(TIntermAggregate *node,
                                                spirv::IdRef typeId,
                                                const spirv::IdRefList &parameters);
    spirv::IdRef createConstructorScalarFromNonScalar(TIntermAggregate *node,
                                                      spirv::IdRef typeId,
                                                      const spirv::IdRefList &parameters);
    spirv::IdRef createConstructorVectorFromScalar(const TType &parameterType,
                                                   const TType &expectedType,
                                                   spirv::IdRef typeId,
                                                   const spirv::IdRefList &parameters);
    spirv::IdRef createConstructorVectorFromMatrix(TIntermAggregate *node,
                                                   spirv::IdRef typeId,
                                                   const spirv::IdRefList &parameters);
    spirv::IdRef createConstructorVectorFromMultiple(TIntermAggregate *node,
                                                     spirv::IdRef typeId,
                                                     const spirv::IdRefList &parameters);
    spirv::IdRef createConstructorMatrixFromScalar(TIntermAggregate *node,
                                                   spirv::IdRef typeId,
                                                   const spirv::IdRefList &parameters);
    spirv::IdRef createConstructorMatrixFromVectors(TIntermAggregate *node,
                                                    spirv::IdRef typeId,
                                                    const spirv::IdRefList &parameters);
    spirv::IdRef createConstructorMatrixFromMatrix(TIntermAggregate *node,
                                                   spirv::IdRef typeId,
                                                   const spirv::IdRefList &parameters);
    // Load N values where N is the number of node's children.  In some cases, the last M values are
    // lvalues which should be skipped.
    spirv::IdRefList loadAllParams(TIntermOperator *node,
                                   size_t skipCount,
                                   spirv::IdRefList *paramTypeIds);
    void extractComponents(TIntermAggregate *node,
                           size_t componentCount,
                           const spirv::IdRefList &parameters,
                           spirv::IdRefList *extractedComponentsOut);

    void startShortCircuit(TIntermBinary *node);
    spirv::IdRef endShortCircuit(TIntermBinary *node, spirv::IdRef *typeId);

    spirv::IdRef visitOperator(TIntermOperator *node, spirv::IdRef resultTypeId);
    spirv::IdRef createCompare(TIntermOperator *node, spirv::IdRef resultTypeId);
    spirv::IdRef createAtomicBuiltIn(TIntermOperator *node, spirv::IdRef resultTypeId);
    spirv::IdRef createImageTextureBuiltIn(TIntermOperator *node, spirv::IdRef resultTypeId);
    spirv::IdRef createSubpassLoadBuiltIn(TIntermOperator *node, spirv::IdRef resultTypeId);
    spirv::IdRef createInterpolate(TIntermOperator *node, spirv::IdRef resultTypeId);

    spirv::IdRef createFunctionCall(TIntermAggregate *node, spirv::IdRef resultTypeId);

    void visitArrayLength(TIntermUnary *node);

    // Cast between types.  There are two kinds of casts:
    //
    // - A constructor can cast between basic types, for example vec4(someInt).
    // - Assignments, constructors, function calls etc may copy an array or struct between different
    //   block storages, invariance etc (which due to their decorations generate different SPIR-V
    //   types).  For example:
    //
    //       layout(std140) uniform U { invariant Struct s; } u; ... Struct s2 = u.s;
    //
    spirv::IdRef castBasicType(spirv::IdRef value,
                               const TType &valueType,
                               const TType &expectedType,
                               spirv::IdRef *resultTypeIdOut);
    spirv::IdRef cast(spirv::IdRef value,
                      const TType &valueType,
                      const SpirvTypeSpec &valueTypeSpec,
                      const SpirvTypeSpec &expectedTypeSpec,
                      spirv::IdRef *resultTypeIdOut);

    // Given a list of parameters to an operator, extend the scalars to match the vectors.  GLSL
    // frequently has operators that mix vectors and scalars, while SPIR-V usually applies the
    // operations per component (requiring the scalars to turn into a vector).
    void extendScalarParamsToVector(TIntermOperator *node,
                                    spirv::IdRef resultTypeId,
                                    spirv::IdRefList *parameters);

    // Helper to reduce vector == and != with OpAll and OpAny respectively.  If multiple ids are
    // given, either OpLogicalAnd or OpLogicalOr is used (if two operands) or a bool vector is
    // constructed and OpAll and OpAny used.
    spirv::IdRef reduceBoolVector(TOperator op,
                                  const spirv::IdRefList &valueIds,
                                  spirv::IdRef typeId,
                                  const SpirvDecorations &decorations);
    // Helper to implement == and !=, supporting vectors, matrices, structs and arrays.
    void createCompareImpl(TOperator op,
                           const TType &operandType,
                           spirv::IdRef resultTypeId,
                           spirv::IdRef leftId,
                           spirv::IdRef rightId,
                           const SpirvDecorations &operandDecorations,
                           const SpirvDecorations &resultDecorations,
                           spirv::LiteralIntegerList *currentAccessChain,
                           spirv::IdRefList *intermediateResultsOut);

    // For some builtins, SPIR-V outputs two values in a struct.  This function defines such a
    // struct if not already defined.
    spirv::IdRef makeBuiltInOutputStructType(TIntermOperator *node, size_t lvalueCount);
    // Once the builtin instruction is generated, the two return values are extracted from the
    // struct.  These are written to the return value (if any) and the out parameters.
    void storeBuiltInStructOutputInParamsAndReturnValue(TIntermOperator *node,
                                                        size_t lvalueCount,
                                                        spirv::IdRef structValue,
                                                        spirv::IdRef returnValue,
                                                        spirv::IdRef returnValueType);
    void storeBuiltInStructOutputInParamHelper(NodeData *data,
                                               TIntermTyped *param,
                                               spirv::IdRef structValue,
                                               uint32_t fieldIndex);

    void markVertexOutputOnShaderEnd();
    void markVertexOutputOnEmitVertex();

    TCompiler *mCompiler;
    ANGLE_MAYBE_UNUSED_PRIVATE_FIELD const ShCompileOptions &mCompileOptions;

    SPIRVBuilder mBuilder;

    // Traversal state.  Nodes generally push() once to this stack on PreVisit.  On InVisit and
    // PostVisit, they pop() once (data corresponding to the result of the child) and accumulate it
    // in back() (data corresponding to the node itself).  On PostVisit, code is generated.
    std::vector<NodeData> mNodeData;

    // A map of TSymbol to its SPIR-V id.  This could be a:
    //
    // - TVariable, or
    // - TInterfaceBlock: because TIntermSymbols referencing a field of an unnamed interface block
    //   don't reference the TVariable that defines the struct, but the TInterfaceBlock itself.
    angle::HashMap<const TSymbol *, spirv::IdRef> mSymbolIdMap;

    // A map of TFunction to its various SPIR-V ids.
    angle::HashMap<const TFunction *, FunctionIds> mFunctionIdMap;

    // A map of internally defined structs used to capture result of some SPIR-V instructions.
    angle::HashMap<BuiltInResultStruct, spirv::IdRef, BuiltInResultStructHash>
        mBuiltInResultStructMap;

    // Whether the current symbol being visited is being declared.
    bool mIsSymbolBeingDeclared = false;

    // What is the id of the current function being generated.
    spirv::IdRef mCurrentFunctionId;
};

spv::StorageClass GetStorageClass(const ShCompileOptions &compileOptions,
                                  const TType &type,
                                  GLenum shaderType)
{
    // Opaque uniforms (samplers, images and subpass inputs) have the UniformConstant storage class
    if (IsOpaqueType(type.getBasicType()))
    {
        return spv::StorageClassUniformConstant;
    }

    const TQualifier qualifier = type.getQualifier();

    // Input varying and IO blocks have the Input storage class
    if (IsShaderIn(qualifier))
    {
        return spv::StorageClassInput;
    }

    // Output varying and IO blocks have the Input storage class
    if (IsShaderOut(qualifier))
    {
        return spv::StorageClassOutput;
    }

    switch (qualifier)
    {
        case EvqShared:
            // Compute shader shared memory has the Workgroup storage class
            return spv::StorageClassWorkgroup;

        case EvqGlobal:
        case EvqConst:
            // Global variables have the Private class.  Complex constant variables that are not
            // folded are also defined globally.
            return spv::StorageClassPrivate;

        case EvqTemporary:
        case EvqParamIn:
        case EvqParamOut:
        case EvqParamInOut:
            // Function-local variables have the Function class
            return spv::StorageClassFunction;

        case EvqVertexID:
        case EvqInstanceID:
        case EvqFragCoord:
        case EvqFrontFacing:
        case EvqPointCoord:
        case EvqSampleID:
        case EvqSamplePosition:
        case EvqSampleMaskIn:
        case EvqPatchVerticesIn:
        case EvqTessCoord:
        case EvqPrimitiveIDIn:
        case EvqInvocationID:
        case EvqHelperInvocation:
        case EvqNumWorkGroups:
        case EvqWorkGroupID:
        case EvqLocalInvocationID:
        case EvqGlobalInvocationID:
        case EvqLocalInvocationIndex:
        case EvqViewIDOVR:
        case EvqLayerIn:
        case EvqLastFragColor:
        case EvqLastFragDepth:
        case EvqLastFragStencil:
            return spv::StorageClassInput;

        case EvqPosition:
        case EvqPointSize:
        case EvqFragDepth:
        case EvqSampleMask:
        case EvqLayerOut:
            return spv::StorageClassOutput;

        case EvqClipDistance:
        case EvqCullDistance:
            // gl_Clip/CullDistance (not accessed through gl_in/gl_out) are inputs in FS and outputs
            // otherwise.
            return shaderType == GL_FRAGMENT_SHADER ? spv::StorageClassInput
                                                    : spv::StorageClassOutput;

        case EvqTessLevelOuter:
        case EvqTessLevelInner:
            // gl_TessLevelOuter/Inner are outputs in TCS and inputs in TES.
            return shaderType == GL_TESS_CONTROL_SHADER_EXT ? spv::StorageClassOutput
                                                            : spv::StorageClassInput;

        case EvqPrimitiveID:
            // gl_PrimitiveID is output in GS and input in TCS, TES and FS.
            return shaderType == GL_GEOMETRY_SHADER ? spv::StorageClassOutput
                                                    : spv::StorageClassInput;

        default:
            // Uniform buffers have the Uniform storage class.  Storage buffers have the Uniform
            // storage class in SPIR-V 1.3, and the StorageBuffer storage class in SPIR-V 1.4.
            // Default uniforms are gathered in a uniform block as well.  Push constants use the
            // PushConstant storage class instead.
            ASSERT(type.getInterfaceBlock() != nullptr || qualifier == EvqUniform);
            // I/O blocks must have already been classified as input or output above.
            ASSERT(!IsShaderIoBlock(qualifier));

            if (type.getLayoutQualifier().pushConstant)
            {
                ASSERT(type.getInterfaceBlock() != nullptr);
                return spv::StorageClassPushConstant;
            }
            return compileOptions.emitSPIRV14 && qualifier == EvqBuffer
                       ? spv::StorageClassStorageBuffer
                       : spv::StorageClassUniform;
    }
}

OutputSPIRVTraverser::OutputSPIRVTraverser(TCompiler *compiler,
                                           const ShCompileOptions &compileOptions,
                                           const angle::HashMap<int, uint32_t> &uniqueToSpirvIdMap,
                                           uint32_t firstUnusedSpirvId)
    : TIntermTraverser(true, true, true, &compiler->getSymbolTable()),
      mCompiler(compiler),
      mCompileOptions(compileOptions),
      mBuilder(compiler, compileOptions, uniqueToSpirvIdMap, firstUnusedSpirvId)
{}

OutputSPIRVTraverser::~OutputSPIRVTraverser()
{
    ASSERT(mNodeData.empty());
}

spirv::IdRef OutputSPIRVTraverser::getSymbolIdAndStorageClass(const TSymbol *symbol,
                                                              const TType &type,
                                                              spv::StorageClass *storageClass)
{
    *storageClass = GetStorageClass(mCompileOptions, type, mCompiler->getShaderType());
    auto iter     = mSymbolIdMap.find(symbol);
    if (iter != mSymbolIdMap.end())
    {
        return iter->second;
    }

    // This must be an implicitly defined variable, define it now.
    const char *name                = nullptr;
    spv::BuiltIn builtInDecoration  = spv::BuiltInMax;
    const TSymbolUniqueId *uniqueId = nullptr;

    switch (type.getQualifier())
    {
        // Vertex shader built-ins
        case EvqVertexID:
            name              = "gl_VertexIndex";
            builtInDecoration = spv::BuiltInVertexIndex;
            break;
        case EvqInstanceID:
            name              = "gl_InstanceIndex";
            builtInDecoration = spv::BuiltInInstanceIndex;
            break;

        // Fragment shader built-ins
        case EvqFragCoord:
            name              = "gl_FragCoord";
            builtInDecoration = spv::BuiltInFragCoord;
            break;
        case EvqFrontFacing:
            name              = "gl_FrontFacing";
            builtInDecoration = spv::BuiltInFrontFacing;
            break;
        case EvqPointCoord:
            name              = "gl_PointCoord";
            builtInDecoration = spv::BuiltInPointCoord;
            break;
        case EvqFragDepth:
            name              = "gl_FragDepth";
            builtInDecoration = spv::BuiltInFragDepth;
            mBuilder.addExecutionMode(spv::ExecutionModeDepthReplacing);
            switch (type.getLayoutQualifier().depth)
            {
                case EdGreater:
                    mBuilder.addExecutionMode(spv::ExecutionModeDepthGreater);
                    break;
                case EdLess:
                    mBuilder.addExecutionMode(spv::ExecutionModeDepthLess);
                    break;
                case EdUnchanged:
                    mBuilder.addExecutionMode(spv::ExecutionModeDepthUnchanged);
                    break;
                default:
                    break;
            }
            break;
        case EvqSampleMask:
            name              = "gl_SampleMask";
            builtInDecoration = spv::BuiltInSampleMask;
            break;
        case EvqSampleMaskIn:
            name              = "gl_SampleMaskIn";
            builtInDecoration = spv::BuiltInSampleMask;
            break;
        case EvqSampleID:
            name              = "gl_SampleID";
            builtInDecoration = spv::BuiltInSampleId;
            mBuilder.addCapability(spv::CapabilitySampleRateShading);
            uniqueId = &symbol->uniqueId();
            break;
        case EvqSamplePosition:
            name              = "gl_SamplePosition";
            builtInDecoration = spv::BuiltInSamplePosition;
            mBuilder.addCapability(spv::CapabilitySampleRateShading);
            break;
        case EvqClipDistance:
            name              = "gl_ClipDistance";
            builtInDecoration = spv::BuiltInClipDistance;
            mBuilder.addCapability(spv::CapabilityClipDistance);
            break;
        case EvqCullDistance:
            name              = "gl_CullDistance";
            builtInDecoration = spv::BuiltInCullDistance;
            mBuilder.addCapability(spv::CapabilityCullDistance);
            break;
        case EvqHelperInvocation:
            name              = "gl_HelperInvocation";
            builtInDecoration = spv::BuiltInHelperInvocation;
            break;

        // Tessellation built-ins
        case EvqPatchVerticesIn:
            name              = "gl_PatchVerticesIn";
            builtInDecoration = spv::BuiltInPatchVertices;
            break;
        case EvqTessLevelOuter:
            name              = "gl_TessLevelOuter";
            builtInDecoration = spv::BuiltInTessLevelOuter;
            break;
        case EvqTessLevelInner:
            name              = "gl_TessLevelInner";
            builtInDecoration = spv::BuiltInTessLevelInner;
            break;
        case EvqTessCoord:
            name              = "gl_TessCoord";
            builtInDecoration = spv::BuiltInTessCoord;
            break;

        // Shared geometry and tessellation built-ins
        case EvqInvocationID:
            name              = "gl_InvocationID";
            builtInDecoration = spv::BuiltInInvocationId;
            break;
        case EvqPrimitiveID:
            name              = "gl_PrimitiveID";
            builtInDecoration = spv::BuiltInPrimitiveId;

            // In fragment shader, add the Geometry capability.
            if (mCompiler->getShaderType() == GL_FRAGMENT_SHADER)
            {
                mBuilder.addCapability(spv::CapabilityGeometry);
            }

            break;

        // Geometry shader built-ins
        case EvqPrimitiveIDIn:
            name              = "gl_PrimitiveIDIn";
            builtInDecoration = spv::BuiltInPrimitiveId;
            break;
        case EvqLayerOut:
        case EvqLayerIn:
            name              = "gl_Layer";
            builtInDecoration = spv::BuiltInLayer;

            // gl_Layer requires the Geometry capability, even in fragment shaders.
            mBuilder.addCapability(spv::CapabilityGeometry);

            break;

        // Compute shader built-ins
        case EvqNumWorkGroups:
            name              = "gl_NumWorkGroups";
            builtInDecoration = spv::BuiltInNumWorkgroups;
            break;
        case EvqWorkGroupID:
            name              = "gl_WorkGroupID";
            builtInDecoration = spv::BuiltInWorkgroupId;
            break;
        case EvqLocalInvocationID:
            name              = "gl_LocalInvocationID";
            builtInDecoration = spv::BuiltInLocalInvocationId;
            break;
        case EvqGlobalInvocationID:
            name              = "gl_GlobalInvocationID";
            builtInDecoration = spv::BuiltInGlobalInvocationId;
            break;
        case EvqLocalInvocationIndex:
            name              = "gl_LocalInvocationIndex";
            builtInDecoration = spv::BuiltInLocalInvocationIndex;
            break;

        // Extensions
        case EvqViewIDOVR:
            name              = "gl_ViewID_OVR";
            builtInDecoration = spv::BuiltInViewIndex;
            mBuilder.addCapability(spv::CapabilityMultiView);
            mBuilder.addExtension(SPIRVExtensions::MultiviewOVR);
            break;

        default:
            UNREACHABLE();
    }

    const spirv::IdRef typeId = mBuilder.getTypeData(type, {}).id;
    const spirv::IdRef varId  = mBuilder.declareVariable(
        typeId, *storageClass, mBuilder.getDecorations(type), nullptr, name, uniqueId);

    spirv::WriteDecorate(mBuilder.getSpirvDecorations(), varId, spv::DecorationBuiltIn,
                         {spirv::LiteralInteger(builtInDecoration)});

    // Additionally:
    //
    // - decorate int inputs in FS with Flat (gl_Layer, gl_SampleID, gl_PrimitiveID, gl_ViewID_OVR).
    // - decorate gl_TessLevel* with Patch.
    switch (type.getQualifier())
    {
        case EvqLayerIn:
        case EvqSampleID:
        case EvqPrimitiveID:
        case EvqViewIDOVR:
            if (mCompiler->getShaderType() == GL_FRAGMENT_SHADER)
            {
                spirv::WriteDecorate(mBuilder.getSpirvDecorations(), varId, spv::DecorationFlat,
                                     {});
            }
            break;
        case EvqTessLevelInner:
        case EvqTessLevelOuter:
            spirv::WriteDecorate(mBuilder.getSpirvDecorations(), varId, spv::DecorationPatch, {});
            break;
        default:
            break;
    }

    mSymbolIdMap.insert({symbol, varId});
    return varId;
}

void OutputSPIRVTraverser::nodeDataInitLValue(NodeData *data,
                                              spirv::IdRef baseId,
                                              spirv::IdRef typeId,
                                              spv::StorageClass storageClass,
                                              const SpirvTypeSpec &typeSpec) const
{
    *data = {};

    // Initialize the access chain as an lvalue.  Useful when an access chain is resolved, but needs
    // to be replaced by a reference to a temporary variable holding the result.
    data->baseId                       = baseId;
    data->accessChain.baseTypeId       = typeId;
    data->accessChain.preSwizzleTypeId = typeId;
    data->accessChain.storageClass     = storageClass;
    data->accessChain.typeSpec         = typeSpec;
}

void OutputSPIRVTraverser::nodeDataInitRValue(NodeData *data,
                                              spirv::IdRef baseId,
                                              spirv::IdRef typeId) const
{
    *data = {};

    // Initialize the access chain as an rvalue.  Useful when an access chain is resolved, and needs
    // to be replaced by a reference to it.
    data->baseId                       = baseId;
    data->accessChain.baseTypeId       = typeId;
    data->accessChain.preSwizzleTypeId = typeId;
}

void OutputSPIRVTraverser::accessChainOnPush(NodeData *data, const TType &parentType, size_t index)
{
    AccessChain &accessChain = data->accessChain;

    // Adjust |typeSpec| based on the type (which implies what the index does; select an array
    // element, a block field etc).  Index is only meaningful for selecting block fields.
    if (parentType.isArray())
    {
        accessChain.typeSpec.onArrayElementSelection(
            (parentType.getStruct() != nullptr || parentType.isInterfaceBlock()),
            parentType.isArrayOfArrays());
    }
    else if (parentType.isInterfaceBlock() || parentType.getStruct() != nullptr)
    {
        const TFieldListCollection *block = parentType.getInterfaceBlock();
        if (!parentType.isInterfaceBlock())
        {
            block = parentType.getStruct();
        }

        const TType &fieldType = *block->fields()[index]->type();
        accessChain.typeSpec.onBlockFieldSelection(fieldType);
    }
    else if (parentType.isMatrix())
    {
        accessChain.typeSpec.onMatrixColumnSelection();
    }
    else
    {
        ASSERT(parentType.isVector());
        accessChain.typeSpec.onVectorComponentSelection();
    }
}

void OutputSPIRVTraverser::accessChainPush(NodeData *data,
                                           spirv::IdRef index,
                                           spirv::IdRef typeId) const
{
    // Simply add the index to the chain of indices.
    data->idList.emplace_back(index);
    data->accessChain.areAllIndicesLiteral = false;
    data->accessChain.preSwizzleTypeId     = typeId;
}

void OutputSPIRVTraverser::accessChainPushLiteral(NodeData *data,
                                                  spirv::LiteralInteger index,
                                                  spirv::IdRef typeId) const
{
    // Add the literal integer in the chain of indices.  Since this is an id list, fake it as an id.
    data->idList.emplace_back(index);
    data->accessChain.preSwizzleTypeId = typeId;

    // Literal index of a swizzle must be already folded in the AST.
    ASSERT(data->accessChain.swizzles.empty());
}

void OutputSPIRVTraverser::accessChainPushSwizzle(NodeData *data,
                                                  const TVector<int> &swizzle,
                                                  spirv::IdRef typeId,
                                                  uint8_t componentCount) const
{
    AccessChain &accessChain = data->accessChain;

    // Record the swizzle as multi-component swizzles require special handling.  When loading
    // through the access chain, the swizzle is applied after loading the vector first (see
    // |accessChainLoad()|).  When storing through the access chain, the whole vector is loaded,
    // swizzled components overwritten and the whole vector written back (see |accessChainStore()|).
    ASSERT(accessChain.swizzles.empty());

    if (swizzle.size() == 1)
    {
        // If this swizzle is selecting a single component, fold it into the access chain.
        accessChainPushLiteral(data, spirv::LiteralInteger(swizzle[0]), typeId);
    }
    else
    {
        // Otherwise keep them separate.
        accessChain.swizzles.insert(accessChain.swizzles.end(), swizzle.begin(), swizzle.end());
        accessChain.postSwizzleTypeId            = typeId;
        accessChain.swizzledVectorComponentCount = componentCount;
    }
}

void OutputSPIRVTraverser::accessChainPushDynamicComponent(NodeData *data,
                                                           spirv::IdRef index,
                                                           spirv::IdRef typeId)
{
    AccessChain &accessChain = data->accessChain;

    // Record the index used to dynamically select a component of a vector.
    ASSERT(!accessChain.dynamicComponent.valid());

    if (IsAccessChainRValue(accessChain) && accessChain.areAllIndicesLiteral)
    {
        // If the access chain is an rvalue with all-literal indices, keep this index separate so
        // that OpCompositeExtract can be used for the access chain up to this index.
        accessChain.dynamicComponent           = index;
        accessChain.postDynamicComponentTypeId = typeId;
        return;
    }

    if (!accessChain.swizzles.empty())
    {
        // Otherwise if there's a swizzle, fold the swizzle and dynamic component selection into a
        // single dynamic component selection.
        ASSERT(accessChain.swizzles.size() > 1);

        // Create a vector constant from the swizzles.
        spirv::IdRefList swizzleIds;
        for (uint32_t component : accessChain.swizzles)
        {
            swizzleIds.push_back(mBuilder.getUintConstant(component));
        }

        const spirv::IdRef uintTypeId = mBuilder.getBasicTypeId(EbtUInt, 1);
        const spirv::IdRef uvecTypeId = mBuilder.getBasicTypeId(EbtUInt, swizzleIds.size());

        const spirv::IdRef swizzlesId = mBuilder.getNewId({});
        spirv::WriteConstantComposite(mBuilder.getSpirvTypeAndConstantDecls(), uvecTypeId,
                                      swizzlesId, swizzleIds);

        // Index that vector constant with the dynamic index.  For example, vec.ywxz[i] becomes the
        // constant {1, 3, 0, 2} indexed with i, and that index used on vec.
        const spirv::IdRef newIndex = mBuilder.getNewId({});
        spirv::WriteVectorExtractDynamic(mBuilder.getSpirvCurrentFunctionBlock(), uintTypeId,
                                         newIndex, swizzlesId, index);

        index = newIndex;
        accessChain.swizzles.clear();
    }

    // Fold it into the access chain.
    accessChainPush(data, index, typeId);
}

spirv::IdRef OutputSPIRVTraverser::accessChainCollapse(NodeData *data)
{
    AccessChain &accessChain = data->accessChain;

    ASSERT(accessChain.storageClass != spv::StorageClassMax);

    if (accessChain.accessChainId.valid())
    {
        return accessChain.accessChainId;
    }

    // If there are no indices, the baseId is where access is done to/from.
    if (data->idList.empty())
    {
        accessChain.accessChainId = data->baseId;
        return accessChain.accessChainId;
    }

    // Otherwise create an OpAccessChain instruction.  Swizzle handling is special as it selects
    // multiple components, and is done differently for load and store.
    spirv::IdRefList indexIds;
    makeAccessChainIdList(data, &indexIds);

    const spirv::IdRef typePointerId =
        mBuilder.getTypePointerId(accessChain.preSwizzleTypeId, accessChain.storageClass);

    accessChain.accessChainId = mBuilder.getNewId({});
    spirv::WriteAccessChain(mBuilder.getSpirvCurrentFunctionBlock(), typePointerId,
                            accessChain.accessChainId, data->baseId, indexIds);

    return accessChain.accessChainId;
}

spirv::IdRef OutputSPIRVTraverser::accessChainLoad(NodeData *data,
                                                   const TType &valueType,
                                                   spirv::IdRef *resultTypeIdOut)
{
    const SpirvDecorations &decorations = mBuilder.getDecorations(valueType);

    // Loading through the access chain can generate different instructions based on whether it's an
    // rvalue, the indices are literal, there's a swizzle etc.
    //
    // - If rvalue:
    //  * With indices:
    //   + All literal: OpCompositeExtract which uses literal integers to access the rvalue.
    //   + Otherwise: Can't use OpAccessChain on an rvalue, so create a temporary variable, OpStore
    //     the rvalue into it, then use OpAccessChain and OpLoad to load from it.
    //  * Without indices: Take the base id.
    // - If lvalue:
    //  * With indices: Use OpAccessChain and OpLoad
    //  * Without indices: Use OpLoad
    // - With swizzle: Use OpVectorShuffle on the result of the previous step
    // - With dynamic component: Use OpVectorExtractDynamic on the result of the previous step

    AccessChain &accessChain = data->accessChain;

    if (resultTypeIdOut)
    {
        *resultTypeIdOut = getAccessChainTypeId(data);
    }

    spirv::IdRef loadResult = data->baseId;

    if (IsAccessChainRValue(accessChain))
    {
        if (data->idList.size() > 0)
        {
            if (accessChain.areAllIndicesLiteral)
            {
                // Use OpCompositeExtract on an rvalue with all literal indices.
                spirv::LiteralIntegerList indexList;
                makeAccessChainLiteralList(data, &indexList);

                const spirv::IdRef result = mBuilder.getNewId(decorations);
                spirv::WriteCompositeExtract(mBuilder.getSpirvCurrentFunctionBlock(),
                                             accessChain.preSwizzleTypeId, result, loadResult,
                                             indexList);
                loadResult = result;
            }
            else
            {
                // Create a temp variable to hold the rvalue so an access chain can be made on it.
                const spirv::IdRef tempVar =
                    mBuilder.declareVariable(accessChain.baseTypeId, spv::StorageClassFunction,
                                             decorations, nullptr, "indexable", nullptr);

                // Write the rvalue into the temp variable
                spirv::WriteStore(mBuilder.getSpirvCurrentFunctionBlock(), tempVar, loadResult,
                                  nullptr);

                // Make the temp variable the source of the access chain.
                data->baseId                   = tempVar;
                data->accessChain.storageClass = spv::StorageClassFunction;

                // Load from the temp variable.
                const spirv::IdRef accessChainId = accessChainCollapse(data);
                loadResult                       = mBuilder.getNewId(decorations);
                spirv::WriteLoad(mBuilder.getSpirvCurrentFunctionBlock(),
                                 accessChain.preSwizzleTypeId, loadResult, accessChainId, nullptr);
            }
        }
    }
    else
    {
        // Load from the access chain.
        const spirv::IdRef accessChainId = accessChainCollapse(data);
        loadResult                       = mBuilder.getNewId(decorations);
        spirv::WriteLoad(mBuilder.getSpirvCurrentFunctionBlock(), accessChain.preSwizzleTypeId,
                         loadResult, accessChainId, nullptr);
    }

    if (!accessChain.swizzles.empty())
    {
        // Single-component swizzles are already folded into the index list.
        ASSERT(accessChain.swizzles.size() > 1);

        // Take the loaded value and use OpVectorShuffle to create the swizzle.
        spirv::LiteralIntegerList swizzleList;
        for (uint32_t component : accessChain.swizzles)
        {
            swizzleList.push_back(spirv::LiteralInteger(component));
        }

        const spirv::IdRef result = mBuilder.getNewId(decorations);
        spirv::WriteVectorShuffle(mBuilder.getSpirvCurrentFunctionBlock(),
                                  accessChain.postSwizzleTypeId, result, loadResult, loadResult,
                                  swizzleList);
        loadResult = result;
    }

    if (accessChain.dynamicComponent.valid())
    {
        // Use OpVectorExtractDynamic to select the component.
        const spirv::IdRef result = mBuilder.getNewId(decorations);
        spirv::WriteVectorExtractDynamic(mBuilder.getSpirvCurrentFunctionBlock(),
                                         accessChain.postDynamicComponentTypeId, result, loadResult,
                                         accessChain.dynamicComponent);
        loadResult = result;
    }

    // Upon loading values, cast them to the default SPIR-V variant.
    const spirv::IdRef castResult =
        cast(loadResult, valueType, accessChain.typeSpec, {}, resultTypeIdOut);

    return castResult;
}

void OutputSPIRVTraverser::accessChainStore(NodeData *data,
                                            spirv::IdRef value,
                                            const TType &valueType)
{
    // Storing through the access chain can generate different instructions based on whether the
    // there's a swizzle.
    //
    // - Without swizzle: Use OpAccessChain and OpStore
    // - With swizzle: Use OpAccessChain and OpLoad to load the vector, then use OpVectorShuffle to
    //   replace the components being overwritten.  Finally, use OpStore to write the result back.

    AccessChain &accessChain = data->accessChain;

    // Single-component swizzles are already folded into the indices.
    ASSERT(accessChain.swizzles.size() != 1);
    // Since store can only happen through lvalues, it's impossible to have a dynamic component as
    // that always gets folded into the indices except for rvalues.
    ASSERT(!accessChain.dynamicComponent.valid());

    const spirv::IdRef accessChainId = accessChainCollapse(data);

    // Store through the access chain.  The values are always cast to the default SPIR-V type
    // variant when loaded from memory and operated on as such.  When storing, we need to cast the
    // result to the variant specified by the access chain.
    value = cast(value, valueType, {}, accessChain.typeSpec, nullptr);

    if (!accessChain.swizzles.empty())
    {
        // Load the vector before the swizzle.
        const spirv::IdRef loadResult = mBuilder.getNewId({});
        spirv::WriteLoad(mBuilder.getSpirvCurrentFunctionBlock(), accessChain.preSwizzleTypeId,
                         loadResult, accessChainId, nullptr);

        // Overwrite the components being written.  This is done by first creating an identity
        // swizzle, then replacing the components being written with a swizzle from the value.  For
        // example, take the following:
        //
        //     vec4 v;
        //     v.zx = u;
        //
        // The OpVectorShuffle instruction takes two vectors (v and u) and selects components from
        // each (in this example, swizzles [0, 3] select from v and [4, 7] select from u).  This
        // algorithm first creates the identity swizzles {0, 1, 2, 3}, then replaces z and x (the
        // 0th and 2nd element) with swizzles from u (4 + {0, 1}) to get the result
        // {4+1, 1, 4+0, 3}.

        spirv::LiteralIntegerList swizzleList;
        for (uint32_t component = 0; component < accessChain.swizzledVectorComponentCount;
             ++component)
        {
            swizzleList.push_back(spirv::LiteralInteger(component));
        }
        uint32_t srcComponent = 0;
        for (uint32_t dstComponent : accessChain.swizzles)
        {
            swizzleList[dstComponent] =
                spirv::LiteralInteger(accessChain.swizzledVectorComponentCount + srcComponent);
            ++srcComponent;
        }

        // Use the generated swizzle to select components from the loaded vector and the value to be
        // written.  Use the final result as the value to be written to the vector.
        const spirv::IdRef result = mBuilder.getNewId({});
        spirv::WriteVectorShuffle(mBuilder.getSpirvCurrentFunctionBlock(),
                                  accessChain.preSwizzleTypeId, result, loadResult, value,
                                  swizzleList);
        value = result;
    }

    spirv::WriteStore(mBuilder.getSpirvCurrentFunctionBlock(), accessChainId, value, nullptr);
}

void OutputSPIRVTraverser::makeAccessChainIdList(NodeData *data, spirv::IdRefList *idsOut)
{
    for (size_t index = 0; index < data->idList.size(); ++index)
    {
        spirv::IdRef indexId = data->idList[index].id;

        if (!indexId.valid())
        {
            // The index is a literal integer, so replace it with an OpConstant id.
            indexId = mBuilder.getUintConstant(data->idList[index].literal);
        }

        idsOut->push_back(indexId);
    }
}

void OutputSPIRVTraverser::makeAccessChainLiteralList(NodeData *data,
                                                      spirv::LiteralIntegerList *literalsOut)
{
    for (size_t index = 0; index < data->idList.size(); ++index)
    {
        ASSERT(!data->idList[index].id.valid());
        literalsOut->push_back(data->idList[index].literal);
    }
}

spirv::IdRef OutputSPIRVTraverser::getAccessChainTypeId(NodeData *data)
{
    // Load and store through the access chain may be done in multiple steps.  These steps produce
    // the following types:
    //
    // - preSwizzleTypeId
    // - postSwizzleTypeId
    // - postDynamicComponentTypeId
    //
    // The last of these types is the final type of the expression this access chain corresponds to.
    const AccessChain &accessChain = data->accessChain;

    if (accessChain.postDynamicComponentTypeId.valid())
    {
        return accessChain.postDynamicComponentTypeId;
    }
    if (accessChain.postSwizzleTypeId.valid())
    {
        return accessChain.postSwizzleTypeId;
    }
    ASSERT(accessChain.preSwizzleTypeId.valid());
    return accessChain.preSwizzleTypeId;
}

void OutputSPIRVTraverser::declareConst(TIntermDeclaration *decl)
{
    const TIntermSequence &sequence = *decl->getSequence();
    ASSERT(sequence.size() == 1);

    TIntermBinary *assign = sequence.front()->getAsBinaryNode();
    ASSERT(assign != nullptr && assign->getOp() == EOpInitialize);

    TIntermSymbol *symbol = assign->getLeft()->getAsSymbolNode();
    ASSERT(symbol != nullptr && symbol->getType().getQualifier() == EvqConst);

    TIntermTyped *initializer = assign->getRight();
    ASSERT(initializer->getAsConstantUnion() != nullptr || initializer->hasConstantValue());

    const TType &type         = symbol->getType();
    const TVariable *variable = &symbol->variable();

    const spirv::IdRef typeId = mBuilder.getTypeData(type, {}).id;
    const spirv::IdRef constId =
        createConstant(type, type.getBasicType(), initializer->getConstantValue(),
                       initializer->isConstantNullValue());

    // Remember the id of the variable for future look up.
    ASSERT(mSymbolIdMap.count(variable) == 0);
    mSymbolIdMap[variable] = constId;

    if (!mInGlobalScope)
    {
        mNodeData.emplace_back();
        nodeDataInitRValue(&mNodeData.back(), constId, typeId);
    }
}

void OutputSPIRVTraverser::declareSpecConst(TIntermDeclaration *decl)
{
    const TIntermSequence &sequence = *decl->getSequence();
    ASSERT(sequence.size() == 1);

    TIntermBinary *assign = sequence.front()->getAsBinaryNode();
    ASSERT(assign != nullptr && assign->getOp() == EOpInitialize);

    TIntermSymbol *symbol = assign->getLeft()->getAsSymbolNode();
    ASSERT(symbol != nullptr && symbol->getType().getQualifier() == EvqSpecConst);

    TIntermConstantUnion *initializer = assign->getRight()->getAsConstantUnion();
    ASSERT(initializer != nullptr);

    const TType &type         = symbol->getType();
    const TVariable *variable = &symbol->variable();

    // All spec consts in ANGLE are initialized to 0.
    ASSERT(initializer->isZero(0));

    const spirv::IdRef specConstId = mBuilder.declareSpecConst(
        type.getBasicType(), type.getLayoutQualifier().location, mBuilder.getName(variable).data());

    // Remember the id of the variable for future look up.
    ASSERT(mSymbolIdMap.count(variable) == 0);
    mSymbolIdMap[variable] = specConstId;
}

spirv::IdRef OutputSPIRVTraverser::createConstant(const TType &type,
                                                  TBasicType expectedBasicType,
                                                  const TConstantUnion *constUnion,
                                                  bool isConstantNullValue)
{
    const spirv::IdRef typeId = mBuilder.getTypeData(type, {}).id;
    spirv::IdRefList componentIds;

    // If the object is all zeros, use OpConstantNull to avoid creating a bunch of constants.  This
    // is not done for basic scalar types as some instructions require an OpConstant and validation
    // doesn't accept OpConstantNull (likely a spec bug).
    const size_t size            = type.getObjectSize();
    const TBasicType basicType   = type.getBasicType();
    const bool isBasicScalar     = size == 1 && (basicType == EbtFloat || basicType == EbtInt ||
                                             basicType == EbtUInt || basicType == EbtBool);
    const bool useOpConstantNull = isConstantNullValue && !isBasicScalar;
    if (useOpConstantNull)
    {
        return mBuilder.getNullConstant(typeId);
    }

    if (type.isArray())
    {
        TType elementType(type);
        elementType.toArrayElementType();

        // If it's an array constant, get the constant id of each element.
        for (unsigned int elementIndex = 0; elementIndex < type.getOutermostArraySize();
             ++elementIndex)
        {
            componentIds.push_back(
                createConstant(elementType, expectedBasicType, constUnion, false));
            constUnion += elementType.getObjectSize();
        }
    }
    else if (type.getBasicType() == EbtStruct)
    {
        // If it's a struct constant, get the constant id for each field.
        for (const TField *field : type.getStruct()->fields())
        {
            const TType *fieldType = field->type();
            componentIds.push_back(
                createConstant(*fieldType, fieldType->getBasicType(), constUnion, false));

            constUnion += fieldType->getObjectSize();
        }
    }
    else
    {
        // Otherwise get the constant id for each component.
        ASSERT(expectedBasicType == EbtFloat || expectedBasicType == EbtInt ||
               expectedBasicType == EbtUInt || expectedBasicType == EbtBool ||
               expectedBasicType == EbtYuvCscStandardEXT);

        for (size_t component = 0; component < size; ++component, ++constUnion)
        {
            spirv::IdRef componentId;

            // If the constant has a different type than expected, cast it right away.
            TConstantUnion castConstant;
            bool valid = castConstant.cast(expectedBasicType, *constUnion);
            ASSERT(valid);

            switch (castConstant.getType())
            {
                case EbtFloat:
                    componentId = mBuilder.getFloatConstant(castConstant.getFConst());
                    break;
                case EbtInt:
                    componentId = mBuilder.getIntConstant(castConstant.getIConst());
                    break;
                case EbtUInt:
                    componentId = mBuilder.getUintConstant(castConstant.getUConst());
                    break;
                case EbtBool:
                    componentId = mBuilder.getBoolConstant(castConstant.getBConst());
                    break;
                case EbtYuvCscStandardEXT:
                    componentId =
                        mBuilder.getUintConstant(castConstant.getYuvCscStandardEXTConst());
                    break;
                default:
                    UNREACHABLE();
            }
            componentIds.push_back(componentId);
        }
    }

    // If this is a composite, create a composite constant from the components.
    if (type.isArray() || type.getBasicType() == EbtStruct || componentIds.size() > 1)
    {
        return createComplexConstant(type, typeId, componentIds);
    }

    // Otherwise return the sole component.
    ASSERT(componentIds.size() == 1);
    return componentIds[0];
}

spirv::IdRef OutputSPIRVTraverser::createComplexConstant(const TType &type,
                                                         spirv::IdRef typeId,
                                                         const spirv::IdRefList &parameters)
{
    ASSERT(!type.isScalar());

    if (type.isMatrix() && !type.isArray())
    {
        ASSERT(parameters.size() == type.getRows() * type.getCols());

        // Matrices are constructed from their columns.
        spirv::IdRefList columnIds;

        const spirv::IdRef columnTypeId =
            mBuilder.getBasicTypeId(type.getBasicType(), type.getRows());

        for (uint8_t columnIndex = 0; columnIndex < type.getCols(); ++columnIndex)
        {
            auto columnParametersStart = parameters.begin() + columnIndex * type.getRows();
            spirv::IdRefList columnParameters(columnParametersStart,
                                              columnParametersStart + type.getRows());

            columnIds.push_back(mBuilder.getCompositeConstant(columnTypeId, columnParameters));
        }

        return mBuilder.getCompositeConstant(typeId, columnIds);
    }

    return mBuilder.getCompositeConstant(typeId, parameters);
}

spirv::IdRef OutputSPIRVTraverser::createConstructor(TIntermAggregate *node, spirv::IdRef typeId)
{
    const TType &type                = node->getType();
    const TIntermSequence &arguments = *node->getSequence();
    const TType &arg0Type            = arguments[0]->getAsTyped()->getType();

    // In some cases, constructors-with-constant values are not folded.  If the constructor is a
    // null value, use OpConstantNull to avoid creating a bunch of instructions.  Otherwise, the
    // constant is created below.
    if (node->isConstantNullValue())
    {
        return mBuilder.getNullConstant(typeId);
    }

    // Take each constructor argument that is visited and evaluate it as rvalue
    spirv::IdRefList parameters = loadAllParams(node, 0, nullptr);

    // Constructors in GLSL can take various shapes, resulting in different translations to SPIR-V
    // (in each case, if the parameter doesn't match the type being constructed, it must be cast):
    //
    // - float(f): This should translate to just f
    // - float(v): This should translate to OpCompositeExtract %scalar %v 0
    // - float(m): This should translate to OpCompositeExtract %scalar %m 0 0
    // - vecN(f): This should translate to OpCompositeConstruct %vecN %f %f .. %f
    // - vecN(v1.zy, v2.x): This can technically translate to OpCompositeConstruct with two ids; the
    //   results of v1.zy and v2.x.  However, for simplicity it's easier to generate that
    //   instruction with three ids; the results of v1.z, v1.y and v2.x (see below where a matrix is
    //   used as parameter).
    // - vecN(m): This takes N components from m in column-major order (for example, vec4
    //   constructed out of a 4x3 matrix would select components (0,0), (0,1), (0,2) and (1,0)).
    //   This translates to OpCompositeConstruct with the id of the individual components extracted
    //   from m.
    // - matNxM(f): This creates a diagonal matrix.  It generates N OpCompositeConstruct
    //   instructions for each column (which are vecM), followed by an OpCompositeConstruct that
    //   constructs the final result.
    // - matNxM(m):
    //   * With m larger than NxM, this extracts a submatrix out of m.  It generates
    //     OpCompositeExtracts for N columns of m, followed by an OpVectorShuffle (swizzle) if the
    //     rows of m are more than M.  OpCompositeConstruct is used to construct the final result.
    //   * If m is not larger than NxM, an identity matrix is created and superimposed with m.
    //     OpCompositeExtract is used to extract each component of m (that is necessary), and
    //     together with the zero or one constants necessary used to create the columns (with
    //     OpCompositeConstruct).  OpCompositeConstruct is used to construct the final result.
    // - matNxM(v1.zy, v2.x, ...): Similarly to constructing a vector, a list of single components
    //   are extracted from the parameters, which are divided up and used to construct each column,
    //   which is finally constructed into the final result.
    //
    // Additionally, array and structs are constructed by OpCompositeConstruct followed by ids of
    // each parameter which must enumerate every individual element / field.

    // In some cases, constructors-with-constant values are not folded such as for large constants.
    // Some transformations may also produce constructors-with-constants instead of constants even
    // for basic types.  These are handled here.
    if (node->hasConstantValue())
    {
        if (!type.isScalar())
        {
            return createComplexConstant(node->getType(), typeId, parameters);
        }

        // If a transformation creates scalar(constant), return the constant as-is.
        // visitConstantUnion has already cast it to the right type.
        if (arguments[0]->getAsConstantUnion() != nullptr)
        {
            return parameters[0];
        }
    }

    if (type.isArray() || type.getStruct() != nullptr)
    {
        return createArrayOrStructConstructor(node, typeId, parameters);
    }

    // The following are simple casts:
    //
    // - basic(s) (where basic is int, uint, float or bool, and s is scalar).
    // - gvecN(vN) (where the argument is a single vector with the same number of components).
    // - matNxM(mNxM) (where the argument is a single matrix with the same dimensions).  Note that
    //   matrices are always float, so there's no actual cast and this would be a no-op.
    //
    const bool isSingleScalarCast = arguments.size() == 1 && type.isScalar() && arg0Type.isScalar();
    const bool isSingleVectorCast = arguments.size() == 1 && type.isVector() &&
                                    arg0Type.isVector() &&
                                    type.getNominalSize() == arg0Type.getNominalSize();
    const bool isSingleMatrixCast = arguments.size() == 1 && type.isMatrix() &&
                                    arg0Type.isMatrix() && type.getCols() == arg0Type.getCols() &&
                                    type.getRows() == arg0Type.getRows();
    if (isSingleScalarCast || isSingleVectorCast || isSingleMatrixCast)
    {
        return castBasicType(parameters[0], arg0Type, type, nullptr);
    }

    if (type.isScalar())
    {
        ASSERT(arguments.size() == 1);
        return createConstructorScalarFromNonScalar(node, typeId, parameters);
    }

    if (type.isVector())
    {
        if (arguments.size() == 1 && arg0Type.isScalar())
        {
            return createConstructorVectorFromScalar(arg0Type, type, typeId, parameters);
        }
        if (arg0Type.isMatrix())
        {
            // If the first argument is a matrix, it will always have enough components to fill an
            // entire vector, so it doesn't matter what's specified after it.
            return createConstructorVectorFromMatrix(node, typeId, parameters);
        }
        return createConstructorVectorFromMultiple(node, typeId, parameters);
    }

    ASSERT(type.isMatrix());

    if (arg0Type.isScalar() && arguments.size() == 1)
    {
        parameters[0] = castBasicType(parameters[0], arg0Type, type, nullptr);
        return createConstructorMatrixFromScalar(node, typeId, parameters);
    }
    if (arg0Type.isMatrix())
    {
        return createConstructorMatrixFromMatrix(node, typeId, parameters);
    }
    return createConstructorMatrixFromVectors(node, typeId, parameters);
}

spirv::IdRef OutputSPIRVTraverser::createArrayOrStructConstructor(
    TIntermAggregate *node,
    spirv::IdRef typeId,
    const spirv::IdRefList &parameters)
{
    const spirv::IdRef result = mBuilder.getNewId(mBuilder.getDecorations(node->getType()));
    spirv::WriteCompositeConstruct(mBuilder.getSpirvCurrentFunctionBlock(), typeId, result,
                                   parameters);
    return result;
}

spirv::IdRef OutputSPIRVTraverser::createConstructorScalarFromNonScalar(
    TIntermAggregate *node,
    spirv::IdRef typeId,
    const spirv::IdRefList &parameters)
{
    ASSERT(parameters.size() == 1);
    const TType &type     = node->getType();
    const TType &arg0Type = node->getChildNode(0)->getAsTyped()->getType();

    const spirv::IdRef result = mBuilder.getNewId(mBuilder.getDecorations(type));

    spirv::LiteralIntegerList indices = {spirv::LiteralInteger(0)};
    if (arg0Type.isMatrix())
    {
        indices.push_back(spirv::LiteralInteger(0));
    }

    spirv::WriteCompositeExtract(mBuilder.getSpirvCurrentFunctionBlock(),
                                 mBuilder.getBasicTypeId(arg0Type.getBasicType(), 1), result,
                                 parameters[0], indices);

    TType arg0TypeAsScalar(arg0Type);
    arg0TypeAsScalar.toComponentType();

    return castBasicType(result, arg0TypeAsScalar, type, nullptr);
}

spirv::IdRef OutputSPIRVTraverser::createConstructorVectorFromScalar(
    const TType &parameterType,
    const TType &expectedType,
    spirv::IdRef typeId,
    const spirv::IdRefList &parameters)
{
    // vecN(f) translates to OpCompositeConstruct %vecN %f ... %f
    ASSERT(parameters.size() == 1);

    const spirv::IdRef castParameter =
        castBasicType(parameters[0], parameterType, expectedType, nullptr);

    spirv::IdRefList replicatedParameter(expectedType.getNominalSize(), castParameter);

    const spirv::IdRef result = mBuilder.getNewId(mBuilder.getDecorations(parameterType));
    spirv::WriteCompositeConstruct(mBuilder.getSpirvCurrentFunctionBlock(), typeId, result,
                                   replicatedParameter);
    return result;
}

spirv::IdRef OutputSPIRVTraverser::createConstructorVectorFromMatrix(
    TIntermAggregate *node,
    spirv::IdRef typeId,
    const spirv::IdRefList &parameters)
{
    // vecN(m) translates to OpCompositeConstruct %vecN %m[0][0] %m[0][1] ...
    spirv::IdRefList extractedComponents;
    extractComponents(node, node->getType().getNominalSize(), parameters, &extractedComponents);

    // Construct the vector with the basic type of the argument, and cast it at end if needed.
    ASSERT(parameters.size() == 1);
    const TType &arg0Type     = node->getChildNode(0)->getAsTyped()->getType();
    const TType &expectedType = node->getType();

    spirv::IdRef argumentTypeId = typeId;
    TType arg0TypeAsVector(arg0Type);
    arg0TypeAsVector.setPrimarySize(node->getType().getNominalSize());
    arg0TypeAsVector.setSecondarySize(1);

    if (arg0Type.getBasicType() != expectedType.getBasicType())
    {
        argumentTypeId = mBuilder.getTypeData(arg0TypeAsVector, {}).id;
    }

    spirv::IdRef result = mBuilder.getNewId(mBuilder.getDecorations(node->getType()));
    spirv::WriteCompositeConstruct(mBuilder.getSpirvCurrentFunctionBlock(), argumentTypeId, result,
                                   extractedComponents);

    if (arg0Type.getBasicType() != expectedType.getBasicType())
    {
        result = castBasicType(result, arg0TypeAsVector, expectedType, nullptr);
    }

    return result;
}

spirv::IdRef OutputSPIRVTraverser::createConstructorVectorFromMultiple(
    TIntermAggregate *node,
    spirv::IdRef typeId,
    const spirv::IdRefList &parameters)
{
    const TType &type = node->getType();
    // vecN(v1.zy, v2.x) translates to OpCompositeConstruct %vecN %v1.z %v1.y %v2.x
    spirv::IdRefList extractedComponents;
    extractComponents(node, type.getNominalSize(), parameters, &extractedComponents);

    // Handle the case where a matrix is used in the constructor anywhere but the first place.  In
    // that case, the components extracted from the matrix might need casting to the right type.
    const TIntermSequence &arguments = *node->getSequence();
    for (size_t argumentIndex = 0, componentIndex = 0;
         argumentIndex < arguments.size() && componentIndex < extractedComponents.size();
         ++argumentIndex)
    {
        TIntermNode *argument     = arguments[argumentIndex];
        const TType &argumentType = argument->getAsTyped()->getType();
        if (argumentType.isScalar() || argumentType.isVector())
        {
            // extractComponents already casts scalar and vector components.
            componentIndex += argumentType.getNominalSize();
            continue;
        }

        TType componentType(argumentType);
        componentType.toComponentType();

        for (uint8_t columnIndex = 0;
             columnIndex < argumentType.getCols() && componentIndex < extractedComponents.size();
             ++columnIndex)
        {
            for (uint8_t rowIndex = 0;
                 rowIndex < argumentType.getRows() && componentIndex < extractedComponents.size();
                 ++rowIndex, ++componentIndex)
            {
                extractedComponents[componentIndex] = castBasicType(
                    extractedComponents[componentIndex], componentType, type, nullptr);
            }
        }

        // Matrices all have enough components to fill a vector, so it's impossible to need to visit
        // any other arguments that may come after.
        ASSERT(componentIndex == extractedComponents.size());
    }

    const spirv::IdRef result = mBuilder.getNewId(mBuilder.getDecorations(node->getType()));
    spirv::WriteCompositeConstruct(mBuilder.getSpirvCurrentFunctionBlock(), typeId, result,
                                   extractedComponents);
    return result;
}

spirv::IdRef OutputSPIRVTraverser::createConstructorMatrixFromScalar(
    TIntermAggregate *node,
    spirv::IdRef typeId,
    const spirv::IdRefList &parameters)
{
    // matNxM(f) translates to
    //
    //     %c0 = OpCompositeConstruct %vecM %f %zero %zero ..
    //     %c1 = OpCompositeConstruct %vecM %zero %f %zero ..
    //     %c2 = OpCompositeConstruct %vecM %zero %zero %f ..
    //     ...
    //     %m  = OpCompositeConstruct %matNxM %c0 %c1 %c2 ...

    const TType &type           = node->getType();
    const spirv::IdRef scalarId = parameters[0];
    spirv::IdRef zeroId;

    SpirvDecorations decorations = mBuilder.getDecorations(type);

    switch (type.getBasicType())
    {
        case EbtFloat:
            zeroId = mBuilder.getFloatConstant(0);
            break;
        case EbtInt:
            zeroId = mBuilder.getIntConstant(0);
            break;
        case EbtUInt:
            zeroId = mBuilder.getUintConstant(0);
            break;
        case EbtBool:
            zeroId = mBuilder.getBoolConstant(0);
            break;
        default:
            UNREACHABLE();
    }

    spirv::IdRefList componentIds(type.getRows(), zeroId);
    spirv::IdRefList columnIds;

    const spirv::IdRef columnTypeId = mBuilder.getBasicTypeId(type.getBasicType(), type.getRows());

    for (uint8_t columnIndex = 0; columnIndex < type.getCols(); ++columnIndex)
    {
        columnIds.push_back(mBuilder.getNewId(decorations));

        // Place the scalar at the correct index (diagonal of the matrix, i.e. row == col).
        if (columnIndex < type.getRows())
        {
            componentIds[columnIndex] = scalarId;
        }
        if (columnIndex > 0 && columnIndex <= type.getRows())
        {
            componentIds[columnIndex - 1] = zeroId;
        }

        // Create the column.
        spirv::WriteCompositeConstruct(mBuilder.getSpirvCurrentFunctionBlock(), columnTypeId,
                                       columnIds.back(), componentIds);
    }

    // Create the matrix out of the columns.
    const spirv::IdRef result = mBuilder.getNewId(decorations);
    spirv::WriteCompositeConstruct(mBuilder.getSpirvCurrentFunctionBlock(), typeId, result,
                                   columnIds);
    return result;
}

spirv::IdRef OutputSPIRVTraverser::createConstructorMatrixFromVectors(
    TIntermAggregate *node,
    spirv::IdRef typeId,
    const spirv::IdRefList &parameters)
{
    // matNxM(v1.zy, v2.x, ...) translates to:
    //
    //     %c0 = OpCompositeConstruct %vecM %v1.z %v1.y %v2.x ..
    //     ...
    //     %m  = OpCompositeConstruct %matNxM %c0 %c1 %c2 ...

    const TType &type = node->getType();

    SpirvDecorations decorations = mBuilder.getDecorations(type);

    spirv::IdRefList extractedComponents;
    extractComponents(node, type.getCols() * type.getRows(), parameters, &extractedComponents);

    spirv::IdRefList columnIds;

    const spirv::IdRef columnTypeId = mBuilder.getBasicTypeId(type.getBasicType(), type.getRows());

    // Chunk up the extracted components by column and construct intermediary vectors.
    for (uint8_t columnIndex = 0; columnIndex < type.getCols(); ++columnIndex)
    {
        columnIds.push_back(mBuilder.getNewId(decorations));

        auto componentsStart = extractedComponents.begin() + columnIndex * type.getRows();
        const spirv::IdRefList componentIds(componentsStart, componentsStart + type.getRows());

        // Create the column.
        spirv::WriteCompositeConstruct(mBuilder.getSpirvCurrentFunctionBlock(), columnTypeId,
                                       columnIds.back(), componentIds);
    }

    const spirv::IdRef result = mBuilder.getNewId(decorations);
    spirv::WriteCompositeConstruct(mBuilder.getSpirvCurrentFunctionBlock(), typeId, result,
                                   columnIds);
    return result;
}

spirv::IdRef OutputSPIRVTraverser::createConstructorMatrixFromMatrix(
    TIntermAggregate *node,
    spirv::IdRef typeId,
    const spirv::IdRefList &parameters)
{
    // matNxM(m) translates to:
    //
    // - If m is SxR where S>=N and R>=M:
    //
    //     %c0 = OpCompositeExtract %vecR %m 0
    //     %c1 = OpCompositeExtract %vecR %m 1
    //     ...
    //     // If R (column size of m) != M, OpVectorShuffle to extract M components out of %ci.
    //     ...
    //     %m  = OpCompositeConstruct %matNxM %c0 %c1 %c2 ...
    //
    // - Otherwise, an identity matrix is created and superimposed by m:
    //
    //     %c0 = OpCompositeConstruct %vecM %m[0][0] %m[0][1] %0 %0
    //     %c1 = OpCompositeConstruct %vecM %m[1][0] %m[1][1] %0 %0
    //     %c2 = OpCompositeConstruct %vecM %m[2][0] %m[2][1] %1 %0
    //     %c3 = OpCompositeConstruct %vecM       %0       %0 %0 %1
    //     %m  = OpCompositeConstruct %matNxM %c0 %c1 %c2 %c3

    const TType &type          = node->getType();
    const TType &parameterType = (*node->getSequence())[0]->getAsTyped()->getType();

    SpirvDecorations decorations = mBuilder.getDecorations(type);

    ASSERT(parameters.size() == 1);

    spirv::IdRefList columnIds;

    const spirv::IdRef columnTypeId = mBuilder.getBasicTypeId(type.getBasicType(), type.getRows());

    if (parameterType.getCols() >= type.getCols() && parameterType.getRows() >= type.getRows())
    {
        // If the parameter is a larger matrix than the constructor type, extract the columns
        // directly and potentially swizzle them.
        TType paramColumnType(parameterType);
        paramColumnType.toMatrixColumnType();
        const spirv::IdRef paramColumnTypeId = mBuilder.getTypeData(paramColumnType, {}).id;

        const bool needsSwizzle           = parameterType.getRows() > type.getRows();
        spirv::LiteralIntegerList swizzle = {spirv::LiteralInteger(0), spirv::LiteralInteger(1),
                                             spirv::LiteralInteger(2), spirv::LiteralInteger(3)};
        swizzle.resize_down(type.getRows());

        for (uint8_t columnIndex = 0; columnIndex < type.getCols(); ++columnIndex)
        {
            // Extract the column.
            const spirv::IdRef parameterColumnId = mBuilder.getNewId(decorations);
            spirv::WriteCompositeExtract(mBuilder.getSpirvCurrentFunctionBlock(), paramColumnTypeId,
                                         parameterColumnId, parameters[0],
                                         {spirv::LiteralInteger(columnIndex)});

            // If the column has too many components, select the appropriate number of components.
            spirv::IdRef constructorColumnId = parameterColumnId;
            if (needsSwizzle)
            {
                constructorColumnId = mBuilder.getNewId(decorations);
                spirv::WriteVectorShuffle(mBuilder.getSpirvCurrentFunctionBlock(), columnTypeId,
                                          constructorColumnId, parameterColumnId, parameterColumnId,
                                          swizzle);
            }

            columnIds.push_back(constructorColumnId);
        }
    }
    else
    {
        // Otherwise create an identity matrix and fill in the components that can be taken from the
        // given parameter.
        TType paramComponentType(parameterType);
        paramComponentType.toComponentType();
        const spirv::IdRef paramComponentTypeId = mBuilder.getTypeData(paramComponentType, {}).id;

        for (uint8_t columnIndex = 0; columnIndex < type.getCols(); ++columnIndex)
        {
            spirv::IdRefList componentIds;

            for (uint8_t componentIndex = 0; componentIndex < type.getRows(); ++componentIndex)
            {
                // Take the component from the constructor parameter if possible.
                spirv::IdRef componentId;
                if (componentIndex < parameterType.getRows() &&
                    columnIndex < parameterType.getCols())
                {
                    componentId = mBuilder.getNewId(decorations);
                    spirv::WriteCompositeExtract(mBuilder.getSpirvCurrentFunctionBlock(),
                                                 paramComponentTypeId, componentId, parameters[0],
                                                 {spirv::LiteralInteger(columnIndex),
                                                  spirv::LiteralInteger(componentIndex)});
                }
                else
                {
                    const bool isOnDiagonal = columnIndex == componentIndex;
                    switch (type.getBasicType())
                    {
                        case EbtFloat:
                            componentId = mBuilder.getFloatConstant(isOnDiagonal ? 1.0f : 0.0f);
                            break;
                        case EbtInt:
                            componentId = mBuilder.getIntConstant(isOnDiagonal ? 1 : 0);
                            break;
                        case EbtUInt:
                            componentId = mBuilder.getUintConstant(isOnDiagonal ? 1 : 0);
                            break;
                        case EbtBool:
                            componentId = mBuilder.getBoolConstant(isOnDiagonal);
                            break;
                        default:
                            UNREACHABLE();
                    }
                }

                componentIds.push_back(componentId);
            }

            // Create the column vector.
            columnIds.push_back(mBuilder.getNewId(decorations));
            spirv::WriteCompositeConstruct(mBuilder.getSpirvCurrentFunctionBlock(), columnTypeId,
                                           columnIds.back(), componentIds);
        }
    }

    const spirv::IdRef result = mBuilder.getNewId(decorations);
    spirv::WriteCompositeConstruct(mBuilder.getSpirvCurrentFunctionBlock(), typeId, result,
                                   columnIds);
    return result;
}

spirv::IdRefList OutputSPIRVTraverser::loadAllParams(TIntermOperator *node,
                                                     size_t skipCount,
                                                     spirv::IdRefList *paramTypeIds)
{
    const size_t parameterCount = node->getChildCount();
    spirv::IdRefList parameters;

    for (size_t paramIndex = 0; paramIndex + skipCount < parameterCount; ++paramIndex)
    {
        // Take each parameter that is visited and evaluate it as rvalue
        NodeData &param = mNodeData[mNodeData.size() - parameterCount + paramIndex];

        spirv::IdRef paramTypeId;
        const spirv::IdRef paramValue = accessChainLoad(
            &param, node->getChildNode(paramIndex)->getAsTyped()->getType(), &paramTypeId);

        parameters.push_back(paramValue);
        if (paramTypeIds)
        {
            paramTypeIds->push_back(paramTypeId);
        }
    }

    return parameters;
}

void OutputSPIRVTraverser::extractComponents(TIntermAggregate *node,
                                             size_t componentCount,
                                             const spirv::IdRefList &parameters,
                                             spirv::IdRefList *extractedComponentsOut)
{
    // A helper function that takes the list of parameters passed to a constructor (which may have
    // more components than necessary) and extracts the first componentCount components.
    const TIntermSequence &arguments = *node->getSequence();

    const SpirvDecorations decorations = mBuilder.getDecorations(node->getType());
    const TType &expectedType          = node->getType();

    ASSERT(arguments.size() == parameters.size());

    for (size_t argumentIndex = 0;
         argumentIndex < arguments.size() && extractedComponentsOut->size() < componentCount;
         ++argumentIndex)
    {
        TIntermNode *argument          = arguments[argumentIndex];
        const TType &argumentType      = argument->getAsTyped()->getType();
        const spirv::IdRef parameterId = parameters[argumentIndex];

        if (argumentType.isScalar())
        {
            // For scalar parameters, there's nothing to do other than a potential cast.
            const spirv::IdRef castParameterId =
                argument->getAsConstantUnion()
                    ? parameterId
                    : castBasicType(parameterId, argumentType, expectedType, nullptr);
            extractedComponentsOut->push_back(castParameterId);
            continue;
        }
        if (argumentType.isVector())
        {
            TType componentType(argumentType);
            componentType.toComponentType();
            componentType.setBasicType(expectedType.getBasicType());
            const spirv::IdRef componentTypeId = mBuilder.getTypeData(componentType, {}).id;

            // Cast the whole vector parameter in one go.
            const spirv::IdRef castParameterId =
                argument->getAsConstantUnion()
                    ? parameterId
                    : castBasicType(parameterId, argumentType, expectedType, nullptr);

            // For vector parameters, take components out of the vector one by one.
            for (uint8_t componentIndex = 0; componentIndex < argumentType.getNominalSize() &&
                                             extractedComponentsOut->size() < componentCount;
                 ++componentIndex)
            {
                const spirv::IdRef componentId = mBuilder.getNewId(decorations);
                spirv::WriteCompositeExtract(mBuilder.getSpirvCurrentFunctionBlock(),
                                             componentTypeId, componentId, castParameterId,
                                             {spirv::LiteralInteger(componentIndex)});

                extractedComponentsOut->push_back(componentId);
            }
            continue;
        }

        ASSERT(argumentType.isMatrix());

        TType componentType(argumentType);
        componentType.toComponentType();
        const spirv::IdRef componentTypeId = mBuilder.getTypeData(componentType, {}).id;

        // For matrix parameters, take components out of the matrix one by one in column-major
        // order.  No cast is done here; it would only be required for vector constructors with
        // matrix parameters, in which case the resulting vector is cast in the end.
        for (uint8_t columnIndex = 0; columnIndex < argumentType.getCols() &&
                                      extractedComponentsOut->size() < componentCount;
             ++columnIndex)
        {
            for (uint8_t componentIndex = 0; componentIndex < argumentType.getRows() &&
                                             extractedComponentsOut->size() < componentCount;
                 ++componentIndex)
            {
                const spirv::IdRef componentId = mBuilder.getNewId(decorations);
                spirv::WriteCompositeExtract(
                    mBuilder.getSpirvCurrentFunctionBlock(), componentTypeId, componentId,
                    parameterId,
                    {spirv::LiteralInteger(columnIndex), spirv::LiteralInteger(componentIndex)});

                extractedComponentsOut->push_back(componentId);
            }
        }
    }
}

void OutputSPIRVTraverser::startShortCircuit(TIntermBinary *node)
{
    // Emulate && and || as such:
    //
    //   || => if (!left) result = right
    //   && => if ( left) result = right
    //
    // When this function is called, |left| has already been visited, so it creates the appropriate
    // |if| construct in preparation for visiting |right|.

    // Load |left| and replace the access chain with an rvalue that's the result.
    spirv::IdRef typeId;
    const spirv::IdRef left =
        accessChainLoad(&mNodeData.back(), node->getLeft()->getType(), &typeId);
    nodeDataInitRValue(&mNodeData.back(), left, typeId);

    // Keep the id of the block |left| was evaluated in.
    mNodeData.back().idList.push_back(mBuilder.getSpirvCurrentFunctionBlockId());

    // Two blocks necessary, one for the |if| block, and one for the merge block.
    mBuilder.startConditional(2, false, false);

    // Generate the branch instructions.
    const SpirvConditional *conditional = mBuilder.getCurrentConditional();

    const spirv::IdRef mergeBlock = conditional->blockIds.back();
    const spirv::IdRef ifBlock    = conditional->blockIds.front();
    const spirv::IdRef trueBlock  = node->getOp() == EOpLogicalAnd ? ifBlock : mergeBlock;
    const spirv::IdRef falseBlock = node->getOp() == EOpLogicalOr ? ifBlock : mergeBlock;

    // Note that no logical not is necessary.  For ||, the branch will target the merge block in the
    // true case.
    mBuilder.writeBranchConditional(left, trueBlock, falseBlock, mergeBlock);
}

spirv::IdRef OutputSPIRVTraverser::endShortCircuit(TIntermBinary *node, spirv::IdRef *typeId)
{
    // Load the right hand side.
    const spirv::IdRef right =
        accessChainLoad(&mNodeData.back(), node->getRight()->getType(), nullptr);
    mNodeData.pop_back();

    // Get the id of the block |right| is evaluated in.
    const spirv::IdRef rightBlockId = mBuilder.getSpirvCurrentFunctionBlockId();

    // And the cached id of the block |left| is evaluated in.
    ASSERT(mNodeData.back().idList.size() == 1);
    const spirv::IdRef leftBlockId = mNodeData.back().idList[0].id;
    mNodeData.back().idList.clear();

    // Move on to the merge block.
    mBuilder.writeBranchConditionalBlockEnd();

    // Pop from the conditional stack.
    mBuilder.endConditional();

    // Get the previously loaded result of the left hand side.
    *typeId                 = mNodeData.back().accessChain.baseTypeId;
    const spirv::IdRef left = mNodeData.back().baseId;

    // Create an OpPhi instruction that selects either the |left| or |right| based on which block
    // was traversed.
    const spirv::IdRef result = mBuilder.getNewId(mBuilder.getDecorations(node->getType()));

    spirv::WritePhi(
        mBuilder.getSpirvCurrentFunctionBlock(), *typeId, result,
        {spirv::PairIdRefIdRef{left, leftBlockId}, spirv::PairIdRefIdRef{right, rightBlockId}});

    return result;
}

spirv::IdRef OutputSPIRVTraverser::createFunctionCall(TIntermAggregate *node,
                                                      spirv::IdRef resultTypeId)
{
    const TFunction *function = node->getFunction();
    ASSERT(function);

    ASSERT(mFunctionIdMap.count(function) > 0);
    const spirv::IdRef functionId = mFunctionIdMap[function].functionId;

    // Get the list of parameters passed to the function.  The function parameters can only be
    // memory variables, or if the function argument is |const|, an rvalue.
    //
    // For opaque uniforms, pass it directly as lvalue.
    //
    // For in variables:
    //
    // - If the parameter is const, pass it directly as rvalue, otherwise
    // - Write it to a temp variable first and pass that.
    //
    // For out variables:
    //
    // - Pass a temporary variable.  After the function call, copy that variable to the parameter.
    //
    // For inout variables:
    //
    // - Write the parameter to a temp variable and pass that.  After the function call, copy that
    //   variable back to the parameter.
    //
    // Note that in GLSL, in parameters are considered "copied" to the function.  In SPIR-V, every
    // parameter is implicitly inout.  If a function takes an in parameter and modifies it, the
    // caller has to ensure that it calls the function with a copy.  Currently, the functions don't
    // track whether an in parameter is modified, so we conservatively assume it is.  Even for out
    // and inout parameters, GLSL expects each function to operate on their local copy until the end
    // of the function; this has observable side effects if the out variable aliases another
    // variable the function has access to (another out variable, a global variable etc).
    //
    const size_t parameterCount = node->getChildCount();
    spirv::IdRefList parameters;
    spirv::IdRefList tempVarIds(parameterCount);
    spirv::IdRefList tempVarTypeIds(parameterCount);

    for (size_t paramIndex = 0; paramIndex < parameterCount; ++paramIndex)
    {
        const TType &paramType           = function->getParam(paramIndex)->getType();
        const TType &argType             = node->getChildNode(paramIndex)->getAsTyped()->getType();
        const TQualifier &paramQualifier = paramType.getQualifier();
        NodeData &param = mNodeData[mNodeData.size() - parameterCount + paramIndex];

        spirv::IdRef paramValue;

        if (paramQualifier == EvqParamConst)
        {
            // |const| parameters are passed as rvalue.
            paramValue = accessChainLoad(&param, argType, nullptr);
        }
        else if (IsOpaqueType(paramType.getBasicType()))
        {
            // Opaque uniforms are passed by pointer.
            paramValue = accessChainCollapse(&param);
        }
        else
        {
            ASSERT(paramQualifier == EvqParamIn || paramQualifier == EvqParamOut ||
                   paramQualifier == EvqParamInOut);

            // Need to create a temp variable and pass that.
            tempVarTypeIds[paramIndex] = mBuilder.getTypeData(paramType, {}).id;
            tempVarIds[paramIndex]     = mBuilder.declareVariable(
                tempVarTypeIds[paramIndex], spv::StorageClassFunction,
                mBuilder.getDecorations(argType), nullptr, "param", nullptr);

            // If it's an in or inout parameter, the temp variable needs to be initialized with the
            // value of the parameter first.
            if (paramQualifier == EvqParamIn || paramQualifier == EvqParamInOut)
            {
                paramValue = accessChainLoad(&param, argType, nullptr);
                spirv::WriteStore(mBuilder.getSpirvCurrentFunctionBlock(), tempVarIds[paramIndex],
                                  paramValue, nullptr);
            }

            paramValue = tempVarIds[paramIndex];
        }

        parameters.push_back(paramValue);
    }

    // Make the actual function call.
    const spirv::IdRef result = mBuilder.getNewId(mBuilder.getDecorations(node->getType()));
    spirv::WriteFunctionCall(mBuilder.getSpirvCurrentFunctionBlock(), resultTypeId, result,
                             functionId, parameters);

    // Copy from the out and inout temp variables back to the original parameters.
    for (size_t paramIndex = 0; paramIndex < parameterCount; ++paramIndex)
    {
        if (!tempVarIds[paramIndex].valid())
        {
            continue;
        }

        const TType &paramType           = function->getParam(paramIndex)->getType();
        const TType &argType             = node->getChildNode(paramIndex)->getAsTyped()->getType();
        const TQualifier &paramQualifier = paramType.getQualifier();
        NodeData &param = mNodeData[mNodeData.size() - parameterCount + paramIndex];

        if (paramQualifier == EvqParamIn)
        {
            continue;
        }

        // Copy from the temp variable to the parameter.
        NodeData tempVarData;
        nodeDataInitLValue(&tempVarData, tempVarIds[paramIndex], tempVarTypeIds[paramIndex],
                           spv::StorageClassFunction, {});
        const spirv::IdRef tempVarValue = accessChainLoad(&tempVarData, argType, nullptr);
        accessChainStore(&param, tempVarValue, function->getParam(paramIndex)->getType());
    }

    return result;
}

void OutputSPIRVTraverser::visitArrayLength(TIntermUnary *node)
{
    // .length() on sized arrays is already constant folded, so this operation only applies to
    // ssbo[N].last_member.length().  OpArrayLength takes the ssbo block *pointer* and the field
    // index of last_member, so those need to be extracted from the access chain.  Additionally,
    // OpArrayLength produces an unsigned int while GLSL produces an int, so a final cast is
    // necessary.

    // Inspect the children.  There are two possibilities:
    //
    // - last_member.length(): In this case, the id of the nameless ssbo is used.
    // - ssbo.last_member.length(): In this case, the id of the variable |ssbo| itself is used.
    // - ssbo[N][M].last_member.length(): In this case, the access chain |ssbo N M| is used.
    //
    // We can't visit the child in its entirety as it will create the access chain |ssbo N M field|
    // which is not useful.

    spirv::IdRef accessChainId;
    spirv::LiteralInteger fieldIndex;

    if (node->getOperand()->getAsSymbolNode())
    {
        // If the operand is a symbol referencing the last member of a nameless interface block,
        // visit the symbol to get the id of the interface block.
        node->getOperand()->getAsSymbolNode()->traverse(this);

        // The access chain must only include the base id + one literal field index.
        ASSERT(mNodeData.back().idList.size() == 1 && !mNodeData.back().idList.back().id.valid());

        accessChainId = mNodeData.back().baseId;
        fieldIndex    = mNodeData.back().idList.back().literal;
    }
    else
    {
        // Otherwise make sure not to traverse the field index selection node so that the access
        // chain would not include it.
        TIntermBinary *fieldSelectionNode = node->getOperand()->getAsBinaryNode();
        ASSERT(fieldSelectionNode && fieldSelectionNode->getOp() == EOpIndexDirectInterfaceBlock);

        TIntermTyped *interfaceBlockExpression = fieldSelectionNode->getLeft();
        TIntermConstantUnion *indexNode = fieldSelectionNode->getRight()->getAsConstantUnion();
        ASSERT(indexNode);

        // Visit the expression.
        interfaceBlockExpression->traverse(this);

        accessChainId = accessChainCollapse(&mNodeData.back());
        fieldIndex    = spirv::LiteralInteger(indexNode->getIConst(0));
    }

    // Get the int and uint type ids.
    const spirv::IdRef intTypeId  = mBuilder.getBasicTypeId(EbtInt, 1);
    const spirv::IdRef uintTypeId = mBuilder.getBasicTypeId(EbtUInt, 1);

    // Generate the instruction.
    const spirv::IdRef resultId = mBuilder.getNewId({});
    spirv::WriteArrayLength(mBuilder.getSpirvCurrentFunctionBlock(), uintTypeId, resultId,
                            accessChainId, fieldIndex);

    // Cast to int.
    const spirv::IdRef castResultId = mBuilder.getNewId({});
    spirv::WriteBitcast(mBuilder.getSpirvCurrentFunctionBlock(), intTypeId, castResultId, resultId);

    // Replace the access chain with an rvalue that's the result.
    nodeDataInitRValue(&mNodeData.back(), castResultId, intTypeId);
}

bool IsShortCircuitNeeded(TIntermOperator *node)
{
    TOperator op = node->getOp();

    // Short circuit is only necessary for && and ||.
    if (op != EOpLogicalAnd && op != EOpLogicalOr)
    {
        return false;
    }

    ASSERT(node->getChildCount() == 2);

    // If the right hand side does not have side effects, short-circuiting is unnecessary.
    // TODO: experiment with the performance of OpLogicalAnd/Or vs short-circuit based on the
    // complexity of the right hand side expression.  We could potentially only allow
    // OpLogicalAnd/Or if the right hand side is a constant or an access chain and have more complex
    // expressions be placed inside an if block.  http://anglebug.com/40096715
    return node->getChildNode(1)->getAsTyped()->hasSideEffects();
}

using WriteUnaryOp      = void (*)(spirv::Blob *blob,
                              spirv::IdResultType idResultType,
                              spirv::IdResult idResult,
                              spirv::IdRef operand);
using WriteBinaryOp     = void (*)(spirv::Blob *blob,
                               spirv::IdResultType idResultType,
                               spirv::IdResult idResult,
                               spirv::IdRef operand1,
                               spirv::IdRef operand2);
using WriteTernaryOp    = void (*)(spirv::Blob *blob,
                                spirv::IdResultType idResultType,
                                spirv::IdResult idResult,
                                spirv::IdRef operand1,
                                spirv::IdRef operand2,
                                spirv::IdRef operand3);
using WriteQuaternaryOp = void (*)(spirv::Blob *blob,
                                   spirv::IdResultType idResultType,
                                   spirv::IdResult idResult,
                                   spirv::IdRef operand1,
                                   spirv::IdRef operand2,
                                   spirv::IdRef operand3,
                                   spirv::IdRef operand4);
using WriteAtomicOp     = void (*)(spirv::Blob *blob,
                               spirv::IdResultType idResultType,
                               spirv::IdResult idResult,
                               spirv::IdRef pointer,
                               spirv::IdScope scope,
                               spirv::IdMemorySemantics semantics,
                               spirv::IdRef value);

spirv::IdRef OutputSPIRVTraverser::visitOperator(TIntermOperator *node, spirv::IdRef resultTypeId)
{
    // Handle special groups.
    const TOperator op = node->getOp();
    if (op == EOpEqual || op == EOpNotEqual)
    {
        return createCompare(node, resultTypeId);
    }
    if (BuiltInGroup::IsAtomicMemory(op) || BuiltInGroup::IsImageAtomic(op))
    {
        return createAtomicBuiltIn(node, resultTypeId);
    }
    if (BuiltInGroup::IsImage(op) || BuiltInGroup::IsTexture(op))
    {
        return createImageTextureBuiltIn(node, resultTypeId);
    }
    if (op == EOpSubpassLoad)
    {
        return createSubpassLoadBuiltIn(node, resultTypeId);
    }
    if (BuiltInGroup::IsInterpolationFS(op))
    {
        return createInterpolate(node, resultTypeId);
    }

    const size_t childCount   = node->getChildCount();
    TIntermTyped *firstChild  = node->getChildNode(0)->getAsTyped();
    TIntermTyped *secondChild = childCount > 1 ? node->getChildNode(1)->getAsTyped() : nullptr;

    const TType &firstOperandType = firstChild->getType();
    const TBasicType basicType    = firstOperandType.getBasicType();
    const bool isFloat            = basicType == EbtFloat;
    const bool isUnsigned         = basicType == EbtUInt;
    const bool isBool             = basicType == EbtBool;
    // Whether this is a pre/post increment/decrement operator.
    bool isIncrementOrDecrement = false;
    // Whether the operation needs to be applied column by column.
    bool operateOnColumns =
        childCount == 2 && (firstChild->getType().isMatrix() || secondChild->getType().isMatrix());
    // Whether the operands need to be swapped in the (binary) instruction
    bool binarySwapOperands = false;
    // Whether the instruction is matrix/scalar.  This is implemented with matrix*(1/scalar).
    bool binaryInvertSecondParameter = false;
    // Whether the scalar operand needs to be extended to match the other operand which is a vector
    // (in a binary or extended operation).
    bool extendScalarToVector = true;
    // Some built-ins have out parameters at the end of the list of parameters.
    size_t lvalueCount = 0;

    WriteUnaryOp writeUnaryOp           = nullptr;
    WriteBinaryOp writeBinaryOp         = nullptr;
    WriteTernaryOp writeTernaryOp       = nullptr;
    WriteQuaternaryOp writeQuaternaryOp = nullptr;

    // Some operators are implemented with an extended instruction.
    spv::GLSLstd450 extendedInst = spv::GLSLstd450Bad;

    switch (op)
    {
        case EOpNegative:
            operateOnColumns = firstOperandType.isMatrix();
            if (isFloat)
                writeUnaryOp = spirv::WriteFNegate;
            else
                writeUnaryOp = spirv::WriteSNegate;
            break;
        case EOpPositive:
            // This is a noop.
            return accessChainLoad(&mNodeData.back(), firstOperandType, nullptr);

        case EOpLogicalNot:
        case EOpNotComponentWise:
            writeUnaryOp = spirv::WriteLogicalNot;
            break;
        case EOpBitwiseNot:
            writeUnaryOp = spirv::WriteNot;
            break;

        case EOpPostIncrement:
        case EOpPreIncrement:
            isIncrementOrDecrement = true;
            operateOnColumns       = firstOperandType.isMatrix();
            if (isFloat)
                writeBinaryOp = spirv::WriteFAdd;
            else
                writeBinaryOp = spirv::WriteIAdd;
            break;
        case EOpPostDecrement:
        case EOpPreDecrement:
            isIncrementOrDecrement = true;
            operateOnColumns       = firstOperandType.isMatrix();
            if (isFloat)
                writeBinaryOp = spirv::WriteFSub;
            else
                writeBinaryOp = spirv::WriteISub;
            break;

        case EOpAdd:
        case EOpAddAssign:
            if (isFloat)
                writeBinaryOp = spirv::WriteFAdd;
            else
                writeBinaryOp = spirv::WriteIAdd;
            break;
        case EOpSub:
        case EOpSubAssign:
            if (isFloat)
                writeBinaryOp = spirv::WriteFSub;
            else
                writeBinaryOp = spirv::WriteISub;
            break;
        case EOpMul:
        case EOpMulAssign:
        case EOpMatrixCompMult:
            if (isFloat)
                writeBinaryOp = spirv::WriteFMul;
            else
                writeBinaryOp = spirv::WriteIMul;
            break;
        case EOpDiv:
        case EOpDivAssign:
            if (isFloat)
            {
                if (firstOperandType.isMatrix() && secondChild->getType().isScalar())
                {
                    writeBinaryOp               = spirv::WriteMatrixTimesScalar;
                    binaryInvertSecondParameter = true;
                    operateOnColumns            = false;
                    extendScalarToVector        = false;
                }
                else
                {
                    writeBinaryOp = spirv::WriteFDiv;
                }
            }
            else if (isUnsigned)
                writeBinaryOp = spirv::WriteUDiv;
            else
                writeBinaryOp = spirv::WriteSDiv;
            break;
        case EOpIMod:
        case EOpIModAssign:
            if (isFloat)
                writeBinaryOp = spirv::WriteFMod;
            else if (isUnsigned)
                writeBinaryOp = spirv::WriteUMod;
            else
                writeBinaryOp = spirv::WriteSMod;
            break;

        case EOpEqualComponentWise:
            if (isFloat)
                writeBinaryOp = spirv::WriteFOrdEqual;
            else if (isBool)
                writeBinaryOp = spirv::WriteLogicalEqual;
            else
                writeBinaryOp = spirv::WriteIEqual;
            break;
        case EOpNotEqualComponentWise:
            if (isFloat)
                writeBinaryOp = spirv::WriteFUnordNotEqual;
            else if (isBool)
                writeBinaryOp = spirv::WriteLogicalNotEqual;
            else
                writeBinaryOp = spirv::WriteINotEqual;
            break;
        case EOpLessThan:
        case EOpLessThanComponentWise:
            if (isFloat)
                writeBinaryOp = spirv::WriteFOrdLessThan;
            else if (isUnsigned)
                writeBinaryOp = spirv::WriteULessThan;
            else
                writeBinaryOp = spirv::WriteSLessThan;
            break;
        case EOpGreaterThan:
        case EOpGreaterThanComponentWise:
            if (isFloat)
                writeBinaryOp = spirv::WriteFOrdGreaterThan;
            else if (isUnsigned)
                writeBinaryOp = spirv::WriteUGreaterThan;
            else
                writeBinaryOp = spirv::WriteSGreaterThan;
            break;
        case EOpLessThanEqual:
        case EOpLessThanEqualComponentWise:
            if (isFloat)
                writeBinaryOp = spirv::WriteFOrdLessThanEqual;
            else if (isUnsigned)
                writeBinaryOp = spirv::WriteULessThanEqual;
            else
                writeBinaryOp = spirv::WriteSLessThanEqual;
            break;
        case EOpGreaterThanEqual:
        case EOpGreaterThanEqualComponentWise:
            if (isFloat)
                writeBinaryOp = spirv::WriteFOrdGreaterThanEqual;
            else if (isUnsigned)
                writeBinaryOp = spirv::WriteUGreaterThanEqual;
            else
                writeBinaryOp = spirv::WriteSGreaterThanEqual;
            break;

        case EOpVectorTimesScalar:
        case EOpVectorTimesScalarAssign:
            if (isFloat)
            {
                writeBinaryOp        = spirv::WriteVectorTimesScalar;
                binarySwapOperands   = node->getChildNode(1)->getAsTyped()->getType().isVector();
                extendScalarToVector = false;
            }
            else
                writeBinaryOp = spirv::WriteIMul;
            break;
        case EOpVectorTimesMatrix:
        case EOpVectorTimesMatrixAssign:
            writeBinaryOp    = spirv::WriteVectorTimesMatrix;
            operateOnColumns = false;
            break;
        case EOpMatrixTimesVector:
            writeBinaryOp    = spirv::WriteMatrixTimesVector;
            operateOnColumns = false;
            break;
        case EOpMatrixTimesScalar:
        case EOpMatrixTimesScalarAssign:
            writeBinaryOp        = spirv::WriteMatrixTimesScalar;
            binarySwapOperands   = secondChild->getType().isMatrix();
            operateOnColumns     = false;
            extendScalarToVector = false;
            break;
        case EOpMatrixTimesMatrix:
        case EOpMatrixTimesMatrixAssign:
            writeBinaryOp    = spirv::WriteMatrixTimesMatrix;
            operateOnColumns = false;
            break;

        case EOpLogicalOr:
            ASSERT(!IsShortCircuitNeeded(node));
            extendScalarToVector = false;
            writeBinaryOp        = spirv::WriteLogicalOr;
            break;
        case EOpLogicalXor:
            extendScalarToVector = false;
            writeBinaryOp        = spirv::WriteLogicalNotEqual;
            break;
        case EOpLogicalAnd:
            ASSERT(!IsShortCircuitNeeded(node));
            extendScalarToVector = false;
            writeBinaryOp        = spirv::WriteLogicalAnd;
            break;

        case EOpBitShiftLeft:
        case EOpBitShiftLeftAssign:
            writeBinaryOp = spirv::WriteShiftLeftLogical;
            break;
        case EOpBitShiftRight:
        case EOpBitShiftRightAssign:
            if (isUnsigned)
                writeBinaryOp = spirv::WriteShiftRightLogical;
            else
                writeBinaryOp = spirv::WriteShiftRightArithmetic;
            break;
        case EOpBitwiseAnd:
        case EOpBitwiseAndAssign:
            writeBinaryOp = spirv::WriteBitwiseAnd;
            break;
        case EOpBitwiseXor:
        case EOpBitwiseXorAssign:
            writeBinaryOp = spirv::WriteBitwiseXor;
            break;
        case EOpBitwiseOr:
        case EOpBitwiseOrAssign:
            writeBinaryOp = spirv::WriteBitwiseOr;
            break;

        case EOpRadians:
            extendedInst = spv::GLSLstd450Radians;
            break;
        case EOpDegrees:
            extendedInst = spv::GLSLstd450Degrees;
            break;
        case EOpSin:
            extendedInst = spv::GLSLstd450Sin;
            break;
        case EOpCos:
            extendedInst = spv::GLSLstd450Cos;
            break;
        case EOpTan:
            extendedInst = spv::GLSLstd450Tan;
            break;
        case EOpAsin:
            extendedInst = spv::GLSLstd450Asin;
            break;
        case EOpAcos:
            extendedInst = spv::GLSLstd450Acos;
            break;
        case EOpAtan:
            extendedInst = childCount == 1 ? spv::GLSLstd450Atan : spv::GLSLstd450Atan2;
            break;
        case EOpSinh:
            extendedInst = spv::GLSLstd450Sinh;
            break;
        case EOpCosh:
            extendedInst = spv::GLSLstd450Cosh;
            break;
        case EOpTanh:
            extendedInst = spv::GLSLstd450Tanh;
            break;
        case EOpAsinh:
            extendedInst = spv::GLSLstd450Asinh;
            break;
        case EOpAcosh:
            extendedInst = spv::GLSLstd450Acosh;
            break;
        case EOpAtanh:
            extendedInst = spv::GLSLstd450Atanh;
            break;

        case EOpPow:
            extendedInst = spv::GLSLstd450Pow;
            break;
        case EOpExp:
            extendedInst = spv::GLSLstd450Exp;
            break;
        case EOpLog:
            extendedInst = spv::GLSLstd450Log;
            break;
        case EOpExp2:
            extendedInst = spv::GLSLstd450Exp2;
            break;
        case EOpLog2:
            extendedInst = spv::GLSLstd450Log2;
            break;
        case EOpSqrt:
            extendedInst = spv::GLSLstd450Sqrt;
            break;
        case EOpInversesqrt:
            extendedInst = spv::GLSLstd450InverseSqrt;
            break;

        case EOpAbs:
            if (isFloat)
                extendedInst = spv::GLSLstd450FAbs;
            else
                extendedInst = spv::GLSLstd450SAbs;
            break;
        case EOpSign:
            if (isFloat)
                extendedInst = spv::GLSLstd450FSign;
            else
                extendedInst = spv::GLSLstd450SSign;
            break;
        case EOpFloor:
            extendedInst = spv::GLSLstd450Floor;
            break;
        case EOpTrunc:
            extendedInst = spv::GLSLstd450Trunc;
            break;
        case EOpRound:
            extendedInst = spv::GLSLstd450Round;
            break;
        case EOpRoundEven:
            extendedInst = spv::GLSLstd450RoundEven;
            break;
        case EOpCeil:
            extendedInst = spv::GLSLstd450Ceil;
            break;
        case EOpFract:
            extendedInst = spv::GLSLstd450Fract;
            break;
        case EOpMod:
            if (isFloat)
                writeBinaryOp = spirv::WriteFMod;
            else if (isUnsigned)
                writeBinaryOp = spirv::WriteUMod;
            else
                writeBinaryOp = spirv::WriteSMod;
            break;
        case EOpMin:
            if (isFloat)
                extendedInst = spv::GLSLstd450FMin;
            else if (isUnsigned)
                extendedInst = spv::GLSLstd450UMin;
            else
                extendedInst = spv::GLSLstd450SMin;
            break;
        case EOpMax:
            if (isFloat)
                extendedInst = spv::GLSLstd450FMax;
            else if (isUnsigned)
                extendedInst = spv::GLSLstd450UMax;
            else
                extendedInst = spv::GLSLstd450SMax;
            break;
        case EOpClamp:
            if (isFloat)
                extendedInst = spv::GLSLstd450FClamp;
            else if (isUnsigned)
                extendedInst = spv::GLSLstd450UClamp;
            else
                extendedInst = spv::GLSLstd450SClamp;
            break;
        case EOpMix:
            if (node->getChildNode(childCount - 1)->getAsTyped()->getType().getBasicType() ==
                EbtBool)
            {
                writeTernaryOp = spirv::WriteSelect;
            }
            else
            {
                ASSERT(isFloat);
                extendedInst = spv::GLSLstd450FMix;
            }
            break;
        case EOpStep:
            extendedInst = spv::GLSLstd450Step;
            break;
        case EOpSmoothstep:
            extendedInst = spv::GLSLstd450SmoothStep;
            break;
        case EOpModf:
            extendedInst = spv::GLSLstd450ModfStruct;
            lvalueCount  = 1;
            break;
        case EOpIsnan:
            writeUnaryOp = spirv::WriteIsNan;
            break;
        case EOpIsinf:
            writeUnaryOp = spirv::WriteIsInf;
            break;
        case EOpFloatBitsToInt:
        case EOpFloatBitsToUint:
        case EOpIntBitsToFloat:
        case EOpUintBitsToFloat:
            writeUnaryOp = spirv::WriteBitcast;
            break;
        case EOpFma:
            extendedInst = spv::GLSLstd450Fma;
            break;
        case EOpFrexp:
            extendedInst = spv::GLSLstd450FrexpStruct;
            lvalueCount  = 1;
            break;
        case EOpLdexp:
            extendedInst = spv::GLSLstd450Ldexp;
            break;
        case EOpPackSnorm2x16:
            extendedInst = spv::GLSLstd450PackSnorm2x16;
            break;
        case EOpPackUnorm2x16:
            extendedInst = spv::GLSLstd450PackUnorm2x16;
            break;
        case EOpPackHalf2x16:
            extendedInst = spv::GLSLstd450PackHalf2x16;
            break;
        case EOpUnpackSnorm2x16:
            extendedInst         = spv::GLSLstd450UnpackSnorm2x16;
            extendScalarToVector = false;
            break;
        case EOpUnpackUnorm2x16:
            extendedInst         = spv::GLSLstd450UnpackUnorm2x16;
            extendScalarToVector = false;
            break;
        case EOpUnpackHalf2x16:
            extendedInst         = spv::GLSLstd450UnpackHalf2x16;
            extendScalarToVector = false;
            break;
        case EOpPackUnorm4x8:
            extendedInst = spv::GLSLstd450PackUnorm4x8;
            break;
        case EOpPackSnorm4x8:
            extendedInst = spv::GLSLstd450PackSnorm4x8;
            break;
        case EOpUnpackUnorm4x8:
            extendedInst         = spv::GLSLstd450UnpackUnorm4x8;
            extendScalarToVector = false;
            break;
        case EOpUnpackSnorm4x8:
            extendedInst         = spv::GLSLstd450UnpackSnorm4x8;
            extendScalarToVector = false;
            break;

        case EOpLength:
            extendedInst = spv::GLSLstd450Length;
            break;
        case EOpDistance:
            extendedInst = spv::GLSLstd450Distance;
            break;
        case EOpDot:
            // Use normal multiplication for scalars.
            if (firstOperandType.isScalar())
            {
                if (isFloat)
                    writeBinaryOp = spirv::WriteFMul;
                else
                    writeBinaryOp = spirv::WriteIMul;
            }
            else
            {
                writeBinaryOp = spirv::WriteDot;
            }
            break;
        case EOpCross:
            extendedInst = spv::GLSLstd450Cross;
            break;
        case EOpNormalize:
            extendedInst = spv::GLSLstd450Normalize;
            break;
        case EOpFaceforward:
            extendedInst = spv::GLSLstd450FaceForward;
            break;
        case EOpReflect:
            extendedInst = spv::GLSLstd450Reflect;
            break;
        case EOpRefract:
            extendedInst         = spv::GLSLstd450Refract;
            extendScalarToVector = false;
            break;

        case EOpOuterProduct:
            writeBinaryOp = spirv::WriteOuterProduct;
            break;
        case EOpTranspose:
            writeUnaryOp = spirv::WriteTranspose;
            break;
        case EOpDeterminant:
            extendedInst = spv::GLSLstd450Determinant;
            break;
        case EOpInverse:
            extendedInst = spv::GLSLstd450MatrixInverse;
            break;

        case EOpAny:
            writeUnaryOp = spirv::WriteAny;
            break;
        case EOpAll:
            writeUnaryOp = spirv::WriteAll;
            break;

        case EOpBitfieldExtract:
            if (isUnsigned)
                writeTernaryOp = spirv::WriteBitFieldUExtract;
            else
                writeTernaryOp = spirv::WriteBitFieldSExtract;
            break;
        case EOpBitfieldInsert:
            writeQuaternaryOp = spirv::WriteBitFieldInsert;
            break;
        case EOpBitfieldReverse:
            writeUnaryOp = spirv::WriteBitReverse;
            break;
        case EOpBitCount:
            writeUnaryOp = spirv::WriteBitCount;
            break;
        case EOpFindLSB:
            extendedInst = spv::GLSLstd450FindILsb;
            break;
        case EOpFindMSB:
            if (isUnsigned)
                extendedInst = spv::GLSLstd450FindUMsb;
            else
                extendedInst = spv::GLSLstd450FindSMsb;
            break;
        case EOpUaddCarry:
            writeBinaryOp = spirv::WriteIAddCarry;
            lvalueCount   = 1;
            break;
        case EOpUsubBorrow:
            writeBinaryOp = spirv::WriteISubBorrow;
            lvalueCount   = 1;
            break;
        case EOpUmulExtended:
            writeBinaryOp = spirv::WriteUMulExtended;
            lvalueCount   = 2;
            break;
        case EOpImulExtended:
            writeBinaryOp = spirv::WriteSMulExtended;
            lvalueCount   = 2;
            break;

        case EOpRgb_2_yuv:
        case EOpYuv_2_rgb:
            // These built-ins are emulated, and shouldn't be encountered at this point.
            UNREACHABLE();
            break;

        case EOpDFdx:
            writeUnaryOp = spirv::WriteDPdx;
            break;
        case EOpDFdy:
            writeUnaryOp = spirv::WriteDPdy;
            break;
        case EOpFwidth:
            writeUnaryOp = spirv::WriteFwidth;
            break;

        default:
            UNREACHABLE();
    }

    // Load the parameters.
    spirv::IdRefList parameterTypeIds;
    spirv::IdRefList parameters = loadAllParams(node, lvalueCount, &parameterTypeIds);

    if (isIncrementOrDecrement)
    {
        // ++ and -- are implemented with binary add and subtract, so add an implicit parameter with
        // size vecN(1).
        const uint8_t vecSize = firstOperandType.isMatrix() ? firstOperandType.getRows()
                                                            : firstOperandType.getNominalSize();
        const spirv::IdRef one =
            isFloat ? mBuilder.getVecConstant(1, vecSize) : mBuilder.getIvecConstant(1, vecSize);
        parameters.push_back(one);
    }

    const SpirvDecorations decorations =
        mBuilder.getArithmeticDecorations(node->getType(), node->isPrecise(), op);
    spirv::IdRef result;
    if (node->getType().getBasicType() != EbtVoid)
    {
        result = mBuilder.getNewId(decorations);
    }

    // In the case of modf, frexp, uaddCarry, usubBorrow, umulExtended and imulExtended, the SPIR-V
    // result is expected to be a struct instead.
    spirv::IdRef builtInResultTypeId = resultTypeId;
    spirv::IdRef builtInResult;
    if (lvalueCount > 0)
    {
        builtInResultTypeId = makeBuiltInOutputStructType(node, lvalueCount);
        builtInResult       = mBuilder.getNewId({});
    }
    else
    {
        builtInResult = result;
    }

    if (operateOnColumns)
    {
        // If negating a matrix, multiplying or comparing them, do that column by column.
        // Matrix-scalar operations (add, sub, mod etc) turn the scalar into a vector before
        // operating on the column.
        spirv::IdRefList columnIds;

        const SpirvDecorations operandDecorations = mBuilder.getDecorations(firstOperandType);

        const TType &matrixType =
            firstOperandType.isMatrix() ? firstOperandType : secondChild->getType();

        const spirv::IdRef columnTypeId =
            mBuilder.getBasicTypeId(matrixType.getBasicType(), matrixType.getRows());

        if (binarySwapOperands)
        {
            std::swap(parameters[0], parameters[1]);
        }

        if (extendScalarToVector)
        {
            extendScalarParamsToVector(node, columnTypeId, &parameters);
        }

        // Extract and apply the operator to each column.
        for (uint8_t columnIndex = 0; columnIndex < matrixType.getCols(); ++columnIndex)
        {
            spirv::IdRef columnIdA = parameters[0];
            if (firstOperandType.isMatrix())
            {
                columnIdA = mBuilder.getNewId(operandDecorations);
                spirv::WriteCompositeExtract(mBuilder.getSpirvCurrentFunctionBlock(), columnTypeId,
                                             columnIdA, parameters[0],
                                             {spirv::LiteralInteger(columnIndex)});
            }

            columnIds.push_back(mBuilder.getNewId(decorations));

            if (writeUnaryOp)
            {
                writeUnaryOp(mBuilder.getSpirvCurrentFunctionBlock(), columnTypeId,
                             columnIds.back(), columnIdA);
            }
            else
            {
                ASSERT(writeBinaryOp);

                spirv::IdRef columnIdB = parameters[1];
                if (secondChild != nullptr && secondChild->getType().isMatrix())
                {
                    columnIdB = mBuilder.getNewId(operandDecorations);
                    spirv::WriteCompositeExtract(mBuilder.getSpirvCurrentFunctionBlock(),
                                                 columnTypeId, columnIdB, parameters[1],
                                                 {spirv::LiteralInteger(columnIndex)});
                }

                writeBinaryOp(mBuilder.getSpirvCurrentFunctionBlock(), columnTypeId,
                              columnIds.back(), columnIdA, columnIdB);
            }
        }

        // Construct the result.
        spirv::WriteCompositeConstruct(mBuilder.getSpirvCurrentFunctionBlock(), builtInResultTypeId,
                                       builtInResult, columnIds);
    }
    else if (writeUnaryOp)
    {
        ASSERT(parameters.size() == 1);
        writeUnaryOp(mBuilder.getSpirvCurrentFunctionBlock(), builtInResultTypeId, builtInResult,
                     parameters[0]);
    }
    else if (writeBinaryOp)
    {
        ASSERT(parameters.size() == 2);

        if (extendScalarToVector)
        {
            extendScalarParamsToVector(node, builtInResultTypeId, &parameters);
        }

        if (binarySwapOperands)
        {
            std::swap(parameters[0], parameters[1]);
        }

        if (binaryInvertSecondParameter)
        {
            const spirv::IdRef one           = mBuilder.getFloatConstant(1);
            const spirv::IdRef invertedParam = mBuilder.getNewId(
                mBuilder.getArithmeticDecorations(secondChild->getType(), node->isPrecise(), op));
            spirv::WriteFDiv(mBuilder.getSpirvCurrentFunctionBlock(), parameterTypeIds.back(),
                             invertedParam, one, parameters[1]);
            parameters[1] = invertedParam;
        }

        // Write the operation that combines the left and right values.
        writeBinaryOp(mBuilder.getSpirvCurrentFunctionBlock(), builtInResultTypeId, builtInResult,
                      parameters[0], parameters[1]);
    }
    else if (writeTernaryOp)
    {
        ASSERT(parameters.size() == 3);

        // mix(a, b, bool) is the same as bool ? b : a;
        if (op == EOpMix)
        {
            std::swap(parameters[0], parameters[2]);
        }

        writeTernaryOp(mBuilder.getSpirvCurrentFunctionBlock(), builtInResultTypeId, builtInResult,
                       parameters[0], parameters[1], parameters[2]);
    }
    else if (writeQuaternaryOp)
    {
        ASSERT(parameters.size() == 4);

        writeQuaternaryOp(mBuilder.getSpirvCurrentFunctionBlock(), builtInResultTypeId,
                          builtInResult, parameters[0], parameters[1], parameters[2],
                          parameters[3]);
    }
    else
    {
        // It's an extended instruction.
        ASSERT(extendedInst != spv::GLSLstd450Bad);

        if (extendScalarToVector)
        {
            extendScalarParamsToVector(node, builtInResultTypeId, &parameters);
        }

        spirv::WriteExtInst(mBuilder.getSpirvCurrentFunctionBlock(), builtInResultTypeId,
                            builtInResult, mBuilder.getExtInstImportIdStd(),
                            spirv::LiteralExtInstInteger(extendedInst), parameters);
    }

    // If it's an assignment, store the calculated value.
    if (IsAssignment(node->getOp()))
    {
        accessChainStore(&mNodeData[mNodeData.size() - childCount], builtInResult,
                         firstOperandType);
    }

    // If the operation returns a struct, load the lsb and msb and store them in result/out
    // parameters.
    if (lvalueCount > 0)
    {
        storeBuiltInStructOutputInParamsAndReturnValue(node, lvalueCount, builtInResult, result,
                                                       resultTypeId);
    }

    // For post increment/decrement, return the value of the parameter itself as the result.
    if (op == EOpPostIncrement || op == EOpPostDecrement)
    {
        result = parameters[0];
    }

    return result;
}

spirv::IdRef OutputSPIRVTraverser::createCompare(TIntermOperator *node, spirv::IdRef resultTypeId)
{
    const TOperator op       = node->getOp();
    TIntermTyped *operand    = node->getChildNode(0)->getAsTyped();
    const TType &operandType = operand->getType();

    const SpirvDecorations resultDecorations  = mBuilder.getDecorations(node->getType());
    const SpirvDecorations operandDecorations = mBuilder.getDecorations(operandType);

    // Load the left and right values.
    spirv::IdRefList parameters = loadAllParams(node, 0, nullptr);
    ASSERT(parameters.size() == 2);

    // In GLSL, operators == and != can operate on the following:
    //
    // - scalars: There's a SPIR-V instruction for this,
    // - vectors: The same SPIR-V instruction as scalars is used here, but the result is reduced
    //   with OpAll/OpAny for == and != respectively,
    // - matrices: Comparison must be done column by column and the result reduced,
    // - arrays: Comparison must be done on every array element and the result reduced,
    // - structs: Comparison must be done on each field and the result reduced.
    //
    // For the latter 3 cases, OpCompositeExtract is used to extract scalars and vectors out of the
    // more complex type, which is recursively traversed.  The results are accumulated in a list
    // that is then reduced 4 by 4 elements until a single boolean is produced.

    spirv::LiteralIntegerList currentAccessChain;
    spirv::IdRefList intermediateResults;

    createCompareImpl(op, operandType, resultTypeId, parameters[0], parameters[1],
                      operandDecorations, resultDecorations, &currentAccessChain,
                      &intermediateResults);

    // Make sure the function correctly pushes and pops access chain indices.
    ASSERT(currentAccessChain.empty());

    // Reduce the intermediate results.
    ASSERT(!intermediateResults.empty());

    // The following code implements this algorithm, assuming N bools are to be reduced:
    //
    //    Reduced           To Reduce
    //     {b1}           {b2, b3, ..., bN}      Initial state
    //                                           Loop
    //  {b1, b2, b3, b4}  {b5, b6, ..., bN}        Take up to 3 new bools
    //     {r1}           {b5, b6, ..., bN}        Reduce it
    //                                             Repeat
    //
    // In the end, a single value is left.
    size_t reducedCount       = 0;
    spirv::IdRefList toReduce = {intermediateResults[reducedCount++]};
    while (reducedCount < intermediateResults.size())
    {
        // Take up to 3 new bools.
        size_t toTakeCount = std::min<size_t>(3, intermediateResults.size() - reducedCount);
        for (size_t i = 0; i < toTakeCount; ++i)
        {
            toReduce.push_back(intermediateResults[reducedCount++]);
        }

        // Reduce them to one bool.
        const spirv::IdRef result = reduceBoolVector(op, toReduce, resultTypeId, resultDecorations);

        // Replace the list of bools to reduce with the reduced one.
        toReduce.clear();
        toReduce.push_back(result);
    }

    ASSERT(toReduce.size() == 1 && reducedCount == intermediateResults.size());
    return toReduce[0];
}

spirv::IdRef OutputSPIRVTraverser::createAtomicBuiltIn(TIntermOperator *node,
                                                       spirv::IdRef resultTypeId)
{
    const TType &operandType          = node->getChildNode(0)->getAsTyped()->getType();
    const TBasicType operandBasicType = operandType.getBasicType();
    const bool isImage                = IsImage(operandBasicType);

    // Most atomic instructions are in the form of:
    //
    //     %result = OpAtomicX %pointer Scope MemorySemantics %value
    //
    // OpAtomicCompareSwap is exceptionally different (note that compare and value are in different
    // order from GLSL):
    //
    //     %result = OpAtomicCompareExchange %pointer
    //                                       Scope MemorySemantics MemorySemantics
    //                                       %value %comparator
    //
    // In all cases, the first parameter is the pointer, and the rest are rvalues.
    //
    // For images, OpImageTexelPointer is used to form a pointer to the texel on which the atomic
    // operation is being performed.
    const size_t parameterCount       = node->getChildCount();
    size_t imagePointerParameterCount = 0;
    spirv::IdRef pointerId;
    spirv::IdRefList imagePointerParameters;
    spirv::IdRefList parameters;

    if (isImage)
    {
        // One parameter for coordinates.
        ++imagePointerParameterCount;
        if (IsImageMS(operandBasicType))
        {
            // One parameter for samples.
            ++imagePointerParameterCount;
        }
    }

    ASSERT(parameterCount >= 2 + imagePointerParameterCount);

    pointerId = accessChainCollapse(&mNodeData[mNodeData.size() - parameterCount]);
    for (size_t paramIndex = 1; paramIndex < parameterCount; ++paramIndex)
    {
        NodeData &param              = mNodeData[mNodeData.size() - parameterCount + paramIndex];
        const spirv::IdRef parameter = accessChainLoad(
            &param, node->getChildNode(paramIndex)->getAsTyped()->getType(), nullptr);

        // imageAtomic* built-ins have a few additional parameters right after the image.  These are
        // kept separately for use with OpImageTexelPointer.
        if (paramIndex <= imagePointerParameterCount)
        {
            imagePointerParameters.push_back(parameter);
        }
        else
        {
            parameters.push_back(parameter);
        }
    }

    // The scope of the operation is always Device as we don't enable the Vulkan memory model
    // extension.
    const spirv::IdScope scopeId = mBuilder.getUintConstant(spv::ScopeDevice);

    // The memory semantics is always relaxed as we don't enable the Vulkan memory model extension.
    const spirv::IdMemorySemantics semanticsId =
        mBuilder.getUintConstant(spv::MemorySemanticsMaskNone);

    WriteAtomicOp writeAtomicOp = nullptr;

    const spirv::IdRef result = mBuilder.getNewId(mBuilder.getDecorations(node->getType()));

    // Determine whether the operation is on ints or uints.
    const bool isUnsigned = isImage ? IsUIntImage(operandBasicType) : operandBasicType == EbtUInt;

    // For images, convert the pointer to the image to a pointer to a texel in the image.
    if (isImage)
    {
        const spirv::IdRef texelTypePointerId =
            mBuilder.getTypePointerId(resultTypeId, spv::StorageClassImage);
        const spirv::IdRef texelPointerId = mBuilder.getNewId({});

        const spirv::IdRef coordinate = imagePointerParameters[0];
        spirv::IdRef sample = imagePointerParameters.size() > 1 ? imagePointerParameters[1]
                                                                : mBuilder.getUintConstant(0);

        spirv::WriteImageTexelPointer(mBuilder.getSpirvCurrentFunctionBlock(), texelTypePointerId,
                                      texelPointerId, pointerId, coordinate, sample);

        pointerId = texelPointerId;
    }

    switch (node->getOp())
    {
        case EOpAtomicAdd:
        case EOpImageAtomicAdd:
            writeAtomicOp = spirv::WriteAtomicIAdd;
            break;
        case EOpAtomicMin:
        case EOpImageAtomicMin:
            writeAtomicOp = isUnsigned ? spirv::WriteAtomicUMin : spirv::WriteAtomicSMin;
            break;
        case EOpAtomicMax:
        case EOpImageAtomicMax:
            writeAtomicOp = isUnsigned ? spirv::WriteAtomicUMax : spirv::WriteAtomicSMax;
            break;
        case EOpAtomicAnd:
        case EOpImageAtomicAnd:
            writeAtomicOp = spirv::WriteAtomicAnd;
            break;
        case EOpAtomicOr:
        case EOpImageAtomicOr:
            writeAtomicOp = spirv::WriteAtomicOr;
            break;
        case EOpAtomicXor:
        case EOpImageAtomicXor:
            writeAtomicOp = spirv::WriteAtomicXor;
            break;
        case EOpAtomicExchange:
        case EOpImageAtomicExchange:
            writeAtomicOp = spirv::WriteAtomicExchange;
            break;
        case EOpAtomicCompSwap:
        case EOpImageAtomicCompSwap:
            // Generate this special instruction right here and early out.  Note again that the
            // value and compare parameters of OpAtomicCompareExchange are in the opposite order
            // from GLSL.
            ASSERT(parameters.size() == 2);
            spirv::WriteAtomicCompareExchange(mBuilder.getSpirvCurrentFunctionBlock(), resultTypeId,
                                              result, pointerId, scopeId, semanticsId, semanticsId,
                                              parameters[1], parameters[0]);
            return result;
        default:
            UNREACHABLE();
    }

    // Write the instruction.
    ASSERT(parameters.size() == 1);
    writeAtomicOp(mBuilder.getSpirvCurrentFunctionBlock(), resultTypeId, result, pointerId, scopeId,
                  semanticsId, parameters[0]);

    return result;
}

spirv::IdRef OutputSPIRVTraverser::createImageTextureBuiltIn(TIntermOperator *node,
                                                             spirv::IdRef resultTypeId)
{
    const TOperator op                = node->getOp();
    const TFunction *function         = node->getAsAggregate()->getFunction();
    const TType &samplerType          = function->getParam(0)->getType();
    const TBasicType samplerBasicType = samplerType.getBasicType();

    // Load the parameters.
    spirv::IdRefList parameters = loadAllParams(node, 0, nullptr);

    // GLSL texture* and image* built-ins map to the following SPIR-V instructions.  Some of these
    // instructions take a "sampled image" while the others take the image itself.  In these
    // functions, the image, coordinates and Dref (for shadow sampling) are specified as positional
    // parameters while the rest are bundled in a list of image operands.
    //
    // Image operations that query:
    //
    // - OpImageQuerySizeLod
    // - OpImageQuerySize
    // - OpImageQueryLod <-- sampled image
    // - OpImageQueryLevels
    // - OpImageQuerySamples
    //
    // Image operations that read/write:
    //
    // - OpImageSampleImplicitLod <-- sampled image
    // - OpImageSampleExplicitLod <-- sampled image
    // - OpImageSampleDrefImplicitLod <-- sampled image
    // - OpImageSampleDrefExplicitLod <-- sampled image
    // - OpImageSampleProjImplicitLod <-- sampled image
    // - OpImageSampleProjExplicitLod <-- sampled image
    // - OpImageSampleProjDrefImplicitLod <-- sampled image
    // - OpImageSampleProjDrefExplicitLod <-- sampled image
    // - OpImageFetch
    // - OpImageGather <-- sampled image
    // - OpImageDrefGather <-- sampled image
    // - OpImageRead
    // - OpImageWrite
    //
    // The additional image parameters are:
    //
    // - Bias: Only used with ImplicitLod.
    // - Lod: Only used with ExplicitLod.
    // - Grad: 2x operands; dx and dy.  Only used with ExplicitLod.
    // - ConstOffset: Constant offset added to coordinates of OpImage*Gather.
    // - Offset: Non-constant offset added to coordinates of OpImage*Gather.
    // - ConstOffsets: Constant offsets added to coordinates of OpImage*Gather.
    // - Sample: Only used with OpImageFetch, OpImageRead and OpImageWrite.
    //
    // Where GLSL's built-in takes a sampler but SPIR-V expects an image, OpImage can be used to get
    // the SPIR-V image out of a SPIR-V sampled image.

    // The first parameter, which is either a sampled image or an image.  Some GLSL built-ins
    // receive a sampled image but their SPIR-V equivalent expects an image.  OpImage is used in
    // that case.
    spirv::IdRef image                = parameters[0];
    bool extractImageFromSampledImage = false;

    // The argument index for different possible parameters.  0 indicates that the argument is
    // unused.  Coordinates are usually at index 1, so it's pre-initialized.
    size_t coordinatesIndex     = 1;
    size_t biasIndex            = 0;
    size_t lodIndex             = 0;
    size_t compareIndex         = 0;
    size_t dPdxIndex            = 0;
    size_t dPdyIndex            = 0;
    size_t offsetIndex          = 0;
    size_t offsetsIndex         = 0;
    size_t gatherComponentIndex = 0;
    size_t sampleIndex          = 0;
    size_t dataIndex            = 0;

    // Whether this is a Dref variant of a sample call.
    bool isDref = IsShadowSampler(samplerBasicType);
    // Whether this is a Proj variant of a sample call.
    bool isProj = false;

    // The SPIR-V op used to implement the built-in.  For OpImageSample* instructions,
    // OpImageSampleImplicitLod is initially specified, which is later corrected based on |isDref|
    // and |isProj|.
    spv::Op spirvOp = BuiltInGroup::IsTexture(op) ? spv::OpImageSampleImplicitLod : spv::OpNop;

    // Organize the parameters and decide the SPIR-V Op to use.
    switch (op)
    {
        case EOpTexture2D:
        case EOpTextureCube:
        case EOpTexture3D:
        case EOpShadow2DEXT:
        case EOpTexture2DRect:
        case EOpTextureVideoWEBGL:
        case EOpTexture:

        case EOpTexture2DBias:
        case EOpTextureCubeBias:
        case EOpTexture3DBias:
        case EOpTextureBias:

            // For shadow cube arrays, the compare value is specified through an additional
            // parameter, while for the rest is taken out of the coordinates.
            if (function->getParamCount() == 3)
            {
                if (samplerBasicType == EbtSamplerCubeArrayShadow)
                {
                    compareIndex = 2;
                }
                else
                {
                    biasIndex = 2;
                }
            }
            else if (function->getParamCount() == 4 &&
                     samplerBasicType == EbtSamplerCubeArrayShadow)
            {
                compareIndex = 2;
                biasIndex    = 3;
            }
            break;

        case EOpTexture2DProj:
        case EOpTexture3DProj:
        case EOpShadow2DProjEXT:
        case EOpTexture2DRectProj:
        case EOpTextureProj:

        case EOpTexture2DProjBias:
        case EOpTexture3DProjBias:
        case EOpTextureProjBias:

            isProj = true;
            if (function->getParamCount() == 3)
            {
                biasIndex = 2;
            }
            break;

        case EOpTexture3DLod:

        case EOpTexture2DLodVS:
        case EOpTextureCubeLodVS:

        case EOpTexture2DLodEXTFS:
        case EOpTextureCubeLodEXTFS:
        case EOpTextureLod:

            if (samplerBasicType == EbtSamplerCubeArrayShadow)
            {
                ASSERT(function->getParamCount() == 4);
                compareIndex = 2;
                lodIndex     = 3;
            }
            else
            {
                ASSERT(function->getParamCount() == 3);
                lodIndex = 2;
            }
            break;

        case EOpTexture3DProjLod:

        case EOpTexture2DProjLodVS:

        case EOpTexture2DProjLodEXTFS:
        case EOpTextureProjLod:

            ASSERT(function->getParamCount() == 3);
            isProj   = true;
            lodIndex = 2;
            break;

        case EOpTexelFetch:
        case EOpTexelFetchOffset:
            // texelFetch has the following forms:
            //
            // - texelFetch(sampler, P);
            // - texelFetch(sampler, P, lod);
            // - texelFetch(samplerMS, P, sample);
            //
            // texelFetchOffset has an additional offset parameter at the end.
            //
            // In SPIR-V, OpImageFetch is used which operates on the image itself.
            spirvOp                      = spv::OpImageFetch;
            extractImageFromSampledImage = true;

            if (IsSamplerMS(samplerBasicType))
            {
                ASSERT(function->getParamCount() == 3);
                sampleIndex = 2;
            }
            else if (function->getParamCount() >= 3)
            {
                lodIndex = 2;
            }
            if (op == EOpTexelFetchOffset)
            {
                offsetIndex = function->getParamCount() - 1;
            }
            break;

        case EOpTexture2DGradEXT:
        case EOpTextureCubeGradEXT:
        case EOpTextureGrad:

            ASSERT(function->getParamCount() == 4);
            dPdxIndex = 2;
            dPdyIndex = 3;
            break;

        case EOpTexture2DProjGradEXT:
        case EOpTextureProjGrad:

            ASSERT(function->getParamCount() == 4);
            isProj    = true;
            dPdxIndex = 2;
            dPdyIndex = 3;
            break;

        case EOpTextureOffset:
        case EOpTextureOffsetBias:

            ASSERT(function->getParamCount() >= 3);
            offsetIndex = 2;
            if (function->getParamCount() == 4)
            {
                biasIndex = 3;
            }
            break;

        case EOpTextureProjOffset:
        case EOpTextureProjOffsetBias:

            ASSERT(function->getParamCount() >= 3);
            isProj      = true;
            offsetIndex = 2;
            if (function->getParamCount() == 4)
            {
                biasIndex = 3;
            }
            break;

        case EOpTextureLodOffset:

            ASSERT(function->getParamCount() == 4);
            lodIndex    = 2;
            offsetIndex = 3;
            break;

        case EOpTextureProjLodOffset:

            ASSERT(function->getParamCount() == 4);
            isProj      = true;
            lodIndex    = 2;
            offsetIndex = 3;
            break;

        case EOpTextureGradOffset:

            ASSERT(function->getParamCount() == 5);
            dPdxIndex   = 2;
            dPdyIndex   = 3;
            offsetIndex = 4;
            break;

        case EOpTextureProjGradOffset:

            ASSERT(function->getParamCount() == 5);
            isProj      = true;
            dPdxIndex   = 2;
            dPdyIndex   = 3;
            offsetIndex = 4;
            break;

        case EOpTextureGather:

            // For shadow textures, refZ (same as Dref) is specified as the last argument.
            // Otherwise a component may be specified which defaults to 0 if not specified.
            spirvOp = spv::OpImageGather;
            if (isDref)
            {
                ASSERT(function->getParamCount() == 3);
                compareIndex = 2;
            }
            else if (function->getParamCount() == 3)
            {
                gatherComponentIndex = 2;
            }
            break;

        case EOpTextureGatherOffset:
        case EOpTextureGatherOffsetComp:

        case EOpTextureGatherOffsets:
        case EOpTextureGatherOffsetsComp:

            // textureGatherOffset and textureGatherOffsets have the following forms:
            //
            // - texelGatherOffset*(sampler, P, offset*);
            // - texelGatherOffset*(sampler, P, offset*, component);
            // - texelGatherOffset*(sampler, P, refZ, offset*);
            //
            spirvOp = spv::OpImageGather;
            if (isDref)
            {
                ASSERT(function->getParamCount() == 4);
                compareIndex = 2;
            }
            else if (function->getParamCount() == 4)
            {
                gatherComponentIndex = 3;
            }

            ASSERT(function->getParamCount() >= 3);
            if (BuiltInGroup::IsTextureGatherOffset(op))
            {
                offsetIndex = isDref ? 3 : 2;
            }
            else
            {
                offsetsIndex = isDref ? 3 : 2;
            }
            break;

        case EOpImageStore:
            // imageStore has the following forms:
            //
            // - imageStore(image, P, data);
            // - imageStore(imageMS, P, sample, data);
            //
            spirvOp = spv::OpImageWrite;
            if (IsSamplerMS(samplerBasicType))
            {
                ASSERT(function->getParamCount() == 4);
                sampleIndex = 2;
                dataIndex   = 3;
            }
            else
            {
                ASSERT(function->getParamCount() == 3);
                dataIndex = 2;
            }
            break;

        case EOpImageLoad:
            // imageStore has the following forms:
            //
            // - imageLoad(image, P);
            // - imageLoad(imageMS, P, sample);
            //
            spirvOp = spv::OpImageRead;
            if (IsSamplerMS(samplerBasicType))
            {
                ASSERT(function->getParamCount() == 3);
                sampleIndex = 2;
            }
            else
            {
                ASSERT(function->getParamCount() == 2);
            }
            break;

            // Queries:
        case EOpTextureSize:
        case EOpImageSize:
            // textureSize has the following forms:
            //
            // - textureSize(sampler);
            // - textureSize(sampler, lod);
            //
            // while imageSize has only one form:
            //
            // - imageSize(image);
            //
            extractImageFromSampledImage = true;
            if (function->getParamCount() == 2)
            {
                spirvOp  = spv::OpImageQuerySizeLod;
                lodIndex = 1;
            }
            else
            {
                spirvOp = spv::OpImageQuerySize;
            }
            // No coordinates parameter.
            coordinatesIndex = 0;
            // No dref parameter.
            isDref = false;
            break;

        default:
            UNREACHABLE();
    }

    // If an implicit-lod instruction is used outside a fragment shader, change that to an explicit
    // one as they are not allowed in SPIR-V outside fragment shaders.
    const bool noLodSupport = IsSamplerBuffer(samplerBasicType) ||
                              IsImageBuffer(samplerBasicType) || IsSamplerMS(samplerBasicType) ||
                              IsImageMS(samplerBasicType);
    const bool makeLodExplicit =
        mCompiler->getShaderType() != GL_FRAGMENT_SHADER && lodIndex == 0 && dPdxIndex == 0 &&
        !noLodSupport && (spirvOp == spv::OpImageSampleImplicitLod || spirvOp == spv::OpImageFetch);

    // Apply any necessary fix up.

    if (extractImageFromSampledImage && IsSampler(samplerBasicType))
    {
        // Get the (non-sampled) image type.
        SpirvType imageType = mBuilder.getSpirvType(samplerType, {});
        ASSERT(!imageType.isSamplerBaseImage);
        imageType.isSamplerBaseImage            = true;
        const spirv::IdRef extractedImageTypeId = mBuilder.getSpirvTypeData(imageType, nullptr).id;

        // Use OpImage to get the image out of the sampled image.
        const spirv::IdRef extractedImage = mBuilder.getNewId({});
        spirv::WriteImage(mBuilder.getSpirvCurrentFunctionBlock(), extractedImageTypeId,
                          extractedImage, image);
        image = extractedImage;
    }

    // Gather operands as necessary.

    // - Coordinates
    uint8_t coordinatesChannelCount = 0;
    spirv::IdRef coordinatesId;
    const TType *coordinatesType = nullptr;
    if (coordinatesIndex > 0)
    {
        coordinatesId           = parameters[coordinatesIndex];
        coordinatesType         = &node->getChildNode(coordinatesIndex)->getAsTyped()->getType();
        coordinatesChannelCount = coordinatesType->getNominalSize();
    }

    // - Dref; either specified as a compare/refz argument (cube array, gather), or:
    //   * coordinates.z for proj variants
    //   * coordinates.<last> for others
    spirv::IdRef drefId;
    if (compareIndex > 0)
    {
        drefId = parameters[compareIndex];
    }
    else if (isDref)
    {
        // Get the component index
        ASSERT(coordinatesChannelCount > 0);
        uint8_t drefComponent = isProj ? 2 : coordinatesChannelCount - 1;

        // Get the component type
        SpirvType drefSpirvType       = mBuilder.getSpirvType(*coordinatesType, {});
        drefSpirvType.primarySize     = 1;
        const spirv::IdRef drefTypeId = mBuilder.getSpirvTypeData(drefSpirvType, nullptr).id;

        // Extract the dref component out of coordinates.
        drefId = mBuilder.getNewId(mBuilder.getDecorations(*coordinatesType));
        spirv::WriteCompositeExtract(mBuilder.getSpirvCurrentFunctionBlock(), drefTypeId, drefId,
                                     coordinatesId, {spirv::LiteralInteger(drefComponent)});
    }

    // - Gather component
    spirv::IdRef gatherComponentId;
    if (gatherComponentIndex > 0)
    {
        gatherComponentId = parameters[gatherComponentIndex];
    }
    else if (spirvOp == spv::OpImageGather)
    {
        // If comp is not specified, component 0 is taken as default.
        gatherComponentId = mBuilder.getIntConstant(0);
    }

    // - Image write data
    spirv::IdRef dataId;
    if (dataIndex > 0)
    {
        dataId = parameters[dataIndex];
    }

    // - Other operands
    spv::ImageOperandsMask operandsMask = spv::ImageOperandsMaskNone;
    spirv::IdRefList imageOperandsList;

    if (biasIndex > 0)
    {
        operandsMask = operandsMask | spv::ImageOperandsBiasMask;
        imageOperandsList.push_back(parameters[biasIndex]);
    }
    if (lodIndex > 0)
    {
        operandsMask = operandsMask | spv::ImageOperandsLodMask;
        imageOperandsList.push_back(parameters[lodIndex]);
    }
    else if (makeLodExplicit)
    {
        // If the implicit-lod variant is used outside fragment shaders, switch to explicit and use
        // lod 0.
        operandsMask = operandsMask | spv::ImageOperandsLodMask;
        imageOperandsList.push_back(spirvOp == spv::OpImageFetch ? mBuilder.getUintConstant(0)
                                                                 : mBuilder.getFloatConstant(0));
    }
    if (dPdxIndex > 0)
    {
        ASSERT(dPdyIndex > 0);
        operandsMask = operandsMask | spv::ImageOperandsGradMask;
        imageOperandsList.push_back(parameters[dPdxIndex]);
        imageOperandsList.push_back(parameters[dPdyIndex]);
    }
    if (offsetIndex > 0)
    {
        // Non-const offsets require the ImageGatherExtended feature.
        if (node->getChildNode(offsetIndex)->getAsTyped()->hasConstantValue())
        {
            operandsMask = operandsMask | spv::ImageOperandsConstOffsetMask;
        }
        else
        {
            ASSERT(spirvOp == spv::OpImageGather);

            operandsMask = operandsMask | spv::ImageOperandsOffsetMask;
            mBuilder.addCapability(spv::CapabilityImageGatherExtended);
        }
        imageOperandsList.push_back(parameters[offsetIndex]);
    }
    if (offsetsIndex > 0)
    {
        ASSERT(node->getChildNode(offsetsIndex)->getAsTyped()->hasConstantValue());

        operandsMask = operandsMask | spv::ImageOperandsConstOffsetsMask;
        mBuilder.addCapability(spv::CapabilityImageGatherExtended);
        imageOperandsList.push_back(parameters[offsetsIndex]);
    }
    if (sampleIndex > 0)
    {
        operandsMask = operandsMask | spv::ImageOperandsSampleMask;
        imageOperandsList.push_back(parameters[sampleIndex]);
    }

    const spv::ImageOperandsMask *imageOperands =
        imageOperandsList.empty() ? nullptr : &operandsMask;

    // GLSL and SPIR-V are different in the way the projective component is specified:
    //
    // In GLSL:
    //
    // > The texture coordinates consumed from P, not including the last component of P, are divided
    // > by the last component of P.
    //
    // In SPIR-V, there's a similar language (division by last element), but with the following
    // added:
    //
    // > ... all unused components will appear after all used components.
    //
    // So for example for textureProj(sampler, vec4 P), the projective coordinates are P.xy/P.w,
    // where P.z is ignored.  In SPIR-V instead that would be P.xy/P.z and P.w is ignored.
    //
    if (isProj)
    {
        uint8_t requiredChannelCount = coordinatesChannelCount;
        // texture*Proj* operate on the following parameters:
        //
        // - sampler2D, vec3 P
        // - sampler2D, vec4 P
        // - sampler2DRect, vec3 P
        // - sampler2DRect, vec4 P
        // - sampler3D, vec4 P
        // - sampler2DShadow, vec4 P
        // - sampler2DRectShadow, vec4 P
        //
        // Of these cases, only (sampler2D*, vec4 P) requires moving the proj channel from .w to the
        // appropriate location (.y for 1D and .z for 2D).
        if (IsSampler2D(samplerBasicType))
        {
            requiredChannelCount = 3;
        }
        if (requiredChannelCount != coordinatesChannelCount)
        {
            ASSERT(coordinatesChannelCount == 4);

            // Get the component type
            SpirvType spirvType                  = mBuilder.getSpirvType(*coordinatesType, {});
            const spirv::IdRef coordinatesTypeId = mBuilder.getSpirvTypeData(spirvType, nullptr).id;
            spirvType.primarySize                = 1;
            const spirv::IdRef channelTypeId     = mBuilder.getSpirvTypeData(spirvType, nullptr).id;

            // Extract the last component out of coordinates.
            const spirv::IdRef projChannelId =
                mBuilder.getNewId(mBuilder.getDecorations(*coordinatesType));
            spirv::WriteCompositeExtract(mBuilder.getSpirvCurrentFunctionBlock(), channelTypeId,
                                         projChannelId, coordinatesId,
                                         {spirv::LiteralInteger(coordinatesChannelCount - 1)});

            // Insert it after the channels that are consumed.  The extra channels are ignored per
            // the SPIR-V spec.
            const spirv::IdRef newCoordinatesId =
                mBuilder.getNewId(mBuilder.getDecorations(*coordinatesType));
            spirv::WriteCompositeInsert(mBuilder.getSpirvCurrentFunctionBlock(), coordinatesTypeId,
                                        newCoordinatesId, projChannelId, coordinatesId,
                                        {spirv::LiteralInteger(requiredChannelCount - 1)});
            coordinatesId = newCoordinatesId;
        }
    }

    // Select the correct sample Op based on whether the Proj, Dref or Explicit variants are used.
    if (spirvOp == spv::OpImageSampleImplicitLod)
    {
        ASSERT(!noLodSupport);
        const bool isExplicitLod = lodIndex != 0 || makeLodExplicit || dPdxIndex != 0;
        if (isDref)
        {
            if (isProj)
            {
                spirvOp = isExplicitLod ? spv::OpImageSampleProjDrefExplicitLod
                                        : spv::OpImageSampleProjDrefImplicitLod;
            }
            else
            {
                spirvOp = isExplicitLod ? spv::OpImageSampleDrefExplicitLod
                                        : spv::OpImageSampleDrefImplicitLod;
            }
        }
        else
        {
            if (isProj)
            {
                spirvOp = isExplicitLod ? spv::OpImageSampleProjExplicitLod
                                        : spv::OpImageSampleProjImplicitLod;
            }
            else
            {
                spirvOp =
                    isExplicitLod ? spv::OpImageSampleExplicitLod : spv::OpImageSampleImplicitLod;
            }
        }
    }
    if (spirvOp == spv::OpImageGather && isDref)
    {
        spirvOp = spv::OpImageDrefGather;
    }

    spirv::IdRef result;
    if (spirvOp != spv::OpImageWrite)
    {
        result = mBuilder.getNewId(mBuilder.getDecorations(node->getType()));
    }

    switch (spirvOp)
    {
        case spv::OpImageQuerySizeLod:
            mBuilder.addCapability(spv::CapabilityImageQuery);
            ASSERT(imageOperandsList.size() == 1);
            spirv::WriteImageQuerySizeLod(mBuilder.getSpirvCurrentFunctionBlock(), resultTypeId,
                                          result, image, imageOperandsList[0]);
            break;
        case spv::OpImageQuerySize:
            mBuilder.addCapability(spv::CapabilityImageQuery);
            spirv::WriteImageQuerySize(mBuilder.getSpirvCurrentFunctionBlock(), resultTypeId,
                                       result, image);
            break;
        case spv::OpImageQueryLod:
            mBuilder.addCapability(spv::CapabilityImageQuery);
            spirv::WriteImageQueryLod(mBuilder.getSpirvCurrentFunctionBlock(), resultTypeId, result,
                                      image, coordinatesId);
            break;
        case spv::OpImageQueryLevels:
            mBuilder.addCapability(spv::CapabilityImageQuery);
            spirv::WriteImageQueryLevels(mBuilder.getSpirvCurrentFunctionBlock(), resultTypeId,
                                         result, image);
            break;
        case spv::OpImageQuerySamples:
            mBuilder.addCapability(spv::CapabilityImageQuery);
            spirv::WriteImageQuerySamples(mBuilder.getSpirvCurrentFunctionBlock(), resultTypeId,
                                          result, image);
            break;
        case spv::OpImageSampleImplicitLod:
            spirv::WriteImageSampleImplicitLod(mBuilder.getSpirvCurrentFunctionBlock(),
                                               resultTypeId, result, image, coordinatesId,
                                               imageOperands, imageOperandsList);
            break;
        case spv::OpImageSampleExplicitLod:
            spirv::WriteImageSampleExplicitLod(mBuilder.getSpirvCurrentFunctionBlock(),
                                               resultTypeId, result, image, coordinatesId,
                                               *imageOperands, imageOperandsList);
            break;
        case spv::OpImageSampleDrefImplicitLod:
            spirv::WriteImageSampleDrefImplicitLod(mBuilder.getSpirvCurrentFunctionBlock(),
                                                   resultTypeId, result, image, coordinatesId,
                                                   drefId, imageOperands, imageOperandsList);
            break;
        case spv::OpImageSampleDrefExplicitLod:
            spirv::WriteImageSampleDrefExplicitLod(mBuilder.getSpirvCurrentFunctionBlock(),
                                                   resultTypeId, result, image, coordinatesId,
                                                   drefId, *imageOperands, imageOperandsList);
            break;
        case spv::OpImageSampleProjImplicitLod:
            spirv::WriteImageSampleProjImplicitLod(mBuilder.getSpirvCurrentFunctionBlock(),
                                                   resultTypeId, result, image, coordinatesId,
                                                   imageOperands, imageOperandsList);
            break;
        case spv::OpImageSampleProjExplicitLod:
            spirv::WriteImageSampleProjExplicitLod(mBuilder.getSpirvCurrentFunctionBlock(),
                                                   resultTypeId, result, image, coordinatesId,
                                                   *imageOperands, imageOperandsList);
            break;
        case spv::OpImageSampleProjDrefImplicitLod:
            spirv::WriteImageSampleProjDrefImplicitLod(mBuilder.getSpirvCurrentFunctionBlock(),
                                                       resultTypeId, result, image, coordinatesId,
                                                       drefId, imageOperands, imageOperandsList);
            break;
        case spv::OpImageSampleProjDrefExplicitLod:
            spirv::WriteImageSampleProjDrefExplicitLod(mBuilder.getSpirvCurrentFunctionBlock(),
                                                       resultTypeId, result, image, coordinatesId,
                                                       drefId, *imageOperands, imageOperandsList);
            break;
        case spv::OpImageFetch:
            spirv::WriteImageFetch(mBuilder.getSpirvCurrentFunctionBlock(), resultTypeId, result,
                                   image, coordinatesId, imageOperands, imageOperandsList);
            break;
        case spv::OpImageGather:
            spirv::WriteImageGather(mBuilder.getSpirvCurrentFunctionBlock(), resultTypeId, result,
                                    image, coordinatesId, gatherComponentId, imageOperands,
                                    imageOperandsList);
            break;
        case spv::OpImageDrefGather:
            spirv::WriteImageDrefGather(mBuilder.getSpirvCurrentFunctionBlock(), resultTypeId,
                                        result, image, coordinatesId, drefId, imageOperands,
                                        imageOperandsList);
            break;
        case spv::OpImageRead:
            spirv::WriteImageRead(mBuilder.getSpirvCurrentFunctionBlock(), resultTypeId, result,
                                  image, coordinatesId, imageOperands, imageOperandsList);
            break;
        case spv::OpImageWrite:
            spirv::WriteImageWrite(mBuilder.getSpirvCurrentFunctionBlock(), image, coordinatesId,
                                   dataId, imageOperands, imageOperandsList);
            break;
        default:
            UNREACHABLE();
    }

    return result;
}

spirv::IdRef OutputSPIRVTraverser::createSubpassLoadBuiltIn(TIntermOperator *node,
                                                            spirv::IdRef resultTypeId)
{
    // Load the parameters.
    spirv::IdRefList parameters = loadAllParams(node, 0, nullptr);
    const spirv::IdRef image    = parameters[0];

    // If multisampled, an additional parameter specifies the sample.  This is passed through as an
    // extra image operand.
    const bool hasSampleParam = parameters.size() == 2;
    const spv::ImageOperandsMask operandsMask =
        hasSampleParam ? spv::ImageOperandsSampleMask : spv::ImageOperandsMaskNone;
    spirv::IdRefList imageOperandsList;
    if (hasSampleParam)
    {
        imageOperandsList.push_back(parameters[1]);
    }

    // |subpassLoad| is implemented with OpImageRead.  This OP takes a coordinate, which is unused
    // and is set to (0, 0) here.
    const spirv::IdRef coordId = mBuilder.getNullConstant(mBuilder.getBasicTypeId(EbtUInt, 2));

    const spirv::IdRef result = mBuilder.getNewId(mBuilder.getDecorations(node->getType()));
    spirv::WriteImageRead(mBuilder.getSpirvCurrentFunctionBlock(), resultTypeId, result, image,
                          coordId, hasSampleParam ? &operandsMask : nullptr, imageOperandsList);

    return result;
}

spirv::IdRef OutputSPIRVTraverser::createInterpolate(TIntermOperator *node,
                                                     spirv::IdRef resultTypeId)
{
    spv::GLSLstd450 extendedInst = spv::GLSLstd450Bad;

    mBuilder.addCapability(spv::CapabilityInterpolationFunction);

    switch (node->getOp())
    {
        case EOpInterpolateAtCentroid:
            extendedInst = spv::GLSLstd450InterpolateAtCentroid;
            break;
        case EOpInterpolateAtSample:
            extendedInst = spv::GLSLstd450InterpolateAtSample;
            break;
        case EOpInterpolateAtOffset:
            extendedInst = spv::GLSLstd450InterpolateAtOffset;
            break;
        default:
            UNREACHABLE();
    }

    size_t childCount = node->getChildCount();

    spirv::IdRefList parameters;

    // interpolateAt* takes the interpolant as the first argument, *pointer* to which needs to be
    // passed to the instruction.  Except interpolateAtCentroid, another parameter follows.
    parameters.push_back(accessChainCollapse(&mNodeData[mNodeData.size() - childCount]));
    if (childCount > 1)
    {
        parameters.push_back(accessChainLoad(
            &mNodeData.back(), node->getChildNode(1)->getAsTyped()->getType(), nullptr));
    }

    const spirv::IdRef result = mBuilder.getNewId(mBuilder.getDecorations(node->getType()));

    spirv::WriteExtInst(mBuilder.getSpirvCurrentFunctionBlock(), resultTypeId, result,
                        mBuilder.getExtInstImportIdStd(),
                        spirv::LiteralExtInstInteger(extendedInst), parameters);

    return result;
}

spirv::IdRef OutputSPIRVTraverser::castBasicType(spirv::IdRef value,
                                                 const TType &valueType,
                                                 const TType &expectedType,
                                                 spirv::IdRef *resultTypeIdOut)
{
    const TBasicType expectedBasicType = expectedType.getBasicType();
    if (valueType.getBasicType() == expectedBasicType)
    {
        return value;
    }

    // Make sure no attempt is made to cast a matrix to int/uint.
    ASSERT(!valueType.isMatrix() || expectedBasicType == EbtFloat);

    SpirvType valueSpirvType                            = mBuilder.getSpirvType(valueType, {});
    valueSpirvType.type                                 = expectedBasicType;
    valueSpirvType.typeSpec.isOrHasBoolInInterfaceBlock = false;
    const spirv::IdRef castTypeId = mBuilder.getSpirvTypeData(valueSpirvType, nullptr).id;

    const spirv::IdRef castValue = mBuilder.getNewId(mBuilder.getDecorations(expectedType));

    // Write the instruction that casts between types.  Different instructions are used based on the
    // types being converted.
    //
    // - int/uint <-> float: OpConvert*To*
    // - int <-> uint: OpBitcast
    // - bool --> int/uint/float: OpSelect with 0 and 1
    // - int/uint --> bool: OPINotEqual 0
    // - float --> bool: OpFUnordNotEqual 0

    WriteUnaryOp writeUnaryOp     = nullptr;
    WriteBinaryOp writeBinaryOp   = nullptr;
    WriteTernaryOp writeTernaryOp = nullptr;

    spirv::IdRef zero;
    spirv::IdRef one;

    switch (valueType.getBasicType())
    {
        case EbtFloat:
            switch (expectedBasicType)
            {
                case EbtInt:
                    writeUnaryOp = spirv::WriteConvertFToS;
                    break;
                case EbtUInt:
                    writeUnaryOp = spirv::WriteConvertFToU;
                    break;
                case EbtBool:
                    zero          = mBuilder.getVecConstant(0, valueType.getNominalSize());
                    writeBinaryOp = spirv::WriteFUnordNotEqual;
                    break;
                default:
                    UNREACHABLE();
            }
            break;

        case EbtInt:
        case EbtUInt:
            switch (expectedBasicType)
            {
                case EbtFloat:
                    writeUnaryOp = valueType.getBasicType() == EbtInt ? spirv::WriteConvertSToF
                                                                      : spirv::WriteConvertUToF;
                    break;
                case EbtInt:
                case EbtUInt:
                    writeUnaryOp = spirv::WriteBitcast;
                    break;
                case EbtBool:
                    zero          = mBuilder.getUvecConstant(0, valueType.getNominalSize());
                    writeBinaryOp = spirv::WriteINotEqual;
                    break;
                default:
                    UNREACHABLE();
            }
            break;

        case EbtBool:
            writeTernaryOp = spirv::WriteSelect;
            switch (expectedBasicType)
            {
                case EbtFloat:
                    zero = mBuilder.getVecConstant(0, valueType.getNominalSize());
                    one  = mBuilder.getVecConstant(1, valueType.getNominalSize());
                    break;
                case EbtInt:
                    zero = mBuilder.getIvecConstant(0, valueType.getNominalSize());
                    one  = mBuilder.getIvecConstant(1, valueType.getNominalSize());
                    break;
                case EbtUInt:
                    zero = mBuilder.getUvecConstant(0, valueType.getNominalSize());
                    one  = mBuilder.getUvecConstant(1, valueType.getNominalSize());
                    break;
                default:
                    UNREACHABLE();
            }
            break;

        default:
            UNREACHABLE();
    }

    if (writeUnaryOp)
    {
        writeUnaryOp(mBuilder.getSpirvCurrentFunctionBlock(), castTypeId, castValue, value);
    }
    else if (writeBinaryOp)
    {
        writeBinaryOp(mBuilder.getSpirvCurrentFunctionBlock(), castTypeId, castValue, value, zero);
    }
    else
    {
        ASSERT(writeTernaryOp);
        writeTernaryOp(mBuilder.getSpirvCurrentFunctionBlock(), castTypeId, castValue, value, one,
                       zero);
    }

    if (resultTypeIdOut)
    {
        *resultTypeIdOut = castTypeId;
    }

    return castValue;
}

spirv::IdRef OutputSPIRVTraverser::cast(spirv::IdRef value,
                                        const TType &valueType,
                                        const SpirvTypeSpec &valueTypeSpec,
                                        const SpirvTypeSpec &expectedTypeSpec,
                                        spirv::IdRef *resultTypeIdOut)
{
    // If there's no difference in type specialization, there's nothing to cast.
    if (valueTypeSpec.blockStorage == expectedTypeSpec.blockStorage &&
        valueTypeSpec.isInvariantBlock == expectedTypeSpec.isInvariantBlock &&
        valueTypeSpec.isRowMajorQualifiedBlock == expectedTypeSpec.isRowMajorQualifiedBlock &&
        valueTypeSpec.isRowMajorQualifiedArray == expectedTypeSpec.isRowMajorQualifiedArray &&
        valueTypeSpec.isOrHasBoolInInterfaceBlock == expectedTypeSpec.isOrHasBoolInInterfaceBlock &&
        valueTypeSpec.isPatchIOBlock == expectedTypeSpec.isPatchIOBlock)
    {
        return value;
    }

    // At this point, a value is loaded with the |valueType| GLSL type which is of a SPIR-V type
    // specialized by |valueTypeSpec|.  However, it's being assigned (for example through operator=,
    // used in a constructor or passed as a function argument) where the same GLSL type is expected
    // but with different SPIR-V type specialization (|expectedTypeSpec|).
    //
    // If SPIR-V 1.4 is available, use OpCopyLogical if possible.  OpCopyLogical works on arrays and
    // structs, and only if the types are logically the same.  This means that arrays and structs
    // can be copied with this instruction despite their SpirvTypeSpec being different.  The only
    // exception is if there is a mismatch in the isOrHasBoolInInterfaceBlock type specialization
    // as it actually changes the type of the struct members.
    if (mCompileOptions.emitSPIRV14 && (valueType.isArray() || valueType.getStruct() != nullptr) &&
        valueTypeSpec.isOrHasBoolInInterfaceBlock == expectedTypeSpec.isOrHasBoolInInterfaceBlock)
    {
        const spirv::IdRef expectedTypeId =
            mBuilder.getTypeDataOverrideTypeSpec(valueType, expectedTypeSpec).id;
        const spirv::IdRef expectedId = mBuilder.getNewId(mBuilder.getDecorations(valueType));

        spirv::WriteCopyLogical(mBuilder.getSpirvCurrentFunctionBlock(), expectedTypeId, expectedId,
                                value);
        if (resultTypeIdOut)
        {
            *resultTypeIdOut = expectedTypeId;
        }
        return expectedId;
    }

    // The following code recursively copies the array elements or struct fields and then constructs
    // the final result with the expected SPIR-V type.

    // Interface blocks cannot be copied or passed as parameters in GLSL.
    ASSERT(!valueType.isInterfaceBlock());

    spirv::IdRefList constituents;

    if (valueType.isArray())
    {
        // Find the SPIR-V type specialization for the element type.
        SpirvTypeSpec valueElementTypeSpec    = valueTypeSpec;
        SpirvTypeSpec expectedElementTypeSpec = expectedTypeSpec;

        const bool isElementBlock = valueType.getStruct() != nullptr;
        const bool isElementArray = valueType.isArrayOfArrays();

        valueElementTypeSpec.onArrayElementSelection(isElementBlock, isElementArray);
        expectedElementTypeSpec.onArrayElementSelection(isElementBlock, isElementArray);

        // Get the element type id.
        TType elementType(valueType);
        elementType.toArrayElementType();

        const spirv::IdRef elementTypeId =
            mBuilder.getTypeDataOverrideTypeSpec(elementType, valueElementTypeSpec).id;

        const SpirvDecorations elementDecorations = mBuilder.getDecorations(elementType);

        // Extract each element of the array and cast it to the expected type.
        for (unsigned int elementIndex = 0; elementIndex < valueType.getOutermostArraySize();
             ++elementIndex)
        {
            const spirv::IdRef elementId = mBuilder.getNewId(elementDecorations);
            spirv::WriteCompositeExtract(mBuilder.getSpirvCurrentFunctionBlock(), elementTypeId,
                                         elementId, value, {spirv::LiteralInteger(elementIndex)});

            constituents.push_back(cast(elementId, elementType, valueElementTypeSpec,
                                        expectedElementTypeSpec, nullptr));
        }
    }
    else if (valueType.getStruct() != nullptr)
    {
        uint32_t fieldIndex = 0;

        // Extract each field of the struct and cast it to the expected type.
        for (const TField *field : valueType.getStruct()->fields())
        {
            const TType &fieldType = *field->type();

            // Find the SPIR-V type specialization for the field type.
            SpirvTypeSpec valueFieldTypeSpec    = valueTypeSpec;
            SpirvTypeSpec expectedFieldTypeSpec = expectedTypeSpec;

            valueFieldTypeSpec.onBlockFieldSelection(fieldType);
            expectedFieldTypeSpec.onBlockFieldSelection(fieldType);

            // Get the field type id.
            const spirv::IdRef fieldTypeId =
                mBuilder.getTypeDataOverrideTypeSpec(fieldType, valueFieldTypeSpec).id;

            // Extract the field.
            const spirv::IdRef fieldId = mBuilder.getNewId(mBuilder.getDecorations(fieldType));
            spirv::WriteCompositeExtract(mBuilder.getSpirvCurrentFunctionBlock(), fieldTypeId,
                                         fieldId, value, {spirv::LiteralInteger(fieldIndex++)});

            constituents.push_back(
                cast(fieldId, fieldType, valueFieldTypeSpec, expectedFieldTypeSpec, nullptr));
        }
    }
    else
    {
        // Bool types in interface blocks are emulated with uint.  bool<->uint cast is done here.
        ASSERT(valueType.getBasicType() == EbtBool);
        ASSERT(valueTypeSpec.isOrHasBoolInInterfaceBlock ||
               expectedTypeSpec.isOrHasBoolInInterfaceBlock);

        TType emulatedValueType(valueType);
        emulatedValueType.setBasicType(EbtUInt);
        emulatedValueType.setPrecise(EbpLow);

        // If value is loaded as uint, it needs to change to bool.  If it's bool, it needs to change
        // to uint before storage.
        if (valueTypeSpec.isOrHasBoolInInterfaceBlock)
        {
            return castBasicType(value, emulatedValueType, valueType, resultTypeIdOut);
        }
        else
        {
            return castBasicType(value, valueType, emulatedValueType, resultTypeIdOut);
        }
    }

    // Construct the value with the expected type from its cast constituents.
    const spirv::IdRef expectedTypeId =
        mBuilder.getTypeDataOverrideTypeSpec(valueType, expectedTypeSpec).id;
    const spirv::IdRef expectedId = mBuilder.getNewId(mBuilder.getDecorations(valueType));

    spirv::WriteCompositeConstruct(mBuilder.getSpirvCurrentFunctionBlock(), expectedTypeId,
                                   expectedId, constituents);

    if (resultTypeIdOut)
    {
        *resultTypeIdOut = expectedTypeId;
    }

    return expectedId;
}

void OutputSPIRVTraverser::extendScalarParamsToVector(TIntermOperator *node,
                                                      spirv::IdRef resultTypeId,
                                                      spirv::IdRefList *parameters)
{
    const TType &type = node->getType();
    if (type.isScalar())
    {
        // Nothing to do if the operation is applied to scalars.
        return;
    }

    const size_t childCount = node->getChildCount();

    for (size_t childIndex = 0; childIndex < childCount; ++childIndex)
    {
        const TType &childType = node->getChildNode(childIndex)->getAsTyped()->getType();

        // If the child is a scalar, replicate it to form a vector of the right size.
        if (childType.isScalar())
        {
            TType vectorType(type);
            if (vectorType.isMatrix())
            {
                vectorType.toMatrixColumnType();
            }
            (*parameters)[childIndex] = createConstructorVectorFromScalar(
                childType, vectorType, resultTypeId, {{(*parameters)[childIndex]}});
        }
    }
}

spirv::IdRef OutputSPIRVTraverser::reduceBoolVector(TOperator op,
                                                    const spirv::IdRefList &valueIds,
                                                    spirv::IdRef typeId,
                                                    const SpirvDecorations &decorations)
{
    if (valueIds.size() == 2)
    {
        // If two values are given, and/or them directly.
        WriteBinaryOp writeBinaryOp =
            op == EOpEqual ? spirv::WriteLogicalAnd : spirv::WriteLogicalOr;
        const spirv::IdRef result = mBuilder.getNewId(decorations);

        writeBinaryOp(mBuilder.getSpirvCurrentFunctionBlock(), typeId, result, valueIds[0],
                      valueIds[1]);
        return result;
    }

    WriteUnaryOp writeUnaryOp = op == EOpEqual ? spirv::WriteAll : spirv::WriteAny;
    spirv::IdRef valueId      = valueIds[0];

    if (valueIds.size() > 2)
    {
        // If multiple values are given, construct a bool vector out of them first.
        const spirv::IdRef bvecTypeId = mBuilder.getBasicTypeId(EbtBool, valueIds.size());
        valueId                       = {mBuilder.getNewId(decorations)};

        spirv::WriteCompositeConstruct(mBuilder.getSpirvCurrentFunctionBlock(), bvecTypeId, valueId,
                                       valueIds);
    }

    const spirv::IdRef result = mBuilder.getNewId(decorations);
    writeUnaryOp(mBuilder.getSpirvCurrentFunctionBlock(), typeId, result, valueId);

    return result;
}

void OutputSPIRVTraverser::createCompareImpl(TOperator op,
                                             const TType &operandType,
                                             spirv::IdRef resultTypeId,
                                             spirv::IdRef leftId,
                                             spirv::IdRef rightId,
                                             const SpirvDecorations &operandDecorations,
                                             const SpirvDecorations &resultDecorations,
                                             spirv::LiteralIntegerList *currentAccessChain,
                                             spirv::IdRefList *intermediateResultsOut)
{
    const TBasicType basicType = operandType.getBasicType();
    const bool isFloat         = basicType == EbtFloat;
    const bool isBool          = basicType == EbtBool;

    WriteBinaryOp writeBinaryOp = nullptr;

    // For arrays, compare them element by element.
    if (operandType.isArray())
    {
        TType elementType(operandType);
        elementType.toArrayElementType();

        currentAccessChain->emplace_back();
        for (unsigned int elementIndex = 0; elementIndex < operandType.getOutermostArraySize();
             ++elementIndex)
        {
            // Select the current element.
            currentAccessChain->back() = spirv::LiteralInteger(elementIndex);

            // Compare and accumulate the results.
            createCompareImpl(op, elementType, resultTypeId, leftId, rightId, operandDecorations,
                              resultDecorations, currentAccessChain, intermediateResultsOut);
        }
        currentAccessChain->pop_back();

        return;
    }

    // For structs, compare them field by field.
    if (operandType.getStruct() != nullptr)
    {
        uint32_t fieldIndex = 0;

        currentAccessChain->emplace_back();
        for (const TField *field : operandType.getStruct()->fields())
        {
            // Select the current field.
            currentAccessChain->back() = spirv::LiteralInteger(fieldIndex++);

            // Compare and accumulate the results.
            createCompareImpl(op, *field->type(), resultTypeId, leftId, rightId, operandDecorations,
                              resultDecorations, currentAccessChain, intermediateResultsOut);
        }
        currentAccessChain->pop_back();

        return;
    }

    // For matrices, compare them column by column.
    if (operandType.isMatrix())
    {
        TType columnType(operandType);
        columnType.toMatrixColumnType();

        currentAccessChain->emplace_back();
        for (uint8_t columnIndex = 0; columnIndex < operandType.getCols(); ++columnIndex)
        {
            // Select the current column.
            currentAccessChain->back() = spirv::LiteralInteger(columnIndex);

            // Compare and accumulate the results.
            createCompareImpl(op, columnType, resultTypeId, leftId, rightId, operandDecorations,
                              resultDecorations, currentAccessChain, intermediateResultsOut);
        }
        currentAccessChain->pop_back();

        return;
    }

    // For scalars and vectors generate a single instruction for comparison.
    if (op == EOpEqual)
    {
        if (isFloat)
            writeBinaryOp = spirv::WriteFOrdEqual;
        else if (isBool)
            writeBinaryOp = spirv::WriteLogicalEqual;
        else
            writeBinaryOp = spirv::WriteIEqual;
    }
    else
    {
        ASSERT(op == EOpNotEqual);

        if (isFloat)
            writeBinaryOp = spirv::WriteFUnordNotEqual;
        else if (isBool)
            writeBinaryOp = spirv::WriteLogicalNotEqual;
        else
            writeBinaryOp = spirv::WriteINotEqual;
    }

    // Extract the scalar and vector from composite types, if any.
    spirv::IdRef leftComponentId  = leftId;
    spirv::IdRef rightComponentId = rightId;
    if (!currentAccessChain->empty())
    {
        leftComponentId  = mBuilder.getNewId(operandDecorations);
        rightComponentId = mBuilder.getNewId(operandDecorations);

        const spirv::IdRef componentTypeId =
            mBuilder.getBasicTypeId(operandType.getBasicType(), operandType.getNominalSize());

        spirv::WriteCompositeExtract(mBuilder.getSpirvCurrentFunctionBlock(), componentTypeId,
                                     leftComponentId, leftId, *currentAccessChain);
        spirv::WriteCompositeExtract(mBuilder.getSpirvCurrentFunctionBlock(), componentTypeId,
                                     rightComponentId, rightId, *currentAccessChain);
    }

    const bool reduceResult     = !operandType.isScalar();
    spirv::IdRef result         = mBuilder.getNewId({});
    spirv::IdRef opResultTypeId = resultTypeId;
    if (reduceResult)
    {
        opResultTypeId = mBuilder.getBasicTypeId(EbtBool, operandType.getNominalSize());
    }

    // Write the comparison operation itself.
    writeBinaryOp(mBuilder.getSpirvCurrentFunctionBlock(), opResultTypeId, result, leftComponentId,
                  rightComponentId);

    // If it's a vector, reduce the result.
    if (reduceResult)
    {
        result = reduceBoolVector(op, {result}, resultTypeId, resultDecorations);
    }

    intermediateResultsOut->push_back(result);
}

spirv::IdRef OutputSPIRVTraverser::makeBuiltInOutputStructType(TIntermOperator *node,
                                                               size_t lvalueCount)
{
    // The built-ins with lvalues are in one of the following forms:
    //
    // - lsb = builtin(..., out msb): These are identified by lvalueCount == 1
    // - builtin(..., out msb, out lsb): These are identified by lvalueCount == 2
    //
    // In SPIR-V, the result of all these instructions is a struct { lsb; msb; }.

    const size_t childCount = node->getChildCount();
    ASSERT(childCount >= 2);

    TIntermTyped *lastChild       = node->getChildNode(childCount - 1)->getAsTyped();
    TIntermTyped *beforeLastChild = node->getChildNode(childCount - 2)->getAsTyped();

    const TType &lsbType = lvalueCount == 1 ? node->getType() : lastChild->getType();
    const TType &msbType = lvalueCount == 1 ? lastChild->getType() : beforeLastChild->getType();

    ASSERT(lsbType.isScalar() || lsbType.isVector());
    ASSERT(msbType.isScalar() || msbType.isVector());

    const BuiltInResultStruct key = {
        lsbType.getBasicType(),
        msbType.getBasicType(),
        static_cast<uint32_t>(lsbType.getNominalSize()),
        static_cast<uint32_t>(msbType.getNominalSize()),
    };

    auto iter = mBuiltInResultStructMap.find(key);
    if (iter == mBuiltInResultStructMap.end())
    {
        // Create a TStructure and TType for the required structure.
        TType *lsbTypeCopy = new TType(lsbType.getBasicType(), lsbType.getNominalSize(), 1);
        TType *msbTypeCopy = new TType(msbType.getBasicType(), msbType.getNominalSize(), 1);

        TFieldList *fields = new TFieldList;
        fields->push_back(
            new TField(lsbTypeCopy, ImmutableString("lsb"), {}, SymbolType::AngleInternal));
        fields->push_back(
            new TField(msbTypeCopy, ImmutableString("msb"), {}, SymbolType::AngleInternal));

        TStructure *structure =
            new TStructure(&mCompiler->getSymbolTable(), ImmutableString("BuiltInResultType"),
                           fields, SymbolType::AngleInternal);

        TType structType(structure, true);

        // Get an id for the type and store in the hash map.
        const spirv::IdRef structTypeId = mBuilder.getTypeData(structType, {}).id;
        iter                            = mBuiltInResultStructMap.insert({key, structTypeId}).first;
    }

    return iter->second;
}

// Once the builtin instruction is generated, the two return values are extracted from the
// struct.  These are written to the return value (if any) and the out parameters.
void OutputSPIRVTraverser::storeBuiltInStructOutputInParamsAndReturnValue(
    TIntermOperator *node,
    size_t lvalueCount,
    spirv::IdRef structValue,
    spirv::IdRef returnValue,
    spirv::IdRef returnValueType)
{
    const size_t childCount = node->getChildCount();
    ASSERT(childCount >= 2);

    TIntermTyped *lastChild       = node->getChildNode(childCount - 1)->getAsTyped();
    TIntermTyped *beforeLastChild = node->getChildNode(childCount - 2)->getAsTyped();

    if (lvalueCount == 1)
    {
        // The built-in is the form:
        //
        //     lsb = builtin(..., out msb): These are identified by lvalueCount == 1

        // Field 0 is lsb, which is extracted as the builtin's return value.
        spirv::WriteCompositeExtract(mBuilder.getSpirvCurrentFunctionBlock(), returnValueType,
                                     returnValue, structValue, {spirv::LiteralInteger(0)});

        // Field 1 is msb, which is extracted and stored through the out parameter.
        storeBuiltInStructOutputInParamHelper(&mNodeData[mNodeData.size() - 1], lastChild,
                                              structValue, 1);
    }
    else
    {
        // The built-in is the form:
        //
        //     builtin(..., out msb, out lsb): These are identified by lvalueCount == 2
        ASSERT(lvalueCount == 2);

        // Field 0 is lsb, which is extracted and stored through the second out parameter.
        storeBuiltInStructOutputInParamHelper(&mNodeData[mNodeData.size() - 1], lastChild,
                                              structValue, 0);

        // Field 1 is msb, which is extracted and stored through the first out parameter.
        storeBuiltInStructOutputInParamHelper(&mNodeData[mNodeData.size() - 2], beforeLastChild,
                                              structValue, 1);
    }
}

void OutputSPIRVTraverser::storeBuiltInStructOutputInParamHelper(NodeData *data,
                                                                 TIntermTyped *param,
                                                                 spirv::IdRef structValue,
                                                                 uint32_t fieldIndex)
{
    spirv::IdRef fieldTypeId  = mBuilder.getTypeData(param->getType(), {}).id;
    spirv::IdRef fieldValueId = mBuilder.getNewId(mBuilder.getDecorations(param->getType()));

    spirv::WriteCompositeExtract(mBuilder.getSpirvCurrentFunctionBlock(), fieldTypeId, fieldValueId,
                                 structValue, {spirv::LiteralInteger(fieldIndex)});

    accessChainStore(data, fieldValueId, param->getType());
}

void OutputSPIRVTraverser::visitSymbol(TIntermSymbol *node)
{
    // No-op visits to symbols that are being declared.  They are handled in visitDeclaration.
    if (mIsSymbolBeingDeclared)
    {
        // Make sure this does not affect other symbols, for example in the initializer expression.
        mIsSymbolBeingDeclared = false;
        return;
    }

    mNodeData.emplace_back();

    // The symbol is either:
    //
    // - A specialization constant
    // - A variable (local, varying etc)
    // - An interface block
    // - A field of an unnamed interface block
    //
    // Specialization constants in SPIR-V are treated largely like constants, in which case make
    // this behave like visitConstantUnion().

    const TType &type                     = node->getType();
    const TInterfaceBlock *interfaceBlock = type.getInterfaceBlock();
    const TSymbol *symbol                 = interfaceBlock;
    if (interfaceBlock == nullptr)
    {
        symbol = &node->variable();
    }

    // Track the properties that lead to the symbol's specific SPIR-V type based on the GLSL type.
    // They are needed to determine the derived type in an access chain, but are not promoted in
    // intermediate nodes' TTypes.
    SpirvTypeSpec typeSpec;
    typeSpec.inferDefaults(type, mCompiler);

    const spirv::IdRef typeId = mBuilder.getTypeData(type, typeSpec).id;

    // If the symbol is a const variable, a const function parameter or specialization constant,
    // create an rvalue.
    if (type.getQualifier() == EvqConst || type.getQualifier() == EvqParamConst ||
        type.getQualifier() == EvqSpecConst)
    {
        ASSERT(interfaceBlock == nullptr);
        ASSERT(mSymbolIdMap.count(symbol) > 0);
        nodeDataInitRValue(&mNodeData.back(), mSymbolIdMap[symbol], typeId);
        return;
    }

    // Otherwise create an lvalue.
    spv::StorageClass storageClass;
    const spirv::IdRef symbolId = getSymbolIdAndStorageClass(symbol, type, &storageClass);

    nodeDataInitLValue(&mNodeData.back(), symbolId, typeId, storageClass, typeSpec);

    // If a field of a nameless interface block, create an access chain.
    if (type.getInterfaceBlock() && !type.isInterfaceBlock())
    {
        uint32_t fieldIndex = static_cast<uint32_t>(type.getInterfaceBlockFieldIndex());
        accessChainPushLiteral(&mNodeData.back(), spirv::LiteralInteger(fieldIndex), typeId);
    }

    // Add gl_PerVertex capabilities only if the field is actually used.
    switch (type.getQualifier())
    {
        case EvqClipDistance:
            mBuilder.addCapability(spv::CapabilityClipDistance);
            break;
        case EvqCullDistance:
            mBuilder.addCapability(spv::CapabilityCullDistance);
            break;
        default:
            break;
    }
}

void OutputSPIRVTraverser::visitConstantUnion(TIntermConstantUnion *node)
{
    mNodeData.emplace_back();

    const TType &type = node->getType();

    // Find out the expected type for this constant, so it can be cast right away and not need an
    // instruction to do that.
    TIntermNode *parent     = getParentNode();
    const size_t childIndex = getParentChildIndex(PreVisit);

    TBasicType expectedBasicType = type.getBasicType();
    if (parent->getAsAggregate())
    {
        TIntermAggregate *parentAggregate = parent->getAsAggregate();

        // Note that only constructors can cast a type.  There are two possibilities:
        //
        // - It's a struct constructor: The basic type must match that of the corresponding field of
        //   the struct.
        // - It's a non struct constructor: The basic type must match that of the type being
        //   constructed.
        if (parentAggregate->isConstructor())
        {
            const TType &parentType     = parentAggregate->getType();
            const TStructure *structure = parentType.getStruct();

            if (structure != nullptr && !parentType.isArray())
            {
                expectedBasicType = structure->fields()[childIndex]->type()->getBasicType();
            }
            else
            {
                expectedBasicType = parentAggregate->getType().getBasicType();
            }
        }
    }

    const spirv::IdRef typeId  = mBuilder.getTypeData(type, {}).id;
    const spirv::IdRef constId = createConstant(type, expectedBasicType, node->getConstantValue(),
                                                node->isConstantNullValue());

    nodeDataInitRValue(&mNodeData.back(), constId, typeId);
}

bool OutputSPIRVTraverser::visitSwizzle(Visit visit, TIntermSwizzle *node)
{
    // Constants are expected to be folded.
    ASSERT(!node->hasConstantValue());

    if (visit == PreVisit)
    {
        // Don't add an entry to the stack.  The child will create one, which we won't pop.
        return true;
    }

    ASSERT(visit == PostVisit);
    ASSERT(mNodeData.size() >= 1);

    const TType &vectorType            = node->getOperand()->getType();
    const uint8_t vectorComponentCount = static_cast<uint8_t>(vectorType.getNominalSize());
    const TVector<int> &swizzle        = node->getSwizzleOffsets();

    // As an optimization, do nothing if the swizzle is selecting all the components of the vector
    // in order.
    bool isIdentity = swizzle.size() == vectorComponentCount;
    for (size_t index = 0; index < swizzle.size(); ++index)
    {
        isIdentity = isIdentity && static_cast<size_t>(swizzle[index]) == index;
    }

    if (isIdentity)
    {
        return true;
    }

    accessChainOnPush(&mNodeData.back(), vectorType, 0);

    const spirv::IdRef typeId =
        mBuilder.getTypeData(node->getType(), mNodeData.back().accessChain.typeSpec).id;

    accessChainPushSwizzle(&mNodeData.back(), swizzle, typeId, vectorComponentCount);

    return true;
}

bool OutputSPIRVTraverser::visitBinary(Visit visit, TIntermBinary *node)
{
    // Constants are expected to be folded.
    ASSERT(!node->hasConstantValue());

    if (visit == PreVisit)
    {
        // Don't add an entry to the stack.  The left child will create one, which we won't pop.
        return true;
    }

    // If this is a variable initialization node, defer any code generation to visitDeclaration.
    if (node->getOp() == EOpInitialize)
    {
        ASSERT(getParentNode()->getAsDeclarationNode() != nullptr);
        return true;
    }

    if (IsShortCircuitNeeded(node))
    {
        // For && and ||, if short-circuiting behavior is needed, we need to emulate it with an
        // |if| construct.  At this point, the left-hand side is already evaluated, so we need to
        // create an appropriate conditional on in-visit and visit the right-hand-side inside the
        // conditional block.  On post-visit, OpPhi is used to calculate the result.
        if (visit == InVisit)
        {
            startShortCircuit(node);
            return true;
        }

        spirv::IdRef typeId;
        const spirv::IdRef result = endShortCircuit(node, &typeId);

        // Replace the access chain with an rvalue that's the result.
        nodeDataInitRValue(&mNodeData.back(), result, typeId);

        return true;
    }

    if (visit == InVisit)
    {
        // Left child visited.  Take the entry it created as the current node's.
        ASSERT(mNodeData.size() >= 1);

        // As an optimization, if the index is EOpIndexDirect*, take the constant index directly and
        // add it to the access chain as literal.
        switch (node->getOp())
        {
            default:
                break;

            case EOpIndexDirect:
            case EOpIndexDirectStruct:
            case EOpIndexDirectInterfaceBlock:
                const uint32_t index = node->getRight()->getAsConstantUnion()->getIConst(0);
                accessChainOnPush(&mNodeData.back(), node->getLeft()->getType(), index);

                const spirv::IdRef typeId =
                    mBuilder.getTypeData(node->getType(), mNodeData.back().accessChain.typeSpec).id;
                accessChainPushLiteral(&mNodeData.back(), spirv::LiteralInteger(index), typeId);

                // Don't visit the right child, it's already processed.
                return false;
        }

        return true;
    }

    // There are at least two entries, one for the left node and one for the right one.
    ASSERT(mNodeData.size() >= 2);

    SpirvTypeSpec resultTypeSpec;
    if (node->getOp() == EOpIndexIndirect || node->getOp() == EOpAssign)
    {
        if (node->getOp() == EOpIndexIndirect)
        {
            accessChainOnPush(&mNodeData[mNodeData.size() - 2], node->getLeft()->getType(), 0);
        }
        resultTypeSpec = mNodeData[mNodeData.size() - 2].accessChain.typeSpec;
    }
    const spirv::IdRef resultTypeId = mBuilder.getTypeData(node->getType(), resultTypeSpec).id;

    // For EOpIndex* operations, push the right value as an index to the left value's access chain.
    // For the other operations, evaluate the expression.
    switch (node->getOp())
    {
        case EOpIndexDirect:
        case EOpIndexDirectStruct:
        case EOpIndexDirectInterfaceBlock:
            UNREACHABLE();
            break;
        case EOpIndexIndirect:
        {
            // Load the index.
            const spirv::IdRef rightValue =
                accessChainLoad(&mNodeData.back(), node->getRight()->getType(), nullptr);
            mNodeData.pop_back();

            if (!node->getLeft()->getType().isArray() && node->getLeft()->getType().isVector())
            {
                accessChainPushDynamicComponent(&mNodeData.back(), rightValue, resultTypeId);
            }
            else
            {
                accessChainPush(&mNodeData.back(), rightValue, resultTypeId);
            }
            break;
        }

        case EOpAssign:
        {
            // Load the right hand side of assignment.
            const spirv::IdRef rightValue =
                accessChainLoad(&mNodeData.back(), node->getRight()->getType(), nullptr);
            mNodeData.pop_back();

            // Store into the access chain.  Since the result of the (a = b) expression is b, change
            // the access chain to an unindexed rvalue which is |rightValue|.
            accessChainStore(&mNodeData.back(), rightValue, node->getLeft()->getType());
            nodeDataInitRValue(&mNodeData.back(), rightValue, resultTypeId);
            break;
        }

        case EOpComma:
            // When the expression a,b is visited, all side effects of a and b are already
            // processed.  What's left is to to replace the expression with the result of b.  This
            // is simply done by dropping the left node and placing the right node as the result.
            mNodeData.erase(mNodeData.begin() + mNodeData.size() - 2);
            break;

        default:
            const spirv::IdRef result = visitOperator(node, resultTypeId);
            mNodeData.pop_back();
            nodeDataInitRValue(&mNodeData.back(), result, resultTypeId);
            break;
    }

    return true;
}

bool OutputSPIRVTraverser::visitUnary(Visit visit, TIntermUnary *node)
{
    // Constants are expected to be folded.
    ASSERT(!node->hasConstantValue());

    // Special case EOpArrayLength.
    if (node->getOp() == EOpArrayLength)
    {
        visitArrayLength(node);

        // Children already visited.
        return false;
    }

    if (visit == PreVisit)
    {
        // Don't add an entry to the stack.  The child will create one, which we won't pop.
        return true;
    }

    // It's a unary operation, so there can't be an InVisit.
    ASSERT(visit != InVisit);

    // There is at least on entry for the child.
    ASSERT(mNodeData.size() >= 1);

    const spirv::IdRef resultTypeId = mBuilder.getTypeData(node->getType(), {}).id;
    const spirv::IdRef result       = visitOperator(node, resultTypeId);

    // Keep the result as rvalue.
    nodeDataInitRValue(&mNodeData.back(), result, resultTypeId);

    return true;
}

bool OutputSPIRVTraverser::visitTernary(Visit visit, TIntermTernary *node)
{
    if (visit == PreVisit)
    {
        // Don't add an entry to the stack.  The condition will create one, which we won't pop.
        return true;
    }

    size_t lastChildIndex = getLastTraversedChildIndex(visit);

    // If the condition was just visited, evaluate it and decide if OpSelect could be used or an
    // if-else must be emitted.  OpSelect is only used if neither side has a side effect.  SPIR-V
    // prior to 1.4 requires the type to be either scalar or vector.
    const TType &type   = node->getType();
    bool canUseOpSelect = (type.isScalar() || type.isVector() || mCompileOptions.emitSPIRV14) &&
                          !node->getTrueExpression()->hasSideEffects() &&
                          !node->getFalseExpression()->hasSideEffects();

    // Don't use OpSelect on buggy drivers.  Technically this is only needed if the two sides don't
    // have matching use of RelaxedPrecision, but not worth being precise about it.
    if (mCompileOptions.avoidOpSelectWithMismatchingRelaxedPrecision)
    {
        canUseOpSelect = false;
    }

    if (lastChildIndex == 0)
    {
        const TType &conditionType = node->getCondition()->getType();

        spirv::IdRef typeId;
        spirv::IdRef conditionValue = accessChainLoad(&mNodeData.back(), conditionType, &typeId);

        // If OpSelect can be used, keep the condition for later usage.
        if (canUseOpSelect)
        {
            // SPIR-V prior to 1.4 requires that the condition value have as many components as the
            // result.  So when selecting between vectors, we must replicate the condition scalar.
            if (!mCompileOptions.emitSPIRV14 && type.isVector())
            {
                const TType &boolVectorType =
                    *StaticType::GetForVec<EbtBool, EbpUndefined>(EvqGlobal, type.getNominalSize());
                typeId =
                    mBuilder.getBasicTypeId(conditionType.getBasicType(), type.getNominalSize());
                conditionValue = createConstructorVectorFromScalar(conditionType, boolVectorType,
                                                                   typeId, {{conditionValue}});
            }
            nodeDataInitRValue(&mNodeData.back(), conditionValue, typeId);
            return true;
        }

        // Otherwise generate an if-else construct.

        // Three blocks necessary; the true, false and merge.
        mBuilder.startConditional(3, false, false);

        // Generate the branch instructions.
        const SpirvConditional *conditional = mBuilder.getCurrentConditional();

        const spirv::IdRef trueBlockId  = conditional->blockIds[0];
        const spirv::IdRef falseBlockId = conditional->blockIds[1];
        const spirv::IdRef mergeBlockId = conditional->blockIds.back();

        mBuilder.writeBranchConditional(conditionValue, trueBlockId, falseBlockId, mergeBlockId);
        nodeDataInitRValue(&mNodeData.back(), conditionValue, typeId);
        return true;
    }

    // Load the result of the true or false part, and keep it for the end.  It's either used in
    // OpSelect or OpPhi.
    spirv::IdRef typeId;
    const spirv::IdRef value = accessChainLoad(&mNodeData.back(), type, &typeId);
    mNodeData.pop_back();
    mNodeData.back().idList.push_back(value);

    // Additionally store the id of block that has produced the result.
    mNodeData.back().idList.push_back(mBuilder.getSpirvCurrentFunctionBlockId());

    if (!canUseOpSelect)
    {
        // Move on to the next block.
        mBuilder.writeBranchConditionalBlockEnd();
    }

    // When done, generate either OpSelect or OpPhi.
    if (visit == PostVisit)
    {
        const spirv::IdRef result = mBuilder.getNewId(mBuilder.getDecorations(node->getType()));

        ASSERT(mNodeData.back().idList.size() == 4);
        const spirv::IdRef trueValue    = mNodeData.back().idList[0].id;
        const spirv::IdRef trueBlockId  = mNodeData.back().idList[1].id;
        const spirv::IdRef falseValue   = mNodeData.back().idList[2].id;
        const spirv::IdRef falseBlockId = mNodeData.back().idList[3].id;

        if (canUseOpSelect)
        {
            const spirv::IdRef conditionValue = mNodeData.back().baseId;

            spirv::WriteSelect(mBuilder.getSpirvCurrentFunctionBlock(), typeId, result,
                               conditionValue, trueValue, falseValue);
        }
        else
        {
            spirv::WritePhi(mBuilder.getSpirvCurrentFunctionBlock(), typeId, result,
                            {spirv::PairIdRefIdRef{trueValue, trueBlockId},
                             spirv::PairIdRefIdRef{falseValue, falseBlockId}});

            mBuilder.endConditional();
        }

        // Replace the access chain with an rvalue that's the result.
        nodeDataInitRValue(&mNodeData.back(), result, typeId);
    }

    return true;
}

bool OutputSPIRVTraverser::visitIfElse(Visit visit, TIntermIfElse *node)
{
    // An if condition may or may not have an else block.  When both blocks are present, the
    // translation is as follows:
    //
    // if (cond) { trueBody } else { falseBody }
    //
    //               // pre-if block
    //       %cond = ...
    //               OpSelectionMerge %merge None
    //               OpBranchConditional %cond %true %false
    //
    //       %true = OpLabel
    //               trueBody
    //               OpBranch %merge
    //
    //      %false = OpLabel
    //               falseBody
    //               OpBranch %merge
    //
    //               // post-if block
    //       %merge = OpLabel
    //
    // If the else block is missing, OpBranchConditional will simply jump to %merge on the false
    // condition and the %false block is removed.  Due to the way ParseContext prunes compile-time
    // constant conditionals, the if block itself may also be missing, which is treated similarly.

    // It's simpler if this function performs the traversal.
    ASSERT(visit == PreVisit);

    // Visit the condition.
    node->getCondition()->traverse(this);
    const spirv::IdRef conditionValue =
        accessChainLoad(&mNodeData.back(), node->getCondition()->getType(), nullptr);

    // If both true and false blocks are missing, there's nothing to do.
    if (node->getTrueBlock() == nullptr && node->getFalseBlock() == nullptr)
    {
        return false;
    }

    // Create a conditional with maximum 3 blocks, one for the true block (if any), one for the
    // else block (if any), and one for the merge block.  getChildCount() works here as it
    // produces an identical count.
    mBuilder.startConditional(node->getChildCount(), false, false);

    // Generate the branch instructions.
    const SpirvConditional *conditional = mBuilder.getCurrentConditional();

    const spirv::IdRef mergeBlock = conditional->blockIds.back();
    spirv::IdRef trueBlock        = mergeBlock;
    spirv::IdRef falseBlock       = mergeBlock;

    size_t nextBlockIndex = 0;
    if (node->getTrueBlock())
    {
        trueBlock = conditional->blockIds[nextBlockIndex++];
    }
    if (node->getFalseBlock())
    {
        falseBlock = conditional->blockIds[nextBlockIndex++];
    }

    mBuilder.writeBranchConditional(conditionValue, trueBlock, falseBlock, mergeBlock);

    // Visit the true block, if any.
    if (node->getTrueBlock())
    {
        node->getTrueBlock()->traverse(this);
        mBuilder.writeBranchConditionalBlockEnd();
    }

    // Visit the false block, if any.
    if (node->getFalseBlock())
    {
        node->getFalseBlock()->traverse(this);
        mBuilder.writeBranchConditionalBlockEnd();
    }

    // Pop from the conditional stack when done.
    mBuilder.endConditional();

    // Don't traverse the children, that's done already.
    return false;
}

bool OutputSPIRVTraverser::visitSwitch(Visit visit, TIntermSwitch *node)
{
    // Take the following switch:
    //
    //     switch (c)
    //     {
    //     case A:
    //         ABlock;
    //         break;
    //     case B:
    //     default:
    //         BBlock;
    //         break;
    //     case C:
    //         CBlock;
    //         // fallthrough
    //     case D:
    //         DBlock;
    //     }
    //
    // In SPIR-V, this is implemented similarly to the following pseudo-code:
    //
    //     switch c:
    //         A       -> jump %A
    //         B       -> jump %B
    //         C       -> jump %C
    //         D       -> jump %D
    //         default -> jump %B
    //
    //     %A:
    //         ABlock
    //         jump %merge
    //
    //     %B:
    //         BBlock
    //         jump %merge
    //
    //     %C:
    //         CBlock
    //         jump %D
    //
    //     %D:
    //         DBlock
    //         jump %merge
    //
    // The OpSwitch instruction contains the jump labels for the default and other cases.  Each
    // block either terminates with a jump to the merge block or the next block as fallthrough.
    //
    //               // pre-switch block
    //               OpSelectionMerge %merge None
    //               OpSwitch %cond %C A %A B %B C %C D %D
    //
    //          %A = OpLabel
    //               ABlock
    //               OpBranch %merge
    //
    //          %B = OpLabel
    //               BBlock
    //               OpBranch %merge
    //
    //          %C = OpLabel
    //               CBlock
    //               OpBranch %D
    //
    //          %D = OpLabel
    //               DBlock
    //               OpBranch %merge

    if (visit == PreVisit)
    {
        // Artificially add `if (true)` around switches as a driver bug workaround
        if (mCompileOptions.wrapSwitchInIfTrue)
        {
            const spirv::IdRef conditionValue = mBuilder.getBoolConstant(true);
            mBuilder.startConditional(2, false, false);
            const SpirvConditional *conditional = mBuilder.getCurrentConditional();
            const spirv::IdRef trueBlock        = conditional->blockIds[0];
            const spirv::IdRef mergeBlock       = conditional->blockIds[1];
            mBuilder.writeBranchConditional(conditionValue, trueBlock, mergeBlock, mergeBlock);
        }

        // Don't add an entry to the stack.  The condition will create one, which we won't pop.
        return true;
    }

    // If the condition was just visited, evaluate it and create the switch instruction.
    if (visit == InVisit)
    {
        ASSERT(getLastTraversedChildIndex(visit) == 0);

        const spirv::IdRef conditionValue =
            accessChainLoad(&mNodeData.back(), node->getInit()->getType(), nullptr);

        // First, need to find out how many blocks are there in the switch.
        const TIntermSequence &statements = *node->getStatementList()->getSequence();
        bool lastWasCase                  = true;
        size_t blockIndex                 = 0;

        size_t defaultBlockIndex = std::numeric_limits<size_t>::max();
        TVector<uint32_t> caseValues;
        TVector<size_t> caseBlockIndices;

        for (TIntermNode *statement : statements)
        {
            TIntermCase *caseLabel = statement->getAsCaseNode();
            const bool isCaseLabel = caseLabel != nullptr;

            if (isCaseLabel)
            {
                // For every case label, remember its block index.  This is used later to generate
                // the OpSwitch instruction.
                if (caseLabel->hasCondition())
                {
                    // All switch conditions are literals.
                    TIntermConstantUnion *condition =
                        caseLabel->getCondition()->getAsConstantUnion();
                    ASSERT(condition != nullptr);

                    TConstantUnion caseValue;
                    if (condition->getType().getBasicType() == EbtYuvCscStandardEXT)
                    {
                        caseValue.setUConst(
                            condition->getConstantValue()->getYuvCscStandardEXTConst());
                    }
                    else
                    {
                        bool valid = caseValue.cast(EbtUInt, *condition->getConstantValue());
                        ASSERT(valid);
                    }

                    caseValues.push_back(caseValue.getUConst());
                    caseBlockIndices.push_back(blockIndex);
                }
                else
                {
                    // Remember the block index of the default case.
                    defaultBlockIndex = blockIndex;
                }
                lastWasCase = true;
            }
            else if (lastWasCase)
            {
                // Every time a non-case node is visited and the previous statement was a case node,
                // it's a new block.
                ++blockIndex;
                lastWasCase = false;
            }
        }

        // Block count is the number of blocks based on cases + 1 for the merge block.
        const size_t blockCount = blockIndex + 1;
        mBuilder.startConditional(blockCount, false, true);

        // Generate the switch instructions.
        const SpirvConditional *conditional = mBuilder.getCurrentConditional();

        // Generate the list of caseValue->blockIndex mapping used by the OpSwitch instruction.  If
        // the switch ends in a number of cases with no statements following them, they will
        // naturally jump to the merge block!
        spirv::PairLiteralIntegerIdRefList switchTargets;

        for (size_t caseIndex = 0; caseIndex < caseValues.size(); ++caseIndex)
        {
            uint32_t value        = caseValues[caseIndex];
            size_t caseBlockIndex = caseBlockIndices[caseIndex];

            switchTargets.push_back(
                {spirv::LiteralInteger(value), conditional->blockIds[caseBlockIndex]});
        }

        const spirv::IdRef mergeBlock   = conditional->blockIds.back();
        const spirv::IdRef defaultBlock = defaultBlockIndex <= caseValues.size()
                                              ? conditional->blockIds[defaultBlockIndex]
                                              : mergeBlock;

        mBuilder.writeSwitch(conditionValue, defaultBlock, switchTargets, mergeBlock);
        return true;
    }

    // Terminate the last block if not already and end the conditional.
    mBuilder.writeSwitchCaseBlockEnd();
    mBuilder.endConditional();

    if (mCompileOptions.wrapSwitchInIfTrue)
    {
        mBuilder.writeBranchConditionalBlockEnd();
        mBuilder.endConditional();
    }

    return true;
}

bool OutputSPIRVTraverser::visitCase(Visit visit, TIntermCase *node)
{
    ASSERT(visit == PreVisit);

    mNodeData.emplace_back();

    TIntermBlock *parent    = getParentNode()->getAsBlock();
    const size_t childIndex = getParentChildIndex(PreVisit);

    ASSERT(parent);
    const TIntermSequence &parentStatements = *parent->getSequence();

    // Check the previous statement.  If it was not a |case|, then a new block is being started so
    // handle fallthrough:
    //
    //     ...
    //        statement;
    //     case X:         <--- end the previous block here
    //     case Y:
    //
    //
    if (childIndex > 0 && parentStatements[childIndex - 1]->getAsCaseNode() == nullptr)
    {
        mBuilder.writeSwitchCaseBlockEnd();
    }

    // Don't traverse the condition, as it was processed in visitSwitch.
    return false;
}

bool OutputSPIRVTraverser::visitBlock(Visit visit, TIntermBlock *node)
{
    // If global block, nothing to do.
    if (getCurrentTraversalDepth() == 0)
    {
        return true;
    }

    // Any construct that needs code blocks must have already handled creating the necessary blocks
    // and setting the right one "current".  If there's a block opened in GLSL for scoping reasons,
    // it's ignored here as there are no scopes within a function in SPIR-V.
    if (visit == PreVisit)
    {
        return node->getChildCount() > 0;
    }

    // Any node that needed to generate code has already done so, just clean up its data.  If
    // the child node has no effect, it's automatically discarded (such as variable.field[n].x,
    // side effects of n already having generated code).
    //
    // Blocks inside blocks like:
    //
    //     {
    //         statement;
    //         {
    //             statement2;
    //         }
    //     }
    //
    // don't generate nodes.
    const size_t childIndex           = getLastTraversedChildIndex(visit);
    const TIntermSequence &statements = *node->getSequence();

    if (statements[childIndex]->getAsBlock() == nullptr)
    {
        mNodeData.pop_back();
    }

    return true;
}

bool OutputSPIRVTraverser::visitFunctionDefinition(Visit visit, TIntermFunctionDefinition *node)
{
    if (visit == PreVisit)
    {
        return true;
    }

    const TFunction *function = node->getFunction();

    ASSERT(mFunctionIdMap.count(function) > 0);
    const FunctionIds &ids = mFunctionIdMap[function];

    // After the prototype is visited, generate the initial code for the function.
    if (visit == InVisit)
    {
        // Declare the function.
        spirv::WriteFunction(mBuilder.getSpirvFunctions(), ids.returnTypeId, ids.functionId,
                             spv::FunctionControlMaskNone, ids.functionTypeId);

        for (size_t paramIndex = 0; paramIndex < function->getParamCount(); ++paramIndex)
        {
            const TVariable *paramVariable = function->getParam(paramIndex);

            const spirv::IdRef paramId =
                mBuilder.getNewId(mBuilder.getDecorations(paramVariable->getType()));
            spirv::WriteFunctionParameter(mBuilder.getSpirvFunctions(),
                                          ids.parameterTypeIds[paramIndex], paramId);

            // Remember the id of the variable for future look up.
            ASSERT(mSymbolIdMap.count(paramVariable) == 0);
            mSymbolIdMap[paramVariable] = paramId;

            mBuilder.writeDebugName(paramId, mBuilder.getName(paramVariable).data());
        }

        mBuilder.startNewFunction(ids.functionId, function);

        // For main(), add a non-semantic instruction at the beginning for any initialization code
        // the transformer may want to add.
        if (ids.functionId == vk::spirv::kIdEntryPoint &&
            mCompiler->getShaderType() != GL_COMPUTE_SHADER)
        {
            ASSERT(function->isMain());
            mBuilder.writeNonSemanticInstruction(vk::spirv::kNonSemanticEnter);
        }

        mCurrentFunctionId = ids.functionId;

        return true;
    }

    // If no explicit return was specified, add one automatically here.
    if (!mBuilder.isCurrentFunctionBlockTerminated())
    {
        if (function->getReturnType().getBasicType() == EbtVoid)
        {
            switch (ids.functionId)
            {
                case vk::spirv::kIdEntryPoint:
                    // For main(), add a non-semantic instruction at the end of the shader.
                    markVertexOutputOnShaderEnd();
                    break;
                case vk::spirv::kIdXfbEmulationCaptureFunction:
                    // For the transform feedback emulation capture function, add a non-semantic
                    // instruction before return for the transformer to fill in as necessary.
                    mBuilder.writeNonSemanticInstruction(
                        vk::spirv::kNonSemanticTransformFeedbackEmulation);
                    break;
            }

            spirv::WriteReturn(mBuilder.getSpirvCurrentFunctionBlock());
        }
        else
        {
            // GLSL allows functions expecting a return value to miss a return.  In that case,
            // return a null constant.
            const TType &returnType = function->getReturnType();
            spirv::IdRef nullConstant;
            if (returnType.isScalar() && !returnType.isArray())
            {
                switch (function->getReturnType().getBasicType())
                {
                    case EbtFloat:
                        nullConstant = mBuilder.getFloatConstant(0);
                        break;
                    case EbtUInt:
                        nullConstant = mBuilder.getUintConstant(0);
                        break;
                    case EbtInt:
                        nullConstant = mBuilder.getIntConstant(0);
                        break;
                    default:
                        break;
                }
            }
            if (!nullConstant.valid())
            {
                nullConstant = mBuilder.getNullConstant(ids.returnTypeId);
            }
            spirv::WriteReturnValue(mBuilder.getSpirvCurrentFunctionBlock(), nullConstant);
        }
        mBuilder.terminateCurrentFunctionBlock();
    }

    mBuilder.assembleSpirvFunctionBlocks();

    // End the function
    spirv::WriteFunctionEnd(mBuilder.getSpirvFunctions());

    mCurrentFunctionId = {};

    return true;
}

bool OutputSPIRVTraverser::visitGlobalQualifierDeclaration(Visit visit,
                                                           TIntermGlobalQualifierDeclaration *node)
{
    if (node->isPrecise())
    {
        // Nothing to do for |precise|.
        return false;
    }

    // Global qualifier declarations apply to variables that are already declared.  Invariant simply
    // adds a decoration to the variable declaration, which can be done right away.  Note that
    // invariant cannot be applied to block members like this, except for gl_PerVertex built-ins,
    // which are applied to the members directly by DeclarePerVertexBlocks.
    ASSERT(node->isInvariant());

    const TVariable *variable = &node->getSymbol()->variable();
    ASSERT(mSymbolIdMap.count(variable) > 0);

    const spirv::IdRef variableId = mSymbolIdMap[variable];

    spirv::WriteDecorate(mBuilder.getSpirvDecorations(), variableId, spv::DecorationInvariant, {});

    return false;
}

void OutputSPIRVTraverser::visitFunctionPrototype(TIntermFunctionPrototype *node)
{
    const TFunction *function = node->getFunction();

    // If the function was previously forward declared, skip this.
    if (mFunctionIdMap.count(function) > 0)
    {
        return;
    }

    FunctionIds ids;

    // Declare the function type
    ids.returnTypeId = mBuilder.getTypeData(function->getReturnType(), {}).id;

    spirv::IdRefList paramTypeIds;
    for (size_t paramIndex = 0; paramIndex < function->getParamCount(); ++paramIndex)
    {
        const TType &paramType = function->getParam(paramIndex)->getType();

        spirv::IdRef paramId = mBuilder.getTypeData(paramType, {}).id;

        // const function parameters are intermediate values, while the rest are "variables"
        // with the Function storage class.
        if (paramType.getQualifier() != EvqParamConst)
        {
            const spv::StorageClass storageClass = IsOpaqueType(paramType.getBasicType())
                                                       ? spv::StorageClassUniformConstant
                                                       : spv::StorageClassFunction;
            paramId                              = mBuilder.getTypePointerId(paramId, storageClass);
        }

        ids.parameterTypeIds.push_back(paramId);
    }

    ids.functionTypeId = mBuilder.getFunctionTypeId(ids.returnTypeId, ids.parameterTypeIds);

    // Allocate an id for the function up-front.
    //
    // Apply decorations to the return value of the function by applying them to the OpFunction
    // instruction.
    //
    // Note that some functions have predefined ids.
    if (function->isMain())
    {
        ids.functionId = spirv::IdRef(vk::spirv::kIdEntryPoint);
    }
    else
    {
        ids.functionId = mBuilder.getReservedOrNewId(
            function->uniqueId(), mBuilder.getDecorations(function->getReturnType()));
    }

    // Remember the id of the function for future look up.
    mFunctionIdMap[function] = ids;
}

bool OutputSPIRVTraverser::visitAggregate(Visit visit, TIntermAggregate *node)
{
    // Constants are expected to be folded.  However, large constructors (such as arrays) are not
    // folded and are handled here.
    ASSERT(node->getOp() == EOpConstruct || !node->hasConstantValue());

    if (visit == PreVisit)
    {
        mNodeData.emplace_back();
        return true;
    }

    // Keep the parameters on the stack.  If a function call contains out or inout parameters, we
    // need to know the access chains for the eventual write back to them.
    if (visit == InVisit)
    {
        return true;
    }

    // Expect to have accumulated as many parameters as the node requires.
    ASSERT(mNodeData.size() > node->getChildCount());

    const spirv::IdRef resultTypeId = mBuilder.getTypeData(node->getType(), {}).id;
    spirv::IdRef result;

    switch (node->getOp())
    {
        case EOpConstruct:
            // Construct a value out of the accumulated parameters.
            result = createConstructor(node, resultTypeId);
            break;
        case EOpCallFunctionInAST:
            // Create a call to the function.
            result = createFunctionCall(node, resultTypeId);
            break;

            // For barrier functions the scope is device, or with the Vulkan memory model, the queue
            // family.  We don't use the Vulkan memory model.
        case EOpBarrier:
            spirv::WriteControlBarrier(
                mBuilder.getSpirvCurrentFunctionBlock(),
                mBuilder.getUintConstant(spv::ScopeWorkgroup),
                mBuilder.getUintConstant(spv::ScopeWorkgroup),
                mBuilder.getUintConstant(spv::MemorySemanticsWorkgroupMemoryMask |
                                         spv::MemorySemanticsAcquireReleaseMask));
            break;
        case EOpBarrierTCS:
            // Note: The memory scope and semantics are different with the Vulkan memory model,
            // which is not supported.
            spirv::WriteControlBarrier(mBuilder.getSpirvCurrentFunctionBlock(),
                                       mBuilder.getUintConstant(spv::ScopeWorkgroup),
                                       mBuilder.getUintConstant(spv::ScopeInvocation),
                                       mBuilder.getUintConstant(spv::MemorySemanticsMaskNone));
            break;
        case EOpMemoryBarrier:
        case EOpGroupMemoryBarrier:
        {
            const spv::Scope scope =
                node->getOp() == EOpMemoryBarrier ? spv::ScopeDevice : spv::ScopeWorkgroup;
            spirv::WriteMemoryBarrier(
                mBuilder.getSpirvCurrentFunctionBlock(), mBuilder.getUintConstant(scope),
                mBuilder.getUintConstant(spv::MemorySemanticsUniformMemoryMask |
                                         spv::MemorySemanticsWorkgroupMemoryMask |
                                         spv::MemorySemanticsImageMemoryMask |
                                         spv::MemorySemanticsAcquireReleaseMask));
            break;
        }
        case EOpMemoryBarrierBuffer:
            spirv::WriteMemoryBarrier(
                mBuilder.getSpirvCurrentFunctionBlock(), mBuilder.getUintConstant(spv::ScopeDevice),
                mBuilder.getUintConstant(spv::MemorySemanticsUniformMemoryMask |
                                         spv::MemorySemanticsAcquireReleaseMask));
            break;
        case EOpMemoryBarrierImage:
            spirv::WriteMemoryBarrier(
                mBuilder.getSpirvCurrentFunctionBlock(), mBuilder.getUintConstant(spv::ScopeDevice),
                mBuilder.getUintConstant(spv::MemorySemanticsImageMemoryMask |
                                         spv::MemorySemanticsAcquireReleaseMask));
            break;
        case EOpMemoryBarrierShared:
            spirv::WriteMemoryBarrier(
                mBuilder.getSpirvCurrentFunctionBlock(), mBuilder.getUintConstant(spv::ScopeDevice),
                mBuilder.getUintConstant(spv::MemorySemanticsWorkgroupMemoryMask |
                                         spv::MemorySemanticsAcquireReleaseMask));
            break;
        case EOpMemoryBarrierAtomicCounter:
            // Atomic counters are emulated.
            UNREACHABLE();
            break;

        case EOpEmitVertex:
            if (mCurrentFunctionId == vk::spirv::kIdEntryPoint)
            {
                // Add a non-semantic instruction before EmitVertex.
                markVertexOutputOnEmitVertex();
            }
            spirv::WriteEmitVertex(mBuilder.getSpirvCurrentFunctionBlock());
            break;
        case EOpEndPrimitive:
            spirv::WriteEndPrimitive(mBuilder.getSpirvCurrentFunctionBlock());
            break;

        case EOpBeginInvocationInterlockARB:
            // Set up a "pixel_interlock_ordered" execution mode, as that is the default
            // interlocked execution mode in GLSL, and we don't currently expose an option to change
            // that.
            mBuilder.addExtension(SPIRVExtensions::FragmentShaderInterlockARB);
            mBuilder.addCapability(spv::CapabilityFragmentShaderPixelInterlockEXT);
            mBuilder.addExecutionMode(spv::ExecutionMode::ExecutionModePixelInterlockOrderedEXT);
            // Compile GL_ARB_fragment_shader_interlock to SPV_EXT_fragment_shader_interlock.
            spirv::WriteBeginInvocationInterlockEXT(mBuilder.getSpirvCurrentFunctionBlock());
            break;

        case EOpEndInvocationInterlockARB:
            // Compile GL_ARB_fragment_shader_interlock to SPV_EXT_fragment_shader_interlock.
            spirv::WriteEndInvocationInterlockEXT(mBuilder.getSpirvCurrentFunctionBlock());
            break;

        default:
            result = visitOperator(node, resultTypeId);
            break;
    }

    // Pop the parameters.
    mNodeData.resize(mNodeData.size() - node->getChildCount());

    // Keep the result as rvalue.
    nodeDataInitRValue(&mNodeData.back(), result, resultTypeId);

    return false;
}

bool OutputSPIRVTraverser::visitDeclaration(Visit visit, TIntermDeclaration *node)
{
    const TIntermSequence &sequence = *node->getSequence();

    // Enforced by ValidateASTOptions::validateMultiDeclarations.
    ASSERT(sequence.size() == 1);

    // Declare specialization constants especially; they don't require processing the left and right
    // nodes, and they are like constant declarations with special instructions and decorations.
    const TQualifier qualifier = sequence.front()->getAsTyped()->getType().getQualifier();
    if (qualifier == EvqSpecConst)
    {
        declareSpecConst(node);
        return false;
    }
    // Similarly, constant declarations are turned into actual constants.
    if (qualifier == EvqConst)
    {
        declareConst(node);
        return false;
    }

    // Skip redeclaration of builtins.  They will correctly declare as built-in on first use.
    if (mInGlobalScope &&
        (qualifier == EvqClipDistance || qualifier == EvqCullDistance || qualifier == EvqFragDepth))
    {
        return false;
    }

    if (!mInGlobalScope && visit == PreVisit)
    {
        mNodeData.emplace_back();
    }

    mIsSymbolBeingDeclared = visit == PreVisit;

    if (visit != PostVisit)
    {
        return true;
    }

    TIntermSymbol *symbol = sequence.front()->getAsSymbolNode();
    spirv::IdRef initializerId;
    bool initializeWithDeclaration = false;
    bool needsQuantizeTo16         = false;

    // Handle declarations with initializer.
    if (symbol == nullptr)
    {
        TIntermBinary *assign = sequence.front()->getAsBinaryNode();
        ASSERT(assign != nullptr && assign->getOp() == EOpInitialize);

        symbol = assign->getLeft()->getAsSymbolNode();
        ASSERT(symbol != nullptr);

        // In SPIR-V, it's only possible to initialize a variable together with its declaration if
        // the initializer is a constant or a global variable.  We ignore the global variable case
        // to avoid tracking whether the variable has been modified since the beginning of the
        // function.  Since variable declarations are always placed at the beginning of the function
        // in SPIR-V, it would be wrong for example to initialize |var| below with the global
        // variable at declaration time:
        //
        //     vec4 global = A;
        //     void f()
        //     {
        //         global = B;
        //         {
        //             vec4 var = global;
        //         }
        //     }
        //
        // So the initializer is only used when declaring a variable when it's a constant
        // expression.  Note that if the variable being declared is itself global (and the
        // initializer is not constant), a previous AST transformation (DeferGlobalInitializers)
        // makes sure their initialization is deferred to the beginning of main.
        //
        // Additionally, if the variable is being defined inside a loop, the initializer is not used
        // as that would prevent it from being reinitialized in the next iteration of the loop.

        TIntermTyped *initializer = assign->getRight();
        initializeWithDeclaration =
            !mBuilder.isInLoop() &&
            (initializer->getAsConstantUnion() != nullptr || initializer->hasConstantValue());

        if (initializeWithDeclaration)
        {
            // If a constant, take the Id directly.
            initializerId = mNodeData.back().baseId;
        }
        else
        {
            // Otherwise generate code to load from right hand side expression.
            initializerId = accessChainLoad(&mNodeData.back(), symbol->getType(), nullptr);

            // Workaround for issuetracker.google.com/274859104
            // ARM SpirV compiler may utilize the RelaxedPrecision of mediump float,
            // and chooses to not cast mediump float to 16 bit. This causes deqp test
            // dEQP-GLES2.functional.shaders.algorithm.rgb_to_hsl_vertex failed.
            // The reason is that GLSL shader code expects below condition to be true:
            // mediump float a == mediump float b;
            // However, the condition is false after translating to SpirV
            // due to one of them is 32 bit, and the other is 16 bit.
            // To resolve the deqp test failure, we will add an OpQuantizeToF16
            // SpirV instruction to explicitly cast mediump float scalar or mediump float
            // vector to 16 bit, if the right-hand-side is a highp float.
            if (mCompileOptions.castMediumpFloatTo16Bit)
            {
                const TType leftType            = assign->getLeft()->getType();
                const TType rightType           = assign->getRight()->getType();
                const TPrecision leftPrecision  = leftType.getPrecision();
                const TPrecision rightPrecision = rightType.getPrecision();
                const bool isLeftScalarFloat    = leftType.isScalarFloat();
                const bool isLeftVectorFloat = leftType.isVector() && !leftType.isVectorArray() &&
                                               leftType.getBasicType() == EbtFloat;

                if (leftPrecision == TPrecision::EbpMedium &&
                    rightPrecision == TPrecision::EbpHigh &&
                    (isLeftScalarFloat || isLeftVectorFloat))
                {
                    needsQuantizeTo16 = true;
                }
            }
        }

        // Clean up the initializer data.
        mNodeData.pop_back();
    }

    const TType &type         = symbol->getType();
    const TVariable *variable = &symbol->variable();

    // If this is just a struct declaration (and not a variable declaration), don't declare the
    // struct up-front and let it be lazily defined.  If the struct is only used inside an interface
    // block for example, this avoids it being doubly defined (once with the unspecified block
    // storage and once with interface block's).
    if (type.isStructSpecifier() && variable->symbolType() == SymbolType::Empty)
    {
        return false;
    }

    const spirv::IdRef typeId = mBuilder.getTypeData(type, {}).id;

    spv::StorageClass storageClass =
        GetStorageClass(mCompileOptions, type, mCompiler->getShaderType());

    SpirvDecorations decorations = mBuilder.getDecorations(type);
    if (mBuilder.isInvariantOutput(type))
    {
        // Apply the Invariant decoration to output variables if specified or if globally enabled.
        decorations.push_back(spv::DecorationInvariant);
    }
    // Apply the declared memory qualifiers.
    TMemoryQualifier memoryQualifier = type.getMemoryQualifier();
    if (memoryQualifier.coherent)
    {
        decorations.push_back(spv::DecorationCoherent);
    }
    if (memoryQualifier.volatileQualifier)
    {
        decorations.push_back(spv::DecorationVolatile);
    }
    if (memoryQualifier.restrictQualifier)
    {
        decorations.push_back(spv::DecorationRestrict);
    }
    if (memoryQualifier.readonly)
    {
        decorations.push_back(spv::DecorationNonWritable);
    }
    if (memoryQualifier.writeonly)
    {
        decorations.push_back(spv::DecorationNonReadable);
    }

    const spirv::IdRef variableId = mBuilder.declareVariable(
        typeId, storageClass, decorations, initializeWithDeclaration ? &initializerId : nullptr,
        mBuilder.getName(variable).data(), &variable->uniqueId());

    if (!initializeWithDeclaration && initializerId.valid())
    {
        // If not initializing at the same time as the declaration, issue a store
        if (needsQuantizeTo16)
        {
            // Insert OpQuantizeToF16 instruction to explicitly cast mediump float to 16 bit before
            // issuing an OpStore instruction.
            const spirv::IdRef quantizeToF16Result =
                mBuilder.getNewId(mBuilder.getDecorations(symbol->getType()));
            spirv::WriteQuantizeToF16(mBuilder.getSpirvCurrentFunctionBlock(), typeId,
                                      quantizeToF16Result, initializerId);
            initializerId = quantizeToF16Result;
        }
        spirv::WriteStore(mBuilder.getSpirvCurrentFunctionBlock(), variableId, initializerId,
                          nullptr);
    }

    const bool isShaderInOut = IsShaderIn(type.getQualifier()) || IsShaderOut(type.getQualifier());
    const bool isInterfaceBlock = type.getBasicType() == EbtInterfaceBlock;

    // Add decorations, which apply to the element type of arrays, if array.
    spirv::IdRef nonArrayTypeId = typeId;
    if (type.isArray() && (isShaderInOut || isInterfaceBlock))
    {
        SpirvType elementType  = mBuilder.getSpirvType(type, {});
        elementType.arraySizes = {};
        nonArrayTypeId         = mBuilder.getSpirvTypeData(elementType, nullptr).id;
    }

    if (isShaderInOut)
    {
        if (IsShaderIoBlock(type.getQualifier()) && type.isInterfaceBlock())
        {
            // For gl_PerVertex in particular, write the necessary BuiltIn decorations
            if (type.getQualifier() == EvqPerVertexIn || type.getQualifier() == EvqPerVertexOut)
            {
                mBuilder.writePerVertexBuiltIns(type, nonArrayTypeId);
            }

            // I/O blocks are decorated with Block
            spirv::WriteDecorate(mBuilder.getSpirvDecorations(), nonArrayTypeId,
                                 spv::DecorationBlock, {});
        }
        else if (type.getQualifier() == EvqPatchIn || type.getQualifier() == EvqPatchOut)
        {
            // Tessellation shaders can have their input or output qualified with |patch|.  For I/O
            // blocks, the members are decorated instead.
            spirv::WriteDecorate(mBuilder.getSpirvDecorations(), variableId, spv::DecorationPatch,
                                 {});
        }
    }
    else if (isInterfaceBlock)
    {
        // For uniform and buffer variables, with SPIR-V 1.3 add Block and BufferBlock decorations
        // respectively.  With SPIR-V 1.4, always add Block.
        const spv::Decoration decoration =
            mCompileOptions.emitSPIRV14 || type.getQualifier() == EvqUniform
                ? spv::DecorationBlock
                : spv::DecorationBufferBlock;
        spirv::WriteDecorate(mBuilder.getSpirvDecorations(), nonArrayTypeId, decoration, {});

        if (type.getQualifier() == EvqBuffer && !memoryQualifier.restrictQualifier &&
            mCompileOptions.aliasedUnlessRestrict)
        {
            // If GLSL does not specify the SSBO has restrict memory qualifier, assume the
            // memory qualifier is aliased
            // issuetracker.google.com/266235549
            spirv::WriteDecorate(mBuilder.getSpirvDecorations(), variableId, spv::DecorationAliased,
                                 {});
        }
    }
    else if (IsImage(type.getBasicType()) && type.getQualifier() == EvqUniform)
    {
        // If GLSL does not specify the image has restrict memory qualifier, assume the memory
        // qualifier is aliased
        // issuetracker.google.com/266235549
        if (!memoryQualifier.restrictQualifier && mCompileOptions.aliasedUnlessRestrict)
        {
            spirv::WriteDecorate(mBuilder.getSpirvDecorations(), variableId, spv::DecorationAliased,
                                 {});
        }
    }

    // Write DescriptorSet, Binding, Location etc decorations if necessary.
    mBuilder.writeInterfaceVariableDecorations(type, variableId);

    // Remember the id of the variable for future look up.  For interface blocks, also remember the
    // id of the interface block.
    ASSERT(mSymbolIdMap.count(variable) == 0);
    mSymbolIdMap[variable] = variableId;

    if (type.isInterfaceBlock())
    {
        ASSERT(mSymbolIdMap.count(type.getInterfaceBlock()) == 0);
        mSymbolIdMap[type.getInterfaceBlock()] = variableId;
    }

    return false;
}

void GetLoopBlocks(const SpirvConditional *conditional,
                   TLoopType loopType,
                   bool hasCondition,
                   spirv::IdRef *headerBlock,
                   spirv::IdRef *condBlock,
                   spirv::IdRef *bodyBlock,
                   spirv::IdRef *continueBlock,
                   spirv::IdRef *mergeBlock)
{
    // The order of the blocks is for |for| and |while|:
    //
    //     %header %cond [optional] %body %continue %merge
    //
    // and for |do-while|:
    //
    //     %header %body %cond %merge
    //
    // Note that the |break| target is always the last block and the |continue| target is the one
    // before last.
    //
    // If %continue is not present, all jumps are made to %cond (which is necessarily present).
    // If %cond is not present, all jumps are made to %body instead.

    size_t nextBlock = 0;
    *headerBlock     = conditional->blockIds[nextBlock++];
    // %cond, if any is after header except for |do-while|.
    if (loopType != ELoopDoWhile && hasCondition)
    {
        *condBlock = conditional->blockIds[nextBlock++];
    }
    *bodyBlock = conditional->blockIds[nextBlock++];
    // After the block is either %cond or %continue based on |do-while| or not.
    if (loopType != ELoopDoWhile)
    {
        *continueBlock = conditional->blockIds[nextBlock++];
    }
    else
    {
        *condBlock = conditional->blockIds[nextBlock++];
    }
    *mergeBlock = conditional->blockIds[nextBlock++];

    ASSERT(nextBlock == conditional->blockIds.size());

    if (!continueBlock->valid())
    {
        ASSERT(condBlock->valid());
        *continueBlock = *condBlock;
    }
    if (!condBlock->valid())
    {
        *condBlock = *bodyBlock;
    }
}

bool OutputSPIRVTraverser::visitLoop(Visit visit, TIntermLoop *node)
{
    // There are three kinds of loops, and they translate as such:
    //
    // for (init; cond; expr) body;
    //
    //               // pre-loop block
    //               init
    //               OpBranch %header
    //
    //     %header = OpLabel
    //               OpLoopMerge %merge %continue None
    //               OpBranch %cond
    //
    //               // Note: if cond doesn't exist, this section is not generated.  The above
    //               // OpBranch would jump directly to %body.
    //       %cond = OpLabel
    //          %v = cond
    //               OpBranchConditional %v %body %merge None
    //
    //       %body = OpLabel
    //               body
    //               OpBranch %continue
    //
    //   %continue = OpLabel
    //               expr
    //               OpBranch %header
    //
    //               // post-loop block
    //       %merge = OpLabel
    //
    //
    // while (cond) body;
    //
    //               // pre-for block
    //               OpBranch %header
    //
    //     %header = OpLabel
    //               OpLoopMerge %merge %continue None
    //               OpBranch %cond
    //
    //       %cond = OpLabel
    //          %v = cond
    //               OpBranchConditional %v %body %merge None
    //
    //       %body = OpLabel
    //               body
    //               OpBranch %continue
    //
    //   %continue = OpLabel
    //               OpBranch %header
    //
    //               // post-loop block
    //       %merge = OpLabel
    //
    //
    // do body; while (cond);
    //
    //               // pre-for block
    //               OpBranch %header
    //
    //     %header = OpLabel
    //               OpLoopMerge %merge %cond None
    //               OpBranch %body
    //
    //       %body = OpLabel
    //               body
    //               OpBranch %cond
    //
    //       %cond = OpLabel
    //          %v = cond
    //               OpBranchConditional %v %header %merge None
    //
    //               // post-loop block
    //       %merge = OpLabel
    //

    // The order of the blocks is not necessarily the same as traversed, so it's much simpler if
    // this function enforces traversal in the right order.
    ASSERT(visit == PreVisit);
    mNodeData.emplace_back();

    const TLoopType loopType = node->getType();

    // The init statement of a for loop is placed in the previous block, so continue generating code
    // as-is until that statement is done.
    if (node->getInit())
    {
        ASSERT(loopType == ELoopFor);
        node->getInit()->traverse(this);
        mNodeData.pop_back();
    }

    const bool hasCondition = node->getCondition() != nullptr;

    // Once the init node is visited, if any, we need to set up the loop.
    //
    // For |for| and |while|, we need %header, %body, %continue and %merge.  For |do-while|, we
    // need %header, %body and %merge.  If condition is present, an additional %cond block is
    // needed in each case.
    const size_t blockCount = (loopType == ELoopDoWhile ? 3 : 4) + (hasCondition ? 1 : 0);
    mBuilder.startConditional(blockCount, true, true);

    // Generate the %header block.
    const SpirvConditional *conditional = mBuilder.getCurrentConditional();

    spirv::IdRef headerBlock, condBlock, bodyBlock, continueBlock, mergeBlock;
    GetLoopBlocks(conditional, loopType, hasCondition, &headerBlock, &condBlock, &bodyBlock,
                  &continueBlock, &mergeBlock);

    mBuilder.writeLoopHeader(loopType == ELoopDoWhile ? bodyBlock : condBlock, continueBlock,
                             mergeBlock);

    // %cond, if any is after header except for |do-while|.
    if (loopType != ELoopDoWhile && hasCondition)
    {
        node->getCondition()->traverse(this);

        // Generate the branch at the end of the %cond block.
        const spirv::IdRef conditionValue =
            accessChainLoad(&mNodeData.back(), node->getCondition()->getType(), nullptr);
        mBuilder.writeLoopConditionEnd(conditionValue, bodyBlock, mergeBlock);

        mNodeData.pop_back();
    }

    // Next comes %body.
    {
        node->getBody()->traverse(this);

        // Generate the branch at the end of the %body block.
        mBuilder.writeLoopBodyEnd(continueBlock);
    }

    switch (loopType)
    {
        case ELoopFor:
            // For |for| loops, the expression is placed after the body and acts as the continue
            // block.
            if (node->getExpression())
            {
                node->getExpression()->traverse(this);
                mNodeData.pop_back();
            }

            // Generate the branch at the end of the %continue block.
            mBuilder.writeLoopContinueEnd(headerBlock);
            break;

        case ELoopWhile:
            // |for| loops have the expression in the continue block and |do-while| loops have their
            // condition block act as the loop's continue block.  |while| loops need a branch-only
            // continue loop, which is generated here.
            mBuilder.writeLoopContinueEnd(headerBlock);
            break;

        case ELoopDoWhile:
            // For |do-while|, %cond comes last.
            ASSERT(hasCondition);
            node->getCondition()->traverse(this);

            // Generate the branch at the end of the %cond block.
            const spirv::IdRef conditionValue =
                accessChainLoad(&mNodeData.back(), node->getCondition()->getType(), nullptr);
            mBuilder.writeLoopConditionEnd(conditionValue, headerBlock, mergeBlock);

            mNodeData.pop_back();
            break;
    }

    // Pop from the conditional stack when done.
    mBuilder.endConditional();

    // Don't traverse the children, that's done already.
    return false;
}

bool OutputSPIRVTraverser::visitBranch(Visit visit, TIntermBranch *node)
{
    if (visit == PreVisit)
    {
        mNodeData.emplace_back();
        return true;
    }

    // There is only ever one child at most.
    ASSERT(visit != InVisit);

    switch (node->getFlowOp())
    {
        case EOpKill:
            spirv::WriteKill(mBuilder.getSpirvCurrentFunctionBlock());
            mBuilder.terminateCurrentFunctionBlock();
            break;
        case EOpBreak:
            spirv::WriteBranch(mBuilder.getSpirvCurrentFunctionBlock(),
                               mBuilder.getBreakTargetId());
            mBuilder.terminateCurrentFunctionBlock();
            break;
        case EOpContinue:
            spirv::WriteBranch(mBuilder.getSpirvCurrentFunctionBlock(),
                               mBuilder.getContinueTargetId());
            mBuilder.terminateCurrentFunctionBlock();
            break;
        case EOpReturn:
            // Evaluate the expression if any, and return.
            if (node->getExpression() != nullptr)
            {
                ASSERT(mNodeData.size() >= 1);

                const spirv::IdRef expressionValue =
                    accessChainLoad(&mNodeData.back(), node->getExpression()->getType(), nullptr);
                mNodeData.pop_back();

                spirv::WriteReturnValue(mBuilder.getSpirvCurrentFunctionBlock(), expressionValue);
                mBuilder.terminateCurrentFunctionBlock();
            }
            else
            {
                if (mCurrentFunctionId == vk::spirv::kIdEntryPoint)
                {
                    // For main(), add a non-semantic instruction at the end of the shader.
                    markVertexOutputOnShaderEnd();
                }
                spirv::WriteReturn(mBuilder.getSpirvCurrentFunctionBlock());
                mBuilder.terminateCurrentFunctionBlock();
            }
            break;
        default:
            UNREACHABLE();
    }

    return true;
}

void OutputSPIRVTraverser::visitPreprocessorDirective(TIntermPreprocessorDirective *node)
{
    // No preprocessor directives expected at this point.
    UNREACHABLE();
}

void OutputSPIRVTraverser::markVertexOutputOnShaderEnd()
{
    // Output happens in vertex and fragment stages at return from main.
    // In geometry shaders, it's done at EmitVertex.
    switch (mCompiler->getShaderType())
    {
        case GL_FRAGMENT_SHADER:
        case GL_VERTEX_SHADER:
        case GL_TESS_CONTROL_SHADER_EXT:
        case GL_TESS_EVALUATION_SHADER_EXT:
            mBuilder.writeNonSemanticInstruction(vk::spirv::kNonSemanticOutput);
            break;
        default:
            break;
    }
}

void OutputSPIRVTraverser::markVertexOutputOnEmitVertex()
{
    // Vertex output happens in the geometry stage at EmitVertex.
    if (mCompiler->getShaderType() == GL_GEOMETRY_SHADER)
    {
        mBuilder.writeNonSemanticInstruction(vk::spirv::kNonSemanticOutput);
    }
}

spirv::Blob OutputSPIRVTraverser::getSpirv()
{
    spirv::Blob result = mBuilder.getSpirv();

    // Validate that correct SPIR-V was generated
    ASSERT(spirv::Validate(result));

#if ANGLE_DEBUG_SPIRV_GENERATION
    // Disassemble and log the generated SPIR-V for debugging.
    spvtools::SpirvTools spirvTools(mCompileOptions.emitSPIRV14 ? SPV_ENV_VULKAN_1_1_SPIRV_1_4
                                                                : SPV_ENV_VULKAN_1_1);
    std::string readableSpirv;
    spirvTools.Disassemble(result, &readableSpirv,
                           SPV_BINARY_TO_TEXT_OPTION_COMMENT | SPV_BINARY_TO_TEXT_OPTION_INDENT |
                               SPV_BINARY_TO_TEXT_OPTION_NESTED_INDENT);
    fprintf(stderr, "%s\n", readableSpirv.c_str());
#endif  // ANGLE_DEBUG_SPIRV_GENERATION

    return result;
}
}  // anonymous namespace

bool OutputSPIRV(TCompiler *compiler,
                 TIntermBlock *root,
                 const ShCompileOptions &compileOptions,
                 const angle::HashMap<int, uint32_t> &uniqueToSpirvIdMap,
                 uint32_t firstUnusedSpirvId)
{
    // Find the list of nodes that require NoContraction (as a result of |precise|).
    if (compiler->hasAnyPreciseType())
    {
        FindPreciseNodes(compiler, root);
    }

    // Traverse the tree and generate SPIR-V instructions
    OutputSPIRVTraverser traverser(compiler, compileOptions, uniqueToSpirvIdMap,
                                   firstUnusedSpirvId);
    root->traverse(&traverser);

    // Generate the final SPIR-V and store in the sink
    spirv::Blob spirvBlob = traverser.getSpirv();
    compiler->getInfoSink().obj.setBinary(std::move(spirvBlob));

    return true;
}
}  // namespace sh
