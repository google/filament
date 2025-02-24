//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/glsl/OutputGLSLBase.h"

#include "angle_gl.h"
#include "common/debug.h"
#include "common/mathutil.h"
#include "compiler/translator/Compiler.h"
#include "compiler/translator/util.h"

#include <cfloat>

namespace sh
{

namespace
{

bool isSingleStatement(TIntermNode *node)
{
    if (node->getAsFunctionDefinition())
    {
        return false;
    }
    else if (node->getAsBlock())
    {
        return false;
    }
    else if (node->getAsIfElseNode())
    {
        return false;
    }
    else if (node->getAsLoopNode())
    {
        return false;
    }
    else if (node->getAsSwitchNode())
    {
        return false;
    }
    else if (node->getAsCaseNode())
    {
        return false;
    }
    else if (node->getAsPreprocessorDirective())
    {
        return false;
    }
    return true;
}

class CommaSeparatedListItemPrefixGenerator
{
  public:
    CommaSeparatedListItemPrefixGenerator() : mFirst(true) {}

  private:
    bool mFirst;

    template <typename Stream>
    friend Stream &operator<<(Stream &out, CommaSeparatedListItemPrefixGenerator &gen);
};

template <typename Stream>
Stream &operator<<(Stream &out, CommaSeparatedListItemPrefixGenerator &gen)
{
    if (gen.mFirst)
    {
        gen.mFirst = false;
    }
    else
    {
        out << ", ";
    }
    return out;
}

}  // namespace

TOutputGLSLBase::TOutputGLSLBase(TCompiler *compiler,
                                 TInfoSinkBase &objSink,
                                 const ShCompileOptions &compileOptions)
    : TIntermTraverser(true, true, true, &compiler->getSymbolTable()),
      mObjSink(objSink),
      mDeclaringVariable(false),
      mHashFunction(compiler->getHashFunction()),
      mNameMap(compiler->getNameMap()),
      mShaderType(compiler->getShaderType()),
      mShaderVersion(compiler->getShaderVersion()),
      mOutput(compiler->getOutputType()),
      mHighPrecisionSupported(compiler->isHighPrecisionSupported()),
      // If pixel local storage introduces new fragment outputs, we are now required to specify a
      // location for _all_ fragment outputs, including previously valid outputs that had an
      // implicit location of zero.
      mAlwaysSpecifyFragOutLocation(
          compileOptions.explicitFragmentLocations ||
          (compiler->hasPixelLocalStorageUniforms() &&
           compileOptions.pls.type == ShPixelLocalStorageType::FramebufferFetch)),
      mCompileOptions(compileOptions)
{}

void TOutputGLSLBase::writeInvariantQualifier(const TType &type)
{
    if (!sh::RemoveInvariant(mShaderType, mShaderVersion, mOutput, mCompileOptions))
    {
        TInfoSinkBase &out = objSink();
        out << "invariant ";
    }
}

void TOutputGLSLBase::writePreciseQualifier(const TType &type)
{
    TInfoSinkBase &out = objSink();
    out << "precise ";
}

void TOutputGLSLBase::writeFloat(TInfoSinkBase &out, float f)
{
    if ((gl::isInf(f) || gl::isNaN(f)) && mShaderVersion >= 300)
    {
        out << "uintBitsToFloat(" << gl::bitCast<uint32_t>(f) << "u)";
    }
    else
    {
        out << std::min(FLT_MAX, std::max(-FLT_MAX, f));
    }
}

void TOutputGLSLBase::writeTriplet(Visit visit,
                                   const char *preStr,
                                   const char *inStr,
                                   const char *postStr)
{
    TInfoSinkBase &out = objSink();
    if (visit == PreVisit && preStr)
        out << preStr;
    else if (visit == InVisit && inStr)
        out << inStr;
    else if (visit == PostVisit && postStr)
        out << postStr;
}

void TOutputGLSLBase::writeFunctionTriplet(Visit visit,
                                           const ImmutableString &functionName,
                                           bool useEmulatedFunction)
{
    TInfoSinkBase &out = objSink();
    if (visit == PreVisit)
    {
        if (useEmulatedFunction)
        {
            BuiltInFunctionEmulator::WriteEmulatedFunctionName(out, functionName.data());
        }
        else
        {
            out << functionName;
        }
        out << "(";
    }
    else
    {
        writeTriplet(visit, nullptr, ", ", ")");
    }
}

// Outputs what goes inside layout(), except for location and binding qualifiers, as they are
// handled differently between GL GLSL and Vulkan GLSL.
std::string TOutputGLSLBase::getCommonLayoutQualifiers(TIntermSymbol *variable)
{
    std::ostringstream out;
    CommaSeparatedListItemPrefixGenerator listItemPrefix;

    const TType &type                       = variable->getType();
    const TLayoutQualifier &layoutQualifier = type.getLayoutQualifier();

    if (type.getQualifier() == EvqFragDepth)
    {
        ASSERT(layoutQualifier.depth != EdUnspecified);
        out << listItemPrefix << getDepthString(layoutQualifier.depth);
    }

    if (type.getQualifier() == EvqFragmentOut || type.getQualifier() == EvqFragmentInOut)
    {
        if (layoutQualifier.index >= 0)
        {
            out << listItemPrefix << "index = " << layoutQualifier.index;
        }
        if (layoutQualifier.yuv)
        {
            out << listItemPrefix << "yuv";
        }
    }

    if (type.getQualifier() == EvqFragmentInOut && layoutQualifier.noncoherent)
    {
        out << listItemPrefix << "noncoherent";
    }

    if (IsImage(type.getBasicType()))
    {
        if (layoutQualifier.imageInternalFormat != EiifUnspecified)
        {
            ASSERT(type.getQualifier() == EvqTemporary || type.getQualifier() == EvqUniform);
            out << listItemPrefix
                << getImageInternalFormatString(layoutQualifier.imageInternalFormat);
        }
    }

    if (IsAtomicCounter(type.getBasicType()))
    {
        out << listItemPrefix << "offset = " << layoutQualifier.offset;
    }

    return out.str();
}

// Outputs memory qualifiers applied to images, buffers and its fields, as well as image function
// arguments.
std::string TOutputGLSLBase::getMemoryQualifiers(const TType &type)
{
    std::ostringstream out;

    const TMemoryQualifier &memoryQualifier = type.getMemoryQualifier();
    if (memoryQualifier.readonly)
    {
        out << "readonly ";
    }

    if (memoryQualifier.writeonly)
    {
        out << "writeonly ";
    }

    if (memoryQualifier.coherent)
    {
        out << "coherent ";
    }

    if (memoryQualifier.restrictQualifier)
    {
        out << "restrict ";
    }

    if (memoryQualifier.volatileQualifier)
    {
        out << "volatile ";
    }

    return out.str();
}

void TOutputGLSLBase::writeLayoutQualifier(TIntermSymbol *variable)
{
    const TType &type = variable->getType();

    if (!needsToWriteLayoutQualifier(type))
    {
        return;
    }

    if (type.getBasicType() == EbtInterfaceBlock)
    {
        declareInterfaceBlockLayout(type);
        return;
    }

    TInfoSinkBase &out                      = objSink();
    const TLayoutQualifier &layoutQualifier = type.getLayoutQualifier();
    out << "layout(";

    CommaSeparatedListItemPrefixGenerator listItemPrefix;

    if (IsFragmentOutput(type.getQualifier()) || type.getQualifier() == EvqVertexIn ||
        IsVarying(type.getQualifier()))
    {
        if (layoutQualifier.location >= 0 ||
            (mAlwaysSpecifyFragOutLocation && IsFragmentOutput(type.getQualifier()) &&
             !layoutQualifier.yuv))
        {
            out << listItemPrefix << "location = " << std::max(layoutQualifier.location, 0);
        }
    }

    if (IsOpaqueType(type.getBasicType()))
    {
        if (layoutQualifier.binding >= 0)
        {
            out << listItemPrefix << "binding = " << layoutQualifier.binding;
        }
    }

    std::string otherQualifiers = getCommonLayoutQualifiers(variable);
    if (!otherQualifiers.empty())
    {
        out << listItemPrefix << otherQualifiers;
    }

    out << ") ";
}

void TOutputGLSLBase::writeFieldLayoutQualifier(const TField *field)
{
    TLayoutQualifier layoutQualifier = field->type()->getLayoutQualifier();
    if (!field->type()->isMatrix() && !field->type()->isStructureContainingMatrices() &&
        layoutQualifier.imageInternalFormat == EiifUnspecified)
    {
        return;
    }

    TInfoSinkBase &out = objSink();

    out << "layout(";
    CommaSeparatedListItemPrefixGenerator listItemPrefix;
    if (field->type()->isMatrix() || field->type()->isStructureContainingMatrices())
    {
        switch (layoutQualifier.matrixPacking)
        {
            case EmpUnspecified:
            case EmpColumnMajor:
                // Default matrix packing is column major.
                out << listItemPrefix << "column_major";
                break;

            case EmpRowMajor:
                out << listItemPrefix << "row_major";
                break;

            default:
                UNREACHABLE();
                break;
        }
    }
    // EXT_shader_pixel_local_storage.
    if (layoutQualifier.imageInternalFormat != EiifUnspecified)
    {
        out << listItemPrefix << getImageInternalFormatString(layoutQualifier.imageInternalFormat);
    }
    out << ") ";
}

void TOutputGLSLBase::writeQualifier(TQualifier qualifier, const TType &type, const TSymbol *symbol)
{
    const char *result = mapQualifierToString(qualifier);
    if (result && result[0] != '\0')
    {
        objSink() << result << " ";
    }

    objSink() << getMemoryQualifiers(type);
}

const char *TOutputGLSLBase::mapQualifierToString(TQualifier qualifier)
{
    if (sh::IsGLSL410OrOlder(mOutput) && mShaderVersion >= 300 &&
        mCompileOptions.removeInvariantAndCentroidForESSL3)
    {
        switch (qualifier)
        {
            // The return string is consistent with sh::getQualifierString() from
            // BaseTypes.h minus the "centroid" keyword.
            case EvqCentroid:
                return "";
            case EvqCentroidIn:
                return "smooth in";
            case EvqCentroidOut:
                return "smooth out";
            case EvqNoPerspectiveCentroid:
                return "noperspective";
            case EvqNoPerspectiveCentroidIn:
                return "noperspective in";
            case EvqNoPerspectiveCentroidOut:
                return "noperspective out";
            default:
                break;
        }
    }
    if (sh::IsGLSL130OrNewer(mOutput))
    {
        switch (qualifier)
        {
            case EvqAttribute:
                return "in";
            case EvqVaryingIn:
                return "in";
            case EvqVaryingOut:
                return "out";
            default:
                break;
        }
    }

    switch (qualifier)
    {
        // When emulated, gl_ViewID_OVR uses flat qualifiers.
        case EvqViewIDOVR:
            return mShaderType == GL_FRAGMENT_SHADER ? "flat in" : "flat out";

        // gl_ClipDistance / gl_CullDistance require different qualifiers based on shader type.
        case EvqClipDistance:
        case EvqCullDistance:
            return (sh::IsGLSL130OrNewer(mOutput) || mShaderVersion > 100)
                       ? (mShaderType == GL_FRAGMENT_SHADER ? "in" : "out")
                       : "varying";

        case EvqFragDepth:
            return "out";

        // gl_LastFragColor / gl_LastFragData have no qualifiers.
        case EvqLastFragData:
        case EvqLastFragColor:
            return nullptr;

        default:
            return sh::getQualifierString(qualifier);
    }
}

namespace
{

constexpr char kIndent[]      = "                    ";  // 10x2 spaces
constexpr int kIndentWidth    = 2;
constexpr int kMaxIndentLevel = sizeof(kIndent) / kIndentWidth;

}  // namespace

const char *TOutputGLSLBase::getIndentPrefix(int extraIndentation)
{
    int indentDepth = std::min(kMaxIndentLevel, getCurrentBlockDepth() + extraIndentation);
    ASSERT(indentDepth >= 0);
    return kIndent + (kMaxIndentLevel - indentDepth) * kIndentWidth;
}

void TOutputGLSLBase::writeVariableType(const TType &type,
                                        const TSymbol *symbol,
                                        bool isFunctionArgument)
{
    TQualifier qualifier = type.getQualifier();
    TInfoSinkBase &out   = objSink();
    if (type.isInvariant())
    {
        writeInvariantQualifier(type);
    }
    if (type.isPrecise())
    {
        writePreciseQualifier(type);
    }
    if (qualifier != EvqTemporary && qualifier != EvqGlobal)
    {
        writeQualifier(qualifier, type, symbol);
    }
    if (isFunctionArgument)
    {
        // Function arguments are the only place (other than image/SSBO/field declaration) where
        // memory qualifiers can appear.
        out << getMemoryQualifiers(type);
    }

    // Declare the struct.
    if (type.isStructSpecifier())
    {
        const TStructure *structure = type.getStruct();

        declareStruct(structure);
    }
    else if (type.getBasicType() == EbtInterfaceBlock)
    {
        declareInterfaceBlock(type);
    }
    else
    {
        if (writeVariablePrecision(type.getPrecision()))
            out << " ";
        out << getTypeName(type);
    }
}

void TOutputGLSLBase::writeFunctionParameters(const TFunction *func)
{
    TInfoSinkBase &out = objSink();
    size_t paramCount  = func->getParamCount();
    for (size_t i = 0; i < paramCount; ++i)
    {
        const TVariable *param = func->getParam(i);
        const TType &type      = param->getType();
        writeVariableType(type, param, true);

        if (param->symbolType() != SymbolType::Empty)
        {
            out << " " << hashName(param);
        }
        if (type.isArray())
        {
            out << ArrayString(type);
        }

        // Put a comma if this is not the last argument.
        if (i != paramCount - 1)
            out << ", ";
    }
}

const TConstantUnion *TOutputGLSLBase::writeConstantUnion(const TType &type,
                                                          const TConstantUnion *pConstUnion)
{
    TInfoSinkBase &out = objSink();

    if (type.getBasicType() == EbtStruct)
    {
        const TStructure *structure = type.getStruct();
        out << hashName(structure) << "(";

        const TFieldList &fields = structure->fields();
        for (size_t i = 0; i < fields.size(); ++i)
        {
            const TType *fieldType = fields[i]->type();
            ASSERT(fieldType != nullptr);
            pConstUnion = writeConstantUnion(*fieldType, pConstUnion);
            if (i != fields.size() - 1)
                out << ", ";
        }
        out << ")";
    }
    else
    {
        size_t size    = type.getObjectSize();
        bool writeType = size > 1;
        if (writeType)
            out << getTypeName(type) << "(";
        for (size_t i = 0; i < size; ++i, ++pConstUnion)
        {
            switch (pConstUnion->getType())
            {
                case EbtFloat:
                    writeFloat(out, pConstUnion->getFConst());
                    break;
                case EbtInt:
                    out << pConstUnion->getIConst();
                    break;
                case EbtUInt:
                    out << pConstUnion->getUConst() << "u";
                    break;
                case EbtBool:
                    out << pConstUnion->getBConst();
                    break;
                case EbtYuvCscStandardEXT:
                    out << getYuvCscStandardEXTString(pConstUnion->getYuvCscStandardEXTConst());
                    break;
                default:
                    UNREACHABLE();
            }
            if (i != size - 1)
                out << ", ";
        }
        if (writeType)
            out << ")";
    }
    return pConstUnion;
}

void TOutputGLSLBase::writeConstructorTriplet(Visit visit, const TType &type)
{
    TInfoSinkBase &out = objSink();
    if (visit == PreVisit)
    {
        if (type.isArray())
        {
            out << getTypeName(type);
            out << ArrayString(type);
            out << "(";
        }
        else
        {
            out << getTypeName(type) << "(";
        }
    }
    else
    {
        writeTriplet(visit, nullptr, ", ", ")");
    }
}

void TOutputGLSLBase::visitSymbol(TIntermSymbol *node)
{
    TInfoSinkBase &out = objSink();
    out << hashName(&node->variable());

    if (mDeclaringVariable && node->getType().isArray())
        out << ArrayString(node->getType());
}

void TOutputGLSLBase::visitConstantUnion(TIntermConstantUnion *node)
{
    writeConstantUnion(node->getType(), node->getConstantValue());
}

bool TOutputGLSLBase::visitSwizzle(Visit visit, TIntermSwizzle *node)
{
    TInfoSinkBase &out = objSink();
    if (visit == PostVisit)
    {
        out << ".";
        node->writeOffsetsAsXYZW(&out);
    }
    return true;
}

bool TOutputGLSLBase::visitBinary(Visit visit, TIntermBinary *node)
{
    bool visitChildren = true;
    TInfoSinkBase &out = objSink();
    switch (node->getOp())
    {
        case EOpComma:
            writeTriplet(visit, "(", ", ", ")");
            break;
        case EOpInitialize:
            if (visit == InVisit)
            {
                out << " = ";
                // RHS of initialize is not being declared.
                mDeclaringVariable = false;
            }
            break;
        case EOpAssign:
            writeTriplet(visit, "(", " = ", ")");
            break;
        case EOpAddAssign:
            writeTriplet(visit, "(", " += ", ")");
            break;
        case EOpSubAssign:
            writeTriplet(visit, "(", " -= ", ")");
            break;
        case EOpDivAssign:
            writeTriplet(visit, "(", " /= ", ")");
            break;
        case EOpIModAssign:
            writeTriplet(visit, "(", " %= ", ")");
            break;
        // Notice the fall-through.
        case EOpMulAssign:
        case EOpVectorTimesMatrixAssign:
        case EOpVectorTimesScalarAssign:
        case EOpMatrixTimesScalarAssign:
        case EOpMatrixTimesMatrixAssign:
            writeTriplet(visit, "(", " *= ", ")");
            break;
        case EOpBitShiftLeftAssign:
            writeTriplet(visit, "(", " <<= ", ")");
            break;
        case EOpBitShiftRightAssign:
            writeTriplet(visit, "(", " >>= ", ")");
            break;
        case EOpBitwiseAndAssign:
            writeTriplet(visit, "(", " &= ", ")");
            break;
        case EOpBitwiseXorAssign:
            writeTriplet(visit, "(", " ^= ", ")");
            break;
        case EOpBitwiseOrAssign:
            writeTriplet(visit, "(", " |= ", ")");
            break;

        case EOpIndexDirect:
        case EOpIndexIndirect:
            writeTriplet(visit, nullptr, "[", "]");
            break;
        case EOpIndexDirectStruct:
            if (visit == InVisit)
            {
                // Here we are writing out "foo.bar", where "foo" is struct
                // and "bar" is field. In AST, it is represented as a binary
                // node, where left child represents "foo" and right child "bar".
                // The node itself represents ".". The struct field "bar" is
                // actually stored as an index into TStructure::fields.
                out << ".";
                const TStructure *structure       = node->getLeft()->getType().getStruct();
                const TIntermConstantUnion *index = node->getRight()->getAsConstantUnion();
                const TField *field               = structure->fields()[index->getIConst(0)];

                out << hashFieldName(field);
                visitChildren = false;
            }
            break;
        case EOpIndexDirectInterfaceBlock:
            if (visit == InVisit)
            {
                out << ".";
                const TInterfaceBlock *interfaceBlock =
                    node->getLeft()->getType().getInterfaceBlock();
                const TIntermConstantUnion *index = node->getRight()->getAsConstantUnion();
                const TField *field               = interfaceBlock->fields()[index->getIConst(0)];
                out << hashFieldName(field);
                visitChildren = false;
            }
            break;

        case EOpAdd:
            writeTriplet(visit, "(", " + ", ")");
            break;
        case EOpSub:
            writeTriplet(visit, "(", " - ", ")");
            break;
        case EOpMul:
            writeTriplet(visit, "(", " * ", ")");
            break;
        case EOpDiv:
            writeTriplet(visit, "(", " / ", ")");
            break;
        case EOpIMod:
            writeTriplet(visit, "(", " % ", ")");
            break;
        case EOpBitShiftLeft:
            writeTriplet(visit, "(", " << ", ")");
            break;
        case EOpBitShiftRight:
            writeTriplet(visit, "(", " >> ", ")");
            break;
        case EOpBitwiseAnd:
            writeTriplet(visit, "(", " & ", ")");
            break;
        case EOpBitwiseXor:
            writeTriplet(visit, "(", " ^ ", ")");
            break;
        case EOpBitwiseOr:
            writeTriplet(visit, "(", " | ", ")");
            break;

        case EOpEqual:
            writeTriplet(visit, "(", " == ", ")");
            break;
        case EOpNotEqual:
            writeTriplet(visit, "(", " != ", ")");
            break;
        case EOpLessThan:
            writeTriplet(visit, "(", " < ", ")");
            break;
        case EOpGreaterThan:
            writeTriplet(visit, "(", " > ", ")");
            break;
        case EOpLessThanEqual:
            writeTriplet(visit, "(", " <= ", ")");
            break;
        case EOpGreaterThanEqual:
            writeTriplet(visit, "(", " >= ", ")");
            break;

        // Notice the fall-through.
        case EOpVectorTimesScalar:
        case EOpVectorTimesMatrix:
        case EOpMatrixTimesVector:
        case EOpMatrixTimesScalar:
        case EOpMatrixTimesMatrix:
            writeTriplet(visit, "(", " * ", ")");
            break;

        case EOpLogicalOr:
            writeTriplet(visit, "(", " || ", ")");
            break;
        case EOpLogicalXor:
            writeTriplet(visit, "(", " ^^ ", ")");
            break;
        case EOpLogicalAnd:
            writeTriplet(visit, "(", " && ", ")");
            break;
        default:
            UNREACHABLE();
    }

    return visitChildren;
}

bool TOutputGLSLBase::visitUnary(Visit visit, TIntermUnary *node)
{
    const char *preString  = "";
    const char *postString = ")";

    switch (node->getOp())
    {
        case EOpNegative:
            preString = "(-";
            break;
        case EOpPositive:
            preString = "(+";
            break;
        case EOpLogicalNot:
            preString = "(!";
            break;
        case EOpBitwiseNot:
            preString = "(~";
            break;

        case EOpPostIncrement:
            preString  = "(";
            postString = "++)";
            break;
        case EOpPostDecrement:
            preString  = "(";
            postString = "--)";
            break;
        case EOpPreIncrement:
            preString = "(++";
            break;
        case EOpPreDecrement:
            preString = "(--";
            break;
        case EOpArrayLength:
            preString  = "((";
            postString = ").length())";
            break;

        default:
            writeFunctionTriplet(visit, node->getFunction()->name(),
                                 node->getUseEmulatedFunction());
            return true;
    }

    writeTriplet(visit, preString, nullptr, postString);

    return true;
}

bool TOutputGLSLBase::visitTernary(Visit visit, TIntermTernary *node)
{
    TInfoSinkBase &out = objSink();
    // Notice two brackets at the beginning and end. The outer ones
    // encapsulate the whole ternary expression. This preserves the
    // order of precedence when ternary expressions are used in a
    // compound expression, i.e., c = 2 * (a < b ? 1 : 2).
    out << "((";
    node->getCondition()->traverse(this);
    out << ") ? (";
    node->getTrueExpression()->traverse(this);
    out << ") : (";
    node->getFalseExpression()->traverse(this);
    out << "))";
    return false;
}

bool TOutputGLSLBase::visitIfElse(Visit visit, TIntermIfElse *node)
{
    TInfoSinkBase &out = objSink();

    out << "if (";
    node->getCondition()->traverse(this);
    out << ")\n";

    visitCodeBlock(node->getTrueBlock());

    if (node->getFalseBlock())
    {
        out << getIndentPrefix() << "else\n";
        visitCodeBlock(node->getFalseBlock());
    }
    return false;
}

bool TOutputGLSLBase::visitSwitch(Visit visit, TIntermSwitch *node)
{
    ASSERT(node->getStatementList());
    writeTriplet(visit, "switch (", ") ", nullptr);
    // The curly braces get written when visiting the statementList aggregate
    return true;
}

bool TOutputGLSLBase::visitCase(Visit visit, TIntermCase *node)
{
    if (node->hasCondition())
    {
        writeTriplet(visit, "case (", nullptr, "):\n");
        return true;
    }
    else
    {
        TInfoSinkBase &out = objSink();
        out << "default:\n";
        return false;
    }
}

bool TOutputGLSLBase::visitBlock(Visit visit, TIntermBlock *node)
{
    TInfoSinkBase &out = objSink();
    // Scope the blocks except when at the global scope.
    if (getCurrentTraversalDepth() > 0)
    {
        out << "{\n";
    }

    for (TIntermSequence::const_iterator iter = node->getSequence()->begin();
         iter != node->getSequence()->end(); ++iter)
    {
        TIntermNode *curNode = *iter;
        ASSERT(curNode != nullptr);

        out << getIndentPrefix(curNode->getAsCaseNode() ? -1 : 0);

        curNode->traverse(this);

        if (isSingleStatement(curNode))
            out << ";\n";
    }

    // Scope the blocks except when at the global scope.
    if (getCurrentTraversalDepth() > 0)
    {
        out << getIndentPrefix(-1) << "}\n";
    }
    return false;
}

bool TOutputGLSLBase::visitFunctionDefinition(Visit visit, TIntermFunctionDefinition *node)
{
    TIntermFunctionPrototype *prototype = node->getFunctionPrototype();
    prototype->traverse(this);
    visitCodeBlock(node->getBody());

    // Fully processed; no need to visit children.
    return false;
}

bool TOutputGLSLBase::visitGlobalQualifierDeclaration(Visit visit,
                                                      TIntermGlobalQualifierDeclaration *node)
{
    TInfoSinkBase &out = objSink();
    ASSERT(visit == PreVisit);
    const TIntermSymbol *symbol = node->getSymbol();
    out << (node->isPrecise() ? "precise " : "invariant ") << hashName(&symbol->variable());
    return false;
}

void TOutputGLSLBase::visitFunctionPrototype(TIntermFunctionPrototype *node)
{
    TInfoSinkBase &out = objSink();

    const TType &type = node->getType();
    writeVariableType(type, node->getFunction(), false);
    if (type.isArray())
        out << ArrayString(type);

    out << " " << hashFunctionNameIfNeeded(node->getFunction());

    out << "(";
    writeFunctionParameters(node->getFunction());
    out << ")";
}

bool TOutputGLSLBase::visitAggregate(Visit visit, TIntermAggregate *node)
{
    bool visitChildren = true;
    if (node->getOp() == EOpConstruct)
    {
        writeConstructorTriplet(visit, node->getType());
    }
    else
    {
        // Function call.
        ImmutableString functionName = node->getFunction()->name();
        if (visit == PreVisit)
        {
            // No raw function is expected.
            ASSERT(node->getOp() != EOpCallInternalRawFunction);

            if (node->getOp() == EOpCallFunctionInAST)
            {
                functionName = hashFunctionNameIfNeeded(node->getFunction());
            }
            else
            {
                functionName =
                    translateTextureFunction(node->getFunction()->name(), mCompileOptions);
            }
        }
        writeFunctionTriplet(visit, functionName, node->getUseEmulatedFunction());
    }
    return visitChildren;
}

bool TOutputGLSLBase::visitDeclaration(Visit visit, TIntermDeclaration *node)
{
    TInfoSinkBase &out = objSink();

    // Variable declaration.
    if (visit == PreVisit)
    {
        const TIntermSequence &sequence = *(node->getSequence());
        TIntermTyped *decl              = sequence.front()->getAsTyped();
        TIntermSymbol *symbolNode       = decl->getAsSymbolNode();
        if (symbolNode == nullptr)
        {
            ASSERT(decl->getAsBinaryNode() && decl->getAsBinaryNode()->getOp() == EOpInitialize);
            symbolNode = decl->getAsBinaryNode()->getLeft()->getAsSymbolNode();
        }
        ASSERT(symbolNode);

        if (symbolNode->getName() != "gl_ClipDistance" &&
            symbolNode->getName() != "gl_CullDistance")
        {
            // gl_Clip/CullDistance re-declaration doesn't need layout.
            writeLayoutQualifier(symbolNode);
        }

        writeVariableType(symbolNode->getType(), &symbolNode->variable(), false);
        if (symbolNode->variable().symbolType() != SymbolType::Empty)
        {
            out << " ";
        }
        mDeclaringVariable = true;
    }
    else if (visit == InVisit)
    {
        UNREACHABLE();
    }
    else
    {
        mDeclaringVariable = false;
    }
    return true;
}

bool TOutputGLSLBase::visitLoop(Visit visit, TIntermLoop *node)
{
    TInfoSinkBase &out = objSink();

    TLoopType loopType = node->getType();

    if (loopType == ELoopFor)  // for loop
    {
        out << "for (";
        if (node->getInit())
            node->getInit()->traverse(this);
        out << "; ";

        if (node->getCondition())
            node->getCondition()->traverse(this);
        out << "; ";

        if (node->getExpression())
            node->getExpression()->traverse(this);
        out << ")\n";

        visitCodeBlock(node->getBody());
    }
    else if (loopType == ELoopWhile)  // while loop
    {
        out << "while (";
        ASSERT(node->getCondition() != nullptr);
        node->getCondition()->traverse(this);
        out << ")\n";

        visitCodeBlock(node->getBody());
    }
    else  // do-while loop
    {
        ASSERT(loopType == ELoopDoWhile);
        out << "do\n";

        visitCodeBlock(node->getBody());

        out << "while (";
        ASSERT(node->getCondition() != nullptr);
        node->getCondition()->traverse(this);
        out << ");\n";
    }

    // No need to visit children. They have been already processed in
    // this function.
    return false;
}

bool TOutputGLSLBase::visitBranch(Visit visit, TIntermBranch *node)
{
    switch (node->getFlowOp())
    {
        case EOpKill:
            writeTriplet(visit, "discard", nullptr, nullptr);
            break;
        case EOpBreak:
            writeTriplet(visit, "break", nullptr, nullptr);
            break;
        case EOpContinue:
            writeTriplet(visit, "continue", nullptr, nullptr);
            break;
        case EOpReturn:
            writeTriplet(visit, "return ", nullptr, nullptr);
            break;
        default:
            UNREACHABLE();
    }

    return true;
}

void TOutputGLSLBase::visitCodeBlock(TIntermBlock *node)
{
    TInfoSinkBase &out = objSink();
    if (node != nullptr)
    {
        out << getIndentPrefix();
        node->traverse(this);
        // Single statements not part of a sequence need to be terminated
        // with semi-colon.
        if (isSingleStatement(node))
            out << ";\n";
    }
    else
    {
        out << "{\n}\n";  // Empty code block.
    }
}

void TOutputGLSLBase::visitPreprocessorDirective(TIntermPreprocessorDirective *node)
{
    TInfoSinkBase &out = objSink();

    out << "\n";

    switch (node->getDirective())
    {
        case PreprocessorDirective::Define:
            out << "#define";
            break;
        case PreprocessorDirective::Endif:
            out << "#endif";
            break;
        case PreprocessorDirective::If:
            out << "#if";
            break;
        case PreprocessorDirective::Ifdef:
            out << "#ifdef";
            break;

        default:
            UNREACHABLE();
            break;
    }

    if (!node->getCommand().empty())
    {
        out << " " << node->getCommand();
    }

    out << "\n";
}

ImmutableString TOutputGLSLBase::getTypeName(const TType &type)
{
    if (type.getBasicType() == EbtSamplerVideoWEBGL)
    {
        // TODO(http://anglebug.com/42262534): translate SamplerVideoWEBGL into different token
        // when necessary (e.g. on Android devices)
        return ImmutableString("sampler2D");
    }

    return GetTypeName(type, mHashFunction, &mNameMap);
}

ImmutableString TOutputGLSLBase::hashName(const TSymbol *symbol)
{
    return HashName(symbol, mHashFunction, &mNameMap);
}

ImmutableString TOutputGLSLBase::hashFieldName(const TField *field)
{
    ASSERT(field->symbolType() != SymbolType::Empty);
    if (field->symbolType() == SymbolType::UserDefined)
    {
        return HashName(field->name(), mHashFunction, &mNameMap);
    }

    return field->name();
}

ImmutableString TOutputGLSLBase::hashFunctionNameIfNeeded(const TFunction *func)
{
    if (func->isMain())
    {
        return func->name();
    }
    else
    {
        return hashName(func);
    }
}

void TOutputGLSLBase::declareStruct(const TStructure *structure)
{
    TInfoSinkBase &out = objSink();

    out << "struct ";

    if (structure->symbolType() != SymbolType::Empty)
    {
        out << hashName(structure) << " ";
    }
    out << "{\n";
    const TFieldList &fields = structure->fields();
    for (size_t i = 0; i < fields.size(); ++i)
    {
        out << getIndentPrefix(1);
        const TField *field    = fields[i];
        const TType &fieldType = *field->type();
        if (writeVariablePrecision(fieldType.getPrecision()))
        {
            out << " ";
        }
        if (fieldType.isPrecise())
        {
            writePreciseQualifier(fieldType);
        }
        out << getTypeName(fieldType) << " " << hashFieldName(field);
        if (fieldType.isArray())
        {
            out << ArrayString(fieldType);
        }
        out << ";\n";
    }
    out << getIndentPrefix() << "}";
}

void TOutputGLSLBase::declareInterfaceBlockLayout(const TType &type)
{
    // 4.4.5 Uniform and Shader Storage Block Layout Qualifiers in GLSL 4.5 spec.
    // Layout qualifiers can be used for uniform and shader storage blocks,
    // but not for non-block uniform declarations.
    if (IsShaderIoBlock(type.getQualifier()))
    {
        return;
    }

    const TInterfaceBlock *interfaceBlock = type.getInterfaceBlock();
    TInfoSinkBase &out                    = objSink();

    out << "layout(";

    switch (interfaceBlock->blockStorage())
    {
        case EbsUnspecified:
        case EbsShared:
            // Default block storage is shared.
            out << "shared";
            break;

        case EbsPacked:
            out << "packed";
            break;

        case EbsStd140:
            out << "std140";
            break;

        case EbsStd430:
            out << "std430";
            break;

        default:
            UNREACHABLE();
            break;
    }

    if (interfaceBlock->blockBinding() >= 0)
    {
        out << ", ";
        out << "binding = " << interfaceBlock->blockBinding();
    }

    out << ") ";
}

const char *getVariableInterpolation(TQualifier qualifier)
{
    switch (qualifier)
    {
        case EvqSmoothOut:
            return "smooth out ";
        case EvqFlatOut:
            return "flat out ";
        case EvqNoPerspectiveOut:
            return "noperspective out ";
        case EvqCentroidOut:
            return "centroid out ";
        case EvqSampleOut:
            return "sample out ";
        case EvqNoPerspectiveCentroidOut:
            return "noperspective centroid out ";
        case EvqNoPerspectiveSampleOut:
            return "noperspective sample out ";
        case EvqSmoothIn:
            return "smooth in ";
        case EvqFlatIn:
            return "flat in ";
        case EvqNoPerspectiveIn:
            return "noperspective in ";
        case EvqCentroidIn:
            return "centroid in ";
        case EvqSampleIn:
            return "sample in ";
        case EvqNoPerspectiveCentroidIn:
            return "noperspective centroid in ";
        case EvqNoPerspectiveSampleIn:
            return "noperspective sample in ";
        default:
            break;
    }
    return nullptr;
}

void TOutputGLSLBase::declareInterfaceBlock(const TType &type)
{
    const TInterfaceBlock *interfaceBlock = type.getInterfaceBlock();
    TInfoSinkBase &out                    = objSink();

    out << hashName(interfaceBlock) << "{\n";
    const TFieldList &fields = interfaceBlock->fields();
    for (const TField *field : fields)
    {
        out << getIndentPrefix(1);
        if (!IsShaderIoBlock(type.getQualifier()) && type.getQualifier() != EvqPatchIn &&
            type.getQualifier() != EvqPatchOut)
        {
            writeFieldLayoutQualifier(field);
        }

        const TType &fieldType = *field->type();

        out << getMemoryQualifiers(fieldType);
        if (writeVariablePrecision(fieldType.getPrecision()))
            out << " ";
        if (fieldType.isInvariant())
        {
            writeInvariantQualifier(fieldType);
        }
        if (fieldType.isPrecise())
        {
            writePreciseQualifier(fieldType);
        }

        const char *qualifier = getVariableInterpolation(fieldType.getQualifier());
        if (qualifier != nullptr)
            out << qualifier;

        out << getTypeName(fieldType) << " " << hashFieldName(field);

        if (fieldType.isArray())
            out << ArrayString(fieldType);
        out << ";\n";
    }
    out << "}";
}

void WritePragma(TInfoSinkBase &out, const ShCompileOptions &compileOptions, const TPragma &pragma)
{
    if (!compileOptions.flattenPragmaSTDGLInvariantAll)
    {
        if (pragma.stdgl.invariantAll)
            out << "#pragma STDGL invariant(all)\n";
    }
}

void WriteGeometryShaderLayoutQualifiers(TInfoSinkBase &out,
                                         sh::TLayoutPrimitiveType inputPrimitive,
                                         int invocations,
                                         sh::TLayoutPrimitiveType outputPrimitive,
                                         int maxVertices)
{
    // Omit 'invocations = 1'
    if (inputPrimitive != EptUndefined || invocations > 1)
    {
        out << "layout (";

        if (inputPrimitive != EptUndefined)
        {
            out << getGeometryShaderPrimitiveTypeString(inputPrimitive);
        }

        if (invocations > 1)
        {
            if (inputPrimitive != EptUndefined)
            {
                out << ", ";
            }
            out << "invocations = " << invocations;
        }
        out << ") in;\n";
    }

    if (outputPrimitive != EptUndefined || maxVertices != -1)
    {
        out << "layout (";

        if (outputPrimitive != EptUndefined)
        {
            out << getGeometryShaderPrimitiveTypeString(outputPrimitive);
        }

        if (maxVertices != -1)
        {
            if (outputPrimitive != EptUndefined)
            {
                out << ", ";
            }
            out << "max_vertices = " << maxVertices;
        }
        out << ") out;\n";
    }
}

void WriteTessControlShaderLayoutQualifiers(TInfoSinkBase &out, int inputVertices)
{
    if (inputVertices != 0)
    {
        out << "layout (vertices = " << inputVertices << ") out;\n";
    }
}

void WriteTessEvaluationShaderLayoutQualifiers(TInfoSinkBase &out,
                                               sh::TLayoutTessEvaluationType inputPrimitive,
                                               sh::TLayoutTessEvaluationType inputVertexSpacing,
                                               sh::TLayoutTessEvaluationType inputOrdering,
                                               sh::TLayoutTessEvaluationType inputPoint)
{
    if (inputPrimitive != EtetUndefined)
    {
        out << "layout (";
        out << getTessEvaluationShaderTypeString(inputPrimitive);
        if (inputVertexSpacing != EtetUndefined)
        {
            out << ", " << getTessEvaluationShaderTypeString(inputVertexSpacing);
        }
        if (inputOrdering != EtetUndefined)
        {
            out << ", " << getTessEvaluationShaderTypeString(inputOrdering);
        }
        if (inputPoint != EtetUndefined)
        {
            out << ", " << getTessEvaluationShaderTypeString(inputPoint);
        }
        out << ") in;\n";
    }
}

void WriteFragmentShaderLayoutQualifiers(TInfoSinkBase &out,
                                         const AdvancedBlendEquations &advancedBlendEquations)
{
    if (advancedBlendEquations.any())
    {
        out << "layout (";

        const char *delimiter = "";
        auto emitQualifer     = [&](const char *qualifier) {
            out << delimiter << qualifier;
            delimiter = ", ";
        };

        if (advancedBlendEquations.all())
        {
            emitQualifer(AdvancedBlendEquations::GetAllEquationsLayoutString());
        }
        else
        {
            for (gl::BlendEquationType blendEquation :
                 gl::BlendEquationBitSet(advancedBlendEquations.bits()))
            {
                emitQualifer(
                    AdvancedBlendEquations::GetLayoutString(static_cast<uint32_t>(blendEquation)));
            }
        }

        out << ") out;\n";
    }
}

// If SH_SCALARIZE_VEC_AND_MAT_CONSTRUCTOR_ARGS is enabled, layout qualifiers are spilled whenever
// variables with specified layout qualifiers are copied. Additional checks are needed against the
// type and storage qualifier of the variable to verify that layout qualifiers have to be outputted.
// TODO (mradev): Fix layout qualifier spilling in ScalarizeVecAndMatConstructorArgs and remove
// NeedsToWriteLayoutQualifier.
bool TOutputGLSLBase::needsToWriteLayoutQualifier(const TType &type)
{
    const TLayoutQualifier &layoutQualifier = type.getLayoutQualifier();

    if (type.getBasicType() == EbtInterfaceBlock)
    {
        if (type.getQualifier() == EvqPixelLocalEXT)
        {
            // We only use per-member EXT_shader_pixel_local_storage formats, so the PLS interface
            // block will never have a layout qualifier.
            ASSERT(layoutQualifier.imageInternalFormat == EiifUnspecified);
            return false;
        }
        return true;
    }

    if (IsFragmentOutput(type.getQualifier()) || type.getQualifier() == EvqVertexIn ||
        IsVarying(type.getQualifier()))
    {
        if (layoutQualifier.location >= 0 ||
            (mAlwaysSpecifyFragOutLocation && IsFragmentOutput(type.getQualifier()) &&
             !layoutQualifier.yuv))
        {
            return true;
        }
    }

    if (type.getQualifier() == EvqFragDepth && layoutQualifier.depth != EdUnspecified)
    {
        return true;
    }

    if (type.getQualifier() == EvqFragmentOut || type.getQualifier() == EvqFragmentInOut)
    {
        if (layoutQualifier.index >= 0)
        {
            return true;
        }
        if (layoutQualifier.yuv)
        {
            return true;
        }
    }

    if (type.getQualifier() == EvqFragmentInOut && layoutQualifier.noncoherent)
    {
        return true;
    }

    if (IsOpaqueType(type.getBasicType()) && layoutQualifier.binding != -1)
    {
        return true;
    }

    if (IsImage(type.getBasicType()) && layoutQualifier.imageInternalFormat != EiifUnspecified)
    {
        return true;
    }
    return false;
}

void EmitEarlyFragmentTestsGLSL(const TCompiler &compiler, TInfoSinkBase &sink)
{
    if (compiler.isEarlyFragmentTestsSpecified())
    {
        sink << "layout (early_fragment_tests) in;\n";
    }
}

void EmitWorkGroupSizeGLSL(const TCompiler &compiler, TInfoSinkBase &sink)
{
    if (compiler.isComputeShaderLocalSizeDeclared())
    {
        const sh::WorkGroupSize &localSize = compiler.getComputeShaderLocalSize();
        sink << "layout (local_size_x=" << localSize[0] << ", local_size_y=" << localSize[1]
             << ", local_size_z=" << localSize[2] << ") in;\n";
    }
}

void EmitMultiviewGLSL(const TCompiler &compiler,
                       const ShCompileOptions &compileOptions,
                       const TExtension extension,
                       const TBehavior behavior,
                       TInfoSinkBase &sink)
{
    ASSERT(behavior != EBhUndefined);
    if (behavior == EBhDisable)
        return;

    const bool isVertexShader = (compiler.getShaderType() == GL_VERTEX_SHADER);
    if (compileOptions.initializeBuiltinsForInstancedMultiview)
    {
        // Emit ARB_shader_viewport_layer_array/NV_viewport_array2 in a vertex shader if the
        // SH_SELECT_VIEW_IN_NV_GLSL_VERTEX_SHADER option is set and the
        // OVR_multiview(2) extension is requested.
        if (isVertexShader && compileOptions.selectViewInNvGLSLVertexShader)
        {
            sink << "#if defined(GL_ARB_shader_viewport_layer_array)\n"
                 << "#extension GL_ARB_shader_viewport_layer_array : require\n"
                 << "#elif defined(GL_NV_viewport_array2)\n"
                 << "#extension GL_NV_viewport_array2 : require\n"
                 << "#endif\n";
        }
    }
    else
    {
        sink << "#extension GL_OVR_multiview";
        if (extension == TExtension::OVR_multiview2)
        {
            sink << "2";
        }
        sink << " : " << GetBehaviorString(behavior) << "\n";

        const auto &numViews = compiler.getNumViews();
        if (isVertexShader && numViews != -1)
        {
            sink << "layout(num_views=" << numViews << ") in;\n";
        }
    }
}

}  // namespace sh
