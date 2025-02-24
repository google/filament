//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_GLSL_OUTPUTGLSLBASE_H_
#define COMPILER_TRANSLATOR_GLSL_OUTPUTGLSLBASE_H_

#include <set>

#include "compiler/translator/ExtensionBehavior.h"
#include "compiler/translator/HashNames.h"
#include "compiler/translator/InfoSink.h"
#include "compiler/translator/Pragma.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{
class TCompiler;

class TOutputGLSLBase : public TIntermTraverser
{
  public:
    TOutputGLSLBase(TCompiler *compiler,
                    TInfoSinkBase &objSink,
                    const ShCompileOptions &compileOptions);

    ShShaderOutput getShaderOutput() const { return mOutput; }

    // Return the original name if hash function pointer is NULL;
    // otherwise return the hashed name. Has special handling for internal names and built-ins,
    // which are not hashed.
    ImmutableString hashName(const TSymbol *symbol);

  protected:
    TInfoSinkBase &objSink() { return mObjSink; }
    void writeFloat(TInfoSinkBase &out, float f);
    void writeTriplet(Visit visit, const char *preStr, const char *inStr, const char *postStr);
    std::string getCommonLayoutQualifiers(TIntermSymbol *variable);
    std::string getMemoryQualifiers(const TType &type);
    virtual void writeLayoutQualifier(TIntermSymbol *variable);
    void writeFieldLayoutQualifier(const TField *field);
    void writeInvariantQualifier(const TType &type);
    void writePreciseQualifier(const TType &type);
    virtual void writeVariableType(const TType &type,
                                   const TSymbol *symbol,
                                   bool isFunctionArgument);
    virtual bool writeVariablePrecision(TPrecision precision) = 0;
    void writeFunctionParameters(const TFunction *func);
    const TConstantUnion *writeConstantUnion(const TType &type, const TConstantUnion *pConstUnion);
    void writeConstructorTriplet(Visit visit, const TType &type);
    ImmutableString getTypeName(const TType &type);

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

    void visitCodeBlock(TIntermBlock *node);

    ImmutableString hashFieldName(const TField *field);
    // Same as hashName(), but without hashing "main".
    ImmutableString hashFunctionNameIfNeeded(const TFunction *func);
    // Used to translate function names for differences between ESSL and GLSL
    virtual ImmutableString translateTextureFunction(const ImmutableString &name,
                                                     const ShCompileOptions &option)
    {
        return name;
    }

    void declareStruct(const TStructure *structure);
    void writeQualifier(TQualifier qualifier, const TType &type, const TSymbol *symbol);

    const char *mapQualifierToString(TQualifier qualifier);

    sh::GLenum getShaderType() const { return mShaderType; }
    bool isHighPrecisionSupported() const { return mHighPrecisionSupported; }
    const char *getIndentPrefix(int extraIndentDepth = 0);

    bool needsToWriteLayoutQualifier(const TType &type);

  private:
    void declareInterfaceBlockLayout(const TType &type);
    void declareInterfaceBlock(const TType &type);

    void writeFunctionTriplet(Visit visit,
                              const ImmutableString &functionName,
                              bool useEmulatedFunction);

    TInfoSinkBase &mObjSink;
    bool mDeclaringVariable;

    // name hashing.
    ShHashFunction64 mHashFunction;
    NameMap &mNameMap;

    sh::GLenum mShaderType;
    const int mShaderVersion;
    ShShaderOutput mOutput;

    bool mHighPrecisionSupported;

    // Emit "layout(locaton = 0)" for fragment outputs whose location is unspecified. This is for
    // transformations like pixel local storage, where new outputs are introduced to the shader, and
    // previously valid fragment outputs with an implicit location of 0 are now required to specify
    // their location.
    bool mAlwaysSpecifyFragOutLocation;

    const ShCompileOptions &mCompileOptions;
};

void WritePragma(TInfoSinkBase &out, const ShCompileOptions &compileOptions, const TPragma &pragma);

void WriteGeometryShaderLayoutQualifiers(TInfoSinkBase &out,
                                         sh::TLayoutPrimitiveType inputPrimitive,
                                         int invocations,
                                         sh::TLayoutPrimitiveType outputPrimitive,
                                         int maxVertices);

void WriteTessControlShaderLayoutQualifiers(TInfoSinkBase &out, int inputVertices);

void WriteTessEvaluationShaderLayoutQualifiers(TInfoSinkBase &out,
                                               sh::TLayoutTessEvaluationType inputPrimitive,
                                               sh::TLayoutTessEvaluationType inputVertexSpacing,
                                               sh::TLayoutTessEvaluationType inputOrdering,
                                               sh::TLayoutTessEvaluationType inputPoint);

void WriteFragmentShaderLayoutQualifiers(TInfoSinkBase &out,
                                         const AdvancedBlendEquations &advancedBlendEquations);

void EmitEarlyFragmentTestsGLSL(const TCompiler &, TInfoSinkBase &sink);
void EmitWorkGroupSizeGLSL(const TCompiler &, TInfoSinkBase &sink);
void EmitMultiviewGLSL(const TCompiler &,
                       const ShCompileOptions &,
                       const TExtension,
                       const TBehavior,
                       TInfoSinkBase &sink);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_GLSL_OUTPUTGLSLBASE_H_
