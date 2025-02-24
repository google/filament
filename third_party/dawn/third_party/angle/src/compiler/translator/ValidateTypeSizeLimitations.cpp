//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/ValidateTypeSizeLimitations.h"

#include "angle_gl.h"
#include "common/mathutil.h"
#include "common/span.h"
#include "compiler/translator/Diagnostics.h"
#include "compiler/translator/Symbol.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/blocklayout.h"
#include "compiler/translator/tree_util/IntermTraverse.h"
#include "compiler/translator/util.h"

namespace sh
{

namespace
{

// Arbitrarily enforce that all types declared with a size in bytes of over 2 GB will cause
// compilation failure.
//
// For local and global variables, the limit is much lower (64KB) as that much memory won't fit in
// the GPU registers anyway.
constexpr size_t kMaxVariableSizeInBytes             = static_cast<size_t>(2) * 1024 * 1024 * 1024;
constexpr size_t kMaxPrivateVariableSizeInBytes      = static_cast<size_t>(64) * 1024;
constexpr size_t kMaxTotalPrivateVariableSizeInBytes = static_cast<size_t>(16) * 1024 * 1024;

// Traverses intermediate tree to ensure that the shader does not
// exceed certain implementation-defined limits on the sizes of types.
// Some code was copied from the CollectVariables pass.
class ValidateTypeSizeLimitationsTraverser : public TIntermTraverser
{
  public:
    ValidateTypeSizeLimitationsTraverser(TSymbolTable *symbolTable, TDiagnostics *diagnostics)
        : TIntermTraverser(true, false, false, symbolTable),
          mDiagnostics(diagnostics),
          mTotalPrivateVariablesSize(0)
    {
        ASSERT(diagnostics);
    }

    bool visitDeclaration(Visit visit, TIntermDeclaration *node) override
    {
        const TIntermSequence &sequence = *(node->getSequence());

        for (TIntermNode *variableNode : sequence)
        {
            // See CollectVariablesTraverser::visitDeclaration for a
            // deeper analysis of the AST structures that might be
            // encountered.
            TIntermSymbol *asSymbol = variableNode->getAsSymbolNode();
            TIntermBinary *asBinary = variableNode->getAsBinaryNode();

            if (asBinary != nullptr)
            {
                ASSERT(asBinary->getOp() == EOpInitialize);
                asSymbol = asBinary->getLeft()->getAsSymbolNode();
            }

            ASSERT(asSymbol);

            const TVariable &variable = asSymbol->variable();
            if (variable.symbolType() == SymbolType::AngleInternal)
            {
                // Ignore internal variables.
                continue;
            }

            if (!validateVariableSize(variable, asSymbol->getLine()))
            {
                return false;
            }
        }

        return true;
    }

    void visitFunctionPrototype(TIntermFunctionPrototype *node) override
    {
        const TFunction *function = node->getFunction();
        const size_t paramCount   = function->getParamCount();

        for (size_t paramIndex = 0; paramIndex < paramCount; ++paramIndex)
        {
            validateVariableSize(*function->getParam(paramIndex), node->getLine());
        }
    }

    bool validateVariableSize(const TVariable &variable, const TSourceLoc &location)
    {
        const TType &variableType = variable.getType();

        // Create a ShaderVariable from which to compute
        // (conservative) sizing information.
        ShaderVariable shaderVar;
        setCommonVariableProperties(variableType, variable, &shaderVar);

        size_t variableSize;
        {
            // Compute the std140 layout of this variable, assuming
            // it's a member of a block (which it might not be).
            Std140BlockEncoder layoutEncoder;
            BlockEncoderVisitor visitor("", "", &layoutEncoder);
            // Since the size limit's arbitrary, it doesn't matter
            // whether the row-major layout is correctly determined.
            constexpr bool isRowMajorLayout = false;

            // For efficiency, don't actually iterate over all array elements, as only the size
            // calculation matters.  Instead, for arrays, the size is reduced to 2 (so array-ness
            // and padding is taken into account), and the total size is derived from the array
            // size.
            const uint32_t arraySizeProduct =
                shaderVar.isArray() ? shaderVar.getArraySizeProduct() : 1;
            if (arraySizeProduct > 1)
            {
                shaderVar.arraySizes.resize(1);
                shaderVar.arraySizes[0] = 2;
            }

            TraverseShaderVariable(shaderVar, isRowMajorLayout, &visitor);
            variableSize = layoutEncoder.getCurrentOffset();

            if (arraySizeProduct > 1)
            {
                // Calculate the actual size of the variable.
                ASSERT(variableSize % 2 == 0);
                variableSize = variableSize / 2 * arraySizeProduct;
            }
        }

        if (variableSize > kMaxVariableSizeInBytes)
        {
            error(location, "Size of declared variable exceeds implementation-defined limit",
                  variable.name());
            return false;
        }

        // Skip over struct declarations.  As long as they are not used (or if they are used later
        // in a less-restricted context (such as a UBO or SSBO)), they can be larger than
        // kMaxPrivateVariableSizeInBytes.
        if (variable.symbolType() == SymbolType::Empty && variableType.isStructSpecifier())
        {
            return true;
        }

        switch (variableType.getQualifier())
        {
            // List of all types that need to be limited (for example because they cause overflows
            // in drivers, or create trouble for the SPIR-V gen as the number of an instruction's
            // arguments cannot be more than 64KB (see OutputSPIRVTraverser::cast)).

            // Local/global variables
            case EvqTemporary:
            case EvqGlobal:
            case EvqConst:

            // Function arguments
            case EvqParamIn:
            case EvqParamOut:
            case EvqParamInOut:
            case EvqParamConst:

            // Varyings
            case EvqVaryingIn:
            case EvqVaryingOut:
            case EvqSmoothOut:
            case EvqFlatOut:
            case EvqNoPerspectiveOut:
            case EvqCentroidOut:
            case EvqSampleOut:
            case EvqNoPerspectiveCentroidOut:
            case EvqNoPerspectiveSampleOut:
            case EvqSmoothIn:
            case EvqFlatIn:
            case EvqNoPerspectiveIn:
            case EvqCentroidIn:
            case EvqNoPerspectiveCentroidIn:
            case EvqNoPerspectiveSampleIn:
            case EvqVertexOut:
            case EvqFragmentIn:
            case EvqGeometryIn:
            case EvqGeometryOut:
            case EvqPerVertexIn:
            case EvqPerVertexOut:
            case EvqPatchIn:
            case EvqPatchOut:
            case EvqTessControlIn:
            case EvqTessControlOut:
            case EvqTessEvaluationIn:
            case EvqTessEvaluationOut:

                if (variableSize > kMaxPrivateVariableSizeInBytes)
                {
                    error(location,
                          "Size of declared private variable exceeds implementation-defined limit",
                          variable.name());
                    return false;
                }
                mTotalPrivateVariablesSize += variableSize;
                break;
            default:
                break;
        }

        return true;
    }

    void validateTotalPrivateVariableSize()
    {
        if (mTotalPrivateVariablesSize.ValueOrDefault(std::numeric_limits<size_t>::max()) >
            kMaxTotalPrivateVariableSizeInBytes)
        {
            mDiagnostics->error(
                TSourceLoc{},
                "Total size of declared private variables exceeds implementation-defined limit",
                "");
        }
    }

  private:
    void error(TSourceLoc loc, const char *reason, const ImmutableString &token)
    {
        mDiagnostics->error(loc, reason, token.data());
    }

    void setFieldOrVariableProperties(const TType &type,
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
            if (structure->symbolType() != SymbolType::Empty)
            {
                variableOut->structOrBlockName = structure->name().data();
            }

            const TFieldList &fields = structure->fields();

            for (const TField *field : fields)
            {
                // Regardless of the variable type (uniform, in/out etc.) its fields are always
                // plain ShaderVariable objects.
                ShaderVariable fieldVariable;
                setFieldProperties(*field->type(), field->name(), staticUse, isShaderIOBlock,
                                   isPatch, &fieldVariable);
                variableOut->fields.push_back(fieldVariable);
            }
        }
        else if (interfaceBlock && isShaderIOBlock)
        {
            variableOut->type = GL_NONE;
            if (interfaceBlock->symbolType() != SymbolType::Empty)
            {
                variableOut->structOrBlockName = interfaceBlock->name().data();
            }
            const TFieldList &fields = interfaceBlock->fields();
            for (const TField *field : fields)
            {
                ShaderVariable fieldVariable;
                setFieldProperties(*field->type(), field->name(), staticUse, true, isPatch,
                                   &fieldVariable);
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
            // WebGL does not support tessellation shaders; removed
            // code specific to that shader type.
        }
    }

    void setFieldProperties(const TType &type,
                            const ImmutableString &name,
                            bool staticUse,
                            bool isShaderIOBlock,
                            bool isPatch,
                            ShaderVariable *variableOut) const
    {
        ASSERT(variableOut);
        setFieldOrVariableProperties(type, staticUse, isShaderIOBlock, isPatch, variableOut);
        variableOut->name.assign(name.data(), name.length());
    }

    void setCommonVariableProperties(const TType &type,
                                     const TVariable &variable,
                                     ShaderVariable *variableOut) const
    {
        ASSERT(variableOut);

        // Shortcut some processing that's unnecessary for this analysis.
        const bool staticUse       = true;
        const bool isShaderIOBlock = type.getInterfaceBlock() != nullptr;
        const bool isPatch         = false;

        setFieldOrVariableProperties(type, staticUse, isShaderIOBlock, isPatch, variableOut);

        const bool isNamed = variable.symbolType() != SymbolType::Empty;

        if (isNamed)
        {
            variableOut->name.assign(variable.name().data(), variable.name().length());
        }
    }

    TDiagnostics *mDiagnostics;
    std::vector<int> mLoopSymbolIds;

    angle::base::CheckedNumeric<size_t> mTotalPrivateVariablesSize;
};

}  // namespace

bool ValidateTypeSizeLimitations(TIntermNode *root,
                                 TSymbolTable *symbolTable,
                                 TDiagnostics *diagnostics)
{
    ValidateTypeSizeLimitationsTraverser validate(symbolTable, diagnostics);
    root->traverse(&validate);
    validate.validateTotalPrivateVariableSize();
    return diagnostics->numErrors() == 0;
}

}  // namespace sh
