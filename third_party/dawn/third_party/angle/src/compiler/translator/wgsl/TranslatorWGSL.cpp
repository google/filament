//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/wgsl/TranslatorWGSL.h"

#include <iostream>
#include <variant>

#include "GLSLANG/ShaderLang.h"
#include "common/log_utils.h"
#include "common/span.h"
#include "compiler/translator/BaseTypes.h"
#include "compiler/translator/Common.h"
#include "compiler/translator/Diagnostics.h"
#include "compiler/translator/ImmutableString.h"
#include "compiler/translator/ImmutableStringBuilder.h"
#include "compiler/translator/InfoSink.h"
#include "compiler/translator/IntermNode.h"
#include "compiler/translator/OutputTree.h"
#include "compiler/translator/StaticType.h"
#include "compiler/translator/SymbolUniqueId.h"
#include "compiler/translator/Types.h"
#include "compiler/translator/tree_util/BuiltIn_autogen.h"
#include "compiler/translator/tree_util/FindMain.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"
#include "compiler/translator/tree_util/RunAtTheEndOfShader.h"
#include "compiler/translator/wgsl/OutputUniformBlocks.h"
#include "compiler/translator/wgsl/RewritePipelineVariables.h"
#include "compiler/translator/wgsl/Utils.h"

namespace sh
{
namespace
{

constexpr bool kOutputTreeBeforeTranslation = false;
constexpr bool kOutputTranslatedShader      = false;

struct VarDecl
{
    const SymbolType symbolType = SymbolType::Empty;
    const ImmutableString &symbolName;
    const TType &type;
};

bool IsDefaultUniform(const TType &type)
{
    return type.getQualifier() == EvqUniform && type.getInterfaceBlock() == nullptr &&
           !IsOpaqueType(type.getBasicType());
}

// When emitting a list of statements, this determines whether a semicolon follows the statement.
bool RequiresSemicolonTerminator(TIntermNode &node)
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

// For pretty formatting of the resulting WGSL text.
bool NewlinePad(TIntermNode &node)
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

// A traverser that generates WGSL as it walks the AST.
class OutputWGSLTraverser : public TIntermTraverser
{
  public:
    OutputWGSLTraverser(TInfoSinkBase *sink,
                        RewritePipelineVarOutput *rewritePipelineVarOutput,
                        UniformBlockMetadata *uniformBlockMetadata,
                        WGSLGenerationMetadataForUniforms *arrayElementTypesInUniforms);
    ~OutputWGSLTraverser() override;

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
    struct EmitVariableDeclarationConfig
    {
        EmitTypeConfig typeConfig;
        bool isParameter            = false;
        bool disableStructSpecifier = false;
        bool needsVar               = false;
        bool isGlobalScope          = false;
    };

    void groupedTraverse(TIntermNode &node);
    void emitNameOf(const VarDecl &decl);
    void emitBareTypeName(const TType &type);
    void emitType(const TType &type);
    void emitSingleConstant(const TConstantUnion *const constUnion);
    const TConstantUnion *emitConstantUnionArray(const TConstantUnion *const constUnion,
                                                 const size_t size);
    const TConstantUnion *emitConstantUnion(const TType &type,
                                            const TConstantUnion *constUnionBegin);
    const TField &getDirectField(const TIntermTyped &fieldsNode, TIntermTyped &indexNode);
    void emitIndentation();
    void emitOpenBrace();
    void emitCloseBrace();
    bool emitBlock(angle::Span<TIntermNode *> nodes);
    void emitFunctionSignature(const TFunction &func);
    void emitFunctionReturn(const TFunction &func);
    void emitFunctionParameter(const TFunction &func, const TVariable &param);
    void emitStructDeclaration(const TType &type);
    void emitVariableDeclaration(const VarDecl &decl,
                                 const EmitVariableDeclarationConfig &evdConfig);
    void emitArrayIndex(TIntermTyped &leftNode, TIntermTyped &rightNode);
    void emitStructIndex(TIntermBinary *binaryNode);
    void emitStructIndexNoUnwrapping(TIntermBinary *binaryNode);

    bool emitForLoop(TIntermLoop *);
    bool emitWhileLoop(TIntermLoop *);
    bool emulateDoWhileLoop(TIntermLoop *);

    TInfoSinkBase &mSink;
    const RewritePipelineVarOutput *mRewritePipelineVarOutput;
    const UniformBlockMetadata *mUniformBlockMetadata;
    WGSLGenerationMetadataForUniforms *mWGSLGenerationMetadataForUniforms;

    int mIndentLevel        = -1;
    int mLastIndentationPos = -1;
};

OutputWGSLTraverser::OutputWGSLTraverser(
    TInfoSinkBase *sink,
    RewritePipelineVarOutput *rewritePipelineVarOutput,
    UniformBlockMetadata *uniformBlockMetadata,
    WGSLGenerationMetadataForUniforms *wgslGenerationMetadataForUniforms)
    : TIntermTraverser(true, false, false),
      mSink(*sink),
      mRewritePipelineVarOutput(rewritePipelineVarOutput),
      mUniformBlockMetadata(uniformBlockMetadata),
      mWGSLGenerationMetadataForUniforms(wgslGenerationMetadataForUniforms)
{}

OutputWGSLTraverser::~OutputWGSLTraverser() = default;

void OutputWGSLTraverser::groupedTraverse(TIntermNode &node)
{
    // TODO(anglebug.com/42267100): to make generated code more readable, do not always
    // emit parentheses like WGSL is some Lisp dialect.
    const bool emitParens = true;

    if (emitParens)
    {
        mSink << "(";
    }

    node.traverse(this);

    if (emitParens)
    {
        mSink << ")";
    }
}

void OutputWGSLTraverser::emitNameOf(const VarDecl &decl)
{
    WriteNameOf(mSink, decl.symbolType, decl.symbolName);
}

void OutputWGSLTraverser::emitIndentation()
{
    ASSERT(mIndentLevel >= 0);

    if (mLastIndentationPos == mSink.size())
    {
        return;  // Line is already indented.
    }

    for (int i = 0; i < mIndentLevel; ++i)
    {
        mSink << "  ";
    }

    mLastIndentationPos = mSink.size();
}

void OutputWGSLTraverser::emitOpenBrace()
{
    ASSERT(mIndentLevel >= 0);

    emitIndentation();
    mSink << "{\n";
    ++mIndentLevel;
}

void OutputWGSLTraverser::emitCloseBrace()
{
    ASSERT(mIndentLevel >= 1);

    --mIndentLevel;
    emitIndentation();
    mSink << "}";
}

void OutputWGSLTraverser::visitSymbol(TIntermSymbol *symbolNode)
{

    const TVariable &var = symbolNode->variable();
    const TType &type    = var.getType();
    ASSERT(var.symbolType() != SymbolType::Empty);

    if (type.getBasicType() == TBasicType::EbtVoid)
    {
        UNREACHABLE();
    }
    else
    {
        // Accesses of pipeline variables should be rewritten as struct accesses.
        if (mRewritePipelineVarOutput->IsInputVar(var.uniqueId()))
        {
            mSink << kBuiltinInputStructName << "." << var.name();
        }
        else if (mRewritePipelineVarOutput->IsOutputVar(var.uniqueId()))
        {
            mSink << kBuiltinOutputStructName << "." << var.name();
        }
        // Accesses of basic uniforms need to be converted to struct accesses.
        else if (IsDefaultUniform(type))
        {
            mSink << kDefaultUniformBlockVarName << "." << var.name();
        }
        else
        {
            WriteNameOf(mSink, var);
        }

        if (var.symbolType() == SymbolType::BuiltIn)
        {
            ASSERT(mRewritePipelineVarOutput->IsInputVar(var.uniqueId()) ||
                   mRewritePipelineVarOutput->IsOutputVar(var.uniqueId()) ||
                   var.uniqueId() == BuiltInId::gl_DepthRange);
            // TODO(anglebug.com/42267100): support gl_DepthRange.
            // Match the name of the struct field in `mRewritePipelineVarOutput`.
            mSink << "_";
        }
    }
}

void OutputWGSLTraverser::emitSingleConstant(const TConstantUnion *const constUnion)
{
    switch (constUnion->getType())
    {
        case TBasicType::EbtBool:
        {
            mSink << (constUnion->getBConst() ? "true" : "false");
        }
        break;

        case TBasicType::EbtFloat:
        {
            float value = constUnion->getFConst();
            if (std::isnan(value))
            {
                UNIMPLEMENTED();
                // TODO(anglebug.com/42267100): this is not a valid constant in WGPU.
                // You can't even do something like bitcast<f32>(0xffffffffu).
                // The WGSL compiler still complains. I think this is because
                // WGSL supports implementations compiling with -ffastmath and
                // therefore nans and infinities are assumed to not exist.
                // See also https://github.com/gpuweb/gpuweb/issues/3749.
                mSink << "NAN_INVALID";
            }
            else if (std::isinf(value))
            {
                UNIMPLEMENTED();
                // see above.
                mSink << "INFINITY_INVALID";
            }
            else
            {
                mSink << value << "f";
            }
        }
        break;

        case TBasicType::EbtInt:
        {
            mSink << constUnion->getIConst() << "i";
        }
        break;

        case TBasicType::EbtUInt:
        {
            mSink << constUnion->getUConst() << "u";
        }
        break;

        default:
        {
            UNIMPLEMENTED();
        }
    }
}

const TConstantUnion *OutputWGSLTraverser::emitConstantUnionArray(
    const TConstantUnion *const constUnion,
    const size_t size)
{
    const TConstantUnion *constUnionIterated = constUnion;
    for (size_t i = 0; i < size; i++, constUnionIterated++)
    {
        emitSingleConstant(constUnionIterated);

        if (i != size - 1)
        {
            mSink << ", ";
        }
    }
    return constUnionIterated;
}

const TConstantUnion *OutputWGSLTraverser::emitConstantUnion(const TType &type,
                                                             const TConstantUnion *constUnionBegin)
{
    const TConstantUnion *constUnionCurr = constUnionBegin;
    const TStructure *structure          = type.getStruct();
    if (structure)
    {
        emitType(type);
        // Structs are constructed with parentheses in WGSL.
        mSink << "(";
        // Emit the constructor parameters. Both GLSL and WGSL require there to be the same number
        // of parameters as struct fields.
        const TFieldList &fields = structure->fields();
        for (size_t i = 0; i < fields.size(); ++i)
        {
            const TType *fieldType = fields[i]->type();
            constUnionCurr         = emitConstantUnion(*fieldType, constUnionCurr);
            if (i != fields.size() - 1)
            {
                mSink << ", ";
            }
        }
        mSink << ")";
    }
    else
    {
        size_t size = type.getObjectSize();
        // If the type's size is more than 1, the type needs to be written with parantheses. This
        // applies for vectors, matrices, and arrays.
        bool writeType = size > 1;
        if (writeType)
        {
            emitType(type);
            mSink << "(";
        }
        constUnionCurr = emitConstantUnionArray(constUnionCurr, size);
        if (writeType)
        {
            mSink << ")";
        }
    }
    return constUnionCurr;
}

void OutputWGSLTraverser::visitConstantUnion(TIntermConstantUnion *constValueNode)
{
    emitConstantUnion(constValueNode->getType(), constValueNode->getConstantValue());
}

bool OutputWGSLTraverser::visitSwizzle(Visit, TIntermSwizzle *swizzleNode)
{
    groupedTraverse(*swizzleNode->getOperand());
    mSink << "." << swizzleNode->getOffsetsAsXYZW();

    return false;
}

const char *GetOperatorString(TOperator op,
                              const TType &resultType,
                              const TType *argType0,
                              const TType *argType1,
                              const TType *argType2)
{
    switch (op)
    {
        case TOperator::EOpComma:
            // WGSL does not have a comma operator or any other way to implement "statement list as
            // an expression", so nested expressions will have to be pulled out into statements.
            UNIMPLEMENTED();
            return "TODO_operator";
        case TOperator::EOpAssign:
            return "=";
        case TOperator::EOpInitialize:
            return "=";
        // Compound assignments now exist: https://www.w3.org/TR/WGSL/#compound-assignment-sec
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
            return "<<=";
        case TOperator::EOpBitShiftRightAssign:
            return ">>=";
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
        // TODO(anglebug.com/42267100): Works different from GLSL for negative numbers.
        // https://github.com/gpuweb/gpuweb/discussions/2204#:~:text=not%20WGSL%3B%20etc.-,Inconsistent%20mod/%25%20operator,-At%20first%20glance
        // GLSL does `x - y * floor(x/y)`, WGSL does x - y * trunc(x/y).
        case TOperator::EOpIMod:
        case TOperator::EOpMod:
            return "%";
        case TOperator::EOpBitShiftLeft:
            return "<<";
        case TOperator::EOpBitShiftRight:
            return ">>";
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
        // Component-wise comparisons are done with regular infix operators in WGSL:
        // https://www.w3.org/TR/WGSL/#comparison-expr
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
        // Logical XOR is only applied to boolean expressions so it's the same as "not equals".
        // Neither short-circuits.
        case TOperator::EOpLogicalXor:
            return "!=";
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
        // Component-wise not done with normal prefix unary operator in WGSL:
        // https://www.w3.org/TR/WGSL/#logical-expr
        case TOperator::EOpNotComponentWise:
            return "!";
        case TOperator::EOpBitwiseNot:
            return "~";
        // TODO(anglebug.com/42267100): increment operations cannot be used as expressions in WGSL.
        case TOperator::EOpPostIncrement:
            return "++";
        case TOperator::EOpPostDecrement:
            return "--";
        case TOperator::EOpPreIncrement:
        case TOperator::EOpPreDecrement:
            // TODO(anglebug.com/42267100): pre increments and decrements do not exist in WGSL.
            UNIMPLEMENTED();
            return "TODO_operator";
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

        // TODO(anglebug.com/42267100): structs, matrices, and arrays are not comparable with WGSL's
        // == or !=. Comparing vectors results in a component-wise comparison returning a boolean
        // vector, which is different from GLSL (which use equal(vec, vec) for component-wise
        // comparison)
        case TOperator::EOpEqual:
            if ((argType0->isVector() && argType1->isVector()) ||
                (argType0->getStruct() && argType1->getStruct()) ||
                (argType0->isArray() && argType1->isArray()) ||
                (argType0->isMatrix() && argType1->isMatrix()))

            {
                UNIMPLEMENTED();
                return "TODO_operator";
            }

            return "==";

        case TOperator::EOpNotEqual:
            if ((argType0->isVector() && argType1->isVector()) ||
                (argType0->getStruct() && argType1->getStruct()) ||
                (argType0->isArray() && argType1->isArray()) ||
                (argType0->isMatrix() && argType1->isMatrix()))
            {
                UNIMPLEMENTED();
                return "TODO_operator";
            }
            return "!=";

        case TOperator::EOpKill:
        case TOperator::EOpReturn:
        case TOperator::EOpBreak:
        case TOperator::EOpContinue:
            // These should all be emitted in visitBranch().
            UNREACHABLE();
            return "UNREACHABLE_operator";
        case TOperator::EOpRadians:
            return "radians";
        case TOperator::EOpDegrees:
            return "degrees";
        case TOperator::EOpAtan:
            return argType1 == nullptr ? "atan" : "atan2";
        case TOperator::EOpRefract:
            return argType0->isVector() ? "refract" : "TODO_operator";
        case TOperator::EOpDistance:
            return "distance";
        case TOperator::EOpLength:
            return "length";
        case TOperator::EOpDot:
            return argType0->isVector() ? "dot" : "*";
        case TOperator::EOpNormalize:
            return argType0->isVector() ? "normalize" : "sign";
        case TOperator::EOpFaceforward:
            return argType0->isVector() ? "faceForward" : "TODO_Operator";
        case TOperator::EOpReflect:
            return argType0->isVector() ? "reflect" : "TODO_Operator";
        case TOperator::EOpMatrixCompMult:
            return "TODO_Operator";
        case TOperator::EOpOuterProduct:
            return "TODO_Operator";
        case TOperator::EOpSign:
            return "sign";

        case TOperator::EOpAbs:
            return "abs";
        case TOperator::EOpAll:
            return "all";
        case TOperator::EOpAny:
            return "any";
        case TOperator::EOpSin:
            return "sin";
        case TOperator::EOpCos:
            return "cos";
        case TOperator::EOpTan:
            return "tan";
        case TOperator::EOpAsin:
            return "asin";
        case TOperator::EOpAcos:
            return "acos";
        case TOperator::EOpSinh:
            return "sinh";
        case TOperator::EOpCosh:
            return "cosh";
        case TOperator::EOpTanh:
            return "tanh";
        case TOperator::EOpAsinh:
            return "asinh";
        case TOperator::EOpAcosh:
            return "acosh";
        case TOperator::EOpAtanh:
            return "atanh";
        case TOperator::EOpFma:
            return "fma";
        // TODO(anglebug.com/42267100): Won't accept pow(vec<f32>, f32).
        // https://github.com/gpuweb/gpuweb/discussions/2204#:~:text=Similarly%20pow(vec3%3Cf32%3E%2C%20f32)%20works%20in%20GLSL%20but%20not%20WGSL
        case TOperator::EOpPow:
            return "pow";  // GLSL's pow excludes negative x
        case TOperator::EOpExp:
            return "exp";
        case TOperator::EOpExp2:
            return "exp2";
        case TOperator::EOpLog:
            return "log";
        case TOperator::EOpLog2:
            return "log2";
        case TOperator::EOpSqrt:
            return "sqrt";
        case TOperator::EOpFloor:
            return "floor";
        case TOperator::EOpTrunc:
            return "trunc";
        case TOperator::EOpCeil:
            return "ceil";
        case TOperator::EOpFract:
            return "fract";
        case TOperator::EOpMin:
            return "min";
        case TOperator::EOpMax:
            return "max";
        case TOperator::EOpRound:
            return "round";  // TODO(anglebug.com/42267100): this is wrong and must round away from
                             // zero if there is a tie. This always rounds to the even number.
        case TOperator::EOpRoundEven:
            return "round";
        // TODO(anglebug.com/42267100):
        // https://github.com/gpuweb/gpuweb/discussions/2204#:~:text=clamp(vec2%3Cf32%3E%2C%20f32%2C%20f32)%20works%20in%20GLSL%20but%20not%20WGSL%3B%20etc.
        // Need to expand clamp(vec<f32>, low : f32, high : f32) ->
        // clamp(vec<f32>, vec<f32>(low), vec<f32>(high))
        case TOperator::EOpClamp:
            return "clamp";
        case TOperator::EOpSaturate:
            return "saturate";
        case TOperator::EOpMix:
            if (!argType1->isScalar() && argType2 && argType2->getBasicType() == EbtBool)
            {
                return "TODO_Operator";
            }
            return "mix";
        case TOperator::EOpStep:
            return "step";
        case TOperator::EOpSmoothstep:
            return "smoothstep";
        case TOperator::EOpModf:
            UNIMPLEMENTED();  // TODO(anglebug.com/42267100): in WGSL this returns a struct, GLSL it
                              // uses a return value and an outparam
            return "modf";
        case TOperator::EOpIsnan:
        case TOperator::EOpIsinf:
            UNIMPLEMENTED();  // TODO(anglebug.com/42267100): WGSL does not allow NaNs or infinity.
                              // What to do about shaders that require this?
            // Implementations are allowed to assume overflow, infinities, and NaNs are not present
            // at runtime, however. https://www.w3.org/TR/WGSL/#floating-point-evaluation
            return "TODO_Operator";
        case TOperator::EOpLdexp:
            // TODO(anglebug.com/42267100): won't accept first arg vector, second arg scalar
            return "ldexp";
        case TOperator::EOpFrexp:
            return "frexp";  // TODO(anglebug.com/42267100): returns a struct
        case TOperator::EOpInversesqrt:
            return "inverseSqrt";
        case TOperator::EOpCross:
            return "cross";
            // TODO(anglebug.com/42267100): are these the same? dpdxCoarse() vs dpdxFine()?
        case TOperator::EOpDFdx:
            return "dpdx";
        case TOperator::EOpDFdy:
            return "dpdy";
        case TOperator::EOpFwidth:
            return "fwidth";
        case TOperator::EOpTranspose:
            return "transpose";
        case TOperator::EOpDeterminant:
            return "determinant";

        case TOperator::EOpInverse:
            return "TODO_Operator";  // No builtin invert().
                                     // https://github.com/gpuweb/gpuweb/issues/4115

        // TODO(anglebug.com/42267100): these interpolateAt*() are not builtin
        case TOperator::EOpInterpolateAtCentroid:
            return "TODO_Operator";
        case TOperator::EOpInterpolateAtSample:
            return "TODO_Operator";
        case TOperator::EOpInterpolateAtOffset:
            return "TODO_Operator";
        case TOperator::EOpInterpolateAtCenter:
            return "TODO_Operator";

        case TOperator::EOpFloatBitsToInt:
        case TOperator::EOpFloatBitsToUint:
        case TOperator::EOpIntBitsToFloat:
        case TOperator::EOpUintBitsToFloat:
        {
#define BITCAST_SCALAR()                   \
    do                                     \
        switch (resultType.getBasicType()) \
        {                                  \
            case TBasicType::EbtInt:       \
                return "bitcast<i32>";     \
            case TBasicType::EbtUInt:      \
                return "bitcast<u32>";     \
            case TBasicType::EbtFloat:     \
                return "bitcast<f32>";     \
            default:                       \
                UNIMPLEMENTED();           \
                return "TOperator_TODO";   \
        }                                  \
    while (false)

#define BITCAST_VECTOR(vecSize)                        \
    do                                                 \
        switch (resultType.getBasicType())             \
        {                                              \
            case TBasicType::EbtInt:                   \
                return "bitcast<vec" vecSize "<i32>>"; \
            case TBasicType::EbtUInt:                  \
                return "bitcast<vec" vecSize "<u32>>"; \
            case TBasicType::EbtFloat:                 \
                return "bitcast<vec" vecSize "<f32>>"; \
            default:                                   \
                UNIMPLEMENTED();                       \
                return "TOperator_TODO";               \
        }                                              \
    while (false)

            if (resultType.isScalar())
            {
                BITCAST_SCALAR();
            }
            else if (resultType.isVector())
            {
                switch (resultType.getNominalSize())
                {
                    case 2:
                        BITCAST_VECTOR("2");
                    case 3:
                        BITCAST_VECTOR("3");
                    case 4:
                        BITCAST_VECTOR("4");
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

#undef BITCAST_SCALAR
#undef BITCAST_VECTOR
        }

        case TOperator::EOpPackUnorm2x16:
            return "pack2x16unorm";
        case TOperator::EOpPackSnorm2x16:
            return "pack2x16snorm";

        case TOperator::EOpPackUnorm4x8:
            return "pack4x8unorm";
        case TOperator::EOpPackSnorm4x8:
            return "pack4x8snorm";

        case TOperator::EOpUnpackUnorm2x16:
            return "unpack2x16unorm";
        case TOperator::EOpUnpackSnorm2x16:
            return "unpack2x16snorm";

        case TOperator::EOpUnpackUnorm4x8:
            return "unpack4x8unorm";
        case TOperator::EOpUnpackSnorm4x8:
            return "unpack4x8snorm";

        case TOperator::EOpPackHalf2x16:
            return "pack2x16float";
        case TOperator::EOpUnpackHalf2x16:
            return "unpack2x16float";

        case TOperator::EOpBarrier:
            UNREACHABLE();
            return "TOperator_TODO";
        case TOperator::EOpMemoryBarrier:
            // TODO(anglebug.com/42267100): does this exist in WGPU? Device-scoped memory barrier?
            // Maybe storageBarrier()?
            UNREACHABLE();
            return "TOperator_TODO";
        case TOperator::EOpGroupMemoryBarrier:
            return "workgroupBarrier";
        case TOperator::EOpMemoryBarrierAtomicCounter:
        case TOperator::EOpMemoryBarrierBuffer:
        case TOperator::EOpMemoryBarrierShared:
            UNREACHABLE();
            return "TOperator_TODO";
        case TOperator::EOpAtomicAdd:
            return "atomicAdd";
        case TOperator::EOpAtomicMin:
            return "atomicMin";
        case TOperator::EOpAtomicMax:
            return "atomicMax";
        case TOperator::EOpAtomicAnd:
            return "atomicAnd";
        case TOperator::EOpAtomicOr:
            return "atomicOr";
        case TOperator::EOpAtomicXor:
            return "atomicXor";
        case TOperator::EOpAtomicExchange:
            return "atomicExchange";
        case TOperator::EOpAtomicCompSwap:
            return "atomicCompareExchangeWeak";  // TODO(anglebug.com/42267100): returns a struct.
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

bool IsSymbolicOperator(TOperator op,
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

const TField &OutputWGSLTraverser::getDirectField(const TIntermTyped &fieldsNode,
                                                  TIntermTyped &indexNode)
{
    const TType &fieldsType = fieldsNode.getType();

    const TFieldListCollection *fieldListCollection = fieldsType.getStruct();
    if (fieldListCollection == nullptr)
    {
        fieldListCollection = fieldsType.getInterfaceBlock();
    }
    ASSERT(fieldListCollection);

    const TIntermConstantUnion *indexNodeAsConstantUnion = indexNode.getAsConstantUnion();
    ASSERT(indexNodeAsConstantUnion);
    const TConstantUnion &index = *indexNodeAsConstantUnion->getConstantValue();

    ASSERT(index.getType() == TBasicType::EbtInt);

    const TFieldList &fieldList = fieldListCollection->fields();
    const int indexVal          = index.getIConst();
    const TField &field         = *fieldList[indexVal];

    return field;
}

void OutputWGSLTraverser::emitArrayIndex(TIntermTyped &leftNode, TIntermTyped &rightNode)
{
    TType leftType = leftNode.getType();

    // Some arrays within the uniform address space have their element types wrapped in a struct
    // when generating WGSL, so this unwraps the element (as an optimization of converting the
    // entire array back to the unwrapped type).
    bool needsUnwrapping                  = false;
    bool isUniformMatrixNeedingConversion = false;
    TIntermBinary *leftNodeBinary         = leftNode.getAsBinaryNode();
    if (leftNodeBinary && leftNodeBinary->getOp() == TOperator::EOpIndexDirectStruct)
    {
        const TStructure *structure = leftNodeBinary->getLeft()->getType().getStruct();

        bool isInUniformAddressSpace =
            mUniformBlockMetadata->structsInUniformAddressSpace.count(structure->uniqueId().get());

        needsUnwrapping =
            structure && ElementTypeNeedsUniformWrapperStruct(isInUniformAddressSpace, &leftType);

        isUniformMatrixNeedingConversion = isInUniformAddressSpace && IsMatCx2(&leftType);

        ASSERT(!needsUnwrapping || !isUniformMatrixNeedingConversion);
    }

    // Emit the left side, which should be of type array.
    if (needsUnwrapping || isUniformMatrixNeedingConversion)
    {
        if (isUniformMatrixNeedingConversion)
        {
            // If this array index expression is yielding an std140 matCx2 (i.e.
            // array<ANGLE_wrapped_vec2, C>), just convert the entire expression to a WGSL matCx2,
            // instead of converting the entire array of std140 matCx2s into an array of WGSL
            // matCx2s and then indexing into it.
            TType baseType = leftType;
            baseType.toArrayBaseType();
            mSink << MakeMatCx2ConversionFunctionName(&baseType) << "(";
            // Make sure the conversion function referenced here is actually generated in the
            // resulting WGSL.
            mWGSLGenerationMetadataForUniforms->outputMatCx2Conversion.insert(baseType);
        }
        emitStructIndexNoUnwrapping(leftNodeBinary);
    }
    else
    {
        groupedTraverse(leftNode);
    }

    mSink << "[";
    const TConstantUnion *constIndex = rightNode.getConstantValue();
    // If the array index is a constant that we can statically verify is within array
    // bounds, just emit that constant.
    if (!leftType.isUnsizedArray() && constIndex != nullptr && constIndex->getType() == EbtInt &&
        constIndex->getIConst() >= 0 &&
        constIndex->getIConst() < static_cast<int>(leftType.isArray()
                                                       ? leftType.getOutermostArraySize()
                                                       : leftType.getNominalSize()))
    {
        emitSingleConstant(constIndex);
    }
    else
    {
        // If the array index is not a constant within the bounds of the array, clamp the
        // index.
        mSink << "clamp(";
        groupedTraverse(rightNode);
        mSink << ", 0, ";
        // Now find the array size and clamp it.
        if (leftType.isUnsizedArray())
        {
            // TODO(anglebug.com/42267100): This is a bug to traverse the `leftNode` a
            // second time if `leftNode` has side effects (and could also have performance
            // implications). This should be stored in a temporary variable. This might also
            // be a bug in the MSL shader compiler.
            mSink << "arrayLength(&";
            groupedTraverse(leftNode);
            mSink << ")";
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
            mSink << maxSize;
        }
        // End the clamp() function.
        mSink << ")";
    }
    // End the array index operation.
    mSink << "]";

    if (needsUnwrapping)
    {
        mSink << "." << kWrappedStructFieldName;
    }
    else if (isUniformMatrixNeedingConversion)
    {
        // Close conversion function call
        mSink << ")";
    }
}

void OutputWGSLTraverser::emitStructIndex(TIntermBinary *binaryNode)
{
    ASSERT(binaryNode->getOp() == TOperator::EOpIndexDirectStruct);
    TIntermTyped &leftNode  = *binaryNode->getLeft();
    const TType *binaryNodeType = &binaryNode->getType();

    const TStructure *structure = leftNode.getType().getStruct();
    ASSERT(structure);

    bool isInUniformAddressSpace =
        mUniformBlockMetadata->structsInUniformAddressSpace.count(structure->uniqueId().get());

    bool isUniformMatrixNeedingConversion = isInUniformAddressSpace && IsMatCx2(binaryNodeType);

    bool needsUnwrapping =
        ElementTypeNeedsUniformWrapperStruct(isInUniformAddressSpace, binaryNodeType);
    if (needsUnwrapping)
    {
        ASSERT(!isUniformMatrixNeedingConversion);

        mSink << MakeUnwrappingArrayConversionFunctionName(&binaryNode->getType()) << "(";
        // Make sure the conversion function referenced here is actually generated in the resulting
        // WGSL.
        mWGSLGenerationMetadataForUniforms->arrayElementTypesThatNeedUnwrappingConversions.insert(
            *binaryNodeType);
    }
    else if (isUniformMatrixNeedingConversion)
    {
        mSink << MakeMatCx2ConversionFunctionName(binaryNodeType) << "(";
        // Make sure the conversion function referenced here is actually generated in the resulting
        // WGSL.
        mWGSLGenerationMetadataForUniforms->outputMatCx2Conversion.insert(*binaryNodeType);
    }
    emitStructIndexNoUnwrapping(binaryNode);
    if (needsUnwrapping || isUniformMatrixNeedingConversion)
    {
        mSink << ")";
    }
}

void OutputWGSLTraverser::emitStructIndexNoUnwrapping(TIntermBinary *binaryNode)
{
    ASSERT(binaryNode->getOp() == TOperator::EOpIndexDirectStruct);
    TIntermTyped &leftNode  = *binaryNode->getLeft();
    TIntermTyped &rightNode = *binaryNode->getRight();

    groupedTraverse(leftNode);
    mSink << ".";
    WriteNameOf(mSink, getDirectField(leftNode, rightNode));
}

bool OutputWGSLTraverser::visitBinary(Visit, TIntermBinary *binaryNode)
{
    const TOperator op      = binaryNode->getOp();
    TIntermTyped &leftNode  = *binaryNode->getLeft();
    TIntermTyped &rightNode = *binaryNode->getRight();

    switch (op)
    {
        case TOperator::EOpIndexDirectStruct:
        case TOperator::EOpIndexDirectInterfaceBlock:
            emitStructIndex(binaryNode);
            break;

        case TOperator::EOpIndexDirect:
        case TOperator::EOpIndexIndirect:
            emitArrayIndex(leftNode, rightNode);
            break;

        default:
        {
            const TType &resultType = binaryNode->getType();
            const TType &leftType   = leftNode.getType();
            const TType &rightType  = rightNode.getType();

            // x * y, x ^ y, etc.
            if (IsSymbolicOperator(op, resultType, &leftType, &rightType))
            {
                groupedTraverse(leftNode);
                if (op != TOperator::EOpComma)
                {
                    mSink << " ";
                }
                mSink << GetOperatorString(op, resultType, &leftType, &rightType, nullptr) << " ";
                groupedTraverse(rightNode);
            }
            // E.g. builtin function calls
            else
            {
                mSink << GetOperatorString(op, resultType, &leftType, &rightType, nullptr) << "(";
                leftNode.traverse(this);
                mSink << ", ";
                rightNode.traverse(this);
                mSink << ")";
            }
        }
    }

    return false;
}

bool IsPostfix(TOperator op)
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

bool OutputWGSLTraverser::visitUnary(Visit, TIntermUnary *unaryNode)
{
    const TOperator op      = unaryNode->getOp();
    const TType &resultType = unaryNode->getType();

    TIntermTyped &arg    = *unaryNode->getOperand();
    const TType &argType = arg.getType();

    const char *name = GetOperatorString(op, resultType, &argType, nullptr, nullptr);

    // Examples: -x, ~x, ~x
    if (IsSymbolicOperator(op, resultType, &argType, nullptr))
    {
        const bool postfix = IsPostfix(op);
        if (!postfix)
        {
            mSink << name;
        }
        groupedTraverse(arg);
        if (postfix)
        {
            mSink << name;
        }
    }
    else
    {
        mSink << name << "(";
        arg.traverse(this);
        mSink << ")";
    }

    return false;
}

bool OutputWGSLTraverser::visitTernary(Visit, TIntermTernary *conditionalNode)
{
    // WGSL does not have a ternary. https://github.com/gpuweb/gpuweb/issues/3747
    // The select() builtin is not short circuiting. Maybe we can get if () {} else {} as an
    // expression, which would also solve the comma operator problem.
    // TODO(anglebug.com/42267100): as mentioned above this is not correct if the operands have side
    // effects. Even if they don't have side effects it could have performance implications.
    // It also doesn't work with all types that ternaries do, e.g. arrays or structs.
    mSink << "select(";
    groupedTraverse(*conditionalNode->getFalseExpression());
    mSink << ", ";
    groupedTraverse(*conditionalNode->getTrueExpression());
    mSink << ", ";
    groupedTraverse(*conditionalNode->getCondition());
    mSink << ")";

    return false;
}

bool OutputWGSLTraverser::visitIfElse(Visit, TIntermIfElse *ifThenElseNode)
{
    TIntermTyped &condNode = *ifThenElseNode->getCondition();
    TIntermBlock *thenNode = ifThenElseNode->getTrueBlock();
    TIntermBlock *elseNode = ifThenElseNode->getFalseBlock();

    mSink << "if (";
    condNode.traverse(this);
    mSink << ")";

    if (thenNode)
    {
        mSink << "\n";
        thenNode->traverse(this);
    }
    else
    {
        mSink << " {}";
    }

    if (elseNode)
    {
        mSink << "\n";
        emitIndentation();
        mSink << "else\n";
        elseNode->traverse(this);
    }

    return false;
}

bool OutputWGSLTraverser::visitSwitch(Visit, TIntermSwitch *switchNode)
{
    TIntermBlock &stmtList = *switchNode->getStatementList();

    emitIndentation();
    mSink << "switch ";
    switchNode->getInit()->traverse(this);
    mSink << "\n";

    emitOpenBrace();

    // TODO(anglebug.com/42267100): Case statements that fall through need to combined into a single
    // case statement with multiple labels.

    const size_t stmtCount = stmtList.getChildCount();
    bool inCaseList        = false;
    size_t currStmt        = 0;
    while (currStmt < stmtCount)
    {
        TIntermNode &stmtNode = *stmtList.getChildNode(currStmt);
        TIntermCase *caseNode = stmtNode.getAsCaseNode();
        if (caseNode)
        {
            if (inCaseList)
            {
                mSink << ", ";
            }
            else
            {
                emitIndentation();
                mSink << "case ";
                inCaseList = true;
            }
            caseNode->traverse(this);

            // Process the next statement.
            currStmt++;
        }
        else
        {
            // The current statement is not a case statement, end the current case list and emit all
            // the code until the next case statement. WGSL requires braces around the case
            // statement's code.
            ASSERT(inCaseList);
            inCaseList = false;
            mSink << ":\n";

            // Count the statements until the next case (or the end of the switch) and emit them as
            // a block. This assumes that the current statement list will never fallthrough to the
            // next case statement.
            size_t nextCaseStmt = currStmt + 1;
            for (;
                 nextCaseStmt < stmtCount && !stmtList.getChildNode(nextCaseStmt)->getAsCaseNode();
                 nextCaseStmt++)
            {
            }
            angle::Span<TIntermNode *> stmtListView(&stmtList.getSequence()->at(currStmt),
                                                    nextCaseStmt - currStmt);
            emitBlock(stmtListView);
            mSink << "\n";

            // Skip to the next case statement.
            currStmt = nextCaseStmt;
        }
    }

    emitCloseBrace();

    return false;
}

bool OutputWGSLTraverser::visitCase(Visit, TIntermCase *caseNode)
{
    // "case" will have been emitted in the visitSwitch() override.

    if (caseNode->hasCondition())
    {
        TIntermTyped *condExpr = caseNode->getCondition();
        condExpr->traverse(this);
    }
    else
    {
        mSink << "default";
    }

    return false;
}

void OutputWGSLTraverser::emitFunctionReturn(const TFunction &func)
{
    const TType &returnType = func.getReturnType();
    if (returnType.getBasicType() == EbtVoid)
    {
        return;
    }
    mSink << " -> ";
    emitType(returnType);
}

// TODO(anglebug.com/42267100): Function overloads are not supported in WGSL, so function names
// should either be emitted mangled or overloaded functions should be renamed in the AST as a
// pre-pass. As of Apr 2024, WGSL function overloads are "not coming soon"
// (https://github.com/gpuweb/gpuweb/issues/876).
void OutputWGSLTraverser::emitFunctionSignature(const TFunction &func)
{
    mSink << "fn ";

    WriteNameOf(mSink, func);
    mSink << "(";

    bool emitComma          = false;
    const size_t paramCount = func.getParamCount();
    for (size_t i = 0; i < paramCount; ++i)
    {
        if (emitComma)
        {
            mSink << ", ";
        }
        emitComma = true;

        const TVariable &param = *func.getParam(i);
        emitFunctionParameter(func, param);
    }

    mSink << ")";

    emitFunctionReturn(func);
}

void OutputWGSLTraverser::emitFunctionParameter(const TFunction &func, const TVariable &param)
{
    // TODO(anglebug.com/42267100): function parameters are immutable and will need to be renamed if
    // they are mutated.
    EmitVariableDeclarationConfig evdConfig;
    evdConfig.isParameter = true;
    emitVariableDeclaration({param.symbolType(), param.name(), param.getType()}, evdConfig);
}

void OutputWGSLTraverser::visitFunctionPrototype(TIntermFunctionPrototype *funcProtoNode)
{
    const TFunction &func = *funcProtoNode->getFunction();

    emitIndentation();
    // TODO(anglebug.com/42267100): output correct signature for main() if main() is declared as a
    // function prototype, or perhaps just emit nothing.
    emitFunctionSignature(func);
}

bool OutputWGSLTraverser::visitFunctionDefinition(Visit, TIntermFunctionDefinition *funcDefNode)
{
    const TFunction &func = *funcDefNode->getFunction();
    TIntermBlock &body    = *funcDefNode->getBody();

    emitIndentation();
    emitFunctionSignature(func);
    mSink << "\n";
    body.traverse(this);

    return false;
}

bool OutputWGSLTraverser::visitAggregate(Visit, TIntermAggregate *aggregateNode)
{
    const TIntermSequence &args = *aggregateNode->getSequence();

    auto emitArgList = [&]() {
        mSink << "(";

        bool emitComma = false;
        for (TIntermNode *arg : args)
        {
            if (emitComma)
            {
                mSink << ", ";
            }
            emitComma = true;
            arg->traverse(this);
        }

        mSink << ")";
    };

    const TType &retType = aggregateNode->getType();

    if (aggregateNode->isConstructor())
    {

        emitType(retType);
        emitArgList();

        return false;
    }
    else
    {
        const TOperator op = aggregateNode->getOp();
        switch (op)
        {
            case TOperator::EOpCallFunctionInAST:
                WriteNameOf(mSink, *aggregateNode->getFunction());
                emitArgList();
                return false;

            default:
                // Do not allow raw function calls, i.e. calls to functions
                // not present in the AST.
                ASSERT(op != TOperator::EOpCallInternalRawFunction);
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
                                mSink << opName;
                                groupedTraverse(operandNode);
                            }
                            else
                            {
                                groupedTraverse(operandNode);
                                mSink << opName;
                            }
                            return false;
                        }

                        case 2:
                        {
                            // symbolic operators with 2 args are emitted with infix notation.
                            TIntermNode &leftNode  = *aggregateNode->getChildNode(0);
                            TIntermNode &rightNode = *aggregateNode->getChildNode(1);
                            groupedTraverse(leftNode);
                            mSink << " " << opName << " ";
                            groupedTraverse(rightNode);
                            return false;
                        }

                        default:
                            UNREACHABLE();
                            return false;
                    }
                }
                else
                {
                    if (opName == nullptr)
                    {
                        // TODO(anglebug.com/42267100): opName should not be allowed to be nullptr
                        // here, but for now not all builtins are mapped to a string.
                        opName = "TODO_Operator";
                    }
                    // If the operator is not symbolic then it is a builtin that uses function call
                    // syntax: builtin(arg1, arg2, ..);
                    mSink << opName;
                    emitArgList();
                    return false;
                }
        }
    }
}

bool OutputWGSLTraverser::emitBlock(angle::Span<TIntermNode *> nodes)
{
    ASSERT(mIndentLevel >= -1);
    const bool isGlobalScope = mIndentLevel == -1;

    if (isGlobalScope)
    {
        ++mIndentLevel;
    }
    else
    {
        emitOpenBrace();
    }

    TIntermNode *prevStmtNode = nullptr;

    const size_t stmtCount = nodes.size();
    for (size_t i = 0; i < stmtCount; ++i)
    {
        TIntermNode &stmtNode = *nodes[i];

        if (isGlobalScope && prevStmtNode && (NewlinePad(*prevStmtNode) || NewlinePad(stmtNode)))
        {
            mSink << "\n";
        }
        const bool isCase = stmtNode.getAsCaseNode();
        mIndentLevel -= isCase;
        emitIndentation();
        mIndentLevel += isCase;
        stmtNode.traverse(this);
        if (RequiresSemicolonTerminator(stmtNode))
        {
            mSink << ";";
        }
        mSink << "\n";

        prevStmtNode = &stmtNode;
    }

    if (isGlobalScope)
    {
        ASSERT(mIndentLevel == 0);
        --mIndentLevel;
    }
    else
    {
        emitCloseBrace();
    }

    return false;
}

bool OutputWGSLTraverser::visitBlock(Visit, TIntermBlock *blockNode)
{
    return emitBlock(
        angle::Span(blockNode->getSequence()->data(), blockNode->getSequence()->size()));
}

bool OutputWGSLTraverser::visitGlobalQualifierDeclaration(Visit,
                                                          TIntermGlobalQualifierDeclaration *)
{
    return false;
}

void OutputWGSLTraverser::emitStructDeclaration(const TType &type)
{
    ASSERT(type.getBasicType() == TBasicType::EbtStruct);
    ASSERT(type.isStructSpecifier());

    mSink << "struct ";
    emitBareTypeName(type);

    mSink << "\n";
    emitOpenBrace();

    const TStructure &structure = *type.getStruct();
    bool isInUniformAddressSpace =
        mUniformBlockMetadata->structsInUniformAddressSpace.count(structure.uniqueId().get()) != 0;

    bool alignTo16InUniformAddressSpace = true;
    for (const TField *field : structure.fields())
    {
        const TType *fieldType = field->type();

        emitIndentation();
        // If this struct is used in the uniform address space, it must obey the uniform address
        // space's layout constaints (https://www.w3.org/TR/WGSL/#address-space-layout-constraints).
        // WGSL's address space layout constraints nearly match std140, and the places they don't
        // are handled elsewhere.
        if (isInUniformAddressSpace)
        {
            // Here, the field must be aligned to 16 if:
            // 1. The field is a struct or array (note that matCx2 is represented as an array of
            // vec2)
            // 2. The previous field is a struct
            // 3. The field is the first in the struct (for convenience).
            if (field->type()->getStruct() || fieldType->isArray() || IsMatCx2(fieldType))
            {
                alignTo16InUniformAddressSpace = true;
            }
            if (alignTo16InUniformAddressSpace)
            {
                mSink << "@align(16) ";
            }

            // If this field is a struct, the next member should be aligned to 16.
            alignTo16InUniformAddressSpace = fieldType->getStruct();

            // If the field is an array whose stride is not aligned to 16, the element type must be
            // emitted with a wrapper struct. Record that the wrapper struct needs to be emitted.
            // Note that if the array element type is already of struct type, it doesn't need
            // another wrapper struct, it will automatically be aligned to 16 because its first
            // member is aligned to 16 (implemented above).
            if (ElementTypeNeedsUniformWrapperStruct(/*inUniformAddressSpace=*/true, fieldType))
            {
                TType innerType = *fieldType;
                innerType.toArrayElementType();
                // Multidimensional arrays not currently supported in uniforms in the WebGPU backend
                ASSERT(!innerType.isArray());
                mWGSLGenerationMetadataForUniforms->arrayElementTypesInUniforms.insert(innerType);
            }
        }

        // TODO(anglebug.com/42267100): emit qualifiers.
        EmitVariableDeclarationConfig evdConfig;
        evdConfig.typeConfig.addressSpace =
            isInUniformAddressSpace ? WgslAddressSpace::Uniform : WgslAddressSpace::NonUniform;
        evdConfig.disableStructSpecifier = true;
        emitVariableDeclaration({field->symbolType(), field->name(), *fieldType}, evdConfig);
        mSink << ",\n";
    }

    emitCloseBrace();
}

void OutputWGSLTraverser::emitVariableDeclaration(const VarDecl &decl,
                                                  const EmitVariableDeclarationConfig &evdConfig)
{
    const TBasicType basicType = decl.type.getBasicType();

    if (decl.type.getQualifier() == EvqUniform)
    {
        // Uniforms are declared in a pre-pass, and don't need to be outputted here.
        return;
    }

    if (basicType == TBasicType::EbtStruct && decl.type.isStructSpecifier() &&
        !evdConfig.disableStructSpecifier)
    {
        // TODO(anglebug.com/42267100): in WGSL structs probably can't be declared in
        // function parameters or in uniform declarations or in variable declarations, or
        // anonymously either within other structs or within a variable declaration. Handle
        // these with the same AST pre-passes as other shader translators.
        ASSERT(!evdConfig.isParameter);
        emitStructDeclaration(decl.type);
        if (decl.symbolType != SymbolType::Empty)
        {
            mSink << " ";
            emitNameOf(decl);
        }
        return;
    }

    ASSERT(basicType == TBasicType::EbtStruct || decl.symbolType != SymbolType::Empty ||
           evdConfig.isParameter);

    if (evdConfig.needsVar)
    {
        // "const" and "let" probably don't need to be ever emitted because they are more for
        // readability, and the GLSL compiler constant folds most (all?) the consts anyway.
        mSink << "var";
        // TODO(anglebug.com/42267100): <workgroup> or <storage>?
        if (evdConfig.isGlobalScope)
        {
            mSink << "<private>";
        }
        mSink << " ";
    }
    else
    {
        ASSERT(!evdConfig.isGlobalScope);
    }

    if (decl.symbolType != SymbolType::Empty)
    {
        emitNameOf(decl);
    }
    mSink << " : ";
    WriteWgslType(mSink, decl.type, evdConfig.typeConfig);
}

bool OutputWGSLTraverser::visitDeclaration(Visit, TIntermDeclaration *declNode)
{
    ASSERT(declNode->getChildCount() == 1);
    TIntermNode &node = *declNode->getChildNode(0);

    EmitVariableDeclarationConfig evdConfig;
    evdConfig.needsVar      = true;
    evdConfig.isGlobalScope = mIndentLevel == 0;

    if (TIntermSymbol *symbolNode = node.getAsSymbolNode())
    {
        const TVariable &var = symbolNode->variable();
        if (mRewritePipelineVarOutput->IsInputVar(var.uniqueId()) ||
            mRewritePipelineVarOutput->IsOutputVar(var.uniqueId()))
        {
            // Some variables, like shader inputs/outputs/builtins, are declared in the WGSL source
            // outside of the traverser.
            return false;
        }
        emitVariableDeclaration({var.symbolType(), var.name(), var.getType()}, evdConfig);
    }
    else if (TIntermBinary *initNode = node.getAsBinaryNode())
    {
        ASSERT(initNode->getOp() == TOperator::EOpInitialize);
        TIntermSymbol *leftSymbolNode = initNode->getLeft()->getAsSymbolNode();
        TIntermTyped *valueNode       = initNode->getRight()->getAsTyped();
        ASSERT(leftSymbolNode && valueNode);

        const TVariable &var = leftSymbolNode->variable();
        if (mRewritePipelineVarOutput->IsInputVar(var.uniqueId()) ||
            mRewritePipelineVarOutput->IsOutputVar(var.uniqueId()))
        {
            // Some variables, like shader inputs/outputs/builtins, are declared in the WGSL source
            // outside of the traverser.
            return false;
        }

        emitVariableDeclaration({var.symbolType(), var.name(), var.getType()}, evdConfig);
        mSink << " = ";
        groupedTraverse(*valueNode);
    }
    else
    {
        UNREACHABLE();
    }

    return false;
}

bool OutputWGSLTraverser::visitLoop(Visit, TIntermLoop *loopNode)
{
    const TLoopType loopType = loopNode->getType();

    switch (loopType)
    {
        case TLoopType::ELoopFor:
            return emitForLoop(loopNode);
        case TLoopType::ELoopWhile:
            return emitWhileLoop(loopNode);
        case TLoopType::ELoopDoWhile:
            return emulateDoWhileLoop(loopNode);
    }
}

bool OutputWGSLTraverser::emitForLoop(TIntermLoop *loopNode)
{
    ASSERT(loopNode->getType() == TLoopType::ELoopFor);

    TIntermNode *initNode  = loopNode->getInit();
    TIntermTyped *condNode = loopNode->getCondition();
    TIntermTyped *exprNode = loopNode->getExpression();

    mSink << "for (";

    if (initNode)
    {
        initNode->traverse(this);
    }
    else
    {
        mSink << " ";
    }

    mSink << "; ";

    if (condNode)
    {
        condNode->traverse(this);
    }

    mSink << "; ";

    if (exprNode)
    {
        exprNode->traverse(this);
    }

    mSink << ")\n";

    loopNode->getBody()->traverse(this);

    return false;
}

bool OutputWGSLTraverser::emitWhileLoop(TIntermLoop *loopNode)
{
    ASSERT(loopNode->getType() == TLoopType::ELoopWhile);

    TIntermNode *initNode  = loopNode->getInit();
    TIntermTyped *condNode = loopNode->getCondition();
    TIntermTyped *exprNode = loopNode->getExpression();
    ASSERT(condNode);
    ASSERT(!initNode && !exprNode);

    emitIndentation();
    mSink << "while (";
    condNode->traverse(this);
    mSink << ")\n";
    loopNode->getBody()->traverse(this);

    return false;
}

bool OutputWGSLTraverser::emulateDoWhileLoop(TIntermLoop *loopNode)
{
    ASSERT(loopNode->getType() == TLoopType::ELoopDoWhile);

    TIntermNode *initNode  = loopNode->getInit();
    TIntermTyped *condNode = loopNode->getCondition();
    TIntermTyped *exprNode = loopNode->getExpression();
    ASSERT(condNode);
    ASSERT(!initNode && !exprNode);

    emitIndentation();
    // Write an infinite loop.
    mSink << "loop {\n";
    mIndentLevel++;
    loopNode->getBody()->traverse(this);
    mSink << "\n";
    emitIndentation();
    // At the end of the loop, break if the loop condition dos not still hold.
    mSink << "if (!(";
    condNode->traverse(this);
    mSink << ") { break; }\n";
    mIndentLevel--;
    emitIndentation();
    mSink << "}";

    return false;
}

bool OutputWGSLTraverser::visitBranch(Visit, TIntermBranch *branchNode)
{
    const TOperator flowOp = branchNode->getFlowOp();
    TIntermTyped *exprNode = branchNode->getExpression();

    emitIndentation();

    switch (flowOp)
    {
        case TOperator::EOpKill:
        {
            ASSERT(exprNode == nullptr);
            mSink << "discard";
        }
        break;

        case TOperator::EOpReturn:
        {
            mSink << "return";
            if (exprNode)
            {
                mSink << " ";
                exprNode->traverse(this);
            }
        }
        break;

        case TOperator::EOpBreak:
        {
            ASSERT(exprNode == nullptr);
            mSink << "break";
        }
        break;

        case TOperator::EOpContinue:
        {
            ASSERT(exprNode == nullptr);
            mSink << "continue";
        }
        break;

        default:
        {
            UNREACHABLE();
        }
    }

    return false;
}

void OutputWGSLTraverser::visitPreprocessorDirective(TIntermPreprocessorDirective *node)
{
    // No preprocessor directives expected at this point.
    UNREACHABLE();
}

void OutputWGSLTraverser::emitBareTypeName(const TType &type)
{
    WriteWgslBareTypeName(mSink, type, {});
}

void OutputWGSLTraverser::emitType(const TType &type)
{
    WriteWgslType(mSink, type, {});
}

}  // namespace

TranslatorWGSL::TranslatorWGSL(sh::GLenum type, ShShaderSpec spec, ShShaderOutput output)
    : TCompiler(type, spec, output)
{}

bool TranslatorWGSL::translate(TIntermBlock *root,
                               const ShCompileOptions &compileOptions,
                               PerformanceDiagnostics *perfDiagnostics)
{
    if (kOutputTreeBeforeTranslation)
    {
        OutputTree(root, getInfoSink().info);
        std::cout << getInfoSink().info.c_str();
    }

    RewritePipelineVarOutput rewritePipelineVarOutput(getShaderType());
    WGSLGenerationMetadataForUniforms wgslGenerationMetadataForUniforms;

    // WGSL's main() will need to take parameters or return values if any glsl (input/output)
    // builtin variables are used.
    if (!GenerateMainFunctionAndIOStructs(*this, *root, rewritePipelineVarOutput))
    {
        return false;
    }

    TInfoSinkBase &sink = getInfoSink().obj;
    // Start writing the output structs that will be referred to by the `traverser`'s output.'
    if (!rewritePipelineVarOutput.OutputStructs(sink))
    {
        return false;
    }

    if (!OutputUniformBlocks(this, root))
    {
        return false;
    }

    UniformBlockMetadata uniformBlockMetadata;
    if (!RecordUniformBlockMetadata(root, uniformBlockMetadata))
    {
        return false;
    }

    // Generate the body of the WGSL including the GLSL main() function.
    TInfoSinkBase traverserOutput;
    OutputWGSLTraverser traverser(&traverserOutput, &rewritePipelineVarOutput,
                                  &uniformBlockMetadata, &wgslGenerationMetadataForUniforms);
    root->traverse(&traverser);

    sink << "\n";
    OutputUniformWrapperStructsAndConversions(sink, wgslGenerationMetadataForUniforms);

    // The traverser output needs to be in the code after uniform wrapper structs are emitted above,
    // since the traverser code references the wrapper struct types.
    sink << traverserOutput.str();

    // Write the actual WGSL main function, wgslMain(), which calls the GLSL main function.
    if (!rewritePipelineVarOutput.OutputMainFunction(sink))
    {
        return false;
    }

    if (kOutputTranslatedShader)
    {
        std::cout << sink.str();
    }

    return true;
}

bool TranslatorWGSL::shouldFlattenPragmaStdglInvariantAll()
{
    // Not neccesary for WGSL transformation.
    return false;
}
}  // namespace sh
