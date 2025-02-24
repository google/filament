//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ShaderImage_test.cpp:
// Tests for images
//

#include "GLSLANG/ShaderLang.h"
#include "angle_gl.h"
#include "compiler/translator/StaticType.h"
#include "gtest/gtest.h"
#include "tests/test_utils/ShaderCompileTreeTest.h"
#include "tests/test_utils/compiler_test.h"

using namespace sh;

namespace
{

// Checks that the imageStore call with mangled name imageStoreMangledName exists in the AST.
// Further each argument is checked whether it matches the expected properties given the compiled
// shader.
void CheckImageStoreCall(TIntermNode *astRoot,
                         const TString &imageStoreMangledName,
                         TBasicType imageType,
                         int storeLocationNominalSize,
                         TBasicType storeValueType,
                         int storeValueNominalSize)
{
    const TIntermAggregate *imageStoreFunctionCall =
        FindFunctionCallNode(astRoot, imageStoreMangledName);
    ASSERT_NE(nullptr, imageStoreFunctionCall);

    const TIntermSequence *storeArguments = imageStoreFunctionCall->getSequence();
    ASSERT_EQ(3u, storeArguments->size());

    const TIntermTyped *storeArgument1Typed = (*storeArguments)[0]->getAsTyped();
    ASSERT_EQ(imageType, storeArgument1Typed->getBasicType());

    const TIntermTyped *storeArgument2Typed = (*storeArguments)[1]->getAsTyped();
    ASSERT_EQ(EbtInt, storeArgument2Typed->getBasicType());
    ASSERT_EQ(storeLocationNominalSize, storeArgument2Typed->getNominalSize());

    const TIntermTyped *storeArgument3Typed = (*storeArguments)[2]->getAsTyped();
    ASSERT_EQ(storeValueType, storeArgument3Typed->getBasicType());
    ASSERT_EQ(storeValueNominalSize, storeArgument3Typed->getNominalSize());
}

// Checks that the imageLoad call with mangled name imageLoadMangledName exists in the AST.
// Further each argument is checked whether it matches the expected properties given the compiled
// shader.
void CheckImageLoadCall(TIntermNode *astRoot,
                        const TString &imageLoadMangledName,
                        TBasicType imageType,
                        int loadLocationNominalSize)
{
    const TIntermAggregate *imageLoadFunctionCall =
        FindFunctionCallNode(astRoot, imageLoadMangledName);
    ASSERT_NE(nullptr, imageLoadFunctionCall);

    const TIntermSequence *loadArguments = imageLoadFunctionCall->getSequence();
    ASSERT_EQ(2u, loadArguments->size());

    const TIntermTyped *loadArgument1Typed = (*loadArguments)[0]->getAsTyped();
    ASSERT_EQ(imageType, loadArgument1Typed->getBasicType());

    const TIntermTyped *loadArgument2Typed = (*loadArguments)[1]->getAsTyped();
    ASSERT_EQ(EbtInt, loadArgument2Typed->getBasicType());
    ASSERT_EQ(loadLocationNominalSize, loadArgument2Typed->getNominalSize());
}

// Checks whether the image is properly exported as a uniform by the compiler.
void CheckExportedImageUniform(const std::vector<sh::ShaderVariable> &uniforms,
                               size_t uniformIndex,
                               ::GLenum imageTypeGL,
                               const TString &imageName)
{
    ASSERT_EQ(1u, uniforms.size());

    const auto &imageUniform = uniforms[uniformIndex];
    ASSERT_EQ(imageTypeGL, imageUniform.type);
    ASSERT_STREQ(imageUniform.name.c_str(), imageName.c_str());
}

// Checks whether the image is saved in the AST as a node with the correct properties given the
// shader.
void CheckImageDeclaration(TIntermNode *astRoot,
                           const ImmutableString &imageName,
                           TBasicType imageType,
                           TLayoutImageInternalFormat internalFormat,
                           bool readonly,
                           bool writeonly,
                           bool coherent,
                           bool restrictQualifier,
                           bool volatileQualifier,
                           int binding)
{
    const TIntermSymbol *myImageNode = FindSymbolNode(astRoot, imageName);
    ASSERT_NE(nullptr, myImageNode);

    ASSERT_EQ(imageType, myImageNode->getBasicType());
    const TType &myImageType                = myImageNode->getType();
    TLayoutQualifier myImageLayoutQualifier = myImageType.getLayoutQualifier();
    ASSERT_EQ(internalFormat, myImageLayoutQualifier.imageInternalFormat);
    TMemoryQualifier myImageMemoryQualifier = myImageType.getMemoryQualifier();
    ASSERT_EQ(readonly, myImageMemoryQualifier.readonly);
    ASSERT_EQ(writeonly, myImageMemoryQualifier.writeonly);
    ASSERT_EQ(coherent, myImageMemoryQualifier.coherent);
    ASSERT_EQ(restrictQualifier, myImageMemoryQualifier.restrictQualifier);
    ASSERT_EQ(volatileQualifier, myImageMemoryQualifier.volatileQualifier);
    ASSERT_EQ(binding, myImageType.getLayoutQualifier().binding);
}

}  // namespace

class ShaderImageTest : public ShaderCompileTreeTest
{
  public:
    ShaderImageTest() {}

  protected:
    void SetUp() override { ShaderCompileTreeTest::SetUp(); }

    ::GLenum getShaderType() const override { return GL_COMPUTE_SHADER; }
    ShShaderSpec getShaderSpec() const override { return SH_GLES3_1_SPEC; }
};

// Test that an image2D is properly parsed and exported as a uniform.
TEST_F(ShaderImageTest, Image2DDeclaration)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(local_size_x = 4) in;\n"
        "layout(rgba32f, binding = 1) uniform highp readonly image2D myImage;\n"
        "void main() {\n"
        "   ivec2 sz = imageSize(myImage);\n"
        "}";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed" << mInfoLog;
    }

    CheckExportedImageUniform(getUniforms(), 0, GL_IMAGE_2D, "myImage");
    CheckImageDeclaration(mASTRoot, ImmutableString("myImage"), EbtImage2D, EiifRGBA32F, true,
                          false, false, false, false, 1);
}

// Test that an image3D is properly parsed and exported as a uniform.
TEST_F(ShaderImageTest, Image3DDeclaration)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(local_size_x = 4) in;\n"
        "layout(rgba32ui, binding = 3) uniform highp writeonly readonly uimage3D myImage;\n"
        "void main() {\n"
        "   ivec3 sz = imageSize(myImage);\n"
        "}";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed" << mInfoLog;
    }

    CheckExportedImageUniform(getUniforms(), 0, GL_UNSIGNED_INT_IMAGE_3D, "myImage");
    CheckImageDeclaration(mASTRoot, ImmutableString("myImage"), EbtUImage3D, EiifRGBA32UI, true,
                          true, false, false, false, 3);
}

// Check that imageLoad calls get correctly parsed.
TEST_F(ShaderImageTest, ImageLoad)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(local_size_x = 4) in;\n"
        "layout(rgba32f) uniform highp readonly image2D my2DImageInput;\n"
        "layout(rgba32i) uniform highp readonly iimage3D my3DImageInput;\n"
        "layout(rgba32f) uniform highp writeonly image2D imageOutput;\n"
        "void main() {\n"
        "   vec4 result = imageLoad(my2DImageInput, ivec2(gl_LocalInvocationID.xy));\n"
        "   ivec4 result2 = imageLoad(my3DImageInput, ivec3(gl_LocalInvocationID.xyz));\n"
        "   // Ensure the imageLoad calls are not dead-code eliminated\n"
        "   imageStore(imageOutput, ivec2(0), result + vec4(result2));\n"
        "}";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed" << mInfoLog;
    }

    // imageLoad call with image2D passed
    std::string mangledName2D = "imageLoad(";
    mangledName2D += StaticType::GetBasic<EbtImage2D, EbpUndefined>()->getMangledName();
    mangledName2D += StaticType::GetBasic<EbtInt, EbpUndefined, 2>()->getMangledName();
    CheckImageLoadCall(mASTRoot, mangledName2D.c_str(), EbtImage2D, 2);

    // imageLoad call with image3D passed
    std::string mangledName3D = "imageLoad(";
    mangledName3D += StaticType::GetBasic<EbtIImage3D, EbpUndefined>()->getMangledName();
    mangledName3D += StaticType::GetBasic<EbtInt, EbpUndefined, 3>()->getMangledName();
    CheckImageLoadCall(mASTRoot, mangledName3D.c_str(), EbtIImage3D, 3);
}

// Check that imageStore calls get correctly parsed.
TEST_F(ShaderImageTest, ImageStore)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(local_size_x = 4) in;\n"
        "layout(rgba32f) uniform highp writeonly image2D my2DImageOutput;\n"
        "layout(rgba32ui) uniform highp writeonly uimage2DArray my2DImageArrayOutput;\n"
        "void main() {\n"
        "   imageStore(my2DImageOutput, ivec2(gl_LocalInvocationID.xy), vec4(0.0));\n"
        "   imageStore(my2DImageArrayOutput, ivec3(gl_LocalInvocationID.xyz), uvec4(0));\n"
        "}";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed" << mInfoLog;
    }

    // imageStore call with image2D
    std::string mangledName2D = "imageStore(";
    mangledName2D += StaticType::GetBasic<EbtImage2D, EbpUndefined>()->getMangledName();
    mangledName2D += StaticType::GetBasic<EbtInt, EbpUndefined, 2>()->getMangledName();
    mangledName2D += StaticType::GetBasic<EbtFloat, EbpUndefined, 4>()->getMangledName();
    CheckImageStoreCall(mASTRoot, mangledName2D.c_str(), EbtImage2D, 2, EbtFloat, 4);

    // imageStore call with image2DArray
    std::string mangledName2DArray = "imageStore(";
    mangledName2DArray += StaticType::GetBasic<EbtUImage2DArray, EbpUndefined>()->getMangledName();
    mangledName2DArray += StaticType::GetBasic<EbtInt, EbpUndefined, 3>()->getMangledName();
    mangledName2DArray += StaticType::GetBasic<EbtUInt, EbpUndefined, 4>()->getMangledName();
    CheckImageStoreCall(mASTRoot, mangledName2DArray.c_str(), EbtUImage2DArray, 3, EbtUInt, 4);
}

// Check that memory qualifiers are correctly parsed.
TEST_F(ShaderImageTest, ImageMemoryQualifiers)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(local_size_x = 4) in;"
        "layout(rgba32f) uniform highp coherent readonly image2D image1;\n"
        "layout(rgba32f) uniform highp volatile writeonly image2D image2;\n"
        "layout(rgba32f) uniform highp volatile restrict readonly writeonly image2D image3;\n"
        "void main() {\n"
        "    imageSize(image1);\n"
        "    imageSize(image2);\n"
        "    imageSize(image3);\n"
        "}";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed" << mInfoLog;
    }

    CheckImageDeclaration(mASTRoot, ImmutableString("image1"), EbtImage2D, EiifRGBA32F, true, false,
                          true, false, false, -1);
    CheckImageDeclaration(mASTRoot, ImmutableString("image2"), EbtImage2D, EiifRGBA32F, false, true,
                          true, false, true, -1);
    CheckImageDeclaration(mASTRoot, ImmutableString("image3"), EbtImage2D, EiifRGBA32F, true, true,
                          true, true, true, -1);
}
