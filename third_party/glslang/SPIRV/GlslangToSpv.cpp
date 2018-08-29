//
// Copyright (C) 2014-2016 LunarG, Inc.
// Copyright (C) 2015-2016 Google, Inc.
// Copyright (C) 2017 ARM Limited.
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above
//    copyright notice, this list of conditions and the following
//    disclaimer in the documentation and/or other materials provided
//    with the distribution.
//
//    Neither the name of 3Dlabs Inc. Ltd. nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

//
// Visit the nodes in the glslang intermediate tree representation to
// translate them to SPIR-V.
//

#include "spirv.hpp"
#include "GlslangToSpv.h"
#include "SpvBuilder.h"
namespace spv {
    #include "GLSL.std.450.h"
    #include "GLSL.ext.KHR.h"
    #include "GLSL.ext.EXT.h"
#ifdef AMD_EXTENSIONS
    #include "GLSL.ext.AMD.h"
#endif
#ifdef NV_EXTENSIONS
    #include "GLSL.ext.NV.h"
#endif
}

#if ENABLE_OPT
    #include "spirv-tools/optimizer.hpp"
    #include "message.h"
#endif

#if ENABLE_OPT
using namespace spvtools;
#endif

// Glslang includes
#include "../glslang/MachineIndependent/localintermediate.h"
#include "../glslang/MachineIndependent/SymbolTable.h"
#include "../glslang/Include/Common.h"
#include "../glslang/Include/revision.h"

#include <fstream>
#include <iomanip>
#include <list>
#include <map>
#include <stack>
#include <string>
#include <vector>

namespace {

namespace {
class SpecConstantOpModeGuard {
public:
    SpecConstantOpModeGuard(spv::Builder* builder)
        : builder_(builder) {
        previous_flag_ = builder->isInSpecConstCodeGenMode();
    }
    ~SpecConstantOpModeGuard() {
        previous_flag_ ? builder_->setToSpecConstCodeGenMode()
                       : builder_->setToNormalCodeGenMode();
    }
    void turnOnSpecConstantOpMode() {
        builder_->setToSpecConstCodeGenMode();
    }

private:
    spv::Builder* builder_;
    bool previous_flag_;
};

struct OpDecorations {
    spv::Decoration precision;
    spv::Decoration noContraction;
    spv::Decoration nonUniform;
};

} // namespace

//
// The main holder of information for translating glslang to SPIR-V.
//
// Derives from the AST walking base class.
//
class TGlslangToSpvTraverser : public glslang::TIntermTraverser {
public:
    TGlslangToSpvTraverser(unsigned int spvVersion, const glslang::TIntermediate*, spv::SpvBuildLogger* logger,
        glslang::SpvOptions& options);
    virtual ~TGlslangToSpvTraverser() { }

    bool visitAggregate(glslang::TVisit, glslang::TIntermAggregate*);
    bool visitBinary(glslang::TVisit, glslang::TIntermBinary*);
    void visitConstantUnion(glslang::TIntermConstantUnion*);
    bool visitSelection(glslang::TVisit, glslang::TIntermSelection*);
    bool visitSwitch(glslang::TVisit, glslang::TIntermSwitch*);
    void visitSymbol(glslang::TIntermSymbol* symbol);
    bool visitUnary(glslang::TVisit, glslang::TIntermUnary*);
    bool visitLoop(glslang::TVisit, glslang::TIntermLoop*);
    bool visitBranch(glslang::TVisit visit, glslang::TIntermBranch*);

    void finishSpv();
    void dumpSpv(std::vector<unsigned int>& out);

protected:
    TGlslangToSpvTraverser(TGlslangToSpvTraverser&);
    TGlslangToSpvTraverser& operator=(TGlslangToSpvTraverser&);

    spv::Decoration TranslateInterpolationDecoration(const glslang::TQualifier& qualifier);
    spv::Decoration TranslateAuxiliaryStorageDecoration(const glslang::TQualifier& qualifier);
    spv::Decoration TranslateNonUniformDecoration(const glslang::TQualifier& qualifier);
    spv::BuiltIn TranslateBuiltInDecoration(glslang::TBuiltInVariable, bool memberDeclaration);
    spv::ImageFormat TranslateImageFormat(const glslang::TType& type);
    spv::SelectionControlMask TranslateSelectionControl(const glslang::TIntermSelection&) const;
    spv::SelectionControlMask TranslateSwitchControl(const glslang::TIntermSwitch&) const;
    spv::LoopControlMask TranslateLoopControl(const glslang::TIntermLoop&, unsigned int& dependencyLength) const;
    spv::StorageClass TranslateStorageClass(const glslang::TType&);
    void addIndirectionIndexCapabilities(const glslang::TType& baseType, const glslang::TType& indexType);
    spv::Id createSpvVariable(const glslang::TIntermSymbol*);
    spv::Id getSampledType(const glslang::TSampler&);
    spv::Id getInvertedSwizzleType(const glslang::TIntermTyped&);
    spv::Id createInvertedSwizzle(spv::Decoration precision, const glslang::TIntermTyped&, spv::Id parentResult);
    void convertSwizzle(const glslang::TIntermAggregate&, std::vector<unsigned>& swizzle);
    spv::Id convertGlslangToSpvType(const glslang::TType& type);
    spv::Id convertGlslangToSpvType(const glslang::TType& type, glslang::TLayoutPacking, const glslang::TQualifier&,
        bool lastBufferBlockMember);
    bool filterMember(const glslang::TType& member);
    spv::Id convertGlslangStructToSpvType(const glslang::TType&, const glslang::TTypeList* glslangStruct,
                                          glslang::TLayoutPacking, const glslang::TQualifier&);
    void decorateStructType(const glslang::TType&, const glslang::TTypeList* glslangStruct, glslang::TLayoutPacking,
                            const glslang::TQualifier&, spv::Id);
    spv::Id makeArraySizeId(const glslang::TArraySizes&, int dim);
    spv::Id accessChainLoad(const glslang::TType& type);
    void    accessChainStore(const glslang::TType& type, spv::Id rvalue);
    void multiTypeStore(const glslang::TType&, spv::Id rValue);
    glslang::TLayoutPacking getExplicitLayout(const glslang::TType& type) const;
    int getArrayStride(const glslang::TType& arrayType, glslang::TLayoutPacking, glslang::TLayoutMatrix);
    int getMatrixStride(const glslang::TType& matrixType, glslang::TLayoutPacking, glslang::TLayoutMatrix);
    void updateMemberOffset(const glslang::TType& structType, const glslang::TType& memberType, int& currentOffset,
                            int& nextOffset, glslang::TLayoutPacking, glslang::TLayoutMatrix);
    void declareUseOfStructMember(const glslang::TTypeList& members, int glslangMember);

    bool isShaderEntryPoint(const glslang::TIntermAggregate* node);
    bool writableParam(glslang::TStorageQualifier) const;
    bool originalParam(glslang::TStorageQualifier, const glslang::TType&, bool implicitThisParam);
    void makeFunctions(const glslang::TIntermSequence&);
    void makeGlobalInitializers(const glslang::TIntermSequence&);
    void visitFunctions(const glslang::TIntermSequence&);
    void handleFunctionEntry(const glslang::TIntermAggregate* node);
    void translateArguments(const glslang::TIntermAggregate& node, std::vector<spv::Id>& arguments);
    void translateArguments(glslang::TIntermUnary& node, std::vector<spv::Id>& arguments);
    spv::Id createImageTextureFunctionCall(glslang::TIntermOperator* node);
    spv::Id handleUserFunctionCall(const glslang::TIntermAggregate*);

    spv::Id createBinaryOperation(glslang::TOperator op, OpDecorations&, spv::Id typeId, spv::Id left, spv::Id right,
                                  glslang::TBasicType typeProxy, bool reduceComparison = true);
    spv::Id createBinaryMatrixOperation(spv::Op, OpDecorations&, spv::Id typeId, spv::Id left, spv::Id right);
    spv::Id createUnaryOperation(glslang::TOperator op, OpDecorations&, spv::Id typeId, spv::Id operand,
                                 glslang::TBasicType typeProxy);
    spv::Id createUnaryMatrixOperation(spv::Op op, OpDecorations&, spv::Id typeId, spv::Id operand,
                                       glslang::TBasicType typeProxy);
    spv::Id createConversion(glslang::TOperator op, OpDecorations&, spv::Id destTypeId, spv::Id operand,
                             glslang::TBasicType typeProxy);
    spv::Id createIntWidthConversion(glslang::TOperator op, spv::Id operand, int vectorSize);
    spv::Id makeSmearedConstant(spv::Id constant, int vectorSize);
    spv::Id createAtomicOperation(glslang::TOperator op, spv::Decoration precision, spv::Id typeId, std::vector<spv::Id>& operands, glslang::TBasicType typeProxy);
    spv::Id createInvocationsOperation(glslang::TOperator op, spv::Id typeId, std::vector<spv::Id>& operands, glslang::TBasicType typeProxy);
    spv::Id CreateInvocationsVectorOperation(spv::Op op, spv::GroupOperation groupOperation, spv::Id typeId, std::vector<spv::Id>& operands);
    spv::Id createSubgroupOperation(glslang::TOperator op, spv::Id typeId, std::vector<spv::Id>& operands, glslang::TBasicType typeProxy);
    spv::Id createMiscOperation(glslang::TOperator op, spv::Decoration precision, spv::Id typeId, std::vector<spv::Id>& operands, glslang::TBasicType typeProxy);
    spv::Id createNoArgOperation(glslang::TOperator op, spv::Decoration precision, spv::Id typeId);
    spv::Id getSymbolId(const glslang::TIntermSymbol* node);
    spv::Id createSpvConstant(const glslang::TIntermTyped&);
    spv::Id createSpvConstantFromConstUnionArray(const glslang::TType& type, const glslang::TConstUnionArray&, int& nextConst, bool specConstant);
    bool isTrivialLeaf(const glslang::TIntermTyped* node);
    bool isTrivial(const glslang::TIntermTyped* node);
    spv::Id createShortCircuit(glslang::TOperator, glslang::TIntermTyped& left, glslang::TIntermTyped& right);
#ifdef AMD_EXTENSIONS
    spv::Id getExtBuiltins(const char* name);
#endif
    void addPre13Extension(const char* ext)
    {
        if (builder.getSpvVersion() < glslang::EShTargetSpv_1_3)
            builder.addExtension(ext);
    }

    glslang::SpvOptions& options;
    spv::Function* shaderEntry;
    spv::Function* currentFunction;
    spv::Instruction* entryPoint;
    int sequenceDepth;

    spv::SpvBuildLogger* logger;

    // There is a 1:1 mapping between a spv builder and a module; this is thread safe
    spv::Builder builder;
    bool inEntryPoint;
    bool entryPointTerminated;
    bool linkageOnly;                  // true when visiting the set of objects in the AST present only for establishing interface, whether or not they were statically used
    std::set<spv::Id> iOSet;           // all input/output variables from either static use or declaration of interface
    const glslang::TIntermediate* glslangIntermediate;
    spv::Id stdBuiltins;
    std::unordered_map<const char*, spv::Id> extBuiltinMap;

    std::unordered_map<int, spv::Id> symbolValues;
    std::unordered_set<int> rValueParameters;  // set of formal function parameters passed as rValues, rather than a pointer
    std::unordered_map<std::string, spv::Function*> functionMap;
    std::unordered_map<const glslang::TTypeList*, spv::Id> structMap[glslang::ElpCount][glslang::ElmCount];
    // for mapping glslang block indices to spv indices (e.g., due to hidden members):
    std::unordered_map<const glslang::TTypeList*, std::vector<int> > memberRemapper;
    std::stack<bool> breakForLoop;  // false means break for switch
    std::unordered_map<std::string, const glslang::TIntermSymbol*> counterOriginator;
};

//
// Helper functions for translating glslang representations to SPIR-V enumerants.
//

// Translate glslang profile to SPIR-V source language.
spv::SourceLanguage TranslateSourceLanguage(glslang::EShSource source, EProfile profile)
{
    switch (source) {
    case glslang::EShSourceGlsl:
        switch (profile) {
        case ENoProfile:
        case ECoreProfile:
        case ECompatibilityProfile:
            return spv::SourceLanguageGLSL;
        case EEsProfile:
            return spv::SourceLanguageESSL;
        default:
            return spv::SourceLanguageUnknown;
        }
    case glslang::EShSourceHlsl:
        return spv::SourceLanguageHLSL;
    default:
        return spv::SourceLanguageUnknown;
    }
}

// Translate glslang language (stage) to SPIR-V execution model.
spv::ExecutionModel TranslateExecutionModel(EShLanguage stage)
{
    switch (stage) {
    case EShLangVertex:           return spv::ExecutionModelVertex;
    case EShLangTessControl:      return spv::ExecutionModelTessellationControl;
    case EShLangTessEvaluation:   return spv::ExecutionModelTessellationEvaluation;
    case EShLangGeometry:         return spv::ExecutionModelGeometry;
    case EShLangFragment:         return spv::ExecutionModelFragment;
    case EShLangCompute:          return spv::ExecutionModelGLCompute;
    default:
        assert(0);
        return spv::ExecutionModelFragment;
    }
}

// Translate glslang sampler type to SPIR-V dimensionality.
spv::Dim TranslateDimensionality(const glslang::TSampler& sampler)
{
    switch (sampler.dim) {
    case glslang::Esd1D:      return spv::Dim1D;
    case glslang::Esd2D:      return spv::Dim2D;
    case glslang::Esd3D:      return spv::Dim3D;
    case glslang::EsdCube:    return spv::DimCube;
    case glslang::EsdRect:    return spv::DimRect;
    case glslang::EsdBuffer:  return spv::DimBuffer;
    case glslang::EsdSubpass: return spv::DimSubpassData;
    default:
        assert(0);
        return spv::Dim2D;
    }
}

// Translate glslang precision to SPIR-V precision decorations.
spv::Decoration TranslatePrecisionDecoration(glslang::TPrecisionQualifier glslangPrecision)
{
    switch (glslangPrecision) {
    case glslang::EpqLow:    return spv::DecorationRelaxedPrecision;
    case glslang::EpqMedium: return spv::DecorationRelaxedPrecision;
    default:
        return spv::NoPrecision;
    }
}

// Translate glslang type to SPIR-V precision decorations.
spv::Decoration TranslatePrecisionDecoration(const glslang::TType& type)
{
    return TranslatePrecisionDecoration(type.getQualifier().precision);
}

// Translate glslang type to SPIR-V block decorations.
spv::Decoration TranslateBlockDecoration(const glslang::TType& type, bool useStorageBuffer)
{
    if (type.getBasicType() == glslang::EbtBlock) {
        switch (type.getQualifier().storage) {
        case glslang::EvqUniform:      return spv::DecorationBlock;
        case glslang::EvqBuffer:       return useStorageBuffer ? spv::DecorationBlock : spv::DecorationBufferBlock;
        case glslang::EvqVaryingIn:    return spv::DecorationBlock;
        case glslang::EvqVaryingOut:   return spv::DecorationBlock;
        default:
            assert(0);
            break;
        }
    }

    return spv::DecorationMax;
}

// Translate glslang type to SPIR-V memory decorations.
void TranslateMemoryDecoration(const glslang::TQualifier& qualifier, std::vector<spv::Decoration>& memory)
{
    if (qualifier.coherent)
        memory.push_back(spv::DecorationCoherent);
    if (qualifier.volatil) {
        memory.push_back(spv::DecorationVolatile);
        memory.push_back(spv::DecorationCoherent);
    }
    if (qualifier.restrict)
        memory.push_back(spv::DecorationRestrict);
    if (qualifier.readonly)
        memory.push_back(spv::DecorationNonWritable);
    if (qualifier.writeonly)
       memory.push_back(spv::DecorationNonReadable);
}

// Translate glslang type to SPIR-V layout decorations.
spv::Decoration TranslateLayoutDecoration(const glslang::TType& type, glslang::TLayoutMatrix matrixLayout)
{
    if (type.isMatrix()) {
        switch (matrixLayout) {
        case glslang::ElmRowMajor:
            return spv::DecorationRowMajor;
        case glslang::ElmColumnMajor:
            return spv::DecorationColMajor;
        default:
            // opaque layouts don't need a majorness
            return spv::DecorationMax;
        }
    } else {
        switch (type.getBasicType()) {
        default:
            return spv::DecorationMax;
            break;
        case glslang::EbtBlock:
            switch (type.getQualifier().storage) {
            case glslang::EvqUniform:
            case glslang::EvqBuffer:
                switch (type.getQualifier().layoutPacking) {
                case glslang::ElpShared:  return spv::DecorationGLSLShared;
                case glslang::ElpPacked:  return spv::DecorationGLSLPacked;
                default:
                    return spv::DecorationMax;
                }
            case glslang::EvqVaryingIn:
            case glslang::EvqVaryingOut:
                assert(type.getQualifier().layoutPacking == glslang::ElpNone);
                return spv::DecorationMax;
            default:
                assert(0);
                return spv::DecorationMax;
            }
        }
    }
}

// Translate glslang type to SPIR-V interpolation decorations.
// Returns spv::DecorationMax when no decoration
// should be applied.
spv::Decoration TGlslangToSpvTraverser::TranslateInterpolationDecoration(const glslang::TQualifier& qualifier)
{
    if (qualifier.smooth)
        // Smooth decoration doesn't exist in SPIR-V 1.0
        return spv::DecorationMax;
    else if (qualifier.nopersp)
        return spv::DecorationNoPerspective;
    else if (qualifier.flat)
        return spv::DecorationFlat;
#ifdef AMD_EXTENSIONS
    else if (qualifier.explicitInterp) {
        builder.addExtension(spv::E_SPV_AMD_shader_explicit_vertex_parameter);
        return spv::DecorationExplicitInterpAMD;
    }
#endif
    else
        return spv::DecorationMax;
}

// Translate glslang type to SPIR-V auxiliary storage decorations.
// Returns spv::DecorationMax when no decoration
// should be applied.
spv::Decoration TGlslangToSpvTraverser::TranslateAuxiliaryStorageDecoration(const glslang::TQualifier& qualifier)
{
    if (qualifier.patch)
        return spv::DecorationPatch;
    else if (qualifier.centroid)
        return spv::DecorationCentroid;
    else if (qualifier.sample) {
        builder.addCapability(spv::CapabilitySampleRateShading);
        return spv::DecorationSample;
    } else
        return spv::DecorationMax;
}

// If glslang type is invariant, return SPIR-V invariant decoration.
spv::Decoration TranslateInvariantDecoration(const glslang::TQualifier& qualifier)
{
    if (qualifier.invariant)
        return spv::DecorationInvariant;
    else
        return spv::DecorationMax;
}

// If glslang type is noContraction, return SPIR-V NoContraction decoration.
spv::Decoration TranslateNoContractionDecoration(const glslang::TQualifier& qualifier)
{
    if (qualifier.noContraction)
        return spv::DecorationNoContraction;
    else
        return spv::DecorationMax;
}

// If glslang type is nonUniform, return SPIR-V NonUniform decoration.
spv::Decoration TGlslangToSpvTraverser::TranslateNonUniformDecoration(const glslang::TQualifier& qualifier)
{
    if (qualifier.isNonUniform()) {
        builder.addExtension("SPV_EXT_descriptor_indexing");
        builder.addCapability(spv::CapabilityShaderNonUniformEXT);
        return spv::DecorationNonUniformEXT;
    } else
        return spv::DecorationMax;
}

// Translate a glslang built-in variable to a SPIR-V built in decoration.  Also generate
// associated capabilities when required.  For some built-in variables, a capability
// is generated only when using the variable in an executable instruction, but not when
// just declaring a struct member variable with it.  This is true for PointSize,
// ClipDistance, and CullDistance.
spv::BuiltIn TGlslangToSpvTraverser::TranslateBuiltInDecoration(glslang::TBuiltInVariable builtIn, bool memberDeclaration)
{
    switch (builtIn) {
    case glslang::EbvPointSize:
        // Defer adding the capability until the built-in is actually used.
        if (! memberDeclaration) {
            switch (glslangIntermediate->getStage()) {
            case EShLangGeometry:
                builder.addCapability(spv::CapabilityGeometryPointSize);
                break;
            case EShLangTessControl:
            case EShLangTessEvaluation:
                builder.addCapability(spv::CapabilityTessellationPointSize);
                break;
            default:
                break;
            }
        }
        return spv::BuiltInPointSize;

    // These *Distance capabilities logically belong here, but if the member is declared and
    // then never used, consumers of SPIR-V prefer the capability not be declared.
    // They are now generated when used, rather than here when declared.
    // Potentially, the specification should be more clear what the minimum
    // use needed is to trigger the capability.
    //
    case glslang::EbvClipDistance:
        if (!memberDeclaration)
            builder.addCapability(spv::CapabilityClipDistance);
        return spv::BuiltInClipDistance;

    case glslang::EbvCullDistance:
        if (!memberDeclaration)
            builder.addCapability(spv::CapabilityCullDistance);
        return spv::BuiltInCullDistance;

    case glslang::EbvViewportIndex:
        builder.addCapability(spv::CapabilityMultiViewport);
        if (glslangIntermediate->getStage() == EShLangVertex ||
            glslangIntermediate->getStage() == EShLangTessControl ||
            glslangIntermediate->getStage() == EShLangTessEvaluation) {

            builder.addExtension(spv::E_SPV_EXT_shader_viewport_index_layer);
            builder.addCapability(spv::CapabilityShaderViewportIndexLayerEXT);
        }
        return spv::BuiltInViewportIndex;

    case glslang::EbvSampleId:
        builder.addCapability(spv::CapabilitySampleRateShading);
        return spv::BuiltInSampleId;

    case glslang::EbvSamplePosition:
        builder.addCapability(spv::CapabilitySampleRateShading);
        return spv::BuiltInSamplePosition;

    case glslang::EbvSampleMask:
        return spv::BuiltInSampleMask;

    case glslang::EbvLayer:
        builder.addCapability(spv::CapabilityGeometry);
        if (glslangIntermediate->getStage() == EShLangVertex ||
            glslangIntermediate->getStage() == EShLangTessControl ||
            glslangIntermediate->getStage() == EShLangTessEvaluation) {

            builder.addExtension(spv::E_SPV_EXT_shader_viewport_index_layer);
            builder.addCapability(spv::CapabilityShaderViewportIndexLayerEXT);
        }
        return spv::BuiltInLayer;

    case glslang::EbvPosition:             return spv::BuiltInPosition;
    case glslang::EbvVertexId:             return spv::BuiltInVertexId;
    case glslang::EbvInstanceId:           return spv::BuiltInInstanceId;
    case glslang::EbvVertexIndex:          return spv::BuiltInVertexIndex;
    case glslang::EbvInstanceIndex:        return spv::BuiltInInstanceIndex;

    case glslang::EbvBaseVertex:
        addPre13Extension(spv::E_SPV_KHR_shader_draw_parameters);
        builder.addCapability(spv::CapabilityDrawParameters);
        return spv::BuiltInBaseVertex;

    case glslang::EbvBaseInstance:
        addPre13Extension(spv::E_SPV_KHR_shader_draw_parameters);
        builder.addCapability(spv::CapabilityDrawParameters);
        return spv::BuiltInBaseInstance;

    case glslang::EbvDrawId:
        addPre13Extension(spv::E_SPV_KHR_shader_draw_parameters);
        builder.addCapability(spv::CapabilityDrawParameters);
        return spv::BuiltInDrawIndex;

    case glslang::EbvPrimitiveId:
        if (glslangIntermediate->getStage() == EShLangFragment)
            builder.addCapability(spv::CapabilityGeometry);
        return spv::BuiltInPrimitiveId;

    case glslang::EbvFragStencilRef:
        builder.addExtension(spv::E_SPV_EXT_shader_stencil_export);
        builder.addCapability(spv::CapabilityStencilExportEXT);
        return spv::BuiltInFragStencilRefEXT;

    case glslang::EbvInvocationId:         return spv::BuiltInInvocationId;
    case glslang::EbvTessLevelInner:       return spv::BuiltInTessLevelInner;
    case glslang::EbvTessLevelOuter:       return spv::BuiltInTessLevelOuter;
    case glslang::EbvTessCoord:            return spv::BuiltInTessCoord;
    case glslang::EbvPatchVertices:        return spv::BuiltInPatchVertices;
    case glslang::EbvFragCoord:            return spv::BuiltInFragCoord;
    case glslang::EbvPointCoord:           return spv::BuiltInPointCoord;
    case glslang::EbvFace:                 return spv::BuiltInFrontFacing;
    case glslang::EbvFragDepth:            return spv::BuiltInFragDepth;
    case glslang::EbvHelperInvocation:     return spv::BuiltInHelperInvocation;
    case glslang::EbvNumWorkGroups:        return spv::BuiltInNumWorkgroups;
    case glslang::EbvWorkGroupSize:        return spv::BuiltInWorkgroupSize;
    case glslang::EbvWorkGroupId:          return spv::BuiltInWorkgroupId;
    case glslang::EbvLocalInvocationId:    return spv::BuiltInLocalInvocationId;
    case glslang::EbvLocalInvocationIndex: return spv::BuiltInLocalInvocationIndex;
    case glslang::EbvGlobalInvocationId:   return spv::BuiltInGlobalInvocationId;

    case glslang::EbvSubGroupSize:
        builder.addExtension(spv::E_SPV_KHR_shader_ballot);
        builder.addCapability(spv::CapabilitySubgroupBallotKHR);
        return spv::BuiltInSubgroupSize;

    case glslang::EbvSubGroupInvocation:
        builder.addExtension(spv::E_SPV_KHR_shader_ballot);
        builder.addCapability(spv::CapabilitySubgroupBallotKHR);
        return spv::BuiltInSubgroupLocalInvocationId;

    case glslang::EbvSubGroupEqMask:
        builder.addExtension(spv::E_SPV_KHR_shader_ballot);
        builder.addCapability(spv::CapabilitySubgroupBallotKHR);
        return spv::BuiltInSubgroupEqMaskKHR;

    case glslang::EbvSubGroupGeMask:
        builder.addExtension(spv::E_SPV_KHR_shader_ballot);
        builder.addCapability(spv::CapabilitySubgroupBallotKHR);
        return spv::BuiltInSubgroupGeMaskKHR;

    case glslang::EbvSubGroupGtMask:
        builder.addExtension(spv::E_SPV_KHR_shader_ballot);
        builder.addCapability(spv::CapabilitySubgroupBallotKHR);
        return spv::BuiltInSubgroupGtMaskKHR;

    case glslang::EbvSubGroupLeMask:
        builder.addExtension(spv::E_SPV_KHR_shader_ballot);
        builder.addCapability(spv::CapabilitySubgroupBallotKHR);
        return spv::BuiltInSubgroupLeMaskKHR;

    case glslang::EbvSubGroupLtMask:
        builder.addExtension(spv::E_SPV_KHR_shader_ballot);
        builder.addCapability(spv::CapabilitySubgroupBallotKHR);
        return spv::BuiltInSubgroupLtMaskKHR;

    case glslang::EbvNumSubgroups:
        builder.addCapability(spv::CapabilityGroupNonUniform);
        return spv::BuiltInNumSubgroups;

    case glslang::EbvSubgroupID:
        builder.addCapability(spv::CapabilityGroupNonUniform);
        return spv::BuiltInSubgroupId;

    case glslang::EbvSubgroupSize2:
        builder.addCapability(spv::CapabilityGroupNonUniform);
        return spv::BuiltInSubgroupSize;

    case glslang::EbvSubgroupInvocation2:
        builder.addCapability(spv::CapabilityGroupNonUniform);
        return spv::BuiltInSubgroupLocalInvocationId;

    case glslang::EbvSubgroupEqMask2:
        builder.addCapability(spv::CapabilityGroupNonUniform);
        builder.addCapability(spv::CapabilityGroupNonUniformBallot);
        return spv::BuiltInSubgroupEqMask;

    case glslang::EbvSubgroupGeMask2:
        builder.addCapability(spv::CapabilityGroupNonUniform);
        builder.addCapability(spv::CapabilityGroupNonUniformBallot);
        return spv::BuiltInSubgroupGeMask;

    case glslang::EbvSubgroupGtMask2:
        builder.addCapability(spv::CapabilityGroupNonUniform);
        builder.addCapability(spv::CapabilityGroupNonUniformBallot);
        return spv::BuiltInSubgroupGtMask;

    case glslang::EbvSubgroupLeMask2:
        builder.addCapability(spv::CapabilityGroupNonUniform);
        builder.addCapability(spv::CapabilityGroupNonUniformBallot);
        return spv::BuiltInSubgroupLeMask;

    case glslang::EbvSubgroupLtMask2:
        builder.addCapability(spv::CapabilityGroupNonUniform);
        builder.addCapability(spv::CapabilityGroupNonUniformBallot);
        return spv::BuiltInSubgroupLtMask;
#ifdef AMD_EXTENSIONS
    case glslang::EbvBaryCoordNoPersp:
        builder.addExtension(spv::E_SPV_AMD_shader_explicit_vertex_parameter);
        return spv::BuiltInBaryCoordNoPerspAMD;

    case glslang::EbvBaryCoordNoPerspCentroid:
        builder.addExtension(spv::E_SPV_AMD_shader_explicit_vertex_parameter);
        return spv::BuiltInBaryCoordNoPerspCentroidAMD;

    case glslang::EbvBaryCoordNoPerspSample:
        builder.addExtension(spv::E_SPV_AMD_shader_explicit_vertex_parameter);
        return spv::BuiltInBaryCoordNoPerspSampleAMD;

    case glslang::EbvBaryCoordSmooth:
        builder.addExtension(spv::E_SPV_AMD_shader_explicit_vertex_parameter);
        return spv::BuiltInBaryCoordSmoothAMD;

    case glslang::EbvBaryCoordSmoothCentroid:
        builder.addExtension(spv::E_SPV_AMD_shader_explicit_vertex_parameter);
        return spv::BuiltInBaryCoordSmoothCentroidAMD;

    case glslang::EbvBaryCoordSmoothSample:
        builder.addExtension(spv::E_SPV_AMD_shader_explicit_vertex_parameter);
        return spv::BuiltInBaryCoordSmoothSampleAMD;

    case glslang::EbvBaryCoordPullModel:
        builder.addExtension(spv::E_SPV_AMD_shader_explicit_vertex_parameter);
        return spv::BuiltInBaryCoordPullModelAMD;
#endif

    case glslang::EbvDeviceIndex:
        addPre13Extension(spv::E_SPV_KHR_device_group);
        builder.addCapability(spv::CapabilityDeviceGroup);
        return spv::BuiltInDeviceIndex;

    case glslang::EbvViewIndex:
        addPre13Extension(spv::E_SPV_KHR_multiview);
        builder.addCapability(spv::CapabilityMultiView);
        return spv::BuiltInViewIndex;

#ifdef NV_EXTENSIONS
    case glslang::EbvViewportMaskNV:
        if (!memberDeclaration) {
            builder.addExtension(spv::E_SPV_NV_viewport_array2);
            builder.addCapability(spv::CapabilityShaderViewportMaskNV);
        }
        return spv::BuiltInViewportMaskNV;
    case glslang::EbvSecondaryPositionNV:
        if (!memberDeclaration) {
            builder.addExtension(spv::E_SPV_NV_stereo_view_rendering);
            builder.addCapability(spv::CapabilityShaderStereoViewNV);
        }
        return spv::BuiltInSecondaryPositionNV;
    case glslang::EbvSecondaryViewportMaskNV:
        if (!memberDeclaration) {
            builder.addExtension(spv::E_SPV_NV_stereo_view_rendering);
            builder.addCapability(spv::CapabilityShaderStereoViewNV);
        }
        return spv::BuiltInSecondaryViewportMaskNV;
    case glslang::EbvPositionPerViewNV:
        if (!memberDeclaration) {
            builder.addExtension(spv::E_SPV_NVX_multiview_per_view_attributes);
            builder.addCapability(spv::CapabilityPerViewAttributesNV);
        }
        return spv::BuiltInPositionPerViewNV;
    case glslang::EbvViewportMaskPerViewNV:
        if (!memberDeclaration) {
            builder.addExtension(spv::E_SPV_NVX_multiview_per_view_attributes);
            builder.addCapability(spv::CapabilityPerViewAttributesNV);
        }
        return spv::BuiltInViewportMaskPerViewNV;
    case glslang::EbvFragFullyCoveredNV:
        builder.addExtension(spv::E_SPV_EXT_fragment_fully_covered);
        builder.addCapability(spv::CapabilityFragmentFullyCoveredEXT);
        return spv::BuiltInFullyCoveredEXT;
#endif 
    default:
        return spv::BuiltInMax;
    }
}

// Translate glslang image layout format to SPIR-V image format.
spv::ImageFormat TGlslangToSpvTraverser::TranslateImageFormat(const glslang::TType& type)
{
    assert(type.getBasicType() == glslang::EbtSampler);

    // Check for capabilities
    switch (type.getQualifier().layoutFormat) {
    case glslang::ElfRg32f:
    case glslang::ElfRg16f:
    case glslang::ElfR11fG11fB10f:
    case glslang::ElfR16f:
    case glslang::ElfRgba16:
    case glslang::ElfRgb10A2:
    case glslang::ElfRg16:
    case glslang::ElfRg8:
    case glslang::ElfR16:
    case glslang::ElfR8:
    case glslang::ElfRgba16Snorm:
    case glslang::ElfRg16Snorm:
    case glslang::ElfRg8Snorm:
    case glslang::ElfR16Snorm:
    case glslang::ElfR8Snorm:

    case glslang::ElfRg32i:
    case glslang::ElfRg16i:
    case glslang::ElfRg8i:
    case glslang::ElfR16i:
    case glslang::ElfR8i:

    case glslang::ElfRgb10a2ui:
    case glslang::ElfRg32ui:
    case glslang::ElfRg16ui:
    case glslang::ElfRg8ui:
    case glslang::ElfR16ui:
    case glslang::ElfR8ui:
        builder.addCapability(spv::CapabilityStorageImageExtendedFormats);
        break;

    default:
        break;
    }

    // do the translation
    switch (type.getQualifier().layoutFormat) {
    case glslang::ElfNone:          return spv::ImageFormatUnknown;
    case glslang::ElfRgba32f:       return spv::ImageFormatRgba32f;
    case glslang::ElfRgba16f:       return spv::ImageFormatRgba16f;
    case glslang::ElfR32f:          return spv::ImageFormatR32f;
    case glslang::ElfRgba8:         return spv::ImageFormatRgba8;
    case glslang::ElfRgba8Snorm:    return spv::ImageFormatRgba8Snorm;
    case glslang::ElfRg32f:         return spv::ImageFormatRg32f;
    case glslang::ElfRg16f:         return spv::ImageFormatRg16f;
    case glslang::ElfR11fG11fB10f:  return spv::ImageFormatR11fG11fB10f;
    case glslang::ElfR16f:          return spv::ImageFormatR16f;
    case glslang::ElfRgba16:        return spv::ImageFormatRgba16;
    case glslang::ElfRgb10A2:       return spv::ImageFormatRgb10A2;
    case glslang::ElfRg16:          return spv::ImageFormatRg16;
    case glslang::ElfRg8:           return spv::ImageFormatRg8;
    case glslang::ElfR16:           return spv::ImageFormatR16;
    case glslang::ElfR8:            return spv::ImageFormatR8;
    case glslang::ElfRgba16Snorm:   return spv::ImageFormatRgba16Snorm;
    case glslang::ElfRg16Snorm:     return spv::ImageFormatRg16Snorm;
    case glslang::ElfRg8Snorm:      return spv::ImageFormatRg8Snorm;
    case glslang::ElfR16Snorm:      return spv::ImageFormatR16Snorm;
    case glslang::ElfR8Snorm:       return spv::ImageFormatR8Snorm;
    case glslang::ElfRgba32i:       return spv::ImageFormatRgba32i;
    case glslang::ElfRgba16i:       return spv::ImageFormatRgba16i;
    case glslang::ElfRgba8i:        return spv::ImageFormatRgba8i;
    case glslang::ElfR32i:          return spv::ImageFormatR32i;
    case glslang::ElfRg32i:         return spv::ImageFormatRg32i;
    case glslang::ElfRg16i:         return spv::ImageFormatRg16i;
    case glslang::ElfRg8i:          return spv::ImageFormatRg8i;
    case glslang::ElfR16i:          return spv::ImageFormatR16i;
    case glslang::ElfR8i:           return spv::ImageFormatR8i;
    case glslang::ElfRgba32ui:      return spv::ImageFormatRgba32ui;
    case glslang::ElfRgba16ui:      return spv::ImageFormatRgba16ui;
    case glslang::ElfRgba8ui:       return spv::ImageFormatRgba8ui;
    case glslang::ElfR32ui:         return spv::ImageFormatR32ui;
    case glslang::ElfRg32ui:        return spv::ImageFormatRg32ui;
    case glslang::ElfRg16ui:        return spv::ImageFormatRg16ui;
    case glslang::ElfRgb10a2ui:     return spv::ImageFormatRgb10a2ui;
    case glslang::ElfRg8ui:         return spv::ImageFormatRg8ui;
    case glslang::ElfR16ui:         return spv::ImageFormatR16ui;
    case glslang::ElfR8ui:          return spv::ImageFormatR8ui;
    default:                        return spv::ImageFormatMax;
    }
}

spv::SelectionControlMask TGlslangToSpvTraverser::TranslateSelectionControl(const glslang::TIntermSelection& selectionNode) const
{
    if (selectionNode.getFlatten())
        return spv::SelectionControlFlattenMask;
    if (selectionNode.getDontFlatten())
        return spv::SelectionControlDontFlattenMask;
    return spv::SelectionControlMaskNone;
}

spv::SelectionControlMask TGlslangToSpvTraverser::TranslateSwitchControl(const glslang::TIntermSwitch& switchNode) const
{
    if (switchNode.getFlatten())
        return spv::SelectionControlFlattenMask;
    if (switchNode.getDontFlatten())
        return spv::SelectionControlDontFlattenMask;
    return spv::SelectionControlMaskNone;
}

// return a non-0 dependency if the dependency argument must be set
spv::LoopControlMask TGlslangToSpvTraverser::TranslateLoopControl(const glslang::TIntermLoop& loopNode,
    unsigned int& dependencyLength) const
{
    spv::LoopControlMask control = spv::LoopControlMaskNone;

    if (loopNode.getDontUnroll())
        control = control | spv::LoopControlDontUnrollMask;
    if (loopNode.getUnroll())
        control = control | spv::LoopControlUnrollMask;
    if (unsigned(loopNode.getLoopDependency()) == glslang::TIntermLoop::dependencyInfinite)
        control = control | spv::LoopControlDependencyInfiniteMask;
    else if (loopNode.getLoopDependency() > 0) {
        control = control | spv::LoopControlDependencyLengthMask;
        dependencyLength = loopNode.getLoopDependency();
    }

    return control;
}

// Translate glslang type to SPIR-V storage class.
spv::StorageClass TGlslangToSpvTraverser::TranslateStorageClass(const glslang::TType& type)
{
    if (type.getQualifier().isPipeInput())
        return spv::StorageClassInput;
    if (type.getQualifier().isPipeOutput())
        return spv::StorageClassOutput;

    if (glslangIntermediate->getSource() != glslang::EShSourceHlsl ||
        type.getQualifier().storage == glslang::EvqUniform) {
        if (type.getBasicType() == glslang::EbtAtomicUint)
            return spv::StorageClassAtomicCounter;
        if (type.containsOpaque())
            return spv::StorageClassUniformConstant;
    }

    if (glslangIntermediate->usingStorageBuffer() && type.getQualifier().storage == glslang::EvqBuffer) {
        addPre13Extension(spv::E_SPV_KHR_storage_buffer_storage_class);
        return spv::StorageClassStorageBuffer;
    }

    if (type.getQualifier().isUniformOrBuffer()) {
        if (type.getQualifier().layoutPushConstant)
            return spv::StorageClassPushConstant;
        if (type.getBasicType() == glslang::EbtBlock)
            return spv::StorageClassUniform;
        return spv::StorageClassUniformConstant;
    }

    switch (type.getQualifier().storage) {
    case glslang::EvqShared:        return spv::StorageClassWorkgroup;
    case glslang::EvqGlobal:        return spv::StorageClassPrivate;
    case glslang::EvqConstReadOnly: return spv::StorageClassFunction;
    case glslang::EvqTemporary:     return spv::StorageClassFunction;
    default:
        assert(0);
        break;
    }

    return spv::StorageClassFunction;
}

// Add capabilities pertaining to how an array is indexed.
void TGlslangToSpvTraverser::addIndirectionIndexCapabilities(const glslang::TType& baseType,
                                                             const glslang::TType& indexType)
{
    if (indexType.getQualifier().isNonUniform()) {
        // deal with an asserted non-uniform index
        // SPV_EXT_descriptor_indexing already added in TranslateNonUniformDecoration
        if (baseType.getBasicType() == glslang::EbtSampler) {
            if (baseType.getQualifier().hasAttachment())
                builder.addCapability(spv::CapabilityInputAttachmentArrayNonUniformIndexingEXT);
            else if (baseType.isImage() && baseType.getSampler().dim == glslang::EsdBuffer)
                builder.addCapability(spv::CapabilityStorageTexelBufferArrayNonUniformIndexingEXT);
            else if (baseType.isTexture() && baseType.getSampler().dim == glslang::EsdBuffer)
                builder.addCapability(spv::CapabilityUniformTexelBufferArrayNonUniformIndexingEXT);
            else if (baseType.isImage())
                builder.addCapability(spv::CapabilityStorageImageArrayNonUniformIndexingEXT);
            else if (baseType.isTexture())
                builder.addCapability(spv::CapabilitySampledImageArrayNonUniformIndexingEXT);
        } else if (baseType.getBasicType() == glslang::EbtBlock) {
            if (baseType.getQualifier().storage == glslang::EvqBuffer)
                builder.addCapability(spv::CapabilityStorageBufferArrayNonUniformIndexingEXT);
            else if (baseType.getQualifier().storage == glslang::EvqUniform)
                builder.addCapability(spv::CapabilityUniformBufferArrayNonUniformIndexingEXT);
        }
    } else {
        // assume a dynamically uniform index
        if (baseType.getBasicType() == glslang::EbtSampler) {
            if (baseType.getQualifier().hasAttachment()) {
                builder.addExtension("SPV_EXT_descriptor_indexing");
                builder.addCapability(spv::CapabilityInputAttachmentArrayDynamicIndexingEXT);
            } else if (baseType.isImage() && baseType.getSampler().dim == glslang::EsdBuffer) {
                builder.addExtension("SPV_EXT_descriptor_indexing");
                builder.addCapability(spv::CapabilityStorageTexelBufferArrayDynamicIndexingEXT);
            } else if (baseType.isTexture() && baseType.getSampler().dim == glslang::EsdBuffer) {
                builder.addExtension("SPV_EXT_descriptor_indexing");
                builder.addCapability(spv::CapabilityUniformTexelBufferArrayDynamicIndexingEXT);
            }
        }
    }
}

// Return whether or not the given type is something that should be tied to a
// descriptor set.
bool IsDescriptorResource(const glslang::TType& type)
{
    // uniform and buffer blocks are included, unless it is a push_constant
    if (type.getBasicType() == glslang::EbtBlock)
        return type.getQualifier().isUniformOrBuffer() && ! type.getQualifier().layoutPushConstant;

    // non block...
    // basically samplerXXX/subpass/sampler/texture are all included
    // if they are the global-scope-class, not the function parameter
    // (or local, if they ever exist) class.
    if (type.getBasicType() == glslang::EbtSampler)
        return type.getQualifier().isUniformOrBuffer();

    // None of the above.
    return false;
}

void InheritQualifiers(glslang::TQualifier& child, const glslang::TQualifier& parent)
{
    if (child.layoutMatrix == glslang::ElmNone)
        child.layoutMatrix = parent.layoutMatrix;

    if (parent.invariant)
        child.invariant = true;
    if (parent.nopersp)
        child.nopersp = true;
#ifdef AMD_EXTENSIONS
    if (parent.explicitInterp)
        child.explicitInterp = true;
#endif
    if (parent.flat)
        child.flat = true;
    if (parent.centroid)
        child.centroid = true;
    if (parent.patch)
        child.patch = true;
    if (parent.sample)
        child.sample = true;
    if (parent.coherent)
        child.coherent = true;
    if (parent.volatil)
        child.volatil = true;
    if (parent.restrict)
        child.restrict = true;
    if (parent.readonly)
        child.readonly = true;
    if (parent.writeonly)
        child.writeonly = true;
}

bool HasNonLayoutQualifiers(const glslang::TType& type, const glslang::TQualifier& qualifier)
{
    // This should list qualifiers that simultaneous satisfy:
    // - struct members might inherit from a struct declaration
    //     (note that non-block structs don't explicitly inherit,
    //      only implicitly, meaning no decoration involved)
    // - affect decorations on the struct members
    //     (note smooth does not, and expecting something like volatile
    //      to effect the whole object)
    // - are not part of the offset/st430/etc or row/column-major layout
    return qualifier.invariant || (qualifier.hasLocation() && type.getBasicType() == glslang::EbtBlock);
}

//
// Implement the TGlslangToSpvTraverser class.
//

TGlslangToSpvTraverser::TGlslangToSpvTraverser(unsigned int spvVersion, const glslang::TIntermediate* glslangIntermediate,
                                               spv::SpvBuildLogger* buildLogger, glslang::SpvOptions& options)
    : TIntermTraverser(true, false, true),
      options(options),
      shaderEntry(nullptr), currentFunction(nullptr),
      sequenceDepth(0), logger(buildLogger),
      builder(spvVersion, (glslang::GetKhronosToolId() << 16) | glslang::GetSpirvGeneratorVersion(), logger),
      inEntryPoint(false), entryPointTerminated(false), linkageOnly(false),
      glslangIntermediate(glslangIntermediate)
{
    spv::ExecutionModel executionModel = TranslateExecutionModel(glslangIntermediate->getStage());

    builder.clearAccessChain();
    builder.setSource(TranslateSourceLanguage(glslangIntermediate->getSource(), glslangIntermediate->getProfile()),
                      glslangIntermediate->getVersion());

    if (options.generateDebugInfo) {
        builder.setEmitOpLines();
        builder.setSourceFile(glslangIntermediate->getSourceFile());

        // Set the source shader's text. If for SPV version 1.0, include
        // a preamble in comments stating the OpModuleProcessed instructions.
        // Otherwise, emit those as actual instructions.
        std::string text;
        const std::vector<std::string>& processes = glslangIntermediate->getProcesses();
        for (int p = 0; p < (int)processes.size(); ++p) {
            if (glslangIntermediate->getSpv().spv < 0x00010100) {
                text.append("// OpModuleProcessed ");
                text.append(processes[p]);
                text.append("\n");
            } else
                builder.addModuleProcessed(processes[p]);
        }
        if (glslangIntermediate->getSpv().spv < 0x00010100 && (int)processes.size() > 0)
            text.append("#line 1\n");
        text.append(glslangIntermediate->getSourceText());
        builder.setSourceText(text);
    }
    stdBuiltins = builder.import("GLSL.std.450");
    builder.setMemoryModel(spv::AddressingModelLogical, spv::MemoryModelGLSL450);
    shaderEntry = builder.makeEntryPoint(glslangIntermediate->getEntryPointName().c_str());
    entryPoint = builder.addEntryPoint(executionModel, shaderEntry, glslangIntermediate->getEntryPointName().c_str());

    // Add the source extensions
    const auto& sourceExtensions = glslangIntermediate->getRequestedExtensions();
    for (auto it = sourceExtensions.begin(); it != sourceExtensions.end(); ++it)
        builder.addSourceExtension(it->c_str());

    // Add the top-level modes for this shader.

    if (glslangIntermediate->getXfbMode()) {
        builder.addCapability(spv::CapabilityTransformFeedback);
        builder.addExecutionMode(shaderEntry, spv::ExecutionModeXfb);
    }

    unsigned int mode;
    switch (glslangIntermediate->getStage()) {
    case EShLangVertex:
        builder.addCapability(spv::CapabilityShader);
        break;

    case EShLangTessEvaluation:
    case EShLangTessControl:
        builder.addCapability(spv::CapabilityTessellation);

        glslang::TLayoutGeometry primitive;

        if (glslangIntermediate->getStage() == EShLangTessControl) {
            builder.addExecutionMode(shaderEntry, spv::ExecutionModeOutputVertices, glslangIntermediate->getVertices());
            primitive = glslangIntermediate->getOutputPrimitive();
        } else {
            primitive = glslangIntermediate->getInputPrimitive();
        }

        switch (primitive) {
        case glslang::ElgTriangles:           mode = spv::ExecutionModeTriangles;     break;
        case glslang::ElgQuads:               mode = spv::ExecutionModeQuads;         break;
        case glslang::ElgIsolines:            mode = spv::ExecutionModeIsolines;      break;
        default:                              mode = spv::ExecutionModeMax;           break;
        }
        if (mode != spv::ExecutionModeMax)
            builder.addExecutionMode(shaderEntry, (spv::ExecutionMode)mode);

        switch (glslangIntermediate->getVertexSpacing()) {
        case glslang::EvsEqual:            mode = spv::ExecutionModeSpacingEqual;          break;
        case glslang::EvsFractionalEven:   mode = spv::ExecutionModeSpacingFractionalEven; break;
        case glslang::EvsFractionalOdd:    mode = spv::ExecutionModeSpacingFractionalOdd;  break;
        default:                           mode = spv::ExecutionModeMax;                   break;
        }
        if (mode != spv::ExecutionModeMax)
            builder.addExecutionMode(shaderEntry, (spv::ExecutionMode)mode);

        switch (glslangIntermediate->getVertexOrder()) {
        case glslang::EvoCw:     mode = spv::ExecutionModeVertexOrderCw;  break;
        case glslang::EvoCcw:    mode = spv::ExecutionModeVertexOrderCcw; break;
        default:                 mode = spv::ExecutionModeMax;            break;
        }
        if (mode != spv::ExecutionModeMax)
            builder.addExecutionMode(shaderEntry, (spv::ExecutionMode)mode);

        if (glslangIntermediate->getPointMode())
            builder.addExecutionMode(shaderEntry, spv::ExecutionModePointMode);
        break;

    case EShLangGeometry:
        builder.addCapability(spv::CapabilityGeometry);
        switch (glslangIntermediate->getInputPrimitive()) {
        case glslang::ElgPoints:             mode = spv::ExecutionModeInputPoints;             break;
        case glslang::ElgLines:              mode = spv::ExecutionModeInputLines;              break;
        case glslang::ElgLinesAdjacency:     mode = spv::ExecutionModeInputLinesAdjacency;     break;
        case glslang::ElgTriangles:          mode = spv::ExecutionModeTriangles;               break;
        case glslang::ElgTrianglesAdjacency: mode = spv::ExecutionModeInputTrianglesAdjacency; break;
        default:                             mode = spv::ExecutionModeMax;                     break;
        }
        if (mode != spv::ExecutionModeMax)
            builder.addExecutionMode(shaderEntry, (spv::ExecutionMode)mode);

        builder.addExecutionMode(shaderEntry, spv::ExecutionModeInvocations, glslangIntermediate->getInvocations());

        switch (glslangIntermediate->getOutputPrimitive()) {
        case glslang::ElgPoints:        mode = spv::ExecutionModeOutputPoints;                 break;
        case glslang::ElgLineStrip:     mode = spv::ExecutionModeOutputLineStrip;              break;
        case glslang::ElgTriangleStrip: mode = spv::ExecutionModeOutputTriangleStrip;          break;
        default:                        mode = spv::ExecutionModeMax;                          break;
        }
        if (mode != spv::ExecutionModeMax)
            builder.addExecutionMode(shaderEntry, (spv::ExecutionMode)mode);
        builder.addExecutionMode(shaderEntry, spv::ExecutionModeOutputVertices, glslangIntermediate->getVertices());
        break;

    case EShLangFragment:
        builder.addCapability(spv::CapabilityShader);
        if (glslangIntermediate->getPixelCenterInteger())
            builder.addExecutionMode(shaderEntry, spv::ExecutionModePixelCenterInteger);

        if (glslangIntermediate->getOriginUpperLeft())
            builder.addExecutionMode(shaderEntry, spv::ExecutionModeOriginUpperLeft);
        else
            builder.addExecutionMode(shaderEntry, spv::ExecutionModeOriginLowerLeft);

        if (glslangIntermediate->getEarlyFragmentTests())
            builder.addExecutionMode(shaderEntry, spv::ExecutionModeEarlyFragmentTests);

        if (glslangIntermediate->getPostDepthCoverage()) {
            builder.addCapability(spv::CapabilitySampleMaskPostDepthCoverage);
            builder.addExecutionMode(shaderEntry, spv::ExecutionModePostDepthCoverage);
            builder.addExtension(spv::E_SPV_KHR_post_depth_coverage);
        }

        switch(glslangIntermediate->getDepth()) {
        case glslang::EldGreater:  mode = spv::ExecutionModeDepthGreater; break;
        case glslang::EldLess:     mode = spv::ExecutionModeDepthLess;    break;
        default:                   mode = spv::ExecutionModeMax;          break;
        }
        if (mode != spv::ExecutionModeMax)
            builder.addExecutionMode(shaderEntry, (spv::ExecutionMode)mode);

        if (glslangIntermediate->getDepth() != glslang::EldUnchanged && glslangIntermediate->isDepthReplacing())
            builder.addExecutionMode(shaderEntry, spv::ExecutionModeDepthReplacing);
        break;

    case EShLangCompute:
        builder.addCapability(spv::CapabilityShader);
        builder.addExecutionMode(shaderEntry, spv::ExecutionModeLocalSize, glslangIntermediate->getLocalSize(0),
                                                                           glslangIntermediate->getLocalSize(1),
                                                                           glslangIntermediate->getLocalSize(2));
        break;

    default:
        break;
    }
}

// Finish creating SPV, after the traversal is complete.
void TGlslangToSpvTraverser::finishSpv()
{
    // Finish the entry point function
    if (! entryPointTerminated) {
        builder.setBuildPoint(shaderEntry->getLastBlock());
        builder.leaveFunction();
    }

    // finish off the entry-point SPV instruction by adding the Input/Output <id>
    for (auto it = iOSet.cbegin(); it != iOSet.cend(); ++it)
        entryPoint->addIdOperand(*it);

    // Add capabilities, extensions, remove unneeded decorations, etc., 
    // based on the resulting SPIR-V.
    builder.postProcess();
}

// Write the SPV into 'out'.
void TGlslangToSpvTraverser::dumpSpv(std::vector<unsigned int>& out)
{
    builder.dump(out);
}

//
// Implement the traversal functions.
//
// Return true from interior nodes to have the external traversal
// continue on to children.  Return false if children were
// already processed.
//

//
// Symbols can turn into
//  - uniform/input reads
//  - output writes
//  - complex lvalue base setups:  foo.bar[3]....  , where we see foo and start up an access chain
//  - something simple that degenerates into the last bullet
//
void TGlslangToSpvTraverser::visitSymbol(glslang::TIntermSymbol* symbol)
{
    SpecConstantOpModeGuard spec_constant_op_mode_setter(&builder);
    if (symbol->getType().getQualifier().isSpecConstant())
        spec_constant_op_mode_setter.turnOnSpecConstantOpMode();

    // getSymbolId() will set up all the IO decorations on the first call.
    // Formal function parameters were mapped during makeFunctions().
    spv::Id id = getSymbolId(symbol);

    // Include all "static use" and "linkage only" interface variables on the OpEntryPoint instruction
    if (builder.isPointer(id)) {
        spv::StorageClass sc = builder.getStorageClass(id);
        if (sc == spv::StorageClassInput || sc == spv::StorageClassOutput) {
            if (!symbol->getType().isStruct() || symbol->getType().getStruct()->size() > 0)
                iOSet.insert(id);
        }
    }

    // Only process non-linkage-only nodes for generating actual static uses
    if (! linkageOnly || symbol->getQualifier().isSpecConstant()) {
        // Prepare to generate code for the access

        // L-value chains will be computed left to right.  We're on the symbol now,
        // which is the left-most part of the access chain, so now is "clear" time,
        // followed by setting the base.
        builder.clearAccessChain();

        // For now, we consider all user variables as being in memory, so they are pointers,
        // except for
        // A) R-Value arguments to a function, which are an intermediate object.
        //    See comments in handleUserFunctionCall().
        // B) Specialization constants (normal constants don't even come in as a variable),
        //    These are also pure R-values.
        glslang::TQualifier qualifier = symbol->getQualifier();
        if (qualifier.isSpecConstant() || rValueParameters.find(symbol->getId()) != rValueParameters.end())
            builder.setAccessChainRValue(id);
        else
            builder.setAccessChainLValue(id);
    }

    // Process linkage-only nodes for any special additional interface work.
    if (linkageOnly) {
        if (glslangIntermediate->getHlslFunctionality1()) {
            // Map implicit counter buffers to their originating buffers, which should have been
            // seen by now, given earlier pruning of unused counters, and preservation of order
            // of declaration.
            if (symbol->getType().getQualifier().isUniformOrBuffer()) {
                if (!glslangIntermediate->hasCounterBufferName(symbol->getName())) {
                    // Save possible originating buffers for counter buffers, keyed by
                    // making the potential counter-buffer name.
                    std::string keyName = symbol->getName().c_str();
                    keyName = glslangIntermediate->addCounterBufferName(keyName);
                    counterOriginator[keyName] = symbol;
                } else {
                    // Handle a counter buffer, by finding the saved originating buffer.
                    std::string keyName = symbol->getName().c_str();
                    auto it = counterOriginator.find(keyName);
                    if (it != counterOriginator.end()) {
                        id = getSymbolId(it->second);
                        if (id != spv::NoResult) {
                            spv::Id counterId = getSymbolId(symbol);
                            if (counterId != spv::NoResult) {
                                builder.addExtension("SPV_GOOGLE_hlsl_functionality1");
                                builder.addDecorationId(id, spv::DecorationHlslCounterBufferGOOGLE, counterId);
                            }
                        }
                    }
                }
            }
        }
    }
}

bool TGlslangToSpvTraverser::visitBinary(glslang::TVisit /* visit */, glslang::TIntermBinary* node)
{
    builder.setLine(node->getLoc().line);

    SpecConstantOpModeGuard spec_constant_op_mode_setter(&builder);
    if (node->getType().getQualifier().isSpecConstant())
        spec_constant_op_mode_setter.turnOnSpecConstantOpMode();

    // First, handle special cases
    switch (node->getOp()) {
    case glslang::EOpAssign:
    case glslang::EOpAddAssign:
    case glslang::EOpSubAssign:
    case glslang::EOpMulAssign:
    case glslang::EOpVectorTimesMatrixAssign:
    case glslang::EOpVectorTimesScalarAssign:
    case glslang::EOpMatrixTimesScalarAssign:
    case glslang::EOpMatrixTimesMatrixAssign:
    case glslang::EOpDivAssign:
    case glslang::EOpModAssign:
    case glslang::EOpAndAssign:
    case glslang::EOpInclusiveOrAssign:
    case glslang::EOpExclusiveOrAssign:
    case glslang::EOpLeftShiftAssign:
    case glslang::EOpRightShiftAssign:
        // A bin-op assign "a += b" means the same thing as "a = a + b"
        // where a is evaluated before b. For a simple assignment, GLSL
        // says to evaluate the left before the right.  So, always, left
        // node then right node.
        {
            // get the left l-value, save it away
            builder.clearAccessChain();
            node->getLeft()->traverse(this);
            spv::Builder::AccessChain lValue = builder.getAccessChain();

            // evaluate the right
            builder.clearAccessChain();
            node->getRight()->traverse(this);
            spv::Id rValue = accessChainLoad(node->getRight()->getType());

            if (node->getOp() != glslang::EOpAssign) {
                // the left is also an r-value
                builder.setAccessChain(lValue);
                spv::Id leftRValue = accessChainLoad(node->getLeft()->getType());

                // do the operation
                OpDecorations decorations = { TranslatePrecisionDecoration(node->getOperationPrecision()),
                                              TranslateNoContractionDecoration(node->getType().getQualifier()),
                                              TranslateNonUniformDecoration(node->getType().getQualifier()) };
                rValue = createBinaryOperation(node->getOp(), decorations,
                                               convertGlslangToSpvType(node->getType()), leftRValue, rValue,
                                               node->getType().getBasicType());

                // these all need their counterparts in createBinaryOperation()
                assert(rValue != spv::NoResult);
            }

            // store the result
            builder.setAccessChain(lValue);
            multiTypeStore(node->getType(), rValue);

            // assignments are expressions having an rValue after they are evaluated...
            builder.clearAccessChain();
            builder.setAccessChainRValue(rValue);
        }
        return false;
    case glslang::EOpIndexDirect:
    case glslang::EOpIndexDirectStruct:
        {
            // Get the left part of the access chain.
            node->getLeft()->traverse(this);

            // Add the next element in the chain

            const int glslangIndex = node->getRight()->getAsConstantUnion()->getConstArray()[0].getIConst();
            if (! node->getLeft()->getType().isArray() &&
                node->getLeft()->getType().isVector() &&
                node->getOp() == glslang::EOpIndexDirect) {
                // This is essentially a hard-coded vector swizzle of size 1,
                // so short circuit the access-chain stuff with a swizzle.
                std::vector<unsigned> swizzle;
                swizzle.push_back(glslangIndex);
                builder.accessChainPushSwizzle(swizzle, convertGlslangToSpvType(node->getLeft()->getType()));
            } else {
                int spvIndex = glslangIndex;
                if (node->getLeft()->getBasicType() == glslang::EbtBlock &&
                    node->getOp() == glslang::EOpIndexDirectStruct)
                {
                    // This may be, e.g., an anonymous block-member selection, which generally need
                    // index remapping due to hidden members in anonymous blocks.
                    std::vector<int>& remapper = memberRemapper[node->getLeft()->getType().getStruct()];
                    assert(remapper.size() > 0);
                    spvIndex = remapper[glslangIndex];
                }

                // normal case for indexing array or structure or block
                builder.accessChainPush(builder.makeIntConstant(spvIndex));

                // Add capabilities here for accessing PointSize and clip/cull distance.
                // We have deferred generation of associated capabilities until now.
                if (node->getLeft()->getType().isStruct() && ! node->getLeft()->getType().isArray())
                    declareUseOfStructMember(*(node->getLeft()->getType().getStruct()), glslangIndex);
            }
        }
        return false;
    case glslang::EOpIndexIndirect:
        {
            // Structure or array or vector indirection.
            // Will use native SPIR-V access-chain for struct and array indirection;
            // matrices are arrays of vectors, so will also work for a matrix.
            // Will use the access chain's 'component' for variable index into a vector.

            // This adapter is building access chains left to right.
            // Set up the access chain to the left.
            node->getLeft()->traverse(this);

            // save it so that computing the right side doesn't trash it
            spv::Builder::AccessChain partial = builder.getAccessChain();

            // compute the next index in the chain
            builder.clearAccessChain();
            node->getRight()->traverse(this);
            spv::Id index = accessChainLoad(node->getRight()->getType());

            addIndirectionIndexCapabilities(node->getLeft()->getType(), node->getRight()->getType());

            // restore the saved access chain
            builder.setAccessChain(partial);

            if (! node->getLeft()->getType().isArray() && node->getLeft()->getType().isVector())
                builder.accessChainPushComponent(index, convertGlslangToSpvType(node->getLeft()->getType()));
            else
                builder.accessChainPush(index);
        }
        return false;
    case glslang::EOpVectorSwizzle:
        {
            node->getLeft()->traverse(this);
            std::vector<unsigned> swizzle;
            convertSwizzle(*node->getRight()->getAsAggregate(), swizzle);
            builder.accessChainPushSwizzle(swizzle, convertGlslangToSpvType(node->getLeft()->getType()));
        }
        return false;
    case glslang::EOpMatrixSwizzle:
        logger->missingFunctionality("matrix swizzle");
        return true;
    case glslang::EOpLogicalOr:
    case glslang::EOpLogicalAnd:
        {

            // These may require short circuiting, but can sometimes be done as straight
            // binary operations.  The right operand must be short circuited if it has
            // side effects, and should probably be if it is complex.
            if (isTrivial(node->getRight()->getAsTyped()))
                break; // handle below as a normal binary operation
            // otherwise, we need to do dynamic short circuiting on the right operand
            spv::Id result = createShortCircuit(node->getOp(), *node->getLeft()->getAsTyped(), *node->getRight()->getAsTyped());
            builder.clearAccessChain();
            builder.setAccessChainRValue(result);
        }
        return false;
    default:
        break;
    }

    // Assume generic binary op...

    // get right operand
    builder.clearAccessChain();
    node->getLeft()->traverse(this);
    spv::Id left = accessChainLoad(node->getLeft()->getType());

    // get left operand
    builder.clearAccessChain();
    node->getRight()->traverse(this);
    spv::Id right = accessChainLoad(node->getRight()->getType());

    // get result
    OpDecorations decorations = { TranslatePrecisionDecoration(node->getOperationPrecision()),
                                  TranslateNoContractionDecoration(node->getType().getQualifier()),
                                  TranslateNonUniformDecoration(node->getType().getQualifier()) };
    spv::Id result = createBinaryOperation(node->getOp(), decorations,
                                           convertGlslangToSpvType(node->getType()), left, right,
                                           node->getLeft()->getType().getBasicType());

    builder.clearAccessChain();
    if (! result) {
        logger->missingFunctionality("unknown glslang binary operation");
        return true;  // pick up a child as the place-holder result
    } else {
        builder.setAccessChainRValue(result);
        return false;
    }
}

bool TGlslangToSpvTraverser::visitUnary(glslang::TVisit /* visit */, glslang::TIntermUnary* node)
{
    builder.setLine(node->getLoc().line);

    SpecConstantOpModeGuard spec_constant_op_mode_setter(&builder);
    if (node->getType().getQualifier().isSpecConstant())
        spec_constant_op_mode_setter.turnOnSpecConstantOpMode();

    spv::Id result = spv::NoResult;

    // try texturing first
    result = createImageTextureFunctionCall(node);
    if (result != spv::NoResult) {
        builder.clearAccessChain();
        builder.setAccessChainRValue(result);

        return false; // done with this node
    }

    // Non-texturing.

    if (node->getOp() == glslang::EOpArrayLength) {
        // Quite special; won't want to evaluate the operand.

        // Currently, the front-end does not allow .length() on an array until it is sized,
        // except for the last block membeor of an SSBO.
        // TODO: If this changes, link-time sized arrays might show up here, and need their
        // size extracted.

        // Normal .length() would have been constant folded by the front-end.
        // So, this has to be block.lastMember.length().
        // SPV wants "block" and member number as the operands, go get them.

        glslang::TIntermTyped* block = node->getOperand()->getAsBinaryNode()->getLeft();
        block->traverse(this);
        unsigned int member = node->getOperand()->getAsBinaryNode()->getRight()->getAsConstantUnion()->getConstArray()[0].getUConst();
        spv::Id length = builder.createArrayLength(builder.accessChainGetLValue(), member);

        builder.clearAccessChain();
        builder.setAccessChainRValue(length);

        return false;
    }

    // Start by evaluating the operand

    // Does it need a swizzle inversion?  If so, evaluation is inverted;
    // operate first on the swizzle base, then apply the swizzle.
    spv::Id invertedType = spv::NoType;
    auto resultType = [&invertedType, &node, this](){ return invertedType != spv::NoType ? invertedType : convertGlslangToSpvType(node->getType()); };
    if (node->getOp() == glslang::EOpInterpolateAtCentroid)
        invertedType = getInvertedSwizzleType(*node->getOperand());

    builder.clearAccessChain();
    if (invertedType != spv::NoType)
        node->getOperand()->getAsBinaryNode()->getLeft()->traverse(this);
    else
        node->getOperand()->traverse(this);

    spv::Id operand = spv::NoResult;

    if (node->getOp() == glslang::EOpAtomicCounterIncrement ||
        node->getOp() == glslang::EOpAtomicCounterDecrement ||
        node->getOp() == glslang::EOpAtomicCounter          ||
        node->getOp() == glslang::EOpInterpolateAtCentroid)
        operand = builder.accessChainGetLValue(); // Special case l-value operands
    else
        operand = accessChainLoad(node->getOperand()->getType());

    OpDecorations decorations = { TranslatePrecisionDecoration(node->getOperationPrecision()),
                                  TranslateNoContractionDecoration(node->getType().getQualifier()),
                                  TranslateNonUniformDecoration(node->getType().getQualifier()) };

    // it could be a conversion
    if (! result)
        result = createConversion(node->getOp(), decorations, resultType(), operand, node->getOperand()->getBasicType());

    // if not, then possibly an operation
    if (! result)
        result = createUnaryOperation(node->getOp(), decorations, resultType(), operand, node->getOperand()->getBasicType());

    if (result) {
        if (invertedType) {
            result = createInvertedSwizzle(decorations.precision, *node->getOperand(), result);
            builder.addDecoration(result, decorations.nonUniform);
        }

        builder.clearAccessChain();
        builder.setAccessChainRValue(result);

        return false; // done with this node
    }

    // it must be a special case, check...
    switch (node->getOp()) {
    case glslang::EOpPostIncrement:
    case glslang::EOpPostDecrement:
    case glslang::EOpPreIncrement:
    case glslang::EOpPreDecrement:
        {
            // we need the integer value "1" or the floating point "1.0" to add/subtract
            spv::Id one = 0;
            if (node->getBasicType() == glslang::EbtFloat)
                one = builder.makeFloatConstant(1.0F);
            else if (node->getBasicType() == glslang::EbtDouble)
                one = builder.makeDoubleConstant(1.0);
            else if (node->getBasicType() == glslang::EbtFloat16)
                one = builder.makeFloat16Constant(1.0F);
            else if (node->getBasicType() == glslang::EbtInt8  || node->getBasicType() == glslang::EbtUint8)
                one = builder.makeInt8Constant(1);
            else if (node->getBasicType() == glslang::EbtInt16 || node->getBasicType() == glslang::EbtUint16)
                one = builder.makeInt16Constant(1);
            else if (node->getBasicType() == glslang::EbtInt64 || node->getBasicType() == glslang::EbtUint64)
                one = builder.makeInt64Constant(1);
            else
                one = builder.makeIntConstant(1);
            glslang::TOperator op;
            if (node->getOp() == glslang::EOpPreIncrement ||
                node->getOp() == glslang::EOpPostIncrement)
                op = glslang::EOpAdd;
            else
                op = glslang::EOpSub;

            spv::Id result = createBinaryOperation(op, decorations,
                                                   convertGlslangToSpvType(node->getType()), operand, one,
                                                   node->getType().getBasicType());
            assert(result != spv::NoResult);

            // The result of operation is always stored, but conditionally the
            // consumed result.  The consumed result is always an r-value.
            builder.accessChainStore(result);
            builder.clearAccessChain();
            if (node->getOp() == glslang::EOpPreIncrement ||
                node->getOp() == glslang::EOpPreDecrement)
                builder.setAccessChainRValue(result);
            else
                builder.setAccessChainRValue(operand);
        }

        return false;

    case glslang::EOpEmitStreamVertex:
        builder.createNoResultOp(spv::OpEmitStreamVertex, operand);
        return false;
    case glslang::EOpEndStreamPrimitive:
        builder.createNoResultOp(spv::OpEndStreamPrimitive, operand);
        return false;

    default:
        logger->missingFunctionality("unknown glslang unary");
        return true;  // pick up operand as placeholder result
    }
}

bool TGlslangToSpvTraverser::visitAggregate(glslang::TVisit visit, glslang::TIntermAggregate* node)
{
    SpecConstantOpModeGuard spec_constant_op_mode_setter(&builder);
    if (node->getType().getQualifier().isSpecConstant())
        spec_constant_op_mode_setter.turnOnSpecConstantOpMode();

    spv::Id result = spv::NoResult;
    spv::Id invertedType = spv::NoType;  // to use to override the natural type of the node
    auto resultType = [&invertedType, &node, this](){ return invertedType != spv::NoType ? invertedType : convertGlslangToSpvType(node->getType()); };

    // try texturing
    result = createImageTextureFunctionCall(node);
    if (result != spv::NoResult) {
        builder.clearAccessChain();
        builder.setAccessChainRValue(result);

        return false;
#ifdef AMD_EXTENSIONS
    } else if (node->getOp() == glslang::EOpImageStore || node->getOp() == glslang::EOpImageStoreLod) {
#else
    } else if (node->getOp() == glslang::EOpImageStore) {
#endif
        // "imageStore" is a special case, which has no result
        return false;
    }

    glslang::TOperator binOp = glslang::EOpNull;
    bool reduceComparison = true;
    bool isMatrix = false;
    bool noReturnValue = false;
    bool atomic = false;

    assert(node->getOp());

    spv::Decoration precision = TranslatePrecisionDecoration(node->getOperationPrecision());

    switch (node->getOp()) {
    case glslang::EOpSequence:
    {
        if (preVisit)
            ++sequenceDepth;
        else
            --sequenceDepth;

        if (sequenceDepth == 1) {
            // If this is the parent node of all the functions, we want to see them
            // early, so all call points have actual SPIR-V functions to reference.
            // In all cases, still let the traverser visit the children for us.
            makeFunctions(node->getAsAggregate()->getSequence());

            // Also, we want all globals initializers to go into the beginning of the entry point, before
            // anything else gets there, so visit out of order, doing them all now.
            makeGlobalInitializers(node->getAsAggregate()->getSequence());

            // Initializers are done, don't want to visit again, but functions and link objects need to be processed,
            // so do them manually.
            visitFunctions(node->getAsAggregate()->getSequence());

            return false;
        }

        return true;
    }
    case glslang::EOpLinkerObjects:
    {
        if (visit == glslang::EvPreVisit)
            linkageOnly = true;
        else
            linkageOnly = false;

        return true;
    }
    case glslang::EOpComma:
    {
        // processing from left to right naturally leaves the right-most
        // lying around in the access chain
        glslang::TIntermSequence& glslangOperands = node->getSequence();
        for (int i = 0; i < (int)glslangOperands.size(); ++i)
            glslangOperands[i]->traverse(this);

        return false;
    }
    case glslang::EOpFunction:
        if (visit == glslang::EvPreVisit) {
            if (isShaderEntryPoint(node)) {
                inEntryPoint = true;
                builder.setBuildPoint(shaderEntry->getLastBlock());
                currentFunction = shaderEntry;
            } else {
                handleFunctionEntry(node);
            }
        } else {
            if (inEntryPoint)
                entryPointTerminated = true;
            builder.leaveFunction();
            inEntryPoint = false;
        }

        return true;
    case glslang::EOpParameters:
        // Parameters will have been consumed by EOpFunction processing, but not
        // the body, so we still visited the function node's children, making this
        // child redundant.
        return false;
    case glslang::EOpFunctionCall:
    {
        builder.setLine(node->getLoc().line);
        if (node->isUserDefined())
            result = handleUserFunctionCall(node);
        // assert(result);  // this can happen for bad shaders because the call graph completeness checking is not yet done
        if (result) {
            builder.clearAccessChain();
            builder.setAccessChainRValue(result);
        } else
            logger->missingFunctionality("missing user function; linker needs to catch that");

        return false;
    }
    case glslang::EOpConstructMat2x2:
    case glslang::EOpConstructMat2x3:
    case glslang::EOpConstructMat2x4:
    case glslang::EOpConstructMat3x2:
    case glslang::EOpConstructMat3x3:
    case glslang::EOpConstructMat3x4:
    case glslang::EOpConstructMat4x2:
    case glslang::EOpConstructMat4x3:
    case glslang::EOpConstructMat4x4:
    case glslang::EOpConstructDMat2x2:
    case glslang::EOpConstructDMat2x3:
    case glslang::EOpConstructDMat2x4:
    case glslang::EOpConstructDMat3x2:
    case glslang::EOpConstructDMat3x3:
    case glslang::EOpConstructDMat3x4:
    case glslang::EOpConstructDMat4x2:
    case glslang::EOpConstructDMat4x3:
    case glslang::EOpConstructDMat4x4:
    case glslang::EOpConstructIMat2x2:
    case glslang::EOpConstructIMat2x3:
    case glslang::EOpConstructIMat2x4:
    case glslang::EOpConstructIMat3x2:
    case glslang::EOpConstructIMat3x3:
    case glslang::EOpConstructIMat3x4:
    case glslang::EOpConstructIMat4x2:
    case glslang::EOpConstructIMat4x3:
    case glslang::EOpConstructIMat4x4:
    case glslang::EOpConstructUMat2x2:
    case glslang::EOpConstructUMat2x3:
    case glslang::EOpConstructUMat2x4:
    case glslang::EOpConstructUMat3x2:
    case glslang::EOpConstructUMat3x3:
    case glslang::EOpConstructUMat3x4:
    case glslang::EOpConstructUMat4x2:
    case glslang::EOpConstructUMat4x3:
    case glslang::EOpConstructUMat4x4:
    case glslang::EOpConstructBMat2x2:
    case glslang::EOpConstructBMat2x3:
    case glslang::EOpConstructBMat2x4:
    case glslang::EOpConstructBMat3x2:
    case glslang::EOpConstructBMat3x3:
    case glslang::EOpConstructBMat3x4:
    case glslang::EOpConstructBMat4x2:
    case glslang::EOpConstructBMat4x3:
    case glslang::EOpConstructBMat4x4:
    case glslang::EOpConstructF16Mat2x2:
    case glslang::EOpConstructF16Mat2x3:
    case glslang::EOpConstructF16Mat2x4:
    case glslang::EOpConstructF16Mat3x2:
    case glslang::EOpConstructF16Mat3x3:
    case glslang::EOpConstructF16Mat3x4:
    case glslang::EOpConstructF16Mat4x2:
    case glslang::EOpConstructF16Mat4x3:
    case glslang::EOpConstructF16Mat4x4:
        isMatrix = true;
        // fall through
    case glslang::EOpConstructFloat:
    case glslang::EOpConstructVec2:
    case glslang::EOpConstructVec3:
    case glslang::EOpConstructVec4:
    case glslang::EOpConstructDouble:
    case glslang::EOpConstructDVec2:
    case glslang::EOpConstructDVec3:
    case glslang::EOpConstructDVec4:
    case glslang::EOpConstructFloat16:
    case glslang::EOpConstructF16Vec2:
    case glslang::EOpConstructF16Vec3:
    case glslang::EOpConstructF16Vec4:
    case glslang::EOpConstructBool:
    case glslang::EOpConstructBVec2:
    case glslang::EOpConstructBVec3:
    case glslang::EOpConstructBVec4:
    case glslang::EOpConstructInt8:
    case glslang::EOpConstructI8Vec2:
    case glslang::EOpConstructI8Vec3:
    case glslang::EOpConstructI8Vec4:
    case glslang::EOpConstructUint8:
    case glslang::EOpConstructU8Vec2:
    case glslang::EOpConstructU8Vec3:
    case glslang::EOpConstructU8Vec4:
    case glslang::EOpConstructInt16:
    case glslang::EOpConstructI16Vec2:
    case glslang::EOpConstructI16Vec3:
    case glslang::EOpConstructI16Vec4:
    case glslang::EOpConstructUint16:
    case glslang::EOpConstructU16Vec2:
    case glslang::EOpConstructU16Vec3:
    case glslang::EOpConstructU16Vec4:
    case glslang::EOpConstructInt:
    case glslang::EOpConstructIVec2:
    case glslang::EOpConstructIVec3:
    case glslang::EOpConstructIVec4:
    case glslang::EOpConstructUint:
    case glslang::EOpConstructUVec2:
    case glslang::EOpConstructUVec3:
    case glslang::EOpConstructUVec4:
    case glslang::EOpConstructInt64:
    case glslang::EOpConstructI64Vec2:
    case glslang::EOpConstructI64Vec3:
    case glslang::EOpConstructI64Vec4:
    case glslang::EOpConstructUint64:
    case glslang::EOpConstructU64Vec2:
    case glslang::EOpConstructU64Vec3:
    case glslang::EOpConstructU64Vec4:
    case glslang::EOpConstructStruct:
    case glslang::EOpConstructTextureSampler:
    {
        builder.setLine(node->getLoc().line);
        std::vector<spv::Id> arguments;
        translateArguments(*node, arguments);
        spv::Id constructed;
        if (node->getOp() == glslang::EOpConstructTextureSampler)
            constructed = builder.createOp(spv::OpSampledImage, resultType(), arguments);
        else if (node->getOp() == glslang::EOpConstructStruct || node->getType().isArray()) {
            std::vector<spv::Id> constituents;
            for (int c = 0; c < (int)arguments.size(); ++c)
                constituents.push_back(arguments[c]);
            constructed = builder.createCompositeConstruct(resultType(), constituents);
        } else if (isMatrix)
            constructed = builder.createMatrixConstructor(precision, arguments, resultType());
        else
            constructed = builder.createConstructor(precision, arguments, resultType());

        builder.clearAccessChain();
        builder.setAccessChainRValue(constructed);

        return false;
    }

    // These six are component-wise compares with component-wise results.
    // Forward on to createBinaryOperation(), requesting a vector result.
    case glslang::EOpLessThan:
    case glslang::EOpGreaterThan:
    case glslang::EOpLessThanEqual:
    case glslang::EOpGreaterThanEqual:
    case glslang::EOpVectorEqual:
    case glslang::EOpVectorNotEqual:
    {
        // Map the operation to a binary
        binOp = node->getOp();
        reduceComparison = false;
        switch (node->getOp()) {
        case glslang::EOpVectorEqual:     binOp = glslang::EOpVectorEqual;      break;
        case glslang::EOpVectorNotEqual:  binOp = glslang::EOpVectorNotEqual;   break;
        default:                          binOp = node->getOp();                break;
        }

        break;
    }
    case glslang::EOpMul:
        // component-wise matrix multiply
        binOp = glslang::EOpMul;
        break;
    case glslang::EOpOuterProduct:
        // two vectors multiplied to make a matrix
        binOp = glslang::EOpOuterProduct;
        break;
    case glslang::EOpDot:
    {
        // for scalar dot product, use multiply
        glslang::TIntermSequence& glslangOperands = node->getSequence();
        if (glslangOperands[0]->getAsTyped()->getVectorSize() == 1)
            binOp = glslang::EOpMul;
        break;
    }
    case glslang::EOpMod:
        // when an aggregate, this is the floating-point mod built-in function,
        // which can be emitted by the one in createBinaryOperation()
        binOp = glslang::EOpMod;
        break;
    case glslang::EOpEmitVertex:
    case glslang::EOpEndPrimitive:
    case glslang::EOpBarrier:
    case glslang::EOpMemoryBarrier:
    case glslang::EOpMemoryBarrierAtomicCounter:
    case glslang::EOpMemoryBarrierBuffer:
    case glslang::EOpMemoryBarrierImage:
    case glslang::EOpMemoryBarrierShared:
    case glslang::EOpGroupMemoryBarrier:
    case glslang::EOpDeviceMemoryBarrier:
    case glslang::EOpAllMemoryBarrierWithGroupSync:
    case glslang::EOpDeviceMemoryBarrierWithGroupSync:
    case glslang::EOpWorkgroupMemoryBarrier:
    case glslang::EOpWorkgroupMemoryBarrierWithGroupSync:
    case glslang::EOpSubgroupBarrier:
    case glslang::EOpSubgroupMemoryBarrier:
    case glslang::EOpSubgroupMemoryBarrierBuffer:
    case glslang::EOpSubgroupMemoryBarrierImage:
    case glslang::EOpSubgroupMemoryBarrierShared:
        noReturnValue = true;
        // These all have 0 operands and will naturally finish up in the code below for 0 operands
        break;

    case glslang::EOpAtomicAdd:
    case glslang::EOpAtomicMin:
    case glslang::EOpAtomicMax:
    case glslang::EOpAtomicAnd:
    case glslang::EOpAtomicOr:
    case glslang::EOpAtomicXor:
    case glslang::EOpAtomicExchange:
    case glslang::EOpAtomicCompSwap:
        atomic = true;
        break;

    case glslang::EOpAtomicCounterAdd:
    case glslang::EOpAtomicCounterSubtract:
    case glslang::EOpAtomicCounterMin:
    case glslang::EOpAtomicCounterMax:
    case glslang::EOpAtomicCounterAnd:
    case glslang::EOpAtomicCounterOr:
    case glslang::EOpAtomicCounterXor:
    case glslang::EOpAtomicCounterExchange:
    case glslang::EOpAtomicCounterCompSwap:
        builder.addExtension("SPV_KHR_shader_atomic_counter_ops");
        builder.addCapability(spv::CapabilityAtomicStorageOps);
        atomic = true;
        break;

    default:
        break;
    }

    //
    // See if it maps to a regular operation.
    //
    if (binOp != glslang::EOpNull) {
        glslang::TIntermTyped* left = node->getSequence()[0]->getAsTyped();
        glslang::TIntermTyped* right = node->getSequence()[1]->getAsTyped();
        assert(left && right);

        builder.clearAccessChain();
        left->traverse(this);
        spv::Id leftId = accessChainLoad(left->getType());

        builder.clearAccessChain();
        right->traverse(this);
        spv::Id rightId = accessChainLoad(right->getType());

        builder.setLine(node->getLoc().line);
        OpDecorations decorations = { precision,
                                      TranslateNoContractionDecoration(node->getType().getQualifier()),
                                      TranslateNonUniformDecoration(node->getType().getQualifier()) };
        result = createBinaryOperation(binOp, decorations,
                                       resultType(), leftId, rightId,
                                       left->getType().getBasicType(), reduceComparison);

        // code above should only make binOp that exists in createBinaryOperation
        assert(result != spv::NoResult);
        builder.clearAccessChain();
        builder.setAccessChainRValue(result);

        return false;
    }

    //
    // Create the list of operands.
    //
    glslang::TIntermSequence& glslangOperands = node->getSequence();
    std::vector<spv::Id> operands;
    for (int arg = 0; arg < (int)glslangOperands.size(); ++arg) {
        // special case l-value operands; there are just a few
        bool lvalue = false;
        switch (node->getOp()) {
        case glslang::EOpFrexp:
        case glslang::EOpModf:
            if (arg == 1)
                lvalue = true;
            break;
        case glslang::EOpInterpolateAtSample:
        case glslang::EOpInterpolateAtOffset:
#ifdef AMD_EXTENSIONS
        case glslang::EOpInterpolateAtVertex:
#endif
            if (arg == 0) {
                lvalue = true;

                // Does it need a swizzle inversion?  If so, evaluation is inverted;
                // operate first on the swizzle base, then apply the swizzle.
                if (glslangOperands[0]->getAsOperator() &&
                    glslangOperands[0]->getAsOperator()->getOp() == glslang::EOpVectorSwizzle)
                    invertedType = convertGlslangToSpvType(glslangOperands[0]->getAsBinaryNode()->getLeft()->getType());
            }
            break;
        case glslang::EOpAtomicAdd:
        case glslang::EOpAtomicMin:
        case glslang::EOpAtomicMax:
        case glslang::EOpAtomicAnd:
        case glslang::EOpAtomicOr:
        case glslang::EOpAtomicXor:
        case glslang::EOpAtomicExchange:
        case glslang::EOpAtomicCompSwap:
        case glslang::EOpAtomicCounterAdd:
        case glslang::EOpAtomicCounterSubtract:
        case glslang::EOpAtomicCounterMin:
        case glslang::EOpAtomicCounterMax:
        case glslang::EOpAtomicCounterAnd:
        case glslang::EOpAtomicCounterOr:
        case glslang::EOpAtomicCounterXor:
        case glslang::EOpAtomicCounterExchange:
        case glslang::EOpAtomicCounterCompSwap:
            if (arg == 0)
                lvalue = true;
            break;
        case glslang::EOpAddCarry:
        case glslang::EOpSubBorrow:
            if (arg == 2)
                lvalue = true;
            break;
        case glslang::EOpUMulExtended:
        case glslang::EOpIMulExtended:
            if (arg >= 2)
                lvalue = true;
            break;
        default:
            break;
        }
        builder.clearAccessChain();
        if (invertedType != spv::NoType && arg == 0)
            glslangOperands[0]->getAsBinaryNode()->getLeft()->traverse(this);
        else
            glslangOperands[arg]->traverse(this);
        if (lvalue)
            operands.push_back(builder.accessChainGetLValue());
        else {
            builder.setLine(node->getLoc().line);
            operands.push_back(accessChainLoad(glslangOperands[arg]->getAsTyped()->getType()));
        }
    }

    builder.setLine(node->getLoc().line);
    if (atomic) {
        // Handle all atomics
        result = createAtomicOperation(node->getOp(), precision, resultType(), operands, node->getBasicType());
    } else {
        // Pass through to generic operations.
        switch (glslangOperands.size()) {
        case 0:
            result = createNoArgOperation(node->getOp(), precision, resultType());
            break;
        case 1:
            {
                OpDecorations decorations = { precision, 
                                              TranslateNoContractionDecoration(node->getType().getQualifier()),
                                              TranslateNonUniformDecoration(node->getType().getQualifier()) };
                result = createUnaryOperation(
                    node->getOp(), decorations,
                    resultType(), operands.front(),
                    glslangOperands[0]->getAsTyped()->getBasicType());
            }
            break;
        default:
            result = createMiscOperation(node->getOp(), precision, resultType(), operands, node->getBasicType());
            break;
        }
        if (invertedType)
            result = createInvertedSwizzle(precision, *glslangOperands[0]->getAsBinaryNode(), result);
    }

    if (noReturnValue)
        return false;

    if (! result) {
        logger->missingFunctionality("unknown glslang aggregate");
        return true;  // pick up a child as a placeholder operand
    } else {
        builder.clearAccessChain();
        builder.setAccessChainRValue(result);
        return false;
    }
}

// This path handles both if-then-else and ?:
// The if-then-else has a node type of void, while
// ?: has either a void or a non-void node type
//
// Leaving the result, when not void:
// GLSL only has r-values as the result of a :?, but
// if we have an l-value, that can be more efficient if it will
// become the base of a complex r-value expression, because the
// next layer copies r-values into memory to use the access-chain mechanism
bool TGlslangToSpvTraverser::visitSelection(glslang::TVisit /* visit */, glslang::TIntermSelection* node)
{
    // See if it simple and safe, or required, to execute both sides.
    // Crucially, side effects must be either semantically required or avoided,
    // and there are performance trade-offs.
    // Return true if required or a good idea (and safe) to execute both sides,
    // false otherwise.
    const auto bothSidesPolicy = [&]() -> bool {
        // do we have both sides?
        if (node->getTrueBlock()  == nullptr ||
            node->getFalseBlock() == nullptr)
            return false;

        // required? (unless we write additional code to look for side effects
        // and make performance trade-offs if none are present)
        if (!node->getShortCircuit())
            return true;

        // if not required to execute both, decide based on performance/practicality...

        // see if OpSelect can handle it
        if ((!node->getType().isScalar() && !node->getType().isVector()) ||
            node->getBasicType() == glslang::EbtVoid)
            return false;

        assert(node->getType() == node->getTrueBlock() ->getAsTyped()->getType() &&
               node->getType() == node->getFalseBlock()->getAsTyped()->getType());

        // return true if a single operand to ? : is okay for OpSelect
        const auto operandOkay = [](glslang::TIntermTyped* node) {
            return node->getAsSymbolNode() || node->getType().getQualifier().isConstant();
        };

        return operandOkay(node->getTrueBlock() ->getAsTyped()) &&
               operandOkay(node->getFalseBlock()->getAsTyped());
    };

    spv::Id result = spv::NoResult; // upcoming result selecting between trueValue and falseValue
    // emit the condition before doing anything with selection
    node->getCondition()->traverse(this);
    spv::Id condition = accessChainLoad(node->getCondition()->getType());

    // Find a way of executing both sides and selecting the right result.
    const auto executeBothSides = [&]() -> void {
        // execute both sides
        node->getTrueBlock()->traverse(this);
        spv::Id trueValue = accessChainLoad(node->getTrueBlock()->getAsTyped()->getType());
        node->getFalseBlock()->traverse(this);
        spv::Id falseValue = accessChainLoad(node->getTrueBlock()->getAsTyped()->getType());

        builder.setLine(node->getLoc().line);

        // done if void
        if (node->getBasicType() == glslang::EbtVoid)
            return;

        // emit code to select between trueValue and falseValue

        // see if OpSelect can handle it
        if (node->getType().isScalar() || node->getType().isVector()) {
            // Emit OpSelect for this selection.

            // smear condition to vector, if necessary (AST is always scalar)
            if (builder.isVector(trueValue))
                condition = builder.smearScalar(spv::NoPrecision, condition, 
                                                builder.makeVectorType(builder.makeBoolType(),
                                                                       builder.getNumComponents(trueValue)));

            // OpSelect
            result = builder.createTriOp(spv::OpSelect,
                                         convertGlslangToSpvType(node->getType()), condition,
                                                                 trueValue, falseValue);

            builder.clearAccessChain();
            builder.setAccessChainRValue(result);
        } else {
            // We need control flow to select the result.
            // TODO: Once SPIR-V OpSelect allows arbitrary types, eliminate this path.
            result = builder.createVariable(spv::StorageClassFunction, convertGlslangToSpvType(node->getType()));

            // Selection control:
            const spv::SelectionControlMask control = TranslateSelectionControl(*node);

            // make an "if" based on the value created by the condition
            spv::Builder::If ifBuilder(condition, control, builder);

            // emit the "then" statement
            builder.createStore(trueValue, result);
            ifBuilder.makeBeginElse();
            // emit the "else" statement
            builder.createStore(falseValue, result);

            // finish off the control flow
            ifBuilder.makeEndIf();

            builder.clearAccessChain();
            builder.setAccessChainLValue(result);
        }
    };

    // Execute the one side needed, as per the condition
    const auto executeOneSide = [&]() {
        // Always emit control flow.
        if (node->getBasicType() != glslang::EbtVoid)
            result = builder.createVariable(spv::StorageClassFunction, convertGlslangToSpvType(node->getType()));

        // Selection control:
        const spv::SelectionControlMask control = TranslateSelectionControl(*node);

        // make an "if" based on the value created by the condition
        spv::Builder::If ifBuilder(condition, control, builder);

        // emit the "then" statement
        if (node->getTrueBlock() != nullptr) {
            node->getTrueBlock()->traverse(this);
            if (result != spv::NoResult)
                builder.createStore(accessChainLoad(node->getTrueBlock()->getAsTyped()->getType()), result);
        }

        if (node->getFalseBlock() != nullptr) {
            ifBuilder.makeBeginElse();
            // emit the "else" statement
            node->getFalseBlock()->traverse(this);
            if (result != spv::NoResult)
                builder.createStore(accessChainLoad(node->getFalseBlock()->getAsTyped()->getType()), result);
        }

        // finish off the control flow
        ifBuilder.makeEndIf();

        if (result != spv::NoResult) {
            builder.clearAccessChain();
            builder.setAccessChainLValue(result);
        }
    };

    // Try for OpSelect (or a requirement to execute both sides)
    if (bothSidesPolicy()) {
        SpecConstantOpModeGuard spec_constant_op_mode_setter(&builder);
        if (node->getType().getQualifier().isSpecConstant())
            spec_constant_op_mode_setter.turnOnSpecConstantOpMode();
        executeBothSides();
    } else
        executeOneSide();

    return false;
}

bool TGlslangToSpvTraverser::visitSwitch(glslang::TVisit /* visit */, glslang::TIntermSwitch* node)
{
    // emit and get the condition before doing anything with switch
    node->getCondition()->traverse(this);
    spv::Id selector = accessChainLoad(node->getCondition()->getAsTyped()->getType());

    // Selection control:
    const spv::SelectionControlMask control = TranslateSwitchControl(*node);

    // browse the children to sort out code segments
    int defaultSegment = -1;
    std::vector<TIntermNode*> codeSegments;
    glslang::TIntermSequence& sequence = node->getBody()->getSequence();
    std::vector<int> caseValues;
    std::vector<int> valueIndexToSegment(sequence.size());  // note: probably not all are used, it is an overestimate
    for (glslang::TIntermSequence::iterator c = sequence.begin(); c != sequence.end(); ++c) {
        TIntermNode* child = *c;
        if (child->getAsBranchNode() && child->getAsBranchNode()->getFlowOp() == glslang::EOpDefault)
            defaultSegment = (int)codeSegments.size();
        else if (child->getAsBranchNode() && child->getAsBranchNode()->getFlowOp() == glslang::EOpCase) {
            valueIndexToSegment[caseValues.size()] = (int)codeSegments.size();
            caseValues.push_back(child->getAsBranchNode()->getExpression()->getAsConstantUnion()->getConstArray()[0].getIConst());
        } else
            codeSegments.push_back(child);
    }

    // handle the case where the last code segment is missing, due to no code
    // statements between the last case and the end of the switch statement
    if ((caseValues.size() && (int)codeSegments.size() == valueIndexToSegment[caseValues.size() - 1]) ||
        (int)codeSegments.size() == defaultSegment)
        codeSegments.push_back(nullptr);

    // make the switch statement
    std::vector<spv::Block*> segmentBlocks; // returned, as the blocks allocated in the call
    builder.makeSwitch(selector, control, (int)codeSegments.size(), caseValues, valueIndexToSegment, defaultSegment, segmentBlocks);

    // emit all the code in the segments
    breakForLoop.push(false);
    for (unsigned int s = 0; s < codeSegments.size(); ++s) {
        builder.nextSwitchSegment(segmentBlocks, s);
        if (codeSegments[s])
            codeSegments[s]->traverse(this);
        else
            builder.addSwitchBreak();
    }
    breakForLoop.pop();

    builder.endSwitch(segmentBlocks);

    return false;
}

void TGlslangToSpvTraverser::visitConstantUnion(glslang::TIntermConstantUnion* node)
{
    int nextConst = 0;
    spv::Id constant = createSpvConstantFromConstUnionArray(node->getType(), node->getConstArray(), nextConst, false);

    builder.clearAccessChain();
    builder.setAccessChainRValue(constant);
}

bool TGlslangToSpvTraverser::visitLoop(glslang::TVisit /* visit */, glslang::TIntermLoop* node)
{
    auto blocks = builder.makeNewLoop();
    builder.createBranch(&blocks.head);

    // Loop control:
    unsigned int dependencyLength = glslang::TIntermLoop::dependencyInfinite;
    const spv::LoopControlMask control = TranslateLoopControl(*node, dependencyLength);

    // Spec requires back edges to target header blocks, and every header block
    // must dominate its merge block.  Make a header block first to ensure these
    // conditions are met.  By definition, it will contain OpLoopMerge, followed
    // by a block-ending branch.  But we don't want to put any other body/test
    // instructions in it, since the body/test may have arbitrary instructions,
    // including merges of its own.
    builder.setLine(node->getLoc().line);
    builder.setBuildPoint(&blocks.head);
    builder.createLoopMerge(&blocks.merge, &blocks.continue_target, control, dependencyLength);
    if (node->testFirst() && node->getTest()) {
        spv::Block& test = builder.makeNewBlock();
        builder.createBranch(&test);

        builder.setBuildPoint(&test);
        node->getTest()->traverse(this);
        spv::Id condition = accessChainLoad(node->getTest()->getType());
        builder.createConditionalBranch(condition, &blocks.body, &blocks.merge);

        builder.setBuildPoint(&blocks.body);
        breakForLoop.push(true);
        if (node->getBody())
            node->getBody()->traverse(this);
        builder.createBranch(&blocks.continue_target);
        breakForLoop.pop();

        builder.setBuildPoint(&blocks.continue_target);
        if (node->getTerminal())
            node->getTerminal()->traverse(this);
        builder.createBranch(&blocks.head);
    } else {
        builder.setLine(node->getLoc().line);
        builder.createBranch(&blocks.body);

        breakForLoop.push(true);
        builder.setBuildPoint(&blocks.body);
        if (node->getBody())
            node->getBody()->traverse(this);
        builder.createBranch(&blocks.continue_target);
        breakForLoop.pop();

        builder.setBuildPoint(&blocks.continue_target);
        if (node->getTerminal())
            node->getTerminal()->traverse(this);
        if (node->getTest()) {
            node->getTest()->traverse(this);
            spv::Id condition =
                accessChainLoad(node->getTest()->getType());
            builder.createConditionalBranch(condition, &blocks.head, &blocks.merge);
        } else {
            // TODO: unless there was a break/return/discard instruction
            // somewhere in the body, this is an infinite loop, so we should
            // issue a warning.
            builder.createBranch(&blocks.head);
        }
    }
    builder.setBuildPoint(&blocks.merge);
    builder.closeLoop();
    return false;
}

bool TGlslangToSpvTraverser::visitBranch(glslang::TVisit /* visit */, glslang::TIntermBranch* node)
{
    if (node->getExpression())
        node->getExpression()->traverse(this);

    builder.setLine(node->getLoc().line);

    switch (node->getFlowOp()) {
    case glslang::EOpKill:
        builder.makeDiscard();
        break;
    case glslang::EOpBreak:
        if (breakForLoop.top())
            builder.createLoopExit();
        else
            builder.addSwitchBreak();
        break;
    case glslang::EOpContinue:
        builder.createLoopContinue();
        break;
    case glslang::EOpReturn:
        if (node->getExpression()) {
            const glslang::TType& glslangReturnType = node->getExpression()->getType();
            spv::Id returnId = accessChainLoad(glslangReturnType);
            if (builder.getTypeId(returnId) != currentFunction->getReturnType()) {
                builder.clearAccessChain();
                spv::Id copyId = builder.createVariable(spv::StorageClassFunction, currentFunction->getReturnType());
                builder.setAccessChainLValue(copyId);
                multiTypeStore(glslangReturnType, returnId);
                returnId = builder.createLoad(copyId);
            }
            builder.makeReturn(false, returnId);
        } else
            builder.makeReturn(false);

        builder.clearAccessChain();
        break;

    default:
        assert(0);
        break;
    }

    return false;
}

spv::Id TGlslangToSpvTraverser::createSpvVariable(const glslang::TIntermSymbol* node)
{
    // First, steer off constants, which are not SPIR-V variables, but
    // can still have a mapping to a SPIR-V Id.
    // This includes specialization constants.
    if (node->getQualifier().isConstant()) {
        return createSpvConstant(*node);
    }

    // Now, handle actual variables
    spv::StorageClass storageClass = TranslateStorageClass(node->getType());
    spv::Id spvType = convertGlslangToSpvType(node->getType());

    const bool contains16BitType = node->getType().containsBasicType(glslang::EbtFloat16) ||
                                   node->getType().containsBasicType(glslang::EbtInt16)   ||
                                   node->getType().containsBasicType(glslang::EbtUint16);
    if (contains16BitType) {
        switch (storageClass) {
        case spv::StorageClassInput:
        case spv::StorageClassOutput:
            addPre13Extension(spv::E_SPV_KHR_16bit_storage);
            builder.addCapability(spv::CapabilityStorageInputOutput16);
            break;
        case spv::StorageClassPushConstant:
            addPre13Extension(spv::E_SPV_KHR_16bit_storage);
            builder.addCapability(spv::CapabilityStoragePushConstant16);
            break;
        case spv::StorageClassUniform:
            addPre13Extension(spv::E_SPV_KHR_16bit_storage);
            if (node->getType().getQualifier().storage == glslang::EvqBuffer)
                builder.addCapability(spv::CapabilityStorageUniformBufferBlock16);
            else
                builder.addCapability(spv::CapabilityStorageUniform16);
            break;
        case spv::StorageClassStorageBuffer:
            addPre13Extension(spv::E_SPV_KHR_16bit_storage);
            builder.addCapability(spv::CapabilityStorageUniformBufferBlock16);
            break;
        default:
            break;
        }
    }

    const bool contains8BitType = node->getType().containsBasicType(glslang::EbtInt8)   ||
                                  node->getType().containsBasicType(glslang::EbtUint8);
    if (contains8BitType) {
        if (storageClass == spv::StorageClassPushConstant) {
            builder.addExtension(spv::E_SPV_KHR_8bit_storage);
            builder.addCapability(spv::CapabilityStoragePushConstant8);
        } else if (storageClass == spv::StorageClassUniform) {
            builder.addExtension(spv::E_SPV_KHR_8bit_storage);
            builder.addCapability(spv::CapabilityUniformAndStorageBuffer8BitAccess);
            if (node->getType().getQualifier().storage == glslang::EvqBuffer)
                builder.addCapability(spv::CapabilityStorageBuffer8BitAccess);
        }
    }

    const char* name = node->getName().c_str();
    if (glslang::IsAnonymous(name))
        name = "";

    return builder.createVariable(storageClass, spvType, name);
}

// Return type Id of the sampled type.
spv::Id TGlslangToSpvTraverser::getSampledType(const glslang::TSampler& sampler)
{
    switch (sampler.type) {
        case glslang::EbtFloat:    return builder.makeFloatType(32);
#ifdef AMD_EXTENSIONS
        case glslang::EbtFloat16:
            builder.addExtension(spv::E_SPV_AMD_gpu_shader_half_float_fetch);
            builder.addCapability(spv::CapabilityFloat16ImageAMD);
            return builder.makeFloatType(16);
#endif
        case glslang::EbtInt:      return builder.makeIntType(32);
        case glslang::EbtUint:     return builder.makeUintType(32);
        default:
            assert(0);
            return builder.makeFloatType(32);
    }
}

// If node is a swizzle operation, return the type that should be used if
// the swizzle base is first consumed by another operation, before the swizzle
// is applied.
spv::Id TGlslangToSpvTraverser::getInvertedSwizzleType(const glslang::TIntermTyped& node)
{
    if (node.getAsOperator() &&
        node.getAsOperator()->getOp() == glslang::EOpVectorSwizzle)
        return convertGlslangToSpvType(node.getAsBinaryNode()->getLeft()->getType());
    else
        return spv::NoType;
}

// When inverting a swizzle with a parent op, this function
// will apply the swizzle operation to a completed parent operation.
spv::Id TGlslangToSpvTraverser::createInvertedSwizzle(spv::Decoration precision, const glslang::TIntermTyped& node, spv::Id parentResult)
{
    std::vector<unsigned> swizzle;
    convertSwizzle(*node.getAsBinaryNode()->getRight()->getAsAggregate(), swizzle);
    return builder.createRvalueSwizzle(precision, convertGlslangToSpvType(node.getType()), parentResult, swizzle);
}

// Convert a glslang AST swizzle node to a swizzle vector for building SPIR-V.
void TGlslangToSpvTraverser::convertSwizzle(const glslang::TIntermAggregate& node, std::vector<unsigned>& swizzle)
{
    const glslang::TIntermSequence& swizzleSequence = node.getSequence();
    for (int i = 0; i < (int)swizzleSequence.size(); ++i)
        swizzle.push_back(swizzleSequence[i]->getAsConstantUnion()->getConstArray()[0].getIConst());
}

// Convert from a glslang type to an SPV type, by calling into a
// recursive version of this function. This establishes the inherited
// layout state rooted from the top-level type.
spv::Id TGlslangToSpvTraverser::convertGlslangToSpvType(const glslang::TType& type)
{
    return convertGlslangToSpvType(type, getExplicitLayout(type), type.getQualifier(), false);
}

// Do full recursive conversion of an arbitrary glslang type to a SPIR-V Id.
// explicitLayout can be kept the same throughout the hierarchical recursive walk.
// Mutually recursive with convertGlslangStructToSpvType().
spv::Id TGlslangToSpvTraverser::convertGlslangToSpvType(const glslang::TType& type,
    glslang::TLayoutPacking explicitLayout, const glslang::TQualifier& qualifier, bool lastBufferBlockMember)
{
    spv::Id spvType = spv::NoResult;

    switch (type.getBasicType()) {
    case glslang::EbtVoid:
        spvType = builder.makeVoidType();
        assert (! type.isArray());
        break;
    case glslang::EbtFloat:
        spvType = builder.makeFloatType(32);
        break;
    case glslang::EbtDouble:
        spvType = builder.makeFloatType(64);
        break;
    case glslang::EbtFloat16:
        builder.addCapability(spv::CapabilityFloat16);
#if AMD_EXTENSIONS
        if (builder.getSpvVersion() < glslang::EShTargetSpv_1_3)
            builder.addExtension(spv::E_SPV_AMD_gpu_shader_half_float);
#endif
        spvType = builder.makeFloatType(16);
        break;
    case glslang::EbtBool:
        // "transparent" bool doesn't exist in SPIR-V.  The GLSL convention is
        // a 32-bit int where non-0 means true.
        if (explicitLayout != glslang::ElpNone)
            spvType = builder.makeUintType(32);
        else
            spvType = builder.makeBoolType();
        break;
   case glslang::EbtInt8:
        builder.addCapability(spv::CapabilityInt8);
        spvType = builder.makeIntType(8);
        break;
    case glslang::EbtUint8:
        builder.addCapability(spv::CapabilityInt8);
        spvType = builder.makeUintType(8);
        break;
   case glslang::EbtInt16:
        builder.addCapability(spv::CapabilityInt16);
#ifdef AMD_EXTENSIONS
        if (builder.getSpvVersion() < glslang::EShTargetSpv_1_3)
            builder.addExtension(spv::E_SPV_AMD_gpu_shader_int16);
#endif
        spvType = builder.makeIntType(16);
        break;
    case glslang::EbtUint16:
        builder.addCapability(spv::CapabilityInt16);
#ifdef AMD_EXTENSIONS
        if (builder.getSpvVersion() < glslang::EShTargetSpv_1_3)
            builder.addExtension(spv::E_SPV_AMD_gpu_shader_int16);
#endif
        spvType = builder.makeUintType(16);
        break;
    case glslang::EbtInt:
        spvType = builder.makeIntType(32);
        break;
    case glslang::EbtUint:
        spvType = builder.makeUintType(32);
        break;
    case glslang::EbtInt64:
        spvType = builder.makeIntType(64);
        break;
    case glslang::EbtUint64:
        spvType = builder.makeUintType(64);
        break;
    case glslang::EbtAtomicUint:
        builder.addCapability(spv::CapabilityAtomicStorage);
        spvType = builder.makeUintType(32);
        break;
    case glslang::EbtSampler:
        {
            const glslang::TSampler& sampler = type.getSampler();
            if (sampler.sampler) {
                // pure sampler
                spvType = builder.makeSamplerType();
            } else {
                // an image is present, make its type
                spvType = builder.makeImageType(getSampledType(sampler), TranslateDimensionality(sampler), sampler.shadow, sampler.arrayed, sampler.ms,
                                                sampler.image ? 2 : 1, TranslateImageFormat(type));
                if (sampler.combined) {
                    // already has both image and sampler, make the combined type
                    spvType = builder.makeSampledImageType(spvType);
                }
            }
        }
        break;
    case glslang::EbtStruct:
    case glslang::EbtBlock:
        {
            // If we've seen this struct type, return it
            const glslang::TTypeList* glslangMembers = type.getStruct();

            // Try to share structs for different layouts, but not yet for other
            // kinds of qualification (primarily not yet including interpolant qualification).
            if (! HasNonLayoutQualifiers(type, qualifier))
                spvType = structMap[explicitLayout][qualifier.layoutMatrix][glslangMembers];
            if (spvType != spv::NoResult)
                break;

            // else, we haven't seen it...
            if (type.getBasicType() == glslang::EbtBlock)
                memberRemapper[glslangMembers].resize(glslangMembers->size());
            spvType = convertGlslangStructToSpvType(type, glslangMembers, explicitLayout, qualifier);
        }
        break;
    default:
        assert(0);
        break;
    }

    if (type.isMatrix())
        spvType = builder.makeMatrixType(spvType, type.getMatrixCols(), type.getMatrixRows());
    else {
        // If this variable has a vector element count greater than 1, create a SPIR-V vector
        if (type.getVectorSize() > 1)
            spvType = builder.makeVectorType(spvType, type.getVectorSize());
    }

    if (type.isArray()) {
        int stride = 0;  // keep this 0 unless doing an explicit layout; 0 will mean no decoration, no stride

        // Do all but the outer dimension
        if (type.getArraySizes()->getNumDims() > 1) {
            // We need to decorate array strides for types needing explicit layout, except blocks.
            if (explicitLayout != glslang::ElpNone && type.getBasicType() != glslang::EbtBlock) {
                // Use a dummy glslang type for querying internal strides of
                // arrays of arrays, but using just a one-dimensional array.
                glslang::TType simpleArrayType(type, 0); // deference type of the array
                while (simpleArrayType.getArraySizes()->getNumDims() > 1)
                    simpleArrayType.getArraySizes()->dereference();

                // Will compute the higher-order strides here, rather than making a whole
                // pile of types and doing repetitive recursion on their contents.
                stride = getArrayStride(simpleArrayType, explicitLayout, qualifier.layoutMatrix);
            }

            // make the arrays
            for (int dim = type.getArraySizes()->getNumDims() - 1; dim > 0; --dim) {
                spvType = builder.makeArrayType(spvType, makeArraySizeId(*type.getArraySizes(), dim), stride);
                if (stride > 0)
                    builder.addDecoration(spvType, spv::DecorationArrayStride, stride);
                stride *= type.getArraySizes()->getDimSize(dim);
            }
        } else {
            // single-dimensional array, and don't yet have stride

            // We need to decorate array strides for types needing explicit layout, except blocks.
            if (explicitLayout != glslang::ElpNone && type.getBasicType() != glslang::EbtBlock)
                stride = getArrayStride(type, explicitLayout, qualifier.layoutMatrix);
        }

        // Do the outer dimension, which might not be known for a runtime-sized array.
        // (Unsized arrays that survive through linking will be runtime-sized arrays)
        if (type.isSizedArray())
            spvType = builder.makeArrayType(spvType, makeArraySizeId(*type.getArraySizes(), 0), stride);
        else {
            if (!lastBufferBlockMember) {
                builder.addExtension("SPV_EXT_descriptor_indexing");
                builder.addCapability(spv::CapabilityRuntimeDescriptorArrayEXT);
            }
            spvType = builder.makeRuntimeArray(spvType);
        }
        if (stride > 0)
            builder.addDecoration(spvType, spv::DecorationArrayStride, stride);
    }

    return spvType;
}

// TODO: this functionality should exist at a higher level, in creating the AST
//
// Identify interface members that don't have their required extension turned on.
//
bool TGlslangToSpvTraverser::filterMember(const glslang::TType& member)
{
    auto& extensions = glslangIntermediate->getRequestedExtensions();

    if (member.getFieldName() == "gl_ViewportMask" &&
        extensions.find("GL_NV_viewport_array2") == extensions.end())
        return true;
    if (member.getFieldName() == "gl_SecondaryViewportMaskNV" &&
        extensions.find("GL_NV_stereo_view_rendering") == extensions.end())
        return true;
    if (member.getFieldName() == "gl_SecondaryPositionNV" &&
        extensions.find("GL_NV_stereo_view_rendering") == extensions.end())
        return true;
    if (member.getFieldName() == "gl_PositionPerViewNV" &&
        extensions.find("GL_NVX_multiview_per_view_attributes") == extensions.end())
        return true;
    if (member.getFieldName() == "gl_ViewportMaskPerViewNV" &&
        extensions.find("GL_NVX_multiview_per_view_attributes") == extensions.end())
        return true;

    return false;
};

// Do full recursive conversion of a glslang structure (or block) type to a SPIR-V Id.
// explicitLayout can be kept the same throughout the hierarchical recursive walk.
// Mutually recursive with convertGlslangToSpvType().
spv::Id TGlslangToSpvTraverser::convertGlslangStructToSpvType(const glslang::TType& type,
                                                              const glslang::TTypeList* glslangMembers,
                                                              glslang::TLayoutPacking explicitLayout,
                                                              const glslang::TQualifier& qualifier)
{
    // Create a vector of struct types for SPIR-V to consume
    std::vector<spv::Id> spvMembers;
    int memberDelta = 0;  // how much the member's index changes from glslang to SPIR-V, normally 0, except sometimes for blocks
    for (int i = 0; i < (int)glslangMembers->size(); i++) {
        glslang::TType& glslangMember = *(*glslangMembers)[i].type;
        if (glslangMember.hiddenMember()) {
            ++memberDelta;
            if (type.getBasicType() == glslang::EbtBlock)
                memberRemapper[glslangMembers][i] = -1;
        } else {
            if (type.getBasicType() == glslang::EbtBlock) {
                memberRemapper[glslangMembers][i] = i - memberDelta;
                if (filterMember(glslangMember))
                    continue;
            }
            // modify just this child's view of the qualifier
            glslang::TQualifier memberQualifier = glslangMember.getQualifier();
            InheritQualifiers(memberQualifier, qualifier);

            // manually inherit location
            if (! memberQualifier.hasLocation() && qualifier.hasLocation())
                memberQualifier.layoutLocation = qualifier.layoutLocation;

            // recurse
            bool lastBufferBlockMember = qualifier.storage == glslang::EvqBuffer &&
                                         i == (int)glslangMembers->size() - 1;
            spvMembers.push_back(
                convertGlslangToSpvType(glslangMember, explicitLayout, memberQualifier, lastBufferBlockMember));
        }
    }

    // Make the SPIR-V type
    spv::Id spvType = builder.makeStructType(spvMembers, type.getTypeName().c_str());
    if (! HasNonLayoutQualifiers(type, qualifier))
        structMap[explicitLayout][qualifier.layoutMatrix][glslangMembers] = spvType;

    // Decorate it
    decorateStructType(type, glslangMembers, explicitLayout, qualifier, spvType);

    return spvType;
}

void TGlslangToSpvTraverser::decorateStructType(const glslang::TType& type,
                                                const glslang::TTypeList* glslangMembers,
                                                glslang::TLayoutPacking explicitLayout,
                                                const glslang::TQualifier& qualifier,
                                                spv::Id spvType)
{
    // Name and decorate the non-hidden members
    int offset = -1;
    int locationOffset = 0;  // for use within the members of this struct
    for (int i = 0; i < (int)glslangMembers->size(); i++) {
        glslang::TType& glslangMember = *(*glslangMembers)[i].type;
        int member = i;
        if (type.getBasicType() == glslang::EbtBlock) {
            member = memberRemapper[glslangMembers][i];
            if (filterMember(glslangMember))
                continue;
        }

        // modify just this child's view of the qualifier
        glslang::TQualifier memberQualifier = glslangMember.getQualifier();
        InheritQualifiers(memberQualifier, qualifier);

        // using -1 above to indicate a hidden member
        if (member < 0)
            continue;

        builder.addMemberName(spvType, member, glslangMember.getFieldName().c_str());
        builder.addMemberDecoration(spvType, member,
                                    TranslateLayoutDecoration(glslangMember, memberQualifier.layoutMatrix));
        builder.addMemberDecoration(spvType, member, TranslatePrecisionDecoration(glslangMember));
        // Add interpolation and auxiliary storage decorations only to
        // top-level members of Input and Output storage classes
        if (type.getQualifier().storage == glslang::EvqVaryingIn ||
            type.getQualifier().storage == glslang::EvqVaryingOut) {
            if (type.getBasicType() == glslang::EbtBlock ||
                glslangIntermediate->getSource() == glslang::EShSourceHlsl) {
                builder.addMemberDecoration(spvType, member, TranslateInterpolationDecoration(memberQualifier));
                builder.addMemberDecoration(spvType, member, TranslateAuxiliaryStorageDecoration(memberQualifier));
            }
        }
        builder.addMemberDecoration(spvType, member, TranslateInvariantDecoration(memberQualifier));

        if (type.getBasicType() == glslang::EbtBlock &&
            qualifier.storage == glslang::EvqBuffer) {
            // Add memory decorations only to top-level members of shader storage block
            std::vector<spv::Decoration> memory;
            TranslateMemoryDecoration(memberQualifier, memory);
            for (unsigned int i = 0; i < memory.size(); ++i)
                builder.addMemberDecoration(spvType, member, memory[i]);
        }

        // Location assignment was already completed correctly by the front end,
        // just track whether a member needs to be decorated.
        // Ignore member locations if the container is an array, as that's
        // ill-specified and decisions have been made to not allow this.
        if (! type.isArray() && memberQualifier.hasLocation())
            builder.addMemberDecoration(spvType, member, spv::DecorationLocation, memberQualifier.layoutLocation);

        if (qualifier.hasLocation())      // track for upcoming inheritance
            locationOffset += glslangIntermediate->computeTypeLocationSize(
                                            glslangMember, glslangIntermediate->getStage());

        // component, XFB, others
        if (glslangMember.getQualifier().hasComponent())
            builder.addMemberDecoration(spvType, member, spv::DecorationComponent,
                                        glslangMember.getQualifier().layoutComponent);
        if (glslangMember.getQualifier().hasXfbOffset())
            builder.addMemberDecoration(spvType, member, spv::DecorationOffset,
                                        glslangMember.getQualifier().layoutXfbOffset);
        else if (explicitLayout != glslang::ElpNone) {
            // figure out what to do with offset, which is accumulating
            int nextOffset;
            updateMemberOffset(type, glslangMember, offset, nextOffset, explicitLayout, memberQualifier.layoutMatrix);
            if (offset >= 0)
                builder.addMemberDecoration(spvType, member, spv::DecorationOffset, offset);
            offset = nextOffset;
        }

        if (glslangMember.isMatrix() && explicitLayout != glslang::ElpNone)
            builder.addMemberDecoration(spvType, member, spv::DecorationMatrixStride,
                                        getMatrixStride(glslangMember, explicitLayout, memberQualifier.layoutMatrix));

        // built-in variable decorations
        spv::BuiltIn builtIn = TranslateBuiltInDecoration(glslangMember.getQualifier().builtIn, true);
        if (builtIn != spv::BuiltInMax)
            builder.addMemberDecoration(spvType, member, spv::DecorationBuiltIn, (int)builtIn);

        // nonuniform
        builder.addMemberDecoration(spvType, member, TranslateNonUniformDecoration(glslangMember.getQualifier()));

        if (glslangIntermediate->getHlslFunctionality1() && memberQualifier.semanticName != nullptr) {
            builder.addExtension("SPV_GOOGLE_hlsl_functionality1");
            builder.addMemberDecoration(spvType, member, (spv::Decoration)spv::DecorationHlslSemanticGOOGLE,
                                        memberQualifier.semanticName);
        }

#ifdef NV_EXTENSIONS
        if (builtIn == spv::BuiltInLayer) {
            // SPV_NV_viewport_array2 extension
            if (glslangMember.getQualifier().layoutViewportRelative){
                builder.addMemberDecoration(spvType, member, (spv::Decoration)spv::DecorationViewportRelativeNV);
                builder.addCapability(spv::CapabilityShaderViewportMaskNV);
                builder.addExtension(spv::E_SPV_NV_viewport_array2);
            }
            if (glslangMember.getQualifier().layoutSecondaryViewportRelativeOffset != -2048){
                builder.addMemberDecoration(spvType, member,
                                            (spv::Decoration)spv::DecorationSecondaryViewportRelativeNV,
                                            glslangMember.getQualifier().layoutSecondaryViewportRelativeOffset);
                builder.addCapability(spv::CapabilityShaderStereoViewNV);
                builder.addExtension(spv::E_SPV_NV_stereo_view_rendering);
            }
        }
        if (glslangMember.getQualifier().layoutPassthrough) {
            builder.addMemberDecoration(spvType, member, (spv::Decoration)spv::DecorationPassthroughNV);
            builder.addCapability(spv::CapabilityGeometryShaderPassthroughNV);
            builder.addExtension(spv::E_SPV_NV_geometry_shader_passthrough);
        }
#endif
    }

    // Decorate the structure
    builder.addDecoration(spvType, TranslateLayoutDecoration(type, qualifier.layoutMatrix));
    builder.addDecoration(spvType, TranslateBlockDecoration(type, glslangIntermediate->usingStorageBuffer()));
    if (type.getQualifier().hasStream() && glslangIntermediate->isMultiStream()) {
        builder.addCapability(spv::CapabilityGeometryStreams);
        builder.addDecoration(spvType, spv::DecorationStream, type.getQualifier().layoutStream);
    }
}

// Turn the expression forming the array size into an id.
// This is not quite trivial, because of specialization constants.
// Sometimes, a raw constant is turned into an Id, and sometimes
// a specialization constant expression is.
spv::Id TGlslangToSpvTraverser::makeArraySizeId(const glslang::TArraySizes& arraySizes, int dim)
{
    // First, see if this is sized with a node, meaning a specialization constant:
    glslang::TIntermTyped* specNode = arraySizes.getDimNode(dim);
    if (specNode != nullptr) {
        builder.clearAccessChain();
        specNode->traverse(this);
        return accessChainLoad(specNode->getAsTyped()->getType());
    }

    // Otherwise, need a compile-time (front end) size, get it:
    int size = arraySizes.getDimSize(dim);
    assert(size > 0);
    return builder.makeUintConstant(size);
}

// Wrap the builder's accessChainLoad to:
//  - localize handling of RelaxedPrecision
//  - use the SPIR-V inferred type instead of another conversion of the glslang type
//    (avoids unnecessary work and possible type punning for structures)
//  - do conversion of concrete to abstract type
spv::Id TGlslangToSpvTraverser::accessChainLoad(const glslang::TType& type)
{
    spv::Id nominalTypeId = builder.accessChainGetInferredType();
    spv::Id loadedId = builder.accessChainLoad(TranslatePrecisionDecoration(type),
                                               TranslateNonUniformDecoration(type.getQualifier()), nominalTypeId);

    // Need to convert to abstract types when necessary
    if (type.getBasicType() == glslang::EbtBool) {
        if (builder.isScalarType(nominalTypeId)) {
            // Conversion for bool
            spv::Id boolType = builder.makeBoolType();
            if (nominalTypeId != boolType)
                loadedId = builder.createBinOp(spv::OpINotEqual, boolType, loadedId, builder.makeUintConstant(0));
        } else if (builder.isVectorType(nominalTypeId)) {
            // Conversion for bvec
            int vecSize = builder.getNumTypeComponents(nominalTypeId);
            spv::Id bvecType = builder.makeVectorType(builder.makeBoolType(), vecSize);
            if (nominalTypeId != bvecType)
                loadedId = builder.createBinOp(spv::OpINotEqual, bvecType, loadedId, makeSmearedConstant(builder.makeUintConstant(0), vecSize));
        }
    }

    return loadedId;
}

// Wrap the builder's accessChainStore to:
//  - do conversion of concrete to abstract type
//
// Implicitly uses the existing builder.accessChain as the storage target.
void TGlslangToSpvTraverser::accessChainStore(const glslang::TType& type, spv::Id rvalue)
{
    // Need to convert to abstract types when necessary
    if (type.getBasicType() == glslang::EbtBool) {
        spv::Id nominalTypeId = builder.accessChainGetInferredType();

        if (builder.isScalarType(nominalTypeId)) {
            // Conversion for bool
            spv::Id boolType = builder.makeBoolType();
            if (nominalTypeId != boolType) {
                // keep these outside arguments, for determinant order-of-evaluation
                spv::Id one = builder.makeUintConstant(1);
                spv::Id zero = builder.makeUintConstant(0);
                rvalue = builder.createTriOp(spv::OpSelect, nominalTypeId, rvalue, one, zero);
            } else if (builder.getTypeId(rvalue) != boolType)
                rvalue = builder.createBinOp(spv::OpINotEqual, boolType, rvalue, builder.makeUintConstant(0));
        } else if (builder.isVectorType(nominalTypeId)) {
            // Conversion for bvec
            int vecSize = builder.getNumTypeComponents(nominalTypeId);
            spv::Id bvecType = builder.makeVectorType(builder.makeBoolType(), vecSize);
            if (nominalTypeId != bvecType) {
                // keep these outside arguments, for determinant order-of-evaluation
                spv::Id one = makeSmearedConstant(builder.makeUintConstant(1), vecSize);
                spv::Id zero = makeSmearedConstant(builder.makeUintConstant(0), vecSize);
                rvalue = builder.createTriOp(spv::OpSelect, nominalTypeId, rvalue, one, zero);
            } else if (builder.getTypeId(rvalue) != bvecType)
                rvalue = builder.createBinOp(spv::OpINotEqual, bvecType, rvalue,
                                             makeSmearedConstant(builder.makeUintConstant(0), vecSize));
        }
    }

    builder.accessChainStore(rvalue);
}

// For storing when types match at the glslang level, but not might match at the
// SPIR-V level.
//
// This especially happens when a single glslang type expands to multiple
// SPIR-V types, like a struct that is used in a member-undecorated way as well
// as in a member-decorated way.
//
// NOTE: This function can handle any store request; if it's not special it
// simplifies to a simple OpStore.
//
// Implicitly uses the existing builder.accessChain as the storage target.
void TGlslangToSpvTraverser::multiTypeStore(const glslang::TType& type, spv::Id rValue)
{
    // we only do the complex path here if it's an aggregate
    if (! type.isStruct() && ! type.isArray()) {
        accessChainStore(type, rValue);
        return;
    }

    // and, it has to be a case of type aliasing
    spv::Id rType = builder.getTypeId(rValue);
    spv::Id lValue = builder.accessChainGetLValue();
    spv::Id lType = builder.getContainedTypeId(builder.getTypeId(lValue));
    if (lType == rType) {
        accessChainStore(type, rValue);
        return;
    }

    // Recursively (as needed) copy an aggregate type to a different aggregate type,
    // where the two types were the same type in GLSL. This requires member
    // by member copy, recursively.

    // If an array, copy element by element.
    if (type.isArray()) {
        glslang::TType glslangElementType(type, 0);
        spv::Id elementRType = builder.getContainedTypeId(rType);
        for (int index = 0; index < type.getOuterArraySize(); ++index) {
            // get the source member
            spv::Id elementRValue = builder.createCompositeExtract(rValue, elementRType, index);

            // set up the target storage
            builder.clearAccessChain();
            builder.setAccessChainLValue(lValue);
            builder.accessChainPush(builder.makeIntConstant(index));

            // store the member
            multiTypeStore(glslangElementType, elementRValue);
        }
    } else {
        assert(type.isStruct());

        // loop over structure members
        const glslang::TTypeList& members = *type.getStruct();
        for (int m = 0; m < (int)members.size(); ++m) {
            const glslang::TType& glslangMemberType = *members[m].type;

            // get the source member
            spv::Id memberRType = builder.getContainedTypeId(rType, m);
            spv::Id memberRValue = builder.createCompositeExtract(rValue, memberRType, m);

            // set up the target storage
            builder.clearAccessChain();
            builder.setAccessChainLValue(lValue);
            builder.accessChainPush(builder.makeIntConstant(m));

            // store the member
            multiTypeStore(glslangMemberType, memberRValue);
        }
    }
}

// Decide whether or not this type should be
// decorated with offsets and strides, and if so
// whether std140 or std430 rules should be applied.
glslang::TLayoutPacking TGlslangToSpvTraverser::getExplicitLayout(const glslang::TType& type) const
{
    // has to be a block
    if (type.getBasicType() != glslang::EbtBlock)
        return glslang::ElpNone;

    // has to be a uniform or buffer block
    if (type.getQualifier().storage != glslang::EvqUniform &&
        type.getQualifier().storage != glslang::EvqBuffer)
        return glslang::ElpNone;

    // return the layout to use
    switch (type.getQualifier().layoutPacking) {
    case glslang::ElpStd140:
    case glslang::ElpStd430:
        return type.getQualifier().layoutPacking;
    default:
        return glslang::ElpNone;
    }
}

// Given an array type, returns the integer stride required for that array
int TGlslangToSpvTraverser::getArrayStride(const glslang::TType& arrayType, glslang::TLayoutPacking explicitLayout, glslang::TLayoutMatrix matrixLayout)
{
    int size;
    int stride;
    glslangIntermediate->getBaseAlignment(arrayType, size, stride, explicitLayout == glslang::ElpStd140, matrixLayout == glslang::ElmRowMajor);

    return stride;
}

// Given a matrix type, or array (of array) of matrixes type, returns the integer stride required for that matrix
// when used as a member of an interface block
int TGlslangToSpvTraverser::getMatrixStride(const glslang::TType& matrixType, glslang::TLayoutPacking explicitLayout, glslang::TLayoutMatrix matrixLayout)
{
    glslang::TType elementType;
    elementType.shallowCopy(matrixType);
    elementType.clearArraySizes();

    int size;
    int stride;
    glslangIntermediate->getBaseAlignment(elementType, size, stride, explicitLayout == glslang::ElpStd140, matrixLayout == glslang::ElmRowMajor);

    return stride;
}

// Given a member type of a struct, realign the current offset for it, and compute
// the next (not yet aligned) offset for the next member, which will get aligned
// on the next call.
// 'currentOffset' should be passed in already initialized, ready to modify, and reflecting
// the migration of data from nextOffset -> currentOffset.  It should be -1 on the first call.
// -1 means a non-forced member offset (no decoration needed).
void TGlslangToSpvTraverser::updateMemberOffset(const glslang::TType& structType, const glslang::TType& memberType, int& currentOffset, int& nextOffset,
                                                glslang::TLayoutPacking explicitLayout, glslang::TLayoutMatrix matrixLayout)
{
    // this will get a positive value when deemed necessary
    nextOffset = -1;

    // override anything in currentOffset with user-set offset
    if (memberType.getQualifier().hasOffset())
        currentOffset = memberType.getQualifier().layoutOffset;

    // It could be that current linker usage in glslang updated all the layoutOffset,
    // in which case the following code does not matter.  But, that's not quite right
    // once cross-compilation unit GLSL validation is done, as the original user
    // settings are needed in layoutOffset, and then the following will come into play.

    if (explicitLayout == glslang::ElpNone) {
        if (! memberType.getQualifier().hasOffset())
            currentOffset = -1;

        return;
    }

    // Getting this far means we need explicit offsets
    if (currentOffset < 0)
        currentOffset = 0;

    // Now, currentOffset is valid (either 0, or from a previous nextOffset),
    // but possibly not yet correctly aligned.

    int memberSize;
    int dummyStride;
    int memberAlignment = glslangIntermediate->getBaseAlignment(memberType, memberSize, dummyStride, explicitLayout == glslang::ElpStd140, matrixLayout == glslang::ElmRowMajor);

    // Adjust alignment for HLSL rules
    // TODO: make this consistent in early phases of code:
    //       adjusting this late means inconsistencies with earlier code, which for reflection is an issue
    // Until reflection is brought in sync with these adjustments, don't apply to $Global,
    // which is the most likely to rely on reflection, and least likely to rely implicit layouts
    if (glslangIntermediate->usingHlslOFfsets() &&
        ! memberType.isArray() && memberType.isVector() && structType.getTypeName().compare("$Global") != 0) {
        int dummySize;
        int componentAlignment = glslangIntermediate->getBaseAlignmentScalar(memberType, dummySize);
        if (componentAlignment <= 4)
            memberAlignment = componentAlignment;
    }

    // Bump up to member alignment
    glslang::RoundToPow2(currentOffset, memberAlignment);

    // Bump up to vec4 if there is a bad straddle
    if (glslangIntermediate->improperStraddle(memberType, memberSize, currentOffset))
        glslang::RoundToPow2(currentOffset, 16);

    nextOffset = currentOffset + memberSize;
}

void TGlslangToSpvTraverser::declareUseOfStructMember(const glslang::TTypeList& members, int glslangMember)
{
    const glslang::TBuiltInVariable glslangBuiltIn = members[glslangMember].type->getQualifier().builtIn;
    switch (glslangBuiltIn)
    {
    case glslang::EbvClipDistance:
    case glslang::EbvCullDistance:
    case glslang::EbvPointSize:
#ifdef NV_EXTENSIONS
    case glslang::EbvViewportMaskNV:
    case glslang::EbvSecondaryPositionNV:
    case glslang::EbvSecondaryViewportMaskNV:
    case glslang::EbvPositionPerViewNV:
    case glslang::EbvViewportMaskPerViewNV:
#endif
        // Generate the associated capability.  Delegate to TranslateBuiltInDecoration.
        // Alternately, we could just call this for any glslang built-in, since the
        // capability already guards against duplicates.
        TranslateBuiltInDecoration(glslangBuiltIn, false);
        break;
    default:
        // Capabilities were already generated when the struct was declared.
        break;
    }
}

bool TGlslangToSpvTraverser::isShaderEntryPoint(const glslang::TIntermAggregate* node)
{
    return node->getName().compare(glslangIntermediate->getEntryPointMangledName().c_str()) == 0;
}

// Does parameter need a place to keep writes, separate from the original?
// Assumes called after originalParam(), which filters out block/buffer/opaque-based
// qualifiers such that we should have only in/out/inout/constreadonly here.
bool TGlslangToSpvTraverser::writableParam(glslang::TStorageQualifier qualifier) const
{
    assert(qualifier == glslang::EvqIn ||
           qualifier == glslang::EvqOut ||
           qualifier == glslang::EvqInOut ||
           qualifier == glslang::EvqConstReadOnly);
    return qualifier != glslang::EvqConstReadOnly;
}

// Is parameter pass-by-original?
bool TGlslangToSpvTraverser::originalParam(glslang::TStorageQualifier qualifier, const glslang::TType& paramType,
                                           bool implicitThisParam)
{
    if (implicitThisParam)                                                                     // implicit this
        return true;
    if (glslangIntermediate->getSource() == glslang::EShSourceHlsl)
        return paramType.getBasicType() == glslang::EbtBlock;
    return paramType.containsOpaque() ||                                                       // sampler, etc.
           (paramType.getBasicType() == glslang::EbtBlock && qualifier == glslang::EvqBuffer); // SSBO
}

// Make all the functions, skeletally, without actually visiting their bodies.
void TGlslangToSpvTraverser::makeFunctions(const glslang::TIntermSequence& glslFunctions)
{
    const auto getParamDecorations = [](std::vector<spv::Decoration>& decorations, const glslang::TType& type) {
        spv::Decoration paramPrecision = TranslatePrecisionDecoration(type);
        if (paramPrecision != spv::NoPrecision)
            decorations.push_back(paramPrecision);
        TranslateMemoryDecoration(type.getQualifier(), decorations);
    };

    for (int f = 0; f < (int)glslFunctions.size(); ++f) {
        glslang::TIntermAggregate* glslFunction = glslFunctions[f]->getAsAggregate();
        if (! glslFunction || glslFunction->getOp() != glslang::EOpFunction || isShaderEntryPoint(glslFunction))
            continue;

        // We're on a user function.  Set up the basic interface for the function now,
        // so that it's available to call.  Translating the body will happen later.
        //
        // Typically (except for a "const in" parameter), an address will be passed to the
        // function.  What it is an address of varies:
        //
        // - "in" parameters not marked as "const" can be written to without modifying the calling
        //   argument so that write needs to be to a copy, hence the address of a copy works.
        //
        // - "const in" parameters can just be the r-value, as no writes need occur.
        //
        // - "out" and "inout" arguments can't be done as pointers to the calling argument, because
        //   GLSL has copy-in/copy-out semantics.  They can be handled though with a pointer to a copy.

        std::vector<spv::Id> paramTypes;
        std::vector<std::vector<spv::Decoration>> paramDecorations; // list of decorations per parameter
        glslang::TIntermSequence& parameters = glslFunction->getSequence()[0]->getAsAggregate()->getSequence();

        bool implicitThis = (int)parameters.size() > 0 && parameters[0]->getAsSymbolNode()->getName() ==
                                                          glslangIntermediate->implicitThisName;

        paramDecorations.resize(parameters.size());
        for (int p = 0; p < (int)parameters.size(); ++p) {
            const glslang::TType& paramType = parameters[p]->getAsTyped()->getType();
            spv::Id typeId = convertGlslangToSpvType(paramType);
            if (originalParam(paramType.getQualifier().storage, paramType, implicitThis && p == 0))
                typeId = builder.makePointer(TranslateStorageClass(paramType), typeId);
            else if (writableParam(paramType.getQualifier().storage))
                typeId = builder.makePointer(spv::StorageClassFunction, typeId);
            else
                rValueParameters.insert(parameters[p]->getAsSymbolNode()->getId());
            getParamDecorations(paramDecorations[p], paramType);
            paramTypes.push_back(typeId);
        }

        spv::Block* functionBlock;
        spv::Function *function = builder.makeFunctionEntry(TranslatePrecisionDecoration(glslFunction->getType()),
                                                            convertGlslangToSpvType(glslFunction->getType()),
                                                            glslFunction->getName().c_str(), paramTypes,
                                                            paramDecorations, &functionBlock);
        if (implicitThis)
            function->setImplicitThis();

        // Track function to emit/call later
        functionMap[glslFunction->getName().c_str()] = function;

        // Set the parameter id's
        for (int p = 0; p < (int)parameters.size(); ++p) {
            symbolValues[parameters[p]->getAsSymbolNode()->getId()] = function->getParamId(p);
            // give a name too
            builder.addName(function->getParamId(p), parameters[p]->getAsSymbolNode()->getName().c_str());
        }
    }
}

// Process all the initializers, while skipping the functions and link objects
void TGlslangToSpvTraverser::makeGlobalInitializers(const glslang::TIntermSequence& initializers)
{
    builder.setBuildPoint(shaderEntry->getLastBlock());
    for (int i = 0; i < (int)initializers.size(); ++i) {
        glslang::TIntermAggregate* initializer = initializers[i]->getAsAggregate();
        if (initializer && initializer->getOp() != glslang::EOpFunction && initializer->getOp() != glslang::EOpLinkerObjects) {

            // We're on a top-level node that's not a function.  Treat as an initializer, whose
            // code goes into the beginning of the entry point.
            initializer->traverse(this);
        }
    }
}

// Process all the functions, while skipping initializers.
void TGlslangToSpvTraverser::visitFunctions(const glslang::TIntermSequence& glslFunctions)
{
    for (int f = 0; f < (int)glslFunctions.size(); ++f) {
        glslang::TIntermAggregate* node = glslFunctions[f]->getAsAggregate();
        if (node && (node->getOp() == glslang::EOpFunction || node->getOp() == glslang::EOpLinkerObjects))
            node->traverse(this);
    }
}

void TGlslangToSpvTraverser::handleFunctionEntry(const glslang::TIntermAggregate* node)
{
    // SPIR-V functions should already be in the functionMap from the prepass
    // that called makeFunctions().
    currentFunction = functionMap[node->getName().c_str()];
    spv::Block* functionBlock = currentFunction->getEntryBlock();
    builder.setBuildPoint(functionBlock);
}

void TGlslangToSpvTraverser::translateArguments(const glslang::TIntermAggregate& node, std::vector<spv::Id>& arguments)
{
    const glslang::TIntermSequence& glslangArguments = node.getSequence();

    glslang::TSampler sampler = {};
    bool cubeCompare = false;
#ifdef AMD_EXTENSIONS
    bool f16ShadowCompare = false;
#endif
    if (node.isTexture() || node.isImage()) {
        sampler = glslangArguments[0]->getAsTyped()->getType().getSampler();
        cubeCompare = sampler.dim == glslang::EsdCube && sampler.arrayed && sampler.shadow;
#ifdef AMD_EXTENSIONS
        f16ShadowCompare = sampler.shadow && glslangArguments[1]->getAsTyped()->getType().getBasicType() == glslang::EbtFloat16;
#endif
    }

    for (int i = 0; i < (int)glslangArguments.size(); ++i) {
        builder.clearAccessChain();
        glslangArguments[i]->traverse(this);

        // Special case l-value operands
        bool lvalue = false;
        switch (node.getOp()) {
        case glslang::EOpImageAtomicAdd:
        case glslang::EOpImageAtomicMin:
        case glslang::EOpImageAtomicMax:
        case glslang::EOpImageAtomicAnd:
        case glslang::EOpImageAtomicOr:
        case glslang::EOpImageAtomicXor:
        case glslang::EOpImageAtomicExchange:
        case glslang::EOpImageAtomicCompSwap:
            if (i == 0)
                lvalue = true;
            break;
        case glslang::EOpSparseImageLoad:
            if ((sampler.ms && i == 3) || (! sampler.ms && i == 2))
                lvalue = true;
            break;
#ifdef AMD_EXTENSIONS
        case glslang::EOpSparseTexture:
            if (((cubeCompare || f16ShadowCompare) && i == 3) || (! (cubeCompare || f16ShadowCompare) && i == 2))
                lvalue = true;
            break;
        case glslang::EOpSparseTextureClamp:
            if (((cubeCompare || f16ShadowCompare) && i == 4) || (! (cubeCompare || f16ShadowCompare) && i == 3))
                lvalue = true;
            break;
        case glslang::EOpSparseTextureLod:
        case glslang::EOpSparseTextureOffset:
            if  ((f16ShadowCompare && i == 4) || (! f16ShadowCompare && i == 3))
                lvalue = true;
            break;
#else
        case glslang::EOpSparseTexture:
            if ((cubeCompare && i == 3) || (! cubeCompare && i == 2))
                lvalue = true;
            break;
        case glslang::EOpSparseTextureClamp:
            if ((cubeCompare && i == 4) || (! cubeCompare && i == 3))
                lvalue = true;
            break;
        case glslang::EOpSparseTextureLod:
        case glslang::EOpSparseTextureOffset:
            if (i == 3)
                lvalue = true;
            break;
#endif
        case glslang::EOpSparseTextureFetch:
            if ((sampler.dim != glslang::EsdRect && i == 3) || (sampler.dim == glslang::EsdRect && i == 2))
                lvalue = true;
            break;
        case glslang::EOpSparseTextureFetchOffset:
            if ((sampler.dim != glslang::EsdRect && i == 4) || (sampler.dim == glslang::EsdRect && i == 3))
                lvalue = true;
            break;
#ifdef AMD_EXTENSIONS
        case glslang::EOpSparseTextureLodOffset:
        case glslang::EOpSparseTextureGrad:
        case glslang::EOpSparseTextureOffsetClamp:
            if ((f16ShadowCompare && i == 5) || (! f16ShadowCompare && i == 4))
                lvalue = true;
            break;
        case glslang::EOpSparseTextureGradOffset:
        case glslang::EOpSparseTextureGradClamp:
            if ((f16ShadowCompare && i == 6) || (! f16ShadowCompare && i == 5))
                lvalue = true;
            break;
        case glslang::EOpSparseTextureGradOffsetClamp:
            if ((f16ShadowCompare && i == 7) || (! f16ShadowCompare && i == 6))
                lvalue = true;
            break;
#else
        case glslang::EOpSparseTextureLodOffset:
        case glslang::EOpSparseTextureGrad:
        case glslang::EOpSparseTextureOffsetClamp:
            if (i == 4)
                lvalue = true;
            break;
        case glslang::EOpSparseTextureGradOffset:
        case glslang::EOpSparseTextureGradClamp:
            if (i == 5)
                lvalue = true;
            break;
        case glslang::EOpSparseTextureGradOffsetClamp:
            if (i == 6)
                lvalue = true;
            break;
#endif
        case glslang::EOpSparseTextureGather:
            if ((sampler.shadow && i == 3) || (! sampler.shadow && i == 2))
                lvalue = true;
            break;
        case glslang::EOpSparseTextureGatherOffset:
        case glslang::EOpSparseTextureGatherOffsets:
            if ((sampler.shadow && i == 4) || (! sampler.shadow && i == 3))
                lvalue = true;
            break;
#ifdef AMD_EXTENSIONS
        case glslang::EOpSparseTextureGatherLod:
            if (i == 3)
                lvalue = true;
            break;
        case glslang::EOpSparseTextureGatherLodOffset:
        case glslang::EOpSparseTextureGatherLodOffsets:
            if (i == 4)
                lvalue = true;
            break;
        case glslang::EOpSparseImageLoadLod:
            if (i == 3)
                lvalue = true;
            break;
#endif
        default:
            break;
        }

        if (lvalue)
            arguments.push_back(builder.accessChainGetLValue());
        else
            arguments.push_back(accessChainLoad(glslangArguments[i]->getAsTyped()->getType()));
    }
}

void TGlslangToSpvTraverser::translateArguments(glslang::TIntermUnary& node, std::vector<spv::Id>& arguments)
{
    builder.clearAccessChain();
    node.getOperand()->traverse(this);
    arguments.push_back(accessChainLoad(node.getOperand()->getType()));
}

spv::Id TGlslangToSpvTraverser::createImageTextureFunctionCall(glslang::TIntermOperator* node)
{
    if (! node->isImage() && ! node->isTexture())
        return spv::NoResult;

    builder.setLine(node->getLoc().line);

    // Process a GLSL texturing op (will be SPV image)
    const glslang::TSampler sampler = node->getAsAggregate() ? node->getAsAggregate()->getSequence()[0]->getAsTyped()->getType().getSampler()
                                                             : node->getAsUnaryNode()->getOperand()->getAsTyped()->getType().getSampler();
#ifdef AMD_EXTENSIONS
    bool f16ShadowCompare = (sampler.shadow && node->getAsAggregate())
                                ? node->getAsAggregate()->getSequence()[1]->getAsTyped()->getType().getBasicType() == glslang::EbtFloat16
                                : false;
#endif

    std::vector<spv::Id> arguments;
    if (node->getAsAggregate())
        translateArguments(*node->getAsAggregate(), arguments);
    else
        translateArguments(*node->getAsUnaryNode(), arguments);
    spv::Decoration precision = TranslatePrecisionDecoration(node->getOperationPrecision());

    spv::Builder::TextureParameters params = { };
    params.sampler = arguments[0];

    glslang::TCrackedTextureOp cracked;
    node->crackTexture(sampler, cracked);

    const bool isUnsignedResult = node->getType().getBasicType() == glslang::EbtUint;

    // Check for queries
    if (cracked.query) {
        // OpImageQueryLod works on a sampled image, for other queries the image has to be extracted first
        if (node->getOp() != glslang::EOpTextureQueryLod && builder.isSampledImage(params.sampler))
            params.sampler = builder.createUnaryOp(spv::OpImage, builder.getImageType(params.sampler), params.sampler);

        switch (node->getOp()) {
        case glslang::EOpImageQuerySize:
        case glslang::EOpTextureQuerySize:
            if (arguments.size() > 1) {
                params.lod = arguments[1];
                return builder.createTextureQueryCall(spv::OpImageQuerySizeLod, params, isUnsignedResult);
            } else
                return builder.createTextureQueryCall(spv::OpImageQuerySize, params, isUnsignedResult);
        case glslang::EOpImageQuerySamples:
        case glslang::EOpTextureQuerySamples:
            return builder.createTextureQueryCall(spv::OpImageQuerySamples, params, isUnsignedResult);
        case glslang::EOpTextureQueryLod:
            params.coords = arguments[1];
            return builder.createTextureQueryCall(spv::OpImageQueryLod, params, isUnsignedResult);
        case glslang::EOpTextureQueryLevels:
            return builder.createTextureQueryCall(spv::OpImageQueryLevels, params, isUnsignedResult);
        case glslang::EOpSparseTexelsResident:
            return builder.createUnaryOp(spv::OpImageSparseTexelsResident, builder.makeBoolType(), arguments[0]);
        default:
            assert(0);
            break;
        }
    }

    int components = node->getType().getVectorSize();

    if (node->getOp() == glslang::EOpTextureFetch) {
        // These must produce 4 components, per SPIR-V spec.  We'll add a conversion constructor if needed.
        // This will only happen through the HLSL path for operator[], so we do not have to handle e.g.
        // the EOpTexture/Proj/Lod/etc family.  It would be harmless to do so, but would need more logic
        // here around e.g. which ones return scalars or other types.
        components = 4;
    }

    glslang::TType returnType(node->getType().getBasicType(), glslang::EvqTemporary, components);

    auto resultType = [&returnType,this]{ return convertGlslangToSpvType(returnType); };

    // Check for image functions other than queries
    if (node->isImage()) {
        std::vector<spv::IdImmediate> operands;
        auto opIt = arguments.begin();
        spv::IdImmediate image = { true, *(opIt++) };
        operands.push_back(image);

        // Handle subpass operations
        // TODO: GLSL should change to have the "MS" only on the type rather than the
        // built-in function.
        if (cracked.subpass) {
            // add on the (0,0) coordinate
            spv::Id zero = builder.makeIntConstant(0);
            std::vector<spv::Id> comps;
            comps.push_back(zero);
            comps.push_back(zero);
            spv::IdImmediate coord = { true,
                builder.makeCompositeConstant(builder.makeVectorType(builder.makeIntType(32), 2), comps) };
            operands.push_back(coord);
            if (sampler.ms) {
                spv::IdImmediate imageOperands = { false, spv::ImageOperandsSampleMask };
                operands.push_back(imageOperands);
                spv::IdImmediate imageOperand = { true, *(opIt++) };
                operands.push_back(imageOperand);
            }
            spv::Id result = builder.createOp(spv::OpImageRead, resultType(), operands);
            builder.setPrecision(result, precision);
            return result;
        }

        spv::IdImmediate coord = { true, *(opIt++) };
        operands.push_back(coord);
#ifdef AMD_EXTENSIONS
        if (node->getOp() == glslang::EOpImageLoad || node->getOp() == glslang::EOpImageLoadLod) {
#else
        if (node->getOp() == glslang::EOpImageLoad) {
#endif
            if (sampler.ms) {
                spv::IdImmediate imageOperands = { false, spv::ImageOperandsSampleMask };
                operands.push_back(imageOperands);
                spv::IdImmediate imageOperand = { true, *opIt };
                operands.push_back(imageOperand);
#ifdef AMD_EXTENSIONS
            } else if (cracked.lod) {
                builder.addExtension(spv::E_SPV_AMD_shader_image_load_store_lod);
                builder.addCapability(spv::CapabilityImageReadWriteLodAMD);

                spv::IdImmediate imageOperands = { false, spv::ImageOperandsLodMask };
                operands.push_back(imageOperands);
                spv::IdImmediate imageOperand = { true, *opIt };
                operands.push_back(imageOperand);
#endif
            }
            if (builder.getImageTypeFormat(builder.getImageType(operands.front().word)) == spv::ImageFormatUnknown)
                builder.addCapability(spv::CapabilityStorageImageReadWithoutFormat);

            std::vector<spv::Id> result(1, builder.createOp(spv::OpImageRead, resultType(), operands));
            builder.setPrecision(result[0], precision);

            // If needed, add a conversion constructor to the proper size.
            if (components != node->getType().getVectorSize())
                result[0] = builder.createConstructor(precision, result, convertGlslangToSpvType(node->getType()));

            return result[0];
#ifdef AMD_EXTENSIONS
        } else if (node->getOp() == glslang::EOpImageStore || node->getOp() == glslang::EOpImageStoreLod) {
#else
        } else if (node->getOp() == glslang::EOpImageStore) {
#endif
            if (sampler.ms) {
                spv::IdImmediate texel = { true, *(opIt + 1) };
                operands.push_back(texel);
                spv::IdImmediate imageOperands = { false, spv::ImageOperandsSampleMask };
                operands.push_back(imageOperands);
                spv::IdImmediate imageOperand = { true, *opIt };
                operands.push_back(imageOperand);
#ifdef AMD_EXTENSIONS
            } else if (cracked.lod) {
                builder.addExtension(spv::E_SPV_AMD_shader_image_load_store_lod);
                builder.addCapability(spv::CapabilityImageReadWriteLodAMD);

                spv::IdImmediate texel = { true, *(opIt + 1) };
                operands.push_back(texel);
                spv::IdImmediate imageOperands = { false, spv::ImageOperandsLodMask };
                operands.push_back(imageOperands);
                spv::IdImmediate imageOperand = { true, *opIt };
                operands.push_back(imageOperand);
#endif
            } else {
                spv::IdImmediate texel = { true, *opIt };
                operands.push_back(texel);
            }
            builder.createNoResultOp(spv::OpImageWrite, operands);
            if (builder.getImageTypeFormat(builder.getImageType(operands.front().word)) == spv::ImageFormatUnknown)
                builder.addCapability(spv::CapabilityStorageImageWriteWithoutFormat);
            return spv::NoResult;
#ifdef AMD_EXTENSIONS
        } else if (node->getOp() == glslang::EOpSparseImageLoad || node->getOp() == glslang::EOpSparseImageLoadLod) {
#else
        } else if (node->getOp() == glslang::EOpSparseImageLoad) {
#endif
            builder.addCapability(spv::CapabilitySparseResidency);
            if (builder.getImageTypeFormat(builder.getImageType(operands.front().word)) == spv::ImageFormatUnknown)
                builder.addCapability(spv::CapabilityStorageImageReadWithoutFormat);

            if (sampler.ms) {
                spv::IdImmediate imageOperands = { false, spv::ImageOperandsSampleMask };
                operands.push_back(imageOperands);
                spv::IdImmediate imageOperand = { true, *opIt++ };
                operands.push_back(imageOperand);
#ifdef AMD_EXTENSIONS
            } else if (cracked.lod) {
                builder.addExtension(spv::E_SPV_AMD_shader_image_load_store_lod);
                builder.addCapability(spv::CapabilityImageReadWriteLodAMD);

                spv::IdImmediate imageOperands = { false, spv::ImageOperandsLodMask };
                operands.push_back(imageOperands);
                spv::IdImmediate imageOperand = { true, *opIt++ };
                operands.push_back(imageOperand);
#endif
            }

            // Create the return type that was a special structure
            spv::Id texelOut = *opIt;
            spv::Id typeId0 = resultType();
            spv::Id typeId1 = builder.getDerefTypeId(texelOut);
            spv::Id resultTypeId = builder.makeStructResultType(typeId0, typeId1);

            spv::Id resultId = builder.createOp(spv::OpImageSparseRead, resultTypeId, operands);

            // Decode the return type
            builder.createStore(builder.createCompositeExtract(resultId, typeId1, 1), texelOut);
            return builder.createCompositeExtract(resultId, typeId0, 0);
        } else {
            // Process image atomic operations

            // GLSL "IMAGE_PARAMS" will involve in constructing an image texel pointer and this pointer,
            // as the first source operand, is required by SPIR-V atomic operations.
            // For non-MS, the sample value should be 0
            spv::IdImmediate sample = { true, sampler.ms ? *(opIt++) : builder.makeUintConstant(0) };
            operands.push_back(sample);

            spv::Id resultTypeId = builder.makePointer(spv::StorageClassImage, resultType());
            spv::Id pointer = builder.createOp(spv::OpImageTexelPointer, resultTypeId, operands);

            std::vector<spv::Id> operands;
            operands.push_back(pointer);
            for (; opIt != arguments.end(); ++opIt)
                operands.push_back(*opIt);

            return createAtomicOperation(node->getOp(), precision, resultType(), operands, node->getBasicType());
        }
    }

#ifdef AMD_EXTENSIONS
    // Check for fragment mask functions other than queries
    if (cracked.fragMask) {
        assert(sampler.ms);

        auto opIt = arguments.begin();
        std::vector<spv::Id> operands;

        // Extract the image if necessary
        if (builder.isSampledImage(params.sampler))
            params.sampler = builder.createUnaryOp(spv::OpImage, builder.getImageType(params.sampler), params.sampler);

        operands.push_back(params.sampler);
        ++opIt;

        if (sampler.isSubpass()) {
            // add on the (0,0) coordinate
            spv::Id zero = builder.makeIntConstant(0);
            std::vector<spv::Id> comps;
            comps.push_back(zero);
            comps.push_back(zero);
            operands.push_back(builder.makeCompositeConstant(builder.makeVectorType(builder.makeIntType(32), 2), comps));
        }

        for (; opIt != arguments.end(); ++opIt)
            operands.push_back(*opIt);

        spv::Op fragMaskOp = spv::OpNop;
        if (node->getOp() == glslang::EOpFragmentMaskFetch)
            fragMaskOp = spv::OpFragmentMaskFetchAMD;
        else if (node->getOp() == glslang::EOpFragmentFetch)
            fragMaskOp = spv::OpFragmentFetchAMD;

        builder.addExtension(spv::E_SPV_AMD_shader_fragment_mask);
        builder.addCapability(spv::CapabilityFragmentMaskAMD);
        return builder.createOp(fragMaskOp, resultType(), operands);
    }
#endif

    // Check for texture functions other than queries
    bool sparse = node->isSparseTexture();
    bool cubeCompare = sampler.dim == glslang::EsdCube && sampler.arrayed && sampler.shadow;

    // check for bias argument
    bool bias = false;
#ifdef AMD_EXTENSIONS
    if (! cracked.lod && ! cracked.grad && ! cracked.fetch && ! cubeCompare) {
#else
    if (! cracked.lod && ! cracked.gather && ! cracked.grad && ! cracked.fetch && ! cubeCompare) {
#endif
        int nonBiasArgCount = 2;
#ifdef AMD_EXTENSIONS
        if (cracked.gather)
            ++nonBiasArgCount; // comp argument should be present when bias argument is present

        if (f16ShadowCompare)
            ++nonBiasArgCount;
#endif
        if (cracked.offset)
            ++nonBiasArgCount;
#ifdef AMD_EXTENSIONS
        else if (cracked.offsets)
            ++nonBiasArgCount;
#endif
        if (cracked.grad)
            nonBiasArgCount += 2;
        if (cracked.lodClamp)
            ++nonBiasArgCount;
        if (sparse)
            ++nonBiasArgCount;

        if ((int)arguments.size() > nonBiasArgCount)
            bias = true;
    }

    // See if the sampler param should really be just the SPV image part
    if (cracked.fetch) {
        // a fetch needs to have the image extracted first
        if (builder.isSampledImage(params.sampler))
            params.sampler = builder.createUnaryOp(spv::OpImage, builder.getImageType(params.sampler), params.sampler);
    }

#ifdef AMD_EXTENSIONS
    if (cracked.gather) {
        const auto& sourceExtensions = glslangIntermediate->getRequestedExtensions();
        if (bias || cracked.lod ||
            sourceExtensions.find(glslang::E_GL_AMD_texture_gather_bias_lod) != sourceExtensions.end()) {
            builder.addExtension(spv::E_SPV_AMD_texture_gather_bias_lod);
            builder.addCapability(spv::CapabilityImageGatherBiasLodAMD);
        }
    }
#endif

    // set the rest of the arguments

    params.coords = arguments[1];
    int extraArgs = 0;
    bool noImplicitLod = false;

    // sort out where Dref is coming from
#ifdef AMD_EXTENSIONS
    if (cubeCompare || f16ShadowCompare) {
#else
    if (cubeCompare) {
#endif
        params.Dref = arguments[2];
        ++extraArgs;
    } else if (sampler.shadow && cracked.gather) {
        params.Dref = arguments[2];
        ++extraArgs;
    } else if (sampler.shadow) {
        std::vector<spv::Id> indexes;
        int dRefComp;
        if (cracked.proj)
            dRefComp = 2;  // "The resulting 3rd component of P in the shadow forms is used as Dref"
        else
            dRefComp = builder.getNumComponents(params.coords) - 1;
        indexes.push_back(dRefComp);
        params.Dref = builder.createCompositeExtract(params.coords, builder.getScalarTypeId(builder.getTypeId(params.coords)), indexes);
    }

    // lod
    if (cracked.lod) {
        params.lod = arguments[2 + extraArgs];
        ++extraArgs;
    } else if (glslangIntermediate->getStage() != EShLangFragment) {
        // we need to invent the default lod for an explicit lod instruction for a non-fragment stage
        noImplicitLod = true;
    }

    // multisample
    if (sampler.ms) {
        params.sample = arguments[2 + extraArgs]; // For MS, "sample" should be specified
        ++extraArgs;
    }

    // gradient
    if (cracked.grad) {
        params.gradX = arguments[2 + extraArgs];
        params.gradY = arguments[3 + extraArgs];
        extraArgs += 2;
    }

    // offset and offsets
    if (cracked.offset) {
        params.offset = arguments[2 + extraArgs];
        ++extraArgs;
    } else if (cracked.offsets) {
        params.offsets = arguments[2 + extraArgs];
        ++extraArgs;
    }

    // lod clamp
    if (cracked.lodClamp) {
        params.lodClamp = arguments[2 + extraArgs];
        ++extraArgs;
    }

    // sparse
    if (sparse) {
        params.texelOut = arguments[2 + extraArgs];
        ++extraArgs;
    }

    // gather component
    if (cracked.gather && ! sampler.shadow) {
        // default component is 0, if missing, otherwise an argument
        if (2 + extraArgs < (int)arguments.size()) {
            params.component = arguments[2 + extraArgs];
            ++extraArgs;
        } else
            params.component = builder.makeIntConstant(0);
    }

    // bias
    if (bias) {
        params.bias = arguments[2 + extraArgs];
        ++extraArgs;
    }

    // projective component (might not to move)
    // GLSL: "The texture coordinates consumed from P, not including the last component of P,
    //       are divided by the last component of P."
    // SPIR-V:  "... (u [, v] [, w], q)... It may be a vector larger than needed, but all
    //          unused components will appear after all used components."
    if (cracked.proj) {
        int projSourceComp = builder.getNumComponents(params.coords) - 1;
        int projTargetComp;
        switch (sampler.dim) {
        case glslang::Esd1D:   projTargetComp = 1;              break;
        case glslang::Esd2D:   projTargetComp = 2;              break;
        case glslang::EsdRect: projTargetComp = 2;              break;
        default:               projTargetComp = projSourceComp; break;
        }
        // copy the projective coordinate if we have to
        if (projTargetComp != projSourceComp) {
            spv::Id projComp = builder.createCompositeExtract(params.coords,
                                                              builder.getScalarTypeId(builder.getTypeId(params.coords)),
                                                              projSourceComp);
            params.coords = builder.createCompositeInsert(projComp, params.coords,
                                                          builder.getTypeId(params.coords), projTargetComp);
        }
    }

    std::vector<spv::Id> result( 1, 
        builder.createTextureCall(precision, resultType(), sparse, cracked.fetch, cracked.proj, cracked.gather, noImplicitLod, params)
    );

    if (components != node->getType().getVectorSize())
        result[0] = builder.createConstructor(precision, result, convertGlslangToSpvType(node->getType()));

    return result[0];
}

spv::Id TGlslangToSpvTraverser::handleUserFunctionCall(const glslang::TIntermAggregate* node)
{
    // Grab the function's pointer from the previously created function
    spv::Function* function = functionMap[node->getName().c_str()];
    if (! function)
        return 0;

    const glslang::TIntermSequence& glslangArgs = node->getSequence();
    const glslang::TQualifierList& qualifiers = node->getQualifierList();

    //  See comments in makeFunctions() for details about the semantics for parameter passing.
    //
    // These imply we need a four step process:
    // 1. Evaluate the arguments
    // 2. Allocate and make copies of in, out, and inout arguments
    // 3. Make the call
    // 4. Copy back the results

    // 1. Evaluate the arguments and their types
    std::vector<spv::Builder::AccessChain> lValues;
    std::vector<spv::Id> rValues;
    std::vector<const glslang::TType*> argTypes;
    for (int a = 0; a < (int)glslangArgs.size(); ++a) {
        argTypes.push_back(&glslangArgs[a]->getAsTyped()->getType());
        // build l-value
        builder.clearAccessChain();
        glslangArgs[a]->traverse(this);
        // keep outputs and pass-by-originals as l-values, evaluate others as r-values
        if (originalParam(qualifiers[a], *argTypes[a], function->hasImplicitThis() && a == 0) ||
            writableParam(qualifiers[a])) {
            // save l-value
            lValues.push_back(builder.getAccessChain());
        } else {
            // process r-value
            rValues.push_back(accessChainLoad(*argTypes.back()));
        }
    }

    // 2. Allocate space for anything needing a copy, and if it's "in" or "inout"
    // copy the original into that space.
    //
    // Also, build up the list of actual arguments to pass in for the call
    int lValueCount = 0;
    int rValueCount = 0;
    std::vector<spv::Id> spvArgs;
    for (int a = 0; a < (int)glslangArgs.size(); ++a) {
        spv::Id arg;
        if (originalParam(qualifiers[a], *argTypes[a], function->hasImplicitThis() && a == 0)) {
            builder.setAccessChain(lValues[lValueCount]);
            arg = builder.accessChainGetLValue();
            ++lValueCount;
        } else if (writableParam(qualifiers[a])) {
            // need space to hold the copy
            arg = builder.createVariable(spv::StorageClassFunction, builder.getContainedTypeId(function->getParamType(a)), "param");
            if (qualifiers[a] == glslang::EvqIn || qualifiers[a] == glslang::EvqInOut) {
                // need to copy the input into output space
                builder.setAccessChain(lValues[lValueCount]);
                spv::Id copy = accessChainLoad(*argTypes[a]);
                builder.clearAccessChain();
                builder.setAccessChainLValue(arg);
                multiTypeStore(*argTypes[a], copy);
            }
            ++lValueCount;
        } else {
            // process r-value, which involves a copy for a type mismatch
            if (function->getParamType(a) != convertGlslangToSpvType(*argTypes[a])) {
                spv::Id argCopy = builder.createVariable(spv::StorageClassFunction, function->getParamType(a), "arg");
                builder.clearAccessChain();
                builder.setAccessChainLValue(argCopy);
                multiTypeStore(*argTypes[a], rValues[rValueCount]);
                arg = builder.createLoad(argCopy);
            } else
                arg = rValues[rValueCount];
            ++rValueCount;
        }
        spvArgs.push_back(arg);
    }

    // 3. Make the call.
    spv::Id result = builder.createFunctionCall(function, spvArgs);
    builder.setPrecision(result, TranslatePrecisionDecoration(node->getType()));

    // 4. Copy back out an "out" arguments.
    lValueCount = 0;
    for (int a = 0; a < (int)glslangArgs.size(); ++a) {
        if (originalParam(qualifiers[a], *argTypes[a], function->hasImplicitThis() && a == 0))
            ++lValueCount;
        else if (writableParam(qualifiers[a])) {
            if (qualifiers[a] == glslang::EvqOut || qualifiers[a] == glslang::EvqInOut) {
                spv::Id copy = builder.createLoad(spvArgs[a]);
                builder.setAccessChain(lValues[lValueCount]);
                multiTypeStore(*argTypes[a], copy);
            }
            ++lValueCount;
        }
    }

    return result;
}

// Translate AST operation to SPV operation, already having SPV-based operands/types.
spv::Id TGlslangToSpvTraverser::createBinaryOperation(glslang::TOperator op, OpDecorations& decorations,
                                                      spv::Id typeId, spv::Id left, spv::Id right,
                                                      glslang::TBasicType typeProxy, bool reduceComparison)
{
    bool isUnsigned = isTypeUnsignedInt(typeProxy);
    bool isFloat = isTypeFloat(typeProxy);
    bool isBool = typeProxy == glslang::EbtBool;

    spv::Op binOp = spv::OpNop;
    bool needMatchingVectors = true;  // for non-matrix ops, would a scalar need to smear to match a vector?
    bool comparison = false;

    switch (op) {
    case glslang::EOpAdd:
    case glslang::EOpAddAssign:
        if (isFloat)
            binOp = spv::OpFAdd;
        else
            binOp = spv::OpIAdd;
        break;
    case glslang::EOpSub:
    case glslang::EOpSubAssign:
        if (isFloat)
            binOp = spv::OpFSub;
        else
            binOp = spv::OpISub;
        break;
    case glslang::EOpMul:
    case glslang::EOpMulAssign:
        if (isFloat)
            binOp = spv::OpFMul;
        else
            binOp = spv::OpIMul;
        break;
    case glslang::EOpVectorTimesScalar:
    case glslang::EOpVectorTimesScalarAssign:
        if (isFloat && (builder.isVector(left) || builder.isVector(right))) {
            if (builder.isVector(right))
                std::swap(left, right);
            assert(builder.isScalar(right));
            needMatchingVectors = false;
            binOp = spv::OpVectorTimesScalar;
        } else
            binOp = spv::OpIMul;
        break;
    case glslang::EOpVectorTimesMatrix:
    case glslang::EOpVectorTimesMatrixAssign:
        binOp = spv::OpVectorTimesMatrix;
        break;
    case glslang::EOpMatrixTimesVector:
        binOp = spv::OpMatrixTimesVector;
        break;
    case glslang::EOpMatrixTimesScalar:
    case glslang::EOpMatrixTimesScalarAssign:
        binOp = spv::OpMatrixTimesScalar;
        break;
    case glslang::EOpMatrixTimesMatrix:
    case glslang::EOpMatrixTimesMatrixAssign:
        binOp = spv::OpMatrixTimesMatrix;
        break;
    case glslang::EOpOuterProduct:
        binOp = spv::OpOuterProduct;
        needMatchingVectors = false;
        break;

    case glslang::EOpDiv:
    case glslang::EOpDivAssign:
        if (isFloat)
            binOp = spv::OpFDiv;
        else if (isUnsigned)
            binOp = spv::OpUDiv;
        else
            binOp = spv::OpSDiv;
        break;
    case glslang::EOpMod:
    case glslang::EOpModAssign:
        if (isFloat)
            binOp = spv::OpFMod;
        else if (isUnsigned)
            binOp = spv::OpUMod;
        else
            binOp = spv::OpSMod;
        break;
    case glslang::EOpRightShift:
    case glslang::EOpRightShiftAssign:
        if (isUnsigned)
            binOp = spv::OpShiftRightLogical;
        else
            binOp = spv::OpShiftRightArithmetic;
        break;
    case glslang::EOpLeftShift:
    case glslang::EOpLeftShiftAssign:
        binOp = spv::OpShiftLeftLogical;
        break;
    case glslang::EOpAnd:
    case glslang::EOpAndAssign:
        binOp = spv::OpBitwiseAnd;
        break;
    case glslang::EOpLogicalAnd:
        needMatchingVectors = false;
        binOp = spv::OpLogicalAnd;
        break;
    case glslang::EOpInclusiveOr:
    case glslang::EOpInclusiveOrAssign:
        binOp = spv::OpBitwiseOr;
        break;
    case glslang::EOpLogicalOr:
        needMatchingVectors = false;
        binOp = spv::OpLogicalOr;
        break;
    case glslang::EOpExclusiveOr:
    case glslang::EOpExclusiveOrAssign:
        binOp = spv::OpBitwiseXor;
        break;
    case glslang::EOpLogicalXor:
        needMatchingVectors = false;
        binOp = spv::OpLogicalNotEqual;
        break;

    case glslang::EOpLessThan:
    case glslang::EOpGreaterThan:
    case glslang::EOpLessThanEqual:
    case glslang::EOpGreaterThanEqual:
    case glslang::EOpEqual:
    case glslang::EOpNotEqual:
    case glslang::EOpVectorEqual:
    case glslang::EOpVectorNotEqual:
        comparison = true;
        break;
    default:
        break;
    }

    // handle mapped binary operations (should be non-comparison)
    if (binOp != spv::OpNop) {
        assert(comparison == false);
        if (builder.isMatrix(left) || builder.isMatrix(right))
            return createBinaryMatrixOperation(binOp, decorations, typeId, left, right);

        // No matrix involved; make both operands be the same number of components, if needed
        if (needMatchingVectors)
            builder.promoteScalar(decorations.precision, left, right);

        spv::Id result = builder.createBinOp(binOp, typeId, left, right);
        builder.addDecoration(result, decorations.noContraction);
        builder.addDecoration(result, decorations.nonUniform);
        return builder.setPrecision(result, decorations.precision);
    }

    if (! comparison)
        return 0;

    // Handle comparison instructions

    if (reduceComparison && (op == glslang::EOpEqual || op == glslang::EOpNotEqual)
                         && (builder.isVector(left) || builder.isMatrix(left) || builder.isAggregate(left))) {
        spv::Id result = builder.createCompositeCompare(decorations.precision, left, right, op == glslang::EOpEqual);
        builder.addDecoration(result, decorations.nonUniform);
        return result;
    }

    switch (op) {
    case glslang::EOpLessThan:
        if (isFloat)
            binOp = spv::OpFOrdLessThan;
        else if (isUnsigned)
            binOp = spv::OpULessThan;
        else
            binOp = spv::OpSLessThan;
        break;
    case glslang::EOpGreaterThan:
        if (isFloat)
            binOp = spv::OpFOrdGreaterThan;
        else if (isUnsigned)
            binOp = spv::OpUGreaterThan;
        else
            binOp = spv::OpSGreaterThan;
        break;
    case glslang::EOpLessThanEqual:
        if (isFloat)
            binOp = spv::OpFOrdLessThanEqual;
        else if (isUnsigned)
            binOp = spv::OpULessThanEqual;
        else
            binOp = spv::OpSLessThanEqual;
        break;
    case glslang::EOpGreaterThanEqual:
        if (isFloat)
            binOp = spv::OpFOrdGreaterThanEqual;
        else if (isUnsigned)
            binOp = spv::OpUGreaterThanEqual;
        else
            binOp = spv::OpSGreaterThanEqual;
        break;
    case glslang::EOpEqual:
    case glslang::EOpVectorEqual:
        if (isFloat)
            binOp = spv::OpFOrdEqual;
        else if (isBool)
            binOp = spv::OpLogicalEqual;
        else
            binOp = spv::OpIEqual;
        break;
    case glslang::EOpNotEqual:
    case glslang::EOpVectorNotEqual:
        if (isFloat)
            binOp = spv::OpFOrdNotEqual;
        else if (isBool)
            binOp = spv::OpLogicalNotEqual;
        else
            binOp = spv::OpINotEqual;
        break;
    default:
        break;
    }

    if (binOp != spv::OpNop) {
        spv::Id result = builder.createBinOp(binOp, typeId, left, right);
        builder.addDecoration(result, decorations.noContraction);
        builder.addDecoration(result, decorations.nonUniform);
        return builder.setPrecision(result, decorations.precision);
    }

    return 0;
}

//
// Translate AST matrix operation to SPV operation, already having SPV-based operands/types.
// These can be any of:
//
//   matrix * scalar
//   scalar * matrix
//   matrix * matrix     linear algebraic
//   matrix * vector
//   vector * matrix
//   matrix * matrix     componentwise
//   matrix op matrix    op in {+, -, /}
//   matrix op scalar    op in {+, -, /}
//   scalar op matrix    op in {+, -, /}
//
spv::Id TGlslangToSpvTraverser::createBinaryMatrixOperation(spv::Op op, OpDecorations& decorations, spv::Id typeId,
                                                            spv::Id left, spv::Id right)
{
    bool firstClass = true;

    // First, handle first-class matrix operations (* and matrix/scalar)
    switch (op) {
    case spv::OpFDiv:
        if (builder.isMatrix(left) && builder.isScalar(right)) {
            // turn matrix / scalar into a multiply...
            spv::Id resultType = builder.getTypeId(right);
            right = builder.createBinOp(spv::OpFDiv, resultType, builder.makeFpConstant(resultType, 1.0), right);
            op = spv::OpMatrixTimesScalar;
        } else
            firstClass = false;
        break;
    case spv::OpMatrixTimesScalar:
        if (builder.isMatrix(right))
            std::swap(left, right);
        assert(builder.isScalar(right));
        break;
    case spv::OpVectorTimesMatrix:
        assert(builder.isVector(left));
        assert(builder.isMatrix(right));
        break;
    case spv::OpMatrixTimesVector:
        assert(builder.isMatrix(left));
        assert(builder.isVector(right));
        break;
    case spv::OpMatrixTimesMatrix:
        assert(builder.isMatrix(left));
        assert(builder.isMatrix(right));
        break;
    default:
        firstClass = false;
        break;
    }

    if (firstClass) {
        spv::Id result = builder.createBinOp(op, typeId, left, right);
        builder.addDecoration(result, decorations.noContraction);
        builder.addDecoration(result, decorations.nonUniform);
        return builder.setPrecision(result, decorations.precision);
    }

    // Handle component-wise +, -, *, %, and / for all combinations of type.
    // The result type of all of them is the same type as the (a) matrix operand.
    // The algorithm is to:
    //   - break the matrix(es) into vectors
    //   - smear any scalar to a vector
    //   - do vector operations
    //   - make a matrix out the vector results
    switch (op) {
    case spv::OpFAdd:
    case spv::OpFSub:
    case spv::OpFDiv:
    case spv::OpFMod:
    case spv::OpFMul:
    {
        // one time set up...
        bool  leftMat = builder.isMatrix(left);
        bool rightMat = builder.isMatrix(right);
        unsigned int numCols = leftMat ? builder.getNumColumns(left) : builder.getNumColumns(right);
        int numRows = leftMat ? builder.getNumRows(left) : builder.getNumRows(right);
        spv::Id scalarType = builder.getScalarTypeId(typeId);
        spv::Id vecType = builder.makeVectorType(scalarType, numRows);
        std::vector<spv::Id> results;
        spv::Id smearVec = spv::NoResult;
        if (builder.isScalar(left))
            smearVec = builder.smearScalar(decorations.precision, left, vecType);
        else if (builder.isScalar(right))
            smearVec = builder.smearScalar(decorations.precision, right, vecType);

        // do each vector op
        for (unsigned int c = 0; c < numCols; ++c) {
            std::vector<unsigned int> indexes;
            indexes.push_back(c);
            spv::Id  leftVec =  leftMat ? builder.createCompositeExtract( left, vecType, indexes) : smearVec;
            spv::Id rightVec = rightMat ? builder.createCompositeExtract(right, vecType, indexes) : smearVec;
            spv::Id result = builder.createBinOp(op, vecType, leftVec, rightVec);
            builder.addDecoration(result, decorations.noContraction);
            builder.addDecoration(result, decorations.nonUniform);
            results.push_back(builder.setPrecision(result, decorations.precision));
        }

        // put the pieces together
        spv::Id result = builder.setPrecision(builder.createCompositeConstruct(typeId, results), decorations.precision);
        builder.addDecoration(result, decorations.nonUniform);
        return result;
    }
    default:
        assert(0);
        return spv::NoResult;
    }
}

spv::Id TGlslangToSpvTraverser::createUnaryOperation(glslang::TOperator op, OpDecorations& decorations, spv::Id typeId,
                                                     spv::Id operand, glslang::TBasicType typeProxy)
{
    spv::Op unaryOp = spv::OpNop;
    int extBuiltins = -1;
    int libCall = -1;
    bool isUnsigned = isTypeUnsignedInt(typeProxy);
    bool isFloat = isTypeFloat(typeProxy);

    switch (op) {
    case glslang::EOpNegative:
        if (isFloat) {
            unaryOp = spv::OpFNegate;
            if (builder.isMatrixType(typeId))
                return createUnaryMatrixOperation(unaryOp, decorations, typeId, operand, typeProxy);
        } else
            unaryOp = spv::OpSNegate;
        break;

    case glslang::EOpLogicalNot:
    case glslang::EOpVectorLogicalNot:
        unaryOp = spv::OpLogicalNot;
        break;
    case glslang::EOpBitwiseNot:
        unaryOp = spv::OpNot;
        break;

    case glslang::EOpDeterminant:
        libCall = spv::GLSLstd450Determinant;
        break;
    case glslang::EOpMatrixInverse:
        libCall = spv::GLSLstd450MatrixInverse;
        break;
    case glslang::EOpTranspose:
        unaryOp = spv::OpTranspose;
        break;

    case glslang::EOpRadians:
        libCall = spv::GLSLstd450Radians;
        break;
    case glslang::EOpDegrees:
        libCall = spv::GLSLstd450Degrees;
        break;
    case glslang::EOpSin:
        libCall = spv::GLSLstd450Sin;
        break;
    case glslang::EOpCos:
        libCall = spv::GLSLstd450Cos;
        break;
    case glslang::EOpTan:
        libCall = spv::GLSLstd450Tan;
        break;
    case glslang::EOpAcos:
        libCall = spv::GLSLstd450Acos;
        break;
    case glslang::EOpAsin:
        libCall = spv::GLSLstd450Asin;
        break;
    case glslang::EOpAtan:
        libCall = spv::GLSLstd450Atan;
        break;

    case glslang::EOpAcosh:
        libCall = spv::GLSLstd450Acosh;
        break;
    case glslang::EOpAsinh:
        libCall = spv::GLSLstd450Asinh;
        break;
    case glslang::EOpAtanh:
        libCall = spv::GLSLstd450Atanh;
        break;
    case glslang::EOpTanh:
        libCall = spv::GLSLstd450Tanh;
        break;
    case glslang::EOpCosh:
        libCall = spv::GLSLstd450Cosh;
        break;
    case glslang::EOpSinh:
        libCall = spv::GLSLstd450Sinh;
        break;

    case glslang::EOpLength:
        libCall = spv::GLSLstd450Length;
        break;
    case glslang::EOpNormalize:
        libCall = spv::GLSLstd450Normalize;
        break;

    case glslang::EOpExp:
        libCall = spv::GLSLstd450Exp;
        break;
    case glslang::EOpLog:
        libCall = spv::GLSLstd450Log;
        break;
    case glslang::EOpExp2:
        libCall = spv::GLSLstd450Exp2;
        break;
    case glslang::EOpLog2:
        libCall = spv::GLSLstd450Log2;
        break;
    case glslang::EOpSqrt:
        libCall = spv::GLSLstd450Sqrt;
        break;
    case glslang::EOpInverseSqrt:
        libCall = spv::GLSLstd450InverseSqrt;
        break;

    case glslang::EOpFloor:
        libCall = spv::GLSLstd450Floor;
        break;
    case glslang::EOpTrunc:
        libCall = spv::GLSLstd450Trunc;
        break;
    case glslang::EOpRound:
        libCall = spv::GLSLstd450Round;
        break;
    case glslang::EOpRoundEven:
        libCall = spv::GLSLstd450RoundEven;
        break;
    case glslang::EOpCeil:
        libCall = spv::GLSLstd450Ceil;
        break;
    case glslang::EOpFract:
        libCall = spv::GLSLstd450Fract;
        break;

    case glslang::EOpIsNan:
        unaryOp = spv::OpIsNan;
        break;
    case glslang::EOpIsInf:
        unaryOp = spv::OpIsInf;
        break;
    case glslang::EOpIsFinite:
        unaryOp = spv::OpIsFinite;
        break;

    case glslang::EOpFloatBitsToInt:
    case glslang::EOpFloatBitsToUint:
    case glslang::EOpIntBitsToFloat:
    case glslang::EOpUintBitsToFloat:
    case glslang::EOpDoubleBitsToInt64:
    case glslang::EOpDoubleBitsToUint64:
    case glslang::EOpInt64BitsToDouble:
    case glslang::EOpUint64BitsToDouble:
    case glslang::EOpFloat16BitsToInt16:
    case glslang::EOpFloat16BitsToUint16:
    case glslang::EOpInt16BitsToFloat16:
    case glslang::EOpUint16BitsToFloat16:
        unaryOp = spv::OpBitcast;
        break;

    case glslang::EOpPackSnorm2x16:
        libCall = spv::GLSLstd450PackSnorm2x16;
        break;
    case glslang::EOpUnpackSnorm2x16:
        libCall = spv::GLSLstd450UnpackSnorm2x16;
        break;
    case glslang::EOpPackUnorm2x16:
        libCall = spv::GLSLstd450PackUnorm2x16;
        break;
    case glslang::EOpUnpackUnorm2x16:
        libCall = spv::GLSLstd450UnpackUnorm2x16;
        break;
    case glslang::EOpPackHalf2x16:
        libCall = spv::GLSLstd450PackHalf2x16;
        break;
    case glslang::EOpUnpackHalf2x16:
        libCall = spv::GLSLstd450UnpackHalf2x16;
        break;
    case glslang::EOpPackSnorm4x8:
        libCall = spv::GLSLstd450PackSnorm4x8;
        break;
    case glslang::EOpUnpackSnorm4x8:
        libCall = spv::GLSLstd450UnpackSnorm4x8;
        break;
    case glslang::EOpPackUnorm4x8:
        libCall = spv::GLSLstd450PackUnorm4x8;
        break;
    case glslang::EOpUnpackUnorm4x8:
        libCall = spv::GLSLstd450UnpackUnorm4x8;
        break;
    case glslang::EOpPackDouble2x32:
        libCall = spv::GLSLstd450PackDouble2x32;
        break;
    case glslang::EOpUnpackDouble2x32:
        libCall = spv::GLSLstd450UnpackDouble2x32;
        break;

    case glslang::EOpPackInt2x32:
    case glslang::EOpUnpackInt2x32:
    case glslang::EOpPackUint2x32:
    case glslang::EOpUnpackUint2x32:
    case glslang::EOpPack16:
    case glslang::EOpPack32:
    case glslang::EOpPack64:
    case glslang::EOpUnpack32:
    case glslang::EOpUnpack16:
    case glslang::EOpUnpack8:
    case glslang::EOpPackInt2x16:
    case glslang::EOpUnpackInt2x16:
    case glslang::EOpPackUint2x16:
    case glslang::EOpUnpackUint2x16:
    case glslang::EOpPackInt4x16:
    case glslang::EOpUnpackInt4x16:
    case glslang::EOpPackUint4x16:
    case glslang::EOpUnpackUint4x16:
    case glslang::EOpPackFloat2x16:
    case glslang::EOpUnpackFloat2x16:
        unaryOp = spv::OpBitcast;
        break;

    case glslang::EOpDPdx:
        unaryOp = spv::OpDPdx;
        break;
    case glslang::EOpDPdy:
        unaryOp = spv::OpDPdy;
        break;
    case glslang::EOpFwidth:
        unaryOp = spv::OpFwidth;
        break;
    case glslang::EOpDPdxFine:
        unaryOp = spv::OpDPdxFine;
        break;
    case glslang::EOpDPdyFine:
        unaryOp = spv::OpDPdyFine;
        break;
    case glslang::EOpFwidthFine:
        unaryOp = spv::OpFwidthFine;
        break;
    case glslang::EOpDPdxCoarse:
        unaryOp = spv::OpDPdxCoarse;
        break;
    case glslang::EOpDPdyCoarse:
        unaryOp = spv::OpDPdyCoarse;
        break;
    case glslang::EOpFwidthCoarse:
        unaryOp = spv::OpFwidthCoarse;
        break;
    case glslang::EOpInterpolateAtCentroid:
#ifdef AMD_EXTENSIONS
        if (typeProxy == glslang::EbtFloat16)
            builder.addExtension(spv::E_SPV_AMD_gpu_shader_half_float);
#endif
        libCall = spv::GLSLstd450InterpolateAtCentroid;
        break;
    case glslang::EOpAny:
        unaryOp = spv::OpAny;
        break;
    case glslang::EOpAll:
        unaryOp = spv::OpAll;
        break;

    case glslang::EOpAbs:
        if (isFloat)
            libCall = spv::GLSLstd450FAbs;
        else
            libCall = spv::GLSLstd450SAbs;
        break;
    case glslang::EOpSign:
        if (isFloat)
            libCall = spv::GLSLstd450FSign;
        else
            libCall = spv::GLSLstd450SSign;
        break;

    case glslang::EOpAtomicCounterIncrement:
    case glslang::EOpAtomicCounterDecrement:
    case glslang::EOpAtomicCounter:
    {
        // Handle all of the atomics in one place, in createAtomicOperation()
        std::vector<spv::Id> operands;
        operands.push_back(operand);
        return createAtomicOperation(op, decorations.precision, typeId, operands, typeProxy);
    }

    case glslang::EOpBitFieldReverse:
        unaryOp = spv::OpBitReverse;
        break;
    case glslang::EOpBitCount:
        unaryOp = spv::OpBitCount;
        break;
    case glslang::EOpFindLSB:
        libCall = spv::GLSLstd450FindILsb;
        break;
    case glslang::EOpFindMSB:
        if (isUnsigned)
            libCall = spv::GLSLstd450FindUMsb;
        else
            libCall = spv::GLSLstd450FindSMsb;
        break;

    case glslang::EOpBallot:
    case glslang::EOpReadFirstInvocation:
    case glslang::EOpAnyInvocation:
    case glslang::EOpAllInvocations:
    case glslang::EOpAllInvocationsEqual:
#ifdef AMD_EXTENSIONS
    case glslang::EOpMinInvocations:
    case glslang::EOpMaxInvocations:
    case glslang::EOpAddInvocations:
    case glslang::EOpMinInvocationsNonUniform:
    case glslang::EOpMaxInvocationsNonUniform:
    case glslang::EOpAddInvocationsNonUniform:
    case glslang::EOpMinInvocationsInclusiveScan:
    case glslang::EOpMaxInvocationsInclusiveScan:
    case glslang::EOpAddInvocationsInclusiveScan:
    case glslang::EOpMinInvocationsInclusiveScanNonUniform:
    case glslang::EOpMaxInvocationsInclusiveScanNonUniform:
    case glslang::EOpAddInvocationsInclusiveScanNonUniform:
    case glslang::EOpMinInvocationsExclusiveScan:
    case glslang::EOpMaxInvocationsExclusiveScan:
    case glslang::EOpAddInvocationsExclusiveScan:
    case glslang::EOpMinInvocationsExclusiveScanNonUniform:
    case glslang::EOpMaxInvocationsExclusiveScanNonUniform:
    case glslang::EOpAddInvocationsExclusiveScanNonUniform:
#endif
    {
        std::vector<spv::Id> operands;
        operands.push_back(operand);
        return createInvocationsOperation(op, typeId, operands, typeProxy);
    }
    case glslang::EOpSubgroupAll:
    case glslang::EOpSubgroupAny:
    case glslang::EOpSubgroupAllEqual:
    case glslang::EOpSubgroupBroadcastFirst:
    case glslang::EOpSubgroupBallot:
    case glslang::EOpSubgroupInverseBallot:
    case glslang::EOpSubgroupBallotBitCount:
    case glslang::EOpSubgroupBallotInclusiveBitCount:
    case glslang::EOpSubgroupBallotExclusiveBitCount:
    case glslang::EOpSubgroupBallotFindLSB:
    case glslang::EOpSubgroupBallotFindMSB:
    case glslang::EOpSubgroupAdd:
    case glslang::EOpSubgroupMul:
    case glslang::EOpSubgroupMin:
    case glslang::EOpSubgroupMax:
    case glslang::EOpSubgroupAnd:
    case glslang::EOpSubgroupOr:
    case glslang::EOpSubgroupXor:
    case glslang::EOpSubgroupInclusiveAdd:
    case glslang::EOpSubgroupInclusiveMul:
    case glslang::EOpSubgroupInclusiveMin:
    case glslang::EOpSubgroupInclusiveMax:
    case glslang::EOpSubgroupInclusiveAnd:
    case glslang::EOpSubgroupInclusiveOr:
    case glslang::EOpSubgroupInclusiveXor:
    case glslang::EOpSubgroupExclusiveAdd:
    case glslang::EOpSubgroupExclusiveMul:
    case glslang::EOpSubgroupExclusiveMin:
    case glslang::EOpSubgroupExclusiveMax:
    case glslang::EOpSubgroupExclusiveAnd:
    case glslang::EOpSubgroupExclusiveOr:
    case glslang::EOpSubgroupExclusiveXor:
    case glslang::EOpSubgroupQuadSwapHorizontal:
    case glslang::EOpSubgroupQuadSwapVertical:
    case glslang::EOpSubgroupQuadSwapDiagonal: {
        std::vector<spv::Id> operands;
        operands.push_back(operand);
        return createSubgroupOperation(op, typeId, operands, typeProxy);
    }
#ifdef AMD_EXTENSIONS
    case glslang::EOpMbcnt:
        extBuiltins = getExtBuiltins(spv::E_SPV_AMD_shader_ballot);
        libCall = spv::MbcntAMD;
        break;

    case glslang::EOpCubeFaceIndex:
        extBuiltins = getExtBuiltins(spv::E_SPV_AMD_gcn_shader);
        libCall = spv::CubeFaceIndexAMD;
        break;

    case glslang::EOpCubeFaceCoord:
        extBuiltins = getExtBuiltins(spv::E_SPV_AMD_gcn_shader);
        libCall = spv::CubeFaceCoordAMD;
        break;
#endif
#ifdef NV_EXTENSIONS
    case glslang::EOpSubgroupPartition:
        unaryOp = spv::OpGroupNonUniformPartitionNV;
        break;
#endif
    default:
        return 0;
    }

    spv::Id id;
    if (libCall >= 0) {
        std::vector<spv::Id> args;
        args.push_back(operand);
        id = builder.createBuiltinCall(typeId, extBuiltins >= 0 ? extBuiltins : stdBuiltins, libCall, args);
    } else {
        id = builder.createUnaryOp(unaryOp, typeId, operand);
    }

    builder.addDecoration(id, decorations.noContraction);
    builder.addDecoration(id, decorations.nonUniform);
    return builder.setPrecision(id, decorations.precision);
}

// Create a unary operation on a matrix
spv::Id TGlslangToSpvTraverser::createUnaryMatrixOperation(spv::Op op, OpDecorations& decorations, spv::Id typeId,
                                                           spv::Id operand, glslang::TBasicType /* typeProxy */)
{
    // Handle unary operations vector by vector.
    // The result type is the same type as the original type.
    // The algorithm is to:
    //   - break the matrix into vectors
    //   - apply the operation to each vector
    //   - make a matrix out the vector results

    // get the types sorted out
    int numCols = builder.getNumColumns(operand);
    int numRows = builder.getNumRows(operand);
    spv::Id srcVecType  = builder.makeVectorType(builder.getScalarTypeId(builder.getTypeId(operand)), numRows);
    spv::Id destVecType = builder.makeVectorType(builder.getScalarTypeId(typeId), numRows);
    std::vector<spv::Id> results;

    // do each vector op
    for (int c = 0; c < numCols; ++c) {
        std::vector<unsigned int> indexes;
        indexes.push_back(c);
        spv::Id srcVec  = builder.createCompositeExtract(operand, srcVecType, indexes);
        spv::Id destVec = builder.createUnaryOp(op, destVecType, srcVec);
        builder.addDecoration(destVec, decorations.noContraction);
        builder.addDecoration(destVec, decorations.nonUniform);
        results.push_back(builder.setPrecision(destVec, decorations.precision));
    }

    // put the pieces together
    spv::Id result = builder.setPrecision(builder.createCompositeConstruct(typeId, results), decorations.precision);
    builder.addDecoration(result, decorations.nonUniform);
    return result;
}

// For converting integers where both the bitwidth and the signedness could
// change, but only do the width change here. The caller is still responsible
// for the signedness conversion.
spv::Id TGlslangToSpvTraverser::createIntWidthConversion(glslang::TOperator op, spv::Id operand, int vectorSize)
{
    // Get the result type width, based on the type to convert to.
    int width = 32;
    switch(op) {
    case glslang::EOpConvInt16ToUint8:
    case glslang::EOpConvIntToUint8:
    case glslang::EOpConvInt64ToUint8:
    case glslang::EOpConvUint16ToInt8:
    case glslang::EOpConvUintToInt8:
    case glslang::EOpConvUint64ToInt8:
        width = 8;
        break;
    case glslang::EOpConvInt8ToUint16:
    case glslang::EOpConvIntToUint16:
    case glslang::EOpConvInt64ToUint16:
    case glslang::EOpConvUint8ToInt16:
    case glslang::EOpConvUintToInt16:
    case glslang::EOpConvUint64ToInt16:
        width = 16;
        break;
    case glslang::EOpConvInt8ToUint:
    case glslang::EOpConvInt16ToUint:
    case glslang::EOpConvInt64ToUint:
    case glslang::EOpConvUint8ToInt:
    case glslang::EOpConvUint16ToInt:
    case glslang::EOpConvUint64ToInt:
        width = 32;
        break;
    case glslang::EOpConvInt8ToUint64:
    case glslang::EOpConvInt16ToUint64:
    case glslang::EOpConvIntToUint64:
    case glslang::EOpConvUint8ToInt64:
    case glslang::EOpConvUint16ToInt64:
    case glslang::EOpConvUintToInt64:
        width = 64;
        break;

    default:
        assert(false && "Default missing");
        break;
    }

    // Get the conversion operation and result type,
    // based on the target width, but the source type.
    spv::Id type = spv::NoType;
    spv::Op convOp = spv::OpNop;
    switch(op) {
    case glslang::EOpConvInt8ToUint16:
    case glslang::EOpConvInt8ToUint:
    case glslang::EOpConvInt8ToUint64:
    case glslang::EOpConvInt16ToUint8:
    case glslang::EOpConvInt16ToUint:
    case glslang::EOpConvInt16ToUint64:
    case glslang::EOpConvIntToUint8:
    case glslang::EOpConvIntToUint16:
    case glslang::EOpConvIntToUint64:
    case glslang::EOpConvInt64ToUint8:
    case glslang::EOpConvInt64ToUint16:
    case glslang::EOpConvInt64ToUint:
        convOp = spv::OpSConvert;
        type = builder.makeIntType(width);
        break;
    default:
        convOp = spv::OpUConvert;
        type = builder.makeUintType(width);
        break;
    }

    if (vectorSize > 0)
        type = builder.makeVectorType(type, vectorSize);

    return builder.createUnaryOp(convOp, type, operand);
}

spv::Id TGlslangToSpvTraverser::createConversion(glslang::TOperator op, OpDecorations& decorations, spv::Id destType,
                                                 spv::Id operand, glslang::TBasicType typeProxy)
{
    spv::Op convOp = spv::OpNop;
    spv::Id zero = 0;
    spv::Id one = 0;

    int vectorSize = builder.isVectorType(destType) ? builder.getNumTypeComponents(destType) : 0;

    switch (op) {
    case glslang::EOpConvInt8ToBool:
    case glslang::EOpConvUint8ToBool:
        zero = builder.makeUint8Constant(0);
        zero = makeSmearedConstant(zero, vectorSize);
        return builder.createBinOp(spv::OpINotEqual, destType, operand, zero);
    case glslang::EOpConvInt16ToBool:
    case glslang::EOpConvUint16ToBool:
        zero = builder.makeUint16Constant(0);
        zero = makeSmearedConstant(zero, vectorSize);
        return builder.createBinOp(spv::OpINotEqual, destType, operand, zero);
    case glslang::EOpConvIntToBool:
    case glslang::EOpConvUintToBool:
        zero = builder.makeUintConstant(0);
        zero = makeSmearedConstant(zero, vectorSize);
        return builder.createBinOp(spv::OpINotEqual, destType, operand, zero);
    case glslang::EOpConvInt64ToBool:
    case glslang::EOpConvUint64ToBool:
        zero = builder.makeUint64Constant(0);
        zero = makeSmearedConstant(zero, vectorSize);
        return builder.createBinOp(spv::OpINotEqual, destType, operand, zero);

    case glslang::EOpConvFloatToBool:
        zero = builder.makeFloatConstant(0.0F);
        zero = makeSmearedConstant(zero, vectorSize);
        return builder.createBinOp(spv::OpFOrdNotEqual, destType, operand, zero);

    case glslang::EOpConvDoubleToBool:
        zero = builder.makeDoubleConstant(0.0);
        zero = makeSmearedConstant(zero, vectorSize);
        return builder.createBinOp(spv::OpFOrdNotEqual, destType, operand, zero);

    case glslang::EOpConvFloat16ToBool:
        zero = builder.makeFloat16Constant(0.0F);
        zero = makeSmearedConstant(zero, vectorSize);
        return builder.createBinOp(spv::OpFOrdNotEqual, destType, operand, zero);

    case glslang::EOpConvBoolToFloat:
        convOp = spv::OpSelect;
        zero = builder.makeFloatConstant(0.0F);
        one  = builder.makeFloatConstant(1.0F);
        break;

    case glslang::EOpConvBoolToDouble:
        convOp = spv::OpSelect;
        zero = builder.makeDoubleConstant(0.0);
        one  = builder.makeDoubleConstant(1.0);
        break;

    case glslang::EOpConvBoolToFloat16:
        convOp = spv::OpSelect;
        zero = builder.makeFloat16Constant(0.0F);
        one = builder.makeFloat16Constant(1.0F);
        break;

    case glslang::EOpConvBoolToInt8:
        zero = builder.makeInt8Constant(0);
        one  = builder.makeInt8Constant(1);
        convOp = spv::OpSelect;
        break;

    case glslang::EOpConvBoolToUint8:
        zero = builder.makeUint8Constant(0);
        one  = builder.makeUint8Constant(1);
        convOp = spv::OpSelect;
        break;

    case glslang::EOpConvBoolToInt16:
        zero = builder.makeInt16Constant(0);
        one  = builder.makeInt16Constant(1);
        convOp = spv::OpSelect;
        break;

    case glslang::EOpConvBoolToUint16:
        zero = builder.makeUint16Constant(0);
        one  = builder.makeUint16Constant(1);
        convOp = spv::OpSelect;
        break;

    case glslang::EOpConvBoolToInt:
    case glslang::EOpConvBoolToInt64:
        if (op == glslang::EOpConvBoolToInt64)
            zero = builder.makeInt64Constant(0);
        else
            zero = builder.makeIntConstant(0);

        if (op == glslang::EOpConvBoolToInt64)
            one = builder.makeInt64Constant(1);
        else
            one = builder.makeIntConstant(1);

        convOp = spv::OpSelect;
        break;

    case glslang::EOpConvBoolToUint:
    case glslang::EOpConvBoolToUint64:
        if (op == glslang::EOpConvBoolToUint64)
            zero = builder.makeUint64Constant(0);
        else
            zero = builder.makeUintConstant(0);

        if (op == glslang::EOpConvBoolToUint64)
            one = builder.makeUint64Constant(1);
        else
            one = builder.makeUintConstant(1);

        convOp = spv::OpSelect;
        break;

    case glslang::EOpConvInt8ToFloat16:
    case glslang::EOpConvInt8ToFloat:
    case glslang::EOpConvInt8ToDouble:
    case glslang::EOpConvInt16ToFloat16:
    case glslang::EOpConvInt16ToFloat:
    case glslang::EOpConvInt16ToDouble:
    case glslang::EOpConvIntToFloat16:
    case glslang::EOpConvIntToFloat:
    case glslang::EOpConvIntToDouble:
    case glslang::EOpConvInt64ToFloat:
    case glslang::EOpConvInt64ToDouble:
    case glslang::EOpConvInt64ToFloat16:
        convOp = spv::OpConvertSToF;
        break;

    case glslang::EOpConvUint8ToFloat16:
    case glslang::EOpConvUint8ToFloat:
    case glslang::EOpConvUint8ToDouble:
    case glslang::EOpConvUint16ToFloat16:
    case glslang::EOpConvUint16ToFloat:
    case glslang::EOpConvUint16ToDouble:
    case glslang::EOpConvUintToFloat16:
    case glslang::EOpConvUintToFloat:
    case glslang::EOpConvUintToDouble:
    case glslang::EOpConvUint64ToFloat:
    case glslang::EOpConvUint64ToDouble:
    case glslang::EOpConvUint64ToFloat16:
        convOp = spv::OpConvertUToF;
        break;

    case glslang::EOpConvDoubleToFloat:
    case glslang::EOpConvFloatToDouble:
    case glslang::EOpConvDoubleToFloat16:
    case glslang::EOpConvFloat16ToDouble:
    case glslang::EOpConvFloatToFloat16:
    case glslang::EOpConvFloat16ToFloat:
        convOp = spv::OpFConvert;
        if (builder.isMatrixType(destType))
            return createUnaryMatrixOperation(convOp, decorations, destType, operand, typeProxy);
        break;

    case glslang::EOpConvFloat16ToInt8:
    case glslang::EOpConvFloatToInt8:
    case glslang::EOpConvDoubleToInt8:
    case glslang::EOpConvFloat16ToInt16:
    case glslang::EOpConvFloatToInt16:
    case glslang::EOpConvDoubleToInt16:
    case glslang::EOpConvFloat16ToInt:
    case glslang::EOpConvFloatToInt:
    case glslang::EOpConvDoubleToInt:
    case glslang::EOpConvFloat16ToInt64:
    case glslang::EOpConvFloatToInt64:
    case glslang::EOpConvDoubleToInt64:
        convOp = spv::OpConvertFToS;
        break;

    case glslang::EOpConvUint8ToInt8:
    case glslang::EOpConvInt8ToUint8:
    case glslang::EOpConvUint16ToInt16:
    case glslang::EOpConvInt16ToUint16:
    case glslang::EOpConvUintToInt:
    case glslang::EOpConvIntToUint:
    case glslang::EOpConvUint64ToInt64:
    case glslang::EOpConvInt64ToUint64:
        if (builder.isInSpecConstCodeGenMode()) {
            // Build zero scalar or vector for OpIAdd.
            if(op == glslang::EOpConvUint8ToInt8 || op == glslang::EOpConvInt8ToUint8) {
                zero = builder.makeUint8Constant(0);
            } else if (op == glslang::EOpConvUint16ToInt16 || op == glslang::EOpConvInt16ToUint16) {
                zero = builder.makeUint16Constant(0);
            } else if (op == glslang::EOpConvUint64ToInt64 || op == glslang::EOpConvInt64ToUint64) {
                zero = builder.makeUint64Constant(0);
            } else {
                zero = builder.makeUintConstant(0);
            }
            zero = makeSmearedConstant(zero, vectorSize);
            // Use OpIAdd, instead of OpBitcast to do the conversion when
            // generating for OpSpecConstantOp instruction.
            return builder.createBinOp(spv::OpIAdd, destType, operand, zero);
        }
        // For normal run-time conversion instruction, use OpBitcast.
        convOp = spv::OpBitcast;
        break;

    case glslang::EOpConvFloat16ToUint8:
    case glslang::EOpConvFloatToUint8:
    case glslang::EOpConvDoubleToUint8:
    case glslang::EOpConvFloat16ToUint16:
    case glslang::EOpConvFloatToUint16:
    case glslang::EOpConvDoubleToUint16:
    case glslang::EOpConvFloat16ToUint:
    case glslang::EOpConvFloatToUint:
    case glslang::EOpConvDoubleToUint:
    case glslang::EOpConvFloatToUint64:
    case glslang::EOpConvDoubleToUint64:
    case glslang::EOpConvFloat16ToUint64:
        convOp = spv::OpConvertFToU;
        break;

    case glslang::EOpConvInt8ToInt16:
    case glslang::EOpConvInt8ToInt:
    case glslang::EOpConvInt8ToInt64:
    case glslang::EOpConvInt16ToInt8:
    case glslang::EOpConvInt16ToInt:
    case glslang::EOpConvInt16ToInt64:
    case glslang::EOpConvIntToInt8:
    case glslang::EOpConvIntToInt16:
    case glslang::EOpConvIntToInt64:
    case glslang::EOpConvInt64ToInt8:
    case glslang::EOpConvInt64ToInt16:
    case glslang::EOpConvInt64ToInt:
        convOp = spv::OpSConvert;
        break;

    case glslang::EOpConvUint8ToUint16:
    case glslang::EOpConvUint8ToUint:
    case glslang::EOpConvUint8ToUint64:
    case glslang::EOpConvUint16ToUint8:
    case glslang::EOpConvUint16ToUint:
    case glslang::EOpConvUint16ToUint64:
    case glslang::EOpConvUintToUint8:
    case glslang::EOpConvUintToUint16:
    case glslang::EOpConvUintToUint64:
    case glslang::EOpConvUint64ToUint8:
    case glslang::EOpConvUint64ToUint16:
    case glslang::EOpConvUint64ToUint:
        convOp = spv::OpUConvert;
        break;

    case glslang::EOpConvInt8ToUint16:
    case glslang::EOpConvInt8ToUint:
    case glslang::EOpConvInt8ToUint64:
    case glslang::EOpConvInt16ToUint8:
    case glslang::EOpConvInt16ToUint:
    case glslang::EOpConvInt16ToUint64:
    case glslang::EOpConvIntToUint8:
    case glslang::EOpConvIntToUint16:
    case glslang::EOpConvIntToUint64:
    case glslang::EOpConvInt64ToUint8:
    case glslang::EOpConvInt64ToUint16:
    case glslang::EOpConvInt64ToUint:
    case glslang::EOpConvUint8ToInt16:
    case glslang::EOpConvUint8ToInt:
    case glslang::EOpConvUint8ToInt64:
    case glslang::EOpConvUint16ToInt8:
    case glslang::EOpConvUint16ToInt:
    case glslang::EOpConvUint16ToInt64:
    case glslang::EOpConvUintToInt8:
    case glslang::EOpConvUintToInt16:
    case glslang::EOpConvUintToInt64:
    case glslang::EOpConvUint64ToInt8:
    case glslang::EOpConvUint64ToInt16:
    case glslang::EOpConvUint64ToInt:
        // OpSConvert/OpUConvert + OpBitCast
        operand = createIntWidthConversion(op, operand, vectorSize);

        if (builder.isInSpecConstCodeGenMode()) {
            // Build zero scalar or vector for OpIAdd.
            switch(op) {
            case glslang::EOpConvInt16ToUint8:
            case glslang::EOpConvIntToUint8:
            case glslang::EOpConvInt64ToUint8:
            case glslang::EOpConvUint16ToInt8:
            case glslang::EOpConvUintToInt8:
            case glslang::EOpConvUint64ToInt8:
                zero = builder.makeUint8Constant(0);
                break;
            case glslang::EOpConvInt8ToUint16:
            case glslang::EOpConvIntToUint16:
            case glslang::EOpConvInt64ToUint16:
            case glslang::EOpConvUint8ToInt16:
            case glslang::EOpConvUintToInt16:
            case glslang::EOpConvUint64ToInt16:
                zero = builder.makeUint16Constant(0);
                break;
            case glslang::EOpConvInt8ToUint:
            case glslang::EOpConvInt16ToUint:
            case glslang::EOpConvInt64ToUint:
            case glslang::EOpConvUint8ToInt:
            case glslang::EOpConvUint16ToInt:
            case glslang::EOpConvUint64ToInt:
                zero = builder.makeUintConstant(0);
                break;
            case glslang::EOpConvInt8ToUint64:
            case glslang::EOpConvInt16ToUint64:
            case glslang::EOpConvIntToUint64:
            case glslang::EOpConvUint8ToInt64:
            case glslang::EOpConvUint16ToInt64:
            case glslang::EOpConvUintToInt64:
                zero = builder.makeUint64Constant(0);
                break;
            default:
                assert(false && "Default missing");
                break;
            }
            zero = makeSmearedConstant(zero, vectorSize);
            // Use OpIAdd, instead of OpBitcast to do the conversion when
            // generating for OpSpecConstantOp instruction.
            return builder.createBinOp(spv::OpIAdd, destType, operand, zero);
        }
        // For normal run-time conversion instruction, use OpBitcast.
        convOp = spv::OpBitcast;
        break;
    default:
        break;
    }

    spv::Id result = 0;
    if (convOp == spv::OpNop)
        return result;

    if (convOp == spv::OpSelect) {
        zero = makeSmearedConstant(zero, vectorSize);
        one  = makeSmearedConstant(one, vectorSize);
        result = builder.createTriOp(convOp, destType, operand, one, zero);
    } else
        result = builder.createUnaryOp(convOp, destType, operand);

    result = builder.setPrecision(result, decorations.precision);
    builder.addDecoration(result, decorations.nonUniform);
    return result;
}

spv::Id TGlslangToSpvTraverser::makeSmearedConstant(spv::Id constant, int vectorSize)
{
    if (vectorSize == 0)
        return constant;

    spv::Id vectorTypeId = builder.makeVectorType(builder.getTypeId(constant), vectorSize);
    std::vector<spv::Id> components;
    for (int c = 0; c < vectorSize; ++c)
        components.push_back(constant);
    return builder.makeCompositeConstant(vectorTypeId, components);
}

// For glslang ops that map to SPV atomic opCodes
spv::Id TGlslangToSpvTraverser::createAtomicOperation(glslang::TOperator op, spv::Decoration /*precision*/, spv::Id typeId, std::vector<spv::Id>& operands, glslang::TBasicType typeProxy)
{
    spv::Op opCode = spv::OpNop;

    switch (op) {
    case glslang::EOpAtomicAdd:
    case glslang::EOpImageAtomicAdd:
    case glslang::EOpAtomicCounterAdd:
        opCode = spv::OpAtomicIAdd;
        break;
    case glslang::EOpAtomicCounterSubtract:
        opCode = spv::OpAtomicISub;
        break;
    case glslang::EOpAtomicMin:
    case glslang::EOpImageAtomicMin:
    case glslang::EOpAtomicCounterMin:
        opCode = (typeProxy == glslang::EbtUint || typeProxy == glslang::EbtUint64) ? spv::OpAtomicUMin : spv::OpAtomicSMin;
        break;
    case glslang::EOpAtomicMax:
    case glslang::EOpImageAtomicMax:
    case glslang::EOpAtomicCounterMax:
        opCode = (typeProxy == glslang::EbtUint || typeProxy == glslang::EbtUint64) ? spv::OpAtomicUMax : spv::OpAtomicSMax;
        break;
    case glslang::EOpAtomicAnd:
    case glslang::EOpImageAtomicAnd:
    case glslang::EOpAtomicCounterAnd:
        opCode = spv::OpAtomicAnd;
        break;
    case glslang::EOpAtomicOr:
    case glslang::EOpImageAtomicOr:
    case glslang::EOpAtomicCounterOr:
        opCode = spv::OpAtomicOr;
        break;
    case glslang::EOpAtomicXor:
    case glslang::EOpImageAtomicXor:
    case glslang::EOpAtomicCounterXor:
        opCode = spv::OpAtomicXor;
        break;
    case glslang::EOpAtomicExchange:
    case glslang::EOpImageAtomicExchange:
    case glslang::EOpAtomicCounterExchange:
        opCode = spv::OpAtomicExchange;
        break;
    case glslang::EOpAtomicCompSwap:
    case glslang::EOpImageAtomicCompSwap:
    case glslang::EOpAtomicCounterCompSwap:
        opCode = spv::OpAtomicCompareExchange;
        break;
    case glslang::EOpAtomicCounterIncrement:
        opCode = spv::OpAtomicIIncrement;
        break;
    case glslang::EOpAtomicCounterDecrement:
        opCode = spv::OpAtomicIDecrement;
        break;
    case glslang::EOpAtomicCounter:
        opCode = spv::OpAtomicLoad;
        break;
    default:
        assert(0);
        break;
    }

    if (typeProxy == glslang::EbtInt64 || typeProxy == glslang::EbtUint64)
        builder.addCapability(spv::CapabilityInt64Atomics);

    // Sort out the operands
    //  - mapping from glslang -> SPV
    //  - there are extra SPV operands with no glslang source
    //  - compare-exchange swaps the value and comparator
    //  - compare-exchange has an extra memory semantics
    //  - EOpAtomicCounterDecrement needs a post decrement
    std::vector<spv::Id> spvAtomicOperands;  // hold the spv operands
    auto opIt = operands.begin();            // walk the glslang operands
    spvAtomicOperands.push_back(*(opIt++));
    spvAtomicOperands.push_back(builder.makeUintConstant(spv::ScopeDevice));     // TBD: what is the correct scope?
    spvAtomicOperands.push_back(builder.makeUintConstant(spv::MemorySemanticsMaskNone)); // TBD: what are the correct memory semantics?
    if (opCode == spv::OpAtomicCompareExchange) {
        // There are 2 memory semantics for compare-exchange. And the operand order of "comparator" and "new value" in GLSL
        // differs from that in SPIR-V. Hence, special processing is required.
        spvAtomicOperands.push_back(builder.makeUintConstant(spv::MemorySemanticsMaskNone));
        spvAtomicOperands.push_back(*(opIt + 1));
        spvAtomicOperands.push_back(*opIt);
        opIt += 2;
    }

    // Add the rest of the operands, skipping any that were dealt with above.
    for (; opIt != operands.end(); ++opIt)
        spvAtomicOperands.push_back(*opIt);

    spv::Id resultId = builder.createOp(opCode, typeId, spvAtomicOperands);

    // GLSL and HLSL atomic-counter decrement return post-decrement value,
    // while SPIR-V returns pre-decrement value. Translate between these semantics.
    if (op == glslang::EOpAtomicCounterDecrement)
        resultId = builder.createBinOp(spv::OpISub, typeId, resultId, builder.makeIntConstant(1));

    return resultId;
}

// Create group invocation operations.
spv::Id TGlslangToSpvTraverser::createInvocationsOperation(glslang::TOperator op, spv::Id typeId, std::vector<spv::Id>& operands, glslang::TBasicType typeProxy)
{
#ifdef AMD_EXTENSIONS
    bool isUnsigned = isTypeUnsignedInt(typeProxy);
    bool isFloat = isTypeFloat(typeProxy);
#endif

    spv::Op opCode = spv::OpNop;
    std::vector<spv::IdImmediate> spvGroupOperands;
    spv::GroupOperation groupOperation = spv::GroupOperationMax;

    if (op == glslang::EOpBallot || op == glslang::EOpReadFirstInvocation ||
        op == glslang::EOpReadInvocation) {
        builder.addExtension(spv::E_SPV_KHR_shader_ballot);
        builder.addCapability(spv::CapabilitySubgroupBallotKHR);
    } else if (op == glslang::EOpAnyInvocation ||
        op == glslang::EOpAllInvocations ||
        op == glslang::EOpAllInvocationsEqual) {
        builder.addExtension(spv::E_SPV_KHR_subgroup_vote);
        builder.addCapability(spv::CapabilitySubgroupVoteKHR);
    } else {
        builder.addCapability(spv::CapabilityGroups);
#ifdef AMD_EXTENSIONS
        if (op == glslang::EOpMinInvocationsNonUniform ||
            op == glslang::EOpMaxInvocationsNonUniform ||
            op == glslang::EOpAddInvocationsNonUniform ||
            op == glslang::EOpMinInvocationsInclusiveScanNonUniform ||
            op == glslang::EOpMaxInvocationsInclusiveScanNonUniform ||
            op == glslang::EOpAddInvocationsInclusiveScanNonUniform ||
            op == glslang::EOpMinInvocationsExclusiveScanNonUniform ||
            op == glslang::EOpMaxInvocationsExclusiveScanNonUniform ||
            op == glslang::EOpAddInvocationsExclusiveScanNonUniform)
            builder.addExtension(spv::E_SPV_AMD_shader_ballot);
#endif

#ifdef AMD_EXTENSIONS
        switch (op) {
        case glslang::EOpMinInvocations:
        case glslang::EOpMaxInvocations:
        case glslang::EOpAddInvocations:
        case glslang::EOpMinInvocationsNonUniform:
        case glslang::EOpMaxInvocationsNonUniform:
        case glslang::EOpAddInvocationsNonUniform:
            groupOperation = spv::GroupOperationReduce;
            break;
        case glslang::EOpMinInvocationsInclusiveScan:
        case glslang::EOpMaxInvocationsInclusiveScan:
        case glslang::EOpAddInvocationsInclusiveScan:
        case glslang::EOpMinInvocationsInclusiveScanNonUniform:
        case glslang::EOpMaxInvocationsInclusiveScanNonUniform:
        case glslang::EOpAddInvocationsInclusiveScanNonUniform:
            groupOperation = spv::GroupOperationInclusiveScan;
            break;
        case glslang::EOpMinInvocationsExclusiveScan:
        case glslang::EOpMaxInvocationsExclusiveScan:
        case glslang::EOpAddInvocationsExclusiveScan:
        case glslang::EOpMinInvocationsExclusiveScanNonUniform:
        case glslang::EOpMaxInvocationsExclusiveScanNonUniform:
        case glslang::EOpAddInvocationsExclusiveScanNonUniform:
            groupOperation = spv::GroupOperationExclusiveScan;
            break;
        default:
            break;
        }
        spv::IdImmediate scope = { true, builder.makeUintConstant(spv::ScopeSubgroup) };
        spvGroupOperands.push_back(scope);
        if (groupOperation != spv::GroupOperationMax) {
            spv::IdImmediate groupOp = { false, groupOperation };
            spvGroupOperands.push_back(groupOp);
        }
#endif
    }

    for (auto opIt = operands.begin(); opIt != operands.end(); ++opIt) {
        spv::IdImmediate op = { true, *opIt };
        spvGroupOperands.push_back(op);
    }

    switch (op) {
    case glslang::EOpAnyInvocation:
        opCode = spv::OpSubgroupAnyKHR;
        break;
    case glslang::EOpAllInvocations:
        opCode = spv::OpSubgroupAllKHR;
        break;
    case glslang::EOpAllInvocationsEqual:
        opCode = spv::OpSubgroupAllEqualKHR;
        break;
    case glslang::EOpReadInvocation:
        opCode = spv::OpSubgroupReadInvocationKHR;
        if (builder.isVectorType(typeId))
            return CreateInvocationsVectorOperation(opCode, groupOperation, typeId, operands);
        break;
    case glslang::EOpReadFirstInvocation:
        opCode = spv::OpSubgroupFirstInvocationKHR;
        break;
    case glslang::EOpBallot:
    {
        // NOTE: According to the spec, the result type of "OpSubgroupBallotKHR" must be a 4 component vector of 32
        // bit integer types. The GLSL built-in function "ballotARB()" assumes the maximum number of invocations in
        // a subgroup is 64. Thus, we have to convert uvec4.xy to uint64_t as follow:
        //
        //     result = Bitcast(SubgroupBallotKHR(Predicate).xy)
        //
        spv::Id uintType  = builder.makeUintType(32);
        spv::Id uvec4Type = builder.makeVectorType(uintType, 4);
        spv::Id result = builder.createOp(spv::OpSubgroupBallotKHR, uvec4Type, spvGroupOperands);

        std::vector<spv::Id> components;
        components.push_back(builder.createCompositeExtract(result, uintType, 0));
        components.push_back(builder.createCompositeExtract(result, uintType, 1));

        spv::Id uvec2Type = builder.makeVectorType(uintType, 2);
        return builder.createUnaryOp(spv::OpBitcast, typeId,
                                     builder.createCompositeConstruct(uvec2Type, components));
    }

#ifdef AMD_EXTENSIONS
    case glslang::EOpMinInvocations:
    case glslang::EOpMaxInvocations:
    case glslang::EOpAddInvocations:
    case glslang::EOpMinInvocationsInclusiveScan:
    case glslang::EOpMaxInvocationsInclusiveScan:
    case glslang::EOpAddInvocationsInclusiveScan:
    case glslang::EOpMinInvocationsExclusiveScan:
    case glslang::EOpMaxInvocationsExclusiveScan:
    case glslang::EOpAddInvocationsExclusiveScan:
        if (op == glslang::EOpMinInvocations ||
            op == glslang::EOpMinInvocationsInclusiveScan ||
            op == glslang::EOpMinInvocationsExclusiveScan) {
            if (isFloat)
                opCode = spv::OpGroupFMin;
            else {
                if (isUnsigned)
                    opCode = spv::OpGroupUMin;
                else
                    opCode = spv::OpGroupSMin;
            }
        } else if (op == glslang::EOpMaxInvocations ||
                   op == glslang::EOpMaxInvocationsInclusiveScan ||
                   op == glslang::EOpMaxInvocationsExclusiveScan) {
            if (isFloat)
                opCode = spv::OpGroupFMax;
            else {
                if (isUnsigned)
                    opCode = spv::OpGroupUMax;
                else
                    opCode = spv::OpGroupSMax;
            }
        } else {
            if (isFloat)
                opCode = spv::OpGroupFAdd;
            else
                opCode = spv::OpGroupIAdd;
        }

        if (builder.isVectorType(typeId))
            return CreateInvocationsVectorOperation(opCode, groupOperation, typeId, operands);

        break;
    case glslang::EOpMinInvocationsNonUniform:
    case glslang::EOpMaxInvocationsNonUniform:
    case glslang::EOpAddInvocationsNonUniform:
    case glslang::EOpMinInvocationsInclusiveScanNonUniform:
    case glslang::EOpMaxInvocationsInclusiveScanNonUniform:
    case glslang::EOpAddInvocationsInclusiveScanNonUniform:
    case glslang::EOpMinInvocationsExclusiveScanNonUniform:
    case glslang::EOpMaxInvocationsExclusiveScanNonUniform:
    case glslang::EOpAddInvocationsExclusiveScanNonUniform:
        if (op == glslang::EOpMinInvocationsNonUniform ||
            op == glslang::EOpMinInvocationsInclusiveScanNonUniform ||
            op == glslang::EOpMinInvocationsExclusiveScanNonUniform) {
            if (isFloat)
                opCode = spv::OpGroupFMinNonUniformAMD;
            else {
                if (isUnsigned)
                    opCode = spv::OpGroupUMinNonUniformAMD;
                else
                    opCode = spv::OpGroupSMinNonUniformAMD;
            }
        }
        else if (op == glslang::EOpMaxInvocationsNonUniform ||
                 op == glslang::EOpMaxInvocationsInclusiveScanNonUniform ||
                 op == glslang::EOpMaxInvocationsExclusiveScanNonUniform) {
            if (isFloat)
                opCode = spv::OpGroupFMaxNonUniformAMD;
            else {
                if (isUnsigned)
                    opCode = spv::OpGroupUMaxNonUniformAMD;
                else
                    opCode = spv::OpGroupSMaxNonUniformAMD;
            }
        }
        else {
            if (isFloat)
                opCode = spv::OpGroupFAddNonUniformAMD;
            else
                opCode = spv::OpGroupIAddNonUniformAMD;
        }

        if (builder.isVectorType(typeId))
            return CreateInvocationsVectorOperation(opCode, groupOperation, typeId, operands);

        break;
#endif
    default:
        logger->missingFunctionality("invocation operation");
        return spv::NoResult;
    }

    assert(opCode != spv::OpNop);
    return builder.createOp(opCode, typeId, spvGroupOperands);
}

// Create group invocation operations on a vector
spv::Id TGlslangToSpvTraverser::CreateInvocationsVectorOperation(spv::Op op, spv::GroupOperation groupOperation,
    spv::Id typeId, std::vector<spv::Id>& operands)
{
#ifdef AMD_EXTENSIONS
    assert(op == spv::OpGroupFMin || op == spv::OpGroupUMin || op == spv::OpGroupSMin ||
           op == spv::OpGroupFMax || op == spv::OpGroupUMax || op == spv::OpGroupSMax ||
           op == spv::OpGroupFAdd || op == spv::OpGroupIAdd || op == spv::OpGroupBroadcast ||
           op == spv::OpSubgroupReadInvocationKHR ||
           op == spv::OpGroupFMinNonUniformAMD || op == spv::OpGroupUMinNonUniformAMD || op == spv::OpGroupSMinNonUniformAMD ||
           op == spv::OpGroupFMaxNonUniformAMD || op == spv::OpGroupUMaxNonUniformAMD || op == spv::OpGroupSMaxNonUniformAMD ||
           op == spv::OpGroupFAddNonUniformAMD || op == spv::OpGroupIAddNonUniformAMD);
#else
    assert(op == spv::OpGroupFMin || op == spv::OpGroupUMin || op == spv::OpGroupSMin ||
           op == spv::OpGroupFMax || op == spv::OpGroupUMax || op == spv::OpGroupSMax ||
           op == spv::OpGroupFAdd || op == spv::OpGroupIAdd || op == spv::OpGroupBroadcast ||
           op == spv::OpSubgroupReadInvocationKHR);
#endif

    // Handle group invocation operations scalar by scalar.
    // The result type is the same type as the original type.
    // The algorithm is to:
    //   - break the vector into scalars
    //   - apply the operation to each scalar
    //   - make a vector out the scalar results

    // get the types sorted out
    int numComponents = builder.getNumComponents(operands[0]);
    spv::Id scalarType = builder.getScalarTypeId(builder.getTypeId(operands[0]));
    std::vector<spv::Id> results;

    // do each scalar op
    for (int comp = 0; comp < numComponents; ++comp) {
        std::vector<unsigned int> indexes;
        indexes.push_back(comp);
        spv::IdImmediate scalar = { true, builder.createCompositeExtract(operands[0], scalarType, indexes) };
        std::vector<spv::IdImmediate> spvGroupOperands;
        if (op == spv::OpSubgroupReadInvocationKHR) {
            spvGroupOperands.push_back(scalar);
            spv::IdImmediate operand = { true, operands[1] };
            spvGroupOperands.push_back(operand);
        } else if (op == spv::OpGroupBroadcast) {
            spv::IdImmediate scope = { true, builder.makeUintConstant(spv::ScopeSubgroup) };
            spvGroupOperands.push_back(scope);
            spvGroupOperands.push_back(scalar);
            spv::IdImmediate operand = { true, operands[1] };
            spvGroupOperands.push_back(operand);
        } else {
            spv::IdImmediate scope = { true, builder.makeUintConstant(spv::ScopeSubgroup) };
            spvGroupOperands.push_back(scope);
            spv::IdImmediate groupOp = { false, groupOperation };
            spvGroupOperands.push_back(groupOp);
            spvGroupOperands.push_back(scalar);
        }

        results.push_back(builder.createOp(op, scalarType, spvGroupOperands));
    }

    // put the pieces together
    return builder.createCompositeConstruct(typeId, results);
}

// Create subgroup invocation operations.
spv::Id TGlslangToSpvTraverser::createSubgroupOperation(glslang::TOperator op, spv::Id typeId,
    std::vector<spv::Id>& operands, glslang::TBasicType typeProxy)
{
    // Add the required capabilities.
    switch (op) {
    case glslang::EOpSubgroupElect:
        builder.addCapability(spv::CapabilityGroupNonUniform);
        break;
    case glslang::EOpSubgroupAll:
    case glslang::EOpSubgroupAny:
    case glslang::EOpSubgroupAllEqual:
        builder.addCapability(spv::CapabilityGroupNonUniform);
        builder.addCapability(spv::CapabilityGroupNonUniformVote);
        break;
    case glslang::EOpSubgroupBroadcast:
    case glslang::EOpSubgroupBroadcastFirst:
    case glslang::EOpSubgroupBallot:
    case glslang::EOpSubgroupInverseBallot:
    case glslang::EOpSubgroupBallotBitExtract:
    case glslang::EOpSubgroupBallotBitCount:
    case glslang::EOpSubgroupBallotInclusiveBitCount:
    case glslang::EOpSubgroupBallotExclusiveBitCount:
    case glslang::EOpSubgroupBallotFindLSB:
    case glslang::EOpSubgroupBallotFindMSB:
        builder.addCapability(spv::CapabilityGroupNonUniform);
        builder.addCapability(spv::CapabilityGroupNonUniformBallot);
        break;
    case glslang::EOpSubgroupShuffle:
    case glslang::EOpSubgroupShuffleXor:
        builder.addCapability(spv::CapabilityGroupNonUniform);
        builder.addCapability(spv::CapabilityGroupNonUniformShuffle);
        break;
    case glslang::EOpSubgroupShuffleUp:
    case glslang::EOpSubgroupShuffleDown:
        builder.addCapability(spv::CapabilityGroupNonUniform);
        builder.addCapability(spv::CapabilityGroupNonUniformShuffleRelative);
        break;
    case glslang::EOpSubgroupAdd:
    case glslang::EOpSubgroupMul:
    case glslang::EOpSubgroupMin:
    case glslang::EOpSubgroupMax:
    case glslang::EOpSubgroupAnd:
    case glslang::EOpSubgroupOr:
    case glslang::EOpSubgroupXor:
    case glslang::EOpSubgroupInclusiveAdd:
    case glslang::EOpSubgroupInclusiveMul:
    case glslang::EOpSubgroupInclusiveMin:
    case glslang::EOpSubgroupInclusiveMax:
    case glslang::EOpSubgroupInclusiveAnd:
    case glslang::EOpSubgroupInclusiveOr:
    case glslang::EOpSubgroupInclusiveXor:
    case glslang::EOpSubgroupExclusiveAdd:
    case glslang::EOpSubgroupExclusiveMul:
    case glslang::EOpSubgroupExclusiveMin:
    case glslang::EOpSubgroupExclusiveMax:
    case glslang::EOpSubgroupExclusiveAnd:
    case glslang::EOpSubgroupExclusiveOr:
    case glslang::EOpSubgroupExclusiveXor:
        builder.addCapability(spv::CapabilityGroupNonUniform);
        builder.addCapability(spv::CapabilityGroupNonUniformArithmetic);
        break;
    case glslang::EOpSubgroupClusteredAdd:
    case glslang::EOpSubgroupClusteredMul:
    case glslang::EOpSubgroupClusteredMin:
    case glslang::EOpSubgroupClusteredMax:
    case glslang::EOpSubgroupClusteredAnd:
    case glslang::EOpSubgroupClusteredOr:
    case glslang::EOpSubgroupClusteredXor:
        builder.addCapability(spv::CapabilityGroupNonUniform);
        builder.addCapability(spv::CapabilityGroupNonUniformClustered);
        break;
    case glslang::EOpSubgroupQuadBroadcast:
    case glslang::EOpSubgroupQuadSwapHorizontal:
    case glslang::EOpSubgroupQuadSwapVertical:
    case glslang::EOpSubgroupQuadSwapDiagonal:
        builder.addCapability(spv::CapabilityGroupNonUniform);
        builder.addCapability(spv::CapabilityGroupNonUniformQuad);
        break;
#ifdef NV_EXTENSIONS
    case glslang::EOpSubgroupPartitionedAdd:
    case glslang::EOpSubgroupPartitionedMul:
    case glslang::EOpSubgroupPartitionedMin:
    case glslang::EOpSubgroupPartitionedMax:
    case glslang::EOpSubgroupPartitionedAnd:
    case glslang::EOpSubgroupPartitionedOr:
    case glslang::EOpSubgroupPartitionedXor:
    case glslang::EOpSubgroupPartitionedInclusiveAdd:
    case glslang::EOpSubgroupPartitionedInclusiveMul:
    case glslang::EOpSubgroupPartitionedInclusiveMin:
    case glslang::EOpSubgroupPartitionedInclusiveMax:
    case glslang::EOpSubgroupPartitionedInclusiveAnd:
    case glslang::EOpSubgroupPartitionedInclusiveOr:
    case glslang::EOpSubgroupPartitionedInclusiveXor:
    case glslang::EOpSubgroupPartitionedExclusiveAdd:
    case glslang::EOpSubgroupPartitionedExclusiveMul:
    case glslang::EOpSubgroupPartitionedExclusiveMin:
    case glslang::EOpSubgroupPartitionedExclusiveMax:
    case glslang::EOpSubgroupPartitionedExclusiveAnd:
    case glslang::EOpSubgroupPartitionedExclusiveOr:
    case glslang::EOpSubgroupPartitionedExclusiveXor:
        builder.addExtension(spv::E_SPV_NV_shader_subgroup_partitioned);
        builder.addCapability(spv::CapabilityGroupNonUniformPartitionedNV);
        break;
#endif
    default: assert(0 && "Unhandled subgroup operation!");
    }

    const bool isUnsigned = typeProxy == glslang::EbtUint || typeProxy == glslang::EbtUint64;
    const bool isFloat = typeProxy == glslang::EbtFloat || typeProxy == glslang::EbtDouble;
    const bool isBool = typeProxy == glslang::EbtBool;

    spv::Op opCode = spv::OpNop;

    // Figure out which opcode to use.
    switch (op) {
    case glslang::EOpSubgroupElect:                   opCode = spv::OpGroupNonUniformElect; break;
    case glslang::EOpSubgroupAll:                     opCode = spv::OpGroupNonUniformAll; break;
    case glslang::EOpSubgroupAny:                     opCode = spv::OpGroupNonUniformAny; break;
    case glslang::EOpSubgroupAllEqual:                opCode = spv::OpGroupNonUniformAllEqual; break;
    case glslang::EOpSubgroupBroadcast:               opCode = spv::OpGroupNonUniformBroadcast; break;
    case glslang::EOpSubgroupBroadcastFirst:          opCode = spv::OpGroupNonUniformBroadcastFirst; break;
    case glslang::EOpSubgroupBallot:                  opCode = spv::OpGroupNonUniformBallot; break;
    case glslang::EOpSubgroupInverseBallot:           opCode = spv::OpGroupNonUniformInverseBallot; break;
    case glslang::EOpSubgroupBallotBitExtract:        opCode = spv::OpGroupNonUniformBallotBitExtract; break;
    case glslang::EOpSubgroupBallotBitCount:
    case glslang::EOpSubgroupBallotInclusiveBitCount:
    case glslang::EOpSubgroupBallotExclusiveBitCount: opCode = spv::OpGroupNonUniformBallotBitCount; break;
    case glslang::EOpSubgroupBallotFindLSB:           opCode = spv::OpGroupNonUniformBallotFindLSB; break;
    case glslang::EOpSubgroupBallotFindMSB:           opCode = spv::OpGroupNonUniformBallotFindMSB; break;
    case glslang::EOpSubgroupShuffle:                 opCode = spv::OpGroupNonUniformShuffle; break;
    case glslang::EOpSubgroupShuffleXor:              opCode = spv::OpGroupNonUniformShuffleXor; break;
    case glslang::EOpSubgroupShuffleUp:               opCode = spv::OpGroupNonUniformShuffleUp; break;
    case glslang::EOpSubgroupShuffleDown:             opCode = spv::OpGroupNonUniformShuffleDown; break;
    case glslang::EOpSubgroupAdd:
    case glslang::EOpSubgroupInclusiveAdd:
    case glslang::EOpSubgroupExclusiveAdd:
    case glslang::EOpSubgroupClusteredAdd:
#ifdef NV_EXTENSIONS
    case glslang::EOpSubgroupPartitionedAdd:
    case glslang::EOpSubgroupPartitionedInclusiveAdd:
    case glslang::EOpSubgroupPartitionedExclusiveAdd:
#endif
        if (isFloat) {
            opCode = spv::OpGroupNonUniformFAdd;
        } else {
            opCode = spv::OpGroupNonUniformIAdd;
        }
        break;
    case glslang::EOpSubgroupMul:
    case glslang::EOpSubgroupInclusiveMul:
    case glslang::EOpSubgroupExclusiveMul:
    case glslang::EOpSubgroupClusteredMul:
#ifdef NV_EXTENSIONS
    case glslang::EOpSubgroupPartitionedMul:
    case glslang::EOpSubgroupPartitionedInclusiveMul:
    case glslang::EOpSubgroupPartitionedExclusiveMul:
#endif
        if (isFloat) {
            opCode = spv::OpGroupNonUniformFMul;
        } else {
            opCode = spv::OpGroupNonUniformIMul;
        }
        break;
    case glslang::EOpSubgroupMin:
    case glslang::EOpSubgroupInclusiveMin:
    case glslang::EOpSubgroupExclusiveMin:
    case glslang::EOpSubgroupClusteredMin:
#ifdef NV_EXTENSIONS
    case glslang::EOpSubgroupPartitionedMin:
    case glslang::EOpSubgroupPartitionedInclusiveMin:
    case glslang::EOpSubgroupPartitionedExclusiveMin:
#endif
        if (isFloat) {
            opCode = spv::OpGroupNonUniformFMin;
        } else if (isUnsigned) {
            opCode = spv::OpGroupNonUniformUMin;
        } else {
            opCode = spv::OpGroupNonUniformSMin;
        }
        break;
    case glslang::EOpSubgroupMax:
    case glslang::EOpSubgroupInclusiveMax:
    case glslang::EOpSubgroupExclusiveMax:
    case glslang::EOpSubgroupClusteredMax:
#ifdef NV_EXTENSIONS
    case glslang::EOpSubgroupPartitionedMax:
    case glslang::EOpSubgroupPartitionedInclusiveMax:
    case glslang::EOpSubgroupPartitionedExclusiveMax:
#endif
        if (isFloat) {
            opCode = spv::OpGroupNonUniformFMax;
        } else if (isUnsigned) {
            opCode = spv::OpGroupNonUniformUMax;
        } else {
            opCode = spv::OpGroupNonUniformSMax;
        }
        break;
    case glslang::EOpSubgroupAnd:
    case glslang::EOpSubgroupInclusiveAnd:
    case glslang::EOpSubgroupExclusiveAnd:
    case glslang::EOpSubgroupClusteredAnd:
#ifdef NV_EXTENSIONS
    case glslang::EOpSubgroupPartitionedAnd:
    case glslang::EOpSubgroupPartitionedInclusiveAnd:
    case glslang::EOpSubgroupPartitionedExclusiveAnd:
#endif
        if (isBool) {
            opCode = spv::OpGroupNonUniformLogicalAnd;
        } else {
            opCode = spv::OpGroupNonUniformBitwiseAnd;
        }
        break;
    case glslang::EOpSubgroupOr:
    case glslang::EOpSubgroupInclusiveOr:
    case glslang::EOpSubgroupExclusiveOr:
    case glslang::EOpSubgroupClusteredOr:
#ifdef NV_EXTENSIONS
    case glslang::EOpSubgroupPartitionedOr:
    case glslang::EOpSubgroupPartitionedInclusiveOr:
    case glslang::EOpSubgroupPartitionedExclusiveOr:
#endif
        if (isBool) {
            opCode = spv::OpGroupNonUniformLogicalOr;
        } else {
            opCode = spv::OpGroupNonUniformBitwiseOr;
        }
        break;
    case glslang::EOpSubgroupXor:
    case glslang::EOpSubgroupInclusiveXor:
    case glslang::EOpSubgroupExclusiveXor:
    case glslang::EOpSubgroupClusteredXor:
#ifdef NV_EXTENSIONS
    case glslang::EOpSubgroupPartitionedXor:
    case glslang::EOpSubgroupPartitionedInclusiveXor:
    case glslang::EOpSubgroupPartitionedExclusiveXor:
#endif
        if (isBool) {
            opCode = spv::OpGroupNonUniformLogicalXor;
        } else {
            opCode = spv::OpGroupNonUniformBitwiseXor;
        }
        break;
    case glslang::EOpSubgroupQuadBroadcast:      opCode = spv::OpGroupNonUniformQuadBroadcast; break;
    case glslang::EOpSubgroupQuadSwapHorizontal:
    case glslang::EOpSubgroupQuadSwapVertical:
    case glslang::EOpSubgroupQuadSwapDiagonal:   opCode = spv::OpGroupNonUniformQuadSwap; break;
    default: assert(0 && "Unhandled subgroup operation!");
    }

    // get the right Group Operation
    spv::GroupOperation groupOperation = spv::GroupOperationMax;
    switch (op) {
    default:
        break;
    case glslang::EOpSubgroupBallotBitCount:
    case glslang::EOpSubgroupAdd:
    case glslang::EOpSubgroupMul:
    case glslang::EOpSubgroupMin:
    case glslang::EOpSubgroupMax:
    case glslang::EOpSubgroupAnd:
    case glslang::EOpSubgroupOr:
    case glslang::EOpSubgroupXor:
        groupOperation = spv::GroupOperationReduce;
        break;
    case glslang::EOpSubgroupBallotInclusiveBitCount:
    case glslang::EOpSubgroupInclusiveAdd:
    case glslang::EOpSubgroupInclusiveMul:
    case glslang::EOpSubgroupInclusiveMin:
    case glslang::EOpSubgroupInclusiveMax:
    case glslang::EOpSubgroupInclusiveAnd:
    case glslang::EOpSubgroupInclusiveOr:
    case glslang::EOpSubgroupInclusiveXor:
        groupOperation = spv::GroupOperationInclusiveScan;
        break;
    case glslang::EOpSubgroupBallotExclusiveBitCount:
    case glslang::EOpSubgroupExclusiveAdd:
    case glslang::EOpSubgroupExclusiveMul:
    case glslang::EOpSubgroupExclusiveMin:
    case glslang::EOpSubgroupExclusiveMax:
    case glslang::EOpSubgroupExclusiveAnd:
    case glslang::EOpSubgroupExclusiveOr:
    case glslang::EOpSubgroupExclusiveXor:
        groupOperation = spv::GroupOperationExclusiveScan;
        break;
    case glslang::EOpSubgroupClusteredAdd:
    case glslang::EOpSubgroupClusteredMul:
    case glslang::EOpSubgroupClusteredMin:
    case glslang::EOpSubgroupClusteredMax:
    case glslang::EOpSubgroupClusteredAnd:
    case glslang::EOpSubgroupClusteredOr:
    case glslang::EOpSubgroupClusteredXor:
        groupOperation = spv::GroupOperationClusteredReduce;
        break;
#ifdef NV_EXTENSIONS
    case glslang::EOpSubgroupPartitionedAdd:
    case glslang::EOpSubgroupPartitionedMul:
    case glslang::EOpSubgroupPartitionedMin:
    case glslang::EOpSubgroupPartitionedMax:
    case glslang::EOpSubgroupPartitionedAnd:
    case glslang::EOpSubgroupPartitionedOr:
    case glslang::EOpSubgroupPartitionedXor:
        groupOperation = spv::GroupOperationPartitionedReduceNV;
        break;
    case glslang::EOpSubgroupPartitionedInclusiveAdd:
    case glslang::EOpSubgroupPartitionedInclusiveMul:
    case glslang::EOpSubgroupPartitionedInclusiveMin:
    case glslang::EOpSubgroupPartitionedInclusiveMax:
    case glslang::EOpSubgroupPartitionedInclusiveAnd:
    case glslang::EOpSubgroupPartitionedInclusiveOr:
    case glslang::EOpSubgroupPartitionedInclusiveXor:
        groupOperation = spv::GroupOperationPartitionedInclusiveScanNV;
        break;
    case glslang::EOpSubgroupPartitionedExclusiveAdd:
    case glslang::EOpSubgroupPartitionedExclusiveMul:
    case glslang::EOpSubgroupPartitionedExclusiveMin:
    case glslang::EOpSubgroupPartitionedExclusiveMax:
    case glslang::EOpSubgroupPartitionedExclusiveAnd:
    case glslang::EOpSubgroupPartitionedExclusiveOr:
    case glslang::EOpSubgroupPartitionedExclusiveXor:
        groupOperation = spv::GroupOperationPartitionedExclusiveScanNV;
        break;
#endif
    }

    // build the instruction
    std::vector<spv::IdImmediate> spvGroupOperands;

    // Every operation begins with the Execution Scope operand.
    spv::IdImmediate executionScope = { true, builder.makeUintConstant(spv::ScopeSubgroup) };
    spvGroupOperands.push_back(executionScope);

    // Next, for all operations that use a Group Operation, push that as an operand.
    if (groupOperation != spv::GroupOperationMax) {
        spv::IdImmediate groupOperand = { false, groupOperation };
        spvGroupOperands.push_back(groupOperand);
    }

    // Push back the operands next.
    for (auto opIt = operands.cbegin(); opIt != operands.cend(); ++opIt) {
        spv::IdImmediate operand = { true, *opIt };
        spvGroupOperands.push_back(operand);
    }

    // Some opcodes have additional operands.
    spv::Id directionId = spv::NoResult;
    switch (op) {
    default: break;
    case glslang::EOpSubgroupQuadSwapHorizontal: directionId = builder.makeUintConstant(0); break;
    case glslang::EOpSubgroupQuadSwapVertical:   directionId = builder.makeUintConstant(1); break;
    case glslang::EOpSubgroupQuadSwapDiagonal:   directionId = builder.makeUintConstant(2); break;
    }
    if (directionId != spv::NoResult) {
        spv::IdImmediate direction = { true, directionId };
        spvGroupOperands.push_back(direction);
    }

    return builder.createOp(opCode, typeId, spvGroupOperands);
}

spv::Id TGlslangToSpvTraverser::createMiscOperation(glslang::TOperator op, spv::Decoration precision, spv::Id typeId, std::vector<spv::Id>& operands, glslang::TBasicType typeProxy)
{
    bool isUnsigned = isTypeUnsignedInt(typeProxy);
    bool isFloat = isTypeFloat(typeProxy);

    spv::Op opCode = spv::OpNop;
    int extBuiltins = -1;
    int libCall = -1;
    size_t consumedOperands = operands.size();
    spv::Id typeId0 = 0;
    if (consumedOperands > 0)
        typeId0 = builder.getTypeId(operands[0]);
    spv::Id typeId1 = 0;
    if (consumedOperands > 1)
        typeId1 = builder.getTypeId(operands[1]);
    spv::Id frexpIntType = 0;

    switch (op) {
    case glslang::EOpMin:
        if (isFloat)
            libCall = spv::GLSLstd450FMin;
        else if (isUnsigned)
            libCall = spv::GLSLstd450UMin;
        else
            libCall = spv::GLSLstd450SMin;
        builder.promoteScalar(precision, operands.front(), operands.back());
        break;
    case glslang::EOpModf:
        libCall = spv::GLSLstd450Modf;
        break;
    case glslang::EOpMax:
        if (isFloat)
            libCall = spv::GLSLstd450FMax;
        else if (isUnsigned)
            libCall = spv::GLSLstd450UMax;
        else
            libCall = spv::GLSLstd450SMax;
        builder.promoteScalar(precision, operands.front(), operands.back());
        break;
    case glslang::EOpPow:
        libCall = spv::GLSLstd450Pow;
        break;
    case glslang::EOpDot:
        opCode = spv::OpDot;
        break;
    case glslang::EOpAtan:
        libCall = spv::GLSLstd450Atan2;
        break;

    case glslang::EOpClamp:
        if (isFloat)
            libCall = spv::GLSLstd450FClamp;
        else if (isUnsigned)
            libCall = spv::GLSLstd450UClamp;
        else
            libCall = spv::GLSLstd450SClamp;
        builder.promoteScalar(precision, operands.front(), operands[1]);
        builder.promoteScalar(precision, operands.front(), operands[2]);
        break;
    case glslang::EOpMix:
        if (! builder.isBoolType(builder.getScalarTypeId(builder.getTypeId(operands.back())))) {
            assert(isFloat);
            libCall = spv::GLSLstd450FMix;
        } else {
            opCode = spv::OpSelect;
            std::swap(operands.front(), operands.back());
        }
        builder.promoteScalar(precision, operands.front(), operands.back());
        break;
    case glslang::EOpStep:
        libCall = spv::GLSLstd450Step;
        builder.promoteScalar(precision, operands.front(), operands.back());
        break;
    case glslang::EOpSmoothStep:
        libCall = spv::GLSLstd450SmoothStep;
        builder.promoteScalar(precision, operands[0], operands[2]);
        builder.promoteScalar(precision, operands[1], operands[2]);
        break;

    case glslang::EOpDistance:
        libCall = spv::GLSLstd450Distance;
        break;
    case glslang::EOpCross:
        libCall = spv::GLSLstd450Cross;
        break;
    case glslang::EOpFaceForward:
        libCall = spv::GLSLstd450FaceForward;
        break;
    case glslang::EOpReflect:
        libCall = spv::GLSLstd450Reflect;
        break;
    case glslang::EOpRefract:
        libCall = spv::GLSLstd450Refract;
        break;
    case glslang::EOpInterpolateAtSample:
#ifdef AMD_EXTENSIONS
        if (typeProxy == glslang::EbtFloat16)
            builder.addExtension(spv::E_SPV_AMD_gpu_shader_half_float);
#endif
        libCall = spv::GLSLstd450InterpolateAtSample;
        break;
    case glslang::EOpInterpolateAtOffset:
#ifdef AMD_EXTENSIONS
        if (typeProxy == glslang::EbtFloat16)
            builder.addExtension(spv::E_SPV_AMD_gpu_shader_half_float);
#endif
        libCall = spv::GLSLstd450InterpolateAtOffset;
        break;
    case glslang::EOpAddCarry:
        opCode = spv::OpIAddCarry;
        typeId = builder.makeStructResultType(typeId0, typeId0);
        consumedOperands = 2;
        break;
    case glslang::EOpSubBorrow:
        opCode = spv::OpISubBorrow;
        typeId = builder.makeStructResultType(typeId0, typeId0);
        consumedOperands = 2;
        break;
    case glslang::EOpUMulExtended:
        opCode = spv::OpUMulExtended;
        typeId = builder.makeStructResultType(typeId0, typeId0);
        consumedOperands = 2;
        break;
    case glslang::EOpIMulExtended:
        opCode = spv::OpSMulExtended;
        typeId = builder.makeStructResultType(typeId0, typeId0);
        consumedOperands = 2;
        break;
    case glslang::EOpBitfieldExtract:
        if (isUnsigned)
            opCode = spv::OpBitFieldUExtract;
        else
            opCode = spv::OpBitFieldSExtract;
        break;
    case glslang::EOpBitfieldInsert:
        opCode = spv::OpBitFieldInsert;
        break;

    case glslang::EOpFma:
        libCall = spv::GLSLstd450Fma;
        break;
    case glslang::EOpFrexp:
        {
            libCall = spv::GLSLstd450FrexpStruct;
            assert(builder.isPointerType(typeId1));
            typeId1 = builder.getContainedTypeId(typeId1);
            int width = builder.getScalarTypeWidth(typeId1);
#ifdef AMD_EXTENSIONS
            if (width == 16)
                // Using 16-bit exp operand, enable extension SPV_AMD_gpu_shader_int16
                builder.addExtension(spv::E_SPV_AMD_gpu_shader_int16);
#endif
            if (builder.getNumComponents(operands[0]) == 1)
                frexpIntType = builder.makeIntegerType(width, true);
            else
                frexpIntType = builder.makeVectorType(builder.makeIntegerType(width, true), builder.getNumComponents(operands[0]));
            typeId = builder.makeStructResultType(typeId0, frexpIntType);
            consumedOperands = 1;
        }
        break;
    case glslang::EOpLdexp:
        libCall = spv::GLSLstd450Ldexp;
        break;

    case glslang::EOpReadInvocation:
        return createInvocationsOperation(op, typeId, operands, typeProxy);

    case glslang::EOpSubgroupBroadcast:
    case glslang::EOpSubgroupBallotBitExtract:
    case glslang::EOpSubgroupShuffle:
    case glslang::EOpSubgroupShuffleXor:
    case glslang::EOpSubgroupShuffleUp:
    case glslang::EOpSubgroupShuffleDown:
    case glslang::EOpSubgroupClusteredAdd:
    case glslang::EOpSubgroupClusteredMul:
    case glslang::EOpSubgroupClusteredMin:
    case glslang::EOpSubgroupClusteredMax:
    case glslang::EOpSubgroupClusteredAnd:
    case glslang::EOpSubgroupClusteredOr:
    case glslang::EOpSubgroupClusteredXor:
    case glslang::EOpSubgroupQuadBroadcast:
#ifdef NV_EXTENSIONS
    case glslang::EOpSubgroupPartitionedAdd:
    case glslang::EOpSubgroupPartitionedMul:
    case glslang::EOpSubgroupPartitionedMin:
    case glslang::EOpSubgroupPartitionedMax:
    case glslang::EOpSubgroupPartitionedAnd:
    case glslang::EOpSubgroupPartitionedOr:
    case glslang::EOpSubgroupPartitionedXor:
    case glslang::EOpSubgroupPartitionedInclusiveAdd:
    case glslang::EOpSubgroupPartitionedInclusiveMul:
    case glslang::EOpSubgroupPartitionedInclusiveMin:
    case glslang::EOpSubgroupPartitionedInclusiveMax:
    case glslang::EOpSubgroupPartitionedInclusiveAnd:
    case glslang::EOpSubgroupPartitionedInclusiveOr:
    case glslang::EOpSubgroupPartitionedInclusiveXor:
    case glslang::EOpSubgroupPartitionedExclusiveAdd:
    case glslang::EOpSubgroupPartitionedExclusiveMul:
    case glslang::EOpSubgroupPartitionedExclusiveMin:
    case glslang::EOpSubgroupPartitionedExclusiveMax:
    case glslang::EOpSubgroupPartitionedExclusiveAnd:
    case glslang::EOpSubgroupPartitionedExclusiveOr:
    case glslang::EOpSubgroupPartitionedExclusiveXor:
#endif
        return createSubgroupOperation(op, typeId, operands, typeProxy);

#ifdef AMD_EXTENSIONS
    case glslang::EOpSwizzleInvocations:
        extBuiltins = getExtBuiltins(spv::E_SPV_AMD_shader_ballot);
        libCall = spv::SwizzleInvocationsAMD;
        break;
    case glslang::EOpSwizzleInvocationsMasked:
        extBuiltins = getExtBuiltins(spv::E_SPV_AMD_shader_ballot);
        libCall = spv::SwizzleInvocationsMaskedAMD;
        break;
    case glslang::EOpWriteInvocation:
        extBuiltins = getExtBuiltins(spv::E_SPV_AMD_shader_ballot);
        libCall = spv::WriteInvocationAMD;
        break;

    case glslang::EOpMin3:
        extBuiltins = getExtBuiltins(spv::E_SPV_AMD_shader_trinary_minmax);
        if (isFloat)
            libCall = spv::FMin3AMD;
        else {
            if (isUnsigned)
                libCall = spv::UMin3AMD;
            else
                libCall = spv::SMin3AMD;
        }
        break;
    case glslang::EOpMax3:
        extBuiltins = getExtBuiltins(spv::E_SPV_AMD_shader_trinary_minmax);
        if (isFloat)
            libCall = spv::FMax3AMD;
        else {
            if (isUnsigned)
                libCall = spv::UMax3AMD;
            else
                libCall = spv::SMax3AMD;
        }
        break;
    case glslang::EOpMid3:
        extBuiltins = getExtBuiltins(spv::E_SPV_AMD_shader_trinary_minmax);
        if (isFloat)
            libCall = spv::FMid3AMD;
        else {
            if (isUnsigned)
                libCall = spv::UMid3AMD;
            else
                libCall = spv::SMid3AMD;
        }
        break;

    case glslang::EOpInterpolateAtVertex:
        if (typeProxy == glslang::EbtFloat16)
            builder.addExtension(spv::E_SPV_AMD_gpu_shader_half_float);
        extBuiltins = getExtBuiltins(spv::E_SPV_AMD_shader_explicit_vertex_parameter);
        libCall = spv::InterpolateAtVertexAMD;
        break;
#endif

    default:
        return 0;
    }

    spv::Id id = 0;
    if (libCall >= 0) {
        // Use an extended instruction from the standard library.
        // Construct the call arguments, without modifying the original operands vector.
        // We might need the remaining arguments, e.g. in the EOpFrexp case.
        std::vector<spv::Id> callArguments(operands.begin(), operands.begin() + consumedOperands);
        id = builder.createBuiltinCall(typeId, extBuiltins >= 0 ? extBuiltins : stdBuiltins, libCall, callArguments);
    } else {
        switch (consumedOperands) {
        case 0:
            // should all be handled by visitAggregate and createNoArgOperation
            assert(0);
            return 0;
        case 1:
            // should all be handled by createUnaryOperation
            assert(0);
            return 0;
        case 2:
            id = builder.createBinOp(opCode, typeId, operands[0], operands[1]);
            break;
        default:
            // anything 3 or over doesn't have l-value operands, so all should be consumed
            assert(consumedOperands == operands.size());
            id = builder.createOp(opCode, typeId, operands);
            break;
        }
    }

    // Decode the return types that were structures
    switch (op) {
    case glslang::EOpAddCarry:
    case glslang::EOpSubBorrow:
        builder.createStore(builder.createCompositeExtract(id, typeId0, 1), operands[2]);
        id = builder.createCompositeExtract(id, typeId0, 0);
        break;
    case glslang::EOpUMulExtended:
    case glslang::EOpIMulExtended:
        builder.createStore(builder.createCompositeExtract(id, typeId0, 0), operands[3]);
        builder.createStore(builder.createCompositeExtract(id, typeId0, 1), operands[2]);
        break;
    case glslang::EOpFrexp:
        {
            assert(operands.size() == 2);
            if (builder.isFloatType(builder.getScalarTypeId(typeId1))) {
                // "exp" is floating-point type (from HLSL intrinsic)
                spv::Id member1 = builder.createCompositeExtract(id, frexpIntType, 1);
                member1 = builder.createUnaryOp(spv::OpConvertSToF, typeId1, member1);
                builder.createStore(member1, operands[1]);
            } else
                // "exp" is integer type (from GLSL built-in function)
                builder.createStore(builder.createCompositeExtract(id, frexpIntType, 1), operands[1]);
            id = builder.createCompositeExtract(id, typeId0, 0);
        }
        break;
    default:
        break;
    }

    return builder.setPrecision(id, precision);
}

// Intrinsics with no arguments (or no return value, and no precision).
spv::Id TGlslangToSpvTraverser::createNoArgOperation(glslang::TOperator op, spv::Decoration precision, spv::Id typeId)
{
    // TODO: get the barrier operands correct

    switch (op) {
    case glslang::EOpEmitVertex:
        builder.createNoResultOp(spv::OpEmitVertex);
        return 0;
    case glslang::EOpEndPrimitive:
        builder.createNoResultOp(spv::OpEndPrimitive);
        return 0;
    case glslang::EOpBarrier:
        if (glslangIntermediate->getStage() == EShLangTessControl) {
            builder.createControlBarrier(spv::ScopeWorkgroup, spv::ScopeInvocation, spv::MemorySemanticsMaskNone);
            // TODO: prefer the following, when available:
            // builder.createControlBarrier(spv::ScopePatch, spv::ScopePatch,
            //                                 spv::MemorySemanticsPatchMask |
            //                                 spv::MemorySemanticsAcquireReleaseMask);
        } else {
            builder.createControlBarrier(spv::ScopeWorkgroup, spv::ScopeWorkgroup,
                                            spv::MemorySemanticsWorkgroupMemoryMask |
                                            spv::MemorySemanticsAcquireReleaseMask);
        }
        return 0;
    case glslang::EOpMemoryBarrier:
        builder.createMemoryBarrier(spv::ScopeDevice, spv::MemorySemanticsAllMemory |
                                                      spv::MemorySemanticsAcquireReleaseMask);
        return 0;
    case glslang::EOpMemoryBarrierAtomicCounter:
        builder.createMemoryBarrier(spv::ScopeDevice, spv::MemorySemanticsAtomicCounterMemoryMask |
                                                      spv::MemorySemanticsAcquireReleaseMask);
        return 0;
    case glslang::EOpMemoryBarrierBuffer:
        builder.createMemoryBarrier(spv::ScopeDevice, spv::MemorySemanticsUniformMemoryMask |
                                                      spv::MemorySemanticsAcquireReleaseMask);
        return 0;
    case glslang::EOpMemoryBarrierImage:
        builder.createMemoryBarrier(spv::ScopeDevice, spv::MemorySemanticsImageMemoryMask |
                                                      spv::MemorySemanticsAcquireReleaseMask);
        return 0;
    case glslang::EOpMemoryBarrierShared:
        builder.createMemoryBarrier(spv::ScopeDevice, spv::MemorySemanticsWorkgroupMemoryMask |
                                                      spv::MemorySemanticsAcquireReleaseMask);
        return 0;
    case glslang::EOpGroupMemoryBarrier:
        builder.createMemoryBarrier(spv::ScopeWorkgroup, spv::MemorySemanticsAllMemory |
                                                         spv::MemorySemanticsAcquireReleaseMask);
        return 0;
    case glslang::EOpAllMemoryBarrierWithGroupSync:
        builder.createControlBarrier(spv::ScopeWorkgroup, spv::ScopeDevice,
                                        spv::MemorySemanticsAllMemory |
                                        spv::MemorySemanticsAcquireReleaseMask);
        return 0;
    case glslang::EOpDeviceMemoryBarrier:
        builder.createMemoryBarrier(spv::ScopeDevice, spv::MemorySemanticsUniformMemoryMask |
                                                      spv::MemorySemanticsImageMemoryMask |
                                                      spv::MemorySemanticsAcquireReleaseMask);
        return 0;
    case glslang::EOpDeviceMemoryBarrierWithGroupSync:
        builder.createControlBarrier(spv::ScopeWorkgroup, spv::ScopeDevice, spv::MemorySemanticsUniformMemoryMask |
                                                                            spv::MemorySemanticsImageMemoryMask |
                                                                            spv::MemorySemanticsAcquireReleaseMask);
        return 0;
    case glslang::EOpWorkgroupMemoryBarrier:
        builder.createMemoryBarrier(spv::ScopeWorkgroup, spv::MemorySemanticsWorkgroupMemoryMask |
                                                         spv::MemorySemanticsAcquireReleaseMask);
        return 0;
    case glslang::EOpWorkgroupMemoryBarrierWithGroupSync:
        builder.createControlBarrier(spv::ScopeWorkgroup, spv::ScopeWorkgroup,
                                        spv::MemorySemanticsWorkgroupMemoryMask |
                                        spv::MemorySemanticsAcquireReleaseMask);
        return 0;
    case glslang::EOpSubgroupBarrier:
        builder.createControlBarrier(spv::ScopeSubgroup, spv::ScopeSubgroup, spv::MemorySemanticsAllMemory |
                                                                             spv::MemorySemanticsAcquireReleaseMask);
        return spv::NoResult;
    case glslang::EOpSubgroupMemoryBarrier:
        builder.createMemoryBarrier(spv::ScopeSubgroup, spv::MemorySemanticsAllMemory |
                                                        spv::MemorySemanticsAcquireReleaseMask);
        return spv::NoResult;
    case glslang::EOpSubgroupMemoryBarrierBuffer:
        builder.createMemoryBarrier(spv::ScopeSubgroup, spv::MemorySemanticsUniformMemoryMask |
                                                        spv::MemorySemanticsAcquireReleaseMask);
        return spv::NoResult;
    case glslang::EOpSubgroupMemoryBarrierImage:
        builder.createMemoryBarrier(spv::ScopeSubgroup, spv::MemorySemanticsImageMemoryMask |
                                                        spv::MemorySemanticsAcquireReleaseMask);
        return spv::NoResult;
    case glslang::EOpSubgroupMemoryBarrierShared:
        builder.createMemoryBarrier(spv::ScopeSubgroup, spv::MemorySemanticsWorkgroupMemoryMask |
                                                        spv::MemorySemanticsAcquireReleaseMask);
        return spv::NoResult;
    case glslang::EOpSubgroupElect: {
        std::vector<spv::Id> operands;
        return createSubgroupOperation(op, typeId, operands, glslang::EbtVoid);
    }
#ifdef AMD_EXTENSIONS
    case glslang::EOpTime:
    {
        std::vector<spv::Id> args; // Dummy arguments
        spv::Id id = builder.createBuiltinCall(typeId, getExtBuiltins(spv::E_SPV_AMD_gcn_shader), spv::TimeAMD, args);
        return builder.setPrecision(id, precision);
    }
#endif
    default:
        logger->missingFunctionality("unknown operation with no arguments");
        return 0;
    }
}

spv::Id TGlslangToSpvTraverser::getSymbolId(const glslang::TIntermSymbol* symbol)
{
    auto iter = symbolValues.find(symbol->getId());
    spv::Id id;
    if (symbolValues.end() != iter) {
        id = iter->second;
        return id;
    }

    // it was not found, create it
    id = createSpvVariable(symbol);
    symbolValues[symbol->getId()] = id;

    if (symbol->getBasicType() != glslang::EbtBlock) {
        builder.addDecoration(id, TranslatePrecisionDecoration(symbol->getType()));
        builder.addDecoration(id, TranslateInterpolationDecoration(symbol->getType().getQualifier()));
        builder.addDecoration(id, TranslateAuxiliaryStorageDecoration(symbol->getType().getQualifier()));
        if (symbol->getType().getQualifier().hasSpecConstantId())
            builder.addDecoration(id, spv::DecorationSpecId, symbol->getType().getQualifier().layoutSpecConstantId);
        if (symbol->getQualifier().hasIndex())
            builder.addDecoration(id, spv::DecorationIndex, symbol->getQualifier().layoutIndex);
        if (symbol->getQualifier().hasComponent())
            builder.addDecoration(id, spv::DecorationComponent, symbol->getQualifier().layoutComponent);
        // atomic counters use this:
        if (symbol->getQualifier().hasOffset())
            builder.addDecoration(id, spv::DecorationOffset, symbol->getQualifier().layoutOffset);
    }

    if (symbol->getQualifier().hasLocation())
        builder.addDecoration(id, spv::DecorationLocation, symbol->getQualifier().layoutLocation);
    builder.addDecoration(id, TranslateInvariantDecoration(symbol->getType().getQualifier()));
    if (symbol->getQualifier().hasStream() && glslangIntermediate->isMultiStream()) {
        builder.addCapability(spv::CapabilityGeometryStreams);
        builder.addDecoration(id, spv::DecorationStream, symbol->getQualifier().layoutStream);
    }
    if (symbol->getQualifier().hasSet())
        builder.addDecoration(id, spv::DecorationDescriptorSet, symbol->getQualifier().layoutSet);
    else if (IsDescriptorResource(symbol->getType())) {
        // default to 0
        builder.addDecoration(id, spv::DecorationDescriptorSet, 0);
    }
    if (symbol->getQualifier().hasBinding())
        builder.addDecoration(id, spv::DecorationBinding, symbol->getQualifier().layoutBinding);
    if (symbol->getQualifier().hasAttachment())
        builder.addDecoration(id, spv::DecorationInputAttachmentIndex, symbol->getQualifier().layoutAttachment);
    if (glslangIntermediate->getXfbMode()) {
        builder.addCapability(spv::CapabilityTransformFeedback);
        if (symbol->getQualifier().hasXfbStride())
            builder.addDecoration(id, spv::DecorationXfbStride, symbol->getQualifier().layoutXfbStride);
        if (symbol->getQualifier().hasXfbBuffer()) {
            builder.addDecoration(id, spv::DecorationXfbBuffer, symbol->getQualifier().layoutXfbBuffer);
            unsigned stride = glslangIntermediate->getXfbStride(symbol->getQualifier().layoutXfbBuffer);
            if (stride != glslang::TQualifier::layoutXfbStrideEnd)
                builder.addDecoration(id, spv::DecorationXfbStride, stride);
        }
        if (symbol->getQualifier().hasXfbOffset())
            builder.addDecoration(id, spv::DecorationOffset, symbol->getQualifier().layoutXfbOffset);
    }

    if (symbol->getType().isImage()) {
        std::vector<spv::Decoration> memory;
        TranslateMemoryDecoration(symbol->getType().getQualifier(), memory);
        for (unsigned int i = 0; i < memory.size(); ++i)
            builder.addDecoration(id, memory[i]);
    }

    // built-in variable decorations
    spv::BuiltIn builtIn = TranslateBuiltInDecoration(symbol->getQualifier().builtIn, false);
    if (builtIn != spv::BuiltInMax)
        builder.addDecoration(id, spv::DecorationBuiltIn, (int)builtIn);

    // nonuniform
    builder.addDecoration(id, TranslateNonUniformDecoration(symbol->getType().getQualifier()));

#ifdef NV_EXTENSIONS
    if (builtIn == spv::BuiltInSampleMask) {
          spv::Decoration decoration;
          // GL_NV_sample_mask_override_coverage extension
          if (glslangIntermediate->getLayoutOverrideCoverage())
              decoration = (spv::Decoration)spv::DecorationOverrideCoverageNV;
          else
              decoration = (spv::Decoration)spv::DecorationMax;
        builder.addDecoration(id, decoration);
        if (decoration != spv::DecorationMax) {
            builder.addExtension(spv::E_SPV_NV_sample_mask_override_coverage);
        }
    }
    else if (builtIn == spv::BuiltInLayer) {
        // SPV_NV_viewport_array2 extension
        if (symbol->getQualifier().layoutViewportRelative) {
            builder.addDecoration(id, (spv::Decoration)spv::DecorationViewportRelativeNV);
            builder.addCapability(spv::CapabilityShaderViewportMaskNV);
            builder.addExtension(spv::E_SPV_NV_viewport_array2);
        }
        if (symbol->getQualifier().layoutSecondaryViewportRelativeOffset != -2048) {
            builder.addDecoration(id, (spv::Decoration)spv::DecorationSecondaryViewportRelativeNV,
                                  symbol->getQualifier().layoutSecondaryViewportRelativeOffset);
            builder.addCapability(spv::CapabilityShaderStereoViewNV);
            builder.addExtension(spv::E_SPV_NV_stereo_view_rendering);
        }
    }

    if (symbol->getQualifier().layoutPassthrough) {
        builder.addDecoration(id, spv::DecorationPassthroughNV);
        builder.addCapability(spv::CapabilityGeometryShaderPassthroughNV);
        builder.addExtension(spv::E_SPV_NV_geometry_shader_passthrough);
    }
#endif

    if (glslangIntermediate->getHlslFunctionality1() && symbol->getType().getQualifier().semanticName != nullptr) {
        builder.addExtension("SPV_GOOGLE_hlsl_functionality1");
        builder.addDecoration(id, (spv::Decoration)spv::DecorationHlslSemanticGOOGLE,
                              symbol->getType().getQualifier().semanticName);
    }

    return id;
}

// Make a full tree of instructions to build a SPIR-V specialization constant,
// or regular constant if possible.
//
// TBD: this is not yet done, nor verified to be the best design, it does do the leaf symbols though
//
// Recursively walk the nodes.  The nodes form a tree whose leaves are
// regular constants, which themselves are trees that createSpvConstant()
// recursively walks.  So, this function walks the "top" of the tree:
//  - emit specialization constant-building instructions for specConstant
//  - when running into a non-spec-constant, switch to createSpvConstant()
spv::Id TGlslangToSpvTraverser::createSpvConstant(const glslang::TIntermTyped& node)
{
    assert(node.getQualifier().isConstant());

    // Handle front-end constants first (non-specialization constants).
    if (! node.getQualifier().specConstant) {
        // hand off to the non-spec-constant path
        assert(node.getAsConstantUnion() != nullptr || node.getAsSymbolNode() != nullptr);
        int nextConst = 0;
        return createSpvConstantFromConstUnionArray(node.getType(), node.getAsConstantUnion() ? node.getAsConstantUnion()->getConstArray() : node.getAsSymbolNode()->getConstArray(),
                                 nextConst, false);
    }

    // We now know we have a specialization constant to build

    // gl_WorkGroupSize is a special case until the front-end handles hierarchical specialization constants,
    // even then, it's specialization ids are handled by special case syntax in GLSL: layout(local_size_x = ...
    if (node.getType().getQualifier().builtIn == glslang::EbvWorkGroupSize) {
        std::vector<spv::Id> dimConstId;
        for (int dim = 0; dim < 3; ++dim) {
            bool specConst = (glslangIntermediate->getLocalSizeSpecId(dim) != glslang::TQualifier::layoutNotSet);
            dimConstId.push_back(builder.makeUintConstant(glslangIntermediate->getLocalSize(dim), specConst));
            if (specConst) {
                builder.addDecoration(dimConstId.back(), spv::DecorationSpecId,
                                      glslangIntermediate->getLocalSizeSpecId(dim));
            }
        }
        return builder.makeCompositeConstant(builder.makeVectorType(builder.makeUintType(32), 3), dimConstId, true);
    }

    // An AST node labelled as specialization constant should be a symbol node.
    // Its initializer should either be a sub tree with constant nodes, or a constant union array.
    if (auto* sn = node.getAsSymbolNode()) {
        if (auto* sub_tree = sn->getConstSubtree()) {
            // Traverse the constant constructor sub tree like generating normal run-time instructions.
            // During the AST traversal, if the node is marked as 'specConstant', SpecConstantOpModeGuard
            // will set the builder into spec constant op instruction generating mode.
            sub_tree->traverse(this);
            return accessChainLoad(sub_tree->getType());
        } else if (auto* const_union_array = &sn->getConstArray()){
            int nextConst = 0;
            spv::Id id = createSpvConstantFromConstUnionArray(sn->getType(), *const_union_array, nextConst, true);
            builder.addName(id, sn->getName().c_str());
            return id;
        }
    }

    // Neither a front-end constant node, nor a specialization constant node with constant union array or
    // constant sub tree as initializer.
    logger->missingFunctionality("Neither a front-end constant nor a spec constant.");
    exit(1);
    return spv::NoResult;
}

// Use 'consts' as the flattened glslang source of scalar constants to recursively
// build the aggregate SPIR-V constant.
//
// If there are not enough elements present in 'consts', 0 will be substituted;
// an empty 'consts' can be used to create a fully zeroed SPIR-V constant.
//
spv::Id TGlslangToSpvTraverser::createSpvConstantFromConstUnionArray(const glslang::TType& glslangType, const glslang::TConstUnionArray& consts, int& nextConst, bool specConstant)
{
    // vector of constants for SPIR-V
    std::vector<spv::Id> spvConsts;

    // Type is used for struct and array constants
    spv::Id typeId = convertGlslangToSpvType(glslangType);

    if (glslangType.isArray()) {
        glslang::TType elementType(glslangType, 0);
        for (int i = 0; i < glslangType.getOuterArraySize(); ++i)
            spvConsts.push_back(createSpvConstantFromConstUnionArray(elementType, consts, nextConst, false));
    } else if (glslangType.isMatrix()) {
        glslang::TType vectorType(glslangType, 0);
        for (int col = 0; col < glslangType.getMatrixCols(); ++col)
            spvConsts.push_back(createSpvConstantFromConstUnionArray(vectorType, consts, nextConst, false));
    } else if (glslangType.getStruct()) {
        glslang::TVector<glslang::TTypeLoc>::const_iterator iter;
        for (iter = glslangType.getStruct()->begin(); iter != glslangType.getStruct()->end(); ++iter)
            spvConsts.push_back(createSpvConstantFromConstUnionArray(*iter->type, consts, nextConst, false));
    } else if (glslangType.getVectorSize() > 1) {
        for (unsigned int i = 0; i < (unsigned int)glslangType.getVectorSize(); ++i) {
            bool zero = nextConst >= consts.size();
            switch (glslangType.getBasicType()) {
            case glslang::EbtInt8:
                spvConsts.push_back(builder.makeInt8Constant(zero ? 0 : consts[nextConst].getI8Const()));
                break;
            case glslang::EbtUint8:
                spvConsts.push_back(builder.makeUint8Constant(zero ? 0 : consts[nextConst].getU8Const()));
                break;
            case glslang::EbtInt16:
                spvConsts.push_back(builder.makeInt16Constant(zero ? 0 : consts[nextConst].getI16Const()));
                break;
            case glslang::EbtUint16:
                spvConsts.push_back(builder.makeUint16Constant(zero ? 0 : consts[nextConst].getU16Const()));
                break;
            case glslang::EbtInt:
                spvConsts.push_back(builder.makeIntConstant(zero ? 0 : consts[nextConst].getIConst()));
                break;
            case glslang::EbtUint:
                spvConsts.push_back(builder.makeUintConstant(zero ? 0 : consts[nextConst].getUConst()));
                break;
            case glslang::EbtInt64:
                spvConsts.push_back(builder.makeInt64Constant(zero ? 0 : consts[nextConst].getI64Const()));
                break;
            case glslang::EbtUint64:
                spvConsts.push_back(builder.makeUint64Constant(zero ? 0 : consts[nextConst].getU64Const()));
                break;
            case glslang::EbtFloat:
                spvConsts.push_back(builder.makeFloatConstant(zero ? 0.0F : (float)consts[nextConst].getDConst()));
                break;
            case glslang::EbtDouble:
                spvConsts.push_back(builder.makeDoubleConstant(zero ? 0.0 : consts[nextConst].getDConst()));
                break;
            case glslang::EbtFloat16:
                spvConsts.push_back(builder.makeFloat16Constant(zero ? 0.0F : (float)consts[nextConst].getDConst()));
                break;
            case glslang::EbtBool:
                spvConsts.push_back(builder.makeBoolConstant(zero ? false : consts[nextConst].getBConst()));
                break;
            default:
                assert(0);
                break;
            }
            ++nextConst;
        }
    } else {
        // we have a non-aggregate (scalar) constant
        bool zero = nextConst >= consts.size();
        spv::Id scalar = 0;
        switch (glslangType.getBasicType()) {
        case glslang::EbtInt8:
            scalar = builder.makeInt8Constant(zero ? 0 : consts[nextConst].getI8Const(), specConstant);
            break;
        case glslang::EbtUint8:
            scalar = builder.makeUint8Constant(zero ? 0 : consts[nextConst].getU8Const(), specConstant);
            break;
        case glslang::EbtInt16:
            scalar = builder.makeInt16Constant(zero ? 0 : consts[nextConst].getI16Const(), specConstant);
            break;
        case glslang::EbtUint16:
            scalar = builder.makeUint16Constant(zero ? 0 : consts[nextConst].getU16Const(), specConstant);
            break;
        case glslang::EbtInt:
            scalar = builder.makeIntConstant(zero ? 0 : consts[nextConst].getIConst(), specConstant);
            break;
        case glslang::EbtUint:
            scalar = builder.makeUintConstant(zero ? 0 : consts[nextConst].getUConst(), specConstant);
            break;
        case glslang::EbtInt64:
            scalar = builder.makeInt64Constant(zero ? 0 : consts[nextConst].getI64Const(), specConstant);
            break;
        case glslang::EbtUint64:
            scalar = builder.makeUint64Constant(zero ? 0 : consts[nextConst].getU64Const(), specConstant);
            break;
        case glslang::EbtFloat:
            scalar = builder.makeFloatConstant(zero ? 0.0F : (float)consts[nextConst].getDConst(), specConstant);
            break;
        case glslang::EbtDouble:
            scalar = builder.makeDoubleConstant(zero ? 0.0 : consts[nextConst].getDConst(), specConstant);
            break;
        case glslang::EbtFloat16:
            scalar = builder.makeFloat16Constant(zero ? 0.0F : (float)consts[nextConst].getDConst(), specConstant);
            break;
        case glslang::EbtBool:
            scalar = builder.makeBoolConstant(zero ? false : consts[nextConst].getBConst(), specConstant);
            break;
        default:
            assert(0);
            break;
        }
        ++nextConst;
        return scalar;
    }

    return builder.makeCompositeConstant(typeId, spvConsts);
}

// Return true if the node is a constant or symbol whose reading has no
// non-trivial observable cost or effect.
bool TGlslangToSpvTraverser::isTrivialLeaf(const glslang::TIntermTyped* node)
{
    // don't know what this is
    if (node == nullptr)
        return false;

    // a constant is safe
    if (node->getAsConstantUnion() != nullptr)
        return true;

    // not a symbol means non-trivial
    if (node->getAsSymbolNode() == nullptr)
        return false;

    // a symbol, depends on what's being read
    switch (node->getType().getQualifier().storage) {
    case glslang::EvqTemporary:
    case glslang::EvqGlobal:
    case glslang::EvqIn:
    case glslang::EvqInOut:
    case glslang::EvqConst:
    case glslang::EvqConstReadOnly:
    case glslang::EvqUniform:
        return true;
    default:
        return false;
    }
}

// A node is trivial if it is a single operation with no side effects.
// HLSL (and/or vectors) are always trivial, as it does not short circuit.
// Otherwise, error on the side of saying non-trivial.
// Return true if trivial.
bool TGlslangToSpvTraverser::isTrivial(const glslang::TIntermTyped* node)
{
    if (node == nullptr)
        return false;

    // count non scalars as trivial, as well as anything coming from HLSL
    if (! node->getType().isScalarOrVec1() || glslangIntermediate->getSource() == glslang::EShSourceHlsl)
        return true;

    // symbols and constants are trivial
    if (isTrivialLeaf(node))
        return true;

    // otherwise, it needs to be a simple operation or one or two leaf nodes

    // not a simple operation
    const glslang::TIntermBinary* binaryNode = node->getAsBinaryNode();
    const glslang::TIntermUnary* unaryNode = node->getAsUnaryNode();
    if (binaryNode == nullptr && unaryNode == nullptr)
        return false;

    // not on leaf nodes
    if (binaryNode && (! isTrivialLeaf(binaryNode->getLeft()) || ! isTrivialLeaf(binaryNode->getRight())))
        return false;

    if (unaryNode && ! isTrivialLeaf(unaryNode->getOperand())) {
        return false;
    }

    switch (node->getAsOperator()->getOp()) {
    case glslang::EOpLogicalNot:
    case glslang::EOpConvIntToBool:
    case glslang::EOpConvUintToBool:
    case glslang::EOpConvFloatToBool:
    case glslang::EOpConvDoubleToBool:
    case glslang::EOpEqual:
    case glslang::EOpNotEqual:
    case glslang::EOpLessThan:
    case glslang::EOpGreaterThan:
    case glslang::EOpLessThanEqual:
    case glslang::EOpGreaterThanEqual:
    case glslang::EOpIndexDirect:
    case glslang::EOpIndexDirectStruct:
    case glslang::EOpLogicalXor:
    case glslang::EOpAny:
    case glslang::EOpAll:
        return true;
    default:
        return false;
    }
}

// Emit short-circuiting code, where 'right' is never evaluated unless
// the left side is true (for &&) or false (for ||).
spv::Id TGlslangToSpvTraverser::createShortCircuit(glslang::TOperator op, glslang::TIntermTyped& left, glslang::TIntermTyped& right)
{
    spv::Id boolTypeId = builder.makeBoolType();

    // emit left operand
    builder.clearAccessChain();
    left.traverse(this);
    spv::Id leftId = accessChainLoad(left.getType());

    // Operands to accumulate OpPhi operands
    std::vector<spv::Id> phiOperands;
    // accumulate left operand's phi information
    phiOperands.push_back(leftId);
    phiOperands.push_back(builder.getBuildPoint()->getId());

    // Make the two kinds of operation symmetric with a "!"
    //   || => emit "if (! left) result = right"
    //   && => emit "if (  left) result = right"
    //
    // TODO: this runtime "not" for || could be avoided by adding functionality
    // to 'builder' to have an "else" without an "then"
    if (op == glslang::EOpLogicalOr)
        leftId = builder.createUnaryOp(spv::OpLogicalNot, boolTypeId, leftId);

    // make an "if" based on the left value
    spv::Builder::If ifBuilder(leftId, spv::SelectionControlMaskNone, builder);

    // emit right operand as the "then" part of the "if"
    builder.clearAccessChain();
    right.traverse(this);
    spv::Id rightId = accessChainLoad(right.getType());

    // accumulate left operand's phi information
    phiOperands.push_back(rightId);
    phiOperands.push_back(builder.getBuildPoint()->getId());

    // finish the "if"
    ifBuilder.makeEndIf();

    // phi together the two results
    return builder.createOp(spv::OpPhi, boolTypeId, phiOperands);
}

#ifdef AMD_EXTENSIONS
// Return type Id of the imported set of extended instructions corresponds to the name.
// Import this set if it has not been imported yet.
spv::Id TGlslangToSpvTraverser::getExtBuiltins(const char* name)
{
    if (extBuiltinMap.find(name) != extBuiltinMap.end())
        return extBuiltinMap[name];
    else {
        builder.addExtension(name);
        spv::Id extBuiltins = builder.import(name);
        extBuiltinMap[name] = extBuiltins;
        return extBuiltins;
    }
}
#endif

};  // end anonymous namespace

namespace glslang {

void GetSpirvVersion(std::string& version)
{
    const int bufSize = 100;
    char buf[bufSize];
    snprintf(buf, bufSize, "0x%08x, Revision %d", spv::Version, spv::Revision);
    version = buf;
}

// For low-order part of the generator's magic number. Bump up
// when there is a change in the style (e.g., if SSA form changes,
// or a different instruction sequence to do something gets used).
int GetSpirvGeneratorVersion()
{
    // return 1; // start
    // return 2; // EOpAtomicCounterDecrement gets a post decrement, to map between GLSL -> SPIR-V
    // return 3; // change/correct barrier-instruction operands, to match memory model group decisions
    // return 4; // some deeper access chains: for dynamic vector component, and local Boolean component
    // return 5; // make OpArrayLength result type be an int with signedness of 0
    // return 6; // revert version 5 change, which makes a different (new) kind of incorrect code,
                 // versions 4 and 6 each generate OpArrayLength as it has long been done
    return 7; // GLSL volatile keyword maps to both SPIR-V decorations Volatile and Coherent
}

// Write SPIR-V out to a binary file
void OutputSpvBin(const std::vector<unsigned int>& spirv, const char* baseName)
{
    std::ofstream out;
    out.open(baseName, std::ios::binary | std::ios::out);
    if (out.fail())
        printf("ERROR: Failed to open file: %s\n", baseName);
    for (int i = 0; i < (int)spirv.size(); ++i) {
        unsigned int word = spirv[i];
        out.write((const char*)&word, 4);
    }
    out.close();
}

// Write SPIR-V out to a text file with 32-bit hexadecimal words
void OutputSpvHex(const std::vector<unsigned int>& spirv, const char* baseName, const char* varName)
{
    std::ofstream out;
    out.open(baseName, std::ios::binary | std::ios::out);
    if (out.fail())
        printf("ERROR: Failed to open file: %s\n", baseName);
    out << "\t// " << 
        glslang::GetSpirvGeneratorVersion() << "." << GLSLANG_MINOR_VERSION << "." << GLSLANG_PATCH_LEVEL <<
        std::endl;
    if (varName != nullptr) {
        out << "\t #pragma once" << std::endl;
        out << "const uint32_t " << varName << "[] = {" << std::endl;
    }
    const int WORDS_PER_LINE = 8;
    for (int i = 0; i < (int)spirv.size(); i += WORDS_PER_LINE) {
        out << "\t";
        for (int j = 0; j < WORDS_PER_LINE && i + j < (int)spirv.size(); ++j) {
            const unsigned int word = spirv[i + j];
            out << "0x" << std::hex << std::setw(8) << std::setfill('0') << word;
            if (i + j + 1 < (int)spirv.size()) {
                out << ",";
            }
        }
        out << std::endl;
    }
    if (varName != nullptr) {
        out << "};";
    }
    out.close();
}

//
// Set up the glslang traversal
//
void GlslangToSpv(const glslang::TIntermediate& intermediate, std::vector<unsigned int>& spirv, SpvOptions* options)
{
    spv::SpvBuildLogger logger;
    GlslangToSpv(intermediate, spirv, &logger, options);
}

void GlslangToSpv(const glslang::TIntermediate& intermediate, std::vector<unsigned int>& spirv,
                  spv::SpvBuildLogger* logger, SpvOptions* options)
{
    TIntermNode* root = intermediate.getTreeRoot();

    if (root == 0)
        return;

    glslang::SpvOptions defaultOptions;
    if (options == nullptr)
        options = &defaultOptions;

    glslang::GetThreadPoolAllocator().push();

    TGlslangToSpvTraverser it(intermediate.getSpv().spv, &intermediate, logger, *options);
    root->traverse(&it);
    it.finishSpv();
    it.dumpSpv(spirv);

#if ENABLE_OPT
    // If from HLSL, run spirv-opt to "legalize" the SPIR-V for Vulkan
    // eg. forward and remove memory writes of opaque types.
    if ((intermediate.getSource() == EShSourceHlsl ||
                options->optimizeSize) &&
            !options->disableOptimizer) {
        spv_target_env target_env = SPV_ENV_UNIVERSAL_1_2;

        spvtools::Optimizer optimizer(target_env);
        optimizer.SetMessageConsumer([](spv_message_level_t level,
                                         const char* source,
                                         const spv_position_t& position,
                                         const char* message) {
            std::cerr << StringifyMessage(level, source, position, message)
                      << std::endl;
        });

        optimizer.RegisterPass(CreateMergeReturnPass());
        optimizer.RegisterPass(CreateInlineExhaustivePass());
        optimizer.RegisterPass(CreateEliminateDeadFunctionsPass());
        optimizer.RegisterPass(CreateScalarReplacementPass());
        optimizer.RegisterPass(CreateLocalAccessChainConvertPass());
        optimizer.RegisterPass(CreateLocalSingleBlockLoadStoreElimPass());
        optimizer.RegisterPass(CreateLocalSingleStoreElimPass());
        optimizer.RegisterPass(CreateSimplificationPass());
        optimizer.RegisterPass(CreateAggressiveDCEPass());
        optimizer.RegisterPass(CreateVectorDCEPass());
        optimizer.RegisterPass(CreateDeadInsertElimPass());
        optimizer.RegisterPass(CreateAggressiveDCEPass());
        optimizer.RegisterPass(CreateDeadBranchElimPass());
        optimizer.RegisterPass(CreateBlockMergePass());
        optimizer.RegisterPass(CreateLocalMultiStoreElimPass());
        optimizer.RegisterPass(CreateIfConversionPass());
        optimizer.RegisterPass(CreateSimplificationPass());
        optimizer.RegisterPass(CreateAggressiveDCEPass());
        optimizer.RegisterPass(CreateVectorDCEPass());
        optimizer.RegisterPass(CreateDeadInsertElimPass());
        if (options->optimizeSize) {
            optimizer.RegisterPass(CreateRedundancyEliminationPass());
            // TODO(greg-lunarg): Add this when AMD driver issues are resolved
            // optimizer.RegisterPass(CreateCommonUniformElimPass());
        }
        optimizer.RegisterPass(CreateAggressiveDCEPass());
        optimizer.RegisterPass(CreateCFGCleanupPass());

        if (!optimizer.Run(spirv.data(), spirv.size(), &spirv))
            return;
    }
#endif

    glslang::GetThreadPoolAllocator().pop();
}

}; // end namespace glslang
