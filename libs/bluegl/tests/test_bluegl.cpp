/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>

#include <bluegl/BlueGL.h>

#include "OpenGLSupport.hpp"

const char * GetGLErrorStr(GLenum err)
{
    switch (err)
    {
        case GL_NO_ERROR:          return "No error";
        case GL_INVALID_ENUM:      return "Invalid enum";
        case GL_INVALID_VALUE:     return "Invalid value";
        case GL_INVALID_OPERATION: return "Invalid operation";
        case GL_STACK_OVERFLOW:    return "Stack overflow";
        case GL_STACK_UNDERFLOW:   return "Stack underflow";
        case GL_OUT_OF_MEMORY:     return "Out of memory";
        default:                   return "Unknown error";
    }
}

void checkError() {
    const GLenum err = glGetError();
    if (GL_NO_ERROR == err)
        return;

    std::cout << "GL Error: " << GetGLErrorStr(err) << std::endl;
}

namespace bluegl {

TEST(BlueGLTest, Initialization) {
    EXPECT_EQ(0, bind());
    unbind();
}

TEST(BlueGLTest, GetVersion) {
    EXPECT_EQ(0, bind());
    gl::OpenGLContext context = gl::createOpenGLContext();

    GLint major = 0, minor = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);

    printf("OpenGL v%d.%d\n", major, minor);
    gl::destroyOpenGLContext(context);
    unbind();
}

TEST(BlueGLTest, DoubleInitialization) {
    int result = bind();
    EXPECT_EQ(result, bind());
    unbind();
    unbind();
}

TEST(BlueGLTest, CallOpenGL) {
    EXPECT_EQ(0, bind());

    gl::OpenGLContext context = gl::createOpenGLContext();
    EXPECT_TRUE(context != nullptr);

    gl::setCurrentOpenGLContext(context);
    EXPECT_TRUE(glGetString(GL_VENDOR) != nullptr);

    gl::destroyOpenGLContext(context);
    unbind();
}

TEST(BlueGLTest, CallOpenGLDoubleInit) {
    int result = bind();
    EXPECT_EQ(result, bind());
    unbind();

    gl::OpenGLContext context = gl::createOpenGLContext();
    EXPECT_TRUE(context != nullptr);

    gl::setCurrentOpenGLContext(context);
    EXPECT_TRUE(glGetString(GL_VENDOR) != nullptr);

    gl::destroyOpenGLContext(context);
    unbind();
}

TEST(BlueGLTest, CallBindAndGetBinded) {
    gl::OpenGLContext context = gl::createOpenGLContext();
    bind();
    GLuint texId;
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);

    GLint bindedTexId;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &bindedTexId);
    EXPECT_TRUE(texId == bindedTexId);

    gl::destroyOpenGLContext(context);
    unbind();
}

}; // namespace bluegl

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
