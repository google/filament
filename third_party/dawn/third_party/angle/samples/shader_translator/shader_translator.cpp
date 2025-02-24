//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "GLSLANG/ShaderLang.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <vector>
#include "angle_gl.h"

#if defined(ANGLE_ENABLE_VULKAN)
// SPIR-V tools include for disassembly.
#    include <spirv-tools/libspirv.hpp>
#endif

//
// Return codes from main.
//
enum TFailCode
{
    ESuccess = 0,
    EFailUsage,
    EFailCompile,
    EFailCompilerCreate,
};

static void usage();
static sh::GLenum FindShaderType(const char *fileName);
static bool CompileFile(char *fileName, ShHandle compiler, const ShCompileOptions &compileOptions);
static void LogMsg(const char *msg, const char *name, const int num, const char *logName);
static void PrintVariable(const std::string &prefix, size_t index, const sh::ShaderVariable &var);
static void PrintActiveVariables(ShHandle compiler);

// If NUM_SOURCE_STRINGS is set to a value > 1, the input file data is
// broken into that many chunks. This will affect file/line numbering in
// the preprocessor.
const unsigned int NUM_SOURCE_STRINGS = 1;
typedef std::vector<char *> ShaderSource;
static bool ReadShaderSource(const char *fileName, ShaderSource &source);
static void FreeShaderSource(ShaderSource &source);

static bool ParseGLSLOutputVersion(const std::string &, ShShaderOutput *outResult);
static bool ParseIntValue(const std::string &, int emptyDefault, int *outValue);

static void PrintSpirv(const sh::BinaryBlob &blob);

//
// Set up the per compile resources
//
void GenerateResources(ShBuiltInResources *resources)
{
    sh::InitBuiltInResources(resources);

    resources->MaxVertexAttribs             = 8;
    resources->MaxVertexUniformVectors      = 128;
    resources->MaxVaryingVectors            = 8;
    resources->MaxVertexTextureImageUnits   = 0;
    resources->MaxCombinedTextureImageUnits = 8;
    resources->MaxTextureImageUnits         = 8;
    resources->MaxFragmentUniformVectors    = 16;
    resources->MaxDrawBuffers               = 1;
    resources->MaxDualSourceDrawBuffers     = 1;

    resources->OES_standard_derivatives  = 0;
    resources->OES_EGL_image_external    = 0;
    resources->EXT_geometry_shader       = 1;
    resources->ANGLE_texture_multisample = 0;
    resources->APPLE_clip_distance       = 0;
}

int main(int argc, char *argv[])
{
    TFailCode failCode = ESuccess;

    ShCompileOptions compileOptions = {};
    int numCompiles                 = 0;
    ShHandle vertexCompiler         = 0;
    ShHandle fragmentCompiler       = 0;
    ShHandle computeCompiler        = 0;
    ShHandle geometryCompiler       = 0;
    ShHandle tessEvalCompiler       = 0;
    ShHandle tessControlCompiler    = 0;
    ShShaderSpec spec               = SH_GLES2_SPEC;
    ShShaderOutput output           = SH_ESSL_OUTPUT;

    sh::Initialize();

    ShBuiltInResources resources;
    GenerateResources(&resources);

    bool printActiveVariables = false;

    argc--;
    argv++;
    for (; (argc >= 1) && (failCode == ESuccess); argc--, argv++)
    {
        if (argv[0][0] == '-')
        {
            switch (argv[0][1])
            {
                case 'i':
                    compileOptions.intermediateTree = true;
                    break;
                case 'o':
                    compileOptions.objectCode = true;
                    break;
                case 'u':
                    printActiveVariables = true;
                    break;
                case 's':
                    if (argv[0][2] == '=')
                    {
                        switch (argv[0][3])
                        {
                            case 'e':
                                if (argv[0][4] == '3')
                                {
                                    if (argv[0][5] == '1')
                                    {
                                        spec = SH_GLES3_1_SPEC;
                                    }
                                    else if (argv[0][5] == '2')
                                    {
                                        spec = SH_GLES3_2_SPEC;
                                    }
                                    else
                                    {
                                        spec = SH_GLES3_SPEC;
                                    }
                                }
                                else
                                {
                                    spec = SH_GLES2_SPEC;
                                }
                                break;
                            case 'w':
                                if (argv[0][4] == '3')
                                {
                                    spec = SH_WEBGL3_SPEC;
                                }
                                else if (argv[0][4] == '2')
                                {
                                    spec = SH_WEBGL2_SPEC;
                                }
                                else if (argv[0][4] == 'n')
                                {
                                    spec = SH_WEBGL_SPEC;
                                }
                                else
                                {
                                    spec                            = SH_WEBGL_SPEC;
                                    resources.FragmentPrecisionHigh = 1;
                                }
                                break;
                            default:
                                failCode = EFailUsage;
                        }
                    }
                    else
                    {
                        failCode = EFailUsage;
                    }
                    break;
                case 'b':
                    if (argv[0][2] == '=')
                    {
                        switch (argv[0][3])
                        {
                            case 'e':
                                output                                       = SH_ESSL_OUTPUT;
                                compileOptions.initializeUninitializedLocals = true;
                                break;
                            case 'g':
                                if (!ParseGLSLOutputVersion(&argv[0][sizeof("-b=g") - 1], &output))
                                {
                                    failCode = EFailUsage;
                                }
                                compileOptions.initializeUninitializedLocals = true;
                                break;
                            case 'v':
                                output = SH_SPIRV_VULKAN_OUTPUT;
                                compileOptions.initializeUninitializedLocals = true;
                                break;
                            case 'h':
                                if (argv[0][4] == '1' && argv[0][5] == '1')
                                {
                                    output = SH_HLSL_4_1_OUTPUT;
                                }
                                else
                                {
                                    output = SH_HLSL_3_0_OUTPUT;
                                }
                                break;
                            case 'm':
                                output = SH_MSL_METAL_OUTPUT;
                                break;
                            default:
                                failCode = EFailUsage;
                        }
                    }
                    else
                    {
                        failCode = EFailUsage;
                    }
                    break;
                case 'x':
                    if (argv[0][2] == '=')
                    {
                        // clang-format off
                    switch (argv[0][3])
                    {
                      case 'i': resources.OES_EGL_image_external = 1; break;
                      case 'd': resources.OES_standard_derivatives = 1; break;
                      case 'r': resources.ARB_texture_rectangle = 1; break;
                      case 'b':
                          if (ParseIntValue(&argv[0][sizeof("-x=b") - 1], 1,
                                            &resources.MaxDualSourceDrawBuffers))
                          {
                              resources.EXT_blend_func_extended = 1;
                          }
                          else
                          {
                              failCode = EFailUsage;
                          }
                          break;
                      case 'w':
                          if (ParseIntValue(&argv[0][sizeof("-x=w") - 1], 1,
                                            &resources.MaxDrawBuffers))
                          {
                              resources.EXT_draw_buffers = 1;
                          }
                          else
                          {
                              failCode = EFailUsage;
                          }
                          break;
                      case 'g': resources.EXT_frag_depth = 1; break;
                      case 'l': resources.EXT_shader_texture_lod = 1; break;
                      case 'f': resources.EXT_shader_framebuffer_fetch = 1; break;
                      case 'n': resources.NV_shader_framebuffer_fetch = 1; break;
                      case 'a': resources.ARM_shader_framebuffer_fetch = 1; break;
                      case 'm':
                          resources.OVR_multiview2 = 1;
                          resources.OVR_multiview = 1;
                          compileOptions.initializeBuiltinsForInstancedMultiview = true;
                          compileOptions.selectViewInNvGLSLVertexShader = true;
                          break;
                      case 'y': resources.EXT_YUV_target = 1; break;
                      case 's': resources.OES_sample_variables = 1; break;
                      default: failCode = EFailUsage;
                    }
                        // clang-format on
                    }
                    else
                    {
                        failCode = EFailUsage;
                    }
                    break;
                default:
                    failCode = EFailUsage;
            }
        }
        else
        {
            if (spec != SH_GLES2_SPEC && spec != SH_WEBGL_SPEC)
            {
                resources.MaxDrawBuffers             = 8;
                resources.MaxVertexTextureImageUnits = 16;
                resources.MaxTextureImageUnits       = 16;
            }
            ShHandle compiler = 0;
            switch (FindShaderType(argv[0]))
            {
                case GL_VERTEX_SHADER:
                    if (vertexCompiler == 0)
                    {
                        vertexCompiler =
                            sh::ConstructCompiler(GL_VERTEX_SHADER, spec, output, &resources);
                    }
                    compiler = vertexCompiler;
                    break;
                case GL_FRAGMENT_SHADER:
                    if (fragmentCompiler == 0)
                    {
                        fragmentCompiler =
                            sh::ConstructCompiler(GL_FRAGMENT_SHADER, spec, output, &resources);
                    }
                    compiler = fragmentCompiler;
                    break;
                case GL_COMPUTE_SHADER:
                    if (computeCompiler == 0)
                    {
                        computeCompiler =
                            sh::ConstructCompiler(GL_COMPUTE_SHADER, spec, output, &resources);
                    }
                    compiler = computeCompiler;
                    break;
                case GL_GEOMETRY_SHADER_EXT:
                    if (geometryCompiler == 0)
                    {
                        resources.EXT_geometry_shader = 1;
                        geometryCompiler =
                            sh::ConstructCompiler(GL_GEOMETRY_SHADER_EXT, spec, output, &resources);
                    }
                    compiler = geometryCompiler;
                    break;
                case GL_TESS_CONTROL_SHADER_EXT:
                    if (tessControlCompiler == 0)
                    {
                        assert(spec == SH_GLES3_1_SPEC || spec == SH_GLES3_2_SPEC);
                        resources.EXT_tessellation_shader = 1;
                        tessControlCompiler = sh::ConstructCompiler(GL_TESS_CONTROL_SHADER_EXT,
                                                                    spec, output, &resources);
                    }
                    compiler = tessControlCompiler;
                    break;
                case GL_TESS_EVALUATION_SHADER_EXT:
                    if (tessEvalCompiler == 0)
                    {
                        assert(spec == SH_GLES3_1_SPEC || spec == SH_GLES3_2_SPEC);
                        resources.EXT_tessellation_shader = 1;
                        tessEvalCompiler = sh::ConstructCompiler(GL_TESS_EVALUATION_SHADER_EXT,
                                                                 spec, output, &resources);
                    }
                    compiler = tessEvalCompiler;
                    break;
                default:
                    break;
            }
            if (compiler)
            {
                switch (output)
                {
                    case SH_HLSL_3_0_OUTPUT:
                    case SH_HLSL_4_1_OUTPUT:
                        compileOptions.selectViewInNvGLSLVertexShader = false;
                        break;
                    default:
                        break;
                }

                bool compiled = CompileFile(argv[0], compiler, compileOptions);

                LogMsg("BEGIN", "COMPILER", numCompiles, "INFO LOG");
                std::string log = sh::GetInfoLog(compiler);
                puts(log.c_str());
                LogMsg("END", "COMPILER", numCompiles, "INFO LOG");
                printf("\n\n");

                if (compiled && compileOptions.objectCode)
                {
                    LogMsg("BEGIN", "COMPILER", numCompiles, "OBJ CODE");
                    if (output != SH_SPIRV_VULKAN_OUTPUT)
                    {
                        const std::string &code = sh::GetObjectCode(compiler);
                        puts(code.c_str());
                    }
                    else
                    {
                        const sh::BinaryBlob &blob = sh::GetObjectBinaryBlob(compiler);
                        PrintSpirv(blob);
                    }
                    LogMsg("END", "COMPILER", numCompiles, "OBJ CODE");
                    printf("\n\n");
                }
                if (compiled && printActiveVariables)
                {
                    LogMsg("BEGIN", "COMPILER", numCompiles, "VARIABLES");
                    PrintActiveVariables(compiler);
                    LogMsg("END", "COMPILER", numCompiles, "VARIABLES");
                    printf("\n\n");
                }
                if (!compiled)
                    failCode = EFailCompile;
                ++numCompiles;
            }
            else
            {
                failCode = EFailCompilerCreate;
            }
        }
    }

    if ((vertexCompiler == 0) && (fragmentCompiler == 0) && (computeCompiler == 0) &&
        (geometryCompiler == 0) && (tessControlCompiler == 0) && (tessEvalCompiler == 0))
    {
        failCode = EFailUsage;
    }
    if (failCode == EFailUsage)
    {
        usage();
    }

    if (vertexCompiler)
    {
        sh::Destruct(vertexCompiler);
    }
    if (fragmentCompiler)
    {
        sh::Destruct(fragmentCompiler);
    }
    if (computeCompiler)
    {
        sh::Destruct(computeCompiler);
    }
    if (geometryCompiler)
    {
        sh::Destruct(geometryCompiler);
    }
    if (tessControlCompiler)
    {
        sh::Destruct(tessControlCompiler);
    }
    if (tessEvalCompiler)
    {
        sh::Destruct(tessEvalCompiler);
    }

    sh::Finalize();

    return failCode;
}

//
//   print usage to stdout
//
void usage()
{
    // clang-format off
    printf(
        "Usage: translate [-i -o -u -l -b=e -b=g -b=h9 -x=i -x=d] file1 file2 ...\n"
        "Where: filename : filename ending in .frag*, .vert*, .comp*, .geom*, .tcs* or .tes*\n"
        "       -i       : print intermediate tree\n"
        "       -o       : print translated code\n"
        "       -u       : print active attribs, uniforms, varyings and program outputs\n"
        "       -s=e2    : use GLES2 spec (this is by default)\n"
        "       -s=e3    : use GLES3 spec\n"
        "       -s=e31   : use GLES31 spec (in development)\n"
        "       -s=e32   : use GLES32 spec (in development)\n"
        "       -s=w     : use WebGL 1.0 spec\n"
        "       -s=wn    : use WebGL 1.0 spec with no highp support in fragment shaders\n"
        "       -s=w2    : use WebGL 2.0 spec\n"
        "       -b=e     : output GLSL ES code (this is by default)\n"
        "       -b=g     : output GLSL code (compatibility profile)\n"
        "       -b=g[NUM]: output GLSL code (NUM can be 130, 140, 150, 330, 400, 410, 420, 430, "
        "440, 450)\n"
        "       -b=v     : output Vulkan SPIR-V code\n"
        "       -b=h9    : output HLSL9 code\n"
        "       -b=h11   : output HLSL11 code\n"
        "       -b=m     : output MSL code (direct)\n"
        "       -x=i     : enable GL_OES_EGL_image_external\n"
        "       -x=d     : enable GL_OES_EGL_standard_derivatives\n"
        "       -x=r     : enable ARB_texture_rectangle\n"
        "       -x=b[NUM]: enable EXT_blend_func_extended (NUM default 1)\n"
        "       -x=w[NUM]: enable EXT_draw_buffers (NUM default 1)\n"
        "       -x=g     : enable EXT_frag_depth\n"
        "       -x=l     : enable EXT_shader_texture_lod\n"
        "       -x=f     : enable EXT_shader_framebuffer_fetch\n"
        "       -x=n     : enable NV_shader_framebuffer_fetch\n"
        "       -x=a     : enable ARM_shader_framebuffer_fetch\n"
        "       -x=m     : enable OVR_multiview\n"
        "       -x=y     : enable YUV_target\n"
        "       -x=s     : enable OES_sample_variables\n");
    // clang-format on
}

//
//   Deduce the shader type from the filename.  Files must end in one of the
//   following extensions:
//
//   .frag*    = fragment shader
//   .vert*    = vertex shader
//   .comp*    = compute shader
//   .geom*    = geometry shader
//   .tcs*     = tessellation control shader
//   .tes*     = tessellation evaluation shader
//
sh::GLenum FindShaderType(const char *fileName)
{
    assert(fileName);

    const char *ext = strrchr(fileName, '.');

    if (ext && strcmp(ext, ".sl") == 0)
        for (; ext > fileName && ext[0] != '.'; ext--)
            ;

    ext = strrchr(fileName, '.');
    if (ext)
    {
        if (strncmp(ext, ".frag", 5) == 0)
            return GL_FRAGMENT_SHADER;
        if (strncmp(ext, ".vert", 5) == 0)
            return GL_VERTEX_SHADER;
        if (strncmp(ext, ".comp", 5) == 0)
            return GL_COMPUTE_SHADER;
        if (strncmp(ext, ".geom", 5) == 0)
            return GL_GEOMETRY_SHADER_EXT;
        if (strncmp(ext, ".tcs", 5) == 0)
            return GL_TESS_CONTROL_SHADER_EXT;
        if (strncmp(ext, ".tes", 5) == 0)
            return GL_TESS_EVALUATION_SHADER_EXT;
    }

    return GL_FRAGMENT_SHADER;
}

//
//   Read a file's data into a string, and compile it using sh::Compile
//
bool CompileFile(char *fileName, ShHandle compiler, const ShCompileOptions &compileOptions)
{
    ShaderSource source;
    if (!ReadShaderSource(fileName, source))
        return false;

    int ret = sh::Compile(compiler, &source[0], source.size(), compileOptions);

    FreeShaderSource(source);
    return ret ? true : false;
}

void LogMsg(const char *msg, const char *name, const int num, const char *logName)
{
    printf("#### %s %s %d %s ####\n", msg, name, num, logName);
}

void PrintVariable(const std::string &prefix, size_t index, const sh::ShaderVariable &var)
{
    std::string typeName;
    switch (var.type)
    {
        case GL_FLOAT:
            typeName = "GL_FLOAT";
            break;
        case GL_FLOAT_VEC2:
            typeName = "GL_FLOAT_VEC2";
            break;
        case GL_FLOAT_VEC3:
            typeName = "GL_FLOAT_VEC3";
            break;
        case GL_FLOAT_VEC4:
            typeName = "GL_FLOAT_VEC4";
            break;
        case GL_INT:
            typeName = "GL_INT";
            break;
        case GL_INT_VEC2:
            typeName = "GL_INT_VEC2";
            break;
        case GL_INT_VEC3:
            typeName = "GL_INT_VEC3";
            break;
        case GL_INT_VEC4:
            typeName = "GL_INT_VEC4";
            break;
        case GL_UNSIGNED_INT:
            typeName = "GL_UNSIGNED_INT";
            break;
        case GL_UNSIGNED_INT_VEC2:
            typeName = "GL_UNSIGNED_INT_VEC2";
            break;
        case GL_UNSIGNED_INT_VEC3:
            typeName = "GL_UNSIGNED_INT_VEC3";
            break;
        case GL_UNSIGNED_INT_VEC4:
            typeName = "GL_UNSIGNED_INT_VEC4";
            break;
        case GL_BOOL:
            typeName = "GL_BOOL";
            break;
        case GL_BOOL_VEC2:
            typeName = "GL_BOOL_VEC2";
            break;
        case GL_BOOL_VEC3:
            typeName = "GL_BOOL_VEC3";
            break;
        case GL_BOOL_VEC4:
            typeName = "GL_BOOL_VEC4";
            break;
        case GL_FLOAT_MAT2:
            typeName = "GL_FLOAT_MAT2";
            break;
        case GL_FLOAT_MAT3:
            typeName = "GL_FLOAT_MAT3";
            break;
        case GL_FLOAT_MAT4:
            typeName = "GL_FLOAT_MAT4";
            break;
        case GL_FLOAT_MAT2x3:
            typeName = "GL_FLOAT_MAT2x3";
            break;
        case GL_FLOAT_MAT3x2:
            typeName = "GL_FLOAT_MAT3x2";
            break;
        case GL_FLOAT_MAT4x2:
            typeName = "GL_FLOAT_MAT4x2";
            break;
        case GL_FLOAT_MAT2x4:
            typeName = "GL_FLOAT_MAT2x4";
            break;
        case GL_FLOAT_MAT3x4:
            typeName = "GL_FLOAT_MAT3x4";
            break;
        case GL_FLOAT_MAT4x3:
            typeName = "GL_FLOAT_MAT4x3";
            break;

        case GL_SAMPLER_2D:
            typeName = "GL_SAMPLER_2D";
            break;
        case GL_SAMPLER_3D:
            typeName = "GL_SAMPLER_3D";
            break;
        case GL_SAMPLER_CUBE:
            typeName = "GL_SAMPLER_CUBE";
            break;
        case GL_SAMPLER_CUBE_SHADOW:
            typeName = "GL_SAMPLER_CUBE_SHADOW";
            break;
        case GL_SAMPLER_2D_SHADOW:
            typeName = "GL_SAMPLER_2D_ARRAY_SHADOW";
            break;
        case GL_SAMPLER_2D_ARRAY:
            typeName = "GL_SAMPLER_2D_ARRAY";
            break;
        case GL_SAMPLER_2D_ARRAY_SHADOW:
            typeName = "GL_SAMPLER_2D_ARRAY_SHADOW";
            break;
        case GL_SAMPLER_2D_MULTISAMPLE:
            typeName = "GL_SAMPLER_2D_MULTISAMPLE";
            break;
        case GL_IMAGE_2D:
            typeName = "GL_IMAGE_2D";
            break;
        case GL_IMAGE_3D:
            typeName = "GL_IMAGE_3D";
            break;
        case GL_IMAGE_CUBE:
            typeName = "GL_IMAGE_CUBE";
            break;
        case GL_IMAGE_2D_ARRAY:
            typeName = "GL_IMAGE_2D_ARRAY";
            break;

        case GL_INT_SAMPLER_2D:
            typeName = "GL_INT_SAMPLER_2D";
            break;
        case GL_INT_SAMPLER_3D:
            typeName = "GL_INT_SAMPLER_3D";
            break;
        case GL_INT_SAMPLER_CUBE:
            typeName = "GL_INT_SAMPLER_CUBE";
            break;
        case GL_INT_SAMPLER_2D_ARRAY:
            typeName = "GL_INT_SAMPLER_2D_ARRAY";
            break;
        case GL_INT_SAMPLER_2D_MULTISAMPLE:
            typeName = "GL_INT_SAMPLER_2D_MULTISAMPLE";
            break;
        case GL_INT_IMAGE_2D:
            typeName = "GL_INT_IMAGE_2D";
            break;
        case GL_INT_IMAGE_3D:
            typeName = "GL_INT_IMAGE_3D";
            break;
        case GL_INT_IMAGE_CUBE:
            typeName = "GL_INT_IMAGE_CUBE";
            break;
        case GL_INT_IMAGE_2D_ARRAY:
            typeName = "GL_INT_IMAGE_2D_ARRAY";
            break;

        case GL_UNSIGNED_INT_SAMPLER_2D:
            typeName = "GL_UNSIGNED_INT_SAMPLER_2D";
            break;
        case GL_UNSIGNED_INT_SAMPLER_3D:
            typeName = "GL_UNSIGNED_INT_SAMPLER_3D";
            break;
        case GL_UNSIGNED_INT_SAMPLER_CUBE:
            typeName = "GL_UNSIGNED_INT_SAMPLER_CUBE";
            break;
        case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
            typeName = "GL_UNSIGNED_INT_SAMPLER_2D_ARRAY";
            break;
        case GL_UNSIGNED_INT_ATOMIC_COUNTER:
            typeName = "GL_UNSIGNED_INT_ATOMIC_COUNTER";
            break;
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
            typeName = "GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE";
            break;
        case GL_UNSIGNED_INT_IMAGE_2D:
            typeName = "GL_UNSIGNED_INT_IMAGE_2D";
            break;
        case GL_UNSIGNED_INT_IMAGE_3D:
            typeName = "GL_UNSIGNED_INT_IMAGE_3D";
            break;
        case GL_UNSIGNED_INT_IMAGE_CUBE:
            typeName = "GL_UNSIGNED_INT_IMAGE_CUBE";
            break;
        case GL_UNSIGNED_INT_IMAGE_2D_ARRAY:
            typeName = "GL_UNSIGNED_INT_IMAGE_2D_ARRAY";
            break;

        case GL_SAMPLER_EXTERNAL_OES:
            typeName = "GL_SAMPLER_EXTERNAL_OES";
            break;
        case GL_SAMPLER_EXTERNAL_2D_Y2Y_EXT:
            typeName = "GL_SAMPLER_EXTERNAL_2D_Y2Y_EXT";
            break;
        default:
            typeName = "UNKNOWN";
            break;
    }

    printf("%s %u : name=%s, mappedName=%s, type=%s, arraySizes=", prefix.c_str(),
           static_cast<unsigned int>(index), var.name.c_str(), var.mappedName.c_str(),
           typeName.c_str());
    for (unsigned int arraySize : var.arraySizes)
    {
        printf("%u ", arraySize);
    }
    printf("\n");
    if (var.fields.size())
    {
        std::string structPrefix;
        for (size_t i = 0; i < prefix.size(); ++i)
            structPrefix += ' ';
        printf("%s  struct %s\n", structPrefix.c_str(), var.structOrBlockName.c_str());
        structPrefix += "    field";
        for (size_t i = 0; i < var.fields.size(); ++i)
            PrintVariable(structPrefix, i, var.fields[i]);
    }
}

static void PrintActiveVariables(ShHandle compiler)
{
    const std::vector<sh::ShaderVariable> *uniforms       = sh::GetUniforms(compiler);
    const std::vector<sh::ShaderVariable> *inputVaryings  = sh::GetInputVaryings(compiler);
    const std::vector<sh::ShaderVariable> *outputVaryings = sh::GetOutputVaryings(compiler);
    const std::vector<sh::ShaderVariable> *attributes     = sh::GetAttributes(compiler);
    const std::vector<sh::ShaderVariable> *outputs        = sh::GetOutputVariables(compiler);
    for (size_t varCategory = 0; varCategory < 5; ++varCategory)
    {
        size_t numVars = 0;
        std::string varCategoryName;
        if (varCategory == 0)
        {
            numVars         = uniforms->size();
            varCategoryName = "uniform";
        }
        else if (varCategory == 1)
        {
            numVars         = inputVaryings->size();
            varCategoryName = "input varying";
        }
        else if (varCategory == 2)
        {
            numVars         = outputVaryings->size();
            varCategoryName = "output varying";
        }
        else if (varCategory == 3)
        {
            numVars         = attributes->size();
            varCategoryName = "attribute";
        }
        else
        {
            numVars         = outputs->size();
            varCategoryName = "output";
        }

        for (size_t i = 0; i < numVars; ++i)
        {
            const sh::ShaderVariable *var;
            if (varCategory == 0)
                var = &((*uniforms)[i]);
            else if (varCategory == 1)
                var = &((*inputVaryings)[i]);
            else if (varCategory == 2)
                var = &((*outputVaryings)[i]);
            else if (varCategory == 3)
                var = &((*attributes)[i]);
            else
                var = &((*outputs)[i]);

            PrintVariable(varCategoryName, i, *var);
        }
        printf("\n");
    }
}

static bool ReadShaderSource(const char *fileName, ShaderSource &source)
{
    FILE *in = fopen(fileName, "rb");
    if (!in)
    {
        printf("Error: unable to open input file: %s\n", fileName);
        return false;
    }

    // Obtain file size.
    fseek(in, 0, SEEK_END);
    size_t count = ftell(in);
    rewind(in);

    int len = (int)ceil((float)count / (float)NUM_SOURCE_STRINGS);
    source.reserve(NUM_SOURCE_STRINGS);
    // Notice the usage of do-while instead of a while loop here.
    // It is there to handle empty files in which case a single empty
    // string is added to vector.
    do
    {
        char *data   = new char[len + 1];
        size_t nread = fread(data, 1, len, in);
        data[nread]  = '\0';
        source.push_back(data);

        count -= nread;
    } while (count > 0);

    fclose(in);
    return true;
}

static void FreeShaderSource(ShaderSource &source)
{
    for (ShaderSource::size_type i = 0; i < source.size(); ++i)
    {
        delete[] source[i];
    }
    source.clear();
}

static bool ParseGLSLOutputVersion(const std::string &num, ShShaderOutput *outResult)
{
    if (num.length() == 0)
    {
        *outResult = SH_GLSL_COMPATIBILITY_OUTPUT;
        return true;
    }
    std::istringstream input(num);
    int value;
    if (!(input >> value && input.eof()))
    {
        return false;
    }

    switch (value)
    {
        case 130:
            *outResult = SH_GLSL_130_OUTPUT;
            return true;
        case 140:
            *outResult = SH_GLSL_140_OUTPUT;
            return true;
        case 150:
            *outResult = SH_GLSL_150_CORE_OUTPUT;
            return true;
        case 330:
            *outResult = SH_GLSL_330_CORE_OUTPUT;
            return true;
        case 400:
            *outResult = SH_GLSL_400_CORE_OUTPUT;
            return true;
        case 410:
            *outResult = SH_GLSL_410_CORE_OUTPUT;
            return true;
        case 420:
            *outResult = SH_GLSL_420_CORE_OUTPUT;
            return true;
        case 430:
            *outResult = SH_GLSL_430_CORE_OUTPUT;
            return true;
        case 440:
            *outResult = SH_GLSL_440_CORE_OUTPUT;
            return true;
        case 450:
            *outResult = SH_GLSL_450_CORE_OUTPUT;
            return true;
        default:
            break;
    }
    return false;
}

static bool ParseIntValue(const std::string &num, int emptyDefault, int *outValue)
{
    if (num.length() == 0)
    {
        *outValue = emptyDefault;
        return true;
    }

    std::istringstream input(num);
    int value;
    if (!(input >> value && input.eof()))
    {
        return false;
    }
    *outValue = value;
    return true;
}

static void PrintSpirv(const sh::BinaryBlob &blob)
{
#if defined(ANGLE_ENABLE_VULKAN)
    spvtools::SpirvTools spirvTools(SPV_ENV_VULKAN_1_1);

    std::string readableSpirv;
    spirvTools.Disassemble(blob, &readableSpirv,
                           SPV_BINARY_TO_TEXT_OPTION_COMMENT | SPV_BINARY_TO_TEXT_OPTION_INDENT |
                               SPV_BINARY_TO_TEXT_OPTION_NESTED_INDENT);

    puts(readableSpirv.c_str());
#endif
}
