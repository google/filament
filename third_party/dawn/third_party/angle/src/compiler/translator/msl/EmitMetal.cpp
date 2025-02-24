//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include <cctype>
#include <map>

#include "common/system_utils.h"
#include "compiler/translator/BaseTypes.h"
#include "compiler/translator/ImmutableStringBuilder.h"
#include "compiler/translator/Name.h"
#include "compiler/translator/OutputTree.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/msl/AstHelpers.h"
#include "compiler/translator/msl/DebugSink.h"
#include "compiler/translator/msl/EmitMetal.h"
#include "compiler/translator/msl/Layout.h"
#include "compiler/translator/msl/ProgramPrelude.h"
#include "compiler/translator/msl/RewritePipelines.h"
#include "compiler/translator/msl/TranslatorMSL.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

using namespace sh;

////////////////////////////////////////////////////////////////////////////////

#if defined(ANGLE_ENABLE_ASSERTS)
using Sink = DebugSink;
#else
using Sink = TInfoSinkBase;
#endif

////////////////////////////////////////////////////////////////////////////////

namespace
{

struct VarDecl
{
    explicit VarDecl(const TVariable &var) : mVariable(&var), mIsField(false) {}
    explicit VarDecl(const TField &field) : mField(&field), mIsField(true) {}

    ANGLE_INLINE const TVariable &variable() const
    {
        ASSERT(isVariable());
        return *mVariable;
    }

    ANGLE_INLINE const TField &field() const
    {
        ASSERT(isField());
        return *mField;
    }

    ANGLE_INLINE bool isVariable() const { return !mIsField; }

    ANGLE_INLINE bool isField() const { return mIsField; }

    const TType &type() const { return isField() ? *field().type() : variable().getType(); }

    SymbolType symbolType() const
    {
        return isField() ? field().symbolType() : variable().symbolType();
    }

  private:
    union
    {
        const TVariable *mVariable;
        const TField *mField;
    };
    bool mIsField;
};

class GenMetalTraverser : public TIntermTraverser
{
  public:
    ~GenMetalTraverser() override;

    GenMetalTraverser(const TCompiler &compiler,
                      Sink &out,
                      IdGen &idGen,
                      const PipelineStructs &pipelineStructs,
                      SymbolEnv &symbolEnv,
                      const ShCompileOptions &compileOptions);

    void visitSymbol(TIntermSymbol *) override;
    void visitConstantUnion(TIntermConstantUnion *) override;
    bool visitSwizzle(Visit, TIntermSwizzle *) override;
    bool visitBinary(Visit, TIntermBinary *) override;
    bool visitUnary(Visit, TIntermUnary *) override;
    bool visitTernary(Visit, TIntermTernary *) override;
    bool visitIfElse(Visit, TIntermIfElse *) override;
    bool visitSwitch(Visit, TIntermSwitch *) override;
    bool visitCase(Visit, TIntermCase *) override;
    void visitFunctionPrototype(TIntermFunctionPrototype *) override;
    bool visitFunctionDefinition(Visit, TIntermFunctionDefinition *) override;
    bool visitAggregate(Visit, TIntermAggregate *) override;
    bool visitBlock(Visit, TIntermBlock *) override;
    bool visitGlobalQualifierDeclaration(Visit, TIntermGlobalQualifierDeclaration *) override;
    bool visitDeclaration(Visit, TIntermDeclaration *) override;
    bool visitLoop(Visit, TIntermLoop *) override;
    bool visitForLoop(TIntermLoop *);
    bool visitWhileLoop(TIntermLoop *);
    bool visitDoWhileLoop(TIntermLoop *);
    bool visitBranch(Visit, TIntermBranch *) override;

  private:
    using FuncToName = std::map<ImmutableString, Name>;
    static FuncToName BuildFuncToName();

    struct EmitVariableDeclarationConfig
    {
        bool isParameter                = false;
        bool isMainParameter            = false;
        bool emitPostQualifier          = false;
        bool isPacked                   = false;
        bool disableStructSpecifier     = false;
        bool isUBO                      = false;
        const AddressSpace *isPointer   = nullptr;
        const AddressSpace *isReference = nullptr;
    };

    struct EmitTypeConfig
    {
        const EmitVariableDeclarationConfig *evdConfig = nullptr;
    };

    void emitIndentation();
    void emitOpeningPointerParen();
    void emitClosingPointerParen();
    void emitFunctionSignature(const TFunction &func);
    void emitFunctionReturn(const TFunction &func);
    void emitFunctionParameter(const TFunction &func, const TVariable &param);

    void emitNameOf(const TField &object);
    void emitNameOf(const TSymbol &object);
    void emitNameOf(const VarDecl &object);

    void emitBareTypeName(const TType &type, const EmitTypeConfig &etConfig);
    void emitType(const TType &type, const EmitTypeConfig &etConfig);
    void emitPostQualifier(const EmitVariableDeclarationConfig &evdConfig,
                           const VarDecl &decl,
                           const TQualifier qualifier);

    void emitLoopBody(TIntermBlock *bodyNode);

    struct FieldAnnotationIndices
    {
        size_t attribute = 0;
        size_t color     = 0;
    };

    void emitFieldDeclaration(const TField &field,
                              const TStructure &parent,
                              FieldAnnotationIndices &annotationIndices);
    void emitAttributeDeclaration(const TField &field, FieldAnnotationIndices &annotationIndices);
    void emitUniformBufferDeclaration(const TField &field,
                                      FieldAnnotationIndices &annotationIndices);
    void emitStructDeclaration(const TType &type);
    void emitOrdinaryVariableDeclaration(const VarDecl &decl,
                                         const EmitVariableDeclarationConfig &evdConfig);
    void emitVariableDeclaration(const VarDecl &decl,
                                 const EmitVariableDeclarationConfig &evdConfig);

    void emitOpenBrace();
    void emitCloseBrace();

    void groupedTraverse(TIntermNode &node);

    const TField &getDirectField(const TFieldListCollection &fieldsNode,
                                 const TConstantUnion &index);
    const TField &getDirectField(const TIntermTyped &fieldsNode, TIntermTyped &indexNode);

    const TConstantUnion *emitConstantUnionArray(const TConstantUnion *const constUnion,
                                                 const size_t size);

    const TConstantUnion *emitConstantUnion(const TType &type, const TConstantUnion *constUnion);

    void emitSingleConstant(const TConstantUnion *const constUnion);

  private:
    Sink &mOut;
    const TCompiler &mCompiler;
    const PipelineStructs &mPipelineStructs;
    SymbolEnv &mSymbolEnv;
    IdGen &mIdGen;
    int mIndentLevel                  = -1;
    int mLastIndentationPos           = -1;
    int mOpenPointerParenCount        = 0;
    bool mParentIsSwitch              = false;
    bool isTraversingVertexMain       = false;
    bool mTemporarilyDisableSemicolon = false;
    std::unordered_map<const TSymbol *, Name> mRenamedSymbols;
    const FuncToName mFuncToName           = BuildFuncToName();
    size_t mMainTextureIndex               = 0;
    size_t mMainSamplerIndex               = 0;
    size_t mMainUniformBufferIndex         = 0;
    size_t mDriverUniformsBindingIndex     = 0;
    size_t mUBOArgumentBufferBindingIndex  = 0;
    bool mRasterOrderGroupsSupported       = false;
    bool mInjectAsmStatementIntoLoopBodies = false;
};
}  // anonymous namespace

GenMetalTraverser::~GenMetalTraverser()
{
    ASSERT(mIndentLevel == -1);
    ASSERT(!mParentIsSwitch);
    ASSERT(mOpenPointerParenCount == 0);
}

GenMetalTraverser::GenMetalTraverser(const TCompiler &compiler,
                                     Sink &out,
                                     IdGen &idGen,
                                     const PipelineStructs &pipelineStructs,
                                     SymbolEnv &symbolEnv,
                                     const ShCompileOptions &compileOptions)
    : TIntermTraverser(true, false, false),
      mOut(out),
      mCompiler(compiler),
      mPipelineStructs(pipelineStructs),
      mSymbolEnv(symbolEnv),
      mIdGen(idGen),
      mMainUniformBufferIndex(compileOptions.metal.defaultUniformsBindingIndex),
      mDriverUniformsBindingIndex(compileOptions.metal.driverUniformsBindingIndex),
      mUBOArgumentBufferBindingIndex(compileOptions.metal.UBOArgumentBufferBindingIndex),
      mRasterOrderGroupsSupported(compileOptions.pls.fragmentSyncType ==
                                  ShFragmentSynchronizationType::RasterOrderGroups_Metal),
      mInjectAsmStatementIntoLoopBodies(compileOptions.metal.injectAsmStatementIntoLoopBodies)
{}

void GenMetalTraverser::emitIndentation()
{
    ASSERT(mIndentLevel >= 0);

    if (mLastIndentationPos == mOut.size())
    {
        return;  // Line is already indented.
    }

    for (int i = 0; i < mIndentLevel; ++i)
    {
        mOut << "  ";
    }

    mLastIndentationPos = mOut.size();
}

void GenMetalTraverser::emitOpeningPointerParen()
{
    mOut << "(*";
    mOpenPointerParenCount++;
}

void GenMetalTraverser::emitClosingPointerParen()
{
    if (mOpenPointerParenCount > 0)
    {
        mOut << ")";
        mOpenPointerParenCount--;
    }
}

static const char *GetOperatorString(TOperator op,
                                     const TType &resultType,
                                     const TType *argType0,
                                     const TType *argType1,
                                     const TType *argType2)
{
    switch (op)
    {
        case TOperator::EOpComma:
            return ",";
        case TOperator::EOpAssign:
            return "=";
        case TOperator::EOpInitialize:
            return "=";
        case TOperator::EOpAddAssign:
            return "+=";
        case TOperator::EOpSubAssign:
            return "-=";
        case TOperator::EOpMulAssign:
            return "*=";
        case TOperator::EOpDivAssign:
            return "/=";
        case TOperator::EOpIModAssign:
            return "%=";
        case TOperator::EOpBitShiftLeftAssign:
            return "<<=";  // TODO: Check logical vs arithmetic shifting.
        case TOperator::EOpBitShiftRightAssign:
            return ">>=";  // TODO: Check logical vs arithmetic shifting.
        case TOperator::EOpBitwiseAndAssign:
            return "&=";
        case TOperator::EOpBitwiseXorAssign:
            return "^=";
        case TOperator::EOpBitwiseOrAssign:
            return "|=";
        case TOperator::EOpAdd:
            return "+";
        case TOperator::EOpSub:
            return "-";
        case TOperator::EOpMul:
            return "*";
        case TOperator::EOpDiv:
            return "/";
        case TOperator::EOpIMod:
            return "%";
        case TOperator::EOpBitShiftLeft:
            return "<<";  // TODO: Check logical vs arithmetic shifting.
        case TOperator::EOpBitShiftRight:
            return ">>";  // TODO: Check logical vs arithmetic shifting.
        case TOperator::EOpBitwiseAnd:
            return "&";
        case TOperator::EOpBitwiseXor:
            return "^";
        case TOperator::EOpBitwiseOr:
            return "|";
        case TOperator::EOpLessThan:
            return "<";
        case TOperator::EOpGreaterThan:
            return ">";
        case TOperator::EOpLessThanEqual:
            return "<=";
        case TOperator::EOpGreaterThanEqual:
            return ">=";
        case TOperator::EOpLessThanComponentWise:
            return "<";
        case TOperator::EOpLessThanEqualComponentWise:
            return "<=";
        case TOperator::EOpGreaterThanEqualComponentWise:
            return ">=";
        case TOperator::EOpGreaterThanComponentWise:
            return ">";
        case TOperator::EOpLogicalOr:
            return "||";
        case TOperator::EOpLogicalXor:
            return "!=/*xor*/";  // XXX: This might need to be handled differently for some obtuse
                                 // use case.
        case TOperator::EOpLogicalAnd:
            return "&&";
        case TOperator::EOpNegative:
            return "-";
        case TOperator::EOpPositive:
            if (argType0->isMatrix())
            {
                return "";
            }
            return "+";
        case TOperator::EOpLogicalNot:
            return "!";
        case TOperator::EOpNotComponentWise:
            return "!";
        case TOperator::EOpBitwiseNot:
            return "~";
        case TOperator::EOpPostIncrement:
            return "++";
        case TOperator::EOpPostDecrement:
            return "--";
        case TOperator::EOpPreIncrement:
            return "++";
        case TOperator::EOpPreDecrement:
            return "--";
        case TOperator::EOpVectorTimesScalarAssign:
            return "*=";
        case TOperator::EOpVectorTimesMatrixAssign:
            return "*=";
        case TOperator::EOpMatrixTimesScalarAssign:
            return "*=";
        case TOperator::EOpMatrixTimesMatrixAssign:
            return "*=";
        case TOperator::EOpVectorTimesScalar:
            return "*";
        case TOperator::EOpVectorTimesMatrix:
            return "*";
        case TOperator::EOpMatrixTimesVector:
            return "*";
        case TOperator::EOpMatrixTimesScalar:
            return "*";
        case TOperator::EOpMatrixTimesMatrix:
            return "*";
        case TOperator::EOpEqualComponentWise:
            return "==";
        case TOperator::EOpNotEqualComponentWise:
            return "!=";

        case TOperator::EOpEqual:
            if ((argType0->getStruct() && argType1->getStruct()) &&
                (argType0->isArray() && argType1->isArray()))
            {
                return "ANGLE_equalStructArray";
            }

            if ((argType0->isVector() && argType1->isVector()) ||
                (argType0->getStruct() && argType1->getStruct()) ||
                (argType0->isArray() && argType1->isArray()) ||
                (argType0->isMatrix() && argType1->isMatrix()))

            {
                return "ANGLE_equal";
            }

            return "==";

        case TOperator::EOpNotEqual:
            if ((argType0->getStruct() && argType1->getStruct()) &&
                (argType0->isArray() && argType1->isArray()))
            {
                return "ANGLE_notEqualStructArray";
            }

            if ((argType0->isVector() && argType1->isVector()) ||
                (argType0->isArray() && argType1->isArray()) ||
                (argType0->isMatrix() && argType1->isMatrix()))
            {
                return "ANGLE_notEqual";
            }
            else if (argType0->getStruct() && argType1->getStruct())
            {
                return "ANGLE_notEqualStruct";
            }
            return "!=";

        case TOperator::EOpKill:
            UNIMPLEMENTED();
            return "kill";
        case TOperator::EOpReturn:
            return "return";
        case TOperator::EOpBreak:
            return "break";
        case TOperator::EOpContinue:
            return "continue";

        case TOperator::EOpRadians:
            return "ANGLE_radians";
        case TOperator::EOpDegrees:
            return "ANGLE_degrees";
        case TOperator::EOpAtan:
            return argType1 == nullptr ? "metal::atan" : "metal::atan2";
        case TOperator::EOpMod:
            return "ANGLE_mod";  // differs from metal::mod
        case TOperator::EOpRefract:
            return argType0->isVector() ? "metal::refract" : "ANGLE_refract_scalar";
        case TOperator::EOpDistance:
            return argType0->isVector() ? "metal::distance" : "ANGLE_distance_scalar";
        case TOperator::EOpLength:
            return argType0->isVector() ? "metal::length" : "metal::abs";
        case TOperator::EOpDot:
            return argType0->isVector() ? "metal::dot" : "*";
        case TOperator::EOpNormalize:
            return argType0->isVector() ? "metal::fast::normalize" : "metal::sign";
        case TOperator::EOpFaceforward:
            return argType0->isVector() ? "metal::faceforward" : "ANGLE_faceforward_scalar";
        case TOperator::EOpReflect:
            return argType0->isVector() ? "metal::reflect" : "ANGLE_reflect_scalar";
        case TOperator::EOpMatrixCompMult:
            return "ANGLE_componentWiseMultiply";
        case TOperator::EOpOuterProduct:
            return "ANGLE_outerProduct";
        case TOperator::EOpSign:
            return argType0->getBasicType() == EbtFloat ? "metal::sign" : "ANGLE_sign_int";

        case TOperator::EOpAbs:
            return "metal::abs";
        case TOperator::EOpAll:
            return "metal::all";
        case TOperator::EOpAny:
            return "metal::any";
        case TOperator::EOpSin:
            return "metal::sin";
        case TOperator::EOpCos:
            return "metal::cos";
        case TOperator::EOpTan:
            return "metal::tan";
        case TOperator::EOpAsin:
            return "metal::asin";
        case TOperator::EOpAcos:
            return "metal::acos";
        case TOperator::EOpSinh:
            return "metal::sinh";
        case TOperator::EOpCosh:
            return "metal::cosh";
        case TOperator::EOpTanh:
            return resultType.getPrecision() == TPrecision::EbpHigh ? "metal::precise::tanh"
                                                                    : "metal::tanh";
        case TOperator::EOpAsinh:
            return "metal::asinh";
        case TOperator::EOpAcosh:
            return "metal::acosh";
        case TOperator::EOpAtanh:
            return "metal::atanh";
        case TOperator::EOpFma:
            return "metal::fma";
        case TOperator::EOpPow:
            return "metal::powr";  // GLSL's pow excludes negative x
        case TOperator::EOpExp:
            return "metal::exp";
        case TOperator::EOpExp2:
            return "metal::exp2";
        case TOperator::EOpLog:
            return "metal::log";
        case TOperator::EOpLog2:
            return "metal::log2";
        case TOperator::EOpSqrt:
            return "metal::sqrt";
        case TOperator::EOpFloor:
            return "metal::floor";
        case TOperator::EOpTrunc:
            return "metal::trunc";
        case TOperator::EOpCeil:
            return "metal::ceil";
        case TOperator::EOpFract:
            return "metal::fract";
        case TOperator::EOpMin:
            return "metal::min";
        case TOperator::EOpMax:
            return "metal::max";
        case TOperator::EOpRound:
            return "metal::round";
        case TOperator::EOpRoundEven:
            return "metal::rint";
        case TOperator::EOpClamp:
            return "metal::clamp";  // TODO fast vs precise namespace
        case TOperator::EOpSaturate:
            return "metal::saturate";  // TODO fast vs precise namespace
        case TOperator::EOpMix:
            if (!argType1->isScalar() && argType2 && argType2->getBasicType() == EbtBool)
            {
                return "ANGLE_mix_bool";
            }
            return "metal::mix";
        case TOperator::EOpStep:
            return "metal::step";
        case TOperator::EOpSmoothstep:
            return "metal::smoothstep";
        case TOperator::EOpModf:
            return "metal::modf";
        case TOperator::EOpIsnan:
            return "metal::isnan";
        case TOperator::EOpIsinf:
            return "metal::isinf";
        case TOperator::EOpLdexp:
            return "metal::ldexp";
        case TOperator::EOpFrexp:
            return "metal::frexp";
        case TOperator::EOpInversesqrt:
            return "metal::rsqrt";
        case TOperator::EOpCross:
            return "metal::cross";
        case TOperator::EOpDFdx:
            return "metal::dfdx";
        case TOperator::EOpDFdy:
            return "metal::dfdy";
        case TOperator::EOpFwidth:
            return "metal::fwidth";
        case TOperator::EOpTranspose:
            return "metal::transpose";
        case TOperator::EOpDeterminant:
            return "metal::determinant";

        case TOperator::EOpInverse:
            return "ANGLE_inverse";

        case TOperator::EOpInterpolateAtCentroid:
            return "ANGLE_interpolateAtCentroid";
        case TOperator::EOpInterpolateAtSample:
            return "ANGLE_interpolateAtSample";
        case TOperator::EOpInterpolateAtOffset:
            return "ANGLE_interpolateAtOffset";
        case TOperator::EOpInterpolateAtCenter:
            return "ANGLE_interpolateAtCenter";

        case TOperator::EOpFloatBitsToInt:
        case TOperator::EOpFloatBitsToUint:
        case TOperator::EOpIntBitsToFloat:
        case TOperator::EOpUintBitsToFloat:
        {
#define RETURN_AS_TYPE_SCALAR()             \
    do                                      \
        switch (resultType.getBasicType())  \
        {                                   \
            case TBasicType::EbtInt:        \
                return "as_type<int>";      \
            case TBasicType::EbtUInt:       \
                return "as_type<uint32_t>"; \
            case TBasicType::EbtFloat:      \
                return "as_type<float>";    \
            default:                        \
                UNIMPLEMENTED();            \
                return "TOperator_TODO";    \
        }                                   \
    while (false)

#define RETURN_AS_TYPE(post)                     \
    do                                           \
        switch (resultType.getBasicType())       \
        {                                        \
            case TBasicType::EbtInt:             \
                return "as_type<int" post ">";   \
            case TBasicType::EbtUInt:            \
                return "as_type<uint" post ">";  \
            case TBasicType::EbtFloat:           \
                return "as_type<float" post ">"; \
            default:                             \
                UNIMPLEMENTED();                 \
                return "TOperator_TODO";         \
        }                                        \
    while (false)

            if (resultType.isScalar())
            {
                RETURN_AS_TYPE_SCALAR();
            }
            else if (resultType.isVector())
            {
                switch (resultType.getNominalSize())
                {
                    case 2:
                        RETURN_AS_TYPE("2");
                    case 3:
                        RETURN_AS_TYPE("3");
                    case 4:
                        RETURN_AS_TYPE("4");
                    default:
                        UNREACHABLE();
                        return nullptr;
                }
            }
            else
            {
                UNIMPLEMENTED();
                return "TOperator_TODO";
            }

#undef RETURN_AS_TYPE
#undef RETURN_AS_TYPE_SCALAR
        }

        case TOperator::EOpPackUnorm2x16:
            return "metal::pack_float_to_unorm2x16";
        case TOperator::EOpPackSnorm2x16:
            return "metal::pack_float_to_snorm2x16";

        case TOperator::EOpPackUnorm4x8:
            return "metal::pack_float_to_unorm4x8";
        case TOperator::EOpPackSnorm4x8:
            return "metal::pack_float_to_snorm4x8";

        case TOperator::EOpUnpackUnorm2x16:
            return "metal::unpack_unorm2x16_to_float";
        case TOperator::EOpUnpackSnorm2x16:
            return "metal::unpack_snorm2x16_to_float";

        case TOperator::EOpUnpackUnorm4x8:
            return "metal::unpack_unorm4x8_to_float";
        case TOperator::EOpUnpackSnorm4x8:
            return "metal::unpack_snorm4x8_to_float";

        case TOperator::EOpPackHalf2x16:
            return "ANGLE_pack_half_2x16";
        case TOperator::EOpUnpackHalf2x16:
            return "ANGLE_unpack_half_2x16";

        case TOperator::EOpNumSamples:
            return "metal::get_num_samples";
        case TOperator::EOpSamplePosition:
            return "metal::get_sample_position";

        case TOperator::EOpBitfieldExtract:
        case TOperator::EOpBitfieldInsert:
        case TOperator::EOpBitfieldReverse:
        case TOperator::EOpBitCount:
        case TOperator::EOpFindLSB:
        case TOperator::EOpFindMSB:
        case TOperator::EOpUaddCarry:
        case TOperator::EOpUsubBorrow:
        case TOperator::EOpUmulExtended:
        case TOperator::EOpImulExtended:
        case TOperator::EOpBarrier:
        case TOperator::EOpMemoryBarrier:
        case TOperator::EOpMemoryBarrierAtomicCounter:
        case TOperator::EOpMemoryBarrierBuffer:
        case TOperator::EOpMemoryBarrierShared:
        case TOperator::EOpGroupMemoryBarrier:
        case TOperator::EOpAtomicAdd:
        case TOperator::EOpAtomicMin:
        case TOperator::EOpAtomicMax:
        case TOperator::EOpAtomicAnd:
        case TOperator::EOpAtomicOr:
        case TOperator::EOpAtomicXor:
        case TOperator::EOpAtomicExchange:
        case TOperator::EOpAtomicCompSwap:
        case TOperator::EOpEmitVertex:
        case TOperator::EOpEndPrimitive:
        case TOperator::EOpArrayLength:
            UNIMPLEMENTED();
            return "TOperator_TODO";

        case TOperator::EOpNull:
        case TOperator::EOpConstruct:
        case TOperator::EOpCallFunctionInAST:
        case TOperator::EOpCallInternalRawFunction:
        case TOperator::EOpIndexDirect:
        case TOperator::EOpIndexIndirect:
        case TOperator::EOpIndexDirectStruct:
        case TOperator::EOpIndexDirectInterfaceBlock:
            UNREACHABLE();
            return nullptr;
        default:
            // Any other built-in function.
            return nullptr;
    }
}

static bool IsSymbolicOperator(TOperator op,
                               const TType &resultType,
                               const TType *argType0,
                               const TType *argType1)
{
    const char *operatorString = GetOperatorString(op, resultType, argType0, argType1, nullptr);
    if (operatorString == nullptr)
    {
        return false;
    }
    return !std::isalnum(operatorString[0]);
}

static TIntermBinary *AsSpecificBinaryNode(TIntermNode &node, TOperator op)
{
    TIntermBinary *binaryNode = node.getAsBinaryNode();
    if (binaryNode)
    {
        return binaryNode->getOp() == op ? binaryNode : nullptr;
    }
    return nullptr;
}

static bool Parenthesize(TIntermNode &node)
{
    if (node.getAsSymbolNode())
    {
        return false;
    }
    if (node.getAsConstantUnion())
    {
        return false;
    }
    if (node.getAsAggregate())
    {
        return false;
    }
    if (node.getAsSwizzleNode())
    {
        return false;
    }

    if (TIntermUnary *unaryNode = node.getAsUnaryNode())
    {
        // TODO: Use a precedence and associativity rules instead of this ad-hoc impl.
        const TType &resultType = unaryNode->getType();
        const TType &argType    = unaryNode->getOperand()->getType();
        return IsSymbolicOperator(unaryNode->getOp(), resultType, &argType, nullptr);
    }

    if (TIntermBinary *binaryNode = node.getAsBinaryNode())
    {
        // TODO: Use a precedence and associativity rules instead of this ad-hoc impl.
        const TOperator op = binaryNode->getOp();
        switch (op)
        {
            case TOperator::EOpIndexDirectStruct:
            case TOperator::EOpIndexDirectInterfaceBlock:
            case TOperator::EOpIndexDirect:
            case TOperator::EOpIndexIndirect:
                return Parenthesize(*binaryNode->getLeft());

            case TOperator::EOpAssign:
            case TOperator::EOpInitialize:
                return AsSpecificBinaryNode(*binaryNode->getRight(), TOperator::EOpComma);

            default:
            {
                const TType &resultType = binaryNode->getType();
                const TType &leftType   = binaryNode->getLeft()->getType();
                const TType &rightType  = binaryNode->getRight()->getType();
                return IsSymbolicOperator(binaryNode->getOp(), resultType, &leftType, &rightType);
            }
        }
    }

    return true;
}

void GenMetalTraverser::groupedTraverse(TIntermNode &node)
{
    const bool emitParens = Parenthesize(node);

    if (emitParens)
    {
        mOut << "(";
    }

    node.traverse(this);

    if (emitParens)
    {
        mOut << ")";
    }
}

void GenMetalTraverser::emitPostQualifier(const EmitVariableDeclarationConfig &evdConfig,
                                          const VarDecl &decl,
                                          const TQualifier qualifier)
{
    bool isInvariant = false;
    switch (qualifier)
    {
        case TQualifier::EvqPosition:
            isInvariant = decl.type().isInvariant();
            [[fallthrough]];
        case TQualifier::EvqFragCoord:
            mOut << " [[position]]";
            break;

        case TQualifier::EvqClipDistance:
            mOut << " [[clip_distance]] [" << decl.type().getOutermostArraySize() << "]";
            break;

        case TQualifier::EvqPointSize:
            mOut << " [[point_size]]";
            break;

        case TQualifier::EvqVertexID:
            if (evdConfig.isMainParameter)
            {
                mOut << " [[vertex_id]]";
            }
            break;

        case TQualifier::EvqPointCoord:
            if (evdConfig.isMainParameter)
            {
                mOut << " [[point_coord]]";
            }
            break;

        case TQualifier::EvqFrontFacing:
            if (evdConfig.isMainParameter)
            {
                mOut << " [[front_facing]]";
            }
            break;

        case TQualifier::EvqSampleID:
            if (evdConfig.isMainParameter)
            {
                mOut << " [[sample_id]]";
            }
            break;

        case TQualifier::EvqSampleMaskIn:
            if (evdConfig.isMainParameter)
            {
                mOut << " [[sample_mask]]";
            }
            break;

        default:
            break;
    }

    if (isInvariant)
    {
        mOut << " [[invariant]]";

        TranslatorMetalReflection *reflection = mtl::getTranslatorMetalReflection(&mCompiler);
        reflection->hasInvariance             = true;
    }
}

void GenMetalTraverser::emitLoopBody(TIntermBlock *bodyNode)
{
    if (mInjectAsmStatementIntoLoopBodies)
    {
        emitOpenBrace();

        emitIndentation();
        mOut << "__asm__(\"\");\n";
    }

    bodyNode->traverse(this);

    if (mInjectAsmStatementIntoLoopBodies)
    {
        emitCloseBrace();
    }
}

static void EmitName(Sink &out, const Name &name)
{
#if defined(ANGLE_ENABLE_ASSERTS)
    DebugSink::EscapedSink escapedOut(out.escape());
#else
    TInfoSinkBase &escapedOut = out;
#endif
    name.emit(escapedOut);
}

void GenMetalTraverser::emitNameOf(const TField &object)
{
    EmitName(mOut, Name(object));
}

void GenMetalTraverser::emitNameOf(const TSymbol &object)
{
    auto it = mRenamedSymbols.find(&object);
    if (it == mRenamedSymbols.end())
    {
        EmitName(mOut, Name(object));
    }
    else
    {
        EmitName(mOut, it->second);
    }
}

void GenMetalTraverser::emitNameOf(const VarDecl &object)
{
    if (object.isField())
    {
        emitNameOf(object.field());
    }
    else
    {
        emitNameOf(object.variable());
    }
}

void GenMetalTraverser::emitBareTypeName(const TType &type, const EmitTypeConfig &etConfig)
{
    const TBasicType basicType = type.getBasicType();

    switch (basicType)
    {
        case TBasicType::EbtVoid:
        case TBasicType::EbtBool:
        case TBasicType::EbtFloat:
        case TBasicType::EbtInt:
        {
            mOut << type.getBasicString();
            break;
        }
        case TBasicType::EbtUInt:
        {
            if (type.isScalar())
            {
                mOut << "uint32_t";
            }
            else
            {
                mOut << type.getBasicString();
            }
        }
        break;

        case TBasicType::EbtStruct:
        {
            const TStructure &structure = *type.getStruct();
            emitNameOf(structure);
        }
        break;

        case TBasicType::EbtInterfaceBlock:
        {
            const TInterfaceBlock &interfaceBlock = *type.getInterfaceBlock();
            emitNameOf(interfaceBlock);
        }
        break;

        default:
        {
            if (IsSampler(basicType))
            {
                if (etConfig.evdConfig && etConfig.evdConfig->isMainParameter)
                {
                    EmitName(mOut, GetTextureTypeName(basicType));
                }
                else
                {
                    const TStructure &env = mSymbolEnv.getTextureEnv(basicType);
                    emitNameOf(env);
                }
            }
            else if (IsImage(basicType))
            {
                mOut << "metal::texture2d<";
                switch (type.getBasicType())
                {
                    case EbtImage2D:
                        mOut << "float";
                        break;
                    case EbtIImage2D:
                        mOut << "int";
                        break;
                    case EbtUImage2D:
                        mOut << "uint";
                        break;
                    default:
                        UNIMPLEMENTED();
                        break;
                }
                if (type.getMemoryQualifier().readonly || type.getMemoryQualifier().writeonly)
                {
                    UNIMPLEMENTED();
                }
                mOut << ", metal::access::read_write>";
            }
            else
            {
                UNIMPLEMENTED();
            }
        }
    }
}

void GenMetalTraverser::emitType(const TType &type, const EmitTypeConfig &etConfig)
{
    const bool isUBO = etConfig.evdConfig ? etConfig.evdConfig->isUBO : false;
    if (etConfig.evdConfig)
    {
        const auto &evdConfig = *etConfig.evdConfig;
        if (isUBO)
        {
            if (type.isArray())
            {
                mOut << "metal::array<";
            }
        }
        if (evdConfig.isPointer)
        {
            mOut << toString(*evdConfig.isPointer);
            mOut << " ";
        }
        else if (evdConfig.isReference)
        {
            mOut << toString(*evdConfig.isReference);
            mOut << " ";
        }
    }

    if (!isUBO)
    {
        if (type.isArray())
        {
            mOut << "metal::array<";
        }
    }

    if (type.isInterpolant())
    {
        mOut << "metal::interpolant<";
    }

    if (type.isVector() || type.isMatrix())
    {
        mOut << "metal::";
    }

    if (etConfig.evdConfig && etConfig.evdConfig->isPacked)
    {
        mOut << "packed_";
    }

    emitBareTypeName(type, etConfig);

    if (type.isVector())
    {
        mOut << static_cast<uint32_t>(type.getNominalSize());
    }
    else if (type.isMatrix())
    {
        mOut << static_cast<uint32_t>(type.getCols()) << "x"
             << static_cast<uint32_t>(type.getRows());
    }

    if (type.isInterpolant())
    {
        mOut << ", metal::interpolation::";
        switch (type.getQualifier())
        {
            case EvqNoPerspectiveIn:
            case EvqNoPerspectiveCentroidIn:
            case EvqNoPerspectiveSampleIn:
                mOut << "no_";
                break;
            default:
                break;
        }
        mOut << "perspective>";
    }

    if (!isUBO)
    {
        if (type.isArray())
        {
            for (auto size : type.getArraySizes())
            {
                mOut << ", " << size;
            }
            mOut << ">";
        }
    }

    if (etConfig.evdConfig)
    {
        const auto &evdConfig = *etConfig.evdConfig;
        if (evdConfig.isPointer)
        {
            mOut << " *";
        }
        else if (evdConfig.isReference)
        {
            mOut << " &";
        }
        if (isUBO)
        {
            if (type.isArray())
            {
                for (auto size : type.getArraySizes())
                {
                    mOut << ", " << size;
                }
                mOut << ">";
            }
        }
    }
}

void GenMetalTraverser::emitFieldDeclaration(const TField &field,
                                             const TStructure &parent,
                                             FieldAnnotationIndices &annotationIndices)
{
    const TType &type      = *field.type();
    const TBasicType basic = type.getBasicType();

    EmitVariableDeclarationConfig evdConfig;
    evdConfig.emitPostQualifier      = true;
    evdConfig.disableStructSpecifier = true;
    evdConfig.isPacked               = mSymbolEnv.isPacked(field);
    evdConfig.isUBO                  = mSymbolEnv.isUBO(field);
    evdConfig.isPointer              = mSymbolEnv.isPointer(field);
    evdConfig.isReference            = mSymbolEnv.isReference(field);
    emitVariableDeclaration(VarDecl(field), evdConfig);

    const TQualifier qual = type.getQualifier();
    switch (qual)
    {
        case TQualifier::EvqFlatIn:
            if (mPipelineStructs.fragmentIn.external == &parent)
            {
                mOut << " [[flat]]";
                TranslatorMetalReflection *reflection =
                    mtl::getTranslatorMetalReflection(&mCompiler);
                reflection->hasFlatInput = true;
            }
            break;

        case TQualifier::EvqNoPerspectiveIn:
            if (mPipelineStructs.fragmentIn.external == &parent && !type.isInterpolant())
            {
                mOut << " [[center_no_perspective]]";
            }
            break;

        case TQualifier::EvqCentroidIn:
            if (mPipelineStructs.fragmentIn.external == &parent && !type.isInterpolant())
            {
                mOut << " [[centroid_perspective]]";
            }
            break;

        case TQualifier::EvqSampleIn:
            if (mPipelineStructs.fragmentIn.external == &parent && !type.isInterpolant())
            {
                mOut << " [[sample_perspective]]";
            }
            break;

        case TQualifier::EvqNoPerspectiveCentroidIn:
            if (mPipelineStructs.fragmentIn.external == &parent && !type.isInterpolant())
            {
                mOut << " [[centroid_no_perspective]]";
            }
            break;

        case TQualifier::EvqNoPerspectiveSampleIn:
            if (mPipelineStructs.fragmentIn.external == &parent && !type.isInterpolant())
            {
                mOut << " [[sample_no_perspective]]";
            }
            break;

        case TQualifier::EvqFragColor:
            mOut << " [[color(0)]]";
            break;

        case TQualifier::EvqSecondaryFragColorEXT:
        case TQualifier::EvqSecondaryFragDataEXT:
            mOut << " [[color(0), index(1)]]";
            break;

        case TQualifier::EvqFragmentOut:
        case TQualifier::EvqFragmentInOut:
        case TQualifier::EvqFragData:
            if (mPipelineStructs.fragmentOut.external == &parent ||
                mPipelineStructs.fragmentOut.externalExtra == &parent)
            {
                if ((type.isVector() &&
                     (basic == TBasicType::EbtInt || basic == TBasicType::EbtUInt ||
                      basic == TBasicType::EbtFloat)) ||
                    qual == EvqFragData)
                {
                    // The OpenGL ES 3.0 spec says locations must be specified
                    // unless there is only a single output, in which case the
                    // location is 0. So, when we get to this point the shader
                    // will have been rejected if locations are not specified
                    // and there is more than one output.
                    const TLayoutQualifier &layoutQualifier = type.getLayoutQualifier();
                    if (layoutQualifier.locationsSpecified)
                    {
                        mOut << " [[color(" << layoutQualifier.location << ")";
                        ASSERT(layoutQualifier.index >= -1 && layoutQualifier.index <= 1);
                        if (layoutQualifier.index == 1)
                        {
                            mOut << ", index(1)";
                        }
                    }
                    else if (qual == EvqFragData)
                    {
                        mOut << " [[color(" << annotationIndices.color++ << ")";
                    }
                    else
                    {
                        // Either the only output or EXT_blend_func_extended is used;
                        // actual assignment will happen in UpdateFragmentShaderOutputs.
                        mOut << " [[" << sh::kUnassignedFragmentOutputString;
                    }
                    if (mRasterOrderGroupsSupported && qual == TQualifier::EvqFragmentInOut)
                    {
                        // Put fragment inouts in their own raster order group for better
                        // parallelism.
                        // NOTE: this is not required for the reads to be ordered and coherent.
                        // TODO(anglebug.com/40096838): Consider making raster order groups a PLS
                        // layout qualifier?
                        mOut << ", raster_order_group(0)";
                    }
                    mOut << "]]";
                }
            }
            break;

        case TQualifier::EvqFragDepth:
            mOut << " [[depth(";
            switch (type.getLayoutQualifier().depth)
            {
                case EdGreater:
                    mOut << "greater";
                    break;
                case EdLess:
                    mOut << "less";
                    break;
                default:
                    mOut << "any";
                    break;
            }
            mOut << "), function_constant(" << sh::mtl::kDepthWriteEnabledConstName << ")]]";
            break;

        case TQualifier::EvqSampleMask:
            if (field.symbolType() == SymbolType::AngleInternal)
            {
                mOut << " [[sample_mask, function_constant("
                     << sh::mtl::kSampleMaskWriteEnabledConstName << ")]]";
            }
            break;

        default:
            break;
    }

    if (IsImage(type.getBasicType()))
    {
        if (type.getArraySizeProduct() != 1)
        {
            UNIMPLEMENTED();
        }
        mOut << " [[";
        if (type.getLayoutQualifier().rasterOrdered)
        {
            mOut << "raster_order_group(0), ";
        }
        mOut << "texture(" << mMainTextureIndex << ")]]";
        TranslatorMetalReflection *reflection = mtl::getTranslatorMetalReflection(&mCompiler);
        reflection->addRWTextureBinding(type.getLayoutQualifier().binding,
                                        static_cast<int>(mMainTextureIndex));
        ++mMainTextureIndex;
    }
}

static std::map<Name, size_t> BuildExternalAttributeIndexMap(
    const TCompiler &compiler,
    const PipelineScoped<TStructure> &structure)
{
    ASSERT(structure.isTotallyFull());

    const auto &shaderVars     = compiler.getAttributes();
    const size_t shaderVarSize = shaderVars.size();
    size_t shaderVarIndex      = 0;

    const auto &externalFields = structure.external->fields();
    const size_t externalSize  = externalFields.size();
    size_t externalIndex       = 0;

    const auto &internalFields = structure.internal->fields();
    const size_t internalSize  = internalFields.size();
    size_t internalIndex       = 0;

    // Internal fields are never split. External fields are sometimes split.
    ASSERT(externalSize >= internalSize);

    // Structures do not contain any inactive fields.
    ASSERT(shaderVarSize >= internalSize);

    std::map<Name, size_t> externalNameToAttributeIndex;
    size_t attributeIndex = 0;

    while (internalIndex < internalSize)
    {
        const TField &internalField = *internalFields[internalIndex];
        const Name internalName     = Name(internalField);
        const TType &internalType   = *internalField.type();
        while (internalName.rawName() != shaderVars[shaderVarIndex].name &&
               internalName.rawName() != shaderVars[shaderVarIndex].mappedName)
        {
            // This case represents an inactive field.

            ++shaderVarIndex;
            ASSERT(shaderVarIndex < shaderVarSize);

            ++attributeIndex;  // TODO: Might need to increment more if shader var type is a matrix.
        }

        const size_t cols =
            (internalType.isMatrix() && !externalFields[externalIndex]->type()->isMatrix())
                ? internalType.getCols()
                : 1;

        for (size_t c = 0; c < cols; ++c)
        {
            const TField &externalField = *externalFields[externalIndex];
            const Name externalName     = Name(externalField);

            externalNameToAttributeIndex[externalName] = attributeIndex;

            ++externalIndex;
            ++attributeIndex;
        }

        ++shaderVarIndex;
        ++internalIndex;
    }

    ASSERT(shaderVarIndex <= shaderVarSize);
    ASSERT(externalIndex <= externalSize);  // less than if padding was introduced
    ASSERT(internalIndex == internalSize);

    return externalNameToAttributeIndex;
}

void GenMetalTraverser::emitAttributeDeclaration(const TField &field,
                                                 FieldAnnotationIndices &annotationIndices)
{
    EmitVariableDeclarationConfig evdConfig;
    evdConfig.disableStructSpecifier = true;
    emitVariableDeclaration(VarDecl(field), evdConfig);
    mOut << sh::kUnassignedAttributeString;
}

void GenMetalTraverser::emitUniformBufferDeclaration(const TField &field,
                                                     FieldAnnotationIndices &annotationIndices)
{
    EmitVariableDeclarationConfig evdConfig;
    evdConfig.disableStructSpecifier = true;
    evdConfig.isUBO                  = mSymbolEnv.isUBO(field);
    evdConfig.isPointer              = mSymbolEnv.isPointer(field);
    emitVariableDeclaration(VarDecl(field), evdConfig);
    mOut << "[[id(" << annotationIndices.attribute << ")]]";

    const TType &type   = *field.type();
    const int arraySize = type.isArray() ? type.getArraySizeProduct() : 1;

    TranslatorMetalReflection *reflection = mtl::getTranslatorMetalReflection(&mCompiler);
    ASSERT(type.getBasicType() == TBasicType::EbtStruct);
    const TStructure *structure    = type.getStruct();
    const std::string originalName = reflection->getOriginalName(structure->uniqueId().get());
    reflection->addUniformBufferBinding(
        originalName,
        {.bindIndex = annotationIndices.attribute, .arraySize = static_cast<size_t>(arraySize)});

    annotationIndices.attribute += arraySize;
}

void GenMetalTraverser::emitStructDeclaration(const TType &type)
{
    ASSERT(type.getBasicType() == TBasicType::EbtStruct);
    ASSERT(type.isStructSpecifier());

    mOut << "struct ";
    emitBareTypeName(type, {});

    mOut << "\n";
    emitOpenBrace();

    const TStructure &structure = *type.getStruct();
    std::map<Name, size_t> fieldToAttributeIndex;
    const bool hasAttributeIndices      = mPipelineStructs.vertexIn.external == &structure;
    const bool hasUniformBufferIndicies = mPipelineStructs.uniformBuffers.external == &structure;
    const bool reclaimUnusedAttributeIndices = mCompiler.getShaderVersion() < 300;

    if (hasAttributeIndices)
    {
        // When attribute aliasing is supported, external attribute struct is filled post-link.
        if (mCompiler.supportsAttributeAliasing())
        {
            mtl::getTranslatorMetalReflection(&mCompiler)->hasAttributeAliasing = true;
            mOut << "@@Attrib-Bindings@@\n";
            emitCloseBrace();
            return;
        }

        fieldToAttributeIndex =
            BuildExternalAttributeIndexMap(mCompiler, mPipelineStructs.vertexIn);
    }

    FieldAnnotationIndices annotationIndices;

    for (const TField *field : structure.fields())
    {
        emitIndentation();
        if (hasAttributeIndices)
        {
            const auto it = fieldToAttributeIndex.find(Name(*field));
            if (it == fieldToAttributeIndex.end())
            {
                ASSERT(field->symbolType() == SymbolType::AngleInternal);
                ASSERT(field->name().beginsWith("_"));
                ASSERT(angle::EndsWith(field->name().data(), "_pad"));
                emitFieldDeclaration(*field, structure, annotationIndices);
            }
            else
            {
                ASSERT(field->symbolType() != SymbolType::AngleInternal ||
                       !field->name().beginsWith("_") ||
                       !angle::EndsWith(field->name().data(), "_pad"));
                if (!reclaimUnusedAttributeIndices)
                {
                    annotationIndices.attribute = it->second;
                }
                emitAttributeDeclaration(*field, annotationIndices);
            }
        }
        else if (hasUniformBufferIndicies)
        {
            emitUniformBufferDeclaration(*field, annotationIndices);
        }
        else
        {
            emitFieldDeclaration(*field, structure, annotationIndices);
        }
        mOut << ";\n";
    }

    emitCloseBrace();
}

void GenMetalTraverser::emitOrdinaryVariableDeclaration(
    const VarDecl &decl,
    const EmitVariableDeclarationConfig &evdConfig)
{
    EmitTypeConfig etConfig;
    etConfig.evdConfig = &evdConfig;

    const TType &type = decl.type();
    if (type.getQualifier() == TQualifier::EvqClipDistance)
    {
        // Clip distance output uses float[n] type instead of metal::array.
        // The element count is emitted after the post qualifier.
        ASSERT(type.getBasicType() == TBasicType::EbtFloat);
        mOut << "float";
    }
    else if (type.getQualifier() == TQualifier::EvqSampleID && evdConfig.isMainParameter)
    {
        // Metal's [[sample_id]] must be unsigned
        ASSERT(type.getBasicType() == TBasicType::EbtInt);
        mOut << "uint32_t";
    }
    else
    {
        emitType(type, etConfig);
    }
    if (decl.symbolType() != SymbolType::Empty)
    {
        mOut << " ";
        emitNameOf(decl);
    }
}

void GenMetalTraverser::emitVariableDeclaration(const VarDecl &decl,
                                                const EmitVariableDeclarationConfig &evdConfig)
{
    const SymbolType symbolType = decl.symbolType();
    const TType &type           = decl.type();
    const TBasicType basicType  = type.getBasicType();

    switch (basicType)
    {
        case TBasicType::EbtStruct:
        {
            if (type.isStructSpecifier() && !evdConfig.disableStructSpecifier)
            {
                // It's invalid to declare a struct inside a function argument. When emitting a
                // function parameter, the callsite should set evdConfig.disableStructSpecifier.
                ASSERT(!evdConfig.isParameter);
                emitStructDeclaration(type);
                if (symbolType != SymbolType::Empty)
                {
                    mOut << " ";
                    emitNameOf(decl);
                }
            }
            else
            {
                emitOrdinaryVariableDeclaration(decl, evdConfig);
            }
        }
        break;

        default:
        {
            ASSERT(symbolType != SymbolType::Empty || evdConfig.isParameter);
            emitOrdinaryVariableDeclaration(decl, evdConfig);
        }
    }

    if (evdConfig.emitPostQualifier)
    {
        emitPostQualifier(evdConfig, decl, type.getQualifier());
    }
}

void GenMetalTraverser::visitSymbol(TIntermSymbol *symbolNode)
{
    const TVariable &var = symbolNode->variable();
    const TType &type    = var.getType();
    ASSERT(var.symbolType() != SymbolType::Empty);

    if (type.getBasicType() == TBasicType::EbtVoid)
    {
        mOut << "/*";
        emitNameOf(var);
        mOut << "*/";
    }
    else
    {
        emitNameOf(var);
    }
}

void GenMetalTraverser::emitSingleConstant(const TConstantUnion *const constUnion)
{
    switch (constUnion->getType())
    {
        case TBasicType::EbtBool:
        {
            mOut << (constUnion->getBConst() ? "true" : "false");
        }
        break;

        case TBasicType::EbtFloat:
        {
            float value = constUnion->getFConst();
            if (std::isnan(value))
            {
                mOut << "NAN";
            }
            else if (std::isinf(value))
            {
                if (value < 0)
                {
                    mOut << "-";
                }
                mOut << "INFINITY";
            }
            else
            {
                mOut << value << "f";
            }
        }
        break;

        case TBasicType::EbtInt:
        {
            mOut << constUnion->getIConst();
        }
        break;

        case TBasicType::EbtUInt:
        {
            mOut << constUnion->getUConst() << "u";
        }
        break;

        default:
        {
            UNIMPLEMENTED();
        }
    }
}

const TConstantUnion *GenMetalTraverser::emitConstantUnionArray(
    const TConstantUnion *const constUnion,
    const size_t size)
{
    const TConstantUnion *constUnionIterated = constUnion;
    for (size_t i = 0; i < size; i++, constUnionIterated++)
    {
        emitSingleConstant(constUnionIterated);

        if (i != size - 1)
        {
            mOut << ", ";
        }
    }
    return constUnionIterated;
}

const TConstantUnion *GenMetalTraverser::emitConstantUnion(const TType &type,
                                                           const TConstantUnion *constUnionBegin)
{
    const TConstantUnion *constUnionCurr = constUnionBegin;
    const TStructure *structure          = type.getStruct();
    if (structure)
    {
        EmitTypeConfig config = EmitTypeConfig{nullptr};
        emitType(type, config);
        mOut << "{";
        const TFieldList &fields = structure->fields();
        for (size_t i = 0; i < fields.size(); ++i)
        {
            const TType *fieldType = fields[i]->type();
            constUnionCurr         = emitConstantUnion(*fieldType, constUnionCurr);
            if (i != fields.size() - 1)
            {
                mOut << ", ";
            }
        }
        mOut << "}";
    }
    else
    {
        size_t size    = type.getObjectSize();
        bool writeType = size > 1;
        if (writeType)
        {
            EmitTypeConfig config = EmitTypeConfig{nullptr};
            emitType(type, config);
            mOut << "(";
        }
        constUnionCurr = emitConstantUnionArray(constUnionCurr, size);
        if (writeType)
        {
            mOut << ")";
        }
    }
    return constUnionCurr;
}

void GenMetalTraverser::visitConstantUnion(TIntermConstantUnion *constValueNode)
{
    emitConstantUnion(constValueNode->getType(), constValueNode->getConstantValue());
}

bool GenMetalTraverser::visitSwizzle(Visit, TIntermSwizzle *swizzleNode)
{
    groupedTraverse(*swizzleNode->getOperand());
    mOut << ".";

    {
#if defined(ANGLE_ENABLE_ASSERTS)
        DebugSink::EscapedSink escapedOut(mOut.escape());
        TInfoSinkBase &out = escapedOut.get();
#else
        TInfoSinkBase &out = mOut;
#endif
        swizzleNode->writeOffsetsAsXYZW(&out);
    }

    return false;
}

const TField &GenMetalTraverser::getDirectField(const TFieldListCollection &fieldListCollection,
                                                const TConstantUnion &index)
{
    ASSERT(index.getType() == TBasicType::EbtInt);

    const TFieldList &fieldList = fieldListCollection.fields();
    const int indexVal          = index.getIConst();
    const TField &field         = *fieldList[indexVal];

    return field;
}

const TField &GenMetalTraverser::getDirectField(const TIntermTyped &fieldsNode,
                                                TIntermTyped &indexNode)
{
    const TType &fieldsType = fieldsNode.getType();

    const TFieldListCollection *fieldListCollection = fieldsType.getStruct();
    if (fieldListCollection == nullptr)
    {
        fieldListCollection = fieldsType.getInterfaceBlock();
    }
    ASSERT(fieldListCollection);

    const TIntermConstantUnion *indexNode_ = indexNode.getAsConstantUnion();
    ASSERT(indexNode_);
    const TConstantUnion &index = *indexNode_->getConstantValue();

    return getDirectField(*fieldListCollection, index);
}

bool GenMetalTraverser::visitBinary(Visit, TIntermBinary *binaryNode)
{
    const TOperator op      = binaryNode->getOp();
    TIntermTyped &leftNode  = *binaryNode->getLeft();
    TIntermTyped &rightNode = *binaryNode->getRight();

    switch (op)
    {
        case TOperator::EOpIndexDirectStruct:
        case TOperator::EOpIndexDirectInterfaceBlock:
        {
            const TField &field = getDirectField(leftNode, rightNode);
            if (mSymbolEnv.isPointer(field) && mSymbolEnv.isUBO(field))
            {
                emitOpeningPointerParen();
            }
            groupedTraverse(leftNode);
            if (!mSymbolEnv.isPointer(field))
            {
                emitClosingPointerParen();
            }
            mOut << ".";
            emitNameOf(field);
        }
        break;

        case TOperator::EOpIndexDirect:
        case TOperator::EOpIndexIndirect:
        {
            TType leftType = leftNode.getType();
            groupedTraverse(leftNode);
            mOut << "[";
            const TConstantUnion *constIndex = rightNode.getConstantValue();
            // TODO(anglebug.com/42266914): Convert type and bound checks to
            // assertions after AST validation is enabled for MSL translation.
            if (!leftType.isUnsizedArray() && constIndex != nullptr &&
                constIndex->getType() == EbtInt && constIndex->getIConst() >= 0 &&
                constIndex->getIConst() < static_cast<int>(leftType.isArray()
                                                               ? leftType.getOutermostArraySize()
                                                               : leftType.getNominalSize()))
            {
                emitSingleConstant(constIndex);
            }
            else
            {
                mOut << "ANGLE_int_clamp(";
                groupedTraverse(rightNode);
                mOut << ", 0, ";
                if (leftType.isUnsizedArray())
                {
                    groupedTraverse(leftNode);
                    mOut << ".size()";
                }
                else
                {
                    uint32_t maxSize;
                    if (leftType.isArray())
                    {
                        maxSize = leftType.getOutermostArraySize() - 1;
                    }
                    else
                    {
                        maxSize = leftType.getNominalSize() - 1;
                    }
                    mOut << maxSize;
                }
                mOut << ")";
            }
            mOut << "]";
        }
        break;

        default:
        {
            const TType &resultType = binaryNode->getType();
            const TType &leftType   = leftNode.getType();
            const TType &rightType  = rightNode.getType();

            if (IsSymbolicOperator(op, resultType, &leftType, &rightType))
            {
                groupedTraverse(leftNode);
                if (op != TOperator::EOpComma)
                {
                    mOut << " ";
                }
                else
                {
                    emitClosingPointerParen();
                }
                mOut << GetOperatorString(op, resultType, &leftType, &rightType, nullptr) << " ";
                groupedTraverse(rightNode);
            }
            else
            {
                emitClosingPointerParen();
                mOut << GetOperatorString(op, resultType, &leftType, &rightType, nullptr) << "(";
                leftNode.traverse(this);
                mOut << ", ";
                rightNode.traverse(this);
                mOut << ")";
            }
        }
    }

    return false;
}

static bool IsPostfix(TOperator op)
{
    switch (op)
    {
        case TOperator::EOpPostIncrement:
        case TOperator::EOpPostDecrement:
            return true;

        default:
            return false;
    }
}

bool GenMetalTraverser::visitUnary(Visit, TIntermUnary *unaryNode)
{
    const TOperator op      = unaryNode->getOp();
    const TType &resultType = unaryNode->getType();

    TIntermTyped &arg    = *unaryNode->getOperand();
    const TType &argType = arg.getType();

    if (op == TOperator::EOpIsnan || op == TOperator::EOpIsinf)
    {
        mtl::getTranslatorMetalReflection(&mCompiler)->hasIsnanOrIsinf = true;
    }

    const char *name = GetOperatorString(op, resultType, &argType, nullptr, nullptr);

    if (IsSymbolicOperator(op, resultType, &argType, nullptr))
    {
        const bool postfix = IsPostfix(op);
        if (!postfix)
        {
            mOut << name;
        }
        groupedTraverse(arg);
        if (postfix)
        {
            mOut << name;
        }
    }
    else
    {
        mOut << name << "(";
        arg.traverse(this);
        mOut << ")";
    }

    return false;
}

bool GenMetalTraverser::visitTernary(Visit, TIntermTernary *conditionalNode)
{
    groupedTraverse(*conditionalNode->getCondition());
    mOut << " ? ";
    groupedTraverse(*conditionalNode->getTrueExpression());
    mOut << " : ";
    groupedTraverse(*conditionalNode->getFalseExpression());

    return false;
}

bool GenMetalTraverser::visitIfElse(Visit, TIntermIfElse *ifThenElseNode)
{
    TIntermTyped &condNode = *ifThenElseNode->getCondition();
    TIntermBlock *thenNode = ifThenElseNode->getTrueBlock();
    TIntermBlock *elseNode = ifThenElseNode->getFalseBlock();

    emitIndentation();
    mOut << "if (";
    condNode.traverse(this);
    mOut << ")";

    if (thenNode)
    {
        mOut << "\n";
        thenNode->traverse(this);
    }
    else
    {
        mOut << " {}";
    }

    if (elseNode)
    {
        mOut << "\n";
        emitIndentation();
        mOut << "else\n";
        elseNode->traverse(this);
    }
    else
    {
        // Always emit "else" even when empty block to avoid nested if-stmt issues.
        mOut << " else {}";
    }

    return false;
}

bool GenMetalTraverser::visitSwitch(Visit, TIntermSwitch *switchNode)
{
    emitIndentation();
    mOut << "switch (";
    switchNode->getInit()->traverse(this);
    mOut << ")\n";

    ASSERT(!mParentIsSwitch);
    mParentIsSwitch = true;
    switchNode->getStatementList()->traverse(this);
    mParentIsSwitch = false;

    return false;
}

bool GenMetalTraverser::visitCase(Visit, TIntermCase *caseNode)
{
    emitIndentation();

    if (caseNode->hasCondition())
    {
        TIntermTyped *condExpr = caseNode->getCondition();
        mOut << "case ";
        condExpr->traverse(this);
        mOut << ":";
    }
    else
    {
        mOut << "default:\n";
    }

    return false;
}

void GenMetalTraverser::emitFunctionSignature(const TFunction &func)
{
    const bool isMain = func.isMain();

    emitFunctionReturn(func);

    mOut << " ";
    emitNameOf(func);
    if (isMain)
    {
        mOut << "0";
    }
    mOut << "(";

    bool emitComma          = false;
    const size_t paramCount = func.getParamCount();
    for (size_t i = 0; i < paramCount; ++i)
    {
        if (emitComma)
        {
            mOut << ", ";
        }
        emitComma = true;

        const TVariable &param = *func.getParam(i);
        emitFunctionParameter(func, param);
    }

    if (isTraversingVertexMain)
    {
        mOut << " @@XFB-Bindings@@ ";
    }

    mOut << ")";
}

void GenMetalTraverser::emitFunctionReturn(const TFunction &func)
{
    const bool isMain       = func.isMain();
    bool isVertexMain       = false;
    const TType &returnType = func.getReturnType();
    if (isMain)
    {
        const TStructure *structure = returnType.getStruct();
        if (mPipelineStructs.fragmentOut.matches(*structure))
        {
            if (mCompiler.isEarlyFragmentTestsSpecified())
            {
                mOut << "[[early_fragment_tests]]\n";
            }
            mOut << "fragment ";
        }
        else if (mPipelineStructs.vertexOut.matches(*structure))
        {
            ASSERT(structure != nullptr);
            mOut << "vertex __VERTEX_OUT(";
            isVertexMain = true;
        }
        else
        {
            UNIMPLEMENTED();
        }
    }
    emitType(returnType, EmitTypeConfig());
    if (isVertexMain)
        mOut << ") ";
}

void GenMetalTraverser::emitFunctionParameter(const TFunction &func, const TVariable &param)
{
    const bool isMain = func.isMain();

    const TType &type           = param.getType();
    const TStructure *structure = type.getStruct();

    EmitVariableDeclarationConfig evdConfig;
    evdConfig.isParameter            = true;
    evdConfig.disableStructSpecifier = true;  // It's invalid to declare a struct in a function arg.
    evdConfig.isMainParameter        = isMain;
    evdConfig.emitPostQualifier      = isMain;
    evdConfig.isUBO                  = mSymbolEnv.isUBO(param);
    evdConfig.isPointer              = mSymbolEnv.isPointer(param);
    evdConfig.isReference            = mSymbolEnv.isReference(param);
    emitVariableDeclaration(VarDecl(param), evdConfig);

    if (isMain)
    {
        TranslatorMetalReflection *reflection = mtl::getTranslatorMetalReflection(&mCompiler);
        if (structure)
        {
            if (mPipelineStructs.fragmentIn.matches(*structure) ||
                mPipelineStructs.vertexIn.matches(*structure))
            {
                mOut << " [[stage_in]]";
            }
            else if (mPipelineStructs.angleUniforms.matches(*structure))
            {
                mOut << " [[buffer(" << mDriverUniformsBindingIndex << ")]]";
            }
            else if (mPipelineStructs.uniformBuffers.matches(*structure))
            {
                mOut << " [[buffer(" << mUBOArgumentBufferBindingIndex << ")]]";
                reflection->hasUBOs = true;
            }
            else if (mPipelineStructs.userUniforms.matches(*structure))
            {
                mOut << " [[buffer(" << mMainUniformBufferIndex << ")]]";
                reflection->addUserUniformBufferBinding(param.name().data(),
                                                        mMainUniformBufferIndex);
                mMainUniformBufferIndex += type.getArraySizeProduct();
            }
            else if (structure->name() == "metal::sampler")
            {
                mOut << " [[sampler(" << (mMainSamplerIndex) << ")]]";
                const std::string originalName =
                    reflection->getOriginalName(param.uniqueId().get());
                reflection->addSamplerBinding(originalName, mMainSamplerIndex);
                mMainSamplerIndex += type.getArraySizeProduct();
            }
        }
        else if (IsSampler(type.getBasicType()))
        {
            mOut << " [[texture(" << (mMainTextureIndex) << ")]]";
            const std::string originalName = reflection->getOriginalName(param.uniqueId().get());
            reflection->addTextureBinding(originalName, mMainTextureIndex);
            mMainTextureIndex += type.getArraySizeProduct();
        }
        else if (Name(param) == Pipeline{Pipeline::Type::InstanceId, nullptr}.getStructInstanceName(
                                    Pipeline::Variant::Modified))
        {
            mOut << " [[instance_id]]";
        }
        else if (Name(param) == kBaseInstanceName)
        {
            mOut << " [[base_instance]]";
        }
    }
}

void GenMetalTraverser::visitFunctionPrototype(TIntermFunctionPrototype *funcProtoNode)
{
    const TFunction &func = *funcProtoNode->getFunction();

    emitIndentation();
    emitFunctionSignature(func);
}

bool GenMetalTraverser::visitFunctionDefinition(Visit, TIntermFunctionDefinition *funcDefNode)
{
    const TFunction &func = *funcDefNode->getFunction();
    TIntermBlock &body    = *funcDefNode->getBody();
    if (func.isMain())
    {
        const TType &returnType     = func.getReturnType();
        const TStructure *structure = returnType.getStruct();
        isTraversingVertexMain      = (mPipelineStructs.vertexOut.matches(*structure)) &&
                                 mCompiler.getShaderType() == GL_VERTEX_SHADER;
    }
    emitIndentation();
    emitFunctionSignature(func);
    mOut << "\n";
    body.traverse(this);
    if (isTraversingVertexMain)
    {
        isTraversingVertexMain = false;
    }
    return false;
}

GenMetalTraverser::FuncToName GenMetalTraverser::BuildFuncToName()
{
    FuncToName map;

    auto putAngle = [&](const char *nameStr) {
        const ImmutableString name(nameStr);
        ASSERT(map.find(name) == map.end());
        map[name] = Name(nameStr, SymbolType::AngleInternal);
    };

    putAngle("texelFetch");
    putAngle("texelFetchOffset");
    putAngle("texture");
    putAngle("texture2D");
    putAngle("texture2DGradEXT");
    putAngle("texture2DLod");
    putAngle("texture2DLodEXT");
    putAngle("texture2DProj");
    putAngle("texture2DProjGradEXT");
    putAngle("texture2DProjLod");
    putAngle("texture2DProjLodEXT");
    putAngle("texture3D");
    putAngle("texture3DLod");
    putAngle("texture3DProj");
    putAngle("texture3DProjLod");
    putAngle("textureCube");
    putAngle("textureCubeGradEXT");
    putAngle("textureCubeLod");
    putAngle("textureCubeLodEXT");
    putAngle("textureGrad");
    putAngle("textureGradOffset");
    putAngle("textureLod");
    putAngle("textureLodOffset");
    putAngle("textureOffset");
    putAngle("textureProj");
    putAngle("textureProjGrad");
    putAngle("textureProjGradOffset");
    putAngle("textureProjLod");
    putAngle("textureProjLodOffset");
    putAngle("textureProjOffset");
    putAngle("textureSize");
    putAngle("imageLoad");
    putAngle("imageStore");
    putAngle("memoryBarrierImage");

    return map;
}

bool GenMetalTraverser::visitAggregate(Visit, TIntermAggregate *aggregateNode)
{
    const TIntermSequence &args = *aggregateNode->getSequence();

    auto emitArgList = [&](const char *open, const char *close) {
        mOut << open;

        bool emitComma = false;
        for (TIntermNode *arg : args)
        {
            if (emitComma)
            {
                emitClosingPointerParen();
                mOut << ", ";
            }
            emitComma = true;
            arg->traverse(this);
        }

        mOut << close;
    };

    const TType &retType = aggregateNode->getType();

    if (aggregateNode->isConstructor())
    {
        const bool isStandalone = getParentNode()->getAsBlock();
        if (isStandalone)
        {
            // Prevent constructor from being interpreted as a declaration by wrapping in parens.
            // This can happen if given something like:
            //      int(symbol); // <- This will be treated like `int symbol;`... don't want that.
            // So instead emit:
            //      (int(symbol));
            mOut << "(";
        }

        const EmitTypeConfig etConfig;

        if (retType.isArray())
        {
            emitType(retType, etConfig);
            emitArgList("{", "}");
        }
        else if (retType.getStruct())
        {
            emitType(retType, etConfig);
            emitArgList("{", "}");
        }
        else
        {
            emitType(retType, etConfig);
            emitArgList("(", ")");
        }

        if (isStandalone)
        {
            mOut << ")";
        }

        return false;
    }
    else
    {
        const TOperator op = aggregateNode->getOp();
        switch (op)
        {
            case TOperator::EOpCallFunctionInAST:
            case TOperator::EOpCallInternalRawFunction:
            {
                const TFunction &func = *aggregateNode->getFunction();
                emitNameOf(func);
                //'@' symbol in name specifices a macro substitution marker.
                if (!func.name().contains("@"))
                {
                    emitArgList("(", ")");
                }
                else
                {
                    mTemporarilyDisableSemicolon =
                        true;  // Disable semicolon for macro substitution.
                }
                return false;
            }

            default:
            {
                auto getArgType = [&](size_t index) -> const TType * {
                    if (index < args.size())
                    {
                        TIntermTyped *arg = args[index]->getAsTyped();
                        ASSERT(arg);
                        return &arg->getType();
                    }
                    return nullptr;
                };

                const TType *argType0 = getArgType(0);
                const TType *argType1 = getArgType(1);
                const TType *argType2 = getArgType(2);

                const char *opName = GetOperatorString(op, retType, argType0, argType1, argType2);

                if (IsSymbolicOperator(op, retType, argType0, argType1))
                {
                    switch (args.size())
                    {
                        case 1:
                        {
                            TIntermNode &operandNode = *aggregateNode->getChildNode(0);
                            if (IsPostfix(op))
                            {
                                mOut << opName;
                                groupedTraverse(operandNode);
                            }
                            else
                            {
                                groupedTraverse(operandNode);
                                mOut << opName;
                            }
                            return false;
                        }

                        case 2:
                        {
                            TIntermNode &leftNode  = *aggregateNode->getChildNode(0);
                            TIntermNode &rightNode = *aggregateNode->getChildNode(1);
                            groupedTraverse(leftNode);
                            mOut << " " << opName << " ";
                            groupedTraverse(rightNode);
                            return false;
                        }

                        default:
                            UNREACHABLE();
                            return false;
                    }
                }
                else if (opName == nullptr)
                {
                    const TFunction &func = *aggregateNode->getFunction();
                    auto it               = mFuncToName.find(func.name());
                    ASSERT(it != mFuncToName.end());
                    EmitName(mOut, it->second);
                    emitArgList("(", ")");
                    return false;
                }
                else
                {
                    mOut << opName;
                    emitArgList("(", ")");
                    return false;
                }
            }
        }
    }
}

void GenMetalTraverser::emitOpenBrace()
{
    ASSERT(mIndentLevel >= 0);

    emitIndentation();
    mOut << "{\n";
    ++mIndentLevel;
}

void GenMetalTraverser::emitCloseBrace()
{
    ASSERT(mIndentLevel >= 1);

    --mIndentLevel;
    emitIndentation();
    mOut << "}";
}

static bool RequiresSemicolonTerminator(TIntermNode &node)
{
    if (node.getAsBlock())
    {
        return false;
    }
    if (node.getAsLoopNode())
    {
        return false;
    }
    if (node.getAsSwitchNode())
    {
        return false;
    }
    if (node.getAsIfElseNode())
    {
        return false;
    }
    if (node.getAsFunctionDefinition())
    {
        return false;
    }
    if (node.getAsCaseNode())
    {
        return false;
    }

    return true;
}

static bool NewlinePad(TIntermNode &node)
{
    if (node.getAsFunctionDefinition())
    {
        return true;
    }
    if (TIntermDeclaration *declNode = node.getAsDeclarationNode())
    {
        ASSERT(declNode->getChildCount() == 1);
        TIntermNode &childNode = *declNode->getChildNode(0);
        if (TIntermSymbol *symbolNode = childNode.getAsSymbolNode())
        {
            const TVariable &var = symbolNode->variable();
            return var.getType().isStructSpecifier();
        }
        return false;
    }
    return false;
}

bool GenMetalTraverser::visitBlock(Visit, TIntermBlock *blockNode)
{
    ASSERT(mIndentLevel >= -1);
    const bool isGlobalScope  = mIndentLevel == -1;
    const bool parentIsSwitch = mParentIsSwitch;
    mParentIsSwitch           = false;

    if (isGlobalScope)
    {
        ++mIndentLevel;
    }
    else
    {
        emitOpenBrace();
        if (parentIsSwitch)
        {
            ++mIndentLevel;
        }
    }

    TIntermNode *prevStmtNode = nullptr;

    const size_t stmtCount = blockNode->getChildCount();
    for (size_t i = 0; i < stmtCount; ++i)
    {
        TIntermNode &stmtNode = *blockNode->getChildNode(i);

        if (isGlobalScope && prevStmtNode && (NewlinePad(*prevStmtNode) || NewlinePad(stmtNode)))
        {
            mOut << "\n";
        }
        const bool isCase = stmtNode.getAsCaseNode();
        mIndentLevel -= isCase;
        emitIndentation();
        mIndentLevel += isCase;
        stmtNode.traverse(this);
        if (RequiresSemicolonTerminator(stmtNode) && !mTemporarilyDisableSemicolon)
        {
            mOut << ";";
        }
        mTemporarilyDisableSemicolon = false;
        mOut << "\n";

        prevStmtNode = &stmtNode;
    }

    if (isGlobalScope)
    {
        ASSERT(mIndentLevel == 0);
        --mIndentLevel;
    }
    else
    {
        if (parentIsSwitch)
        {
            ASSERT(mIndentLevel >= 1);
            --mIndentLevel;
        }
        emitCloseBrace();
        mParentIsSwitch = parentIsSwitch;
    }

    return false;
}

bool GenMetalTraverser::visitGlobalQualifierDeclaration(Visit, TIntermGlobalQualifierDeclaration *)
{
    return false;
}

bool GenMetalTraverser::visitDeclaration(Visit, TIntermDeclaration *declNode)
{
    ASSERT(declNode->getChildCount() == 1);
    TIntermNode &node = *declNode->getChildNode(0);

    EmitVariableDeclarationConfig evdConfig;

    if (TIntermSymbol *symbolNode = node.getAsSymbolNode())
    {
        const TVariable &var = symbolNode->variable();
        emitVariableDeclaration(VarDecl(var), evdConfig);
        if (var.getType().isArray() && var.getType().getQualifier() == EvqTemporary)
        {
            // The translator frontend injects a loop-based init for user arrays when the source
            // shader is using ESSL 1.00. Some Metal drivers may fail to access elements of such
            // arrays at runtime depending on the array size. An empty literal initializer added
            // to the generated MSL bypasses the issue. The frontend may be further optimized to
            // skip the loop-based init when targeting MSL.
            mOut << "{}";
        }
    }
    else if (TIntermBinary *initNode = node.getAsBinaryNode())
    {
        ASSERT(initNode->getOp() == TOperator::EOpInitialize);
        TIntermSymbol *leftSymbolNode = initNode->getLeft()->getAsSymbolNode();
        TIntermTyped *valueNode       = initNode->getRight()->getAsTyped();
        ASSERT(leftSymbolNode && valueNode);

        if (getRootNode() == getParentBlock())
        {
            // DeferGlobalInitializers should have turned non-const global initializers into
            // deferred initializers. Note that variables marked as EvqGlobal can be treated as
            // EvqConst in some ANGLE code but not actually have their qualifier actually changed to
            // EvqConst. Thus just assume all EvqGlobal are actually EvqConst for all code run after
            // DeferGlobalInitializers.
            mOut << "constant ";
        }

        const TVariable &var = leftSymbolNode->variable();
        const Name varName(var);

        if (ExpressionContainsName(varName, *valueNode))
        {
            mRenamedSymbols[&var] = mIdGen.createNewName(varName);
        }

        emitVariableDeclaration(VarDecl(var), evdConfig);
        mOut << " = ";
        groupedTraverse(*valueNode);
    }
    else
    {
        UNREACHABLE();
    }

    return false;
}

bool GenMetalTraverser::visitLoop(Visit, TIntermLoop *loopNode)
{
    const TLoopType loopType = loopNode->getType();

    switch (loopType)
    {
        case TLoopType::ELoopFor:
            return visitForLoop(loopNode);
        case TLoopType::ELoopWhile:
            return visitWhileLoop(loopNode);
        case TLoopType::ELoopDoWhile:
            return visitDoWhileLoop(loopNode);
    }
}

bool GenMetalTraverser::visitForLoop(TIntermLoop *loopNode)
{
    ASSERT(loopNode->getType() == TLoopType::ELoopFor);

    TIntermNode *initNode  = loopNode->getInit();
    TIntermTyped *condNode = loopNode->getCondition();
    TIntermTyped *exprNode = loopNode->getExpression();

    mOut << "for (";

    if (initNode)
    {
        initNode->traverse(this);
    }
    else
    {
        mOut << " ";
    }

    mOut << "; ";

    if (condNode)
    {
        condNode->traverse(this);
    }

    mOut << "; ";

    if (exprNode)
    {
        exprNode->traverse(this);
    }

    mOut << ")\n";

    emitLoopBody(loopNode->getBody());

    return false;
}

bool GenMetalTraverser::visitWhileLoop(TIntermLoop *loopNode)
{
    ASSERT(loopNode->getType() == TLoopType::ELoopWhile);

    TIntermNode *initNode  = loopNode->getInit();
    TIntermTyped *condNode = loopNode->getCondition();
    TIntermTyped *exprNode = loopNode->getExpression();
    ASSERT(condNode);
    ASSERT(!initNode && !exprNode);

    emitIndentation();
    mOut << "while (";
    condNode->traverse(this);
    mOut << ")\n";
    emitLoopBody(loopNode->getBody());

    return false;
}

bool GenMetalTraverser::visitDoWhileLoop(TIntermLoop *loopNode)
{
    ASSERT(loopNode->getType() == TLoopType::ELoopDoWhile);

    TIntermNode *initNode  = loopNode->getInit();
    TIntermTyped *condNode = loopNode->getCondition();
    TIntermTyped *exprNode = loopNode->getExpression();
    ASSERT(condNode);
    ASSERT(!initNode && !exprNode);

    emitIndentation();
    mOut << "do\n";
    emitLoopBody(loopNode->getBody());
    mOut << "\n";
    emitIndentation();
    mOut << "while (";
    condNode->traverse(this);
    mOut << ");";

    return false;
}

bool GenMetalTraverser::visitBranch(Visit, TIntermBranch *branchNode)
{
    const TOperator flowOp = branchNode->getFlowOp();
    TIntermTyped *exprNode = branchNode->getExpression();

    emitIndentation();

    switch (flowOp)
    {
        case TOperator::EOpKill:
        {
            ASSERT(exprNode == nullptr);
            mOut << "metal::discard_fragment()";
        }
        break;

        case TOperator::EOpReturn:
        {
            if (isTraversingVertexMain)
            {
                mOut << "#if TRANSFORM_FEEDBACK_ENABLED\n";
                emitIndentation();
                mOut << "return;\n";
                emitIndentation();
                mOut << "#else\n";
                emitIndentation();
            }
            mOut << "return";
            if (exprNode)
            {
                mOut << " ";
                exprNode->traverse(this);
                mOut << ";";
            }
            if (isTraversingVertexMain)
            {
                mOut << "\n";
                emitIndentation();
                mOut << "#endif\n";
                mTemporarilyDisableSemicolon = true;
            }
        }
        break;

        case TOperator::EOpBreak:
        {
            ASSERT(exprNode == nullptr);
            mOut << "break";
        }
        break;

        case TOperator::EOpContinue:
        {
            ASSERT(exprNode == nullptr);
            mOut << "continue";
        }
        break;

        default:
        {
            UNREACHABLE();
        }
    }

    return false;
}

static size_t emitMetalCallCount = 0;

bool sh::EmitMetal(TCompiler &compiler,
                   TIntermBlock &root,
                   IdGen &idGen,
                   const PipelineStructs &pipelineStructs,
                   SymbolEnv &symbolEnv,
                   const ProgramPreludeConfig &ppc,
                   const ShCompileOptions &compileOptions)
{
    TInfoSinkBase &out = compiler.getInfoSink().obj;

    {
        ++emitMetalCallCount;
        std::string filenameProto = angle::GetEnvironmentVar("GMD_FIXED_EMIT");
        if (!filenameProto.empty())
        {
            if (filenameProto != "/dev/null")
            {
                auto tryOpen = [&](char const *ext) {
                    auto filename = filenameProto;
                    filename += std::to_string(emitMetalCallCount);
                    filename += ".";
                    filename += ext;
                    return fopen(filename.c_str(), "rb");
                };
                FILE *file = tryOpen("metal");
                if (!file)
                {
                    file = tryOpen("cpp");
                }
                ASSERT(file);

                fseek(file, 0, SEEK_END);
                size_t fileSize = ftell(file);
                fseek(file, 0, SEEK_SET);

                std::vector<char> buff;
                buff.resize(fileSize + 1);
                fread(buff.data(), fileSize, 1, file);
                buff.back() = '\0';

                fclose(file);

                out << buff.data();
            }

            return true;
        }
    }

    out << "\n\n";

    if (!EmitProgramPrelude(root, out, ppc))
    {
        return false;
    }

    {
#if defined(ANGLE_ENABLE_ASSERTS)
        DebugSink outWrapper(out, angle::GetBoolEnvironmentVar("GMD_STDOUT"));
        outWrapper.watch(angle::GetEnvironmentVar("GMD_WATCH_STRING"));
#else
        TInfoSinkBase &outWrapper = out;
#endif
        GenMetalTraverser gen(compiler, outWrapper, idGen, pipelineStructs, symbolEnv,
                              compileOptions);
        root.traverse(&gen);
    }

    out << "\n";

    return true;
}
