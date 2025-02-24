//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ParallelShaderCompileTest.cpp : Tests of the GL_KHR_parallel_shader_compile extension.

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

#include "util/random_utils.h"
#include "util/test_utils.h"

using namespace angle;

namespace
{

namespace
{

constexpr int kTaskCount             = 32;
constexpr unsigned int kPollInterval = 100;

}  // anonymous namespace

class ParallelShaderCompileTest : public ANGLETest<>
{
  protected:
    ParallelShaderCompileTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    bool ensureParallelShaderCompileExtensionAvailable()
    {
        if (IsGLExtensionRequestable("GL_KHR_parallel_shader_compile"))
        {
            glRequestExtensionANGLE("GL_KHR_parallel_shader_compile");
        }

        if (!IsGLExtensionEnabled("GL_KHR_parallel_shader_compile"))
        {
            return false;
        }
        return true;
    }

    class Task
    {
      public:
        Task(int id) : mID(id) {}
        virtual ~Task() {}

        virtual bool compile()            = 0;
        virtual bool isCompileCompleted() = 0;
        virtual bool link()               = 0;
        virtual void postLink() {}
        virtual void runAndVerify(ParallelShaderCompileTest *test) = 0;

        bool isLinkCompleted()
        {
            GLint status;
            glGetProgramiv(mProgram, GL_COMPLETION_STATUS_KHR, &status);
            return (status == GL_TRUE);
        }

      protected:
        static std::string InsertRandomString(const std::string &source)
        {
            RNG rng;
            std::ostringstream ostream;
            ostream << source << "\n// Random string to fool program cache: " << rng.randomInt()
                    << "\n";
            return ostream.str();
        }

        static GLuint CompileShader(GLenum type, const std::string &source)
        {
            GLuint shader = glCreateShader(type);

            const char *sourceArray[1] = {source.c_str()};
            glShaderSource(shader, 1, sourceArray, nullptr);
            glCompileShader(shader);
            return shader;
        }

        static void RecompileShader(GLuint shader, const std::string &source)
        {
            const char *sourceArray[1] = {source.c_str()};
            glShaderSource(shader, 1, sourceArray, nullptr);
            glCompileShader(shader);
        }

        static bool CheckShader(GLuint shader)
        {
            GLint compileResult;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResult);

            if (compileResult == 0)
            {
                GLint infoLogLength;
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

                // Info log length includes the null terminator, so 1 means that the info log is an
                // empty string.
                if (infoLogLength > 1)
                {
                    std::vector<GLchar> infoLog(infoLogLength);
                    glGetShaderInfoLog(shader, static_cast<GLsizei>(infoLog.size()), nullptr,
                                       &infoLog[0]);
                    std::cerr << "shader compilation failed: " << &infoLog[0];
                }
                else
                {
                    std::cerr << "shader compilation failed. <Empty log message>";
                }
                std::cerr << std::endl;
            }
            return (compileResult == GL_TRUE);
        }

        GLuint mProgram;
        int mID;
    };

    template <typename T>
    class TaskRunner
    {
      public:
        TaskRunner() {}
        ~TaskRunner() {}

        void run(ParallelShaderCompileTest *test, unsigned int pollInterval)
        {

            std::vector<std::unique_ptr<T>> compileTasks;
            for (int i = 0; i < kTaskCount; ++i)
            {
                std::unique_ptr<T> task(new T(i));
                bool isCompiling = task->compile();
                ASSERT_TRUE(isCompiling);
                compileTasks.push_back(std::move(task));
            }

            std::vector<std::unique_ptr<T>> linkTasks;
            while (!compileTasks.empty())
            {
                for (unsigned int i = 0; i < compileTasks.size();)
                {
                    auto &task = compileTasks[i];

                    if (task->isCompileCompleted())
                    {
                        bool isLinking = task->link();
                        task->postLink();
                        ASSERT_TRUE(isLinking);
                        linkTasks.push_back(std::move(task));
                        compileTasks.erase(compileTasks.begin() + i);
                        continue;
                    }
                    ++i;
                }
                if (pollInterval != 0)
                {
                    angle::Sleep(pollInterval);
                }
            }

            while (!linkTasks.empty())
            {
                for (unsigned int i = 0; i < linkTasks.size();)
                {
                    auto &task = linkTasks[i];

                    if (task->isLinkCompleted())
                    {
                        task->runAndVerify(test);
                        linkTasks.erase(linkTasks.begin() + i);
                        continue;
                    }
                    else
                    {
                        task->postLink();
                    }
                    ++i;
                }
                if (pollInterval != 0)
                {
                    angle::Sleep(pollInterval);
                }
            }
        }
    };

    class ClearColorWithDraw : public Task
    {
      public:
        ClearColorWithDraw(int taskID) : Task(taskID)
        {
            auto color = static_cast<GLubyte>(taskID * 255 / kTaskCount);
            mColor     = {color, color, color, 255};
        }

        bool compile() override
        {
            mVertexShader =
                CompileShader(GL_VERTEX_SHADER, InsertRandomString(essl1_shaders::vs::Simple()));
            mFragmentShader = CompileShader(GL_FRAGMENT_SHADER,
                                            InsertRandomString(essl1_shaders::fs::UniformColor()));
            return (mVertexShader != 0 && mFragmentShader != 0);
        }

        bool isCompileCompleted() override
        {
            GLint status;
            glGetShaderiv(mVertexShader, GL_COMPLETION_STATUS_KHR, &status);
            if (status == GL_TRUE)
            {
                glGetShaderiv(mFragmentShader, GL_COMPLETION_STATUS_KHR, &status);
                return (status == GL_TRUE);
            }
            return false;
        }

        bool link() override
        {
            mProgram = 0;
            if (CheckShader(mVertexShader) && CheckShader(mFragmentShader))
            {
                mProgram = glCreateProgram();
                glAttachShader(mProgram, mVertexShader);
                glAttachShader(mProgram, mFragmentShader);
                glLinkProgram(mProgram);
            }
            glDeleteShader(mVertexShader);
            glDeleteShader(mFragmentShader);
            return (mProgram != 0);
        }

        void runAndVerify(ParallelShaderCompileTest *test) override
        {
            glClearColor(0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT);
            glDisable(GL_DEPTH_TEST);
            glUseProgram(mProgram);
            ASSERT_GL_NO_ERROR();
            GLint colorUniformLocation =
                glGetUniformLocation(mProgram, essl1_shaders::ColorUniform());
            ASSERT_NE(colorUniformLocation, -1);
            auto normalizeColor = mColor.toNormalizedVector();
            glUniform4fv(colorUniformLocation, 1, normalizeColor.data());
            test->drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.5f);
            EXPECT_PIXEL_COLOR_EQ(test->getWindowWidth() / 2, test->getWindowHeight() / 2, mColor);
            glUseProgram(0);
            glDeleteProgram(mProgram);
            ASSERT_GL_NO_ERROR();
        }

      protected:
        void recompile()
        {
            RecompileShader(mVertexShader, essl1_shaders::vs::Simple());
            RecompileShader(mFragmentShader, essl1_shaders::fs::UniformColor());
        }

      private:
        GLuint mVertexShader;
        GLuint mFragmentShader;
        GLColor mColor;
    };

    class ClearColorWithDrawRecompile : public ClearColorWithDraw
    {
      public:
        ClearColorWithDrawRecompile(int taskID) : ClearColorWithDraw(taskID) {}

        void postLink() override { recompile(); }
    };

    class ImageLoadStore : public Task
    {
      public:
        ImageLoadStore(int taskID) : Task(taskID) {}
        ~ImageLoadStore() {}

        bool compile() override
        {
            const char kCSSource[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(r32ui, binding = 0) readonly uniform highp uimage2D uImage_1;
layout(r32ui, binding = 1) writeonly uniform highp uimage2D uImage_2;
void main()
{
    uvec4 value = imageLoad(uImage_1, ivec2(gl_LocalInvocationID.xy));
    imageStore(uImage_2, ivec2(gl_LocalInvocationID.xy), value);
})";

            mShader = CompileShader(GL_COMPUTE_SHADER, InsertRandomString(kCSSource));
            return mShader != 0;
        }

        bool isCompileCompleted() override
        {
            GLint status;
            glGetShaderiv(mShader, GL_COMPLETION_STATUS_KHR, &status);
            return status == GL_TRUE;
        }

        bool link() override
        {
            mProgram = 0;
            if (CheckShader(mShader))
            {
                mProgram = glCreateProgram();
                glAttachShader(mProgram, mShader);
                glLinkProgram(mProgram);
            }
            glDeleteShader(mShader);
            return mProgram != 0;
        }

        void runAndVerify(ParallelShaderCompileTest *test) override
        {
            // Taken from ComputeShaderTest.StoreImageThenLoad.
            constexpr GLuint kInputValues[3][1] = {{300}, {200}, {100}};
            GLTexture texture[3];
            glBindTexture(GL_TEXTURE_2D, texture[0]);
            glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, 1, 1);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT,
                            kInputValues[0]);
            EXPECT_GL_NO_ERROR();

            glBindTexture(GL_TEXTURE_2D, texture[1]);
            glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, 1, 1);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT,
                            kInputValues[1]);
            EXPECT_GL_NO_ERROR();

            glBindTexture(GL_TEXTURE_2D, texture[2]);
            glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, 1, 1);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT,
                            kInputValues[2]);
            EXPECT_GL_NO_ERROR();

            glUseProgram(mProgram);

            glBindImageTexture(0, texture[0], 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32UI);
            glBindImageTexture(1, texture[1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32UI);

            glDispatchCompute(1, 1, 1);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            EXPECT_GL_NO_ERROR();

            glBindImageTexture(0, texture[1], 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32UI);
            glBindImageTexture(1, texture[2], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32UI);

            glDispatchCompute(1, 1, 1);
            glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
            EXPECT_GL_NO_ERROR();

            GLuint outputValue;
            GLFramebuffer framebuffer;
            glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
            glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                   texture[2], 0);
            glReadPixels(0, 0, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, &outputValue);
            EXPECT_GL_NO_ERROR();

            EXPECT_EQ(300u, outputValue);

            glUseProgram(0);
            glDeleteProgram(mProgram);
            ASSERT_GL_NO_ERROR();
        }

      private:
        GLuint mShader;
    };
};

// Test basic functionality of GL_KHR_parallel_shader_compile
TEST_P(ParallelShaderCompileTest, Basic)
{
    ANGLE_SKIP_TEST_IF(!ensureParallelShaderCompileExtensionAvailable());

    GLint count = 0;
    glMaxShaderCompilerThreadsKHR(8);
    EXPECT_GL_NO_ERROR();
    glGetIntegerv(GL_MAX_SHADER_COMPILER_THREADS_KHR, &count);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(8, count);
}

// Test to compile and link many programs in parallel.
TEST_P(ParallelShaderCompileTest, LinkAndDrawManyPrograms)
{
    ANGLE_SKIP_TEST_IF(!ensureParallelShaderCompileExtensionAvailable());

    TaskRunner<ClearColorWithDraw> runner;
    runner.run(this, kPollInterval);
}

// Tests no crash in case that the Shader starts another compile while the Program being attached
// to is still linking.
// crbug.com/1317673
TEST_P(ParallelShaderCompileTest, LinkProgramAndRecompileShader)
{
    ANGLE_SKIP_TEST_IF(!ensureParallelShaderCompileExtensionAvailable());

    TaskRunner<ClearColorWithDrawRecompile> runner;
    runner.run(this, 0);
}

class ParallelShaderCompileTestES31 : public ParallelShaderCompileTest
{};

// Test to compile and link many computing programs in parallel.
TEST_P(ParallelShaderCompileTestES31, LinkAndDispatchManyPrograms)
{
    // Flaky on Win NVIDIA D3D11. http://anglebug.com/40096580
    // Suspectable to the flakyness of http://anglebug.com/40096579.
    ANGLE_SKIP_TEST_IF(IsWindows() && IsD3D11());

    // TODO(http://anglebug.com/42264192): Fails on Linux+Intel+OpenGL
    ANGLE_SKIP_TEST_IF(IsLinux() && IsIntel() && IsOpenGL());

    ANGLE_SKIP_TEST_IF(!ensureParallelShaderCompileExtensionAvailable());

    TaskRunner<ImageLoadStore> runner;
    runner.run(this, kPollInterval);
}

ANGLE_INSTANTIATE_TEST_ES2(ParallelShaderCompileTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ParallelShaderCompileTestES31);
ANGLE_INSTANTIATE_TEST_ES31(ParallelShaderCompileTestES31);

}  // namespace
