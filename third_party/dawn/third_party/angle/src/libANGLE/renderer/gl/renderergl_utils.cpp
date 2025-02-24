//
// Copyright 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// renderergl_utils.cpp: Conversion functions and other utility routines
// specific to the OpenGL renderer.

#include "libANGLE/renderer/gl/renderergl_utils.h"

#include <array>
#include <limits>

#include "common/android_util.h"
#include "common/mathutil.h"
#include "common/platform.h"
#include "common/string_utils.h"
#include "gpu_info_util/SystemInfo.h"
#include "libANGLE/Buffer.h"
#include "libANGLE/Caps.h"
#include "libANGLE/Context.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/queryconversions.h"
#include "libANGLE/renderer/driver_utils.h"
#include "libANGLE/renderer/gl/ContextGL.h"
#include "libANGLE/renderer/gl/FenceNVGL.h"
#include "libANGLE/renderer/gl/FunctionsGL.h"
#include "libANGLE/renderer/gl/QueryGL.h"
#include "libANGLE/renderer/gl/formatutilsgl.h"
#include "platform/autogen/FeaturesGL_autogen.h"
#include "platform/autogen/FrontendFeatures_autogen.h"

#include <EGL/eglext.h>
#include <algorithm>
#include <sstream>

using angle::CheckedNumeric;

namespace rx
{

namespace
{

const char *GetString(const FunctionsGL *functions, GLenum name)
{
    const char *cStr = reinterpret_cast<const char *>(functions->getString(name));
    if (cStr == nullptr)
    {
        return "";
    }
    return cStr;
}

bool IsMesa(const FunctionsGL *functions, std::array<int, 3> *version)
{
    ASSERT(version);

    std::string nativeVersionString(GetString(functions, GL_VERSION));
    size_t pos = nativeVersionString.find("Mesa");
    if (pos == std::string::npos)
    {
        return false;
    }

    int *data = version->data();
    data[0] = data[1] = data[2] = 0;
    std::sscanf(nativeVersionString.c_str() + pos, "Mesa %d.%d.%d", data, data + 1, data + 2);

    return true;
}

int getAdrenoNumber(const FunctionsGL *functions)
{
    static int number = -1;
    if (number == -1)
    {
        const char *nativeGLRenderer = GetString(functions, GL_RENDERER);
        if (std::sscanf(nativeGLRenderer, "Adreno (TM) %d", &number) < 1 &&
            std::sscanf(nativeGLRenderer, "FD%d", &number) < 1)
        {
            number = 0;
        }
    }
    return number;
}

int GetQualcommVersion(const FunctionsGL *functions)
{
    static int version = -1;
    if (version == -1)
    {
        const std::string nativeVersionString(GetString(functions, GL_VERSION));
        const size_t pos = nativeVersionString.find("V@");
        if (pos == std::string::npos ||
            std::sscanf(nativeVersionString.c_str() + pos, "V@%d", &version) < 1)
        {
            version = 0;
        }
    }
    return version;
}

int getMaliTNumber(const FunctionsGL *functions)
{
    static int number = -1;
    if (number == -1)
    {
        const char *nativeGLRenderer = GetString(functions, GL_RENDERER);
        if (std::sscanf(nativeGLRenderer, "Mali-T%d", &number) < 1)
        {
            number = 0;
        }
    }
    return number;
}

int getMaliGNumber(const FunctionsGL *functions)
{
    static int number = -1;
    if (number == -1)
    {
        const char *nativeGLRenderer = GetString(functions, GL_RENDERER);
        if (std::sscanf(nativeGLRenderer, "Mali-G%d", &number) < 1)
        {
            number = 0;
        }
    }
    return number;
}

bool IsAdreno3xx(const FunctionsGL *functions)
{
    int number = getAdrenoNumber(functions);
    return number != 0 && number >= 300 && number < 400;
}

bool IsAdreno42xOr3xx(const FunctionsGL *functions)
{
    int number = getAdrenoNumber(functions);
    return number != 0 && getAdrenoNumber(functions) < 430;
}

bool IsAdreno4xx(const FunctionsGL *functions)
{
    int number = getAdrenoNumber(functions);
    return number != 0 && number >= 400 && number < 500;
}

bool IsAdreno5xxOrOlder(const FunctionsGL *functions)
{
    int number = getAdrenoNumber(functions);
    return number != 0 && number < 600;
}

bool IsAdreno5xx(const FunctionsGL *functions)
{
    int number = getAdrenoNumber(functions);
    return number != 0 && number >= 500 && number < 600;
}

bool IsMaliT8xxOrOlder(const FunctionsGL *functions)
{
    int number = getMaliTNumber(functions);
    return number != 0 && number < 900;
}

bool IsMaliG31OrOlder(const FunctionsGL *functions)
{
    int number = getMaliGNumber(functions);
    return number != 0 && number <= 31;
}

bool IsMaliG72OrG76OrG51(const FunctionsGL *functions)
{
    int number = getMaliGNumber(functions);
    return number == 72 || number == 76 || number == 51;
}

bool IsMaliValhall(const FunctionsGL *functions)
{
    int number = getMaliGNumber(functions);
    return number == 57 || number == 77 || number == 68 || number == 78 || number == 310 ||
           number == 510 || number == 610 || number == 710 || number == 615 || number == 715;
}

bool IsPixel7OrPixel8(const FunctionsGL *functions)
{
    int number = getMaliGNumber(functions);
    return number == 710 || number == 715;
}

[[maybe_unused]] bool IsAndroidEmulator(const FunctionsGL *functions)
{
    constexpr char androidEmulator[] = "Android Emulator";
    const char *nativeGLRenderer     = GetString(functions, GL_RENDERER);
    return angle::BeginsWith(nativeGLRenderer, androidEmulator);
}

bool IsPowerVrRogue(const FunctionsGL *functions)
{
    constexpr char powerVRRogue[] = "PowerVR Rogue";
    const char *nativeGLRenderer  = GetString(functions, GL_RENDERER);
    return angle::BeginsWith(nativeGLRenderer, powerVRRogue);
}

}  // namespace

SwapControlData::SwapControlData()
    : targetSwapInterval(0), maxSwapInterval(-1), currentSwapInterval(-1)
{}

VendorID GetVendorID(const FunctionsGL *functions)
{
    std::string nativeVendorString(GetString(functions, GL_VENDOR));
    // Concatenate GL_RENDERER to the string being checked because some vendors put their names in
    // GL_RENDERER
    nativeVendorString += " ";
    nativeVendorString += GetString(functions, GL_RENDERER);

    if (nativeVendorString.find("NVIDIA") != std::string::npos)
    {
        return VENDOR_ID_NVIDIA;
    }
    else if (nativeVendorString.find("ATI") != std::string::npos ||
             nativeVendorString.find("AMD") != std::string::npos ||
             nativeVendorString.find("Radeon") != std::string::npos)
    {
        return VENDOR_ID_AMD;
    }
    else if (nativeVendorString.find("Qualcomm") != std::string::npos)
    {
        return VENDOR_ID_QUALCOMM;
    }
    else if (nativeVendorString.find("Intel") != std::string::npos)
    {
        return VENDOR_ID_INTEL;
    }
    else if (nativeVendorString.find("Imagination") != std::string::npos)
    {
        return VENDOR_ID_POWERVR;
    }
    else if (nativeVendorString.find("Vivante") != std::string::npos)
    {
        return VENDOR_ID_VIVANTE;
    }
    else if (nativeVendorString.find("Mali") != std::string::npos)
    {
        return VENDOR_ID_ARM;
    }
    else
    {
        return VENDOR_ID_UNKNOWN;
    }
}

uint32_t GetDeviceID(const FunctionsGL *functions)
{
    std::string nativeRendererString(GetString(functions, GL_RENDERER));
    constexpr std::pair<const char *, uint32_t> kKnownDeviceIDs[] = {
        {"Adreno (TM) 418", ANDROID_DEVICE_ID_NEXUS5X},
        {"Adreno (TM) 530", ANDROID_DEVICE_ID_PIXEL1XL},
        {"Adreno (TM) 540", ANDROID_DEVICE_ID_PIXEL2},
    };

    for (const auto &knownDeviceID : kKnownDeviceIDs)
    {
        if (nativeRendererString.find(knownDeviceID.first) != std::string::npos)
        {
            return knownDeviceID.second;
        }
    }

    return 0;
}

ShShaderOutput GetShaderOutputType(const FunctionsGL *functions)
{
    ASSERT(functions);

    if (functions->standard == STANDARD_GL_DESKTOP)
    {
        // GLSL outputs
        if (functions->isAtLeastGL(gl::Version(4, 5)))
        {
            return SH_GLSL_450_CORE_OUTPUT;
        }
        else if (functions->isAtLeastGL(gl::Version(4, 4)))
        {
            return SH_GLSL_440_CORE_OUTPUT;
        }
        else if (functions->isAtLeastGL(gl::Version(4, 3)))
        {
            return SH_GLSL_430_CORE_OUTPUT;
        }
        else if (functions->isAtLeastGL(gl::Version(4, 2)))
        {
            return SH_GLSL_420_CORE_OUTPUT;
        }
        else if (functions->isAtLeastGL(gl::Version(4, 1)))
        {
            return SH_GLSL_410_CORE_OUTPUT;
        }
        else if (functions->isAtLeastGL(gl::Version(4, 0)))
        {
            return SH_GLSL_400_CORE_OUTPUT;
        }
        else if (functions->isAtLeastGL(gl::Version(3, 3)))
        {
            return SH_GLSL_330_CORE_OUTPUT;
        }
        else if (functions->isAtLeastGL(gl::Version(3, 2)))
        {
            return SH_GLSL_150_CORE_OUTPUT;
        }
        else if (functions->isAtLeastGL(gl::Version(3, 1)))
        {
            return SH_GLSL_140_OUTPUT;
        }
        else if (functions->isAtLeastGL(gl::Version(3, 0)))
        {
            return SH_GLSL_130_OUTPUT;
        }
        else
        {
            return SH_GLSL_COMPATIBILITY_OUTPUT;
        }
    }
    else if (functions->standard == STANDARD_GL_ES)
    {
        // ESSL outputs
        return SH_ESSL_OUTPUT;
    }
    else
    {
        UNREACHABLE();
        return ShShaderOutput(0);
    }
}

namespace nativegl_gl
{

static bool MeetsRequirements(const FunctionsGL *functions,
                              const nativegl::SupportRequirement &requirements)
{
    bool hasRequiredExtensions = false;
    for (const std::vector<std::string> &exts : requirements.requiredExtensions)
    {
        bool hasAllExtensionsInSet = true;
        for (const std::string &extension : exts)
        {
            if (!functions->hasExtension(extension))
            {
                hasAllExtensionsInSet = false;
                break;
            }
        }
        if (hasAllExtensionsInSet)
        {
            hasRequiredExtensions = true;
            break;
        }
    }
    if (!requirements.requiredExtensions.empty() && !hasRequiredExtensions)
    {
        return false;
    }

    if (functions->version >= requirements.version)
    {
        return true;
    }
    else if (!requirements.versionExtensions.empty())
    {
        for (const std::string &extension : requirements.versionExtensions)
        {
            if (!functions->hasExtension(extension))
            {
                return false;
            }
        }
        return true;
    }
    else
    {
        return false;
    }
}

static bool CheckSizedInternalFormatTextureRenderability(const FunctionsGL *functions,
                                                         const angle::FeaturesGL &features,
                                                         GLenum internalFormat)
{
    const gl::InternalFormat &formatInfo = gl::GetSizedInternalFormatInfo(internalFormat);
    ASSERT(formatInfo.sized);

    // Query the current texture so it can be rebound afterwards
    GLint oldTextureBinding = 0;
    functions->getIntegerv(GL_TEXTURE_BINDING_2D, &oldTextureBinding);

    // Create a small texture with the same format and type that gl::Texture would use
    GLuint texture = 0;
    functions->genTextures(1, &texture);
    functions->bindTexture(GL_TEXTURE_2D, texture);

    // Nearest filter needed for framebuffer completeness on some drivers.
    functions->texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    nativegl::TexImageFormat texImageFormat = nativegl::GetTexImageFormat(
        functions, features, formatInfo.internalFormat, formatInfo.format, formatInfo.type);
    constexpr GLsizei kTextureSize = 16;
    functions->texImage2D(GL_TEXTURE_2D, 0, texImageFormat.internalFormat, kTextureSize,
                          kTextureSize, 0, texImageFormat.format, texImageFormat.type, nullptr);

    // Query the current framebuffer so it can be rebound afterwards
    GLint oldFramebufferBinding = 0;
    functions->getIntegerv(GL_FRAMEBUFFER_BINDING, &oldFramebufferBinding);

    // Bind the texture to the framebuffer and check renderability
    GLuint fbo = 0;
    functions->genFramebuffers(1, &fbo);
    functions->bindFramebuffer(GL_FRAMEBUFFER, fbo);
    functions->framebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture,
                                    0);

    bool supported = functions->checkFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;

    // Delete the framebuffer and restore the previous binding
    functions->deleteFramebuffers(1, &fbo);
    functions->bindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(oldFramebufferBinding));

    // Delete the texture and restore the previous binding
    functions->deleteTextures(1, &texture);
    functions->bindTexture(GL_TEXTURE_2D, static_cast<GLuint>(oldTextureBinding));

    if (!supported)
    {
        ANGLE_GL_CLEAR_ERRORS(functions);
    }

    ASSERT(functions->getError() == GL_NO_ERROR);
    return supported;
}

static bool CheckInternalFormatRenderbufferRenderability(const FunctionsGL *functions,
                                                         const angle::FeaturesGL &features,
                                                         GLenum internalFormat)
{
    const gl::InternalFormat &formatInfo = gl::GetSizedInternalFormatInfo(internalFormat);
    ASSERT(formatInfo.sized);

    // Query the current renderbuffer so it can be rebound afterwards
    GLint oldRenderbufferBinding = 0;
    functions->getIntegerv(GL_RENDERBUFFER_BINDING, &oldRenderbufferBinding);

    // Create a small renderbuffer with the same format and type that gl::Renderbuffer would use
    GLuint renderbuffer = 0;
    functions->genRenderbuffers(1, &renderbuffer);
    functions->bindRenderbuffer(GL_RENDERBUFFER, renderbuffer);

    nativegl::RenderbufferFormat renderbufferFormat =
        nativegl::GetRenderbufferFormat(functions, features, formatInfo.internalFormat);
    constexpr GLsizei kRenderbufferSize = 16;
    functions->renderbufferStorage(GL_RENDERBUFFER, renderbufferFormat.internalFormat,
                                   kRenderbufferSize, kRenderbufferSize);

    // Query the current framebuffer so it can be rebound afterwards
    GLint oldFramebufferBinding = 0;
    functions->getIntegerv(GL_FRAMEBUFFER_BINDING, &oldFramebufferBinding);

    // Bind the texture to the framebuffer and check renderability
    GLuint fbo = 0;
    functions->genFramebuffers(1, &fbo);
    functions->bindFramebuffer(GL_FRAMEBUFFER, fbo);
    functions->framebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                                       renderbuffer);

    bool supported = functions->checkFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;

    // Delete the framebuffer and restore the previous binding
    functions->deleteFramebuffers(1, &fbo);
    functions->bindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(oldFramebufferBinding));

    // Delete the renderbuffer and restore the previous binding
    functions->deleteRenderbuffers(1, &renderbuffer);
    functions->bindRenderbuffer(GL_RENDERBUFFER, static_cast<GLuint>(oldRenderbufferBinding));

    if (!supported)
    {
        ANGLE_GL_CLEAR_ERRORS(functions);
    }

    ASSERT(functions->getError() == GL_NO_ERROR);
    return supported;
}

static void LimitVersion(gl::Version *curVersion, const gl::Version &maxVersion)
{
    if (*curVersion >= maxVersion)
    {
        *curVersion = maxVersion;
    }
}

static gl::TextureCaps GenerateTextureFormatCaps(const FunctionsGL *functions,
                                                 const angle::FeaturesGL &features,
                                                 GLenum internalFormat,
                                                 gl::Version *maxSupportedESVersion)
{
    ASSERT(functions->getError() == GL_NO_ERROR);

    gl::TextureCaps textureCaps;

    const nativegl::InternalFormat &formatInfo =
        nativegl::GetInternalFormatInfo(internalFormat, functions->standard);
    textureCaps.texturable = MeetsRequirements(functions, formatInfo.texture);
    textureCaps.filterable =
        textureCaps.texturable && MeetsRequirements(functions, formatInfo.filter);
    textureCaps.textureAttachment = MeetsRequirements(functions, formatInfo.textureAttachment);
    textureCaps.renderbuffer      = MeetsRequirements(functions, formatInfo.renderbuffer);
    textureCaps.blendable         = textureCaps.renderbuffer || textureCaps.textureAttachment;

    // Do extra renderability validation for some formats.
    if (internalFormat == GL_R16F || internalFormat == GL_RG16F || internalFormat == GL_RGB16F)
    {
        // SupportRequirement can't currently express a condition of the form (version && extension)
        // || other extensions, so do the (version && extension) part here.
        if (functions->isAtLeastGLES(gl::Version(3, 0)) &&
            functions->hasGLESExtension("GL_EXT_color_buffer_half_float"))
        {
            textureCaps.textureAttachment = true;
            textureCaps.renderbuffer      = true;
        }
    }

    // We require GL_RGBA16F is renderable to expose EXT_color_buffer_half_float but we can't know
    // if the format is supported unless we try to create a framebuffer.
    // Renderability of signed normalized formats is optional on desktop GL.
    if (internalFormat == GL_RGBA16F ||
        (functions->standard == STANDARD_GL_DESKTOP &&
         (internalFormat == GL_R8_SNORM || internalFormat == GL_R16_SNORM ||
          internalFormat == GL_RG8_SNORM || internalFormat == GL_RG16_SNORM ||
          internalFormat == GL_RGBA8_SNORM || internalFormat == GL_RGBA16_SNORM)))
    {
        if (textureCaps.textureAttachment)
        {
            textureCaps.textureAttachment =
                CheckSizedInternalFormatTextureRenderability(functions, features, internalFormat);
        }
        if (textureCaps.renderbuffer)
        {
            textureCaps.renderbuffer =
                CheckInternalFormatRenderbufferRenderability(functions, features, internalFormat);
        }
    }

    // glGetInternalformativ is not available until version 4.2 but may be available through the 3.0
    // extension GL_ARB_internalformat_query
    if (textureCaps.renderbuffer && functions->getInternalformativ)
    {
        GLenum queryInternalFormat = internalFormat;

        if (internalFormat == GL_BGRA8_EXT)
        {
            // Querying GL_NUM_SAMPLE_COUNTS for GL_BGRA8_EXT generates an INVALID_ENUM on some
            // drivers.  It seems however that allocating a multisampled renderbuffer of this format
            // succeeds. To avoid breaking multisampling for this format, query the supported sample
            // counts for GL_RGBA8 instead.
            queryInternalFormat = GL_RGBA8;
        }

        ANGLE_GL_CLEAR_ERRORS(functions);
        GLint numSamples = 0;
        functions->getInternalformativ(GL_RENDERBUFFER, queryInternalFormat, GL_NUM_SAMPLE_COUNTS,
                                       1, &numSamples);
        GLenum error = functions->getError();
        if (error != GL_NO_ERROR)
        {
            ERR() << "glGetInternalformativ generated error " << gl::FmtHex(error) << " for format "
                  << gl::FmtHex(queryInternalFormat) << ". Skipping multisample checks.";
            numSamples = 0;
        }

        if (numSamples > 0)
        {
            std::vector<GLint> samples(numSamples);
            functions->getInternalformativ(GL_RENDERBUFFER, queryInternalFormat, GL_SAMPLES,
                                           static_cast<GLsizei>(samples.size()), &samples[0]);

            for (size_t sampleIndex = 0; sampleIndex < samples.size(); sampleIndex++)
            {
                if (features.limitMaxMSAASamplesTo4.enabled && samples[sampleIndex] > 4)
                {
                    continue;
                }

                // Some NVIDIA drivers expose multisampling modes implemented as a combination of
                // multisampling and supersampling. These are non-conformant and should not be
                // exposed through ANGLE. Query which formats are conformant from the driver if
                // supported.
                GLint conformant = GL_TRUE;
                if (functions->getInternalformatSampleivNV)
                {
                    ASSERT(functions->getError() == GL_NO_ERROR);
                    functions->getInternalformatSampleivNV(GL_RENDERBUFFER, queryInternalFormat,
                                                           samples[sampleIndex], GL_CONFORMANT_NV,
                                                           1, &conformant);
                    // getInternalFormatSampleivNV does not work for all formats on NVIDIA Shield TV
                    // drivers. Assume that formats with large sample counts are non-conformant in
                    // case the query generates an error.
                    if (functions->getError() != GL_NO_ERROR)
                    {
                        conformant = (samples[sampleIndex] <= 8) ? GL_TRUE : GL_FALSE;
                    }
                }
                if (conformant == GL_TRUE)
                {
                    textureCaps.sampleCounts.insert(samples[sampleIndex]);
                }
            }
        }
    }

    // GLES 3.0.5 section 4.4.2.2: "Implementations must support creation of renderbuffers in these
    // required formats with up to the value of MAX_SAMPLES multisamples, with the exception of
    // signed and unsigned integer formats."
    const gl::InternalFormat &glFormatInfo = gl::GetSizedInternalFormatInfo(internalFormat);
    if (textureCaps.renderbuffer && !glFormatInfo.isInt() &&
        glFormatInfo.isRequiredRenderbufferFormat(gl::Version(3, 0)) &&
        textureCaps.getMaxSamples() < 4)
    {
        LimitVersion(maxSupportedESVersion, gl::Version(2, 0));
    }

    ASSERT(functions->getError() == GL_NO_ERROR);
    return textureCaps;
}

static GLint QuerySingleGLInt(const FunctionsGL *functions, GLenum name)
{
    GLint result = 0;
    functions->getIntegerv(name, &result);
    return result;
}

static GLint QuerySingleIndexGLInt(const FunctionsGL *functions, GLenum name, GLuint index)
{
    GLint result;
    functions->getIntegeri_v(name, index, &result);
    return result;
}

static GLint QueryGLIntRange(const FunctionsGL *functions, GLenum name, size_t index)
{
    GLint result[2] = {};
    functions->getIntegerv(name, result);
    return result[index];
}

static GLint64 QuerySingleGLInt64(const FunctionsGL *functions, GLenum name)
{
    // Fall back to 32-bit int if 64-bit query is not available. This can become relevant for some
    // caps that are defined as 64-bit values in core spec, but were introduced earlier in
    // extensions as 32-bit. Triggered in some cases by RenderDoc's emulated OpenGL driver.
    if (!functions->getInteger64v)
    {
        GLint result = 0;
        functions->getIntegerv(name, &result);
        return static_cast<GLint64>(result);
    }
    else
    {
        GLint64 result = 0;
        functions->getInteger64v(name, &result);
        return result;
    }
}

static GLfloat QuerySingleGLFloat(const FunctionsGL *functions, GLenum name)
{
    GLfloat result = 0.0f;
    functions->getFloatv(name, &result);
    return result;
}

static GLfloat QueryGLFloatRange(const FunctionsGL *functions, GLenum name, size_t index)
{
    GLfloat result[2] = {};
    functions->getFloatv(name, result);
    return result[index];
}

static gl::TypePrecision QueryTypePrecision(const FunctionsGL *functions,
                                            GLenum shaderType,
                                            GLenum precisionType)
{
    gl::TypePrecision precision;
    functions->getShaderPrecisionFormat(shaderType, precisionType, precision.range.data(),
                                        &precision.precision);
    return precision;
}

static GLint QueryQueryValue(const FunctionsGL *functions, GLenum target, GLenum name)
{
    GLint result;
    functions->getQueryiv(target, name, &result);
    return result;
}

void CapCombinedLimitToESShaders(GLint *combinedLimit, gl::ShaderMap<GLint> &perShaderLimit)
{
    GLint combinedESLimit = 0;
    for (gl::ShaderType shaderType : gl::kAllGraphicsShaderTypes)
    {
        combinedESLimit += perShaderLimit[shaderType];
    }

    *combinedLimit = std::min(*combinedLimit, combinedESLimit);
}

void GenerateCaps(const FunctionsGL *functions,
                  const angle::FeaturesGL &features,
                  gl::Caps *caps,
                  gl::TextureCapsMap *textureCapsMap,
                  gl::Extensions *extensions,
                  gl::Limitations *limitations,
                  gl::Version *maxSupportedESVersion,
                  MultiviewImplementationTypeGL *multiviewImplementationType,
                  ShPixelLocalStorageOptions *plsOptions)
{
    // Start by assuming ES3.1 support and work down
    *maxSupportedESVersion = gl::Version(3, 1);

    // Texture format support checks
    const gl::FormatSet &allFormats = gl::GetAllSizedInternalFormats();
    for (GLenum internalFormat : allFormats)
    {
        gl::TextureCaps textureCaps =
            GenerateTextureFormatCaps(functions, features, internalFormat, maxSupportedESVersion);
        textureCapsMap->insert(internalFormat, textureCaps);
    }

    // Table 6.28, implementation dependent values
    if (functions->isAtLeastGL(gl::Version(4, 3)) ||
        functions->hasGLExtension("GL_ARB_ES3_compatibility") ||
        functions->isAtLeastGLES(gl::Version(3, 0)))
    {
        caps->maxElementIndex = QuerySingleGLInt64(functions, GL_MAX_ELEMENT_INDEX);

        // Work around the null driver limitations.
        if (caps->maxElementIndex == 0)
        {
            caps->maxElementIndex = 0xFFFF;
        }
    }
    else
    {
        // Doesn't affect ES3 support, can use a pre-defined limit
        caps->maxElementIndex = static_cast<GLint64>(std::numeric_limits<unsigned int>::max());
    }

    if (features.limitWebglMaxTextureSizeTo4096.enabled)
    {
        limitations->webGLTextureSizeLimit = 4096;
    }
    else if (features.limitWebglMaxTextureSizeTo8192.enabled)
    {
        limitations->webGLTextureSizeLimit = 8192;
    }

    GLint max3dArrayTextureSizeLimit = std::numeric_limits<GLint>::max();
    if (features.limitMax3dArrayTextureSizeTo1024.enabled)
    {
        max3dArrayTextureSizeLimit = 1024;
    }

    if (functions->isAtLeastGL(gl::Version(1, 2)) || functions->isAtLeastGLES(gl::Version(3, 0)) ||
        functions->hasGLESExtension("GL_OES_texture_3D"))
    {
        caps->max3DTextureSize = std::min(
            {QuerySingleGLInt(functions, GL_MAX_3D_TEXTURE_SIZE), max3dArrayTextureSizeLimit});
    }
    else
    {
        // Can't support ES3 without 3D textures
        LimitVersion(maxSupportedESVersion, gl::Version(2, 0));
    }

    caps->max2DTextureSize = QuerySingleGLInt(functions, GL_MAX_TEXTURE_SIZE);  // GL 1.0 / ES 2.0
    caps->maxCubeMapTextureSize =
        QuerySingleGLInt(functions, GL_MAX_CUBE_MAP_TEXTURE_SIZE);  // GL 1.3 / ES 2.0

    if (functions->isAtLeastGL(gl::Version(3, 0)) ||
        functions->hasGLExtension("GL_EXT_texture_array") ||
        functions->isAtLeastGLES(gl::Version(3, 0)))
    {
        caps->maxArrayTextureLayers = std::min(
            {QuerySingleGLInt(functions, GL_MAX_ARRAY_TEXTURE_LAYERS), max3dArrayTextureSizeLimit});
    }
    else
    {
        // Can't support ES3 without array textures
        LimitVersion(maxSupportedESVersion, gl::Version(2, 0));
    }

    if (functions->isAtLeastGL(gl::Version(1, 5)) ||
        functions->hasGLExtension("GL_EXT_texture_lod_bias") ||
        functions->isAtLeastGLES(gl::Version(3, 0)))
    {
        caps->maxLODBias = QuerySingleGLFloat(functions, GL_MAX_TEXTURE_LOD_BIAS);
    }
    else
    {
        LimitVersion(maxSupportedESVersion, gl::Version(2, 0));
    }

    if (functions->isAtLeastGL(gl::Version(3, 0)) ||
        functions->hasGLExtension("GL_EXT_framebuffer_object") ||
        functions->isAtLeastGLES(gl::Version(3, 0)))
    {
        caps->maxRenderbufferSize = QuerySingleGLInt(functions, GL_MAX_RENDERBUFFER_SIZE);
        caps->maxColorAttachments = QuerySingleGLInt(functions, GL_MAX_COLOR_ATTACHMENTS);
    }
    else if (functions->isAtLeastGLES(gl::Version(2, 0)))
    {
        caps->maxRenderbufferSize = QuerySingleGLInt(functions, GL_MAX_RENDERBUFFER_SIZE);
        caps->maxColorAttachments = 1;
    }
    else
    {
        // Can't support ES2 without framebuffers and renderbuffers
        LimitVersion(maxSupportedESVersion, gl::Version(0, 0));
    }

    if (functions->isAtLeastGL(gl::Version(2, 0)) ||
        functions->hasGLExtension("ARB_draw_buffers") ||
        functions->isAtLeastGLES(gl::Version(3, 0)) ||
        functions->hasGLESExtension("GL_EXT_draw_buffers"))
    {
        caps->maxDrawBuffers = QuerySingleGLInt(functions, GL_MAX_DRAW_BUFFERS);
    }
    else
    {
        // Framebuffer is required to have at least one drawbuffer even if the extension is not
        // supported
        caps->maxDrawBuffers = 1;
        LimitVersion(maxSupportedESVersion, gl::Version(2, 0));
    }

    caps->maxViewportWidth =
        QueryGLIntRange(functions, GL_MAX_VIEWPORT_DIMS, 0);  // GL 1.0 / ES 2.0
    caps->maxViewportHeight =
        QueryGLIntRange(functions, GL_MAX_VIEWPORT_DIMS, 1);  // GL 1.0 / ES 2.0

    if (functions->standard == STANDARD_GL_DESKTOP &&
        (functions->profile & GL_CONTEXT_CORE_PROFILE_BIT) != 0)
    {
        // Desktop GL core profile deprecated the GL_ALIASED_POINT_SIZE_RANGE query.  Use
        // GL_POINT_SIZE_RANGE instead.
        caps->minAliasedPointSize =
            std::max(1.0f, QueryGLFloatRange(functions, GL_POINT_SIZE_RANGE, 0));
        caps->maxAliasedPointSize = QueryGLFloatRange(functions, GL_POINT_SIZE_RANGE, 1);
    }
    else
    {
        caps->minAliasedPointSize =
            std::max(1.0f, QueryGLFloatRange(functions, GL_ALIASED_POINT_SIZE_RANGE, 0));
        caps->maxAliasedPointSize = QueryGLFloatRange(functions, GL_ALIASED_POINT_SIZE_RANGE, 1);
    }

    caps->minAliasedLineWidth =
        QueryGLFloatRange(functions, GL_ALIASED_LINE_WIDTH_RANGE, 0);  // GL 1.2 / ES 2.0
    caps->maxAliasedLineWidth =
        QueryGLFloatRange(functions, GL_ALIASED_LINE_WIDTH_RANGE, 1);  // GL 1.2 / ES 2.0

    // Table 6.29, implementation dependent values (cont.)
    if (functions->isAtLeastGL(gl::Version(1, 2)) || functions->isAtLeastGLES(gl::Version(3, 0)))
    {
        caps->maxElementsIndices  = QuerySingleGLInt(functions, GL_MAX_ELEMENTS_INDICES);
        caps->maxElementsVertices = QuerySingleGLInt(functions, GL_MAX_ELEMENTS_VERTICES);
    }
    else
    {
        // Doesn't impact supported version
    }

    if (functions->isAtLeastGL(gl::Version(4, 1)) ||
        functions->hasGLExtension("GL_ARB_get_program_binary") ||
        functions->isAtLeastGLES(gl::Version(3, 0)) ||
        functions->hasGLESExtension("GL_OES_get_program_binary"))
    {
        // Able to support the GL_PROGRAM_BINARY_ANGLE format as long as another program binary
        // format is available.
        GLint numBinaryFormats = QuerySingleGLInt(functions, GL_NUM_PROGRAM_BINARY_FORMATS_OES);
        if (numBinaryFormats > 0)
        {
            caps->programBinaryFormats.push_back(GL_PROGRAM_BINARY_ANGLE);
        }
    }
    else
    {
        // Doesn't impact supported version
    }

    // glGetShaderPrecisionFormat is not available until desktop GL version 4.1 or
    // GL_ARB_ES2_compatibility exists
    if (functions->isAtLeastGL(gl::Version(4, 1)) ||
        functions->hasGLExtension("GL_ARB_ES2_compatibility") ||
        functions->isAtLeastGLES(gl::Version(2, 0)))
    {
        caps->vertexHighpFloat   = QueryTypePrecision(functions, GL_VERTEX_SHADER, GL_HIGH_FLOAT);
        caps->vertexMediumpFloat = QueryTypePrecision(functions, GL_VERTEX_SHADER, GL_MEDIUM_FLOAT);
        caps->vertexLowpFloat    = QueryTypePrecision(functions, GL_VERTEX_SHADER, GL_LOW_FLOAT);
        caps->fragmentHighpFloat = QueryTypePrecision(functions, GL_FRAGMENT_SHADER, GL_HIGH_FLOAT);
        caps->fragmentMediumpFloat =
            QueryTypePrecision(functions, GL_FRAGMENT_SHADER, GL_MEDIUM_FLOAT);
        caps->fragmentLowpFloat  = QueryTypePrecision(functions, GL_FRAGMENT_SHADER, GL_LOW_FLOAT);
        caps->vertexHighpInt     = QueryTypePrecision(functions, GL_VERTEX_SHADER, GL_HIGH_INT);
        caps->vertexMediumpInt   = QueryTypePrecision(functions, GL_VERTEX_SHADER, GL_MEDIUM_INT);
        caps->vertexLowpInt      = QueryTypePrecision(functions, GL_VERTEX_SHADER, GL_LOW_INT);
        caps->fragmentHighpInt   = QueryTypePrecision(functions, GL_FRAGMENT_SHADER, GL_HIGH_INT);
        caps->fragmentMediumpInt = QueryTypePrecision(functions, GL_FRAGMENT_SHADER, GL_MEDIUM_INT);
        caps->fragmentLowpInt    = QueryTypePrecision(functions, GL_FRAGMENT_SHADER, GL_LOW_INT);
    }
    else
    {
        // Doesn't impact supported version, set some default values
        caps->vertexHighpFloat.setIEEEFloat();
        caps->vertexMediumpFloat.setIEEEFloat();
        caps->vertexLowpFloat.setIEEEFloat();
        caps->fragmentHighpFloat.setIEEEFloat();
        caps->fragmentMediumpFloat.setIEEEFloat();
        caps->fragmentLowpFloat.setIEEEFloat();
        caps->vertexHighpInt.setTwosComplementInt(32);
        caps->vertexMediumpInt.setTwosComplementInt(32);
        caps->vertexLowpInt.setTwosComplementInt(32);
        caps->fragmentHighpInt.setTwosComplementInt(32);
        caps->fragmentMediumpInt.setTwosComplementInt(32);
        caps->fragmentLowpInt.setTwosComplementInt(32);
    }

    if (functions->isAtLeastGL(gl::Version(3, 2)) || functions->hasGLExtension("GL_ARB_sync") ||
        functions->isAtLeastGLES(gl::Version(3, 0)))
    {
        // Work around Linux NVIDIA driver bug where GL_TIMEOUT_IGNORED is returned.
        caps->maxServerWaitTimeout =
            std::max<GLint64>(QuerySingleGLInt64(functions, GL_MAX_SERVER_WAIT_TIMEOUT), 0);
    }
    else
    {
        LimitVersion(maxSupportedESVersion, gl::Version(2, 0));
    }

    // Table 6.31, implementation dependent vertex shader limits
    if (functions->isAtLeastGL(gl::Version(2, 0)) || functions->isAtLeastGLES(gl::Version(2, 0)))
    {
        caps->maxVertexAttributes = QuerySingleGLInt(functions, GL_MAX_VERTEX_ATTRIBS);
        caps->maxShaderUniformComponents[gl::ShaderType::Vertex] =
            QuerySingleGLInt(functions, GL_MAX_VERTEX_UNIFORM_COMPONENTS);
        caps->maxShaderTextureImageUnits[gl::ShaderType::Vertex] =
            QuerySingleGLInt(functions, GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS);
    }
    else
    {
        // Can't support ES2 version without these caps
        LimitVersion(maxSupportedESVersion, gl::Version(0, 0));
    }

    if (functions->isAtLeastGL(gl::Version(4, 1)) ||
        functions->hasGLExtension("GL_ARB_ES2_compatibility") ||
        functions->isAtLeastGLES(gl::Version(2, 0)))
    {
        caps->maxVertexUniformVectors = QuerySingleGLInt(functions, GL_MAX_VERTEX_UNIFORM_VECTORS);
        caps->maxFragmentUniformVectors =
            QuerySingleGLInt(functions, GL_MAX_FRAGMENT_UNIFORM_VECTORS);
    }
    else
    {
        // Doesn't limit ES version, GL_MAX_VERTEX_UNIFORM_COMPONENTS / 4 is acceptable.
        caps->maxVertexUniformVectors =
            caps->maxShaderUniformComponents[gl::ShaderType::Vertex] / 4;
        // Doesn't limit ES version, GL_MAX_FRAGMENT_UNIFORM_COMPONENTS / 4 is acceptable.
        caps->maxFragmentUniformVectors =
            caps->maxShaderUniformComponents[gl::ShaderType::Fragment] / 4;
    }

    if (functions->isAtLeastGL(gl::Version(3, 2)) || functions->isAtLeastGLES(gl::Version(3, 0)))
    {
        caps->maxVertexOutputComponents =
            QuerySingleGLInt(functions, GL_MAX_VERTEX_OUTPUT_COMPONENTS);
    }
    else
    {
        // There doesn't seem, to be a desktop extension to add this cap, maybe it could be given a
        // safe limit instead of limiting the supported ES version.
        LimitVersion(maxSupportedESVersion, gl::Version(2, 0));
    }

    // Table 6.32, implementation dependent fragment shader limits
    if (functions->isAtLeastGL(gl::Version(2, 0)) || functions->isAtLeastGLES(gl::Version(2, 0)))
    {
        caps->maxShaderUniformComponents[gl::ShaderType::Fragment] =
            QuerySingleGLInt(functions, GL_MAX_FRAGMENT_UNIFORM_COMPONENTS);
        caps->maxShaderTextureImageUnits[gl::ShaderType::Fragment] =
            QuerySingleGLInt(functions, GL_MAX_TEXTURE_IMAGE_UNITS);
    }
    else
    {
        // Can't support ES2 version without these caps
        LimitVersion(maxSupportedESVersion, gl::Version(0, 0));
    }

    if (functions->isAtLeastGL(gl::Version(3, 2)) || functions->isAtLeastGLES(gl::Version(3, 0)))
    {
        caps->maxFragmentInputComponents =
            QuerySingleGLInt(functions, GL_MAX_FRAGMENT_INPUT_COMPONENTS);
    }
    else
    {
        // There doesn't seem, to be a desktop extension to add this cap, maybe it could be given a
        // safe limit instead of limiting the supported ES version.
        LimitVersion(maxSupportedESVersion, gl::Version(2, 0));
    }

    if (functions->isAtLeastGL(gl::Version(3, 0)) || functions->isAtLeastGLES(gl::Version(3, 0)))
    {
        caps->minProgramTexelOffset = QuerySingleGLInt(functions, GL_MIN_PROGRAM_TEXEL_OFFSET);
        caps->maxProgramTexelOffset = QuerySingleGLInt(functions, GL_MAX_PROGRAM_TEXEL_OFFSET);
    }
    else
    {
        // Can't support ES3 without texel offset, could possibly be emulated in the shader
        LimitVersion(maxSupportedESVersion, gl::Version(2, 0));
    }

    // Table 6.33, implementation dependent aggregate shader limits
    if (functions->isAtLeastGL(gl::Version(3, 1)) ||
        functions->hasGLExtension("GL_ARB_uniform_buffer_object") ||
        functions->isAtLeastGLES(gl::Version(3, 0)))
    {
        caps->maxShaderUniformBlocks[gl::ShaderType::Vertex] =
            QuerySingleGLInt(functions, GL_MAX_VERTEX_UNIFORM_BLOCKS);
        caps->maxShaderUniformBlocks[gl::ShaderType::Fragment] =
            QuerySingleGLInt(functions, GL_MAX_FRAGMENT_UNIFORM_BLOCKS);
        caps->maxUniformBufferBindings =
            QuerySingleGLInt(functions, GL_MAX_UNIFORM_BUFFER_BINDINGS);
        caps->maxUniformBlockSize = QuerySingleGLInt64(functions, GL_MAX_UNIFORM_BLOCK_SIZE);
        caps->uniformBufferOffsetAlignment =
            QuerySingleGLInt(functions, GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT);
        caps->maxCombinedUniformBlocks =
            QuerySingleGLInt(functions, GL_MAX_COMBINED_UNIFORM_BLOCKS);
        caps->maxCombinedShaderUniformComponents[gl::ShaderType::Vertex] =
            QuerySingleGLInt64(functions, GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS);
        caps->maxCombinedShaderUniformComponents[gl::ShaderType::Fragment] =
            QuerySingleGLInt64(functions, GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS);

        // Clamp the maxUniformBlockSize to 64KB (majority of devices support up to this size
        // currently), some drivers expose an excessively large value.
        caps->maxUniformBlockSize = std::min<GLint64>(0x10000, caps->maxUniformBlockSize);
    }
    else
    {
        // Can't support ES3 without uniform blocks
        LimitVersion(maxSupportedESVersion, gl::Version(2, 0));
    }

    if (functions->isAtLeastGL(gl::Version(3, 2)) &&
        (functions->profile & GL_CONTEXT_CORE_PROFILE_BIT) != 0)
    {
        caps->maxVaryingComponents = QuerySingleGLInt(functions, GL_MAX_VERTEX_OUTPUT_COMPONENTS);
    }
    else if (functions->isAtLeastGL(gl::Version(3, 0)) ||
             functions->hasGLExtension("GL_ARB_ES2_compatibility") ||
             functions->isAtLeastGLES(gl::Version(2, 0)))
    {
        caps->maxVaryingComponents = QuerySingleGLInt(functions, GL_MAX_VARYING_COMPONENTS);
    }
    else if (functions->isAtLeastGL(gl::Version(2, 0)))
    {
        caps->maxVaryingComponents = QuerySingleGLInt(functions, GL_MAX_VARYING_FLOATS);
        LimitVersion(maxSupportedESVersion, gl::Version(2, 0));
    }
    else
    {
        LimitVersion(maxSupportedESVersion, gl::Version(0, 0));
    }

    if (functions->isAtLeastGL(gl::Version(4, 1)) ||
        functions->hasGLExtension("GL_ARB_ES2_compatibility") ||
        functions->isAtLeastGLES(gl::Version(2, 0)))
    {
        caps->maxVaryingVectors = QuerySingleGLInt(functions, GL_MAX_VARYING_VECTORS);
    }
    else
    {
        // Doesn't limit ES version, GL_MAX_VARYING_COMPONENTS / 4 is acceptable.
        caps->maxVaryingVectors = caps->maxVaryingComponents / 4;
    }

    // Determine the max combined texture image units by adding the vertex and fragment limits.  If
    // the real cap is queried, it would contain the limits for shader types that are not available
    // to ES.
    caps->maxCombinedTextureImageUnits =
        QuerySingleGLInt(functions, GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS);

    // Table 6.34, implementation dependent transform feedback limits
    if (functions->isAtLeastGL(gl::Version(4, 0)) ||
        functions->hasGLExtension("GL_ARB_transform_feedback2") ||
        functions->isAtLeastGLES(gl::Version(3, 0)))
    {
        caps->maxTransformFeedbackInterleavedComponents =
            QuerySingleGLInt(functions, GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS);
        caps->maxTransformFeedbackSeparateAttributes =
            QuerySingleGLInt(functions, GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS);
        caps->maxTransformFeedbackSeparateComponents =
            QuerySingleGLInt(functions, GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS);
    }
    else
    {
        // Can't support ES3 without transform feedback
        LimitVersion(maxSupportedESVersion, gl::Version(2, 0));
    }

    GLint sampleCountLimit = std::numeric_limits<GLint>::max();
    if (features.limitMaxMSAASamplesTo4.enabled)
    {
        sampleCountLimit = 4;
    }

    // Table 6.35, Framebuffer Dependent Values
    if (functions->isAtLeastGL(gl::Version(3, 0)) ||
        functions->hasGLExtension("GL_EXT_framebuffer_multisample") ||
        functions->isAtLeastGLES(gl::Version(3, 0)) ||
        functions->hasGLESExtension("GL_EXT_multisampled_render_to_texture"))
    {
        caps->maxSamples = std::min(QuerySingleGLInt(functions, GL_MAX_SAMPLES), sampleCountLimit);
    }
    else
    {
        LimitVersion(maxSupportedESVersion, gl::Version(2, 0));
    }

    // Non-constant sampler array indexing is required for OpenGL ES 2 and OpenGL ES after 3.2.
    // However having it available on OpenGL ES 2 is a specification bug, and using this
    // indexing in WebGL is undefined. Requiring this feature would break WebGL 1 for some users
    // so we don't check for it. (it is present with ESSL 100, ESSL >= 320, GLSL >= 400 and
    // GL_ARB_gpu_shader5)

    // Check if sampler objects are supported
    if (!functions->isAtLeastGL(gl::Version(3, 3)) &&
        !functions->hasGLExtension("GL_ARB_sampler_objects") &&
        !functions->isAtLeastGLES(gl::Version(3, 0)))
    {
        // Can't support ES3 without sampler objects
        LimitVersion(maxSupportedESVersion, gl::Version(2, 0));
    }

    // Can't support ES3 without texture swizzling
    if (!functions->isAtLeastGL(gl::Version(3, 3)) &&
        !functions->hasGLExtension("GL_ARB_texture_swizzle") &&
        !functions->hasGLExtension("GL_EXT_texture_swizzle") &&
        !functions->isAtLeastGLES(gl::Version(3, 0)))
    {
        LimitVersion(maxSupportedESVersion, gl::Version(2, 0));

        // Texture swizzling is required to work around the luminance texture format not being
        // present in the core profile
        if (functions->profile & GL_CONTEXT_CORE_PROFILE_BIT)
        {
            LimitVersion(maxSupportedESVersion, gl::Version(0, 0));
        }
    }

    // Can't support ES3 without the GLSL packing builtins. We have a workaround for all
    // desktop OpenGL versions starting from 3.3 with the bit packing extension.
    if (!functions->isAtLeastGL(gl::Version(4, 2)) &&
        !(functions->isAtLeastGL(gl::Version(3, 2)) &&
          functions->hasGLExtension("GL_ARB_shader_bit_encoding")) &&
        !functions->hasGLExtension("GL_ARB_shading_language_packing") &&
        !functions->isAtLeastGLES(gl::Version(3, 0)))
    {
        LimitVersion(maxSupportedESVersion, gl::Version(2, 0));
    }

    // ES3 needs to support explicit layout location qualifiers, while it might be possible to
    // fake them in our side, we currently don't support that.
    if (!functions->isAtLeastGL(gl::Version(3, 3)) &&
        !functions->hasGLExtension("GL_ARB_explicit_attrib_location") &&
        !functions->isAtLeastGLES(gl::Version(3, 0)))
    {
        LimitVersion(maxSupportedESVersion, gl::Version(2, 0));
    }

    if (!functions->isAtLeastGL(gl::Version(4, 3)) &&
        !functions->hasGLExtension("GL_ARB_stencil_texturing") &&
        !functions->isAtLeastGLES(gl::Version(3, 1)))
    {
        LimitVersion(maxSupportedESVersion, gl::Version(3, 0));
    }

    if (functions->isAtLeastGL(gl::Version(4, 3)) || functions->isAtLeastGLES(gl::Version(3, 1)) ||
        functions->hasGLExtension("GL_ARB_framebuffer_no_attachments"))
    {
        caps->maxFramebufferWidth  = QuerySingleGLInt(functions, GL_MAX_FRAMEBUFFER_WIDTH);
        caps->maxFramebufferHeight = QuerySingleGLInt(functions, GL_MAX_FRAMEBUFFER_HEIGHT);
        caps->maxFramebufferSamples =
            std::min(QuerySingleGLInt(functions, GL_MAX_FRAMEBUFFER_SAMPLES), sampleCountLimit);
    }
    else
    {
        LimitVersion(maxSupportedESVersion, gl::Version(3, 0));
    }

    if (functions->isAtLeastGL(gl::Version(3, 2)) || functions->isAtLeastGLES(gl::Version(3, 1)) ||
        functions->hasGLExtension("GL_ARB_texture_multisample"))
    {
        caps->maxSampleMaskWords = QuerySingleGLInt(functions, GL_MAX_SAMPLE_MASK_WORDS);
        caps->maxColorTextureSamples =
            std::min(QuerySingleGLInt(functions, GL_MAX_COLOR_TEXTURE_SAMPLES), sampleCountLimit);
        caps->maxDepthTextureSamples =
            std::min(QuerySingleGLInt(functions, GL_MAX_DEPTH_TEXTURE_SAMPLES), sampleCountLimit);
        caps->maxIntegerSamples =
            std::min(QuerySingleGLInt(functions, GL_MAX_INTEGER_SAMPLES), sampleCountLimit);
    }
    else
    {
        LimitVersion(maxSupportedESVersion, gl::Version(3, 0));
    }

    if (functions->isAtLeastGL(gl::Version(4, 3)) || functions->isAtLeastGLES(gl::Version(3, 1)) ||
        functions->hasGLExtension("GL_ARB_vertex_attrib_binding"))
    {
        caps->maxVertexAttribRelativeOffset =
            QuerySingleGLInt(functions, GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET);
        caps->maxVertexAttribBindings = QuerySingleGLInt(functions, GL_MAX_VERTEX_ATTRIB_BINDINGS);

        // OpenGL 4.3 has no limit on maximum value of stride.
        // [OpenGL 4.3 (Core Profile) - February 14, 2013] Chapter 10.3.1 Page 298
        if (features.emulateMaxVertexAttribStride.enabled ||
            (functions->standard == STANDARD_GL_DESKTOP && functions->version == gl::Version(4, 3)))
        {
            caps->maxVertexAttribStride = 2048;
        }
        else
        {
            caps->maxVertexAttribStride = QuerySingleGLInt(functions, GL_MAX_VERTEX_ATTRIB_STRIDE);
        }
    }
    else
    {
        LimitVersion(maxSupportedESVersion, gl::Version(3, 0));
        // Set maxVertexAttribBindings anyway, a number of places assume this value is at least as
        // much as maxVertexAttributes.
        caps->maxVertexAttribBindings = caps->maxVertexAttributes;
    }

    if (functions->isAtLeastGL(gl::Version(4, 3)) || functions->isAtLeastGLES(gl::Version(3, 1)) ||
        functions->hasGLExtension("GL_ARB_shader_storage_buffer_object"))
    {
        caps->maxCombinedShaderOutputResources =
            QuerySingleGLInt(functions, GL_MAX_COMBINED_SHADER_OUTPUT_RESOURCES);
        caps->maxShaderStorageBlocks[gl::ShaderType::Fragment] =
            QuerySingleGLInt(functions, GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS);
        caps->maxShaderStorageBlocks[gl::ShaderType::Vertex] =
            QuerySingleGLInt(functions, GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS);
        caps->maxShaderStorageBufferBindings =
            QuerySingleGLInt(functions, GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS);
        caps->maxShaderStorageBlockSize =
            QuerySingleGLInt64(functions, GL_MAX_SHADER_STORAGE_BLOCK_SIZE);
        caps->maxCombinedShaderStorageBlocks =
            QuerySingleGLInt(functions, GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS);
        caps->shaderStorageBufferOffsetAlignment =
            QuerySingleGLInt(functions, GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT);
    }
    else
    {
        LimitVersion(maxSupportedESVersion, gl::Version(3, 0));
    }

    if (nativegl::SupportsCompute(functions))
    {
        for (GLuint index = 0u; index < 3u; ++index)
        {
            caps->maxComputeWorkGroupCount[index] =
                QuerySingleIndexGLInt(functions, GL_MAX_COMPUTE_WORK_GROUP_COUNT, index);

            caps->maxComputeWorkGroupSize[index] =
                QuerySingleIndexGLInt(functions, GL_MAX_COMPUTE_WORK_GROUP_SIZE, index);
        }
        caps->maxComputeWorkGroupInvocations =
            QuerySingleGLInt(functions, GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS);
        caps->maxShaderUniformBlocks[gl::ShaderType::Compute] =
            QuerySingleGLInt(functions, GL_MAX_COMPUTE_UNIFORM_BLOCKS);
        caps->maxShaderTextureImageUnits[gl::ShaderType::Compute] =
            QuerySingleGLInt(functions, GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS);
        caps->maxComputeSharedMemorySize =
            QuerySingleGLInt(functions, GL_MAX_COMPUTE_SHARED_MEMORY_SIZE);
        caps->maxShaderUniformComponents[gl::ShaderType::Compute] =
            QuerySingleGLInt(functions, GL_MAX_COMPUTE_UNIFORM_COMPONENTS);
        caps->maxShaderAtomicCounterBuffers[gl::ShaderType::Compute] =
            QuerySingleGLInt(functions, GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS);
        caps->maxShaderAtomicCounters[gl::ShaderType::Compute] =
            QuerySingleGLInt(functions, GL_MAX_COMPUTE_ATOMIC_COUNTERS);
        caps->maxShaderImageUniforms[gl::ShaderType::Compute] =
            QuerySingleGLInt(functions, GL_MAX_COMPUTE_IMAGE_UNIFORMS);
        caps->maxCombinedShaderUniformComponents[gl::ShaderType::Compute] =
            QuerySingleGLInt(functions, GL_MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS);
        caps->maxShaderStorageBlocks[gl::ShaderType::Compute] =
            QuerySingleGLInt(functions, GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS);
    }
    else
    {
        LimitVersion(maxSupportedESVersion, gl::Version(3, 0));
    }

    if (functions->isAtLeastGL(gl::Version(4, 3)) || functions->isAtLeastGLES(gl::Version(3, 1)) ||
        functions->hasGLExtension("GL_ARB_explicit_uniform_location"))
    {
        caps->maxUniformLocations = QuerySingleGLInt(functions, GL_MAX_UNIFORM_LOCATIONS);
    }
    else
    {
        LimitVersion(maxSupportedESVersion, gl::Version(3, 0));
    }

    if (functions->isAtLeastGL(gl::Version(4, 0)) || functions->isAtLeastGLES(gl::Version(3, 1)) ||
        functions->hasGLExtension("GL_ARB_texture_gather"))
    {
        caps->minProgramTextureGatherOffset =
            QuerySingleGLInt(functions, GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET);
        caps->maxProgramTextureGatherOffset =
            QuerySingleGLInt(functions, GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET);
    }
    else
    {
        LimitVersion(maxSupportedESVersion, gl::Version(3, 0));
    }

    if (functions->isAtLeastGL(gl::Version(4, 2)) || functions->isAtLeastGLES(gl::Version(3, 1)) ||
        functions->hasGLExtension("GL_ARB_shader_image_load_store"))
    {
        caps->maxShaderImageUniforms[gl::ShaderType::Vertex] =
            QuerySingleGLInt(functions, GL_MAX_VERTEX_IMAGE_UNIFORMS);
        caps->maxShaderImageUniforms[gl::ShaderType::Fragment] =
            QuerySingleGLInt(functions, GL_MAX_FRAGMENT_IMAGE_UNIFORMS);
        caps->maxImageUnits = QuerySingleGLInt(functions, GL_MAX_IMAGE_UNITS);
        caps->maxCombinedImageUniforms =
            QuerySingleGLInt(functions, GL_MAX_COMBINED_IMAGE_UNIFORMS);
    }
    else
    {
        LimitVersion(maxSupportedESVersion, gl::Version(3, 0));
    }

    if (functions->isAtLeastGL(gl::Version(4, 2)) || functions->isAtLeastGLES(gl::Version(3, 1)) ||
        functions->hasGLExtension("GL_ARB_shader_atomic_counters"))
    {
        caps->maxShaderAtomicCounterBuffers[gl::ShaderType::Vertex] =
            QuerySingleGLInt(functions, GL_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS);
        caps->maxShaderAtomicCounters[gl::ShaderType::Vertex] =
            QuerySingleGLInt(functions, GL_MAX_VERTEX_ATOMIC_COUNTERS);
        caps->maxShaderAtomicCounterBuffers[gl::ShaderType::Fragment] =
            QuerySingleGLInt(functions, GL_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS);
        caps->maxShaderAtomicCounters[gl::ShaderType::Fragment] =
            QuerySingleGLInt(functions, GL_MAX_FRAGMENT_ATOMIC_COUNTERS);
        caps->maxAtomicCounterBufferBindings =
            QuerySingleGLInt(functions, GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS);
        caps->maxAtomicCounterBufferSize =
            QuerySingleGLInt(functions, GL_MAX_ATOMIC_COUNTER_BUFFER_SIZE);
        caps->maxCombinedAtomicCounterBuffers =
            QuerySingleGLInt(functions, GL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS);
        caps->maxCombinedAtomicCounters =
            QuerySingleGLInt(functions, GL_MAX_COMBINED_ATOMIC_COUNTERS);
    }
    else
    {
        LimitVersion(maxSupportedESVersion, gl::Version(3, 0));
    }

    // TODO(geofflang): The gl-uniform-arrays WebGL conformance test struggles to complete on time
    // if the max uniform vectors is too large.  Artificially limit the maximum until the test is
    // updated.
    caps->maxVertexUniformVectors = std::min(1024, caps->maxVertexUniformVectors);
    caps->maxShaderUniformComponents[gl::ShaderType::Vertex] =
        std::min(caps->maxVertexUniformVectors * 4,
                 caps->maxShaderUniformComponents[gl::ShaderType::Vertex]);
    caps->maxFragmentUniformVectors = std::min(1024, caps->maxFragmentUniformVectors);
    caps->maxShaderUniformComponents[gl::ShaderType::Fragment] =
        std::min(caps->maxFragmentUniformVectors * 4,
                 caps->maxShaderUniformComponents[gl::ShaderType::Fragment]);

    // If it is not possible to support reading buffer data back, a shadow copy of the buffers must
    // be held. This disallows writing to buffers indirectly through transform feedback, thus
    // disallowing ES3.
    if (!CanMapBufferForRead(functions))
    {
        LimitVersion(maxSupportedESVersion, gl::Version(2, 0));
    }

    // GL_OES_texture_cube_map_array
    if (functions->isAtLeastGL(gl::Version(4, 0)) ||
        functions->hasGLESExtension("GL_OES_texture_cube_map_array") ||
        functions->hasGLESExtension("GL_EXT_texture_cube_map_array") ||
        functions->hasGLExtension("GL_ARB_texture_cube_map_array") ||
        functions->isAtLeastGLES(gl::Version(3, 2)))
    {
        extensions->textureCubeMapArrayOES = true;
        extensions->textureCubeMapArrayEXT = true;
    }
    else
    {
        // Can't support ES3.2 without cube map array textures
        LimitVersion(maxSupportedESVersion, gl::Version(3, 1));
    }

    if (!nativegl::SupportsVertexArrayObjects(functions) ||
        features.syncAllVertexArraysToDefault.enabled)
    {
        // ES 3.1 vertex bindings are not emulated on the default vertex array
        LimitVersion(maxSupportedESVersion, gl::Version(3, 0));
    }

    // Extension support
    extensions->setTextureExtensionSupport(*textureCapsMap);

    // Expose this extension only when we support the formats or we're running on top of a native
    // ES driver.
    extensions->textureCompressionAstcLdrKHR =
        extensions->textureCompressionAstcLdrKHR &&
        (features.allowAstcFormats.enabled || functions->standard == STANDARD_GL_ES);
    extensions->textureCompressionAstcHdrKHR =
        extensions->textureCompressionAstcLdrKHR &&
        functions->hasExtension("GL_KHR_texture_compression_astc_hdr");
    extensions->textureCompressionAstcSliced3dKHR =
        (extensions->textureCompressionAstcLdrKHR &&
         functions->hasExtension("GL_KHR_texture_compression_astc_sliced_3d")) ||
        extensions->textureCompressionAstcHdrKHR;
    extensions->elementIndexUintOES = functions->standard == STANDARD_GL_DESKTOP ||
                                      functions->isAtLeastGLES(gl::Version(3, 0)) ||
                                      functions->hasGLESExtension("GL_OES_element_index_uint");
    extensions->getProgramBinaryOES = caps->programBinaryFormats.size() > 0;
    extensions->readFormatBgraEXT   = functions->isAtLeastGL(gl::Version(1, 2)) ||
                                    functions->hasGLExtension("GL_EXT_bgra") ||
                                    functions->hasGLESExtension("GL_EXT_read_format_bgra");
    extensions->pixelBufferObjectNV = functions->isAtLeastGL(gl::Version(2, 1)) ||
                                      functions->isAtLeastGLES(gl::Version(3, 0)) ||
                                      functions->hasGLExtension("GL_ARB_pixel_buffer_object") ||
                                      functions->hasGLExtension("GL_EXT_pixel_buffer_object") ||
                                      functions->hasGLESExtension("GL_NV_pixel_buffer_object");
    extensions->syncARB      = nativegl::SupportsFenceSync(functions);
    extensions->mapbufferOES = functions->isAtLeastGL(gl::Version(1, 5)) ||
                               functions->isAtLeastGLES(gl::Version(3, 0)) ||
                               functions->hasGLESExtension("GL_OES_mapbuffer");
    extensions->mapBufferRangeEXT = functions->isAtLeastGL(gl::Version(3, 0)) ||
                                    functions->hasGLExtension("GL_ARB_map_buffer_range") ||
                                    functions->isAtLeastGLES(gl::Version(3, 0)) ||
                                    functions->hasGLESExtension("GL_EXT_map_buffer_range");
    extensions->textureNpotOES = functions->standard == STANDARD_GL_DESKTOP ||
                                 functions->isAtLeastGLES(gl::Version(3, 0)) ||
                                 functions->hasGLESExtension("GL_OES_texture_npot");
    // Note that we could emulate EXT_draw_buffers on ES 3.0's core functionality.
    extensions->drawBuffersEXT = functions->isAtLeastGL(gl::Version(2, 0)) ||
                                 functions->hasGLExtension("ARB_draw_buffers") ||
                                 functions->hasGLESExtension("GL_EXT_draw_buffers");
    extensions->drawBuffersIndexedEXT =
        !features.disableDrawBuffersIndexed.enabled &&
        (functions->isAtLeastGL(gl::Version(4, 0)) ||
         (functions->hasGLExtension("GL_EXT_draw_buffers2") &&
          functions->hasGLExtension("GL_ARB_draw_buffers_blend")) ||
         functions->isAtLeastGLES(gl::Version(3, 2)) ||
         functions->hasGLESExtension("GL_OES_draw_buffers_indexed") ||
         functions->hasGLESExtension("GL_EXT_draw_buffers_indexed"));
    extensions->drawBuffersIndexedOES = extensions->drawBuffersIndexedEXT;
    extensions->textureStorageEXT     = functions->standard == STANDARD_GL_DESKTOP ||
                                    functions->hasGLESExtension("GL_EXT_texture_storage");
    extensions->textureFilterAnisotropicEXT =
        functions->hasGLExtension("GL_EXT_texture_filter_anisotropic") ||
        functions->hasGLESExtension("GL_EXT_texture_filter_anisotropic");
    extensions->occlusionQueryBooleanEXT = nativegl::SupportsOcclusionQueries(functions);
    caps->maxTextureAnisotropy =
        extensions->textureFilterAnisotropicEXT
            ? QuerySingleGLFloat(functions, GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT)
            : 0.0f;
    extensions->fenceNV = FenceNVGL::Supported(functions) || FenceNVSyncGL::Supported(functions);
    extensions->blendMinmaxEXT = functions->isAtLeastGL(gl::Version(1, 5)) ||
                                 functions->hasGLExtension("GL_EXT_blend_minmax") ||
                                 functions->isAtLeastGLES(gl::Version(3, 0)) ||
                                 functions->hasGLESExtension("GL_EXT_blend_minmax");
    extensions->framebufferBlitNV = functions->isAtLeastGL(gl::Version(3, 0)) ||
                                    functions->hasGLExtension("GL_EXT_framebuffer_blit") ||
                                    functions->isAtLeastGLES(gl::Version(3, 0)) ||
                                    functions->hasGLESExtension("GL_NV_framebuffer_blit");
    extensions->framebufferBlitANGLE =
        extensions->framebufferBlitNV || functions->hasGLESExtension("GL_ANGLE_framebuffer_blit");
    extensions->framebufferMultisampleANGLE =
        extensions->framebufferBlitANGLE && caps->maxSamples > 0;
    extensions->multisampledRenderToTextureEXT =
        !features.disableMultisampledRenderToTexture.enabled &&
        (functions->hasGLESExtension("GL_EXT_multisampled_render_to_texture") ||
         functions->hasGLESExtension("GL_IMG_multisampled_render_to_texture"));
    extensions->multisampledRenderToTexture2EXT =
        !features.disableMultisampledRenderToTexture.enabled &&
        extensions->multisampledRenderToTextureEXT &&
        functions->hasGLESExtension("GL_EXT_multisampled_render_to_texture2");
    extensions->standardDerivativesOES = functions->isAtLeastGL(gl::Version(2, 0)) ||
                                         functions->hasGLExtension("GL_ARB_fragment_shader") ||
                                         functions->hasGLESExtension("GL_OES_standard_derivatives");
    extensions->shaderTextureLodEXT = functions->isAtLeastGL(gl::Version(3, 0)) ||
                                      functions->hasGLExtension("GL_ARB_shader_texture_lod") ||
                                      functions->hasGLESExtension("GL_EXT_shader_texture_lod");
    extensions->fragDepthEXT = functions->standard == STANDARD_GL_DESKTOP ||
                               functions->hasGLESExtension("GL_EXT_frag_depth");
    extensions->conservativeDepthEXT = functions->isAtLeastGL(gl::Version(4, 2)) ||
                                       functions->hasGLExtension("GL_ARB_conservative_depth") ||
                                       functions->hasGLESExtension("GL_EXT_conservative_depth");
    extensions->depthClampEXT = functions->isAtLeastGL(gl::Version(3, 2)) ||
                                functions->hasGLExtension("GL_ARB_depth_clamp") ||
                                functions->hasGLESExtension("GL_EXT_depth_clamp");
    extensions->polygonOffsetClampEXT = functions->hasExtension("GL_EXT_polygon_offset_clamp");

    if (functions->standard == STANDARD_GL_DESKTOP)
    {
        extensions->polygonModeNV = true;
    }
    else if (functions->hasGLESExtension("GL_NV_polygon_mode"))
    {
        // Some drivers expose the extension string without supporting its caps.
        ANGLE_GL_CLEAR_ERRORS(functions);
        functions->isEnabled(GL_POLYGON_OFFSET_LINE_NV);
        if (functions->getError() != GL_NO_ERROR)
        {
            WARN() << "Not enabling GL_NV_polygon_mode because "
                      "its native driver support is incomplete.";
        }
        else
        {
            extensions->polygonModeNV = true;
        }
    }
    extensions->polygonModeANGLE = extensions->polygonModeNV;

    // This functionality is provided by Shader Model 5 and should be available in GLSL 4.00
    // or even in older versions with GL_ARB_sample_shading and GL_ARB_gpu_shader5. However,
    // some OpenGL implementations (e.g., macOS) that do not support higher context versions
    // do not pass the tests so GLSL 4.20 is required here.
    extensions->sampleVariablesOES = functions->isAtLeastGL(gl::Version(4, 2)) ||
                                     functions->isAtLeastGLES(gl::Version(3, 2)) ||
                                     functions->hasGLESExtension("GL_OES_sample_variables");
    // Some drivers do not support sample qualifiers in ESSL 3.00, so ESSL 3.10 is required on ES.
    extensions->shaderMultisampleInterpolationOES =
        functions->isAtLeastGL(gl::Version(4, 2)) || functions->isAtLeastGLES(gl::Version(3, 2)) ||
        (functions->isAtLeastGLES(gl::Version(3, 1)) &&
         functions->hasGLESExtension("GL_OES_shader_multisample_interpolation"));
    if (extensions->shaderMultisampleInterpolationOES)
    {
        caps->minInterpolationOffset =
            QuerySingleGLFloat(functions, GL_MIN_FRAGMENT_INTERPOLATION_OFFSET_OES);
        caps->maxInterpolationOffset =
            QuerySingleGLFloat(functions, GL_MAX_FRAGMENT_INTERPOLATION_OFFSET_OES);
        caps->subPixelInterpolationOffsetBits =
            QuerySingleGLInt(functions, GL_FRAGMENT_INTERPOLATION_OFFSET_BITS_OES);
    }

    // Support video texture extension on non Android backends.
    // TODO(crbug.com/776222): support Android and Apple devices.
    extensions->videoTextureWEBGL = !IsAndroid() && !IsApple();

    if (functions->hasGLExtension("GL_ARB_shader_viewport_layer_array") ||
        functions->hasGLExtension("GL_NV_viewport_array2"))
    {
        extensions->multiviewOVR  = true;
        extensions->multiview2OVR = true;
        // GL_MAX_ARRAY_TEXTURE_LAYERS is guaranteed to be at least 256.
        const int maxLayers = QuerySingleGLInt(functions, GL_MAX_ARRAY_TEXTURE_LAYERS);
        // GL_MAX_VIEWPORTS is guaranteed to be at least 16.
        const int maxViewports       = QuerySingleGLInt(functions, GL_MAX_VIEWPORTS);
        caps->maxViews               = static_cast<GLuint>(std::min(maxLayers, maxViewports));
        *multiviewImplementationType = MultiviewImplementationTypeGL::NV_VIEWPORT_ARRAY2;
    }

    extensions->fboRenderMipmapOES = functions->isAtLeastGL(gl::Version(3, 0)) ||
                                     functions->hasGLExtension("GL_EXT_framebuffer_object") ||
                                     functions->isAtLeastGLES(gl::Version(3, 0)) ||
                                     functions->hasGLESExtension("GL_OES_fbo_render_mipmap");

    extensions->textureBorderClampEXT =
        !features.disableTextureClampToBorder.enabled &&
        (functions->standard == STANDARD_GL_DESKTOP ||
         functions->hasGLESExtension("GL_EXT_texture_border_clamp") ||
         functions->hasGLESExtension("GL_OES_texture_border_clamp") ||
         functions->hasGLESExtension("GL_NV_texture_border_clamp"));
    extensions->textureBorderClampOES = extensions->textureBorderClampEXT;

    // This functionality is supported on macOS but the extension
    // strings are not listed there for historical reasons.
    extensions->textureMirrorClampToEdgeEXT =
        !features.disableTextureMirrorClampToEdge.enabled &&
        (IsMac() || functions->isAtLeastGL(gl::Version(4, 4)) ||
         functions->hasGLExtension("GL_ARB_texture_mirror_clamp_to_edge") ||
         functions->hasGLExtension("GL_EXT_texture_mirror_clamp") ||
         functions->hasGLExtension("GL_ATI_texture_mirror_once") ||
         functions->hasGLESExtension("GL_EXT_texture_mirror_clamp_to_edge"));

    extensions->textureShadowLodEXT = functions->hasExtension("GL_EXT_texture_shadow_lod");

    extensions->multiDrawIndirectEXT = true;
    extensions->instancedArraysANGLE = functions->isAtLeastGL(gl::Version(3, 1)) ||
                                       (functions->hasGLExtension("GL_ARB_instanced_arrays") &&
                                        (functions->hasGLExtension("GL_ARB_draw_instanced") ||
                                         functions->hasGLExtension("GL_EXT_draw_instanced"))) ||
                                       functions->isAtLeastGLES(gl::Version(3, 0)) ||
                                       functions->hasGLESExtension("GL_EXT_instanced_arrays");
    extensions->instancedArraysEXT = extensions->instancedArraysANGLE;
    extensions->unpackSubimageEXT  = functions->standard == STANDARD_GL_DESKTOP ||
                                    functions->isAtLeastGLES(gl::Version(3, 0)) ||
                                    functions->hasGLESExtension("GL_EXT_unpack_subimage");
    // Some drivers do not support this extension in ESSL 3.00, so ESSL 3.10 is required on ES.
    extensions->shaderNoperspectiveInterpolationNV =
        functions->isAtLeastGL(gl::Version(3, 0)) ||
        (functions->isAtLeastGLES(gl::Version(3, 1)) &&
         functions->hasGLESExtension("GL_NV_shader_noperspective_interpolation"));
    extensions->packSubimageNV = functions->standard == STANDARD_GL_DESKTOP ||
                                 functions->isAtLeastGLES(gl::Version(3, 0)) ||
                                 functions->hasGLESExtension("GL_NV_pack_subimage");
    extensions->vertexArrayObjectOES = functions->isAtLeastGL(gl::Version(3, 0)) ||
                                       functions->hasGLExtension("GL_ARB_vertex_array_object") ||
                                       functions->isAtLeastGLES(gl::Version(3, 0)) ||
                                       functions->hasGLESExtension("GL_OES_vertex_array_object");
    extensions->debugMarkerEXT = functions->isAtLeastGL(gl::Version(4, 3)) ||
                                 functions->hasGLExtension("GL_KHR_debug") ||
                                 functions->hasGLExtension("GL_EXT_debug_marker") ||
                                 functions->isAtLeastGLES(gl::Version(3, 2)) ||
                                 functions->hasGLESExtension("GL_KHR_debug") ||
                                 functions->hasGLESExtension("GL_EXT_debug_marker");
    extensions->EGLImageOES         = functions->hasGLESExtension("GL_OES_EGL_image");
    extensions->EGLImageExternalOES = functions->hasGLESExtension("GL_OES_EGL_image_external");
    extensions->EGLImageExternalWrapModesEXT =
        functions->hasExtension("GL_EXT_EGL_image_external_wrap_modes");
    extensions->EGLImageExternalEssl3OES =
        functions->hasGLESExtension("GL_OES_EGL_image_external_essl3");
    extensions->EGLImageArrayEXT = functions->hasGLESExtension("GL_EXT_EGL_image_array");

    extensions->EGLSyncOES = functions->hasGLESExtension("GL_OES_EGL_sync");

    if (!features.disableTimestampQueries.enabled &&
        (functions->isAtLeastGL(gl::Version(3, 3)) ||
         functions->hasGLExtension("GL_ARB_timer_query") ||
         functions->hasGLESExtension("GL_EXT_disjoint_timer_query")))
    {
        extensions->disjointTimerQueryEXT = true;

        // If we can't query the counter bits, leave them at 0.
        if (!features.queryCounterBitsGeneratesErrors.enabled)
        {
            caps->queryCounterBitsTimeElapsed =
                QueryQueryValue(functions, GL_TIME_ELAPSED, GL_QUERY_COUNTER_BITS);
            caps->queryCounterBitsTimestamp =
                QueryQueryValue(functions, GL_TIMESTAMP, GL_QUERY_COUNTER_BITS);
        }
    }

    // the EXT_multisample_compatibility is written against ES3.1 but can apply
    // to earlier versions so therefore we're only checking for the extension string
    // and not the specific GLES version.
    extensions->multisampleCompatibilityEXT =
        functions->isAtLeastGL(gl::Version(1, 3)) ||
        functions->hasGLESExtension("GL_EXT_multisample_compatibility");

    extensions->framebufferMixedSamplesCHROMIUM =
        functions->hasGLExtension("GL_NV_framebuffer_mixed_samples") ||
        functions->hasGLESExtension("GL_NV_framebuffer_mixed_samples");

    extensions->robustnessEXT = functions->isAtLeastGL(gl::Version(4, 5)) ||
                                functions->hasGLExtension("GL_KHR_robustness") ||
                                functions->hasGLExtension("GL_ARB_robustness") ||
                                functions->isAtLeastGLES(gl::Version(3, 2)) ||
                                functions->hasGLESExtension("GL_KHR_robustness") ||
                                functions->hasGLESExtension("GL_EXT_robustness");
    extensions->robustnessKHR = functions->isAtLeastGL(gl::Version(4, 5)) ||
                                functions->hasGLExtension("GL_KHR_robustness") ||
                                functions->isAtLeastGLES(gl::Version(3, 2)) ||
                                functions->hasGLESExtension("GL_KHR_robustness");

    extensions->robustBufferAccessBehaviorKHR =
        extensions->robustnessEXT &&
        (functions->hasGLExtension("GL_ARB_robust_buffer_access_behavior") ||
         functions->hasGLESExtension("GL_KHR_robust_buffer_access_behavior"));

    extensions->stencilTexturingANGLE = functions->isAtLeastGL(gl::Version(4, 3)) ||
                                        functions->hasGLExtension("GL_ARB_stencil_texturing") ||
                                        functions->isAtLeastGLES(gl::Version(3, 1));

    if (features.supportsShaderFramebufferFetchEXT.enabled)
    {
        // We can support PLS natively, probably in tiled memory.
        extensions->shaderPixelLocalStorageANGLE         = true;
        extensions->shaderPixelLocalStorageCoherentANGLE = true;
        plsOptions->type             = ShPixelLocalStorageType::FramebufferFetch;
        plsOptions->fragmentSyncType = ShFragmentSynchronizationType::Automatic;
    }
    else
    {
        bool hasFragmentShaderImageLoadStore = false;
        if (functions->isAtLeastGL(gl::Version(4, 2)) ||
            functions->hasGLExtension("GL_ARB_shader_image_load_store"))
        {
            // [ANGLE_shader_pixel_local_storage] "New Implementation Dependent State":
            // MAX_PIXEL_LOCAL_STORAGE_PLANES_ANGLE must be at least 4.
            //
            // MAX_FRAGMENT_IMAGE_UNIFORMS is at least 8 on Desktop Core and ARB.
            hasFragmentShaderImageLoadStore = true;
        }
        else if (functions->isAtLeastGLES(gl::Version(3, 1)))
        {
            // [ANGLE_shader_pixel_local_storage] "New Implementation Dependent State":
            // MAX_PIXEL_LOCAL_STORAGE_PLANES_ANGLE must be at least 4.
            //
            // ES 3.1, Table 20.44: MAX_FRAGMENT_IMAGE_UNIFORMS can be 0.
            hasFragmentShaderImageLoadStore =
                caps->maxShaderImageUniforms[gl::ShaderType::Fragment] >= 4;
        }
        if (hasFragmentShaderImageLoadStore &&
            (features.supportsFragmentShaderInterlockNV.enabled ||
             features.supportsFragmentShaderOrderingINTEL.enabled ||
             features.supportsFragmentShaderInterlockARB.enabled))
        {
            // If shader image load/store can be coherent, prefer it over
            // EXT_shader_framebuffer_fetch_non_coherent.
            extensions->shaderPixelLocalStorageANGLE         = true;
            extensions->shaderPixelLocalStorageCoherentANGLE = true;
            plsOptions->type = ShPixelLocalStorageType::ImageLoadStore;
            // Prefer vendor-specific extensions first. The PixelLocalStorageTest.Coherency test
            // doesn't always pass on Intel when we use the ARB extension.
            if (features.supportsFragmentShaderInterlockNV.enabled)
            {
                plsOptions->fragmentSyncType =
                    ShFragmentSynchronizationType::FragmentShaderInterlock_NV_GL;
            }
            else if (features.supportsFragmentShaderOrderingINTEL.enabled)
            {
                plsOptions->fragmentSyncType =
                    ShFragmentSynchronizationType::FragmentShaderOrdering_INTEL_GL;
            }
            else
            {
                ASSERT(features.supportsFragmentShaderInterlockARB.enabled);
                plsOptions->fragmentSyncType =
                    ShFragmentSynchronizationType::FragmentShaderInterlock_ARB_GL;
            }
            // OpenGL ES only allows read/write access to "r32*" images.
            plsOptions->supportsNativeRGBA8ImageFormats =
                functions->standard != StandardGL::STANDARD_GL_ES;
        }
        else if (features.supportsShaderFramebufferFetchNonCoherentEXT.enabled)
        {
            extensions->shaderPixelLocalStorageANGLE = true;
            plsOptions->type                         = ShPixelLocalStorageType::FramebufferFetch;
        }
        else if (hasFragmentShaderImageLoadStore)
        {
            extensions->shaderPixelLocalStorageANGLE = true;
            plsOptions->type                         = ShPixelLocalStorageType::ImageLoadStore;
            // OpenGL ES only allows read/write access to "r32*" images.
            plsOptions->supportsNativeRGBA8ImageFormats =
                functions->standard != StandardGL::STANDARD_GL_ES;
        }
    }

    // EXT_shader_framebuffer_fetch.
    if (features.supportsShaderFramebufferFetchEXT.enabled)
    {
        extensions->shaderFramebufferFetchEXT = true;
    }

    // TODO(http://anglebug.com/42266350): Support ARM_shader_framebuffer_fetch

    // EXT_shader_framebuffer_fetch_non_coherent.
    if (features.supportsShaderFramebufferFetchNonCoherentEXT.enabled)
    {
        extensions->shaderFramebufferFetchNonCoherentEXT = true;
    }

    extensions->copyTextureCHROMIUM = true;
    extensions->syncQueryCHROMIUM   = SyncQueryGL::IsSupported(functions);

    // Note that OES_texture_storage_multisample_2d_array support could be extended down to GL 3.2
    // if we emulated texStorage* API on top of texImage*.
    extensions->textureStorageMultisample2dArrayOES =
        functions->isAtLeastGL(gl::Version(4, 3)) ||
        functions->hasGLExtension("GL_ARB_texture_storage_multisample") ||
        functions->hasGLESExtension("GL_OES_texture_storage_multisample_2d_array");

    extensions->multiviewMultisampleANGLE = extensions->textureStorageMultisample2dArrayOES &&
                                            (extensions->multiviewOVR || extensions->multiview2OVR);

    extensions->textureMultisampleANGLE = functions->isAtLeastGL(gl::Version(3, 2)) ||
                                          functions->hasGLExtension("GL_ARB_texture_multisample") ||
                                          functions->isAtLeastGLES(gl::Version(3, 1));

    extensions->textureSRGBDecodeEXT = functions->hasGLExtension("GL_EXT_texture_sRGB_decode") ||
                                       functions->hasGLESExtension("GL_EXT_texture_sRGB_decode");

    // ANGLE treats ETC1 as ETC2 for ES 3.0 and higher because it becomes a core format, and they
    // are backwards compatible.
    extensions->compressedETC1RGB8SubTextureEXT =
        extensions->compressedETC2RGB8TextureOES || functions->isAtLeastGLES(gl::Version(3, 0)) ||
        functions->hasGLESExtension("GL_EXT_compressed_ETC1_RGB8_sub_texture");

#if ANGLE_ENABLE_CGL
    VendorID vendor = GetVendorID(functions);
    if ((IsAMD(vendor) || IsIntel(vendor)) && *maxSupportedESVersion >= gl::Version(3, 0))
    {
        // Apple Intel/AMD drivers do not correctly use the TEXTURE_SRGB_DECODE property of
        // sampler states.  Disable this extension when we would advertise any ES version
        // that has samplers.
        extensions->textureSRGBDecodeEXT = false;
    }
#endif

    extensions->sRGBWriteControlEXT = !features.srgbBlendingBroken.enabled &&
                                      (functions->isAtLeastGL(gl::Version(3, 0)) ||
                                       functions->hasGLExtension("GL_EXT_framebuffer_sRGB") ||
                                       functions->hasGLExtension("GL_ARB_framebuffer_sRGB") ||
                                       functions->hasGLESExtension("GL_EXT_sRGB_write_control"));

    if (features.bgraTexImageFormatsBroken.enabled)
    {
        extensions->textureFormatBGRA8888EXT = false;
    }

    // EXT_discard_framebuffer can be implemented as long as glDiscardFramebufferEXT or
    // glInvalidateFramebuffer is available
    extensions->discardFramebufferEXT = functions->isAtLeastGL(gl::Version(4, 3)) ||
                                        functions->hasGLExtension("GL_ARB_invalidate_subdata") ||
                                        functions->isAtLeastGLES(gl::Version(3, 0)) ||
                                        functions->hasGLESExtension("GL_EXT_discard_framebuffer") ||
                                        functions->hasGLESExtension("GL_ARB_invalidate_subdata");

    extensions->translatedShaderSourceANGLE = true;

    if (functions->isAtLeastGL(gl::Version(3, 1)) ||
        functions->hasGLExtension("GL_ARB_texture_rectangle"))
    {
        extensions->textureRectangleANGLE = true;
        caps->maxRectangleTextureSize =
            QuerySingleGLInt(functions, GL_MAX_RECTANGLE_TEXTURE_SIZE_ANGLE);
    }

    // OpenGL 4.3 (and above) and OpenGL ES 3.2 can support all features and constants defined in
    // GL_EXT_geometry_shader.
    bool hasCoreGSSupport =
        functions->isAtLeastGL(gl::Version(4, 3)) || functions->isAtLeastGLES(gl::Version(3, 2));
    // OpenGL 4.0 adds the support for instanced geometry shader
    // GL_ARB_shader_atomic_counters adds atomic counters to geometry shader
    // GL_ARB_shader_storage_buffer_object adds shader storage buffers to geometry shader
    // GL_ARB_shader_image_load_store adds images to geometry shader
    bool hasInstancedGSSupport = functions->isAtLeastGL(gl::Version(4, 0)) &&
                                 functions->hasGLExtension("GL_ARB_shader_atomic_counters") &&
                                 functions->hasGLExtension("GL_ARB_shader_storage_buffer_object") &&
                                 functions->hasGLExtension("GL_ARB_shader_image_load_store");
    if (hasCoreGSSupport || functions->hasGLESExtension("GL_OES_geometry_shader") ||
        functions->hasGLESExtension("GL_EXT_geometry_shader") || hasInstancedGSSupport)
    {
        extensions->geometryShaderEXT = functions->hasGLESExtension("GL_EXT_geometry_shader") ||
                                        hasCoreGSSupport || hasInstancedGSSupport;
        extensions->geometryShaderOES = functions->hasGLESExtension("GL_OES_geometry_shader") ||
                                        hasCoreGSSupport || hasInstancedGSSupport;

        caps->maxFramebufferLayers = QuerySingleGLInt(functions, GL_MAX_FRAMEBUFFER_LAYERS_EXT);

        // GL_PROVOKING_VERTEX isn't a valid return value of GL_LAYER_PROVOKING_VERTEX_EXT in
        // GL_EXT_geometry_shader SPEC, however it is legal in desktop OpenGL, which means the value
        // follows the one set by glProvokingVertex.
        // [OpenGL 4.3] Chapter 11.3.4.6
        // The vertex conventions followed for gl_Layer and gl_ViewportIndex may be determined by
        // calling GetIntegerv with the symbolic constants LAYER_PROVOKING_VERTEX and
        // VIEWPORT_INDEX_PROVOKING_VERTEX, respectively. For either query, if the value returned is
        // PROVOKING_VERTEX, then vertex selection follows the convention specified by
        // ProvokingVertex.
        caps->layerProvokingVertex = QuerySingleGLInt(functions, GL_LAYER_PROVOKING_VERTEX_EXT);
        if (caps->layerProvokingVertex == GL_PROVOKING_VERTEX)
        {
            // We should use GL_LAST_VERTEX_CONVENTION_EXT instead because desktop OpenGL SPEC
            // requires the initial value of provoking vertex mode is LAST_VERTEX_CONVENTION.
            // [OpenGL 4.3] Chapter 13.4
            // The initial value of the provoking vertex mode is LAST_VERTEX_CONVENTION.
            caps->layerProvokingVertex = GL_LAST_VERTEX_CONVENTION_EXT;
        }

        caps->maxShaderUniformComponents[gl::ShaderType::Geometry] =
            QuerySingleGLInt(functions, GL_MAX_GEOMETRY_UNIFORM_COMPONENTS_EXT);
        caps->maxShaderUniformBlocks[gl::ShaderType::Geometry] =
            QuerySingleGLInt(functions, GL_MAX_GEOMETRY_UNIFORM_BLOCKS_EXT);
        caps->maxCombinedShaderUniformComponents[gl::ShaderType::Geometry] =
            QuerySingleGLInt(functions, GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS_EXT);
        caps->maxGeometryInputComponents =
            QuerySingleGLInt(functions, GL_MAX_GEOMETRY_INPUT_COMPONENTS_EXT);
        caps->maxGeometryOutputComponents =
            QuerySingleGLInt(functions, GL_MAX_GEOMETRY_OUTPUT_COMPONENTS_EXT);
        caps->maxGeometryOutputVertices =
            QuerySingleGLInt(functions, GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT);
        caps->maxGeometryTotalOutputComponents =
            QuerySingleGLInt(functions, GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_EXT);
        caps->maxGeometryShaderInvocations =
            QuerySingleGLInt(functions, GL_MAX_GEOMETRY_SHADER_INVOCATIONS_EXT);
        caps->maxShaderTextureImageUnits[gl::ShaderType::Geometry] =
            QuerySingleGLInt(functions, GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_EXT);
        caps->maxShaderAtomicCounterBuffers[gl::ShaderType::Geometry] =
            QuerySingleGLInt(functions, GL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS_EXT);
        caps->maxShaderAtomicCounters[gl::ShaderType::Geometry] =
            QuerySingleGLInt(functions, GL_MAX_GEOMETRY_ATOMIC_COUNTERS_EXT);
        caps->maxShaderImageUniforms[gl::ShaderType::Geometry] =
            QuerySingleGLInt(functions, GL_MAX_GEOMETRY_IMAGE_UNIFORMS_EXT);
        caps->maxShaderStorageBlocks[gl::ShaderType::Geometry] =
            QuerySingleGLInt(functions, GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS_EXT);
    }

    // The real combined caps contain limits for shader types that are not available to ES, so limit
    // the caps to the sum of vertex+fragment+geometry shader caps.
    CapCombinedLimitToESShaders(&caps->maxCombinedUniformBlocks, caps->maxShaderUniformBlocks);
    CapCombinedLimitToESShaders(&caps->maxCombinedTextureImageUnits,
                                caps->maxShaderTextureImageUnits);
    CapCombinedLimitToESShaders(&caps->maxCombinedShaderStorageBlocks,
                                caps->maxShaderStorageBlocks);
    CapCombinedLimitToESShaders(&caps->maxCombinedImageUniforms, caps->maxShaderImageUniforms);
    CapCombinedLimitToESShaders(&caps->maxCombinedAtomicCounterBuffers,
                                caps->maxShaderAtomicCounterBuffers);
    CapCombinedLimitToESShaders(&caps->maxCombinedAtomicCounters, caps->maxShaderAtomicCounters);

    // EXT_blend_func_extended is not implemented on top of ARB_blend_func_extended
    // because the latter does not support fragment shader output layout qualifiers.
    extensions->blendFuncExtendedEXT = !features.disableBlendFuncExtended.enabled &&
                                       (functions->isAtLeastGL(gl::Version(3, 3)) ||
                                        functions->hasGLESExtension("GL_EXT_blend_func_extended"));
    if (extensions->blendFuncExtendedEXT)
    {
        // TODO(http://anglebug.com/40644593): Support greater values of
        // MAX_DUAL_SOURCE_DRAW_BUFFERS_EXT queried from the driver. See comments in ProgramGL.cpp
        // for more information about this limitation.
        caps->maxDualSourceDrawBuffers = 1;
    }

    // EXT_float_blend
    // Assume all desktop driver supports this by default.
    extensions->floatBlendEXT = functions->standard == STANDARD_GL_DESKTOP ||
                                functions->hasGLESExtension("GL_EXT_float_blend") ||
                                functions->isAtLeastGLES(gl::Version(3, 2));

    // ANGLE_base_vertex_base_instance
    extensions->baseVertexBaseInstanceANGLE =
        !features.disableBaseInstanceVertex.enabled &&
        (functions->isAtLeastGL(gl::Version(3, 2)) || functions->isAtLeastGLES(gl::Version(3, 2)) ||
         functions->hasGLESExtension("GL_OES_draw_elements_base_vertex") ||
         functions->hasGLESExtension("GL_EXT_draw_elements_base_vertex"));

    // EXT_base_instance
    extensions->baseInstanceEXT =
        !features.disableBaseInstanceVertex.enabled &&
        (functions->isAtLeastGL(gl::Version(3, 2)) || functions->isAtLeastGLES(gl::Version(3, 2)) ||
         functions->hasGLESExtension("GL_OES_draw_elements_base_vertex") ||
         functions->hasGLESExtension("GL_EXT_draw_elements_base_vertex") ||
         functions->hasGLESExtension("GL_EXT_base_instance"));

    // ANGLE_base_vertex_base_instance_shader_builtin
    extensions->baseVertexBaseInstanceShaderBuiltinANGLE = extensions->baseVertexBaseInstanceANGLE;

    // OES_draw_elements_base_vertex
    extensions->drawElementsBaseVertexOES =
        functions->isAtLeastGL(gl::Version(3, 2)) || functions->isAtLeastGLES(gl::Version(3, 2)) ||
        functions->hasGLESExtension("GL_OES_draw_elements_base_vertex");

    // EXT_draw_elements_base_vertex
    extensions->drawElementsBaseVertexEXT =
        functions->isAtLeastGL(gl::Version(3, 2)) || functions->isAtLeastGLES(gl::Version(3, 2)) ||
        functions->hasGLESExtension("GL_EXT_draw_elements_base_vertex");

    // ANGLE_compressed_texture_etc
    // Expose this extension only when we support the formats or we're running on top of a native
    // ES driver.
    extensions->compressedTextureEtcANGLE =
        (features.allowETCFormats.enabled || functions->standard == STANDARD_GL_ES) &&
        gl::DetermineCompressedTextureETCSupport(*textureCapsMap);

    // When running on top of desktop OpenGL drivers and allow_etc_formats feature is not enabled,
    // mark ETC1 as emulated to hide it from WebGL clients.
    limitations->emulatedEtc1 =
        !features.allowETCFormats.enabled && functions->standard == STANDARD_GL_DESKTOP;

    // To work around broken unsized sRGB textures, sized sRGB textures are used. Disable EXT_sRGB
    // if those formats are not available.
    if (features.unsizedSRGBReadPixelsDoesntTransform.enabled &&
        !functions->isAtLeastGLES(gl::Version(3, 0)))
    {
        extensions->sRGBEXT = false;
    }

    extensions->provokingVertexANGLE = functions->hasGLExtension("GL_ARB_provoking_vertex") ||
                                       functions->hasGLExtension("GL_EXT_provoking_vertex") ||
                                       functions->isAtLeastGL(gl::Version(3, 2));

    extensions->textureExternalUpdateANGLE = true;
    extensions->texture3DOES               = functions->isAtLeastGL(gl::Version(1, 2)) ||
                               functions->isAtLeastGLES(gl::Version(3, 0)) ||
                               functions->hasGLESExtension("GL_OES_texture_3D");

    extensions->memoryObjectEXT = functions->hasGLExtension("GL_EXT_memory_object") ||
                                  functions->hasGLESExtension("GL_EXT_memory_object");
    extensions->semaphoreEXT = functions->hasGLExtension("GL_EXT_semaphore") ||
                               functions->hasGLESExtension("GL_EXT_semaphore");
    extensions->memoryObjectFdEXT = functions->hasGLExtension("GL_EXT_memory_object_fd") ||
                                    functions->hasGLESExtension("GL_EXT_memory_object_fd");
    extensions->semaphoreFdEXT = !features.disableSemaphoreFd.enabled &&
                                 (functions->hasGLExtension("GL_EXT_semaphore_fd") ||
                                  functions->hasGLESExtension("GL_EXT_semaphore_fd"));
    extensions->gpuShader5EXT = functions->isAtLeastGL(gl::Version(4, 0)) ||
                                functions->isAtLeastGLES(gl::Version(3, 2)) ||
                                functions->hasGLExtension("GL_ARB_gpu_shader5") ||
                                functions->hasGLESExtension("GL_EXT_gpu_shader5") ||
                                functions->hasGLESExtension("GL_OES_gpu_shader5");
    extensions->gpuShader5OES     = extensions->gpuShader5EXT;
    extensions->shaderIoBlocksOES = functions->isAtLeastGL(gl::Version(3, 2)) ||
                                    functions->isAtLeastGLES(gl::Version(3, 2)) ||
                                    functions->hasGLESExtension("GL_OES_shader_io_blocks") ||
                                    functions->hasGLESExtension("GL_EXT_shader_io_blocks");
    extensions->shaderIoBlocksEXT = extensions->shaderIoBlocksOES;

    extensions->shadowSamplersEXT = functions->isAtLeastGL(gl::Version(2, 0)) ||
                                    functions->isAtLeastGLES(gl::Version(3, 0)) ||
                                    functions->hasGLESExtension("GL_EXT_shadow_samplers");

    if (!features.disableClipControl.enabled)
    {
        extensions->clipControlEXT = functions->isAtLeastGL(gl::Version(4, 5)) ||
                                     functions->hasGLExtension("GL_ARB_clip_control") ||
                                     functions->hasGLESExtension("GL_EXT_clip_control");
    }

    if (features.disableRenderSnorm.enabled)
    {
        extensions->renderSnormEXT = false;
    }

    constexpr uint32_t kRequiredClipDistances                = 8;
    constexpr uint32_t kRequiredCullDistances                = 8;
    constexpr uint32_t kRequiredCombinedClipAndCullDistances = 8;

    // GL_APPLE_clip_distance cannot be implemented on top of GL_EXT_clip_cull_distance,
    // so require either native support or desktop GL.
    extensions->clipDistanceAPPLE = functions->isAtLeastGL(gl::Version(3, 0)) ||
                                    functions->hasGLESExtension("GL_APPLE_clip_distance");
    if (extensions->clipDistanceAPPLE)
    {
        caps->maxClipDistances = QuerySingleGLInt(functions, GL_MAX_CLIP_DISTANCES_APPLE);

        if (caps->maxClipDistances < kRequiredClipDistances)
        {
            WARN() << "Disabling GL_APPLE_clip_distance because only " << caps->maxClipDistances
                   << " clip distances are supported by the driver.";
            extensions->clipDistanceAPPLE = false;
        }
    }

    // GL_EXT_clip_cull_distance spec requires shader interface blocks to support
    // built-in array redeclarations on OpenGL ES.
    extensions->clipCullDistanceEXT =
        functions->isAtLeastGL(gl::Version(4, 5)) ||
        (functions->isAtLeastGL(gl::Version(3, 0)) &&
         functions->hasGLExtension("GL_ARB_cull_distance")) ||
        (extensions->shaderIoBlocksEXT && functions->hasGLESExtension("GL_EXT_clip_cull_distance"));
    if (extensions->clipCullDistanceEXT)
    {
        caps->maxClipDistances = QuerySingleGLInt(functions, GL_MAX_CLIP_DISTANCES_EXT);
        caps->maxCullDistances = QuerySingleGLInt(functions, GL_MAX_CULL_DISTANCES_EXT);
        caps->maxCombinedClipAndCullDistances =
            QuerySingleGLInt(functions, GL_MAX_COMBINED_CLIP_AND_CULL_DISTANCES_EXT);

        if (caps->maxClipDistances < kRequiredClipDistances ||
            caps->maxCullDistances < kRequiredCullDistances ||
            caps->maxCombinedClipAndCullDistances < kRequiredCombinedClipAndCullDistances)
        {
            WARN() << "Disabling GL_EXT_clip_cull_distance because only " << caps->maxClipDistances
                   << " clip distances, " << caps->maxCullDistances << " cull distances and "
                   << caps->maxCombinedClipAndCullDistances
                   << " combined clip/cull distances are supported by the driver.";
            extensions->clipCullDistanceEXT = false;
        }
    }

    // Same as GL_EXT_clip_cull_distance but with cull distance support being optional.
    extensions->clipCullDistanceANGLE =
        (functions->isAtLeastGL(gl::Version(3, 0)) || extensions->clipCullDistanceEXT) &&
        caps->maxClipDistances >= kRequiredClipDistances;
    ASSERT(!extensions->clipCullDistanceANGLE || caps->maxClipDistances > 0);

    // GL_OES_shader_image_atomic
    //
    // Note that imageAtomicExchange() is allowed to accept float textures (of r32f format) in this
    // extension, but that's not supported by ARB_shader_image_load_store which this extension is
    // based on, neither in the spec it was merged into it.  This support was only added to desktop
    // GLSL in version 4.5
    if (functions->isAtLeastGL(gl::Version(4, 5)) || functions->isAtLeastGLES(gl::Version(3, 2)) ||
        functions->hasGLESExtension("GL_OES_shader_image_atomic"))
    {
        extensions->shaderImageAtomicOES = true;
    }

    // GL_OES_texture_buffer
    if (functions->isAtLeastGL(gl::Version(4, 3)) || functions->isAtLeastGLES(gl::Version(3, 2)) ||
        functions->hasGLESExtension("GL_OES_texture_buffer") ||
        functions->hasGLESExtension("GL_EXT_texture_buffer") ||
        functions->hasGLExtension("GL_ARB_texture_buffer_object"))
    {
        caps->maxTextureBufferSize = QuerySingleGLInt(functions, GL_MAX_TEXTURE_BUFFER_SIZE);
        caps->textureBufferOffsetAlignment =
            QuerySingleGLInt(functions, GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT);
        extensions->textureBufferOES = true;
        extensions->textureBufferEXT = true;
    }
    else
    {
        // Can't support ES3.2 without texture buffer objects
        LimitVersion(maxSupportedESVersion, gl::Version(3, 1));
    }

    extensions->YUVTargetEXT = functions->hasGLESExtension("GL_EXT_YUV_target");

    // GL_MESA_framebuffer_flip_y
    extensions->framebufferFlipYMESA = functions->hasGLESExtension("GL_MESA_framebuffer_flip_y") ||
                                       functions->hasGLExtension("GL_MESA_framebuffer_flip_y");

    // GL_KHR_parallel_shader_compile
    extensions->parallelShaderCompileKHR = !features.disableNativeParallelCompile.enabled &&
                                           (functions->maxShaderCompilerThreadsKHR != nullptr ||
                                            functions->maxShaderCompilerThreadsARB != nullptr);

    // GL_ANGLE_logic_op
    extensions->logicOpANGLE = functions->isAtLeastGL(gl::Version(2, 0));

    // GL_EXT_clear_texture
    extensions->clearTextureEXT = functions->isAtLeastGL(gl::Version(4, 4)) ||
                                  functions->hasGLESExtension("GL_EXT_clear_texture") ||
                                  functions->hasGLExtension("GL_ARB_clear_texture");

    // GL_QCOM_tiled_rendering
    extensions->tiledRenderingQCOM = !features.disableTiledRendering.enabled &&
                                     functions->hasGLESExtension("GL_QCOM_tiled_rendering");

    extensions->blendEquationAdvancedKHR =
        !features.disableBlendEquationAdvanced.enabled &&
        (functions->hasGLExtension("GL_NV_blend_equation_advanced") ||
         functions->hasGLExtension("GL_KHR_blend_equation_advanced") ||
         functions->isAtLeastGLES(gl::Version(3, 2)) ||
         functions->hasGLESExtension("GL_KHR_blend_equation_advanced"));
    extensions->blendEquationAdvancedCoherentKHR =
        !features.disableBlendEquationAdvanced.enabled &&
        (functions->hasGLExtension("GL_NV_blend_equation_advanced_coherent") ||
         functions->hasGLExtension("GL_KHR_blend_equation_advanced_coherent") ||
         functions->isAtLeastGLES(gl::Version(3, 2)) ||
         functions->hasGLESExtension("GL_KHR_blend_equation_advanced_coherent"));

    // PVRTC1 textures must be squares on Apple platforms.
    if (IsApple())
    {
        limitations->squarePvrtc1 = true;
    }

    // Check if the driver clamps constant blend color
    if (IsQualcomm(GetVendorID(functions)))
    {
        // Backup current state
        float oldColor[4];
        functions->getFloatv(GL_BLEND_COLOR, oldColor);

        // Probe clamping
        float color[4];
        functions->blendColor(2.0, 0.0, 0.0, 0.0);
        functions->getFloatv(GL_BLEND_COLOR, color);
        limitations->noUnclampedBlendColor = color[0] == 1.0;

        // Restore previous state
        functions->blendColor(oldColor[0], oldColor[1], oldColor[2], oldColor[3]);
    }
}

bool GetSystemInfoVendorIDAndDeviceID(const FunctionsGL *functions,
                                      angle::SystemInfo *outSystemInfo,
                                      angle::VendorID *outVendor,
                                      angle::DeviceID *outDevice)
{
    // Get vendor from GL itself, so on multi-GPU systems the correct GPU is selected.
    *outVendor = GetVendorID(functions);
    *outDevice = 0;

    // Gather additional information from the system to detect multi-GPU scenarios. Do not collect
    // system info on Android as it may use Vulkan which can crash on older devices. All the system
    // info we need is available in the version and renderer strings on Android.
    bool isGetSystemInfoSuccess = !IsAndroid() && angle::GetSystemInfo(outSystemInfo);

    // Get the device id from system info, corresponding to the vendor of the active GPU.
    if (isGetSystemInfoSuccess && !outSystemInfo->gpus.empty())
    {
        if (*outVendor == VENDOR_ID_UNKNOWN)
        {
            // If vendor ID is unknown, take the best estimate of the active GPU.  Chances are there
            // is only one GPU anyway.
            *outVendor = outSystemInfo->gpus[outSystemInfo->activeGPUIndex].vendorId;
            *outDevice = outSystemInfo->gpus[outSystemInfo->activeGPUIndex].deviceId;
        }
        else
        {
            for (const angle::GPUDeviceInfo &gpu : outSystemInfo->gpus)
            {
                if (*outVendor == gpu.vendorId)
                {
                    // Note that deviceId may not necessarily have been possible to retrieve.
                    *outDevice = gpu.deviceId;
                    break;
                }
            }
        }
    }
    else
    {
        // If system info is not available, attempt to deduce the device from GL itself.
        *outDevice = GetDeviceID(functions);
    }

    return isGetSystemInfoSuccess;
}

bool Has9thGenIntelGPU(const angle::SystemInfo &systemInfo)
{
    for (const angle::GPUDeviceInfo &deviceInfo : systemInfo.gpus)
    {
        if (IsIntel(deviceInfo.vendorId) && Is9thGenIntel(deviceInfo.deviceId))
        {
            return true;
        }
    }

    return false;
}

void InitializeFeatures(const FunctionsGL *functions, angle::FeaturesGL *features)
{
    angle::VendorID vendor;
    angle::DeviceID device;
    angle::SystemInfo systemInfo;

    bool isGetSystemInfoSuccess =
        GetSystemInfoVendorIDAndDeviceID(functions, &systemInfo, &vendor, &device);

    bool isAMD      = IsAMD(vendor);
    bool isApple    = IsAppleGPU(vendor);
    bool isIntel    = IsIntel(vendor);
    bool isNvidia   = IsNvidia(vendor);
    bool isQualcomm = IsQualcomm(vendor);
    bool isVMWare   = IsVMWare(vendor);
    bool hasAMD     = systemInfo.hasAMDGPU();
    bool isMali     = IsARM(vendor);

    std::array<int, 3> mesaVersion = {0, 0, 0};
    bool isMesa                    = IsMesa(functions, &mesaVersion);

    int qualcommVersion = -1;
    if (!isMesa && isQualcomm)
    {
        qualcommVersion = GetQualcommVersion(functions);
    }

    // Don't use 1-bit alpha formats on desktop GL with AMD drivers.
    ANGLE_FEATURE_CONDITION(features, avoid1BitAlphaTextureFormats,
                            functions->standard == STANDARD_GL_DESKTOP && isAMD);

    ANGLE_FEATURE_CONDITION(features, RGBA4IsNotSupportedForColorRendering,
                            functions->standard == STANDARD_GL_DESKTOP && isIntel);

    // Although "Sandy Bridge", "Ivy Bridge", and "Haswell" may support GL_ARB_ES3_compatibility
    // extension, ETC2/EAC formats are emulated there. Newer Intel GPUs support them natively.
    ANGLE_FEATURE_CONDITION(
        features, allowETCFormats,
        isIntel && !IsSandyBridge(device) && !IsIvyBridge(device) && !IsHaswell(device));

    // Mesa always exposes ASTC extension but only Intel Gen9, Gen11, and Gen12 have hardware
    // support for it. Newer Intel GPUs (Gen12.5+) do not support ASTC.
    ANGLE_FEATURE_CONDITION(features, allowAstcFormats,
                            !isMesa || isIntel && (Is9thGenIntel(device) || IsGeminiLake(device) ||
                                                   IsCoffeeLake(device) || Is11thGenIntel(device) ||
                                                   Is12thGenIntel(device)));

    // Ported from gpu_driver_bug_list.json (#183)
    ANGLE_FEATURE_CONDITION(features, emulateAbsIntFunction, IsApple() && isIntel);

    ANGLE_FEATURE_CONDITION(features, addAndTrueToLoopCondition, IsApple() && isIntel);

    // Ported from gpu_driver_bug_list.json (#191)
    ANGLE_FEATURE_CONDITION(
        features, emulateIsnanFloat,
        isIntel && IsApple() && IsSkylake(device) && GetMacOSVersion() < OSVersion(10, 13, 2));

    // https://anglebug.com/42266803
    ANGLE_FEATURE_CONDITION(features, clearsWithGapsNeedFlush,
                            !isMesa && isQualcomm && qualcommVersion < 490);

    ANGLE_FEATURE_CONDITION(features, doesSRGBClearsOnLinearFramebufferAttachments,
                            isIntel || isAMD);

    ANGLE_FEATURE_CONDITION(features, emulateMaxVertexAttribStride,
                            IsLinux() && functions->standard == STANDARD_GL_DESKTOP && isAMD);
    ANGLE_FEATURE_CONDITION(
        features, useUnusedBlocksWithStandardOrSharedLayout,
        (IsApple() && functions->standard == STANDARD_GL_DESKTOP) || (IsLinux() && isAMD));

    // Ported from gpu_driver_bug_list.json (#187)
    ANGLE_FEATURE_CONDITION(features, doWhileGLSLCausesGPUHang,
                            IsApple() && functions->standard == STANDARD_GL_DESKTOP &&
                                GetMacOSVersion() < OSVersion(10, 11, 0));

    // Ported from gpu_driver_bug_list.json (#211)
    ANGLE_FEATURE_CONDITION(features, rewriteFloatUnaryMinusOperator,
                            IsApple() && isIntel && GetMacOSVersion() < OSVersion(10, 12, 0));

    ANGLE_FEATURE_CONDITION(features, vertexIDDoesNotIncludeBaseVertex, IsApple() && isAMD);

    // Triggers a bug on Marshmallow Adreno (4xx?) driver.
    // http://anglebug.com/40096454
    ANGLE_FEATURE_CONDITION(features, dontInitializeUninitializedLocals, !isMesa && isQualcomm);

    ANGLE_FEATURE_CONDITION(features, finishDoesNotCauseQueriesToBeAvailable,
                            functions->standard == STANDARD_GL_DESKTOP && isNvidia);

    // TODO(cwallez): Disable this workaround for MacOSX versions 10.9 or later.
    ANGLE_FEATURE_CONDITION(features, alwaysCallUseProgramAfterLink, true);

    ANGLE_FEATURE_CONDITION(features, unpackOverlappingRowsSeparatelyUnpackBuffer, isNvidia);
    ANGLE_FEATURE_CONDITION(features, packOverlappingRowsSeparatelyPackBuffer, isNvidia);

    ANGLE_FEATURE_CONDITION(features, initializeCurrentVertexAttributes, isNvidia);

    ANGLE_FEATURE_CONDITION(features, unpackLastRowSeparatelyForPaddingInclusion,
                            IsApple() || isNvidia);
    ANGLE_FEATURE_CONDITION(features, packLastRowSeparatelyForPaddingInclusion,
                            IsApple() || isNvidia);

    ANGLE_FEATURE_CONDITION(features, removeInvariantAndCentroidForESSL3,
                            functions->isAtMostGL(gl::Version(4, 1)) ||
                                (functions->standard == STANDARD_GL_DESKTOP && isAMD));

    // TODO(oetuaho): Make this specific to the affected driver versions. Versions that came after
    // 364 are known to be affected, at least up to 375.
    ANGLE_FEATURE_CONDITION(features, emulateAtan2Float, isNvidia);

    ANGLE_FEATURE_CONDITION(features, reapplyUBOBindingsAfterUsingBinaryProgram,
                            isAMD || IsAndroid());

    // TODO(oetuaho): Make this specific to the affected driver versions. Versions at least up to
    // 390 are known to be affected. Versions after that are expected not to be affected.
    ANGLE_FEATURE_CONDITION(features, clampFragDepth, isNvidia);

    // TODO(oetuaho): Make this specific to the affected driver versions. Versions since 397.31 are
    // not affected.
    ANGLE_FEATURE_CONDITION(features, rewriteRepeatedAssignToSwizzled, isNvidia);

    // TODO(jmadill): Narrow workaround range for specific devices.

    ANGLE_FEATURE_CONDITION(features, clampPointSize, IsAndroid() || isNvidia);

    // Ported from gpu_driver_bug_list.json (#246, #258)
    ANGLE_FEATURE_CONDITION(features, dontUseLoopsToInitializeVariables,
                            (!isMesa && isQualcomm) || (isIntel && IsApple()));

    // Intel macOS condition ported from gpu_driver_bug_list.json (#327)
    ANGLE_FEATURE_CONDITION(features, disableBlendFuncExtended,
                            IsApple() && isIntel && GetMacOSVersion() < OSVersion(10, 14, 0));

    ANGLE_FEATURE_CONDITION(features, avoidBindFragDataLocation, !isMesa && isQualcomm);

    ANGLE_FEATURE_CONDITION(features, unsizedSRGBReadPixelsDoesntTransform, !isMesa && isQualcomm);

    ANGLE_FEATURE_CONDITION(features, queryCounterBitsGeneratesErrors, IsNexus5X(vendor, device));

    bool isAndroidLessThan14 = IsAndroid() && GetAndroidSDKVersion() < 34;
    bool isAndroidAtLeast14  = IsAndroid() && GetAndroidSDKVersion() >= 34;
    bool isIntelLinuxLessThanKernelVersion5 =
        isIntel && IsLinux() && GetLinuxOSVersion() < OSVersion(5, 0, 0);
    ANGLE_FEATURE_CONDITION(features, limitWebglMaxTextureSizeTo4096,
                            isAndroidLessThan14 || isIntelLinuxLessThanKernelVersion5);
    ANGLE_FEATURE_CONDITION(features, limitWebglMaxTextureSizeTo8192, isAndroidAtLeast14);
    // On Apple switchable graphics, GL_MAX_SAMPLES may differ between the GPUs.
    // 4 is a lowest common denominator that is always supported.
    ANGLE_FEATURE_CONDITION(features, limitMaxMSAASamplesTo4,
                            IsAndroid() || (IsApple() && (isIntel || isAMD || isNvidia)));
    ANGLE_FEATURE_CONDITION(features, limitMax3dArrayTextureSizeTo1024,
                            isIntelLinuxLessThanKernelVersion5);

    ANGLE_FEATURE_CONDITION(features, allowClearForRobustResourceInit, IsApple());

    // The WebGL conformance/uniforms/out-of-bounds-uniform-array-access test has been seen to fail
    // on AMD and Android devices.
    // This test is also flaky on Linux Nvidia. So we just turn it on everywhere and don't rely on
    // driver since security is important.
    ANGLE_FEATURE_CONDITION(
        features, clampArrayAccess,
        IsAndroid() || isAMD || !functions->hasExtension("GL_KHR_robust_buffer_access_behavior"));

    ANGLE_FEATURE_CONDITION(features, resetTexImage2DBaseLevel,
                            IsApple() && isIntel && GetMacOSVersion() >= OSVersion(10, 12, 4));

    ANGLE_FEATURE_CONDITION(features, clearToZeroOrOneBroken,
                            IsApple() && isIntel && GetMacOSVersion() < OSVersion(10, 12, 6));

    ANGLE_FEATURE_CONDITION(features, adjustSrcDstRegionForBlitFramebuffer,
                            IsLinux() || (IsAndroid() && isNvidia) || (IsWindows() && isNvidia) ||
                                (IsApple() && functions->standard == STANDARD_GL_ES));

    ANGLE_FEATURE_CONDITION(features, clipSrcRegionForBlitFramebuffer,
                            IsApple() || (IsLinux() && isAMD));

    ANGLE_FEATURE_CONDITION(features, RGBDXT1TexturesSampleZeroAlpha, IsApple());

    ANGLE_FEATURE_CONDITION(features, unfoldShortCircuits, IsApple());

    ANGLE_FEATURE_CONDITION(features, emulatePrimitiveRestartFixedIndex,
                            functions->standard == STANDARD_GL_DESKTOP &&
                                functions->isAtLeastGL(gl::Version(3, 1)) &&
                                !functions->isAtLeastGL(gl::Version(4, 3)));
    ANGLE_FEATURE_CONDITION(
        features, setPrimitiveRestartFixedIndexForDrawArrays,
        features->emulatePrimitiveRestartFixedIndex.enabled && IsApple() && isIntel);

    ANGLE_FEATURE_CONDITION(features, removeDynamicIndexingOfSwizzledVector,
                            IsApple() || IsAndroid() || IsWindows());

    // Ported from gpu_driver_bug_list.json (#89)
    ANGLE_FEATURE_CONDITION(features, regenerateStructNames, IsApple());

    // Ported from gpu_driver_bug_list.json (#184)
    ANGLE_FEATURE_CONDITION(features, preAddTexelFetchOffsets, IsApple() && isIntel);

    // Workaround for the widespread OpenGL ES driver implementaion bug
    ANGLE_FEATURE_CONDITION(features, readPixelsUsingImplementationColorReadFormatForNorm16,
                            !isIntel && functions->standard == STANDARD_GL_ES &&
                                functions->isAtLeastGLES(gl::Version(3, 1)) &&
                                functions->hasGLESExtension("GL_EXT_texture_norm16"));

    // anglebug.com/40644715
    ANGLE_FEATURE_CONDITION(features, flushBeforeDeleteTextureIfCopiedTo, IsApple() && isIntel);

    // anglebug.com/40096480
    // Seems to affect both Intel and AMD GPUs. Enable workaround for all GPUs on macOS.
    ANGLE_FEATURE_CONDITION(features, rewriteRowMajorMatrices,
                            // IsApple() && functions->standard == STANDARD_GL_DESKTOP);
                            // TODO(anglebug.com/40096480): diagnose crashes with this workaround.
                            false);

    ANGLE_FEATURE_CONDITION(features, disableDrawBuffersIndexed, IsWindows() && isAMD);

    ANGLE_FEATURE_CONDITION(
        features, disableSemaphoreFd,
        IsLinux() && isAMD && isMesa && mesaVersion < (std::array<int, 3>{19, 3, 5}));

    ANGLE_FEATURE_CONDITION(
        features, disableTimestampQueries,
        (IsLinux() && isVMWare) || (IsAndroid() && isNvidia) ||
            (IsAndroid() && GetAndroidSDKVersion() < 27 && IsAdreno5xxOrOlder(functions)) ||
            (!isMesa && IsMaliT8xxOrOlder(functions)) || (!isMesa && IsMaliG31OrOlder(functions)));

    ANGLE_FEATURE_CONDITION(features, decodeEncodeSRGBForGenerateMipmap, IsApple());

    // anglebug.com/42263273
    // The (redundant) explicit exclusion of Windows AMD is because the workaround fails
    // Texture2DRGTest.TextureRGUNormTest on that platform, and the test is skipped. If
    // you'd like to enable the workaround on Windows AMD, please fix the test first.
    ANGLE_FEATURE_CONDITION(
        features, emulateCopyTexImage2DFromRenderbuffers,
        IsApple() && functions->standard == STANDARD_GL_ES && !(isAMD && IsWindows()));

    // anglebug.com/40096747
    // Replace copyTexImage2D with texImage2D + copyTexSubImage2D to bypass driver bug.
    ANGLE_FEATURE_CONDITION(features, emulateCopyTexImage2D, isApple);

    // Don't attempt to use the discrete GPU on NVIDIA-based MacBook Pros, since the
    // driver is unstable in this situation.
    //
    // Note that this feature is only set here in order to advertise this workaround
    // externally. GPU switching support must be enabled or disabled early, during display
    // initialization, before these features are set up.
    bool isDualGPUMacWithNVIDIA = false;
    if (IsApple() && functions->standard == STANDARD_GL_DESKTOP)
    {
        if (isGetSystemInfoSuccess)
        {
            // The full system information must be queried to see whether it's a dual-GPU
            // NVIDIA MacBook Pro since it's likely that the integrated GPU will be active
            // when these features are initialized.
            isDualGPUMacWithNVIDIA = systemInfo.isMacSwitchable && systemInfo.hasNVIDIAGPU();
        }
    }
    ANGLE_FEATURE_CONDITION(features, disableGPUSwitchingSupport, isDualGPUMacWithNVIDIA);

    // Workaround issue in NVIDIA GL driver on Linux when TSAN is enabled
    // http://crbug.com/1094869
    bool isTSANBuild = false;
#ifdef THREAD_SANITIZER
    isTSANBuild = true;
#endif
    ANGLE_FEATURE_CONDITION(features, disableNativeParallelCompile,
                            isTSANBuild && IsLinux() && isNvidia);

    // anglebug.com/40096712
    ANGLE_FEATURE_CONDITION(features, emulatePackSkipRowsAndPackSkipPixels, IsApple());

    // http://crbug.com/1042393
    // XWayland defaults to a 1hz refresh rate when the "surface is not visible", which sometimes
    // causes issues in Chrome. To get around this, default to a 30Hz refresh rate if we see bogus
    // from the driver.
    ANGLE_FEATURE_CONDITION(features, clampMscRate, IsLinux() && IsWayland());

    ANGLE_FEATURE_CONDITION(features, bindTransformFeedbackBufferBeforeBindBufferRange, IsApple());

    // http://crbug.com/1137851
    // Speculative fix for above issue, users can enable it via flags.
    // http://crbug.com/1187475
    // Disable on Mesa 20 / Intel
    ANGLE_FEATURE_CONDITION(features, disableSyncControlSupport,
                            IsLinux() && isIntel && isMesa && mesaVersion[0] == 20);

    ANGLE_FEATURE_CONDITION(features, keepBufferShadowCopy, !CanMapBufferForRead(functions));

    ANGLE_FEATURE_CONDITION(features, setZeroLevelBeforeGenerateMipmap, IsApple());

    ANGLE_FEATURE_CONDITION(features, promotePackedFormatsTo8BitPerChannel, IsApple() && hasAMD);

    // crbug.com/1171371
    // If output variable gl_FragColor is written by fragment shader, it may cause context lost with
    // Adreno 42x and 3xx.
    ANGLE_FEATURE_CONDITION(features, initFragmentOutputVariables, IsAdreno42xOr3xx(functions));

    // http://crbug.com/1144207
    // The Mac bot with Intel Iris GPU seems unaffected by this bug. Exclude the Haswell family for
    // now.
    ANGLE_FEATURE_CONDITION(features, shiftInstancedArrayDataWithOffset,
                            IsApple() && IsIntel(vendor) && !IsHaswell(device));
    ANGLE_FEATURE_CONDITION(features, syncAllVertexArraysToDefault,
                            !nativegl::SupportsVertexArrayObjects(functions));

    // NVIDIA OpenGL ES emulated profile cannot handle client arrays
    ANGLE_FEATURE_CONDITION(features, syncDefaultVertexArraysToDefault,
                            nativegl::CanUseDefaultVertexArrayObject(functions) && !isNvidia);

    // http://crbug.com/1181193
    // On desktop Linux/AMD when using the amdgpu drivers, the precise kernel and DRM version are
    // leaked via GL_RENDERER. We workaround this too improve user security.
    ANGLE_FEATURE_CONDITION(features, sanitizeAMDGPURendererString, IsLinux() && hasAMD);

    // http://crbug.com/1187513
    // Imagination drivers are buggy with context switching. It needs to unbind fbo before context
    // switching to workadround the driver issues.
    ANGLE_FEATURE_CONDITION(features, unbindFBOBeforeSwitchingContext, IsPowerVR(vendor));

    // http://crbug.com/1181068 and http://crbug.com/783979
    ANGLE_FEATURE_CONDITION(features, flushOnFramebufferChange,
                            IsApple() && Has9thGenIntelGPU(systemInfo));

    // Disable GL_EXT_multisampled_render_to_texture on a bunch of different configurations:

    // http://crbug.com/490379
    // http://crbug.com/767913
    bool isAdreno4xxOnAndroidLessThan51 =
        IsAndroid() && IsAdreno4xx(functions) && GetAndroidSDKVersion() < 22;

    // http://crbug.com/612474
    bool isAdreno4xxOnAndroid70 =
        IsAndroid() && IsAdreno4xx(functions) && GetAndroidSDKVersion() == 24;
    bool isAdreno5xxOnAndroidLessThan70 =
        IsAndroid() && IsAdreno5xx(functions) && GetAndroidSDKVersion() < 24;

    // http://crbug.com/663811
    bool isAdreno5xxOnAndroid71 =
        IsAndroid() && IsAdreno5xx(functions) && GetAndroidSDKVersion() == 25;

    // http://crbug.com/594016
    bool isLinuxVivante = IsLinux() && IsVivante(device);

    // http://anglebug.com/42266736
    bool isWindowsNVIDIA = IsWindows() && IsNvidia(vendor);

    // Temporarily disable on all of Android. http://crbug.com/1417485
    ANGLE_FEATURE_CONDITION(features, disableMultisampledRenderToTexture,
                            isAdreno4xxOnAndroidLessThan51 || isAdreno4xxOnAndroid70 ||
                                isAdreno5xxOnAndroidLessThan70 || isAdreno5xxOnAndroid71 ||
                                isLinuxVivante || isWindowsNVIDIA);

    // http://crbug.com/1181068
    ANGLE_FEATURE_CONDITION(features, uploadTextureDataInChunks, IsApple());

    // https://crbug.com/1060012
    ANGLE_FEATURE_CONDITION(features, emulateImmutableCompressedTexture3D, isQualcomm);

    // https://crbug.com/1300575
    ANGLE_FEATURE_CONDITION(features, emulateRGB10, functions->standard == STANDARD_GL_DESKTOP);

    // https://anglebug.com/42264072
    ANGLE_FEATURE_CONDITION(features, alwaysUnbindFramebufferTexture2D,
                            isNvidia && (IsWindows() || IsLinux()));

    // https://anglebug.com/42265877
    ANGLE_FEATURE_CONDITION(features, disableTextureClampToBorder, false);

    // https://anglebug.com/42265995
    ANGLE_FEATURE_CONDITION(features, passHighpToPackUnormSnormBuiltins, isQualcomm);

    // https://anglebug.com/42266348
    ANGLE_FEATURE_CONDITION(features, emulateClipDistanceState, isQualcomm);

    // https://anglebug.com/42266817
    ANGLE_FEATURE_CONDITION(features, emulateClipOrigin,
                            !isMesa && isQualcomm && qualcommVersion < 490 &&
                                functions->hasGLESExtension("GL_EXT_clip_control"));

    // https://anglebug.com/42266740
    ANGLE_FEATURE_CONDITION(features, explicitFragmentLocations, isQualcomm);

    // Desktop GLSL-only fragment synchronization extensions. These are injected internally by the
    // compiler to make pixel local storage coherent.
    // https://anglebug.com/40096838
    ANGLE_FEATURE_CONDITION(features, supportsFragmentShaderInterlockNV,
                            functions->isAtLeastGL(gl::Version(4, 3)) &&
                                functions->hasGLExtension("GL_NV_fragment_shader_interlock"));
    ANGLE_FEATURE_CONDITION(features, supportsFragmentShaderOrderingINTEL,
                            functions->isAtLeastGL(gl::Version(4, 4)) &&
                                functions->hasGLExtension("GL_INTEL_fragment_shader_ordering"));
    ANGLE_FEATURE_CONDITION(features, supportsFragmentShaderInterlockARB,
                            functions->isAtLeastGL(gl::Version(4, 5)) &&
                                functions->hasGLExtension("GL_ARB_fragment_shader_interlock"));

    // EXT_shader_framebuffer_fetch
    ANGLE_FEATURE_CONDITION(features, supportsShaderFramebufferFetchEXT,
                            functions->hasGLESExtension("GL_EXT_shader_framebuffer_fetch"));

    // EXT_shader_framebuffer_fetch_non_coherent
    ANGLE_FEATURE_CONDITION(
        features, supportsShaderFramebufferFetchNonCoherentEXT,
        functions->hasGLESExtension("GL_EXT_shader_framebuffer_fetch_non_coherent"));

    // https://crbug.com/1356053
    ANGLE_FEATURE_CONDITION(features, bindCompleteFramebufferForTimerQueries, isMali);

    // https://crbug.com/40264674
    ANGLE_FEATURE_CONDITION(features, disableClipControl, IsMaliG72OrG76OrG51(functions));

    // https://anglebug.com/42266811
    ANGLE_FEATURE_CONDITION(features, resyncDepthRangeOnClipControl, !isMesa && isQualcomm);

    // https://anglebug.com/42266745
    ANGLE_FEATURE_CONDITION(features, disableRenderSnorm,
                            isMesa && (mesaVersion < (std::array<int, 3>{21, 3, 0}) ||
                                       (mesaVersion < (std::array<int, 3>{23, 3, 0}) &&
                                        functions->standard == STANDARD_GL_ES)));

    // https://anglebug.com/42266748
    ANGLE_FEATURE_CONDITION(features, disableTextureMirrorClampToEdge,
                            functions->standard == STANDARD_GL_ES && isMesa &&
                                mesaVersion < (std::array<int, 3>{23, 1, 7}));

    // http://anglebug.com/42266610
    ANGLE_FEATURE_CONDITION(features, disableBaseInstanceVertex, IsMaliValhall(functions));

    // Mali: http://crbug.com/40063287
    // Nvidia: http://crbug.com/328015191
    ANGLE_FEATURE_CONDITION(features, scalarizeVecAndMatConstructorArgs, isMali || isNvidia);

    // http://crbug.com/40066076
    ANGLE_FEATURE_CONDITION(features, ensureNonEmptyBufferIsBoundForDraw, IsApple() || IsAndroid());

    // https://anglebug.com/42266857
    ANGLE_FEATURE_CONDITION(features, preTransformTextureCubeGradDerivatives, isApple);

    // https://crbug.com/40279678
    ANGLE_FEATURE_CONDITION(features, useIntermediateTextureForGenerateMipmap,
                            IsPixel7OrPixel8(functions));

    // SRGB blending does not appear to work correctly on the Nexus 5 + other QC devices. Writing to
    // an SRGB framebuffer with GL_FRAMEBUFFER_SRGB enabled and then reading back returns the same
    // value. Disabling GL_FRAMEBUFFER_SRGB will then convert in the wrong direction.
    ANGLE_FEATURE_CONDITION(features, srgbBlendingBroken, IsQualcomm(vendor));

    // BGRA formats do not appear to be accepted by the qualcomm driver despite the extension being
    // exposed.
    ANGLE_FEATURE_CONDITION(features, bgraTexImageFormatsBroken, IsQualcomm(vendor));

    // https://github.com/flutter/flutter/issues/47164
    // https://github.com/flutter/flutter/issues/47804
    // Some devices expose the QCOM tiled memory extension string but don't actually provide the
    // start and end tiling functions.
    bool missingTilingEntryPoints = functions->hasGLESExtension("GL_QCOM_tiled_rendering") &&
                                    (!functions->startTilingQCOM || !functions->endTilingQCOM);

    // http://skbug.com/9491: Nexus5 produces rendering artifacts when we use QCOM_tiled_rendering.
    ANGLE_FEATURE_CONDITION(features, disableTiledRendering,
                            missingTilingEntryPoints || IsAdreno3xx(functions));

    // Intel desktop GL drivers fail many Skia blend tests.
    // Block on older Qualcomm and ARM, following Skia's blocklists.
    ANGLE_FEATURE_CONDITION(
        features, disableBlendEquationAdvanced,
        (isIntel && IsWindows()) || IsAdreno4xx(functions) || IsAdreno5xx(functions) || isMali);
}

void InitializeFrontendFeatures(const FunctionsGL *functions, angle::FrontendFeatures *features)
{
    VendorID vendor = GetVendorID(functions);
    bool isQualcomm = IsQualcomm(vendor);

    std::array<int, 3> mesaVersion = {0, 0, 0};
    bool isMesa                    = IsMesa(functions, &mesaVersion);

    ANGLE_FEATURE_CONDITION(features, disableProgramCachingForTransformFeedback,
                            !isMesa && isQualcomm);
    // https://crbug.com/480992
    // Disable shader program cache to workaround PowerVR Rogue issues.
    ANGLE_FEATURE_CONDITION(features, disableProgramBinary, IsPowerVrRogue(functions));

    // The compile and link jobs need a context, and previous experiments showed setting up temp
    // contexts in threads for the sake of program link triggers too many driver bugs.  See
    // https://chromium-review.googlesource.com/c/angle/angle/+/4774785 for context.
    //
    // As a result, the compile and link jobs are done in the same thread as the call.  If the
    // native driver supports parallel compile/link, it's still done internally by the driver, and
    // ANGLE supports delaying post-compile and post-link operations until that is done.
    ANGLE_FEATURE_CONDITION(features, compileJobIsThreadSafe, false);
    ANGLE_FEATURE_CONDITION(features, linkJobIsThreadSafe, false);

    ANGLE_FEATURE_CONDITION(features, cacheCompiledShader, true);
}

void ReInitializeFeaturesAtGPUSwitch(const FunctionsGL *functions, angle::FeaturesGL *features)
{
    angle::VendorID vendor;
    angle::DeviceID device;
    angle::SystemInfo systemInfo;

    GetSystemInfoVendorIDAndDeviceID(functions, &systemInfo, &vendor, &device);

    // http://crbug.com/1144207
    // The Mac bot with Intel Iris GPU seems unaffected by this bug. Exclude the Haswell family for
    // now.
    // We need to reinitialize this feature when switching between buggy and non-buggy GPUs.
    ANGLE_FEATURE_CONDITION(features, shiftInstancedArrayDataWithOffset,
                            IsApple() && IsIntel(vendor) && !IsHaswell(device));
}

}  // namespace nativegl_gl

namespace nativegl
{

bool SupportsVertexArrayObjects(const FunctionsGL *functions)
{
    return functions->isAtLeastGLES(gl::Version(3, 0)) ||
           functions->hasGLESExtension("GL_OES_vertex_array_object") ||
           functions->isAtLeastGL(gl::Version(3, 0)) ||
           functions->hasGLExtension("GL_ARB_vertex_array_object");
}

bool CanUseDefaultVertexArrayObject(const FunctionsGL *functions)
{
    return (functions->profile & GL_CONTEXT_CORE_PROFILE_BIT) == 0;
}

bool CanUseClientSideArrays(const FunctionsGL *functions, GLuint vao)
{
    // Can use client arrays on GLES or GL compatability profile only on the default VAO
    return CanUseDefaultVertexArrayObject(functions) && vao == 0;
}

bool SupportsCompute(const FunctionsGL *functions)
{
    // OpenGL 4.2 is required for GL_ARB_compute_shader, some platform drivers have the extension,
    // but their maximum supported GL versions are less than 4.2. Explicitly limit the minimum
    // GL version to 4.2.
    return (functions->isAtLeastGL(gl::Version(4, 3)) ||
            functions->isAtLeastGLES(gl::Version(3, 1)) ||
            (functions->isAtLeastGL(gl::Version(4, 2)) &&
             functions->hasGLExtension("GL_ARB_compute_shader") &&
             functions->hasGLExtension("GL_ARB_shader_storage_buffer_object")));
}

bool SupportsFenceSync(const FunctionsGL *functions)
{
    return functions->isAtLeastGL(gl::Version(3, 2)) || functions->hasGLExtension("GL_ARB_sync") ||
           functions->isAtLeastGLES(gl::Version(3, 0));
}

bool SupportsOcclusionQueries(const FunctionsGL *functions)
{
    return functions->isAtLeastGL(gl::Version(1, 5)) ||
           functions->hasGLExtension("GL_ARB_occlusion_query2") ||
           functions->isAtLeastGLES(gl::Version(3, 0)) ||
           functions->hasGLESExtension("GL_EXT_occlusion_query_boolean");
}

bool SupportsNativeRendering(const FunctionsGL *functions,
                             gl::TextureType type,
                             GLenum internalFormat)
{
    // Some desktop drivers allow rendering to formats that are not required by the spec, this is
    // exposed through the GL_FRAMEBUFFER_RENDERABLE query.
    bool hasInternalFormatQuery = functions->isAtLeastGL(gl::Version(4, 3)) ||
                                  functions->hasGLExtension("GL_ARB_internalformat_query2");

    // Some Intel drivers have a bug that returns GL_FULL_SUPPORT when asked if they support
    // rendering to compressed texture formats yet return framebuffer incomplete when attempting to
    // render to the format.  Skip any native queries for compressed formats.
    const gl::InternalFormat &internalFormatInfo = gl::GetSizedInternalFormatInfo(internalFormat);

    if (hasInternalFormatQuery && !internalFormatInfo.compressed)
    {
        GLint framebufferRenderable = GL_NONE;
        functions->getInternalformativ(ToGLenum(type), internalFormat, GL_FRAMEBUFFER_RENDERABLE, 1,
                                       &framebufferRenderable);
        return framebufferRenderable != GL_NONE;
    }
    else
    {
        const nativegl::InternalFormat &nativeInfo =
            nativegl::GetInternalFormatInfo(internalFormat, functions->standard);
        return nativegl_gl::MeetsRequirements(functions, nativeInfo.textureAttachment);
    }
}

bool SupportsTexImage(gl::TextureType type)
{
    switch (type)
    {
            // Multi-sample texture types only support TexStorage data upload
        case gl::TextureType::_2DMultisample:
        case gl::TextureType::_2DMultisampleArray:
            return false;

        default:
            return true;
    }
}

bool UseTexImage2D(gl::TextureType textureType)
{
    return textureType == gl::TextureType::_2D || textureType == gl::TextureType::CubeMap ||
           textureType == gl::TextureType::Rectangle ||
           textureType == gl::TextureType::_2DMultisample ||
           textureType == gl::TextureType::External || textureType == gl::TextureType::VideoImage;
}

bool UseTexImage3D(gl::TextureType textureType)
{
    return textureType == gl::TextureType::_2DArray || textureType == gl::TextureType::_3D ||
           textureType == gl::TextureType::_2DMultisampleArray ||
           textureType == gl::TextureType::CubeMapArray;
}

GLenum GetTextureBindingQuery(gl::TextureType textureType)
{
    switch (textureType)
    {
        case gl::TextureType::_2D:
            return GL_TEXTURE_BINDING_2D;
        case gl::TextureType::_2DArray:
            return GL_TEXTURE_BINDING_2D_ARRAY;
        case gl::TextureType::_2DMultisample:
            return GL_TEXTURE_BINDING_2D_MULTISAMPLE;
        case gl::TextureType::_2DMultisampleArray:
            return GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY;
        case gl::TextureType::_3D:
            return GL_TEXTURE_BINDING_3D;
        case gl::TextureType::External:
            return GL_TEXTURE_BINDING_EXTERNAL_OES;
        case gl::TextureType::Rectangle:
            return GL_TEXTURE_BINDING_RECTANGLE;
        case gl::TextureType::CubeMap:
            return GL_TEXTURE_BINDING_CUBE_MAP;
        case gl::TextureType::CubeMapArray:
            return GL_TEXTURE_BINDING_CUBE_MAP_ARRAY_OES;
        case gl::TextureType::Buffer:
            return GL_TEXTURE_BINDING_BUFFER;
        default:
            UNREACHABLE();
            return 0;
    }
}

GLenum GetTextureBindingTarget(gl::TextureType textureType)
{
    return ToGLenum(GetNativeTextureType(textureType));
}

GLenum GetTextureBindingTarget(gl::TextureTarget textureTarget)
{
    return ToGLenum(GetNativeTextureTarget(textureTarget));
}

GLenum GetBufferBindingQuery(gl::BufferBinding bufferBinding)
{
    switch (bufferBinding)
    {
        case gl::BufferBinding::Array:
            return GL_ARRAY_BUFFER_BINDING;
        case gl::BufferBinding::AtomicCounter:
            return GL_ATOMIC_COUNTER_BUFFER_BINDING;
        case gl::BufferBinding::CopyRead:
            return GL_COPY_READ_BUFFER_BINDING;
        case gl::BufferBinding::CopyWrite:
            return GL_COPY_WRITE_BUFFER_BINDING;
        case gl::BufferBinding::DispatchIndirect:
            return GL_DISPATCH_INDIRECT_BUFFER_BINDING;
        case gl::BufferBinding::DrawIndirect:
            return GL_DRAW_INDIRECT_BUFFER_BINDING;
        case gl::BufferBinding::ElementArray:
            return GL_ELEMENT_ARRAY_BUFFER_BINDING;
        case gl::BufferBinding::PixelPack:
            return GL_PIXEL_PACK_BUFFER_BINDING;
        case gl::BufferBinding::PixelUnpack:
            return GL_PIXEL_UNPACK_BUFFER_BINDING;
        case gl::BufferBinding::ShaderStorage:
            return GL_SHADER_STORAGE_BUFFER_BINDING;
        case gl::BufferBinding::TransformFeedback:
            return GL_TRANSFORM_FEEDBACK_BUFFER_BINDING;
        case gl::BufferBinding::Uniform:
            return GL_UNIFORM_BUFFER_BINDING;
        case gl::BufferBinding::Texture:
            return GL_TEXTURE_BUFFER_BINDING;
        default:
            UNREACHABLE();
            return 0;
    }
}

std::string GetBufferBindingString(gl::BufferBinding bufferBinding)
{
    std::ostringstream os;
    os << bufferBinding << "_BINDING";
    return os.str();
}

gl::TextureType GetNativeTextureType(gl::TextureType type)
{
    // VideoImage texture type is a WebGL type. It doesn't have
    // directly mapping type in native OpenGL/OpenGLES.
    // Actually, it will be translated to different texture type
    // (TEXTURE2D, TEXTURE_EXTERNAL_OES and TEXTURE_RECTANGLE)
    // based on OS and other conditions.
    // This will introduce problem that binding VideoImage may
    // unbind native image implicitly. Please make sure state
    // manager is aware of this implicit unbind behaviour.
    if (type != gl::TextureType::VideoImage)
    {
        return type;
    }

    // TODO(http://anglebug.com/42262534): need to figure out rectangle texture and
    // external image when these backend are implemented.
    return gl::TextureType::_2D;
}

gl::TextureTarget GetNativeTextureTarget(gl::TextureTarget target)
{
    // VideoImage texture type is a WebGL type. It doesn't have
    // directly mapping type in native OpenGL/OpenGLES.
    // Actually, it will be translated to different texture target
    // (TEXTURE2D, TEXTURE_EXTERNAL_OES and TEXTURE_RECTANGLE)
    // based on OS and other conditions.
    // This will introduce problem that binding VideoImage may
    // unbind native image implicitly. Please make sure state
    // manager is aware of this implicit unbind behaviour.
    if (target != gl::TextureTarget::VideoImage)
    {
        return target;
    }

    // TODO(http://anglebug.com/42262534): need to figure out rectangle texture and
    // external image when these backend are implemented.
    return gl::TextureTarget::_2D;
}

}  // namespace nativegl

const FunctionsGL *GetFunctionsGL(const gl::Context *context)
{
    return GetImplAs<ContextGL>(context)->getFunctions();
}

StateManagerGL *GetStateManagerGL(const gl::Context *context)
{
    return GetImplAs<ContextGL>(context)->getStateManager();
}

BlitGL *GetBlitGL(const gl::Context *context)
{
    return GetImplAs<ContextGL>(context)->getBlitter();
}

ClearMultiviewGL *GetMultiviewClearer(const gl::Context *context)
{
    return GetImplAs<ContextGL>(context)->getMultiviewClearer();
}

const angle::FeaturesGL &GetFeaturesGL(const gl::Context *context)
{
    return GetImplAs<ContextGL>(context)->getFeaturesGL();
}

void ClearErrors(const FunctionsGL *functions,
                 const char *file,
                 const char *function,
                 unsigned int line)
{
    GLenum error = functions->getError();
    while (error != GL_NO_ERROR)
    {
        INFO() << "Preexisting GL error " << gl::FmtHex(error) << " as of " << file << ", "
               << function << ":" << line << ". ";

        // Skip GL_CONTEXT_LOST errors, they will be generated continuously and result in an
        // infinite loop.
        if (error == GL_CONTEXT_LOST)
        {
            return;
        }

        error = functions->getError();
    }
}

void ClearErrors(const gl::Context *context,
                 const char *file,
                 const char *function,
                 unsigned int line)
{
    const FunctionsGL *functions = GetFunctionsGL(context);
    ClearErrors(functions, file, function, line);
}

angle::Result CheckError(const gl::Context *context,
                         const char *call,
                         const char *file,
                         const char *function,
                         unsigned int line)
{
    return HandleError(context, GetFunctionsGL(context)->getError(), call, file, function, line);
}

angle::Result HandleError(const gl::Context *context,
                          GLenum error,
                          const char *call,
                          const char *file,
                          const char *function,
                          unsigned int line)
{
    const FunctionsGL *functions = GetFunctionsGL(context);
    if (ANGLE_UNLIKELY(error != GL_NO_ERROR))
    {
        ContextGL *contextGL = GetImplAs<ContextGL>(context);
        contextGL->handleError(error, "Unexpected driver error.", file, function, line);
        ERR() << "GL call " << call << " generated error " << gl::FmtHex(error) << " in " << file
              << ", " << function << ":" << line << ". ";

        // Check that only one GL error was generated, ClearErrors should have been called first.
        // Skip GL_CONTEXT_LOST errors, they will be generated continuously and result in an
        // infinite loop.
        GLenum nextError = functions->getError();
        while (nextError != GL_NO_ERROR && nextError != GL_CONTEXT_LOST)
        {
            ERR() << "Additional GL error " << gl::FmtHex(nextError) << " generated.";
            nextError = functions->getError();
        }

        return angle::Result::Stop;
    }

    return angle::Result::Continue;
}

bool CanMapBufferForRead(const FunctionsGL *functions)
{
    return (functions->mapBufferRange != nullptr) ||
           (functions->mapBuffer != nullptr && functions->standard == STANDARD_GL_DESKTOP);
}

uint8_t *MapBufferRangeWithFallback(const FunctionsGL *functions,
                                    GLenum target,
                                    size_t offset,
                                    size_t length,
                                    GLbitfield access)
{
    if (functions->mapBufferRange != nullptr)
    {
        return static_cast<uint8_t *>(functions->mapBufferRange(target, offset, length, access));
    }
    else if (functions->mapBuffer != nullptr &&
             (functions->standard == STANDARD_GL_DESKTOP || access == GL_MAP_WRITE_BIT))
    {
        // Only the read and write bits are supported
        ASSERT((access & (GL_MAP_READ_BIT | GL_MAP_WRITE_BIT)) != 0);

        GLenum accessEnum = 0;
        if (access == (GL_MAP_READ_BIT | GL_MAP_WRITE_BIT))
        {
            accessEnum = GL_READ_WRITE;
        }
        else if (access == GL_MAP_READ_BIT)
        {
            accessEnum = GL_READ_ONLY;
        }
        else if (access == GL_MAP_WRITE_BIT)
        {
            accessEnum = GL_WRITE_ONLY;
        }
        else
        {
            UNREACHABLE();
            return nullptr;
        }

        return static_cast<uint8_t *>(functions->mapBuffer(target, accessEnum)) + offset;
    }
    else
    {
        // No options available
        UNREACHABLE();
        return nullptr;
    }
}

angle::Result ShouldApplyLastRowPaddingWorkaround(ContextGL *contextGL,
                                                  const gl::Extents &size,
                                                  const gl::PixelStoreStateBase &state,
                                                  const gl::Buffer *pixelBuffer,
                                                  GLenum format,
                                                  GLenum type,
                                                  bool is3D,
                                                  const void *pixels,
                                                  bool *shouldApplyOut)
{
    if (pixelBuffer == nullptr)
    {
        *shouldApplyOut = false;
        return angle::Result::Continue;
    }

    // We are using an pack or unpack buffer, compute what the driver thinks is going to be the
    // last byte read or written. If it is past the end of the buffer, we will need to use the
    // workaround otherwise the driver will generate INVALID_OPERATION and not do the operation.

    const gl::InternalFormat &glFormat = gl::GetInternalFormatInfo(format, type);
    GLuint endByte                     = 0;
    ANGLE_CHECK_GL_MATH(contextGL,
                        glFormat.computePackUnpackEndByte(type, size, state, is3D, &endByte));
    GLuint rowPitch = 0;
    ANGLE_CHECK_GL_MATH(contextGL, glFormat.computeRowPitch(type, size.width, state.alignment,
                                                            state.rowLength, &rowPitch));

    CheckedNumeric<size_t> checkedPixelBytes = glFormat.computePixelBytes(type);
    CheckedNumeric<size_t> checkedEndByte =
        angle::CheckedNumeric<size_t>(endByte) + reinterpret_cast<intptr_t>(pixels);

    // At this point checkedEndByte is the actual last byte read.
    // The driver adds an extra row padding (if any), mimic it.
    ANGLE_CHECK_GL_MATH(contextGL, checkedPixelBytes.IsValid());
    if (static_cast<size_t>(checkedPixelBytes.ValueOrDie()) * size.width < rowPitch)
    {
        checkedEndByte += rowPitch - checkedPixelBytes * size.width;
    }

    ANGLE_CHECK_GL_MATH(contextGL, checkedEndByte.IsValid());

    *shouldApplyOut = checkedEndByte.ValueOrDie() > static_cast<size_t>(pixelBuffer->getSize());
    return angle::Result::Continue;
}

std::vector<ContextCreationTry> GenerateContextCreationToTry(EGLint requestedType, bool isMesaGLX)
{
    using Type                         = ContextCreationTry::Type;
    constexpr EGLint kPlatformOpenGL   = EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE;
    constexpr EGLint kPlatformOpenGLES = EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE;

    std::vector<ContextCreationTry> contextsToTry;

    if (requestedType == EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE || requestedType == kPlatformOpenGL)
    {
        contextsToTry.emplace_back(kPlatformOpenGL, Type::DESKTOP_CORE, gl::Version(4, 5));
        contextsToTry.emplace_back(kPlatformOpenGL, Type::DESKTOP_CORE, gl::Version(4, 4));
        contextsToTry.emplace_back(kPlatformOpenGL, Type::DESKTOP_CORE, gl::Version(4, 3));
        contextsToTry.emplace_back(kPlatformOpenGL, Type::DESKTOP_CORE, gl::Version(4, 2));
        contextsToTry.emplace_back(kPlatformOpenGL, Type::DESKTOP_CORE, gl::Version(4, 1));
        contextsToTry.emplace_back(kPlatformOpenGL, Type::DESKTOP_CORE, gl::Version(4, 0));
        contextsToTry.emplace_back(kPlatformOpenGL, Type::DESKTOP_CORE, gl::Version(3, 3));
        contextsToTry.emplace_back(kPlatformOpenGL, Type::DESKTOP_CORE, gl::Version(3, 2));
        contextsToTry.emplace_back(kPlatformOpenGL, Type::DESKTOP_LEGACY, gl::Version(3, 3));

        // On Mesa, do not try to create OpenGL context versions between 3.0 and
        // 3.2 because of compatibility problems. See crbug.com/659030
        if (!isMesaGLX)
        {
            contextsToTry.emplace_back(kPlatformOpenGL, Type::DESKTOP_LEGACY, gl::Version(3, 2));
            contextsToTry.emplace_back(kPlatformOpenGL, Type::DESKTOP_LEGACY, gl::Version(3, 1));
            contextsToTry.emplace_back(kPlatformOpenGL, Type::DESKTOP_LEGACY, gl::Version(3, 0));
        }

        contextsToTry.emplace_back(kPlatformOpenGL, Type::DESKTOP_LEGACY, gl::Version(2, 1));
        contextsToTry.emplace_back(kPlatformOpenGL, Type::DESKTOP_LEGACY, gl::Version(2, 0));
        contextsToTry.emplace_back(kPlatformOpenGL, Type::DESKTOP_LEGACY, gl::Version(1, 5));
        contextsToTry.emplace_back(kPlatformOpenGL, Type::DESKTOP_LEGACY, gl::Version(1, 4));
        contextsToTry.emplace_back(kPlatformOpenGL, Type::DESKTOP_LEGACY, gl::Version(1, 3));
        contextsToTry.emplace_back(kPlatformOpenGL, Type::DESKTOP_LEGACY, gl::Version(1, 2));
        contextsToTry.emplace_back(kPlatformOpenGL, Type::DESKTOP_LEGACY, gl::Version(1, 1));
        contextsToTry.emplace_back(kPlatformOpenGL, Type::DESKTOP_LEGACY, gl::Version(1, 0));
    }

    if (requestedType == EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE ||
        requestedType == kPlatformOpenGLES)
    {
        contextsToTry.emplace_back(kPlatformOpenGLES, Type::ES, gl::Version(3, 2));
        contextsToTry.emplace_back(kPlatformOpenGLES, Type::ES, gl::Version(3, 1));
        contextsToTry.emplace_back(kPlatformOpenGLES, Type::ES, gl::Version(3, 0));
        contextsToTry.emplace_back(kPlatformOpenGLES, Type::ES, gl::Version(2, 0));
    }

    return contextsToTry;
}

std::string GetRendererString(const FunctionsGL *functions)
{
    return GetString(functions, GL_RENDERER);
}

std::string GetVendorString(const FunctionsGL *functions)
{
    return GetString(functions, GL_VENDOR);
}

std::string GetVersionString(const FunctionsGL *functions)
{
    return GetString(functions, GL_VERSION);
}

}  // namespace rx
