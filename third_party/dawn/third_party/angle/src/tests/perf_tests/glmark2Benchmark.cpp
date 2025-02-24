//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// glmark2Benchmark:
//   Runs the glmark2 benchmark.
//

#include <gtest/gtest.h>

#include <stdio.h>
#include <sstream>

#include "../perf_tests/third_party/perf/perf_result_reporter.h"
#include "ANGLEPerfTestArgs.h"
#include "common/platform.h"
#include "common/string_utils.h"
#include "common/system_utils.h"
#include "test_utils/angle_test_configs.h"
#include "test_utils/angle_test_instantiate.h"
#include "util/test_utils.h"

using namespace angle;

namespace
{

struct BenchmarkInfo
{
    const char *glmark2Config;
    const char *name;
};

// Each glmark2 scene is individually benchmarked.  If glmark2 is run without a specific benchmark,
// it can produce an aggregate score, which is not interesting at the moment.  Adding an empty
// string ("") to this list will enable a test where glmark2 is run with the default scenes and the
// score for each test as well as the overall score is output.
constexpr BenchmarkInfo kBenchmarks[] = {
    {"build:use-vbo=false", "build"},
    {"build:use-vbo=true", "build_vbo"},
    {"texture:texture-filter=nearest", "texture_nearest"},
    {"texture:texture-filter=linear", "texture_linear"},
    {"texture:texture-filter=mipmap", "texture_mipmap"},
    {"shading:shading=gouraud", "shading_gouraud"},
    {"shading:shading=blinn-phong-inf", "shading_blinn_phong"},
    {"shading:shading=phong", "shading_phong"},
    {"shading:shading=cel", "shading_cel"},
    {"bump:bump-render=high-poly", "bump_high_poly"},
    {"bump:bump-render=normals", "bump_normals"},
    {"bump:bump-render=height", "bump_height"},
    {"effect2d:kernel=0,1,0;1,-4,1;0,1,0;", "effect2d_edge"},
    {"effect2d:kernel=1,1,1,1,1;1,1,1,1,1;1,1,1,1,1;", "effect2d_blur"},
    {"pulsar:light=false:quads=5:texture=false", "pulsar"},
    {"desktop:blur-radius=5:effect=blur:passes=1:separable=true:windows=4", "desktop_blur"},
    {"desktop:effect=shadow:windows=4", "desktop_shadow"},
    {"buffer:columns=200:interleave=false:update-dispersion=0.9:update-fraction=0.5:update-method="
     "map",
     "buffer_map"},
    {"buffer:columns=200:interleave=false:update-dispersion=0.9:update-fraction=0.5:update-method="
     "subdata",
     "buffer_subdata"},
    {"buffer:columns=200:interleave=true:update-dispersion=0.9:update-fraction=0.5:update-method="
     "map",
     "buffer_map_interleave"},
    {"ideas:speed=duration", "ideas"},
    {"jellyfish", "jellyfish"},
    {"terrain", "terrain"},
    {"shadow", "shadow"},
    {"refract", "refract"},
    {"conditionals:fragment-steps=0:vertex-steps=0", "conditionals"},
    {"conditionals:fragment-steps=5:vertex-steps=0", "conditionals_fragment"},
    {"conditionals:fragment-steps=0:vertex-steps=5", "conditionals_vertex"},
    {"function:fragment-complexity=low:fragment-steps=5", "function"},
    {"function:fragment-complexity=medium:fragment-steps=5", "function_complex"},
    {"loop:fragment-loop=false:fragment-steps=5:vertex-steps=5", "loop_no_fsloop"},
    {"loop:fragment-steps=5:fragment-uniform=false:vertex-steps=5", "loop_no_uniform"},
    {"loop:fragment-steps=5:fragment-uniform=true:vertex-steps=5", "loop"},
};

struct GLMark2TestParams : public PlatformParameters
{
    BenchmarkInfo info;
};

std::ostream &operator<<(std::ostream &os, const GLMark2TestParams &params)
{
    os << static_cast<const PlatformParameters &>(params) << "_" << params.info.name;
    return os;
}

class GLMark2Benchmark : public testing::TestWithParam<GLMark2TestParams>
{
  public:
    GLMark2Benchmark()
    {
        switch (GetParam().getRenderer())
        {
            case EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE:
                mBackend = "d3d11";
                break;
            case EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE:
                mBackend = "gl";
                break;
            case EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE:
                mBackend = "vulkan";
                break;
            case EGL_PLATFORM_ANGLE_TYPE_METAL_ANGLE:
                mBackend = "metal";
                break;
            default:
                break;
        }
        std::string story = GetParam().info.name;
        mReporter = std::make_unique<perf_test::PerfResultReporter>("glmark2_" + mBackend, story);
        mReporter->RegisterImportantMetric(".fps", "fps");
        mReporter->RegisterImportantMetric(".score", "score");
    }

    void run()
    {
        // Only supported on Linux and Windows at the moment.
        if (!IsLinux() && !IsWindows())
        {
            return;
        }

        const BenchmarkInfo benchmarkInfo = GetParam().info;
        const char *benchmark             = benchmarkInfo.glmark2Config;
        const char *benchmarkName         = benchmarkInfo.name;
        bool completeRun                  = benchmark == nullptr || benchmark[0] == '\0';

        Optional<std::string> cwd = GetCWD();

        // Set the current working directory to the executable's, as the data path of glmark2 is
        // set relative to that path.
        std::string executableDir = GetExecutableDirectory();
        SetCWD(executableDir.c_str());
        SetEnvironmentVar("ANGLE_DEFAULT_PLATFORM", mBackend.c_str());

        std::vector<const char *> args = {
            "glmark2_angle",
        };
        if (OneFrame())
        {
            args.push_back("--validate");
        }
        if (!completeRun)
        {
            args.push_back("--benchmark");
            args.push_back(benchmark);
            fprintf(stderr, "Running benchmark: %s\n", benchmark);
        }
        args.push_back(nullptr);

        ProcessHandle process(args, ProcessOutputCapture::StdoutOnly);
        ASSERT_TRUE(process && process->started());
        ASSERT_TRUE(process->finish());

        // Restore the current working directory for the next tests.
        if (cwd.valid())
        {
            SetCWD(cwd.value().c_str());
        }

        ASSERT_EQ(EXIT_SUCCESS, process->getExitCode());

        if (!OneFrame())
        {
            std::string output = process->getStdout();
            parseOutput(output, benchmarkName, completeRun);
        }
    }

  private:
    void parseOutput(const std::string &output, const char *benchmarkName, bool completeRun)
    {
        // Output is in the following format:
        //
        // =======================================================
        //     glmark2 2017.07
        // =======================================================
        //     OpenGL Information
        //     GL_VENDOR:      ...
        //     GL_RENDERER:    ...
        //     GL_VERSION:     ...
        //     Surface Config: ...
        //     Surface Size:   ...
        // =======================================================
        // [test] config: FPS: uint FrameTime: float ms
        // [test] config: Not Supported
        // ...
        // =======================================================
        //                                   glmark2 Score: uint
        // =======================================================
        //
        // This function skips the header, prints one line for each test/config line where there's
        // an FPS value, and finally prints the overall score.
        std::istringstream glmark2Output(output);
        std::string line;

        // Forward any INFO: lines that may have been generated.
        while (std::getline(glmark2Output, line) && BeginsWith(line, "INFO:"))
        {
            fprintf(stderr, "%s\n", line.c_str());
        }

        // Expect ==== at the top of the header
        ASSERT_EQ('=', line[0]);

        // Skip one line
        std::getline(glmark2Output, line);

        // Expect ==== in the middle of the header
        std::getline(glmark2Output, line);
        ASSERT_EQ('=', line[0]);

        // Skip four lines
        std::getline(glmark2Output, line);
        std::getline(glmark2Output, line);
        std::getline(glmark2Output, line);
        std::getline(glmark2Output, line);

        // The fourth line is the GL_VERSION.  Expect it to include ANGLE, otherwise we are not
        // running against ANGLE.
        ASSERT_NE(line.find("ANGLE"), std::string::npos);

        // Skip two Surface lines
        std::getline(glmark2Output, line);
        std::getline(glmark2Output, line);

        // Expect ==== at the bottom of the header
        std::getline(glmark2Output, line);
        ASSERT_EQ('=', line[0]);

        // Read configs until the top of the footer is reached
        while (std::getline(glmark2Output, line) && line[0] != '=')
        {
            // Parse the line
            std::istringstream lin(line);

            std::string testName, testConfig;
            lin >> testName >> testConfig;
            EXPECT_TRUE(lin);

            std::string fpsTag, frametimeTag;
            size_t fps;
            float frametime;

            lin >> fpsTag >> fps >> frametimeTag >> frametime;

            // If the line is not in `FPS: uint FrameTime: Float ms` format, the test is not
            // supported.  It will be skipped.
            if (!lin)
            {
                continue;
            }

            EXPECT_EQ("FPS:", fpsTag);
            EXPECT_EQ("FrameTime:", frametimeTag);

            if (!completeRun)
            {
                mReporter->AddResult(".fps", fps);
            }
        }

        // Get the score line: `glmark2 Score: uint`
        std::string glmark2Tag, scoreTag;
        size_t score;
        glmark2Output >> glmark2Tag >> scoreTag >> score;
        EXPECT_TRUE(glmark2Output);
        EXPECT_EQ("glmark2", glmark2Tag);
        EXPECT_EQ("Score:", scoreTag);

        if (completeRun)
        {
            mReporter->AddResult(".score", score);
        }
    }

    std::string mBackend = "invalid";
    std::unique_ptr<perf_test::PerfResultReporter> mReporter;
};

TEST_P(GLMark2Benchmark, Run)
{
    run();
}

GLMark2TestParams CombineEGLPlatform(const GLMark2TestParams &in, EGLPlatformParameters eglParams)
{
    GLMark2TestParams out = in;
    out.eglParameters     = eglParams;
    return out;
}

GLMark2TestParams CombineInfo(const GLMark2TestParams &in, BenchmarkInfo info)
{
    GLMark2TestParams out = in;
    out.info              = info;
    return out;
}

using namespace egl_platform;

std::vector<GLMark2TestParams> gTestsWithInfo =
    CombineWithValues({GLMark2TestParams()}, kBenchmarks, CombineInfo);
std::vector<EGLPlatformParameters> gEGLPlatforms = {D3D11(), METAL(), OPENGLES(), VULKAN()};
std::vector<GLMark2TestParams> gTestsWithPlatform =
    CombineWithValues(gTestsWithInfo, gEGLPlatforms, CombineEGLPlatform);

ANGLE_INSTANTIATE_TEST_ARRAY(GLMark2Benchmark, gTestsWithPlatform);

}  // namespace
