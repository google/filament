/*
 * Copyright (C) 2023 The Android Open Source Project
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

#include <utils/Log.h>

#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <gtest/gtest.h>

using namespace utils;
using namespace std::literals;

class CompilerTest : public testing::Test {
protected:
    void SetUp() override {
        EGLBoolean success;
        EGLint major, minor;
        dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        ASSERT_NE(dpy, EGL_NO_DISPLAY);

        EGLBoolean const initialized = eglInitialize(dpy, &major, &minor);
        ASSERT_TRUE(initialized);

        EGLint const contextAttribs[] = {
                EGL_CONTEXT_CLIENT_VERSION, 3,
                EGL_NONE
        };

        EGLint configsCount;
        EGLint configAttribs[] = {
                EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,        //  0
                EGL_RED_SIZE,    8,                                 //  2
                EGL_GREEN_SIZE,  8,                                 //  4
                EGL_BLUE_SIZE,   8,                                 //  6
                EGL_NONE                                            // 14
        };
        success = eglChooseConfig(dpy, configAttribs, &config, 1, &configsCount);
        ASSERT_TRUE(success);

        context = eglCreateContext(dpy, config, EGL_NO_CONTEXT, contextAttribs);
        ASSERT_NE(context, EGL_NO_CONTEXT);

        success = eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, context);
        ASSERT_TRUE(success);

        ASSERT_EQ(eglGetError(), EGL_SUCCESS);
    }

    void TearDown() override {
        eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroyContext(dpy, context);
        eglTerminate(dpy);
    }

private:
    EGLDisplay dpy;
    EGLContext context;
    EGLConfig config;
};

TEST_F(CompilerTest, Simple) {
    auto shader = R"(
#version 300 es
void main()
{
})"sv;

    const char* const src = shader.data();
    GLint const len = (GLint)shader.size();

    GLuint const id = glCreateShader(GL_VERTEX_SHADER);
    EXPECT_EQ(glGetError(), GL_NO_ERROR);

    glShaderSource(id, 1, &src, &len);
    EXPECT_EQ(glGetError(), GL_NO_ERROR);

    glCompileShader(id);
    EXPECT_EQ(glGetError(), GL_NO_ERROR);

    GLint result = 0;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    EXPECT_EQ(result, GL_TRUE);

    glDeleteShader(id);
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(CompilerTest, CrashPVRUniFlexCompileToHw) {

    // Some PowerVR driver crash with this shader

    auto shader = R"(
#version 300 es

layout(location = 0) in vec4 mesh_position;

layout(std140) uniform FrameUniforms {
    vec2 i;
} frameUniforms;

void main() {
    gl_Position = mesh_position;
    gl_Position.z = dot(gl_Position.zw, frameUniforms.i);
})"sv;

    const char* const src = shader.data();
    GLint const len = (GLint)shader.size();

    GLuint const id = glCreateShader(GL_VERTEX_SHADER);
    EXPECT_EQ(glGetError(), GL_NO_ERROR);

    glShaderSource(id, 1, &src, &len);
    EXPECT_EQ(glGetError(), GL_NO_ERROR);

    glCompileShader(id);
    EXPECT_EQ(glGetError(), GL_NO_ERROR);

    GLint result = 0;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    EXPECT_EQ(result, GL_TRUE);

    glDeleteShader(id);
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(CompilerTest, ConstParameters) {

    // Some PowerVR driver fail to compile this shader

    auto shader = R"(
#version 300 es

highp mat3 m;
void buggy(const mediump vec3 n) {
    m*n;
}

void main() {
})"sv;

    const char* const src = shader.data();
    GLint const len = (GLint)shader.size();

    GLuint const id = glCreateShader(GL_FRAGMENT_SHADER);
    EXPECT_EQ(glGetError(), GL_NO_ERROR);

    glShaderSource(id, 1, &src, &len);
    EXPECT_EQ(glGetError(), GL_NO_ERROR);

    glCompileShader(id);
    EXPECT_EQ(glGetError(), GL_NO_ERROR);

    GLint result = 0;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    EXPECT_EQ(result, GL_TRUE);

    EXPECT_EQ(glGetError(), GL_NO_ERROR);

    glDeleteShader(id);
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
