//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EGLPrintEGLinfoTest.cpp:
//   This test prints out the extension strings, configs and their attributes
//

#include <gtest/gtest.h>

#include "common/string_utils.h"
#include "test_utils/ANGLETest.h"

#if defined(ANGLE_HAS_RAPIDJSON)
#    include "common/serializer/JsonSerializer.h"
#    include "test_utils/runner/TestSuite.h"
#endif  // defined(ANGLE_HAS_RAPIDJSON)

using namespace angle;

class EGLPrintEGLinfoTest : public ANGLETest<>
{
  protected:
    EGLPrintEGLinfoTest() {}

    void testSetUp() override
    {
        mDisplay = getEGLWindow()->getDisplay();
        ASSERT_TRUE(mDisplay != EGL_NO_DISPLAY);
    }

    EGLDisplay mDisplay = EGL_NO_DISPLAY;
};

namespace
{
// Parse space separated extension string into a vector of strings
std::vector<std::string> ParseExtensions(const char *extensions)
{
    std::string extensionsStr(extensions);
    std::vector<std::string> extensionsVec;
    SplitStringAlongWhitespace(extensionsStr, &extensionsVec);
    return extensionsVec;
}

// Query a EGL attribute
EGLint GetAttrib(EGLDisplay display, EGLConfig config, EGLint attrib)
{
    EGLint value = 0;
    EXPECT_EGL_TRUE(eglGetConfigAttrib(display, config, attrib, &value));
    return value;
}

// Query a egl string
const char *GetEGLString(EGLDisplay display, EGLint name)
{
    const char *value = "";
    value             = eglQueryString(display, name);
    EXPECT_EGL_ERROR(EGL_SUCCESS);
    EXPECT_TRUE(value != nullptr);
    return value;
}

// Query a GL string
const char *GetGLString(EGLint name)
{
    const char *value = "";
    value             = reinterpret_cast<const char *>(glGetString(name));
    EXPECT_GL_ERROR(GL_NO_ERROR);
    EXPECT_TRUE(value != nullptr);
    return value;
}

}  // namespace

// Print the EGL strings and extensions
TEST_P(EGLPrintEGLinfoTest, PrintEGLInfo)
{
    std::cout << "    EGL Information:" << std::endl;
    std::cout << "\tVendor: " << GetEGLString(mDisplay, EGL_VENDOR) << std::endl;
    std::cout << "\tVersion: " << GetEGLString(mDisplay, EGL_VERSION) << std::endl;
    std::cout << "\tClient APIs: " << GetEGLString(mDisplay, EGL_CLIENT_APIS) << std::endl;

    std::cout << "\tEGL Client Extensions:" << std::endl;
    for (auto extension : ParseExtensions(GetEGLString(EGL_NO_DISPLAY, EGL_EXTENSIONS)))
    {
        std::cout << "\t\t" << extension << std::endl;
    }

    std::cout << "\tEGL Display Extensions:" << std::endl;
    for (auto extension : ParseExtensions(GetEGLString(mDisplay, EGL_EXTENSIONS)))
    {
        std::cout << "\t\t" << extension << std::endl;
    }

    std::cout << std::endl;
}

// Print the GL strings and extensions
TEST_P(EGLPrintEGLinfoTest, PrintGLInfo)
{
    std::cout << "    GLES Information:" << std::endl;
    std::cout << "\tVendor: " << GetGLString(GL_VENDOR) << std::endl;
    std::cout << "\tVersion: " << GetGLString(GL_VERSION) << std::endl;
    std::cout << "\tRenderer: " << GetGLString(GL_RENDERER) << std::endl;
    std::cout << "\tShader: " << GetGLString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    std::cout << "\tExtensions:" << std::endl;

    const std::vector<std::string> extensions = ParseExtensions(GetGLString(GL_EXTENSIONS));

    for (const std::string &extension : extensions)
    {
        std::cout << "\t\t" << extension << std::endl;
    }

    std::cout << std::endl;

#if defined(ANGLE_HAS_RAPIDJSON)
    angle::TestSuite *testSuite = angle::TestSuite::GetInstance();
    if (!testSuite->hasTestArtifactsDirectory())
    {
        return;
    }
    JsonSerializer json;
    json.addCString("Vendor", GetGLString(GL_VENDOR));
    json.addCString("Version", GetGLString(GL_VERSION));
    json.addCString("Renderer", GetGLString(GL_RENDERER));
    json.addCString("ShaderLanguageVersion", GetGLString(GL_SHADING_LANGUAGE_VERSION));
    json.addVectorOfStrings("Extensions", extensions);

    constexpr size_t kBufferSize = 1000;
    std::array<char, kBufferSize> buffer;
    std::time_t timeNow = std::time(nullptr);
    std::strftime(buffer.data(), buffer.size(), "%B %e, %Y", std::localtime(&timeNow));
    json.addCString("DateRecorded", buffer.data());

    std::stringstream fnameStream;
    fnameStream << "GLinfo_" << GetParam() << ".json";
    std::string fname = fnameStream.str();

    const std::string artifactPath = testSuite->reserveTestArtifactPath(fname);

    {
        std::vector<uint8_t> jsonData = json.getData();

        FILE *fp = fopen(artifactPath.c_str(), "wb");
        ASSERT(fp);
        fwrite(jsonData.data(), sizeof(uint8_t), jsonData.size(), fp);
        fclose(fp);
    }
#endif  // defined(ANGLE_HAS_RAPIDJSON)
}

#define QUERY_HELPER(enumValue, enumString, stream)                                    \
    {                                                                                  \
        GLint result;                                                                  \
        glGetIntegerv(enumValue, &result);                                             \
        ASSERT_GL_NO_ERROR();                                                          \
        stream << enumString + std::string(",") + std::to_string(result) << std::endl; \
    }

#define QUERY_ARRAY_HELPER(enumValue, enumString, size, stream)               \
    {                                                                         \
        GLint result[size];                                                   \
        glGetIntegerv(enumValue, result);                                     \
        ASSERT_GL_NO_ERROR();                                                 \
        std::stringstream results;                                            \
        for (int i = 0; i < size; i++)                                        \
            results << result[i] << " ";                                      \
        stream << enumString + std::string(",") + results.str() << std::endl; \
    }

#define QUERY_INDEXED_HELPER(enumValue, enumString, index, stream)                     \
    {                                                                                  \
        GLint result;                                                                  \
        glGetIntegeri_v(enumValue, index, &result);                                    \
        ASSERT_GL_NO_ERROR();                                                          \
        stream << enumString + std::string(",") + std::to_string(result) << std::endl; \
    }

#define QUERY_AND_LOG_CAPABILITY(enum, stream) QUERY_HELPER(enum, #enum, stream)

#define QUERY_AND_LOG_CAPABILITY_ARRAY(enum, size, stream) \
    QUERY_ARRAY_HELPER(enum, #enum, size, stream)

#define QUERY_AND_LOG_CAPABILITY_INDEXED(enum, index, stream) \
    QUERY_INDEXED_HELPER(enum, #enum "[" #index "]", index, stream)

static void LogGles2Capabilities(std::ostream &stream)
{
    QUERY_AND_LOG_CAPABILITY(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_CUBE_MAP_TEXTURE_SIZE, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_FRAGMENT_UNIFORM_VECTORS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_RENDERBUFFER_SIZE, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_TEXTURE_IMAGE_UNITS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_TEXTURE_SIZE, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_VARYING_VECTORS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_VERTEX_ATTRIBS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_VERTEX_UNIFORM_VECTORS, stream);
    constexpr int kMaxViewPortDimsReturnValuesSize = 2;
    QUERY_AND_LOG_CAPABILITY_ARRAY(GL_MAX_VIEWPORT_DIMS, kMaxViewPortDimsReturnValuesSize, stream);
    QUERY_AND_LOG_CAPABILITY(GL_NUM_COMPRESSED_TEXTURE_FORMATS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_NUM_SHADER_BINARY_FORMATS, stream);
}

static void LogGles3Capabilities(std::ostream &stream)
{
    QUERY_AND_LOG_CAPABILITY(GL_MAX_3D_TEXTURE_SIZE, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_ARRAY_TEXTURE_LAYERS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_COLOR_ATTACHMENTS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_COMBINED_UNIFORM_BLOCKS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_DRAW_BUFFERS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_ELEMENT_INDEX, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_ELEMENTS_INDICES, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_ELEMENTS_VERTICES, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_FRAGMENT_INPUT_COMPONENTS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_FRAGMENT_UNIFORM_BLOCKS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_PROGRAM_TEXEL_OFFSET, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_SAMPLES, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_SERVER_WAIT_TIMEOUT, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_TEXTURE_LOD_BIAS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_UNIFORM_BLOCK_SIZE, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_UNIFORM_BUFFER_BINDINGS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_VARYING_COMPONENTS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_VERTEX_OUTPUT_COMPONENTS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_VERTEX_UNIFORM_BLOCKS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_VERTEX_UNIFORM_COMPONENTS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MIN_PROGRAM_TEXEL_OFFSET, stream);
    QUERY_AND_LOG_CAPABILITY(GL_NUM_PROGRAM_BINARY_FORMATS, stream);
    // GLES3 capabilities are a superset of GLES2
    LogGles2Capabilities(stream);
}

static void LogGles31Capabilities(std::ostream &stream)
{
    QUERY_AND_LOG_CAPABILITY(GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_ATOMIC_COUNTER_BUFFER_SIZE, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_COLOR_TEXTURE_SAMPLES, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_COMBINED_ATOMIC_COUNTERS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_COMBINED_IMAGE_UNIFORMS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_COMBINED_SHADER_OUTPUT_RESOURCES, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_COMPUTE_ATOMIC_COUNTERS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_COMPUTE_IMAGE_UNIFORMS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_COMPUTE_SHARED_MEMORY_SIZE, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_COMPUTE_UNIFORM_BLOCKS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_COMPUTE_UNIFORM_COMPONENTS, stream);
    QUERY_AND_LOG_CAPABILITY_INDEXED(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, stream);
    QUERY_AND_LOG_CAPABILITY_INDEXED(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, stream);
    QUERY_AND_LOG_CAPABILITY_INDEXED(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, stream);
    QUERY_AND_LOG_CAPABILITY_INDEXED(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, stream);
    QUERY_AND_LOG_CAPABILITY_INDEXED(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, stream);
    QUERY_AND_LOG_CAPABILITY_INDEXED(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_DEPTH_TEXTURE_SAMPLES, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_FRAGMENT_ATOMIC_COUNTERS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_FRAGMENT_IMAGE_UNIFORMS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_FRAMEBUFFER_HEIGHT, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_FRAMEBUFFER_SAMPLES, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_FRAMEBUFFER_WIDTH, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_IMAGE_UNITS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_INTEGER_SAMPLES, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_SAMPLE_MASK_WORDS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_UNIFORM_LOCATIONS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_VERTEX_ATOMIC_COUNTERS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_VERTEX_ATTRIB_BINDINGS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_VERTEX_ATTRIB_STRIDE, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_VERTEX_IMAGE_UNIFORMS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET, stream);

    // GLES31 capabilities are a superset of GLES3
    LogGles3Capabilities(stream);
}

static void LogGles32Capabilities(std::ostream &stream)
{
    QUERY_AND_LOG_CAPABILITY(GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_COMBINED_TESS_CONTROL_UNIFORM_COMPONENTS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_COMBINED_TESS_EVALUATION_UNIFORM_COMPONENTS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_DEBUG_GROUP_STACK_DEPTH, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_DEBUG_LOGGED_MESSAGES, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_DEBUG_MESSAGE_LENGTH, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_FRAGMENT_INTERPOLATION_OFFSET, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_FRAMEBUFFER_LAYERS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_GEOMETRY_ATOMIC_COUNTERS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_GEOMETRY_IMAGE_UNIFORMS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_GEOMETRY_INPUT_COMPONENTS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_GEOMETRY_OUTPUT_COMPONENTS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_GEOMETRY_OUTPUT_VERTICES, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_GEOMETRY_SHADER_INVOCATIONS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_GEOMETRY_UNIFORM_BLOCKS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_GEOMETRY_UNIFORM_COMPONENTS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_LABEL_LENGTH, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_PATCH_VERTICES, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_TESS_CONTROL_ATOMIC_COUNTER_BUFFERS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_TESS_CONTROL_ATOMIC_COUNTERS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_TESS_CONTROL_IMAGE_UNIFORMS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_TESS_CONTROL_INPUT_COMPONENTS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_TESS_CONTROL_OUTPUT_COMPONENTS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_TESS_CONTROL_TEXTURE_IMAGE_UNITS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_TESS_CONTROL_TOTAL_OUTPUT_COMPONENTS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_TESS_CONTROL_UNIFORM_BLOCKS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_TESS_CONTROL_UNIFORM_COMPONENTS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_TESS_EVALUATION_ATOMIC_COUNTER_BUFFERS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_TESS_EVALUATION_ATOMIC_COUNTERS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_TESS_EVALUATION_IMAGE_UNIFORMS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_TESS_EVALUATION_INPUT_COMPONENTS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_TESS_EVALUATION_OUTPUT_COMPONENTS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_TESS_EVALUATION_TEXTURE_IMAGE_UNITS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_TESS_EVALUATION_UNIFORM_BLOCKS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_TESS_EVALUATION_UNIFORM_COMPONENTS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_TESS_GEN_LEVEL, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_TESS_PATCH_COMPONENTS, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MAX_TEXTURE_BUFFER_SIZE, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MIN_FRAGMENT_INTERPOLATION_OFFSET, stream);
    QUERY_AND_LOG_CAPABILITY(GL_MIN_SAMPLE_SHADING_VALUE, stream);

    // GLES32 capabilities are a superset of GLES31
    LogGles31Capabilities(stream);
}

// Prints GLES Capabilities listed at
// https://opengles.gpuinfo.org/listcapabilities.php
// in CSV format
TEST_P(EGLPrintEGLinfoTest, PrintGLESCapabilities)
{
    std::cout << std::endl << "Capability name, value" << std::endl << std::endl;

    std::ostream &stream = std::cout;

    switch (getClientMajorVersion())
    {
        case 1:
            break;
        case 2:
            LogGles2Capabilities(stream);
            break;
        case 3:
            switch (getClientMinorVersion())
            {
                case 2:
                    LogGles32Capabilities(stream);
                    break;
                case 1:
                    LogGles31Capabilities(stream);
                    break;
                case 0:
                    LogGles3Capabilities(stream);
                    break;
                default:
                    FAIL() << "unknown client minor version.";
            }
            break;
        default:
            FAIL() << "unknown client major version.";
    }

    stream << std::endl;
}

// Print the EGL configs with attributes
TEST_P(EGLPrintEGLinfoTest, PrintConfigInfo)
{
    // Get all the configs
    EGLint count;
    EXPECT_EGL_TRUE(eglGetConfigs(mDisplay, nullptr, 0, &count));
    EXPECT_TRUE(count > 0);
    std::vector<EGLConfig> configs(count);
    EXPECT_EGL_TRUE(eglGetConfigs(mDisplay, configs.data(), count, &count));
    configs.resize(count);
    // sort configs by increaing ID
    std::sort(configs.begin(), configs.end(), [this](EGLConfig a, EGLConfig b) -> bool {
        return GetAttrib(mDisplay, a, EGL_CONFIG_ID) < GetAttrib(mDisplay, b, EGL_CONFIG_ID);
    });

    std::cout << "Configs - Count: " << count << std::endl;

    // For each config, print its attributes
    for (auto config : configs)
    {
        // Config ID
        std::cout << "    Config: " << GetAttrib(mDisplay, config, EGL_CONFIG_ID) << std::endl;

        // Color
        const char *componentType = (GetAttrib(mDisplay, config, EGL_COLOR_COMPONENT_TYPE_EXT) ==
                                     EGL_COLOR_COMPONENT_TYPE_FLOAT_EXT)
                                        ? "Float "
                                        : "Fixed ";
        const char *colorBuffType =
            (GetAttrib(mDisplay, config, EGL_COLOR_BUFFER_TYPE) == EGL_LUMINANCE_BUFFER)
                ? "LUMINANCE"
                : "RGB";
        std::cout << "\tColor:" << GetAttrib(mDisplay, config, EGL_BUFFER_SIZE) << "bit "
                  << componentType << colorBuffType
                  << " Red:" << GetAttrib(mDisplay, config, EGL_RED_SIZE)
                  << " Green:" << GetAttrib(mDisplay, config, EGL_GREEN_SIZE)
                  << " Blue:" << GetAttrib(mDisplay, config, EGL_BLUE_SIZE)
                  << " Alpha:" << GetAttrib(mDisplay, config, EGL_ALPHA_SIZE)
                  << " Lum:" << GetAttrib(mDisplay, config, EGL_LUMINANCE_SIZE)
                  << " AlphaMask:" << GetAttrib(mDisplay, config, EGL_ALPHA_MASK_SIZE) << std::endl;

        // Texture Binding
        std::cout << "\tBinding RGB:" << (bool)GetAttrib(mDisplay, config, EGL_BIND_TO_TEXTURE_RGB)
                  << " RGBA:" << (bool)GetAttrib(mDisplay, config, EGL_BIND_TO_TEXTURE_RGBA)
                  << " MaxWidth:" << GetAttrib(mDisplay, config, EGL_MAX_PBUFFER_WIDTH)
                  << " MaxHeight:" << GetAttrib(mDisplay, config, EGL_MAX_PBUFFER_HEIGHT)
                  << " MaxPixels:" << GetAttrib(mDisplay, config, EGL_MAX_PBUFFER_PIXELS)
                  << std::endl;

        // Conformant
        EGLint caveatAttrib = GetAttrib(mDisplay, config, EGL_CONFIG_CAVEAT);
        const char *caveat  = nullptr;
        switch (caveatAttrib)
        {
            case EGL_NONE:
                caveat = "None.";
                break;
            case EGL_SLOW_CONFIG:
                caveat = "Slow.";
                break;
            case EGL_NON_CONFORMANT_CONFIG:
                caveat = "Non-Conformant.";
                break;
            default:
                caveat = ".";
        }
        std::cout << "\tCaveate: " << caveat;

        EGLint conformant = GetAttrib(mDisplay, config, EGL_CONFORMANT);
        std::cout << " Conformant: ";
        if (conformant & EGL_OPENGL_ES_BIT)
            std::cout << "ES1 ";
        if (conformant & EGL_OPENGL_ES2_BIT)
            std::cout << "ES2 ";
        if (conformant & EGL_OPENGL_ES3_BIT)
            std::cout << "ES3";
        std::cout << std::endl;

        // Ancilary buffers
        std::cout << "\tAncilary " << "Depth:" << GetAttrib(mDisplay, config, EGL_DEPTH_SIZE)
                  << " Stencil:" << GetAttrib(mDisplay, config, EGL_STENCIL_SIZE)
                  << " SampleBuffs:" << GetAttrib(mDisplay, config, EGL_SAMPLE_BUFFERS)
                  << " Samples:" << GetAttrib(mDisplay, config, EGL_SAMPLES) << std::endl;

        // Swap interval
        std::cout << "\tSwap Interval"
                  << " Min:" << GetAttrib(mDisplay, config, EGL_MIN_SWAP_INTERVAL)
                  << " Max:" << GetAttrib(mDisplay, config, EGL_MAX_SWAP_INTERVAL) << std::endl;

        // Native
        std::cout << "\tNative Renderable: " << GetAttrib(mDisplay, config, EGL_NATIVE_RENDERABLE)
                  << ", VisualID: " << GetAttrib(mDisplay, config, EGL_NATIVE_VISUAL_ID)
                  << ", VisualType: " << GetAttrib(mDisplay, config, EGL_NATIVE_VISUAL_TYPE)
                  << std::endl;

        // Surface type
        EGLint surfaceType = GetAttrib(mDisplay, config, EGL_SURFACE_TYPE);
        std::cout << "\tSurface Type: ";
        if (surfaceType & EGL_WINDOW_BIT)
            std::cout << "WINDOW ";
        if (surfaceType & EGL_PIXMAP_BIT)
            std::cout << "PIXMAP ";
        if (surfaceType & EGL_PBUFFER_BIT)
            std::cout << "PBUFFER ";
        if (surfaceType & EGL_MULTISAMPLE_RESOLVE_BOX_BIT)
            std::cout << "MULTISAMPLE_RESOLVE_BOX ";
        if (surfaceType & EGL_SWAP_BEHAVIOR_PRESERVED_BIT)
            std::cout << "SWAP_PRESERVE ";
        std::cout << std::endl;

        // Renderable
        EGLint rendType = GetAttrib(mDisplay, config, EGL_RENDERABLE_TYPE);
        std::cout << "\tRender: ";
        if (rendType & EGL_OPENGL_ES_BIT)
            std::cout << "ES1 ";
        if (rendType & EGL_OPENGL_ES2_BIT)
            std::cout << "ES2 ";
        if (rendType & EGL_OPENGL_ES3_BIT)
            std::cout << "ES3 ";
        std::cout << std::endl;

        // Extensions
        if (IsEGLDisplayExtensionEnabled(mDisplay, "EGL_ANDROID_recordable"))
        {
            std::cout << "\tAndroid Recordable: "
                      << GetAttrib(mDisplay, config, EGL_RECORDABLE_ANDROID) << std::endl;
        }
        if (IsEGLDisplayExtensionEnabled(mDisplay, "EGL_ANDROID_framebuffer_target"))
        {
            std::cout << "\tAndroid framebuffer target: "
                      << GetAttrib(mDisplay, config, EGL_FRAMEBUFFER_TARGET_ANDROID) << std::endl;
        }

        // Separator between configs
        std::cout << std::endl;
    }
}

ANGLE_INSTANTIATE_TEST(EGLPrintEGLinfoTest,
                       ES1_VULKAN(),
                       ES1_VULKAN_SWIFTSHADER(),
                       ES2_VULKAN(),
                       ES3_VULKAN(),
                       ES32_VULKAN(),
                       ES31_VULKAN_SWIFTSHADER(),
                       ES32_EGL());

// This test suite is not instantiated on some OSes.
GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(EGLPrintEGLinfoTest);
