//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// InitOutputVariables_test.cpp: Tests correctness of the AST pass enabled through
// SH_INIT_OUTPUT_VARIABLES.
//

#include "common/angleutils.h"

#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_util/FindMain.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"
#include "tests/test_utils/ShaderCompileTreeTest.h"

#include <algorithm>

namespace sh
{

namespace
{

typedef std::vector<TIntermTyped *> ExpectedLValues;

bool AreSymbolsTheSame(const TIntermSymbol *expected, const TIntermSymbol *candidate)
{
    if (expected == nullptr || candidate == nullptr)
    {
        return false;
    }
    const TType &expectedType  = expected->getType();
    const TType &candidateType = candidate->getType();
    const bool sameTypes       = expectedType == candidateType &&
                           expectedType.getPrecision() == candidateType.getPrecision() &&
                           expectedType.getQualifier() == candidateType.getQualifier();
    const bool sameSymbols = (expected->variable().symbolType() == SymbolType::Empty &&
                              candidate->variable().symbolType() == SymbolType::Empty) ||
                             expected->getName() == candidate->getName();
    return sameSymbols && sameTypes;
}

bool AreLValuesTheSame(TIntermTyped *expected, TIntermTyped *candidate)
{
    const TIntermBinary *expectedBinary = expected->getAsBinaryNode();
    if (expectedBinary)
    {
        ASSERT(expectedBinary->getOp() == EOpIndexDirect);
        const TIntermBinary *candidateBinary = candidate->getAsBinaryNode();
        if (candidateBinary == nullptr || candidateBinary->getOp() != EOpIndexDirect)
        {
            return false;
        }
        if (expectedBinary->getRight()->getAsConstantUnion()->getIConst(0) !=
            candidateBinary->getRight()->getAsConstantUnion()->getIConst(0))
        {
            return false;
        }
        return AreSymbolsTheSame(expectedBinary->getLeft()->getAsSymbolNode(),
                                 candidateBinary->getLeft()->getAsSymbolNode());
    }
    return AreSymbolsTheSame(expected->getAsSymbolNode(), candidate->getAsSymbolNode());
}

TIntermTyped *CreateLValueNode(const ImmutableString &lValueName, const TType &type)
{
    // We're using a mock symbol table here, don't need to assign proper symbol ids to these nodes.
    TSymbolTable symbolTable;
    TVariable *variable =
        new TVariable(&symbolTable, lValueName, new TType(type), SymbolType::UserDefined);
    return new TIntermSymbol(variable);
}

ExpectedLValues CreateIndexedLValueNodeList(const ImmutableString &lValueName,
                                            const TType &elementType,
                                            unsigned arraySize)
{
    ASSERT(elementType.isArray() == false);
    TType *arrayType = new TType(elementType);
    arrayType->makeArray(arraySize);

    // We're using a mock symbol table here, don't need to assign proper symbol ids to these nodes.
    TSymbolTable symbolTable;
    TVariable *variable =
        new TVariable(&symbolTable, lValueName, arrayType, SymbolType::UserDefined);
    TIntermSymbol *arraySymbol = new TIntermSymbol(variable);

    ExpectedLValues expected(arraySize);
    for (unsigned index = 0u; index < arraySize; ++index)
    {
        expected[index] = new TIntermBinary(EOpIndexDirect, arraySymbol->deepCopy(),
                                            CreateIndexNode(static_cast<int>(index)));
    }
    return expected;
}

// VerifyOutputVariableInitializers traverses the subtree covering main and collects the lvalues in
// assignments for which the rvalue is an expression containing only zero constants.
class VerifyOutputVariableInitializers final : public TIntermTraverser
{
  public:
    VerifyOutputVariableInitializers(TIntermBlock *root) : TIntermTraverser(true, false, false)
    {
        ASSERT(root != nullptr);

        // The traversal starts in the body of main because this is where the varyings and output
        // variables are initialized.
        sh::TIntermFunctionDefinition *main = FindMain(root);
        ASSERT(main != nullptr);
        main->traverse(this);
    }

    bool visitBinary(Visit visit, TIntermBinary *node) override
    {
        if (node->getOp() == EOpAssign && IsZero(node->getRight()))
        {
            mCandidateLValues.push_back(node->getLeft());
            return false;
        }
        return true;
    }

    // The collected lvalues are considered valid if every expected lvalue in expectedLValues is
    // matched by name and type with any lvalue in mCandidateLValues.
    bool areAllExpectedLValuesFound(const ExpectedLValues &expectedLValues) const
    {
        for (size_t i = 0u; i < expectedLValues.size(); ++i)
        {
            if (!isExpectedLValueFound(expectedLValues[i]))
            {
                return false;
            }
        }
        return true;
    }

    bool isExpectedLValueFound(TIntermTyped *expectedLValue) const
    {
        bool isFound = false;
        for (size_t j = 0; j < mCandidateLValues.size() && !isFound; ++j)
        {
            isFound = AreLValuesTheSame(expectedLValue, mCandidateLValues[j]);
        }
        return isFound;
    }

    const ExpectedLValues &getCandidates() const { return mCandidateLValues; }

  private:
    ExpectedLValues mCandidateLValues;
};

// Traverses the AST and records a pointer to a structure with a given name.
class FindStructByName final : public TIntermTraverser
{
  public:
    FindStructByName(const ImmutableString &structName)
        : TIntermTraverser(true, false, false), mStructName(structName), mStructure(nullptr)
    {}

    void visitSymbol(TIntermSymbol *symbol) override
    {
        if (isStructureFound())
        {
            return;
        }

        const TStructure *structure = symbol->getType().getStruct();

        if (structure != nullptr && structure->symbolType() != SymbolType::Empty &&
            structure->name() == mStructName)
        {
            mStructure = structure;
        }
    }

    bool isStructureFound() const { return mStructure != nullptr; }
    const TStructure *getStructure() const { return mStructure; }

  private:
    ImmutableString mStructName;
    const TStructure *mStructure;
};

}  // namespace

class InitOutputVariablesWebGL2Test : public ShaderCompileTreeTest
{
  public:
    void SetUp() override
    {
        mCompileOptions.initOutputVariables = true;
        if (getShaderType() == GL_VERTEX_SHADER)
        {
            mCompileOptions.initGLPosition = true;
        }
        ShaderCompileTreeTest::SetUp();
    }

  protected:
    ShShaderSpec getShaderSpec() const override { return SH_WEBGL2_SPEC; }
};

class InitOutputVariablesWebGL2VertexShaderTest : public InitOutputVariablesWebGL2Test
{
  protected:
    ::GLenum getShaderType() const override { return GL_VERTEX_SHADER; }
};

class InitOutputVariablesWebGL2FragmentShaderTest : public InitOutputVariablesWebGL2Test
{
  protected:
    ::GLenum getShaderType() const override { return GL_FRAGMENT_SHADER; }
    void initResources(ShBuiltInResources *resources) override
    {
        resources->EXT_draw_buffers = 1;
        resources->MaxDrawBuffers   = 2;
    }
};

class InitOutputVariablesWebGL1FragmentShaderTest : public ShaderCompileTreeTest
{
  public:
    InitOutputVariablesWebGL1FragmentShaderTest() { mCompileOptions.initOutputVariables = true; }

  protected:
    ::GLenum getShaderType() const override { return GL_FRAGMENT_SHADER; }
    ShShaderSpec getShaderSpec() const override { return SH_WEBGL_SPEC; }
    void initResources(ShBuiltInResources *resources) override
    {
        resources->EXT_draw_buffers = 1;
        resources->MaxDrawBuffers   = 2;
    }
};

class InitOutputVariablesVertexShaderClipDistanceTest : public ShaderCompileTreeTest
{
  public:
    InitOutputVariablesVertexShaderClipDistanceTest()
    {
        mCompileOptions.initOutputVariables = true;
        mCompileOptions.validateAST         = true;
    }

  protected:
    ::GLenum getShaderType() const override { return GL_VERTEX_SHADER; }
    ShShaderSpec getShaderSpec() const override { return SH_GLES2_SPEC; }
    void initResources(ShBuiltInResources *resources) override
    {
        resources->APPLE_clip_distance = 1;
        resources->MaxClipDistances    = 8;
    }
};

// Test the initialization of output variables with various qualifiers in a vertex shader.
TEST_F(InitOutputVariablesWebGL2VertexShaderTest, OutputAllQualifiers)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "precision lowp int;\n"
        "out vec4 out1;\n"
        "flat out int out2;\n"
        "centroid out float out3;\n"
        "smooth out float out4;\n"
        "void main() {\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    VerifyOutputVariableInitializers verifier(mASTRoot);

    ExpectedLValues expectedLValues = {
        CreateLValueNode(ImmutableString("out1"), TType(EbtFloat, EbpMedium, EvqVertexOut, 4)),
        CreateLValueNode(ImmutableString("out2"), TType(EbtInt, EbpLow, EvqFlatOut)),
        CreateLValueNode(ImmutableString("out3"), TType(EbtFloat, EbpMedium, EvqCentroidOut)),
        CreateLValueNode(ImmutableString("out4"), TType(EbtFloat, EbpMedium, EvqSmoothOut))};
    EXPECT_TRUE(verifier.areAllExpectedLValuesFound(expectedLValues));
}

// Test the initialization of an output array in a vertex shader.
TEST_F(InitOutputVariablesWebGL2VertexShaderTest, OutputArray)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out float out1[2];\n"
        "void main() {\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    VerifyOutputVariableInitializers verifier(mASTRoot);

    ExpectedLValues expectedLValues = CreateIndexedLValueNodeList(
        ImmutableString("out1"), TType(EbtFloat, EbpMedium, EvqVertexOut), 2);
    EXPECT_TRUE(verifier.areAllExpectedLValuesFound(expectedLValues));
}

// Test the initialization of a struct output variable in a vertex shader.
TEST_F(InitOutputVariablesWebGL2VertexShaderTest, OutputStruct)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "struct MyS{\n"
        "   float a;\n"
        "   float b;\n"
        "};\n"
        "out MyS out1;\n"
        "void main() {\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    VerifyOutputVariableInitializers verifier(mASTRoot);

    FindStructByName findStruct(ImmutableString("MyS"));
    mASTRoot->traverse(&findStruct);
    ASSERT(findStruct.isStructureFound());

    TType type(findStruct.getStructure(), false);
    type.setQualifier(EvqVertexOut);

    TIntermTyped *expectedLValue = CreateLValueNode(ImmutableString("out1"), type);
    EXPECT_TRUE(verifier.isExpectedLValueFound(expectedLValue));
    delete expectedLValue;
}

// Test the initialization of a varying variable in an ESSL1 vertex shader.
TEST_F(InitOutputVariablesWebGL2VertexShaderTest, OutputFromESSL1Shader)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "varying vec4 out1;\n"
        "void main() {\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    VerifyOutputVariableInitializers verifier(mASTRoot);

    TIntermTyped *expectedLValue =
        CreateLValueNode(ImmutableString("out1"), TType(EbtFloat, EbpMedium, EvqVaryingOut, 4));
    EXPECT_TRUE(verifier.isExpectedLValueFound(expectedLValue));
    delete expectedLValue;
}

// Test the initialization of output variables in a fragment shader.
TEST_F(InitOutputVariablesWebGL2FragmentShaderTest, Output)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 out1;\n"
        "void main() {\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    VerifyOutputVariableInitializers verifier(mASTRoot);

    TIntermTyped *expectedLValue =
        CreateLValueNode(ImmutableString("out1"), TType(EbtFloat, EbpMedium, EvqFragmentOut, 4));
    EXPECT_TRUE(verifier.isExpectedLValueFound(expectedLValue));
    delete expectedLValue;
}

// Test the initialization of gl_FragData in a WebGL2 ESSL1 fragment shader. Only writes to
// gl_FragData[0] should be found.
TEST_F(InitOutputVariablesWebGL2FragmentShaderTest, FragData)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "void main() {\n"
        "   gl_FragData[0] = vec4(1.);\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    VerifyOutputVariableInitializers verifier(mASTRoot);

    ExpectedLValues expectedLValues = CreateIndexedLValueNodeList(
        ImmutableString("gl_FragData"), TType(EbtFloat, EbpMedium, EvqFragData, 4), 1);
    EXPECT_TRUE(verifier.isExpectedLValueFound(expectedLValues[0]));
    EXPECT_EQ(1u, verifier.getCandidates().size());
}

// Test the initialization of gl_FragData in a WebGL1 ESSL1 fragment shader. Only writes to
// gl_FragData[0] should be found.
TEST_F(InitOutputVariablesWebGL1FragmentShaderTest, FragData)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "void main() {\n"
        "   gl_FragData[0] = vec4(1.);\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    VerifyOutputVariableInitializers verifier(mASTRoot);

    // In the symbol table, gl_FragData array has 2 elements. However, only the 1st one should be
    // initialized.
    ExpectedLValues expectedLValues = CreateIndexedLValueNodeList(
        ImmutableString("gl_FragData"), TType(EbtFloat, EbpMedium, EvqFragData, 4), 2);
    EXPECT_TRUE(verifier.isExpectedLValueFound(expectedLValues[0]));
    EXPECT_EQ(1u, verifier.getCandidates().size());
}

// Test the initialization of gl_FragData in a WebGL1 ESSL1 fragment shader with GL_EXT_draw_buffers
// enabled. All attachment slots should be initialized.
TEST_F(InitOutputVariablesWebGL1FragmentShaderTest, FragDataWithDrawBuffersExtEnabled)
{
    const std::string &shaderString =
        "#extension GL_EXT_draw_buffers : enable\n"
        "precision mediump float;\n"
        "void main() {\n"
        "   gl_FragData[0] = vec4(1.);\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    VerifyOutputVariableInitializers verifier(mASTRoot);

    ExpectedLValues expectedLValues = CreateIndexedLValueNodeList(
        ImmutableString("gl_FragData"), TType(EbtFloat, EbpMedium, EvqFragData, 4), 2);
    EXPECT_TRUE(verifier.isExpectedLValueFound(expectedLValues[0]));
    EXPECT_TRUE(verifier.isExpectedLValueFound(expectedLValues[1]));
    EXPECT_EQ(2u, verifier.getCandidates().size());
}

// Test that gl_Position is initialized once in case it is not statically used and both
// SH_INIT_OUTPUT_VARIABLES and SH_INIT_GL_POSITION flags are set.
TEST_F(InitOutputVariablesWebGL2VertexShaderTest, InitGLPositionWhenNotStaticallyUsed)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision highp float;\n"
        "void main() {\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    VerifyOutputVariableInitializers verifier(mASTRoot);

    TIntermTyped *glPosition =
        CreateLValueNode(ImmutableString("gl_Position"), TType(EbtFloat, EbpHigh, EvqPosition, 4));
    EXPECT_TRUE(verifier.isExpectedLValueFound(glPosition));
    EXPECT_EQ(1u, verifier.getCandidates().size());
}

// Test that gl_Position is initialized once in case it is statically used and both
// SH_INIT_OUTPUT_VARIABLES and SH_INIT_GL_POSITION flags are set.
TEST_F(InitOutputVariablesWebGL2VertexShaderTest, InitGLPositionOnceWhenStaticallyUsed)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision highp float;\n"
        "void main() {\n"
        "    gl_Position = vec4(1.0);\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    VerifyOutputVariableInitializers verifier(mASTRoot);

    TIntermTyped *glPosition =
        CreateLValueNode(ImmutableString("gl_Position"), TType(EbtFloat, EbpHigh, EvqPosition, 4));
    EXPECT_TRUE(verifier.isExpectedLValueFound(glPosition));
    EXPECT_EQ(1u, verifier.getCandidates().size());
}

// Mirrors ClipDistanceTest.ThreeClipDistancesRedeclared
TEST_F(InitOutputVariablesVertexShaderClipDistanceTest, RedeclareClipDistance)
{
    constexpr char shaderString[] = R"(
#extension GL_APPLE_clip_distance : require

varying highp float gl_ClipDistance[3];

void computeClipDistances(in vec4 position, in vec4 plane[3])
{
    gl_ClipDistance[0] = dot(position, plane[0]);
    gl_ClipDistance[1] = dot(position, plane[1]);
    gl_ClipDistance[2] = dot(position, plane[2]);
}

uniform vec4 u_plane[3];

attribute vec2 a_position;

void main()
{
    gl_Position = vec4(a_position, 0.0, 1.0);

    computeClipDistances(gl_Position, u_plane);
})";

    compileAssumeSuccess(shaderString);
}
}  // namespace sh
