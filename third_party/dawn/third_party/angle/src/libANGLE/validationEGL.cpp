//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// validationEGL.cpp: Validation functions for generic EGL entry point parameters

#include "libANGLE/validationEGL_autogen.h"

#include "common/utilities.h"
#include "libANGLE/Config.h"
#include "libANGLE/Context.h"
#include "libANGLE/Device.h"
#include "libANGLE/Display.h"
#include "libANGLE/EGLSync.h"
#include "libANGLE/Image.h"
#include "libANGLE/Stream.h"
#include "libANGLE/Surface.h"
#include "libANGLE/Texture.h"
#include "libANGLE/Thread.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/DisplayImpl.h"

#include <EGL/eglext.h>

namespace egl
{
namespace
{
size_t GetMaximumMipLevel(const gl::Context *context, gl::TextureType type)
{
    const gl::Caps &caps = context->getCaps();

    int maxDimension = 0;
    switch (type)
    {
        case gl::TextureType::_2D:
        case gl::TextureType::_2DArray:
        case gl::TextureType::_2DMultisample:
            maxDimension = caps.max2DTextureSize;
            break;
        case gl::TextureType::Rectangle:
            maxDimension = caps.maxRectangleTextureSize;
            break;
        case gl::TextureType::CubeMap:
            maxDimension = caps.maxCubeMapTextureSize;
            break;
        case gl::TextureType::_3D:
            maxDimension = caps.max3DTextureSize;
            break;

        default:
            UNREACHABLE();
    }

    return gl::log2(maxDimension);
}

bool TextureHasNonZeroMipLevelsSpecified(const gl::Context *context, const gl::Texture *texture)
{
    size_t maxMip = GetMaximumMipLevel(context, texture->getType());
    for (size_t level = 1; level < maxMip; level++)
    {
        if (texture->getType() == gl::TextureType::CubeMap)
        {
            for (gl::TextureTarget face : gl::AllCubeFaceTextureTargets())
            {
                if (texture->getFormat(face, level).valid())
                {
                    return true;
                }
            }
        }
        else
        {
            if (texture->getFormat(gl::NonCubeTextureTypeToTarget(texture->getType()), level)
                    .valid())
            {
                return true;
            }
        }
    }

    return false;
}

bool CubeTextureHasUnspecifiedLevel0Face(const gl::Texture *texture)
{
    ASSERT(texture->getType() == gl::TextureType::CubeMap);
    for (gl::TextureTarget face : gl::AllCubeFaceTextureTargets())
    {
        if (!texture->getFormat(face, 0).valid())
        {
            return true;
        }
    }

    return false;
}

bool ValidateStreamAttribute(const ValidationContext *val,
                             const EGLAttrib attribute,
                             const EGLAttrib value,
                             const DisplayExtensions &extensions)
{
    switch (attribute)
    {
        case EGL_STREAM_STATE_KHR:
        case EGL_PRODUCER_FRAME_KHR:
        case EGL_CONSUMER_FRAME_KHR:
            val->setError(EGL_BAD_ACCESS, "Attempt to initialize readonly parameter");
            return false;
        case EGL_CONSUMER_LATENCY_USEC_KHR:
            // Technically not in spec but a latency < 0 makes no sense so we check it
            if (value < 0)
            {
                val->setError(EGL_BAD_PARAMETER, "Latency must be positive");
                return false;
            }
            break;
        case EGL_CONSUMER_ACQUIRE_TIMEOUT_USEC_KHR:
            if (!extensions.streamConsumerGLTexture)
            {
                val->setError(EGL_BAD_ATTRIBUTE, "Consumer GL extension not enabled");
                return false;
            }
            // Again not in spec but it should be positive anyways
            if (value < 0)
            {
                val->setError(EGL_BAD_PARAMETER, "Timeout must be positive");
                return false;
            }
            break;
        default:
            val->setError(EGL_BAD_ATTRIBUTE, "Invalid stream attribute");
            return false;
    }
    return true;
}

bool ValidateCreateImageMipLevelCommon(const ValidationContext *val,
                                       const gl::Context *context,
                                       const gl::Texture *texture,
                                       EGLAttrib level)
{
    // Note that the spec EGL_create_image spec does not explicitly specify an error
    // when the level is outside the base/max level range, but it does mention that the
    // level "must be a part of the complete texture object <buffer>". It can be argued
    // that out-of-range levels are not a part of the complete texture.
    const GLuint effectiveBaseLevel = texture->getTextureState().getEffectiveBaseLevel();
    if (level > 0 &&
        (!texture->isMipmapComplete() || static_cast<GLuint>(level) < effectiveBaseLevel ||
         static_cast<GLuint>(level) > texture->getTextureState().getMipmapMaxLevel()))
    {
        val->setError(EGL_BAD_PARAMETER, "texture must be complete if level is non-zero.");
        return false;
    }

    if (level == 0 && !texture->isMipmapComplete() &&
        TextureHasNonZeroMipLevelsSpecified(context, texture))
    {
        val->setError(EGL_BAD_PARAMETER,
                      "if level is zero and the texture is incomplete, it must "
                      "have no mip levels specified except zero.");
        return false;
    }

    return true;
}

bool ValidateConfigAttribute(const ValidationContext *val,
                             const Display *display,
                             EGLAttrib attribute)
{
    switch (attribute)
    {
        case EGL_BUFFER_SIZE:
        case EGL_ALPHA_SIZE:
        case EGL_BLUE_SIZE:
        case EGL_GREEN_SIZE:
        case EGL_RED_SIZE:
        case EGL_DEPTH_SIZE:
        case EGL_STENCIL_SIZE:
        case EGL_CONFIG_CAVEAT:
        case EGL_CONFIG_ID:
        case EGL_LEVEL:
        case EGL_NATIVE_RENDERABLE:
        case EGL_NATIVE_VISUAL_ID:
        case EGL_NATIVE_VISUAL_TYPE:
        case EGL_SAMPLES:
        case EGL_SAMPLE_BUFFERS:
        case EGL_SURFACE_TYPE:
        case EGL_TRANSPARENT_TYPE:
        case EGL_TRANSPARENT_BLUE_VALUE:
        case EGL_TRANSPARENT_GREEN_VALUE:
        case EGL_TRANSPARENT_RED_VALUE:
        case EGL_BIND_TO_TEXTURE_RGB:
        case EGL_BIND_TO_TEXTURE_RGBA:
        case EGL_MIN_SWAP_INTERVAL:
        case EGL_MAX_SWAP_INTERVAL:
        case EGL_LUMINANCE_SIZE:
        case EGL_ALPHA_MASK_SIZE:
        case EGL_COLOR_BUFFER_TYPE:
        case EGL_RENDERABLE_TYPE:
        case EGL_MATCH_NATIVE_PIXMAP:
        case EGL_CONFORMANT:
        case EGL_MAX_PBUFFER_WIDTH:
        case EGL_MAX_PBUFFER_HEIGHT:
        case EGL_MAX_PBUFFER_PIXELS:
            break;

        case EGL_OPTIMAL_SURFACE_ORIENTATION_ANGLE:
            if (!display->getExtensions().surfaceOrientation)
            {
                val->setError(EGL_BAD_ATTRIBUTE, "EGL_ANGLE_surface_orientation is not enabled.");
                return false;
            }
            break;

        case EGL_COLOR_COMPONENT_TYPE_EXT:
            if (!display->getExtensions().pixelFormatFloat)
            {
                val->setError(EGL_BAD_ATTRIBUTE, "EGL_EXT_pixel_format_float is not enabled.");
                return false;
            }
            break;

        case EGL_RECORDABLE_ANDROID:
            if (!display->getExtensions().recordable)
            {
                val->setError(EGL_BAD_ATTRIBUTE, "EGL_ANDROID_recordable is not enabled.");
                return false;
            }
            break;

        case EGL_FRAMEBUFFER_TARGET_ANDROID:
            if (!display->getExtensions().framebufferTargetANDROID)
            {
                val->setError(EGL_BAD_ATTRIBUTE, "EGL_ANDROID_framebuffer_target is not enabled.");
                return false;
            }
            break;

        case EGL_BIND_TO_TEXTURE_TARGET_ANGLE:
            if (!display->getExtensions().iosurfaceClientBuffer)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "EGL_ANGLE_iosurface_client_buffer is not enabled.");
                return false;
            }
            break;

        case EGL_Y_INVERTED_NOK:
            if (!display->getExtensions().textureFromPixmapNOK)
            {
                val->setError(EGL_BAD_ATTRIBUTE, "EGL_NOK_texture_from_pixmap is not enabled.");
                return false;
            }
            break;

        case EGL_MATCH_FORMAT_KHR:
            if (!display->getExtensions().lockSurface3KHR)
            {
                val->setError(EGL_BAD_ATTRIBUTE, "EGL_KHR_lock_surface3 is not enabled.");
                return false;
            }
            break;

        default:
            val->setError(EGL_BAD_ATTRIBUTE, "Unknown attribute: 0x%04" PRIxPTR "X", attribute);
            return false;
    }

    return true;
}

bool ValidateConfigAttributeValue(const ValidationContext *val,
                                  const Display *display,
                                  EGLAttrib attribute,
                                  EGLAttrib value)
{
    switch (attribute)
    {

        case EGL_BIND_TO_TEXTURE_RGB:
        case EGL_BIND_TO_TEXTURE_RGBA:
            switch (value)
            {
                case EGL_DONT_CARE:
                case EGL_TRUE:
                case EGL_FALSE:
                    break;
                default:
                    val->setError(EGL_BAD_ATTRIBUTE, "EGL_bind_to_texture invalid attribute: 0x%X",
                                  static_cast<uint32_t>(value));
                    return false;
            }
            break;

        case EGL_COLOR_BUFFER_TYPE:
            switch (value)
            {
                case EGL_RGB_BUFFER:
                case EGL_LUMINANCE_BUFFER:
                // EGL_DONT_CARE doesn't match the spec, but does match dEQP usage
                case EGL_DONT_CARE:
                    break;
                default:
                    val->setError(EGL_BAD_ATTRIBUTE,
                                  "EGL_color_buffer_type invalid attribute: 0x%X",
                                  static_cast<uint32_t>(value));
                    return false;
            }
            break;

        case EGL_NATIVE_RENDERABLE:
            switch (value)
            {
                case EGL_DONT_CARE:
                case EGL_TRUE:
                case EGL_FALSE:
                    break;
                default:
                    val->setError(EGL_BAD_ATTRIBUTE,
                                  "EGL_native_renderable invalid attribute: 0x%X",
                                  static_cast<uint32_t>(value));
                    return false;
            }
            break;

        case EGL_TRANSPARENT_TYPE:
            switch (value)
            {
                case EGL_NONE:
                case EGL_TRANSPARENT_RGB:
                // EGL_DONT_CARE doesn't match the spec, but does match dEQP usage
                case EGL_DONT_CARE:
                    break;
                default:
                    val->setError(EGL_BAD_ATTRIBUTE, "EGL_transparent_type invalid attribute: 0x%X",
                                  static_cast<uint32_t>(value));
                    return false;
            }
            break;

        case EGL_RECORDABLE_ANDROID:
            switch (value)
            {
                case EGL_TRUE:
                case EGL_FALSE:
                case EGL_DONT_CARE:
                    break;
                default:
                    val->setError(EGL_BAD_ATTRIBUTE,
                                  "EGL_RECORDABLE_ANDROID invalid attribute: 0x%X",
                                  static_cast<uint32_t>(value));
                    return false;
            }
            break;

        case EGL_COLOR_COMPONENT_TYPE_EXT:
            switch (value)
            {
                case EGL_COLOR_COMPONENT_TYPE_FIXED_EXT:
                case EGL_COLOR_COMPONENT_TYPE_FLOAT_EXT:
                case EGL_DONT_CARE:
                    break;
                default:
                    val->setError(EGL_BAD_ATTRIBUTE,
                                  "EGL_COLOR_COMPONENT_TYPE_EXT invalid attribute: 0x%X",
                                  static_cast<uint32_t>(value));
                    return false;
            }
            break;

        case EGL_MATCH_FORMAT_KHR:
            switch (value)
            {
                case EGL_FORMAT_RGB_565_KHR:
                case EGL_FORMAT_RGBA_8888_KHR:
                case EGL_FORMAT_RGB_565_EXACT_KHR:
                case EGL_FORMAT_RGBA_8888_EXACT_KHR:
                    break;
                default:
                    val->setError(EGL_BAD_ATTRIBUTE,
                                  "EGL_KHR_lock_surface3 invalid attribute: 0x%X",
                                  static_cast<uint32_t>(value));
                    return false;
            }
            break;

        case EGL_CONFIG_CAVEAT:
            switch (value)
            {
                case EGL_DONT_CARE:
                case EGL_NONE:
                case EGL_SLOW_CONFIG:
                case EGL_NON_CONFORMANT_CONFIG:
                    break;
                default:
                    val->setError(EGL_BAD_ATTRIBUTE, "EGL_CONFIG_CAVEAT invalid attribute: 0x%X",
                                  static_cast<uint32_t>(value));
                    return false;
            }
            break;

        case EGL_SURFACE_TYPE:
        {
            if (value == EGL_DONT_CARE)
            {
                break;
            }

            EGLint kValidSurfaceTypeFlags =
                (EGL_WINDOW_BIT | EGL_PIXMAP_BIT | EGL_PBUFFER_BIT |
                 EGL_MULTISAMPLE_RESOLVE_BOX_BIT | EGL_SWAP_BEHAVIOR_PRESERVED_BIT |
                 EGL_VG_COLORSPACE_LINEAR_BIT | EGL_VG_ALPHA_FORMAT_PRE_BIT);

            if (display->getExtensions().mutableRenderBufferKHR)
            {
                kValidSurfaceTypeFlags |= EGL_MUTABLE_RENDER_BUFFER_BIT_KHR;
            }
            if (display->getExtensions().lockSurface3KHR)
            {
                kValidSurfaceTypeFlags |= EGL_LOCK_SURFACE_BIT_KHR;
            }

            if ((value & ~kValidSurfaceTypeFlags) != 0)
            {
                val->setError(EGL_BAD_ATTRIBUTE, "EGL_SURFACE_TYPE invalid attribute: 0x%X",
                              static_cast<uint32_t>(value));
                return false;
            }
            break;
        }

        case EGL_CONFORMANT:
        case EGL_RENDERABLE_TYPE:
        {
            if (value == EGL_DONT_CARE)
            {
                break;
            }
            constexpr EGLint kValidAPITypeFlags =
                (EGL_OPENGL_BIT | EGL_OPENGL_ES_BIT | EGL_OPENGL_ES2_BIT | EGL_OPENVG_BIT |
                 EGL_OPENGL_ES3_BIT_KHR);
            if ((value & ~kValidAPITypeFlags) != 0)
            {
                val->setError(
                    EGL_BAD_ATTRIBUTE, "%s invalid attribute: 0x%X",
                    attribute == EGL_CONFORMANT ? "EGL_CONFORMANT" : "EGL_RENDERABLE_TYPE",
                    static_cast<uint32_t>(value));
                return false;
            }
            break;
        }

        default:
            break;
    }

    return true;
}

bool ValidateConfigAttributes(const ValidationContext *val,
                              const Display *display,
                              const AttributeMap &attributes)
{
    ANGLE_VALIDATION_TRY(attributes.validate(val, display, ValidateConfigAttribute));

    for (const auto &attrib : attributes)
    {
        EGLAttrib pname = attrib.first;
        EGLAttrib value = attrib.second;
        ANGLE_VALIDATION_TRY(ValidateConfigAttributeValue(val, display, pname, value));
    }

    return true;
}

bool ValidateColorspaceAttribute(const ValidationContext *val,
                                 const DisplayExtensions &displayExtensions,
                                 EGLAttrib colorSpace)
{
    switch (colorSpace)
    {
        case EGL_GL_COLORSPACE_SRGB:
            break;
        case EGL_GL_COLORSPACE_LINEAR:
            break;
        case EGL_GL_COLORSPACE_DISPLAY_P3_LINEAR_EXT:
            if (!displayExtensions.glColorspaceDisplayP3Linear &&
                !displayExtensions.eglColorspaceAttributePassthroughANGLE)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "EXT_gl_colorspace_display_p3_linear is not available.");
                return false;
            }
            break;
        case EGL_GL_COLORSPACE_DISPLAY_P3_EXT:
            if (!displayExtensions.glColorspaceDisplayP3 &&
                !displayExtensions.eglColorspaceAttributePassthroughANGLE)
            {
                val->setError(EGL_BAD_ATTRIBUTE, "EXT_gl_colorspace_display_p3 is not available.");
                return false;
            }
            break;
        case EGL_GL_COLORSPACE_DISPLAY_P3_PASSTHROUGH_EXT:
            if (!displayExtensions.glColorspaceDisplayP3Passthrough &&
                !displayExtensions.eglColorspaceAttributePassthroughANGLE)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "EGL_EXT_gl_colorspace_display_p3_passthrough is not available.");
                return false;
            }
            break;
        case EGL_GL_COLORSPACE_SCRGB_EXT:
            if (!displayExtensions.glColorspaceScrgb &&
                !displayExtensions.eglColorspaceAttributePassthroughANGLE)
            {
                val->setError(EGL_BAD_ATTRIBUTE, "EXT_gl_colorspace_scrgb is not available.");
                return false;
            }
            break;
        case EGL_GL_COLORSPACE_SCRGB_LINEAR_EXT:
            if (!displayExtensions.glColorspaceScrgbLinear &&
                !displayExtensions.eglColorspaceAttributePassthroughANGLE)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "EXT_gl_colorspace_scrgb_linear is not available.");
                return false;
            }
            break;
        case EGL_GL_COLORSPACE_BT2020_LINEAR_EXT:
            if (!displayExtensions.glColorspaceBt2020Linear &&
                !displayExtensions.eglColorspaceAttributePassthroughANGLE)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "EXT_gl_colorspace_bt2020_linear is not available");
                return false;
            }
            break;
        case EGL_GL_COLORSPACE_BT2020_PQ_EXT:
            if (!displayExtensions.glColorspaceBt2020Pq &&
                !displayExtensions.eglColorspaceAttributePassthroughANGLE)
            {
                val->setError(EGL_BAD_ATTRIBUTE, "EXT_gl_colorspace_bt2020_pq is not available");
                return false;
            }
            break;
        case EGL_GL_COLORSPACE_BT2020_HLG_EXT:
            if (!displayExtensions.glColorspaceBt2020Hlg &&
                !displayExtensions.eglColorspaceAttributePassthroughANGLE)
            {
                val->setError(EGL_BAD_ATTRIBUTE, "EXT_gl_colorspace_bt2020_hlg is not available");
                return false;
            }
            break;
        default:
            val->setError(EGL_BAD_ATTRIBUTE);
            return false;
    }
    return true;
}
bool ValidatePlatformType(const ValidationContext *val,
                          const ClientExtensions &clientExtensions,
                          EGLAttrib platformType)
{
    switch (platformType)
    {
        case EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE:
            break;

        case EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE:
        case EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE:
            if (!clientExtensions.platformANGLED3D)
            {
                val->setError(EGL_BAD_ATTRIBUTE, "Direct3D platform is unsupported.");
                return false;
            }
            break;

        case EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE:
        case EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE:
            if (!clientExtensions.platformANGLEOpenGL)
            {
                val->setError(EGL_BAD_ATTRIBUTE, "OpenGL platform is unsupported.");
                return false;
            }
            break;

        case EGL_PLATFORM_ANGLE_TYPE_NULL_ANGLE:
            if (!clientExtensions.platformANGLENULL)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "Display type EGL_PLATFORM_ANGLE_TYPE_NULL_ANGLE "
                              "requires EGL_ANGLE_platform_angle_null.");
                return false;
            }
            break;

        case EGL_PLATFORM_ANGLE_TYPE_WEBGPU_ANGLE:
            if (!clientExtensions.platformANGLEWebgpu)
            {
                val->setError(EGL_BAD_ATTRIBUTE, "WebGPU platform is unsupported.");
                return false;
            }
            break;

        case EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE:
            if (!clientExtensions.platformANGLEVulkan)
            {
                val->setError(EGL_BAD_ATTRIBUTE, "Vulkan platform is unsupported.");
                return false;
            }
            break;

        case EGL_PLATFORM_ANGLE_TYPE_METAL_ANGLE:
            if (!clientExtensions.platformANGLEMetal)
            {
                val->setError(EGL_BAD_ATTRIBUTE, "Metal platform is unsupported.");
                return false;
            }
            break;

        default:
            val->setError(EGL_BAD_ATTRIBUTE, "Unknown platform type.");
            return false;
    }

    return true;
}

bool ValidateGetPlatformDisplayCommon(const ValidationContext *val,
                                      EGLenum platform,
                                      const void *native_display,
                                      const AttributeMap &attribMap)
{
    const ClientExtensions &clientExtensions = Display::GetClientExtensions();

    switch (platform)
    {
        case EGL_PLATFORM_ANGLE_ANGLE:
            if (!clientExtensions.platformANGLE)
            {
                val->setError(EGL_BAD_PARAMETER, "Platform ANGLE extension is not active");
                return false;
            }
            break;
        case EGL_PLATFORM_DEVICE_EXT:
            if (!clientExtensions.platformDevice)
            {
                val->setError(EGL_BAD_PARAMETER, "Platform Device extension is not active");
                return false;
            }
            break;
        case EGL_PLATFORM_GBM_KHR:
            if (!clientExtensions.platformGbmKHR)
            {
                val->setError(EGL_BAD_PARAMETER, "Platform GBM extension is not active");
                return false;
            }
            break;
        case EGL_PLATFORM_WAYLAND_EXT:
            if (!clientExtensions.platformWaylandEXT)
            {
                val->setError(EGL_BAD_PARAMETER, "Platform Wayland extension is not active");
                return false;
            }
            break;
        case EGL_PLATFORM_SURFACELESS_MESA:
            if (!clientExtensions.platformSurfacelessMESA)
            {
                val->setError(EGL_BAD_PARAMETER, "Platform Surfaceless extension is not active");
                return false;
            }
            break;
        default:
            val->setError(EGL_BAD_CONFIG, "Bad platform type.");
            return false;
    }

    attribMap.initializeWithoutValidation();

    if (platform != EGL_PLATFORM_DEVICE_EXT)
    {
        EGLAttrib platformType       = EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE;
        bool enableAutoTrimSpecified = false;
        bool enableD3D11on12         = false;
        bool presentPathSpecified    = false;
        bool luidSpecified           = false;
        bool deviceIdSpecified       = false;
        bool vkDeviceUuidSpecified   = false;

        Optional<EGLAttrib> majorVersion;
        Optional<EGLAttrib> minorVersion;
        Optional<EGLAttrib> deviceType;
        Optional<EGLAttrib> eglHandle;

        for (const auto &curAttrib : attribMap)
        {
            const EGLAttrib value = curAttrib.second;

            switch (curAttrib.first)
            {
                case EGL_PLATFORM_ANGLE_TYPE_ANGLE:
                {
                    ANGLE_VALIDATION_TRY(ValidatePlatformType(val, clientExtensions, value));
                    platformType = value;
                    break;
                }

                case EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE:
                    if (value != EGL_DONT_CARE)
                    {
                        majorVersion = value;
                    }
                    break;

                case EGL_PLATFORM_ANGLE_MAX_VERSION_MINOR_ANGLE:
                    if (value != EGL_DONT_CARE)
                    {
                        minorVersion = value;
                    }
                    break;

                case EGL_PLATFORM_ANGLE_ENABLE_AUTOMATIC_TRIM_ANGLE:
                    switch (value)
                    {
                        case EGL_TRUE:
                        case EGL_FALSE:
                            break;
                        default:
                            val->setError(EGL_BAD_ATTRIBUTE, "Invalid automatic trim attribute");
                            return false;
                    }
                    enableAutoTrimSpecified = true;
                    break;

                case EGL_PLATFORM_ANGLE_D3D11ON12_ANGLE:
                    if (!clientExtensions.platformANGLED3D ||
                        !clientExtensions.platformANGLED3D11ON12)
                    {
                        val->setError(EGL_BAD_ATTRIBUTE,
                                      "EGL_PLATFORM_ANGLE_D3D11ON12_ANGLE extension not active.");
                        return false;
                    }

                    switch (value)
                    {
                        case EGL_TRUE:
                        case EGL_FALSE:
                            break;
                        default:
                            val->setError(EGL_BAD_ATTRIBUTE, "Invalid D3D11on12 attribute");
                            return false;
                    }
                    enableD3D11on12 = true;
                    break;

                case EGL_EXPERIMENTAL_PRESENT_PATH_ANGLE:
                    if (!clientExtensions.experimentalPresentPath)
                    {
                        val->setError(EGL_BAD_ATTRIBUTE,
                                      "EGL_ANGLE_experimental_present_path extension not active");
                        return false;
                    }

                    switch (value)
                    {
                        case EGL_EXPERIMENTAL_PRESENT_PATH_FAST_ANGLE:
                        case EGL_EXPERIMENTAL_PRESENT_PATH_COPY_ANGLE:
                            break;
                        default:
                            val->setError(EGL_BAD_ATTRIBUTE,
                                          "Invalid value for EGL_EXPERIMENTAL_PRESENT_PATH_ANGLE");
                            return false;
                    }
                    presentPathSpecified = true;
                    break;

                case EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE:
                    switch (value)
                    {
                        case EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE:
                        case EGL_PLATFORM_ANGLE_DEVICE_TYPE_NULL_ANGLE:
                            break;

                        case EGL_PLATFORM_ANGLE_DEVICE_TYPE_D3D_WARP_ANGLE:
                        case EGL_PLATFORM_ANGLE_DEVICE_TYPE_D3D_REFERENCE_ANGLE:
                            if (!clientExtensions.platformANGLED3D)
                            {
                                val->setError(EGL_BAD_ATTRIBUTE,
                                              "EGL_ANGLE_platform_angle_d3d is not supported");
                                return false;
                            }
                            break;

                        case EGL_PLATFORM_ANGLE_DEVICE_TYPE_EGL_ANGLE:
                            if (!clientExtensions.platformANGLEDeviceTypeEGLANGLE)
                            {
                                val->setError(EGL_BAD_ATTRIBUTE,
                                              "EGL_ANGLE_platform_angle_device_type_"
                                              "egl_angle is not supported");
                                return false;
                            }
                            break;

                        case EGL_PLATFORM_ANGLE_DEVICE_TYPE_SWIFTSHADER_ANGLE:
                            if (!clientExtensions.platformANGLEDeviceTypeSwiftShader)
                            {
                                val->setError(EGL_BAD_ATTRIBUTE,
                                              "EGL_ANGLE_platform_angle_device_type_"
                                              "swiftshader is not supported");
                                return false;
                            }
                            break;

                        default:
                            val->setError(EGL_BAD_ATTRIBUTE,
                                          "Invalid value for "
                                          "EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE "
                                          "attrib");
                            return false;
                    }
                    deviceType = value;
                    break;

                case EGL_PLATFORM_ANGLE_DEBUG_LAYERS_ENABLED_ANGLE:
                    if (!clientExtensions.platformANGLE)
                    {
                        val->setError(EGL_BAD_ATTRIBUTE,
                                      "EGL_ANGLE_platform_angle extension not active");
                        return false;
                    }
                    if (value != EGL_TRUE && value != EGL_FALSE && value != EGL_DONT_CARE)
                    {
                        val->setError(EGL_BAD_ATTRIBUTE,
                                      "EGL_PLATFORM_ANGLE_DEBUG_LAYERS_ENABLED_ANGLE "
                                      "must be EGL_TRUE, EGL_FALSE, or "
                                      "EGL_DONT_CARE.");
                        return false;
                    }
                    break;

                case EGL_PLATFORM_ANGLE_EGL_HANDLE_ANGLE:
                    if (value != EGL_DONT_CARE)
                    {
                        eglHandle = value;
                    }
                    break;

                case EGL_PLATFORM_ANGLE_D3D_LUID_HIGH_ANGLE:
                case EGL_PLATFORM_ANGLE_D3D_LUID_LOW_ANGLE:
                    luidSpecified = true;
                    break;

                case EGL_PLATFORM_ANGLE_VULKAN_DEVICE_UUID_ANGLE:
                case EGL_PLATFORM_ANGLE_VULKAN_DRIVER_UUID_ANGLE:
                case EGL_PLATFORM_ANGLE_VULKAN_DRIVER_ID_ANGLE:
                    vkDeviceUuidSpecified = true;
                    break;

                case EGL_PLATFORM_ANGLE_DEVICE_CONTEXT_VOLATILE_CGL_ANGLE:
                    // The property does not have an effect if it's not active, so do not check
                    // for non-support.
                    switch (value)
                    {
                        case EGL_FALSE:
                        case EGL_TRUE:
                            break;
                        default:
                            val->setError(EGL_BAD_ATTRIBUTE,
                                          "Invalid value for "
                                          "EGL_PLATFORM_ANGLE_DEVICE_CONTEXT_VOLATILE_"
                                          "CGL_ANGLE attrib");
                            return false;
                    }
                    break;
                case EGL_PLATFORM_ANGLE_DEVICE_ID_HIGH_ANGLE:
                case EGL_PLATFORM_ANGLE_DEVICE_ID_LOW_ANGLE:
                case EGL_PLATFORM_ANGLE_DISPLAY_KEY_ANGLE:
                    if (!clientExtensions.platformANGLEDeviceId)
                    {
                        val->setError(EGL_BAD_ATTRIBUTE,
                                      "EGL_ANGLE_platform_angle_device_id is not supported");
                        return false;
                    }
                    deviceIdSpecified = true;
                    break;
                default:
                    break;
            }
        }

        if (!majorVersion.valid() && minorVersion.valid())
        {
            val->setError(EGL_BAD_ATTRIBUTE,
                          "Must specify major version if you specify a minor version.");
            return false;
        }

        if (deviceType == EGL_PLATFORM_ANGLE_DEVICE_TYPE_D3D_WARP_ANGLE &&
            platformType != EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE)
        {
            val->setError(EGL_BAD_ATTRIBUTE,
                          "EGL_PLATFORM_ANGLE_DEVICE_TYPE_WARP_ANGLE requires a "
                          "device type of EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE.");
            return false;
        }

        if (enableAutoTrimSpecified && platformType != EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE)
        {
            val->setError(EGL_BAD_ATTRIBUTE,
                          "EGL_PLATFORM_ANGLE_ENABLE_AUTOMATIC_TRIM_ANGLE "
                          "requires a device type of "
                          "EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE.");
            return false;
        }

        if (enableD3D11on12)
        {
            if (platformType != EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "EGL_PLATFORM_ANGLE_D3D11ON12_ANGLE "
                              "requires a platform type of "
                              "EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE.");
                return false;
            }

            if (deviceType.valid() && deviceType != EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE &&
                deviceType != EGL_PLATFORM_ANGLE_DEVICE_TYPE_D3D_WARP_ANGLE)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "EGL_PLATFORM_ANGLE_D3D11ON12_ANGLE requires a device "
                              "type of EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE "
                              "or EGL_PLATFORM_ANGLE_DEVICE_TYPE_D3D_WARP_ANGLE");
                return false;
            }
        }

        if (presentPathSpecified && platformType != EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE)
        {
            val->setError(EGL_BAD_ATTRIBUTE,
                          "EGL_EXPERIMENTAL_PRESENT_PATH_ANGLE requires a "
                          "device type of EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE.");
            return false;
        }

        if (luidSpecified)
        {
            if (platformType != EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "EGL_PLATFORM_ANGLE_D3D_LUID_HIGH_ANGLE and "
                              "EGL_PLATFORM_ANGLE_D3D_LUID_LOW_ANGLE "
                              "require a platform type of "
                              "EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE.");
                return false;
            }

            if (attribMap.get(EGL_PLATFORM_ANGLE_D3D_LUID_HIGH_ANGLE, 0) == 0 &&
                attribMap.get(EGL_PLATFORM_ANGLE_D3D_LUID_LOW_ANGLE, 0) == 0)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "If either EGL_PLATFORM_ANGLE_D3D_LUID_HIGH_ANGLE "
                              "and/or EGL_PLATFORM_ANGLE_D3D_LUID_LOW_ANGLE are "
                              "specified, at least one must non-zero.");
                return false;
            }
        }

        if (vkDeviceUuidSpecified)
        {
            if (platformType != EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "EGL_PLATFORM_ANGLE_VULKAN_DEVICE_UUID, "
                              "EGL_PLATFORM_ANGLE_VULKAN_DRIVER_UUID and "
                              "EGL_PLATFORM_ANGLE_VULKAN_DRIVER_ID "
                              "require a platform type of "
                              "EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE.");
                return false;
            }
        }

        if (deviceIdSpecified)
        {
            if (attribMap.get(EGL_PLATFORM_ANGLE_DEVICE_ID_HIGH_ANGLE, 0) == 0 &&
                attribMap.get(EGL_PLATFORM_ANGLE_DEVICE_ID_LOW_ANGLE, 0) == 0)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "If either EGL_PLATFORM_ANGLE_DEVICE_ID_HIGH_ANGLE "
                              "and/or EGL_PLATFORM_ANGLE_DEVICE_ID_LOW_ANGLE are "
                              "specified, at least one must non-zero.");
                return false;
            }
        }

        if (deviceType.valid())
        {
            switch (deviceType.value())
            {
                case EGL_PLATFORM_ANGLE_DEVICE_TYPE_D3D_REFERENCE_ANGLE:
                case EGL_PLATFORM_ANGLE_DEVICE_TYPE_D3D_WARP_ANGLE:
                    if (platformType != EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE &&
                        platformType != EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE)
                    {
                        val->setError(EGL_BAD_ATTRIBUTE,
                                      "This device type requires a "
                                      "platform type of EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE or "
                                      "EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE.");
                        return false;
                    }
                    break;

                case EGL_PLATFORM_ANGLE_DEVICE_TYPE_SWIFTSHADER_ANGLE:
                    if (platformType != EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE)
                    {
                        val->setError(EGL_BAD_ATTRIBUTE,
                                      "This device type requires a "
                                      "platform type of EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE.");
                        return false;
                    }
                    break;

                default:
                    break;
            }
        }

        if (platformType == EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE)
        {
            if ((majorVersion.valid() && majorVersion.value() != 1) ||
                (minorVersion.valid() && minorVersion.value() != 0))
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE currently "
                              "only supports Vulkan 1.0.");
                return false;
            }
        }

        if (eglHandle.valid() && platformType != EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE &&
            platformType != EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE)
        {
            val->setError(EGL_BAD_ATTRIBUTE,
                          "EGL_PLATFORM_ANGLE_EGL_HANDLE_ANGLE requires a "
                          "device type of EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE.");
            return false;
        }
    }
    else
    {
        const Device *eglDevice = static_cast<const Device *>(native_display);
        if (eglDevice == nullptr || !Device::IsValidDevice(eglDevice))
        {
            val->setError(EGL_BAD_ATTRIBUTE,
                          "native_display should be a valid EGL device if "
                          "platform equals EGL_PLATFORM_DEVICE_EXT");
            return false;
        }
    }

    if (attribMap.contains(EGL_POWER_PREFERENCE_ANGLE))
    {
        if (!clientExtensions.displayPowerPreferenceANGLE)
        {
            val->setError(EGL_BAD_ATTRIBUTE,
                          "Attribute EGL_POWER_PREFERENCE_ANGLE "
                          "requires EGL_ANGLE_display_power_preference.");
            return false;
        }
        EGLAttrib value = attribMap.get(EGL_POWER_PREFERENCE_ANGLE, 0);
        if (value != EGL_LOW_POWER_ANGLE && value != EGL_HIGH_POWER_ANGLE)
        {
            val->setError(EGL_BAD_ATTRIBUTE,
                          "EGL_POWER_PREFERENCE_ANGLE must be "
                          "either EGL_LOW_POWER_ANGLE or EGL_HIGH_POWER_ANGLE.");
            return false;
        }
    }

    if (attribMap.contains(EGL_FEATURE_OVERRIDES_ENABLED_ANGLE))
    {
        if (!clientExtensions.featureControlANGLE)
        {
            val->setError(EGL_BAD_ATTRIBUTE, "EGL_ANGLE_feature_control is not supported");
            return false;
        }
        else if (attribMap.get(EGL_FEATURE_OVERRIDES_ENABLED_ANGLE, 0) == 0)
        {
            val->setError(EGL_BAD_ATTRIBUTE,
                          "EGL_FEATURE_OVERRIDES_ENABLED_ANGLE must be a valid pointer");
            return false;
        }
    }
    if (attribMap.contains(EGL_FEATURE_OVERRIDES_DISABLED_ANGLE))
    {
        if (!clientExtensions.featureControlANGLE)
        {
            val->setError(EGL_BAD_ATTRIBUTE, "EGL_ANGLE_feature_control is not supported");
            return false;
        }
        else if (attribMap.get(EGL_FEATURE_OVERRIDES_DISABLED_ANGLE, 0) == 0)
        {
            val->setError(EGL_BAD_ATTRIBUTE,
                          "EGL_FEATURE_OVERRIDES_DISABLED_ANGLE must be a valid pointer");
            return false;
        }
    }

    return true;
}

bool ValidateStream(const ValidationContext *val, const Display *display, const Stream *stream)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));

    const DisplayExtensions &displayExtensions = display->getExtensions();
    if (!displayExtensions.stream)
    {
        val->setError(EGL_BAD_ACCESS, "Stream extension not active");
        return false;
    }

    if (stream == EGL_NO_STREAM_KHR || !display->isValidStream(stream))
    {
        val->setError(EGL_BAD_STREAM_KHR, "Invalid stream");
        return false;
    }

    return true;
}

bool ValidateLabeledObject(const ValidationContext *val,
                           const Display *display,
                           ObjectType objectType,
                           EGLObjectKHR object,
                           const LabeledObject **outLabeledObject)
{
    switch (objectType)
    {
        case ObjectType::Context:
        {
            EGLContext context      = static_cast<EGLContext>(object);
            gl::ContextID contextID = PackParam<gl::ContextID>(context);
            ANGLE_VALIDATION_TRY(ValidateContext(val, display, contextID));
            *outLabeledObject = display->getContext(contextID);
            break;
        }

        case ObjectType::Display:
        {
            ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));
            if (display != object)
            {
                if (val)
                {
                    val->setError(EGL_BAD_PARAMETER,
                                  "when object type is EGL_OBJECT_DISPLAY_KHR, the "
                                  "object must be the same as the display.");
                }
                return false;
            }

            *outLabeledObject = static_cast<Display *>(object);
            break;
        }

        case ObjectType::Image:
        {
            EGLImage image  = static_cast<EGLImage>(object);
            ImageID imageID = PackParam<ImageID>(image);
            ANGLE_VALIDATION_TRY(ValidateImage(val, display, imageID));
            *outLabeledObject = display->getImage(imageID);
            break;
        }

        case ObjectType::Stream:
        {
            Stream *stream = static_cast<Stream *>(object);
            ANGLE_VALIDATION_TRY(ValidateStream(val, display, stream));
            *outLabeledObject = stream;
            break;
        }

        case ObjectType::Surface:
        {
            EGLSurface surface  = static_cast<EGLSurface>(object);
            SurfaceID surfaceID = PackParam<SurfaceID>(surface);
            ANGLE_VALIDATION_TRY(ValidateSurface(val, display, surfaceID));
            *outLabeledObject = display->getSurface(surfaceID);
            break;
        }

        case ObjectType::Sync:
        {
            Sync *sync    = static_cast<Sync *>(object);
            SyncID syncID = PackParam<SyncID>(sync);
            ANGLE_VALIDATION_TRY(ValidateSync(val, display, syncID));
            *outLabeledObject = sync;
            break;
        }

        case ObjectType::Thread:
        {
            ASSERT(val);
            *outLabeledObject = val->eglThread;
            break;
        }

        default:
            if (val)
            {
                val->setError(EGL_BAD_PARAMETER, "unknown object type.");
            }
            return false;
    }

    return true;
}

bool ValidateLabeledObject(const ValidationContext *val,
                           Display *display,
                           ObjectType objectType,
                           EGLObjectKHR object,
                           LabeledObject **outLabeledObject)
{
    return ValidateLabeledObject(val, const_cast<const Display *>(display), objectType, object,
                                 const_cast<const LabeledObject **>(outLabeledObject));
}

// This is a common sub-check of Display status that's shared by multiple functions
bool ValidateDisplayPointer(const ValidationContext *val, const Display *display)
{
    if (display == EGL_NO_DISPLAY)
    {
        if (val)
        {
            val->setError(EGL_BAD_DISPLAY, "display is EGL_NO_DISPLAY.");
        }
        return false;
    }

    if (!Display::isValidDisplay(display))
    {
        if (val)
        {
            val->setError(EGL_BAD_DISPLAY, "display is not a valid display: 0x%p", display);
        }
        return false;
    }

    return true;
}

bool ValidCompositorTimingName(CompositorTiming name)
{
    switch (name)
    {
        case CompositorTiming::CompositeDeadline:
        case CompositorTiming::CompositInterval:
        case CompositorTiming::CompositToPresentLatency:
            return true;

        default:
            return false;
    }
}

bool ValidTimestampType(Timestamp timestamp)
{
    switch (timestamp)
    {
        case Timestamp::RequestedPresentTime:
        case Timestamp::RenderingCompleteTime:
        case Timestamp::CompositionLatchTime:
        case Timestamp::FirstCompositionStartTime:
        case Timestamp::LastCompositionStartTime:
        case Timestamp::FirstCompositionGPUFinishedTime:
        case Timestamp::DisplayPresentTime:
        case Timestamp::DequeueReadyTime:
        case Timestamp::ReadsDoneTime:
            return true;

        default:
            return false;
    }
}

bool ValidateCompatibleSurface(const ValidationContext *val,
                               const Display *display,
                               const gl::Context *context,
                               const Surface *surface)
{
    const Config *contextConfig = context->getConfig();
    const Config *surfaceConfig = surface->getConfig();

    // Surface compatible with client API - only OPENGL_ES supported
    switch (context->getClientMajorVersion())
    {
        case 1:
            if (!(surfaceConfig->renderableType & EGL_OPENGL_ES_BIT))
            {
                val->setError(EGL_BAD_MATCH, "Surface not compatible with OpenGL ES 1.x.");
                return false;
            }
            break;
        case 2:
            if (!(surfaceConfig->renderableType & EGL_OPENGL_ES2_BIT))
            {
                val->setError(EGL_BAD_MATCH, "Surface not compatible with OpenGL ES 2.x.");
                return false;
            }
            break;
        case 3:
            if (!(surfaceConfig->renderableType & (EGL_OPENGL_ES2_BIT | EGL_OPENGL_ES3_BIT)))
            {
                val->setError(EGL_BAD_MATCH, "Surface not compatible with OpenGL ES 3.x.");
                return false;
            }
            break;
        default:
            val->setError(EGL_BAD_MATCH, "Surface not compatible with Context API.");
            return false;
    }

    // EGL KHR no config context
    if (context->getConfig() == EGL_NO_CONFIG_KHR)
    {
        const DisplayExtensions &displayExtensions = display->getExtensions();
        if (displayExtensions.noConfigContext)
        {
            return true;
        }
        val->setError(EGL_BAD_MATCH, "Context with no config is not supported.");
        return false;
    }

    // Config compatibility is defined in section 2.2 of the EGL 1.5 spec

    bool colorBufferCompat = surfaceConfig->colorBufferType == contextConfig->colorBufferType;
    if (!colorBufferCompat)
    {
        val->setError(EGL_BAD_MATCH, "Color buffer types are not compatible.");
        return false;
    }

    bool colorCompat = surfaceConfig->redSize == contextConfig->redSize &&
                       surfaceConfig->greenSize == contextConfig->greenSize &&
                       surfaceConfig->blueSize == contextConfig->blueSize &&
                       surfaceConfig->alphaSize == contextConfig->alphaSize &&
                       surfaceConfig->luminanceSize == contextConfig->luminanceSize;
    if (!colorCompat)
    {
        val->setError(EGL_BAD_MATCH, "Color buffer sizes are not compatible.");
        return false;
    }

    bool componentTypeCompat =
        surfaceConfig->colorComponentType == contextConfig->colorComponentType;
    if (!componentTypeCompat)
    {
        val->setError(EGL_BAD_MATCH, "Color buffer component types are not compatible.");
        return false;
    }

    bool dsCompat = surfaceConfig->depthSize == contextConfig->depthSize &&
                    surfaceConfig->stencilSize == contextConfig->stencilSize;
    if (!dsCompat)
    {
        val->setError(EGL_BAD_MATCH, "Depth-stencil buffer types are not compatible.");
        return false;
    }

    bool surfaceTypeCompat = (surfaceConfig->surfaceType & contextConfig->surfaceType) != 0;
    if (!surfaceTypeCompat)
    {
        val->setError(EGL_BAD_MATCH, "Surface type is not compatible.");
        return false;
    }

    return true;
}

bool ValidateSurfaceBadAccess(const ValidationContext *val,
                              const gl::Context *previousContext,
                              const Surface *surface)
{
    if (surface->isReferenced() &&
        (previousContext == nullptr || (surface != previousContext->getCurrentDrawSurface() &&
                                        surface != previousContext->getCurrentReadSurface())))
    {
        val->setError(EGL_BAD_ACCESS, "Surface can only be current on one thread");
        return false;
    }
    return true;
}

bool ValidateSyncAttribute(const ValidationContext *val,
                           const Display *display,
                           EGLAttrib attribute)
{
    switch (attribute)
    {
        case EGL_SYNC_CONDITION:
        case EGL_SYNC_NATIVE_FENCE_FD_ANDROID:
        case EGL_SYNC_METAL_SHARED_EVENT_OBJECT_ANGLE:
        case EGL_SYNC_METAL_SHARED_EVENT_SIGNAL_VALUE_LO_ANGLE:
        case EGL_SYNC_METAL_SHARED_EVENT_SIGNAL_VALUE_HI_ANGLE:
            break;

        default:
            val->setError(EGL_BAD_ATTRIBUTE, "Unknown attribute: 0x%04" PRIxPTR "X", attribute);
            return false;
    }

    return true;
}

bool ValidateCreateSyncBase(const ValidationContext *val,
                            const Display *display,
                            EGLenum type,
                            const AttributeMap &attribs,
                            bool isExt)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));
    ANGLE_VALIDATION_TRY(attribs.validate(val, display, ValidateSyncAttribute));

    gl::Context *currentContext  = val->eglThread->getContext();
    egl::Display *currentDisplay = currentContext ? currentContext->getDisplay() : nullptr;

    switch (type)
    {
        case EGL_SYNC_FENCE_KHR:
        case EGL_SYNC_GLOBAL_FENCE_ANGLE:
            if (!attribs.isEmpty())
            {
                val->setError(EGL_BAD_ATTRIBUTE, "Invalid attribute");
                return false;
            }

            if (!display->getExtensions().fenceSync)
            {
                val->setError(EGL_BAD_MATCH, "EGL_KHR_fence_sync extension is not available");
                return false;
            }

            if (type == EGL_SYNC_GLOBAL_FENCE_ANGLE)
            {
                if (!display->getExtensions().globalFenceSyncANGLE)
                {
                    val->setError(EGL_BAD_MATCH,
                                  "EGL_ANGLE_global_fence_sync extension is not available");
                    return false;
                }
            }

            if (display != currentDisplay)
            {
                val->setError(EGL_BAD_MATCH,
                              "CreateSync can only be called on the current display");
                return false;
            }

            ANGLE_VALIDATION_TRY(ValidateContext(val, currentDisplay, currentContext->id()));

            if (!currentContext->getExtensions().EGLSyncOES)
            {
                val->setError(EGL_BAD_MATCH,
                              "EGL_SYNC_FENCE_KHR cannot be used without "
                              "GL_OES_EGL_sync support.");
                return false;
            }
            break;

        case EGL_SYNC_NATIVE_FENCE_ANDROID:
            if (!display->getExtensions().fenceSync)
            {
                val->setError(EGL_BAD_MATCH, "EGL_KHR_fence_sync extension is not available");
                return false;
            }

            if (!display->getExtensions().nativeFenceSyncANDROID)
            {
                val->setError(EGL_BAD_DISPLAY,
                              "EGL_ANDROID_native_fence_sync extension is not available.");
                return false;
            }

            if (display != currentDisplay)
            {
                val->setError(EGL_BAD_MATCH,
                              "CreateSync can only be called on the current display");
                return false;
            }

            ANGLE_VALIDATION_TRY(ValidateContext(val, currentDisplay, currentContext->id()));

            if (!currentContext->getExtensions().EGLSyncOES)
            {
                val->setError(EGL_BAD_MATCH,
                              "EGL_SYNC_NATIVE_FENCE_ANDROID cannot be used without "
                              "GL_OES_EGL_sync support.");
                return false;
            }

            for (const auto &attributeIter : attribs)
            {
                EGLAttrib attribute = attributeIter.first;

                switch (attribute)
                {
                    case EGL_SYNC_NATIVE_FENCE_FD_ANDROID:
                        break;

                    default:
                        val->setError(EGL_BAD_ATTRIBUTE, "Invalid attribute");
                        return false;
                }
            }
            break;

        case EGL_SYNC_REUSABLE_KHR:
            if (!attribs.isEmpty())
            {
                val->setError(EGL_BAD_ATTRIBUTE, "Invalid attribute");
                return false;
            }

            if (!display->getExtensions().reusableSyncKHR)
            {
                val->setError(EGL_BAD_MATCH, "EGL_KHR_reusable_sync extension is not available.");
                return false;
            }
            break;

        case EGL_SYNC_METAL_SHARED_EVENT_ANGLE:
            if (!display->getExtensions().fenceSync)
            {
                val->setError(EGL_BAD_MATCH, "EGL_KHR_fence_sync extension is not available");
                return false;
            }

            if (!display->getExtensions().mtlSyncSharedEventANGLE)
            {
                val->setError(EGL_BAD_DISPLAY,
                              "EGL_ANGLE_metal_shared_event_sync is not available");
                return false;
            }

            if (display != currentDisplay)
            {
                val->setError(EGL_BAD_MATCH,
                              "CreateSync can only be called on the current display");
                return false;
            }

            ANGLE_VALIDATION_TRY(ValidateContext(val, currentDisplay, currentContext->id()));

            // This should be implied by exposing EGL_KHR_fence_sync
            ASSERT(currentContext->getExtensions().EGLSyncOES);

            for (const auto &attributeIter : attribs)
            {
                EGLAttrib attribute = attributeIter.first;
                EGLAttrib value     = attributeIter.second;

                switch (attribute)
                {
                    case EGL_SYNC_CONDITION:
                        if (type != EGL_SYNC_METAL_SHARED_EVENT_ANGLE ||
                            (value != EGL_SYNC_PRIOR_COMMANDS_COMPLETE_KHR &&
                             value != EGL_SYNC_METAL_SHARED_EVENT_SIGNALED_ANGLE))
                        {
                            val->setError(EGL_BAD_ATTRIBUTE, "Invalid attribute");
                        }
                        break;

                    case EGL_SYNC_METAL_SHARED_EVENT_OBJECT_ANGLE:
                        if (!value)
                        {
                            val->setError(EGL_BAD_ATTRIBUTE,
                                          "EGL_SYNC_METAL_SHARED_EVENT_ANGLE can't be NULL");
                            return false;
                        }
                        break;

                    case EGL_SYNC_METAL_SHARED_EVENT_SIGNAL_VALUE_LO_ANGLE:
                    case EGL_SYNC_METAL_SHARED_EVENT_SIGNAL_VALUE_HI_ANGLE:
                        break;

                    default:
                        val->setError(EGL_BAD_ATTRIBUTE, "Invalid attribute");
                        return false;
                }
            }
            break;

        default:
            if (isExt)
            {
                val->setError(EGL_BAD_ATTRIBUTE, "Invalid type parameter");
                return false;
            }
            else
            {
                val->setError(EGL_BAD_PARAMETER, "Invalid type parameter");
                return false;
            }
    }

    return true;
}

bool ValidateGetSyncAttribBase(const ValidationContext *val,
                               const Display *display,
                               SyncID sync,
                               EGLint attribute)
{
    ANGLE_VALIDATION_TRY(ValidateSync(val, display, sync));

    const Sync *syncObj = display->getSync(sync);

    switch (attribute)
    {
        case EGL_SYNC_CONDITION_KHR:
            switch (syncObj->getType())
            {
                case EGL_SYNC_FENCE_KHR:
                case EGL_SYNC_NATIVE_FENCE_ANDROID:
                case EGL_SYNC_GLOBAL_FENCE_ANGLE:
                case EGL_SYNC_METAL_SHARED_EVENT_ANGLE:
                    break;

                default:
                    val->setError(EGL_BAD_ATTRIBUTE,
                                  "EGL_SYNC_CONDITION_KHR is not valid for this sync type.");
                    return false;
            }
            break;

        // The following attributes are accepted by all types
        case EGL_SYNC_TYPE_KHR:
        case EGL_SYNC_STATUS_KHR:
            break;

        default:
            val->setError(EGL_BAD_ATTRIBUTE, "Invalid attribute");
            return false;
    }

    return true;
}

bool ValidateQueryDisplayAttribBase(const ValidationContext *val,
                                    const Display *display,
                                    const EGLint attribute)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));

    switch (attribute)
    {
        case EGL_DEVICE_EXT:
            if (!Display::GetClientExtensions().deviceQueryEXT)
            {
                val->setError(EGL_BAD_DISPLAY, "EGL_EXT_device_query extension is not available.");
                return false;
            }
            break;

        case EGL_FEATURE_COUNT_ANGLE:
            if (!Display::GetClientExtensions().featureControlANGLE)
            {
                val->setError(EGL_BAD_DISPLAY,
                              "EGL_ANGLE_feature_control extension is not available.");
                return false;
            }
            break;

        default:
            val->setError(EGL_BAD_ATTRIBUTE, "attribute is not valid.");
            return false;
    }

    return true;
}

bool ValidateCreateContextAttribute(const ValidationContext *val,
                                    const Display *display,
                                    EGLAttrib attribute)
{
    switch (attribute)
    {
        case EGL_CONTEXT_CLIENT_VERSION:
        case EGL_CONTEXT_MINOR_VERSION:
        case EGL_CONTEXT_FLAGS_KHR:
        case EGL_CONTEXT_OPENGL_DEBUG:
        case EGL_CONTEXT_OPENGL_ROBUST_ACCESS:
        case EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR:
            break;

        case EGL_CONTEXT_OPENGL_ROBUST_ACCESS_EXT:
            if (!display->getExtensions().createContextRobustness)
            {
                val->setError(EGL_BAD_ATTRIBUTE);
                return false;
            }
            break;

        case EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_EXT:
            if (!display->getExtensions().createContextRobustness)
            {
                val->setError(EGL_BAD_ATTRIBUTE);
                return false;
            }
            break;

        case EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY:
        {
            // We either need to have -
            // 1. EGL 1.5 which added support for this as part of core spec
            // 2. EGL_KHR_create_context extension which requires EGL 1.4
            constexpr EGLint kRequiredMajorVersion = 1;
            constexpr EGLint kRequiredMinorVersion = 5;
            if ((kEglMajorVersion < kRequiredMajorVersion ||
                 kEglMinorVersion < kRequiredMinorVersion) &&
                !display->getExtensions().createContext)
            {
                val->setError(EGL_BAD_ATTRIBUTE);
                return false;
            }
            break;
        }

        case EGL_CONTEXT_OPENGL_NO_ERROR_KHR:
            if (!display->getExtensions().createContextNoError)
            {
                val->setError(EGL_BAD_ATTRIBUTE, "Invalid Context attribute.");
                return false;
            }
            break;

        case EGL_CONTEXT_WEBGL_COMPATIBILITY_ANGLE:
            if (!display->getExtensions().createContextWebGLCompatibility)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "Attribute "
                              "EGL_CONTEXT_WEBGL_COMPATIBILITY_ANGLE requires "
                              "EGL_ANGLE_create_context_webgl_compatibility.");
                return false;
            }
            break;

        case EGL_CONTEXT_BIND_GENERATES_RESOURCE_CHROMIUM:
            if (!display->getExtensions().createContextBindGeneratesResource)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "Attribute EGL_CONTEXT_BIND_GENERATES_RESOURCE_CHROMIUM requires "
                              "EGL_CHROMIUM_create_context_bind_generates_resource.");
                return false;
            }
            break;

        case EGL_DISPLAY_TEXTURE_SHARE_GROUP_ANGLE:
            if (!display->getExtensions().displayTextureShareGroup)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "Attribute "
                              "EGL_DISPLAY_TEXTURE_SHARE_GROUP_ANGLE requires "
                              "EGL_ANGLE_display_texture_share_group.");
                return false;
            }
            break;

        case EGL_DISPLAY_SEMAPHORE_SHARE_GROUP_ANGLE:
            if (!display->getExtensions().displayTextureShareGroup)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "Attribute "
                              "EGL_DISPLAY_SEMAPHORE_SHARE_GROUP_ANGLE requires "
                              "EGL_ANGLE_display_semaphore_share_group.");
                return false;
            }
            break;

        case EGL_CONTEXT_CLIENT_ARRAYS_ENABLED_ANGLE:
            if (!display->getExtensions().createContextClientArrays)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "Attribute EGL_CONTEXT_CLIENT_ARRAYS_ENABLED_ANGLE requires "
                              "EGL_ANGLE_create_context_client_arrays.");
                return false;
            }
            break;

        case EGL_CONTEXT_PROGRAM_BINARY_CACHE_ENABLED_ANGLE:
            if (!display->getExtensions().programCacheControlANGLE)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "Attribute EGL_CONTEXT_PROGRAM_BINARY_CACHE_ENABLED_ANGLE "
                              "requires EGL_ANGLE_program_cache_control.");
                return false;
            }
            break;

        case EGL_ROBUST_RESOURCE_INITIALIZATION_ANGLE:
            if (!display->getExtensions().robustResourceInitializationANGLE)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "Attribute EGL_ROBUST_RESOURCE_INITIALIZATION_ANGLE "
                              "requires EGL_ANGLE_robust_resource_initialization.");
                return false;
            }
            break;

        case EGL_EXTENSIONS_ENABLED_ANGLE:
            if (!display->getExtensions().createContextExtensionsEnabled)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "Attribute EGL_EXTENSIONS_ENABLED_ANGLE "
                              "requires EGL_ANGLE_create_context_extensions_enabled.");
                return false;
            }
            break;

        case EGL_POWER_PREFERENCE_ANGLE:
            if (!display->getExtensions().powerPreference)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "Attribute EGL_POWER_PREFERENCE_ANGLE "
                              "requires EGL_ANGLE_power_preference.");
                return false;
            }
            break;

        case EGL_CONTEXT_OPENGL_BACKWARDS_COMPATIBLE_ANGLE:
            if (!display->getExtensions().createContextBackwardsCompatible)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "Attribute EGL_CONTEXT_OPENGL_BACKWARDS_COMPATIBLE_ANGLE "
                              "requires EGL_ANGLE_create_context_backwards_compatible.");
                return false;
            }
            break;

        case EGL_CONTEXT_PRIORITY_LEVEL_IMG:
            if (!display->getExtensions().contextPriority)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "Attribute EGL_CONTEXT_PRIORITY_LEVEL_IMG requires "
                              "extension EGL_IMG_context_priority.");
                return false;
            }
            break;

        case EGL_GENERATE_RESET_ON_VIDEO_MEMORY_PURGE_NV:
            if (!display->getExtensions().robustnessVideoMemoryPurgeNV)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "Attribute EGL_GENERATE_RESET_ON_VIDEO_MEMORY_PURGE_NV requires "
                              "extension EGL_NV_robustness_video_memory_purge.");
                return false;
            }
            break;

        case EGL_EXTERNAL_CONTEXT_ANGLE:
            if (!display->getExtensions().externalContextAndSurface)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "Attribute "
                              "EGL_EXTERNAL_CONTEXT_ANGLE requires "
                              "EGL_ANGLE_external_context_and_surface.");
                return false;
            }
            break;

        case EGL_PROTECTED_CONTENT_EXT:
            if (!display->getExtensions().protectedContentEXT)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "Attribute EGL_PROTECTED_CONTEXT_EXT requires "
                              "extension EGL_EXT_protected_content.");
                return false;
            }
            break;

        case EGL_CONTEXT_VIRTUALIZATION_GROUP_ANGLE:
            if (!display->getExtensions().contextVirtualizationANGLE)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "Attribute EGL_CONTEXT_VIRTUALIZATION_GROUP_ANGLE requires "
                              "extension EGL_ANGLE_context_virtualization.");
                return false;
            }
            break;

        case EGL_CONTEXT_METAL_OWNERSHIP_IDENTITY_ANGLE:
            if (!display->getExtensions().metalCreateContextOwnershipIdentityANGLE)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "Attribute EGL_CONTEXT_METAL_OWNERSHIP_IDENTITY_ANGLE requires "
                              "EGL_ANGLE_metal_create_context_ownership_identity.");
            }
            break;

        default:
            val->setError(EGL_BAD_ATTRIBUTE, "Unknown attribute: 0x%04" PRIxPTR "X", attribute);
            return false;
    }

    return true;
}

bool ValidateCreateContextAttributeValue(const ValidationContext *val,
                                         const Display *display,
                                         const gl::Context *shareContext,
                                         EGLAttrib attribute,
                                         EGLAttrib value)
{
    switch (attribute)
    {
        case EGL_CONTEXT_CLIENT_VERSION:
        case EGL_CONTEXT_MINOR_VERSION:
        case EGL_CONTEXT_OPENGL_DEBUG:
        case EGL_CONTEXT_VIRTUALIZATION_GROUP_ANGLE:
        case EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR:
            break;

        case EGL_CONTEXT_FLAGS_KHR:
        {
            // Note: EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE_BIT_KHR does not apply to ES
            constexpr EGLint kValidContextFlags =
                (EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR | EGL_CONTEXT_OPENGL_ROBUST_ACCESS_BIT_KHR);
            if ((value & ~kValidContextFlags) != 0)
            {
                val->setError(EGL_BAD_ATTRIBUTE);
                return false;
            }
            break;
        }

        case EGL_CONTEXT_OPENGL_ROBUST_ACCESS_EXT:
        case EGL_CONTEXT_OPENGL_ROBUST_ACCESS:
            if (value != EGL_TRUE && value != EGL_FALSE)
            {
                val->setError(EGL_BAD_ATTRIBUTE);
                return false;
            }
            break;

        case EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_EXT:
        case EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY:
            if (value != EGL_LOSE_CONTEXT_ON_RESET_EXT && value != EGL_NO_RESET_NOTIFICATION_EXT)
            {
                val->setError(EGL_BAD_ATTRIBUTE);
                return false;
            }

            if (shareContext && shareContext->isResetNotificationEnabled() !=
                                    (value == EGL_LOSE_CONTEXT_ON_RESET_EXT))
            {
                val->setError(EGL_BAD_MATCH);
                return false;
            }
            break;

        case EGL_CONTEXT_OPENGL_NO_ERROR_KHR:
            if (value != EGL_TRUE && value != EGL_FALSE)
            {
                val->setError(EGL_BAD_ATTRIBUTE, "Attribute must be EGL_TRUE or EGL_FALSE.");
                return false;
            }
            break;

        case EGL_CONTEXT_WEBGL_COMPATIBILITY_ANGLE:
            if (value != EGL_TRUE && value != EGL_FALSE)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "EGL_CONTEXT_WEBGL_COMPATIBILITY_ANGLE must be "
                              "EGL_TRUE or EGL_FALSE.");
                return false;
            }
            break;

        case EGL_CONTEXT_BIND_GENERATES_RESOURCE_CHROMIUM:
            if (value != EGL_TRUE && value != EGL_FALSE)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "EGL_CONTEXT_BIND_GENERATES_RESOURCE_CHROMIUM "
                              "must be EGL_TRUE or EGL_FALSE.");
                return false;
            }
            break;

        case EGL_DISPLAY_TEXTURE_SHARE_GROUP_ANGLE:
            if (value != EGL_TRUE && value != EGL_FALSE)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "EGL_DISPLAY_TEXTURE_SHARE_GROUP_ANGLE must be "
                              "EGL_TRUE or EGL_FALSE.");
                return false;
            }
            if (shareContext &&
                shareContext->usingDisplayTextureShareGroup() != (value == EGL_TRUE))
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "All contexts within a share group must be "
                              "created with the same value of "
                              "EGL_DISPLAY_TEXTURE_SHARE_GROUP_ANGLE.");
                return false;
            }
            break;

        case EGL_DISPLAY_SEMAPHORE_SHARE_GROUP_ANGLE:
            if (value != EGL_TRUE && value != EGL_FALSE)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "EGL_DISPLAY_SEMAPHORE_SHARE_GROUP_ANGLE must be "
                              "EGL_TRUE or EGL_FALSE.");
                return false;
            }
            if (shareContext &&
                shareContext->usingDisplaySemaphoreShareGroup() != (value == EGL_TRUE))
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "All contexts within a share group must be "
                              "created with the same value of "
                              "EGL_DISPLAY_SEMAPHORE_SHARE_GROUP_ANGLE.");
                return false;
            }
            break;

        case EGL_CONTEXT_CLIENT_ARRAYS_ENABLED_ANGLE:
            if (value != EGL_TRUE && value != EGL_FALSE)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "EGL_CONTEXT_CLIENT_ARRAYS_ENABLED_ANGLE must "
                              "be EGL_TRUE or EGL_FALSE.");
                return false;
            }
            break;

        case EGL_CONTEXT_PROGRAM_BINARY_CACHE_ENABLED_ANGLE:
            if (value != EGL_TRUE && value != EGL_FALSE)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "EGL_CONTEXT_PROGRAM_BINARY_CACHE_ENABLED_ANGLE must "
                              "be EGL_TRUE or EGL_FALSE.");
                return false;
            }
            break;

        case EGL_ROBUST_RESOURCE_INITIALIZATION_ANGLE:
            if (value != EGL_TRUE && value != EGL_FALSE)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "EGL_ROBUST_RESOURCE_INITIALIZATION_ANGLE must be "
                              "either EGL_TRUE or EGL_FALSE.");
                return false;
            }
            break;

        case EGL_EXTENSIONS_ENABLED_ANGLE:
            if (value != EGL_TRUE && value != EGL_FALSE)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "EGL_EXTENSIONS_ENABLED_ANGLE must be "
                              "either EGL_TRUE or EGL_FALSE.");
                return false;
            }
            break;

        case EGL_POWER_PREFERENCE_ANGLE:
            if (value != EGL_LOW_POWER_ANGLE && value != EGL_HIGH_POWER_ANGLE)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "EGL_POWER_PREFERENCE_ANGLE must be "
                              "either EGL_LOW_POWER_ANGLE or EGL_HIGH_POWER_ANGLE.");
                return false;
            }
            break;

        case EGL_CONTEXT_OPENGL_BACKWARDS_COMPATIBLE_ANGLE:
            if (value != EGL_TRUE && value != EGL_FALSE)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "EGL_CONTEXT_OPENGL_BACKWARDS_COMPATIBLE_ANGLE must be "
                              "either EGL_TRUE or EGL_FALSE.");
                return false;
            }
            break;

        case EGL_CONTEXT_PRIORITY_LEVEL_IMG:
            switch (value)
            {
                case EGL_CONTEXT_PRIORITY_LOW_IMG:
                case EGL_CONTEXT_PRIORITY_MEDIUM_IMG:
                case EGL_CONTEXT_PRIORITY_HIGH_IMG:
                    break;
                default:
                    val->setError(EGL_BAD_ATTRIBUTE,
                                  "Attribute EGL_CONTEXT_PRIORITY_LEVEL_IMG "
                                  "must be one of: EGL_CONTEXT_PRIORITY_LOW_IMG, "
                                  "EGL_CONTEXT_PRIORITY_MEDIUM_IMG, or "
                                  "EGL_CONTEXT_PRIORITY_HIGH_IMG.");
                    return false;
            }
            break;

        case EGL_GENERATE_RESET_ON_VIDEO_MEMORY_PURGE_NV:
            if (value != EGL_TRUE && value != EGL_FALSE)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "EGL_GENERATE_RESET_ON_VIDEO_MEMORY_PURGE_NV must "
                              "be either EGL_TRUE or EGL_FALSE.");
                return false;
            }
            break;

        case EGL_EXTERNAL_CONTEXT_ANGLE:
            if (value != EGL_TRUE && value != EGL_FALSE)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "EGL_EXTERNAL_CONTEXT_ANGLE must "
                              "be either EGL_TRUE or EGL_FALSE.");
                return false;
            }
            if (shareContext && (value == EGL_TRUE))
            {
                val->setError(
                    EGL_BAD_ATTRIBUTE,
                    "EGL_EXTERNAL_CONTEXT_ANGLE doesn't allow creating with sharedContext.");
                return false;
            }
            break;

        case EGL_PROTECTED_CONTENT_EXT:
            if (value != EGL_TRUE && value != EGL_FALSE)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "EGL_PROTECTED_CONTENT_EXT must "
                              "be either EGL_TRUE or EGL_FALSE.");
                return false;
            }
            break;

        case EGL_CONTEXT_METAL_OWNERSHIP_IDENTITY_ANGLE:
            if (value == 0)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "EGL_CONTEXT_METAL_OWNERSHIP_IDENTITY_ANGLE must"
                              "be non-zero.");
                return false;
            }
            break;

        default:
            UNREACHABLE();
            return false;
    }

    return true;
}

bool ValidateCreatePbufferSurfaceAttribute(const ValidationContext *val,
                                           const Display *display,
                                           EGLAttrib attribute)
{
    const DisplayExtensions &displayExtensions = display->getExtensions();

    switch (attribute)
    {
        case EGL_WIDTH:
        case EGL_HEIGHT:
        case EGL_LARGEST_PBUFFER:
        case EGL_TEXTURE_FORMAT:
        case EGL_TEXTURE_TARGET:
        case EGL_MIPMAP_TEXTURE:
        case EGL_VG_COLORSPACE:
        case EGL_GL_COLORSPACE:
        case EGL_VG_ALPHA_FORMAT:
            break;

        case EGL_ROBUST_RESOURCE_INITIALIZATION_ANGLE:
            if (!displayExtensions.robustResourceInitializationANGLE)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "Attribute EGL_ROBUST_RESOURCE_INITIALIZATION_ANGLE "
                              "requires EGL_ANGLE_robust_resource_initialization.");
                return false;
            }
            break;

        case EGL_PROTECTED_CONTENT_EXT:
            if (!displayExtensions.protectedContentEXT)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "Attribute EGL_PROTECTED_CONTEXT_EXT requires "
                              "extension EGL_EXT_protected_content.");
                return false;
            }
            break;

        default:
            val->setError(EGL_BAD_ATTRIBUTE);
            return false;
    }

    return true;
}

bool ValidateCreatePbufferSurfaceAttributeValue(const ValidationContext *val,
                                                const Display *display,
                                                EGLAttrib attribute,
                                                EGLAttrib value)
{
    const DisplayExtensions &displayExtensions = display->getExtensions();

    switch (attribute)
    {
        case EGL_WIDTH:
        case EGL_HEIGHT:
            if (value < 0)
            {
                val->setError(EGL_BAD_PARAMETER);
                return false;
            }
            break;

        case EGL_LARGEST_PBUFFER:
        case EGL_MIPMAP_TEXTURE:
            switch (value)
            {
                case EGL_TRUE:
                case EGL_FALSE:
                    break;
                default:
                    val->setError(EGL_BAD_ATTRIBUTE);
                    return false;
            }
            break;

        case EGL_TEXTURE_FORMAT:
            switch (value)
            {
                case EGL_NO_TEXTURE:
                case EGL_TEXTURE_RGB:
                case EGL_TEXTURE_RGBA:
                    break;
                default:
                    val->setError(EGL_BAD_ATTRIBUTE);
                    return false;
            }
            break;

        case EGL_TEXTURE_TARGET:
            switch (value)
            {
                case EGL_NO_TEXTURE:
                case EGL_TEXTURE_2D:
                    break;
                default:
                    val->setError(EGL_BAD_ATTRIBUTE);
                    return false;
            }
            break;

        case EGL_VG_COLORSPACE:
            break;

        case EGL_GL_COLORSPACE:
            ANGLE_VALIDATION_TRY(ValidateColorspaceAttribute(val, displayExtensions, value));
            break;

        case EGL_VG_ALPHA_FORMAT:
            break;

        case EGL_ROBUST_RESOURCE_INITIALIZATION_ANGLE:
            ASSERT(displayExtensions.robustResourceInitializationANGLE);
            if (value != EGL_TRUE && value != EGL_FALSE)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "EGL_ROBUST_RESOURCE_INITIALIZATION_ANGLE must be "
                              "either EGL_TRUE or EGL_FALSE.");
                return false;
            }
            break;

        case EGL_PROTECTED_CONTENT_EXT:
            ASSERT(displayExtensions.protectedContentEXT);
            if (value != EGL_TRUE && value != EGL_FALSE)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "EGL_PROTECTED_CONTENT_EXT must "
                              "be either EGL_TRUE or EGL_FALSE.");
                return false;
            }
            break;

        default:
            UNREACHABLE();
            return false;
    }

    return true;
}
}  // anonymous namespace

void ValidationContext::setError(EGLint error) const
{
    eglThread->setError(error, entryPoint, labeledObject, nullptr);
}

void ValidationContext::setError(EGLint error, const char *message...) const
{
    ASSERT(message);

    constexpr uint32_t kBufferSize = 1000;
    char buffer[kBufferSize];

    va_list args;
    va_start(args, message);
    vsnprintf(buffer, kBufferSize, message, args);

    eglThread->setError(error, entryPoint, labeledObject, buffer);
}

bool ValidateDisplay(const ValidationContext *val, const Display *display)
{
    ANGLE_VALIDATION_TRY(ValidateDisplayPointer(val, display));

    if (!display->isInitialized())
    {
        if (val)
        {
            val->setError(EGL_NOT_INITIALIZED, "display is not initialized.");
        }
        return false;
    }

    if (display->isDeviceLost())
    {
        if (val)
        {
            val->setError(EGL_CONTEXT_LOST, "display had a context loss");
        }
        return false;
    }

    return true;
}

bool ValidateSurface(const ValidationContext *val, const Display *display, SurfaceID surfaceID)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));

    if (!display->isValidSurface(surfaceID))
    {
        if (val)
        {
            val->setError(EGL_BAD_SURFACE);
        }
        return false;
    }

    return true;
}

bool ValidateConfig(const ValidationContext *val, const Display *display, const Config *config)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));

    if (!display->isValidConfig(config))
    {
        if (val)
        {
            val->setError(EGL_BAD_CONFIG);
        }
        return false;
    }

    return true;
}

bool ValidateThreadContext(const ValidationContext *val,
                           const Display *display,
                           EGLenum noContextError)
{
    ASSERT(val);
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));

    if (!val->eglThread->getContext())
    {
        val->setError(noContextError, "No context is current.");
        return false;
    }

    return true;
}

bool ValidateContext(const ValidationContext *val, const Display *display, gl::ContextID contextID)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));

    if (!display->isValidContext(contextID))
    {
        if (val)
        {
            val->setError(EGL_BAD_CONTEXT);
        }
        return false;
    }

    return true;
}

bool ValidateImage(const ValidationContext *val, const Display *display, ImageID imageID)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));

    if (!display->isValidImage(imageID))
    {
        if (val)
        {
            val->setError(EGL_BAD_PARAMETER, "image is not valid.");
        }
        return false;
    }

    return true;
}

bool ValidateDevice(const ValidationContext *val, const Device *device)
{
    if (device == EGL_NO_DEVICE_EXT)
    {
        if (val)
        {
            val->setError(EGL_BAD_ACCESS, "device is EGL_NO_DEVICE.");
        }
        return false;
    }

    if (!Device::IsValidDevice(device))
    {
        if (val)
        {
            val->setError(EGL_BAD_ACCESS, "device is not valid.");
        }
        return false;
    }

    return true;
}

bool ValidateSync(const ValidationContext *val, const Display *display, SyncID sync)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));

    if (!display->isValidSync(sync))
    {
        if (val)
        {
            val->setError(EGL_BAD_PARAMETER, "sync object is not valid.");
        }
        return false;
    }

    return true;
}

bool ValidateCreateWindowSurfaceAttributes(const ValidationContext *val,
                                           const Display *display,
                                           const Config *config,
                                           const AttributeMap &attributes)
{
    const DisplayExtensions &displayExtensions = display->getExtensions();
    for (const auto &attributeIter : attributes)
    {
        EGLAttrib attribute = attributeIter.first;
        EGLAttrib value     = attributeIter.second;

        switch (attribute)
        {
            case EGL_RENDER_BUFFER:
                switch (value)
                {
                    case EGL_BACK_BUFFER:
                        break;
                    case EGL_SINGLE_BUFFER:
                        break;
                    default:
                        val->setError(EGL_BAD_ATTRIBUTE);
                        return false;
                }
                break;

            case EGL_POST_SUB_BUFFER_SUPPORTED_NV:
                if (!displayExtensions.postSubBuffer)
                {
                    val->setError(EGL_BAD_ATTRIBUTE);
                    return false;
                }
                break;

            case EGL_WIDTH:
            case EGL_HEIGHT:
                if (!displayExtensions.windowFixedSize)
                {
                    val->setError(EGL_BAD_ATTRIBUTE);
                    return false;
                }
                if (value < 0)
                {
                    val->setError(EGL_BAD_PARAMETER);
                    return false;
                }
                break;

            case EGL_FIXED_SIZE_ANGLE:
                if (!displayExtensions.windowFixedSize)
                {
                    val->setError(EGL_BAD_ATTRIBUTE);
                    return false;
                }
                break;

            case EGL_SURFACE_ORIENTATION_ANGLE:
                if (!displayExtensions.surfaceOrientation)
                {
                    val->setError(EGL_BAD_ATTRIBUTE,
                                  "EGL_ANGLE_surface_orientation is not enabled.");
                    return false;
                }
                break;

            case EGL_VG_COLORSPACE:
                if (value != EGL_VG_COLORSPACE_sRGB)
                {
                    val->setError(EGL_BAD_MATCH);
                    return false;
                }
                break;

            case EGL_GL_COLORSPACE:
                ANGLE_VALIDATION_TRY(ValidateColorspaceAttribute(val, displayExtensions, value));
                break;

            case EGL_VG_ALPHA_FORMAT:
                val->setError(EGL_BAD_MATCH);
                return false;

            case EGL_DIRECT_COMPOSITION_ANGLE:
                if (!displayExtensions.directComposition)
                {
                    val->setError(EGL_BAD_ATTRIBUTE);
                    return false;
                }
                break;

            case EGL_ROBUST_RESOURCE_INITIALIZATION_ANGLE:
                if (!display->getExtensions().robustResourceInitializationANGLE)
                {
                    val->setError(EGL_BAD_ATTRIBUTE,
                                  "Attribute EGL_ROBUST_RESOURCE_INITIALIZATION_ANGLE "
                                  "requires EGL_ANGLE_robust_resource_initialization.");
                    return false;
                }
                if (value != EGL_TRUE && value != EGL_FALSE)
                {
                    val->setError(EGL_BAD_ATTRIBUTE,
                                  "EGL_ROBUST_RESOURCE_INITIALIZATION_ANGLE must be "
                                  "either EGL_TRUE or EGL_FALSE.");
                    return false;
                }
                break;

            case EGL_GGP_STREAM_DESCRIPTOR_ANGLE:
                if (!display->getExtensions().ggpStreamDescriptor)
                {
                    val->setError(EGL_BAD_ATTRIBUTE,
                                  "EGL_GGP_STREAM_DESCRIPTOR_ANGLE requires "
                                  "EGL_ANGLE_ggp_stream_descriptor.");
                    return false;
                }
                break;

            case EGL_PROTECTED_CONTENT_EXT:
                if (!displayExtensions.protectedContentEXT)
                {
                    val->setError(EGL_BAD_ATTRIBUTE,
                                  "Attribute EGL_PROTECTED_CONTEXT_EXT requires "
                                  "extension EGL_EXT_protected_content.");
                    return false;
                }
                if (value != EGL_TRUE && value != EGL_FALSE)
                {
                    val->setError(EGL_BAD_ATTRIBUTE,
                                  "EGL_PROTECTED_CONTENT_EXT must "
                                  "be either EGL_TRUE or EGL_FALSE.");
                    return false;
                }
                break;

            case EGL_SWAP_INTERVAL_ANGLE:
                if (!displayExtensions.createSurfaceSwapIntervalANGLE)
                {
                    val->setError(EGL_BAD_ATTRIBUTE,
                                  "Attribute EGL_SWAP_INTERVAL_ANGLE requires "
                                  "extension EGL_ANGLE_create_surface_swap_interval.");
                    return false;
                }
                if (value < config->minSwapInterval || value > config->maxSwapInterval)
                {
                    val->setError(EGL_BAD_ATTRIBUTE,
                                  "EGL_SWAP_INTERVAL_ANGLE must "
                                  "be within the EGLConfig min and max swap intervals.");
                    return false;
                }
                break;

            case EGL_SURFACE_COMPRESSION_EXT:
                if (!displayExtensions.surfaceCompressionEXT)
                {
                    val->setError(EGL_BAD_ATTRIBUTE,
                                  "Attribute EGL_SURFACE_COMPRESSION_EXT requires "
                                  "extension EGL_EXT_surface_compression.");
                    return false;
                }

                switch (value)
                {
                    case EGL_SURFACE_COMPRESSION_FIXED_RATE_NONE_EXT:
                    case EGL_SURFACE_COMPRESSION_FIXED_RATE_DEFAULT_EXT:
                    case EGL_SURFACE_COMPRESSION_FIXED_RATE_1BPC_EXT:
                    case EGL_SURFACE_COMPRESSION_FIXED_RATE_2BPC_EXT:
                    case EGL_SURFACE_COMPRESSION_FIXED_RATE_3BPC_EXT:
                    case EGL_SURFACE_COMPRESSION_FIXED_RATE_4BPC_EXT:
                    case EGL_SURFACE_COMPRESSION_FIXED_RATE_5BPC_EXT:
                    case EGL_SURFACE_COMPRESSION_FIXED_RATE_6BPC_EXT:
                    case EGL_SURFACE_COMPRESSION_FIXED_RATE_7BPC_EXT:
                    case EGL_SURFACE_COMPRESSION_FIXED_RATE_8BPC_EXT:
                    case EGL_SURFACE_COMPRESSION_FIXED_RATE_9BPC_EXT:
                    case EGL_SURFACE_COMPRESSION_FIXED_RATE_10BPC_EXT:
                    case EGL_SURFACE_COMPRESSION_FIXED_RATE_11BPC_EXT:
                    case EGL_SURFACE_COMPRESSION_FIXED_RATE_12BPC_EXT:
                        break;
                    default:
                        val->setError(EGL_BAD_ATTRIBUTE);
                        return false;
                }

                break;

            default:
                val->setError(EGL_BAD_ATTRIBUTE);
                return false;
        }
    }

    return true;
}

const Thread *GetThreadIfValid(const Thread *thread)
{
    // Threads should always be valid
    return thread;
}

const Display *GetDisplayIfValid(const Display *display)
{
    return ValidateDisplay(nullptr, display) ? display : nullptr;
}

const Surface *GetSurfaceIfValid(const Display *display, SurfaceID surfaceID)
{
    // display->getSurface() - validates surfaceID
    return ValidateDisplay(nullptr, display) ? display->getSurface(surfaceID) : nullptr;
}

const Image *GetImageIfValid(const Display *display, ImageID imageID)
{
    // display->getImage() - validates imageID
    return ValidateDisplay(nullptr, display) ? display->getImage(imageID) : nullptr;
}

const Stream *GetStreamIfValid(const Display *display, const Stream *stream)
{
    return ValidateStream(nullptr, display, stream) ? stream : nullptr;
}

const gl::Context *GetContextIfValid(const Display *display, gl::ContextID contextID)
{
    // display->getContext() - validates contextID
    return ValidateDisplay(nullptr, display) ? display->getContext(contextID) : nullptr;
}

gl::Context *GetContextIfValid(Display *display, gl::ContextID contextID)
{
    return ValidateDisplay(nullptr, display) ? display->getContext(contextID) : nullptr;
}

const Device *GetDeviceIfValid(const Device *device)
{
    return ValidateDevice(nullptr, device) ? device : nullptr;
}

const Sync *GetSyncIfValid(const Display *display, SyncID syncID)
{
    // display->getSync() - validates syncID
    return ValidateDisplay(nullptr, display) ? display->getSync(syncID) : nullptr;
}

const LabeledObject *GetLabeledObjectIfValid(Thread *thread,
                                             const Display *display,
                                             ObjectType objectType,
                                             EGLObjectKHR object)
{
    if (objectType == ObjectType::Thread)
    {
        return thread;
    }

    const LabeledObject *labeledObject = nullptr;
    if (ValidateLabeledObject(nullptr, display, objectType, object, &labeledObject))
    {
        return labeledObject;
    }

    return nullptr;
}

LabeledObject *GetLabeledObjectIfValid(Thread *thread,
                                       Display *display,
                                       ObjectType objectType,
                                       EGLObjectKHR object)
{
    if (objectType == ObjectType::Thread)
    {
        return thread;
    }

    LabeledObject *labeledObject = nullptr;
    if (ValidateLabeledObject(nullptr, display, objectType, object, &labeledObject))
    {
        return labeledObject;
    }

    return nullptr;
}

bool ValidateInitialize(const ValidationContext *val,
                        const Display *display,
                        const EGLint *major,
                        const EGLint *minor)
{
    return ValidateDisplayPointer(val, display);
}

bool ValidateTerminate(const ValidationContext *val, const Display *display)
{
    return ValidateDisplayPointer(val, display);
}

bool ValidateCreateContext(const ValidationContext *val,
                           const Display *display,
                           const Config *configuration,
                           gl::ContextID shareContextID,
                           const AttributeMap &attributes)
{
    if (configuration)
    {
        ANGLE_VALIDATION_TRY(ValidateConfig(val, display, configuration));
    }
    else
    {
        ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));
        const DisplayExtensions &displayExtensions = display->getExtensions();
        if (!displayExtensions.noConfigContext)
        {
            val->setError(EGL_BAD_CONFIG);
            return false;
        }
    }

    if (shareContextID.value != 0)
    {
        // Shared context is invalid or is owned by another display
        if (!display->isValidContext(shareContextID))
        {
            val->setError(EGL_BAD_MATCH);
            return false;
        }
    }

    const gl::Context *shareContext = display->getContext(shareContextID);

    ANGLE_VALIDATION_TRY(attributes.validate(val, display, ValidateCreateContextAttribute));

    for (const auto &attributePair : attributes)
    {
        EGLAttrib attribute = attributePair.first;
        EGLAttrib value     = attributePair.second;
        ANGLE_VALIDATION_TRY(
            ValidateCreateContextAttributeValue(val, display, shareContext, attribute, value));
    }

    // Get the requested client version (default is 1) and check it is 2 or 3.
    EGLAttrib clientMajorVersion = attributes.get(EGL_CONTEXT_CLIENT_VERSION, 1);
    EGLAttrib clientMinorVersion = attributes.get(EGL_CONTEXT_MINOR_VERSION, 0);
    EGLenum api                  = val->eglThread->getAPI();

    switch (api)
    {
        case EGL_OPENGL_ES_API:
            switch (clientMajorVersion)
            {
                case 1:
                    if (clientMinorVersion != 0 && clientMinorVersion != 1)
                    {
                        val->setError(EGL_BAD_ATTRIBUTE);
                        return false;
                    }
                    if (configuration == EGL_NO_CONFIG_KHR)
                    {
                        val->setError(EGL_BAD_MATCH);
                        return false;
                    }
                    if ((configuration != EGL_NO_CONFIG_KHR) &&
                        !(configuration->renderableType & EGL_OPENGL_ES_BIT))
                    {
                        val->setError(EGL_BAD_MATCH);
                        return false;
                    }
                    break;

                case 2:
                    if (clientMinorVersion != 0)
                    {
                        val->setError(EGL_BAD_ATTRIBUTE);
                        return false;
                    }
                    if ((configuration != EGL_NO_CONFIG_KHR) &&
                        !(configuration->renderableType & EGL_OPENGL_ES2_BIT))
                    {
                        val->setError(EGL_BAD_MATCH);
                        return false;
                    }
                    break;
                case 3:
                    if (clientMinorVersion < 0 || clientMinorVersion > 2)
                    {
                        val->setError(EGL_BAD_ATTRIBUTE);
                        return false;
                    }
                    if ((configuration != EGL_NO_CONFIG_KHR) &&
                        !(configuration->renderableType & EGL_OPENGL_ES3_BIT))
                    {
                        val->setError(EGL_BAD_MATCH);
                        return false;
                    }
                    if (display->getMaxSupportedESVersion() <
                        gl::Version(static_cast<GLuint>(clientMajorVersion),
                                    static_cast<GLuint>(clientMinorVersion)))
                    {
                        gl::Version max = display->getMaxSupportedESVersion();
                        val->setError(EGL_BAD_ATTRIBUTE,
                                      "Requested GLES version (%" PRIxPTR ".%" PRIxPTR
                                      ") is greater than "
                                      "max supported (%d, %d).",
                                      clientMajorVersion, clientMinorVersion, max.major, max.minor);
                        return false;
                    }
                    if ((attributes.get(EGL_CONTEXT_WEBGL_COMPATIBILITY_ANGLE, EGL_FALSE) ==
                         EGL_TRUE) &&
                        (clientMinorVersion > 1))
                    {
                        val->setError(EGL_BAD_ATTRIBUTE,
                                      "Requested GLES version (%" PRIxPTR ".%" PRIxPTR
                                      ") is greater than "
                                      "max supported 3.1 for WebGL.",
                                      clientMajorVersion, clientMinorVersion);
                        return false;
                    }
                    break;
                default:
                    val->setError(EGL_BAD_ATTRIBUTE);
                    return false;
            }
            break;

        case EGL_OPENGL_API:
            // Desktop GL is not supported by ANGLE.
            val->setError(EGL_BAD_CONFIG);
            return false;

        default:
            val->setError(EGL_BAD_MATCH, "Unsupported API.");
            return false;
    }

    return true;
}

bool ValidateCreateWindowSurface(const ValidationContext *val,
                                 const Display *display,
                                 const Config *config,
                                 EGLNativeWindowType window,
                                 const AttributeMap &attributes)
{
    ANGLE_VALIDATION_TRY(ValidateConfig(val, display, config));

    if (!display->isValidNativeWindow(window))
    {
        val->setError(EGL_BAD_NATIVE_WINDOW);
        return false;
    }

    attributes.initializeWithoutValidation();

    ANGLE_VALIDATION_TRY(ValidateCreateWindowSurfaceAttributes(val, display, config, attributes));

    if (Display::hasExistingWindowSurface(window))
    {
        val->setError(EGL_BAD_ALLOC);
        return false;
    }

    return true;
}

bool ValidateCreatePbufferSurface(const ValidationContext *val,
                                  const Display *display,
                                  const Config *config,
                                  const AttributeMap &attributes)
{
    ANGLE_VALIDATION_TRY(ValidateConfig(val, display, config));
    ANGLE_VALIDATION_TRY(attributes.validate(val, display, ValidateCreatePbufferSurfaceAttribute));

    for (const auto &attributeIter : attributes)
    {
        EGLAttrib attribute = attributeIter.first;
        EGLAttrib value     = attributeIter.second;

        ANGLE_VALIDATION_TRY(
            ValidateCreatePbufferSurfaceAttributeValue(val, display, attribute, value));
    }

    if ((config->surfaceType & EGL_PBUFFER_BIT) == 0)
    {
        val->setError(EGL_BAD_MATCH);
        return false;
    }

    const Caps &caps = display->getCaps();

    EGLAttrib textureFormat = attributes.get(EGL_TEXTURE_FORMAT, EGL_NO_TEXTURE);
    EGLAttrib textureTarget = attributes.get(EGL_TEXTURE_TARGET, EGL_NO_TEXTURE);

    if ((textureFormat != EGL_NO_TEXTURE && textureTarget == EGL_NO_TEXTURE) ||
        (textureFormat == EGL_NO_TEXTURE && textureTarget != EGL_NO_TEXTURE))
    {
        val->setError(EGL_BAD_MATCH);
        return false;
    }

    if ((textureFormat == EGL_TEXTURE_RGB && config->bindToTextureRGB != EGL_TRUE) ||
        (textureFormat == EGL_TEXTURE_RGBA && config->bindToTextureRGBA != EGL_TRUE))
    {
        val->setError(EGL_BAD_ATTRIBUTE);
        return false;
    }

    EGLint width  = static_cast<EGLint>(attributes.get(EGL_WIDTH, 0));
    EGLint height = static_cast<EGLint>(attributes.get(EGL_HEIGHT, 0));

    EGLBoolean isLargestPbuffer =
        static_cast<EGLBoolean>(attributes.get(EGL_LARGEST_PBUFFER, EGL_FALSE));

    if (!isLargestPbuffer && (width > config->maxPBufferWidth || height > config->maxPBufferHeight))
    {
        val->setError(EGL_BAD_ATTRIBUTE);
        return false;
    }

    if (textureFormat != EGL_NO_TEXTURE && !caps.textureNPOT &&
        (!gl::isPow2(width) || !gl::isPow2(height)))
    {
        val->setError(EGL_BAD_MATCH);
        return false;
    }

    return true;
}

bool ValidateCreatePbufferFromClientBuffer(const ValidationContext *val,
                                           const Display *display,
                                           EGLenum buftype,
                                           EGLClientBuffer buffer,
                                           const Config *config,
                                           const AttributeMap &attributes)
{
    ANGLE_VALIDATION_TRY(ValidateConfig(val, display, config));

    const DisplayExtensions &displayExtensions = display->getExtensions();

    attributes.initializeWithoutValidation();

    switch (buftype)
    {
        case EGL_D3D_TEXTURE_2D_SHARE_HANDLE_ANGLE:
            if (!displayExtensions.d3dShareHandleClientBuffer)
            {
                val->setError(EGL_BAD_PARAMETER);
                return false;
            }
            if (buffer == nullptr)
            {
                val->setError(EGL_BAD_PARAMETER);
                return false;
            }
            break;

        case EGL_D3D_TEXTURE_ANGLE:
            if (!displayExtensions.d3dTextureClientBuffer)
            {
                val->setError(EGL_BAD_PARAMETER);
                return false;
            }
            if (buffer == nullptr)
            {
                val->setError(EGL_BAD_PARAMETER);
                return false;
            }
            break;

        case EGL_IOSURFACE_ANGLE:
            if (!displayExtensions.iosurfaceClientBuffer)
            {
                val->setError(EGL_BAD_PARAMETER,
                              "<buftype> EGL_IOSURFACE_ANGLE requires the "
                              "EGL_ANGLE_iosurface_client_buffer extension.");
                return false;
            }
            if (buffer == nullptr)
            {
                val->setError(EGL_BAD_PARAMETER, "<buffer> must be non null");
                return false;
            }
            break;
        case EGL_EXTERNAL_SURFACE_ANGLE:
            if (!display->getExtensions().externalContextAndSurface)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "Attribute "
                              "EGL_EXTERNAL_SURFACE_ANGLE requires "
                              "EGL_ANGLE_external_context_and_surface.");
                return false;
            }
            if (buffer != nullptr)
            {
                val->setError(EGL_BAD_PARAMETER, "<buffer> must be null");
                return false;
            }
            break;

        default:
            val->setError(EGL_BAD_PARAMETER);
            return false;
    }

    for (AttributeMap::const_iterator attributeIter = attributes.begin();
         attributeIter != attributes.end(); attributeIter++)
    {
        EGLAttrib attribute = attributeIter->first;
        EGLAttrib value     = attributeIter->second;

        switch (attribute)
        {
            case EGL_WIDTH:
            case EGL_HEIGHT:
                if (buftype != EGL_D3D_TEXTURE_2D_SHARE_HANDLE_ANGLE &&
                    buftype != EGL_D3D_TEXTURE_ANGLE && buftype != EGL_IOSURFACE_ANGLE &&
                    buftype != EGL_EXTERNAL_SURFACE_ANGLE)
                {
                    val->setError(EGL_BAD_PARAMETER,
                                  "Width and Height are not supported for this <buftype>");
                    return false;
                }
                if (value < 0)
                {
                    val->setError(EGL_BAD_PARAMETER, "Width and Height must be positive");
                    return false;
                }
                break;

            case EGL_TEXTURE_FORMAT:
                switch (value)
                {
                    case EGL_NO_TEXTURE:
                    case EGL_TEXTURE_RGB:
                    case EGL_TEXTURE_RGBA:
                        break;
                    default:
                        val->setError(EGL_BAD_ATTRIBUTE, "Invalid value for EGL_TEXTURE_FORMAT");
                        return false;
                }
                break;

            case EGL_TEXTURE_TARGET:
                switch (value)
                {
                    case EGL_NO_TEXTURE:
                    case EGL_TEXTURE_2D:
                        break;
                    case EGL_TEXTURE_RECTANGLE_ANGLE:
                        if (buftype != EGL_IOSURFACE_ANGLE)
                        {
                            val->setError(EGL_BAD_PARAMETER,
                                          "<buftype> doesn't support rectangle texture targets");
                            return false;
                        }
                        break;

                    default:
                        val->setError(EGL_BAD_ATTRIBUTE, "Invalid value for EGL_TEXTURE_TARGET");
                        return false;
                }
                break;

            case EGL_MIPMAP_TEXTURE:
                break;

            case EGL_IOSURFACE_PLANE_ANGLE:
                if (buftype != EGL_IOSURFACE_ANGLE)
                {
                    val->setError(EGL_BAD_ATTRIBUTE, "<buftype> doesn't support iosurface plane");
                    return false;
                }
                break;

            case EGL_TEXTURE_TYPE_ANGLE:
                if (buftype != EGL_IOSURFACE_ANGLE)
                {
                    val->setError(EGL_BAD_ATTRIBUTE, "<buftype> doesn't support texture type");
                    return false;
                }
                break;

            case EGL_TEXTURE_INTERNAL_FORMAT_ANGLE:
                if (buftype != EGL_IOSURFACE_ANGLE && buftype != EGL_D3D_TEXTURE_ANGLE)
                {
                    val->setError(EGL_BAD_ATTRIBUTE,
                                  "<buftype> doesn't support texture internal format");
                    return false;
                }
                break;

            case EGL_GL_COLORSPACE:
                if (buftype != EGL_D3D_TEXTURE_ANGLE)
                {
                    val->setError(EGL_BAD_ATTRIBUTE,
                                  "<buftype> doesn't support setting GL colorspace");
                    return false;
                }
                break;

            case EGL_IOSURFACE_USAGE_HINT_ANGLE:
                if (value & ~(EGL_IOSURFACE_READ_HINT_ANGLE | EGL_IOSURFACE_WRITE_HINT_ANGLE))
                {
                    val->setError(EGL_BAD_ATTRIBUTE,
                                  "IOSurface usage hint must only contain READ or WRITE");
                    return false;
                }
                break;

            case EGL_TEXTURE_OFFSET_X_ANGLE:
            case EGL_TEXTURE_OFFSET_Y_ANGLE:
                if (buftype != EGL_D3D_TEXTURE_ANGLE)
                {
                    val->setError(EGL_BAD_ATTRIBUTE,
                                  "<buftype> doesn't support setting texture offset");
                    return false;
                }
                break;

            case EGL_PROTECTED_CONTENT_EXT:
                if (!displayExtensions.protectedContentEXT)
                {
                    val->setError(EGL_BAD_ATTRIBUTE,
                                  "Attribute EGL_PROTECTED_CONTEXT_EXT requires "
                                  "extension EGL_EXT_protected_content.");
                    return false;
                }
                if (value != EGL_TRUE && value != EGL_FALSE)
                {
                    val->setError(EGL_BAD_ATTRIBUTE,
                                  "EGL_PROTECTED_CONTENT_EXT must "
                                  "be either EGL_TRUE or EGL_FALSE.");
                    return false;
                }
                break;

            case EGL_D3D11_TEXTURE_PLANE_ANGLE:
                if (!displayExtensions.imageD3D11Texture)
                {
                    val->setError(EGL_BAD_ATTRIBUTE,
                                  "EGL_D3D11_TEXTURE_PLANE_ANGLE cannot be used without "
                                  "EGL_ANGLE_image_d3d11_texture support.");
                    return false;
                }
                break;

            default:
                val->setError(EGL_BAD_ATTRIBUTE);
                return false;
        }
    }

    EGLAttrib colorspace = attributes.get(EGL_GL_COLORSPACE, EGL_GL_COLORSPACE_LINEAR);
    if (colorspace != EGL_GL_COLORSPACE_LINEAR && colorspace != EGL_GL_COLORSPACE_SRGB)
    {
        val->setError(EGL_BAD_ATTRIBUTE, "invalid GL colorspace");
        return false;
    }

    if (!(config->surfaceType & EGL_PBUFFER_BIT))
    {
        val->setError(EGL_BAD_MATCH);
        return false;
    }

    EGLAttrib textureFormat = attributes.get(EGL_TEXTURE_FORMAT, EGL_NO_TEXTURE);
    EGLAttrib textureTarget = attributes.get(EGL_TEXTURE_TARGET, EGL_NO_TEXTURE);
    if ((textureFormat != EGL_NO_TEXTURE && textureTarget == EGL_NO_TEXTURE) ||
        (textureFormat == EGL_NO_TEXTURE && textureTarget != EGL_NO_TEXTURE))
    {
        val->setError(EGL_BAD_MATCH);
        return false;
    }
    if ((textureFormat == EGL_TEXTURE_RGB && config->bindToTextureRGB != EGL_TRUE) ||
        (textureFormat == EGL_TEXTURE_RGBA && config->bindToTextureRGBA != EGL_TRUE))
    {
        // TODO(cwallez@chromium.org): For IOSurface pbuffers we require that EGL_TEXTURE_RGBA is
        // set so that eglBindTexImage works. Normally this is only allowed if the config exposes
        // the bindToTextureRGB/RGBA flag. This issue is that enabling this flags means that
        // eglBindTexImage should also work for regular pbuffers which isn't implemented on macOS.
        // Instead of adding the flag we special case the check here to be ignored for IOSurfaces.
        // The TODO is to find a proper solution for this, maybe by implementing eglBindTexImage on
        // OSX?
        if (buftype != EGL_IOSURFACE_ANGLE)
        {
            val->setError(EGL_BAD_ATTRIBUTE);
            return false;
        }
    }

    if (buftype == EGL_D3D_TEXTURE_2D_SHARE_HANDLE_ANGLE)
    {
        EGLint width  = static_cast<EGLint>(attributes.get(EGL_WIDTH, 0));
        EGLint height = static_cast<EGLint>(attributes.get(EGL_HEIGHT, 0));

        if (width == 0 || height == 0)
        {
            val->setError(EGL_BAD_ATTRIBUTE);
            return false;
        }

        const Caps &caps = display->getCaps();
        if (textureFormat != EGL_NO_TEXTURE && !caps.textureNPOT &&
            (!gl::isPow2(width) || !gl::isPow2(height)))
        {
            val->setError(EGL_BAD_MATCH);
            return false;
        }
    }

    if (buftype == EGL_IOSURFACE_ANGLE)
    {
        if (static_cast<EGLenum>(textureTarget) != config->bindToTextureTarget)
        {
            val->setError(EGL_BAD_ATTRIBUTE,
                          "EGL_IOSURFACE requires the texture target to match the config");
            return false;
        }
        if (textureFormat != EGL_TEXTURE_RGBA)
        {
            val->setError(EGL_BAD_ATTRIBUTE, "EGL_IOSURFACE requires the EGL_TEXTURE_RGBA format");
            return false;
        }

        if (!attributes.contains(EGL_WIDTH) || !attributes.contains(EGL_HEIGHT) ||
            !attributes.contains(EGL_TEXTURE_FORMAT) ||
            !attributes.contains(EGL_TEXTURE_TYPE_ANGLE) ||
            !attributes.contains(EGL_TEXTURE_INTERNAL_FORMAT_ANGLE) ||
            !attributes.contains(EGL_IOSURFACE_PLANE_ANGLE))
        {
            val->setError(EGL_BAD_PARAMETER, "Missing required attribute for EGL_IOSURFACE");
            return false;
        }
    }

    ANGLE_EGL_TRY_RETURN(val->eglThread,
                         display->validateClientBuffer(config, buftype, buffer, attributes),
                         val->entryPoint, val->labeledObject, false);

    return true;
}

bool ValidateCreatePixmapSurface(const ValidationContext *val,
                                 const Display *display,
                                 const Config *config,
                                 EGLNativePixmapType pixmap,
                                 const AttributeMap &attributes)
{
    ANGLE_VALIDATION_TRY(ValidateConfig(val, display, config));

    const DisplayExtensions &displayExtensions = display->getExtensions();

    attributes.initializeWithoutValidation();

    for (const auto &attributePair : attributes)
    {
        EGLAttrib attribute = attributePair.first;
        EGLAttrib value     = attributePair.second;

        switch (attribute)
        {
            case EGL_GL_COLORSPACE:
                ANGLE_VALIDATION_TRY(ValidateColorspaceAttribute(val, displayExtensions, value));
                break;

            case EGL_VG_COLORSPACE:
                break;
            case EGL_VG_ALPHA_FORMAT:
                break;

            case EGL_TEXTURE_FORMAT:
                if (!displayExtensions.textureFromPixmapNOK)
                {
                    val->setError(EGL_BAD_ATTRIBUTE, "EGL_NOK_texture_from_pixmap is not enabled.");
                    return false;
                }
                switch (value)
                {
                    case EGL_NO_TEXTURE:
                    case EGL_TEXTURE_RGB:
                    case EGL_TEXTURE_RGBA:
                        break;
                    default:
                        val->setError(EGL_BAD_ATTRIBUTE);
                        return false;
                }
                break;

            case EGL_TEXTURE_TARGET:
                if (!displayExtensions.textureFromPixmapNOK)
                {
                    val->setError(EGL_BAD_ATTRIBUTE, "EGL_NOK_texture_from_pixmap is not enabled.");
                    return false;
                }
                switch (value)
                {
                    case EGL_NO_TEXTURE:
                    case EGL_TEXTURE_2D:
                        break;
                    default:
                        val->setError(EGL_BAD_ATTRIBUTE);
                        return false;
                }
                break;

            case EGL_MIPMAP_TEXTURE:
                if (!displayExtensions.textureFromPixmapNOK)
                {
                    val->setError(EGL_BAD_ATTRIBUTE, "EGL_NOK_texture_from_pixmap is not enabled.");
                    return false;
                }
                break;

            case EGL_PROTECTED_CONTENT_EXT:
                if (!displayExtensions.protectedContentEXT)
                {
                    val->setError(EGL_BAD_ATTRIBUTE,
                                  "Attribute EGL_PROTECTED_CONTEXT_EXT requires "
                                  "extension EGL_EXT_protected_content.");
                    return false;
                }
                if (value != EGL_TRUE && value != EGL_FALSE)
                {
                    val->setError(EGL_BAD_ATTRIBUTE,
                                  "EGL_PROTECTED_CONTENT_EXT must "
                                  "be either EGL_TRUE or EGL_FALSE.");
                    return false;
                }
                break;

            default:
                val->setError(EGL_BAD_ATTRIBUTE, "Unknown attribute: 0x%04" PRIxPTR, attribute);
                return false;
        }
    }

    if (!(config->surfaceType & EGL_PIXMAP_BIT))
    {
        val->setError(EGL_BAD_MATCH, "Congfig does not suport pixmaps.");
        return false;
    }

    ANGLE_EGL_TRY_RETURN(val->eglThread, display->valdiatePixmap(config, pixmap, attributes),
                         val->entryPoint, val->labeledObject, false);

    return true;
}

bool ValidateMakeCurrent(const ValidationContext *val,
                         const Display *display,
                         SurfaceID drawSurfaceID,
                         SurfaceID readSurfaceID,
                         gl::ContextID contextID)
{
    bool noDraw    = drawSurfaceID.value == 0;
    bool noRead    = readSurfaceID.value == 0;
    bool noContext = contextID.value == 0;

    if (noContext && (!noDraw || !noRead))
    {
        val->setError(EGL_BAD_MATCH, "If ctx is EGL_NO_CONTEXT, surfaces must be EGL_NO_SURFACE");
        return false;
    }

    // If ctx is EGL_NO_CONTEXT and either draw or read are not EGL_NO_SURFACE, an EGL_BAD_MATCH
    // error is generated. EGL_KHR_surfaceless_context allows both surfaces to be EGL_NO_SURFACE.
    if (!noContext && (noDraw || noRead))
    {
        if (display->getExtensions().surfacelessContext)
        {
            if (noDraw != noRead)
            {
                val->setError(EGL_BAD_MATCH,
                              "If ctx is not EGL_NOT_CONTEXT, draw or read must "
                              "both be EGL_NO_SURFACE, or both not");
                return false;
            }
        }
        else
        {
            val->setError(EGL_BAD_MATCH,
                          "If ctx is not EGL_NO_CONTEXT, surfaces must not be EGL_NO_SURFACE");
            return false;
        }
    }

    // If either of draw or read is a valid surface and the other is EGL_NO_SURFACE, an
    // EGL_BAD_MATCH error is generated.
    if (noRead != noDraw)
    {
        val->setError(EGL_BAD_MATCH,
                      "read and draw must both be valid surfaces, or both be EGL_NO_SURFACE");
        return false;
    }

    if (display == EGL_NO_DISPLAY || !Display::isValidDisplay(display))
    {
        val->setError(EGL_BAD_DISPLAY, "'dpy' not a valid EGLDisplay handle");
        return false;
    }

    // EGL 1.5 spec: dpy can be uninitialized if all other parameters are null
    if (!display->isInitialized() && (!noContext || !noDraw || !noRead))
    {
        val->setError(EGL_NOT_INITIALIZED, "'dpy' not initialized");
        return false;
    }

    if (!noContext)
    {
        ANGLE_VALIDATION_TRY(ValidateContext(val, display, contextID));
    }

    // Allow "un-make" the lost context:
    // If the context is lost, but EGLContext passed to eglMakeCurrent is EGL_NO_CONTEXT, we should
    // not return EGL_CONTEXT_LOST error code.
    if (display->isInitialized() && display->isDeviceLost() && !noContext)
    {
        val->setError(EGL_CONTEXT_LOST);
        return false;
    }

    const Surface *drawSurface = GetSurfaceIfValid(display, drawSurfaceID);
    const Surface *readSurface = GetSurfaceIfValid(display, readSurfaceID);
    const gl::Context *context = GetContextIfValid(display, contextID);

    const gl::Context *previousContext = val->eglThread->getContext();
    if (!noContext && context->isReferenced() && context != previousContext)
    {
        val->setError(EGL_BAD_ACCESS, "Context can only be current on one thread");
        return false;
    }

    if (!noRead)
    {
        ANGLE_VALIDATION_TRY(ValidateSurface(val, display, readSurfaceID));
        ANGLE_VALIDATION_TRY(ValidateCompatibleSurface(val, display, context, readSurface));
        ANGLE_VALIDATION_TRY(ValidateSurfaceBadAccess(val, previousContext, readSurface));
    }

    if (drawSurface != readSurface && !noDraw)
    {
        ANGLE_VALIDATION_TRY(ValidateSurface(val, display, drawSurfaceID));
        ANGLE_VALIDATION_TRY(ValidateCompatibleSurface(val, display, context, drawSurface));
        ANGLE_VALIDATION_TRY(ValidateSurfaceBadAccess(val, previousContext, drawSurface));
    }
    return true;
}

bool ValidateCreateImage(const ValidationContext *val,
                         const Display *display,
                         gl::ContextID contextID,
                         EGLenum target,
                         EGLClientBuffer buffer,
                         const AttributeMap &attributes)
{
    const gl::Context *context = GetContextIfValid(display, contextID);

    ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));

    attributes.initializeWithoutValidation();

    const DisplayExtensions &displayExtensions = display->getExtensions();

    // TODO(geofflang): Complete validation from EGL_KHR_image_base:
    // If the resource specified by <dpy>, <ctx>, <target>, <buffer> and <attrib_list> is itself an
    // EGLImage sibling, the error EGL_BAD_ACCESS is generated.

    for (AttributeMap::const_iterator attributeIter = attributes.begin();
         attributeIter != attributes.end(); attributeIter++)
    {
        EGLAttrib attribute = attributeIter->first;
        EGLAttrib value     = attributeIter->second;

        switch (attribute)
        {
            case EGL_IMAGE_PRESERVED:
                switch (value)
                {
                    case EGL_TRUE:
                    case EGL_FALSE:
                        break;

                    default:
                        val->setError(EGL_BAD_PARAMETER,
                                      "EGL_IMAGE_PRESERVED must be EGL_TRUE or EGL_FALSE.");
                        return false;
                }
                break;

            case EGL_GL_TEXTURE_LEVEL:
                if (!displayExtensions.glTexture2DImage &&
                    !displayExtensions.glTextureCubemapImage && !displayExtensions.glTexture3DImage)
                {
                    val->setError(EGL_BAD_PARAMETER,
                                  "EGL_GL_TEXTURE_LEVEL cannot be used "
                                  "without KHR_gl_texture_*_image support.");
                    return false;
                }

                if (value < 0)
                {
                    val->setError(EGL_BAD_PARAMETER, "EGL_GL_TEXTURE_LEVEL cannot be negative.");
                    return false;
                }
                break;

            case EGL_GL_TEXTURE_ZOFFSET:
                if (!displayExtensions.glTexture3DImage)
                {
                    val->setError(EGL_BAD_PARAMETER,
                                  "EGL_GL_TEXTURE_ZOFFSET cannot be used "
                                  "without KHR_gl_texture_3D_image support.");
                    return false;
                }
                break;

            case EGL_GL_COLORSPACE:
                if (!displayExtensions.glColorspace)
                {
                    val->setError(EGL_BAD_PARAMETER,
                                  "EGL_GL_COLORSPACE cannot be used "
                                  "without EGL_KHR_gl_colorspace support.");
                    return false;
                }
                switch (value)
                {
                    case EGL_GL_COLORSPACE_DEFAULT_EXT:
                        break;
                    default:
                        ANGLE_VALIDATION_TRY(
                            ValidateColorspaceAttribute(val, displayExtensions, value));
                        break;
                }
                break;

            case EGL_TEXTURE_INTERNAL_FORMAT_ANGLE:
                if (!displayExtensions.imageD3D11Texture && !displayExtensions.vulkanImageANGLE &&
                    !displayExtensions.mtlTextureClientBuffer)
                {
                    val->setError(EGL_BAD_PARAMETER,
                                  "EGL_TEXTURE_INTERNAL_FORMAT_ANGLE cannot be used without "
                                  "EGL_ANGLE_image_d3d11_texture, EGL_ANGLE_vulkan_image, or "
                                  "EGL_ANGLE_metal_texture_client_buffer support.");
                    return false;
                }
                break;

            case EGL_D3D11_TEXTURE_PLANE_ANGLE:
                if (!displayExtensions.imageD3D11Texture)
                {
                    val->setError(EGL_BAD_ATTRIBUTE,
                                  "EGL_D3D11_TEXTURE_PLANE_ANGLE cannot be used without "
                                  "EGL_ANGLE_image_d3d11_texture support.");
                    return false;
                }
                break;

            case EGL_D3D11_TEXTURE_ARRAY_SLICE_ANGLE:
                if (!displayExtensions.imageD3D11Texture)
                {
                    val->setError(EGL_BAD_ATTRIBUTE,
                                  "EGL_D3D11_TEXTURE_ARRAY_SLICE_ANGLE cannot be used without "
                                  "EGL_ANGLE_image_d3d11_texture support.");
                    return false;
                }
                break;
            case EGL_METAL_TEXTURE_ARRAY_SLICE_ANGLE:
                if (!displayExtensions.mtlTextureClientBuffer)
                {
                    val->setError(EGL_BAD_ATTRIBUTE,
                                  "EGL_METAL_TEXTURE_ARRAY_SLICE_ANGLE cannot be used without "
                                  "EGL_ANGLE_metal_texture_client_buffer support.");
                    return false;
                }
                break;

            case EGL_WIDTH:
            case EGL_HEIGHT:
                if (target != EGL_LINUX_DMA_BUF_EXT)
                {
                    val->setError(
                        EGL_BAD_PARAMETER,
                        "Parameter cannot be used if target is not EGL_LINUX_DMA_BUF_EXT");
                    return false;
                }
                break;

            case EGL_LINUX_DRM_FOURCC_EXT:
            case EGL_DMA_BUF_PLANE0_FD_EXT:
            case EGL_DMA_BUF_PLANE0_OFFSET_EXT:
            case EGL_DMA_BUF_PLANE0_PITCH_EXT:
            case EGL_DMA_BUF_PLANE1_FD_EXT:
            case EGL_DMA_BUF_PLANE1_OFFSET_EXT:
            case EGL_DMA_BUF_PLANE1_PITCH_EXT:
            case EGL_DMA_BUF_PLANE2_FD_EXT:
            case EGL_DMA_BUF_PLANE2_OFFSET_EXT:
            case EGL_DMA_BUF_PLANE2_PITCH_EXT:
                if (!displayExtensions.imageDmaBufImportEXT)
                {
                    val->setError(EGL_BAD_PARAMETER,
                                  "Parameter cannot be used without "
                                  "EGL_EXT_image_dma_buf_import support.");
                    return false;
                }
                break;

            case EGL_YUV_COLOR_SPACE_HINT_EXT:
                if (!displayExtensions.imageDmaBufImportEXT)
                {
                    val->setError(EGL_BAD_PARAMETER,
                                  "Parameter cannot be used without "
                                  "EGL_EXT_image_dma_buf_import support.");
                    return false;
                }

                switch (value)
                {
                    case EGL_ITU_REC601_EXT:
                    case EGL_ITU_REC709_EXT:
                    case EGL_ITU_REC2020_EXT:
                        break;

                    default:
                        val->setError(EGL_BAD_ATTRIBUTE,
                                      "Invalid value for EGL_YUV_COLOR_SPACE_HINT_EXT.");
                        return false;
                }
                break;

            case EGL_SAMPLE_RANGE_HINT_EXT:
                if (!displayExtensions.imageDmaBufImportEXT)
                {
                    val->setError(EGL_BAD_PARAMETER,
                                  "Parameter cannot be used without "
                                  "EGL_EXT_image_dma_buf_import support.");
                    return false;
                }

                switch (value)
                {
                    case EGL_YUV_FULL_RANGE_EXT:
                    case EGL_YUV_NARROW_RANGE_EXT:
                        break;

                    default:
                        val->setError(EGL_BAD_ATTRIBUTE,
                                      "Invalid value for EGL_SAMPLE_RANGE_HINT_EXT.");
                        return false;
                }
                break;

            case EGL_YUV_CHROMA_HORIZONTAL_SITING_HINT_EXT:
            case EGL_YUV_CHROMA_VERTICAL_SITING_HINT_EXT:
                if (!displayExtensions.imageDmaBufImportEXT)
                {
                    val->setError(EGL_BAD_PARAMETER,
                                  "Parameter cannot be used without "
                                  "EGL_EXT_image_dma_buf_import support.");
                    return false;
                }

                switch (value)
                {
                    case EGL_YUV_CHROMA_SITING_0_EXT:
                    case EGL_YUV_CHROMA_SITING_0_5_EXT:
                        break;

                    default:
                        val->setError(
                            EGL_BAD_ATTRIBUTE,
                            "Invalid value for EGL_YUV_CHROMA_HORIZONTAL_SITING_HINT_EXT or "
                            "EGL_YUV_CHROMA_VERTICAL_SITING_HINT_EXT.");
                        return false;
                }
                break;

            case EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT:
            case EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT:
            case EGL_DMA_BUF_PLANE1_MODIFIER_LO_EXT:
            case EGL_DMA_BUF_PLANE1_MODIFIER_HI_EXT:
            case EGL_DMA_BUF_PLANE2_MODIFIER_LO_EXT:
            case EGL_DMA_BUF_PLANE2_MODIFIER_HI_EXT:
            case EGL_DMA_BUF_PLANE3_FD_EXT:
            case EGL_DMA_BUF_PLANE3_OFFSET_EXT:
            case EGL_DMA_BUF_PLANE3_PITCH_EXT:
            case EGL_DMA_BUF_PLANE3_MODIFIER_LO_EXT:
            case EGL_DMA_BUF_PLANE3_MODIFIER_HI_EXT:
                if (!displayExtensions.imageDmaBufImportModifiersEXT)
                {
                    val->setError(EGL_BAD_PARAMETER,
                                  "Parameter cannot be used without "
                                  "EGL_EXT_image_dma_buf_import_modifiers support.");
                    return false;
                }
                break;

            case EGL_PROTECTED_CONTENT_EXT:
                if (!displayExtensions.protectedContentEXT)
                {
                    val->setError(EGL_BAD_ATTRIBUTE,
                                  "Attribute EGL_PROTECTED_CONTEXT_EXT requires "
                                  "extension EGL_EXT_protected_content.");
                    return false;
                }
                if (value != EGL_TRUE && value != EGL_FALSE)
                {
                    val->setError(EGL_BAD_ATTRIBUTE,
                                  "EGL_PROTECTED_CONTENT_EXT must "
                                  "be either EGL_TRUE or EGL_FALSE.");
                    return false;
                }
                break;

            case EGL_VULKAN_IMAGE_CREATE_INFO_HI_ANGLE:
            case EGL_VULKAN_IMAGE_CREATE_INFO_LO_ANGLE:
                if (!displayExtensions.vulkanImageANGLE)
                {
                    val->setError(EGL_BAD_ATTRIBUTE,
                                  "Attribute EGL_VULKAN_IMAGE_CREATE_INFO_{HI,LO}_ANGLE require "
                                  "extension EGL_ANGLE_vulkan_image.");
                    return false;
                }
                break;

            default:
                val->setError(EGL_BAD_PARAMETER, "invalid attribute: 0x%04" PRIxPTR "X", attribute);
                return false;
        }
    }

    switch (target)
    {
        case EGL_GL_TEXTURE_2D:
        {
            if (!displayExtensions.glTexture2DImage)
            {
                val->setError(EGL_BAD_PARAMETER, "KHR_gl_texture_2D_image not supported.");
                return false;
            }

            if (buffer == 0)
            {
                val->setError(EGL_BAD_PARAMETER,
                              "buffer cannot reference a 2D texture with the name 0.");
                return false;
            }

            ANGLE_VALIDATION_TRY(ValidateContext(val, display, contextID));
            const gl::Texture *texture =
                context->getTexture({egl_gl::EGLClientBufferToGLObjectHandle(buffer)});
            if (texture == nullptr || texture->getType() != gl::TextureType::_2D)
            {
                val->setError(EGL_BAD_PARAMETER, "target is not a 2D texture.");
                return false;
            }

            if (texture->getBoundSurface() != nullptr)
            {
                val->setError(EGL_BAD_ACCESS, "texture has a surface bound to it.");
                return false;
            }

            EGLAttrib level = attributes.get(EGL_GL_TEXTURE_LEVEL, 0);
            if (texture->getWidth(gl::TextureTarget::_2D, static_cast<size_t>(level)) == 0 ||
                texture->getHeight(gl::TextureTarget::_2D, static_cast<size_t>(level)) == 0)
            {
                val->setError(EGL_BAD_PARAMETER,
                              "target 2D texture does not have a valid size at specified level.");
                return false;
            }

            bool protectedContentAttrib =
                (attributes.getAsInt(EGL_PROTECTED_CONTENT_EXT, EGL_FALSE) != EGL_FALSE);
            if (protectedContentAttrib != texture->hasProtectedContent())
            {
                val->setError(EGL_BAD_PARAMETER,
                              "EGL_PROTECTED_CONTENT_EXT attribute does not match protected state "
                              "of target.");
                return false;
            }

            if (texture->isEGLImageSource(gl::ImageIndex::MakeFromTarget(
                    gl::TextureTarget::_2D, static_cast<GLint>(level), 1)))
            {
                val->setError(EGL_BAD_ACCESS,
                              "The texture has been bound to an existing EGL image.");
                return false;
            }

            ANGLE_VALIDATION_TRY(ValidateCreateImageMipLevelCommon(val, context, texture, level));
        }
        break;

        case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_X:
        case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
        case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
        case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
        case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
        case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
        {
            if (!displayExtensions.glTextureCubemapImage)
            {
                val->setError(EGL_BAD_PARAMETER, "KHR_gl_texture_cubemap_image not supported.");
                return false;
            }

            if (buffer == 0)
            {
                val->setError(EGL_BAD_PARAMETER,
                              "buffer cannot reference a cubemap texture with the name 0.");
                return false;
            }

            ANGLE_VALIDATION_TRY(ValidateContext(val, display, contextID));
            const gl::Texture *texture =
                context->getTexture({egl_gl::EGLClientBufferToGLObjectHandle(buffer)});
            if (texture == nullptr || texture->getType() != gl::TextureType::CubeMap)
            {
                val->setError(EGL_BAD_PARAMETER, "target is not a cubemap texture.");
                return false;
            }

            if (texture->getBoundSurface() != nullptr)
            {
                val->setError(EGL_BAD_ACCESS, "texture has a surface bound to it.");
                return false;
            }

            EGLAttrib level               = attributes.get(EGL_GL_TEXTURE_LEVEL, 0);
            gl::TextureTarget cubeMapFace = egl_gl::EGLCubeMapTargetToCubeMapTarget(target);
            if (texture->getWidth(cubeMapFace, static_cast<size_t>(level)) == 0 ||
                texture->getHeight(cubeMapFace, static_cast<size_t>(level)) == 0)
            {
                val->setError(EGL_BAD_PARAMETER,
                              "target cubemap texture does not have a valid "
                              "size at specified level and face.");
                return false;
            }

            ANGLE_VALIDATION_TRY(ValidateCreateImageMipLevelCommon(val, context, texture, level));

            if (level == 0 && !texture->isMipmapComplete() &&
                CubeTextureHasUnspecifiedLevel0Face(texture))
            {
                val->setError(EGL_BAD_PARAMETER,
                              "if level is zero and the texture is incomplete, "
                              "it must have all of its faces specified at level "
                              "zero.");
                return false;
            }

            bool protectedContentAttrib =
                (attributes.getAsInt(EGL_PROTECTED_CONTENT_EXT, EGL_FALSE) != EGL_FALSE);
            if (protectedContentAttrib != texture->hasProtectedContent())
            {
                val->setError(EGL_BAD_PARAMETER,
                              "EGL_PROTECTED_CONTENT_EXT attribute does not match protected state "
                              "of target.");
                return false;
            }

            gl::TextureTarget glTexTarget =
                gl::CubeFaceIndexToTextureTarget(CubeMapTextureTargetToLayerIndex(target));
            if (texture->isEGLImageSource(
                    gl::ImageIndex::MakeCubeMapFace(glTexTarget, static_cast<GLint>(level))))
            {
                val->setError(EGL_BAD_ACCESS,
                              "The texture has been bound to an existing EGL image.");
                return false;
            }
        }
        break;

        case EGL_GL_TEXTURE_3D:
        {
            if (!displayExtensions.glTexture3DImage)
            {
                val->setError(EGL_BAD_PARAMETER, "KHR_gl_texture_3D_image not supported.");
                return false;
            }

            if (buffer == 0)
            {
                val->setError(EGL_BAD_PARAMETER,
                              "buffer cannot reference a 3D texture with the name 0.");
                return false;
            }

            ANGLE_VALIDATION_TRY(ValidateContext(val, display, contextID));
            const gl::Texture *texture =
                context->getTexture({egl_gl::EGLClientBufferToGLObjectHandle(buffer)});
            if (texture == nullptr || texture->getType() != gl::TextureType::_3D)
            {
                val->setError(EGL_BAD_PARAMETER, "target is not a 3D texture.");
                return false;
            }

            if (texture->getBoundSurface() != nullptr)
            {
                val->setError(EGL_BAD_ACCESS, "texture has a surface bound to it.");
                return false;
            }

            EGLAttrib level   = attributes.get(EGL_GL_TEXTURE_LEVEL, 0);
            EGLAttrib zOffset = attributes.get(EGL_GL_TEXTURE_ZOFFSET, 0);
            if (texture->getWidth(gl::TextureTarget::_3D, static_cast<size_t>(level)) == 0 ||
                texture->getHeight(gl::TextureTarget::_3D, static_cast<size_t>(level)) == 0 ||
                texture->getDepth(gl::TextureTarget::_3D, static_cast<size_t>(level)) == 0)
            {
                val->setError(EGL_BAD_PARAMETER,
                              "target 3D texture does not have a valid size at specified level.");
                return false;
            }

            if (static_cast<size_t>(zOffset) >=
                texture->getDepth(gl::TextureTarget::_3D, static_cast<size_t>(level)))
            {
                val->setError(EGL_BAD_PARAMETER,
                              "target 3D texture does not have enough layers "
                              "for the specified Z offset at the specified "
                              "level.");
                return false;
            }

            bool protectedContentAttrib =
                (attributes.getAsInt(EGL_PROTECTED_CONTENT_EXT, EGL_FALSE) != EGL_FALSE);
            if (protectedContentAttrib != texture->hasProtectedContent())
            {
                val->setError(EGL_BAD_PARAMETER,
                              "EGL_PROTECTED_CONTENT_EXT attribute does not match protected state "
                              "of target.");
                return false;
            }
            if (texture->isEGLImageSource(
                    gl::ImageIndex::Make3D(static_cast<GLint>(level), static_cast<GLint>(zOffset))))
            {
                val->setError(EGL_BAD_ACCESS,
                              "The texture has been bound to an existing EGL image.");
                return false;
            }

            ANGLE_VALIDATION_TRY(ValidateCreateImageMipLevelCommon(val, context, texture, level));
        }
        break;

        case EGL_GL_RENDERBUFFER:
        {
            if (!displayExtensions.glRenderbufferImage)
            {
                val->setError(EGL_BAD_PARAMETER, "KHR_gl_renderbuffer_image not supported.");
                return false;
            }

            if (attributes.contains(EGL_GL_TEXTURE_LEVEL))
            {
                val->setError(EGL_BAD_PARAMETER,
                              "EGL_GL_TEXTURE_LEVEL cannot be used in "
                              "conjunction with a renderbuffer target.");
                return false;
            }

            if (buffer == 0)
            {
                val->setError(EGL_BAD_PARAMETER,
                              "buffer cannot reference a renderbuffer with the name 0.");
                return false;
            }

            ANGLE_VALIDATION_TRY(ValidateContext(val, display, contextID));
            const gl::Renderbuffer *renderbuffer =
                context->getRenderbuffer({egl_gl::EGLClientBufferToGLObjectHandle(buffer)});
            if (renderbuffer == nullptr)
            {
                val->setError(EGL_BAD_PARAMETER, "target is not a renderbuffer.");
                return false;
            }

            if (renderbuffer->getSamples() > 0)
            {
                val->setError(EGL_BAD_PARAMETER, "target renderbuffer cannot be multisampled.");
                return false;
            }

            bool protectedContentAttrib =
                (attributes.getAsInt(EGL_PROTECTED_CONTENT_EXT, EGL_FALSE) != EGL_FALSE);
            if (protectedContentAttrib != renderbuffer->hasProtectedContent())
            {
                val->setError(EGL_BAD_ACCESS,
                              "EGL_PROTECTED_CONTENT_EXT attribute does not match protected state "
                              "of target.");
                return false;
            }

            if (renderbuffer->isEGLImageSource())
            {
                val->setError(EGL_BAD_ACCESS,
                              "The renderbuffer has been bound to an existing EGL image.");
                return false;
            }
        }
        break;

        case EGL_NATIVE_BUFFER_ANDROID:
        {
            if (!displayExtensions.imageNativeBuffer)
            {
                val->setError(EGL_BAD_PARAMETER, "EGL_ANDROID_image_native_buffer not supported.");
                return false;
            }

            if (context != nullptr)
            {
                val->setError(EGL_BAD_CONTEXT, "ctx must be EGL_NO_CONTEXT.");
                return false;
            }

            ANGLE_EGL_TRY_RETURN(
                val->eglThread,
                display->validateImageClientBuffer(context, target, buffer, attributes),
                val->entryPoint, val->labeledObject, false);
        }
        break;

        case EGL_D3D11_TEXTURE_ANGLE:
            if (!displayExtensions.imageD3D11Texture)
            {
                val->setError(EGL_BAD_PARAMETER, "EGL_ANGLE_image_d3d11_texture not supported.");
                return false;
            }

            if (context != nullptr)
            {
                val->setError(EGL_BAD_CONTEXT, "ctx must be EGL_NO_CONTEXT.");
                return false;
            }

            ANGLE_EGL_TRY_RETURN(
                val->eglThread,
                display->validateImageClientBuffer(context, target, buffer, attributes),
                val->entryPoint, val->labeledObject, false);
            break;

        case EGL_LINUX_DMA_BUF_EXT:
            if (!displayExtensions.imageDmaBufImportEXT)
            {
                val->setError(EGL_BAD_PARAMETER, "EGL_EXT_image_dma_buf_import not supported.");
                return false;
            }

            if (context != nullptr)
            {
                val->setError(EGL_BAD_CONTEXT, "ctx must be EGL_NO_CONTEXT.");
                return false;
            }

            if (buffer != nullptr)
            {
                val->setError(EGL_BAD_PARAMETER, "buffer must be NULL.");
                return false;
            }

            {
                EGLenum kRequiredParameters[] = {EGL_WIDTH,
                                                 EGL_HEIGHT,
                                                 EGL_LINUX_DRM_FOURCC_EXT,
                                                 EGL_DMA_BUF_PLANE0_FD_EXT,
                                                 EGL_DMA_BUF_PLANE0_OFFSET_EXT,
                                                 EGL_DMA_BUF_PLANE0_PITCH_EXT};
                for (EGLenum requiredParameter : kRequiredParameters)
                {
                    if (!attributes.contains(requiredParameter))
                    {
                        val->setError(EGL_BAD_PARAMETER,
                                      "Missing required parameter 0x%X for image target "
                                      "EGL_LINUX_DMA_BUF_EXT.",
                                      requiredParameter);
                        return false;
                    }
                }

                bool containPlane0ModifierLo =
                    attributes.contains(EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT);
                bool containPlane0ModifierHi =
                    attributes.contains(EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT);
                bool containPlane1ModifierLo =
                    attributes.contains(EGL_DMA_BUF_PLANE1_MODIFIER_LO_EXT);
                bool containPlane1ModifierHi =
                    attributes.contains(EGL_DMA_BUF_PLANE1_MODIFIER_HI_EXT);
                bool containPlane2ModifierLo =
                    attributes.contains(EGL_DMA_BUF_PLANE2_MODIFIER_LO_EXT);
                bool containPlane2ModifierHi =
                    attributes.contains(EGL_DMA_BUF_PLANE2_MODIFIER_HI_EXT);
                bool containPlane3ModifierLo =
                    attributes.contains(EGL_DMA_BUF_PLANE3_MODIFIER_LO_EXT);
                bool containPlane3ModifierHi =
                    attributes.contains(EGL_DMA_BUF_PLANE3_MODIFIER_HI_EXT);
                if ((containPlane0ModifierLo ^ containPlane0ModifierHi) ||
                    (containPlane1ModifierLo ^ containPlane1ModifierHi) ||
                    (containPlane2ModifierLo ^ containPlane2ModifierHi) ||
                    (containPlane3ModifierLo ^ containPlane3ModifierHi))
                {
                    val->setError(
                        EGL_BAD_PARAMETER,
                        "the list of attributes contains EGL_DMA_BUF_PLANE*_MODIFIER_LO_EXT "
                        "but not EGL_DMA_BUF_PLANE*_MODIFIER_HI_EXT or vice versa.");
                    return false;
                }
            }
            break;

        case EGL_METAL_TEXTURE_ANGLE:
            if (!displayExtensions.mtlTextureClientBuffer)
            {
                val->setError(EGL_BAD_PARAMETER,
                              "EGL_ANGLE_metal_texture_client_buffer not supported.");
                return false;
            }

            if (context != nullptr)
            {
                val->setError(EGL_BAD_CONTEXT, "ctx must be EGL_NO_CONTEXT.");
                return false;
            }

            ANGLE_EGL_TRY_RETURN(
                val->eglThread,
                display->validateImageClientBuffer(context, target, buffer, attributes),
                val->entryPoint, val->labeledObject, false);
            break;
        case EGL_VULKAN_IMAGE_ANGLE:
            if (!displayExtensions.vulkanImageANGLE)
            {
                val->setError(EGL_BAD_PARAMETER, "EGL_ANGLE_vulkan_image not supported.");
                return false;
            }

            if (context != nullptr)
            {
                val->setError(EGL_BAD_CONTEXT, "ctx must be EGL_NO_CONTEXT.");
                return false;
            }

            {
                const EGLenum kRequiredParameters[] = {
                    EGL_VULKAN_IMAGE_CREATE_INFO_HI_ANGLE,
                    EGL_VULKAN_IMAGE_CREATE_INFO_LO_ANGLE,
                };
                for (EGLenum requiredParameter : kRequiredParameters)
                {
                    if (!attributes.contains(requiredParameter))
                    {
                        val->setError(EGL_BAD_PARAMETER,
                                      "Missing required parameter 0x%X for image target "
                                      "EGL_VULKAN_IMAGE_ANGLE.",
                                      requiredParameter);
                        return false;
                    }
                }
            }

            ANGLE_EGL_TRY_RETURN(
                val->eglThread,
                display->validateImageClientBuffer(context, target, buffer, attributes),
                val->entryPoint, val->labeledObject, false);
            break;
        default:
            val->setError(EGL_BAD_PARAMETER, "invalid target: 0x%X", target);
            return false;
    }

    if (attributes.contains(EGL_GL_TEXTURE_ZOFFSET) && target != EGL_GL_TEXTURE_3D)
    {
        val->setError(EGL_BAD_PARAMETER,
                      "EGL_GL_TEXTURE_ZOFFSET must be used with a 3D texture target.");
        return false;
    }

    return true;
}

bool ValidateDestroyImage(const ValidationContext *val, const Display *display, ImageID imageID)
{
    ANGLE_VALIDATION_TRY(ValidateImage(val, display, imageID));
    return true;
}

bool ValidateCreateImageKHR(const ValidationContext *val,
                            const Display *display,
                            gl::ContextID contextID,
                            EGLenum target,
                            EGLClientBuffer buffer,
                            const AttributeMap &attributes)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));

    if (!display->getExtensions().imageBase && !display->getExtensions().image)
    {
        // It is out of spec what happens when calling an extension function when the extension is
        // not available.
        // EGL_BAD_DISPLAY seems like a reasonable error.
        val->setError(EGL_BAD_DISPLAY, "EGL_KHR_image not supported.");
        return false;
    }

    return ValidateCreateImage(val, display, contextID, target, buffer, attributes);
}

bool ValidateDestroyImageKHR(const ValidationContext *val, const Display *display, ImageID imageID)
{
    ANGLE_VALIDATION_TRY(ValidateImage(val, display, imageID));

    if (!display->getExtensions().imageBase && !display->getExtensions().image)
    {
        // It is out of spec what happens when calling an extension function when the extension is
        // not available.
        // EGL_BAD_DISPLAY seems like a reasonable error.
        val->setError(EGL_BAD_DISPLAY);
        return false;
    }

    return true;
}

bool ValidateCreateDeviceANGLE(const ValidationContext *val,
                               EGLint device_type,
                               const void *native_device,
                               const EGLAttrib *attrib_list)
{
    const ClientExtensions &clientExtensions = Display::GetClientExtensions();
    if (!clientExtensions.deviceCreation)
    {
        val->setError(EGL_BAD_ACCESS, "Device creation extension not active");
        return false;
    }

    if (attrib_list != nullptr && attrib_list[0] != EGL_NONE)
    {
        val->setError(EGL_BAD_ATTRIBUTE, "Invalid attrib_list parameter");
        return false;
    }

    switch (device_type)
    {
        case EGL_D3D11_DEVICE_ANGLE:
            if (!clientExtensions.deviceCreationD3D11)
            {
                val->setError(EGL_BAD_ATTRIBUTE, "D3D11 device creation extension not active");
                return false;
            }
            break;
        default:
            val->setError(EGL_BAD_ATTRIBUTE, "Invalid device_type parameter");
            return false;
    }

    return true;
}

bool ValidateReleaseDeviceANGLE(const ValidationContext *val, const Device *device)
{
    const ClientExtensions &clientExtensions = Display::GetClientExtensions();
    if (!clientExtensions.deviceCreation)
    {
        val->setError(EGL_BAD_ACCESS, "Device creation extension not active");
        return false;
    }

    if (device == EGL_NO_DEVICE_EXT || !Device::IsValidDevice(device))
    {
        val->setError(EGL_BAD_DEVICE_EXT, "Invalid device parameter");
        return false;
    }

    Display *owningDisplay = device->getOwningDisplay();
    if (owningDisplay != nullptr)
    {
        val->setError(EGL_BAD_DEVICE_EXT, "Device must have been created using eglCreateDevice");
        return false;
    }

    return true;
}

bool ValidateCreateSync(const ValidationContext *val,
                        const Display *display,
                        EGLenum type,
                        const AttributeMap &attribs)
{
    return ValidateCreateSyncBase(val, display, type, attribs, false);
}

bool ValidateCreateSyncKHR(const ValidationContext *val,
                           const Display *display,
                           EGLenum type,
                           const AttributeMap &attribs)
{
    return ValidateCreateSyncBase(val, display, type, attribs, true);
}

bool ValidateDestroySync(const ValidationContext *val, const Display *display, SyncID sync)
{
    ANGLE_VALIDATION_TRY(ValidateSync(val, display, sync));
    return true;
}

bool ValidateDestroySyncKHR(const ValidationContext *val,
                            const Display *dpyPacked,
                            SyncID syncPacked)
{
    return ValidateDestroySync(val, dpyPacked, syncPacked);
}

bool ValidateClientWaitSync(const ValidationContext *val,
                            const Display *display,
                            SyncID sync,
                            EGLint flags,
                            EGLTime timeout)
{
    ANGLE_VALIDATION_TRY(ValidateSync(val, display, sync));
    return true;
}

bool ValidateClientWaitSyncKHR(const ValidationContext *val,
                               const Display *dpyPacked,
                               SyncID syncPacked,
                               EGLint flags,
                               EGLTimeKHR timeout)
{
    return ValidateClientWaitSync(val, dpyPacked, syncPacked, flags, timeout);
}

bool ValidateWaitSync(const ValidationContext *val,
                      const Display *display,
                      SyncID sync,
                      EGLint flags)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));

    const DisplayExtensions &extensions = display->getExtensions();
    if (!extensions.waitSync)
    {
        val->setError(EGL_BAD_ACCESS, "EGL_KHR_wait_sync extension is not available");
        return false;
    }

    ANGLE_VALIDATION_TRY(ValidateSync(val, display, sync));
    ANGLE_VALIDATION_TRY(ValidateThreadContext(val, display, EGL_BAD_MATCH));

    gl::Context *context = val->eglThread->getContext();
    if (!context->getExtensions().EGLSyncOES)
    {
        val->setError(EGL_BAD_MATCH,
                      "Server-side waits cannot be performed without "
                      "GL_OES_EGL_sync support.");
        return false;
    }

    if (flags != 0)
    {
        val->setError(EGL_BAD_PARAMETER, "flags must be zero");
        return false;
    }

    return true;
}

bool ValidateWaitSyncKHR(const ValidationContext *val,
                         const Display *dpyPacked,
                         SyncID syncPacked,
                         EGLint flags)
{
    return ValidateWaitSync(val, dpyPacked, syncPacked, flags);
}

bool ValidateGetSyncAttrib(const ValidationContext *val,
                           const Display *display,
                           SyncID sync,
                           EGLint attribute,
                           const EGLAttrib *value)
{
    if (value == nullptr)
    {
        val->setError(EGL_BAD_PARAMETER, "Invalid value parameter");
        return false;
    }
    return ValidateGetSyncAttribBase(val, display, sync, attribute);
}

bool ValidateGetSyncAttribKHR(const ValidationContext *val,
                              const Display *display,
                              SyncID sync,
                              EGLint attribute,
                              const EGLint *value)
{
    if (value == nullptr)
    {
        val->setError(EGL_BAD_PARAMETER, "Invalid value parameter");
        return false;
    }
    return ValidateGetSyncAttribBase(val, display, sync, attribute);
}

bool ValidateCreateStreamKHR(const ValidationContext *val,
                             const Display *display,
                             const AttributeMap &attributes)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));

    const DisplayExtensions &displayExtensions = display->getExtensions();
    if (!displayExtensions.stream)
    {
        val->setError(EGL_BAD_ALLOC, "Stream extension not active");
        return false;
    }

    attributes.initializeWithoutValidation();

    for (const auto &attributeIter : attributes)
    {
        EGLAttrib attribute = attributeIter.first;
        EGLAttrib value     = attributeIter.second;

        ANGLE_VALIDATION_TRY(ValidateStreamAttribute(val, attribute, value, displayExtensions));
    }

    return true;
}

bool ValidateDestroyStreamKHR(const ValidationContext *val,
                              const Display *display,
                              const Stream *stream)
{
    ANGLE_VALIDATION_TRY(ValidateStream(val, display, stream));
    return true;
}

bool ValidateStreamAttribKHR(const ValidationContext *val,
                             const Display *display,
                             const Stream *stream,
                             EGLenum attribute,
                             EGLint value)
{
    ANGLE_VALIDATION_TRY(ValidateStream(val, display, stream));

    if (stream->getState() == EGL_STREAM_STATE_DISCONNECTED_KHR)
    {
        val->setError(EGL_BAD_STATE_KHR, "Bad stream state");
        return false;
    }

    return ValidateStreamAttribute(val, attribute, value, display->getExtensions());
}

bool ValidateQueryStreamKHR(const ValidationContext *val,
                            const Display *display,
                            const Stream *stream,
                            EGLenum attribute,
                            const EGLint *value)
{
    ANGLE_VALIDATION_TRY(ValidateStream(val, display, stream));

    switch (attribute)
    {
        case EGL_STREAM_STATE_KHR:
        case EGL_CONSUMER_LATENCY_USEC_KHR:
            break;
        case EGL_CONSUMER_ACQUIRE_TIMEOUT_USEC_KHR:
            if (!display->getExtensions().streamConsumerGLTexture)
            {
                val->setError(EGL_BAD_ATTRIBUTE, "Consumer GLTexture extension not active");
                return false;
            }
            break;
        default:
            val->setError(EGL_BAD_ATTRIBUTE, "Invalid attribute");
            return false;
    }

    return true;
}

bool ValidateQueryStreamu64KHR(const ValidationContext *val,
                               const Display *display,
                               const Stream *stream,
                               EGLenum attribute,
                               const EGLuint64KHR *value)
{
    ANGLE_VALIDATION_TRY(ValidateStream(val, display, stream));

    switch (attribute)
    {
        case EGL_CONSUMER_FRAME_KHR:
        case EGL_PRODUCER_FRAME_KHR:
            break;
        default:
            val->setError(EGL_BAD_ATTRIBUTE, "Invalid attribute");
            return false;
    }

    return true;
}

bool ValidateStreamConsumerGLTextureExternalKHR(const ValidationContext *val,
                                                const Display *display,
                                                const Stream *stream)
{
    ANGLE_VALIDATION_TRY(ValidateThreadContext(val, display, EGL_BAD_CONTEXT));

    const DisplayExtensions &displayExtensions = display->getExtensions();
    if (!displayExtensions.streamConsumerGLTexture)
    {
        val->setError(EGL_BAD_ACCESS, "Stream consumer extension not active");
        return false;
    }

    gl::Context *context = val->eglThread->getContext();
    if (!context->getExtensions().EGLStreamConsumerExternalNV)
    {
        val->setError(EGL_BAD_ACCESS, "EGL stream consumer external GL extension not enabled");
        return false;
    }

    if (stream == EGL_NO_STREAM_KHR || !display->isValidStream(stream))
    {
        val->setError(EGL_BAD_STREAM_KHR, "Invalid stream");
        return false;
    }

    if (stream->getState() != EGL_STREAM_STATE_CREATED_KHR)
    {
        val->setError(EGL_BAD_STATE_KHR, "Invalid stream state");
        return false;
    }

    // Lookup the texture and ensure it is correct
    gl::Texture *texture = context->getState().getTargetTexture(gl::TextureType::External);
    if (texture == nullptr || texture->id().value == 0)
    {
        val->setError(EGL_BAD_ACCESS, "No external texture bound");
        return false;
    }

    return true;
}

bool ValidateStreamConsumerAcquireKHR(const ValidationContext *val,
                                      const Display *display,
                                      const Stream *stream)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));

    const DisplayExtensions &displayExtensions = display->getExtensions();
    if (!displayExtensions.streamConsumerGLTexture)
    {
        val->setError(EGL_BAD_ACCESS, "Stream consumer extension not active");
        return false;
    }

    if (stream == EGL_NO_STREAM_KHR || !display->isValidStream(stream))
    {
        val->setError(EGL_BAD_STREAM_KHR, "Invalid stream");
        return false;
    }

    ANGLE_VALIDATION_TRY(ValidateThreadContext(val, display, EGL_BAD_CONTEXT));

    gl::Context *context = val->eglThread->getContext();
    if (!stream->isConsumerBoundToContext(context))
    {
        val->setError(EGL_BAD_ACCESS, "Current GL context not associated with stream consumer");
        return false;
    }

    if (stream->getConsumerType() != Stream::ConsumerType::GLTextureRGB &&
        stream->getConsumerType() != Stream::ConsumerType::GLTextureYUV)
    {
        val->setError(EGL_BAD_ACCESS, "Invalid stream consumer type");
        return false;
    }

    // Note: technically EGL_STREAM_STATE_EMPTY_KHR is a valid state when the timeout is non-zero.
    // However, the timeout is effectively ignored since it has no useful functionality with the
    // current producers that are implemented, so we don't allow that state
    if (stream->getState() != EGL_STREAM_STATE_NEW_FRAME_AVAILABLE_KHR &&
        stream->getState() != EGL_STREAM_STATE_OLD_FRAME_AVAILABLE_KHR)
    {
        val->setError(EGL_BAD_STATE_KHR, "Invalid stream state");
        return false;
    }

    return true;
}

bool ValidateStreamConsumerReleaseKHR(const ValidationContext *val,
                                      const Display *display,
                                      const Stream *stream)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));

    const DisplayExtensions &displayExtensions = display->getExtensions();
    if (!displayExtensions.streamConsumerGLTexture)
    {
        val->setError(EGL_BAD_ACCESS, "Stream consumer extension not active");
        return false;
    }

    if (stream == EGL_NO_STREAM_KHR || !display->isValidStream(stream))
    {
        val->setError(EGL_BAD_STREAM_KHR, "Invalid stream");
        return false;
    }

    ANGLE_VALIDATION_TRY(ValidateThreadContext(val, display, EGL_BAD_CONTEXT));

    gl::Context *context = val->eglThread->getContext();
    if (!stream->isConsumerBoundToContext(context))
    {
        val->setError(EGL_BAD_ACCESS, "Current GL context not associated with stream consumer");
        return false;
    }

    if (stream->getConsumerType() != Stream::ConsumerType::GLTextureRGB &&
        stream->getConsumerType() != Stream::ConsumerType::GLTextureYUV)
    {
        val->setError(EGL_BAD_ACCESS, "Invalid stream consumer type");
        return false;
    }

    if (stream->getState() != EGL_STREAM_STATE_NEW_FRAME_AVAILABLE_KHR &&
        stream->getState() != EGL_STREAM_STATE_OLD_FRAME_AVAILABLE_KHR)
    {
        val->setError(EGL_BAD_STATE_KHR, "Invalid stream state");
        return false;
    }

    return true;
}

bool ValidateStreamConsumerGLTextureExternalAttribsNV(const ValidationContext *val,
                                                      const Display *display,
                                                      const Stream *stream,
                                                      const AttributeMap &attribs)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));

    const DisplayExtensions &displayExtensions = display->getExtensions();
    if (!displayExtensions.streamConsumerGLTexture)
    {
        val->setError(EGL_BAD_ACCESS, "Stream consumer extension not active");
        return false;
    }

    ANGLE_VALIDATION_TRY(ValidateThreadContext(val, display, EGL_BAD_CONTEXT));

    // Although technically not a requirement in spec, the context needs to be checked for support
    // for external textures or future logic will cause assertions. This extension is also
    // effectively useless without external textures.
    gl::Context *context = val->eglThread->getContext();
    if (!context->getExtensions().EGLStreamConsumerExternalNV)
    {
        val->setError(EGL_BAD_ACCESS, "EGL stream consumer external GL extension not enabled");
        return false;
    }

    if (stream == EGL_NO_STREAM_KHR || !display->isValidStream(stream))
    {
        val->setError(EGL_BAD_STREAM_KHR, "Invalid stream");
        return false;
    }

    if (stream->getState() != EGL_STREAM_STATE_CREATED_KHR)
    {
        val->setError(EGL_BAD_STATE_KHR, "Invalid stream state");
        return false;
    }

    const gl::Caps &glCaps = context->getCaps();

    EGLAttrib colorBufferType = EGL_RGB_BUFFER;
    EGLAttrib planeCount      = -1;
    EGLAttrib plane[3];
    for (int i = 0; i < 3; i++)
    {
        plane[i] = -1;
    }

    attribs.initializeWithoutValidation();

    for (const auto &attributeIter : attribs)
    {
        EGLAttrib attribute = attributeIter.first;
        EGLAttrib value     = attributeIter.second;

        switch (attribute)
        {
            case EGL_COLOR_BUFFER_TYPE:
                if (value != EGL_RGB_BUFFER && value != EGL_YUV_BUFFER_EXT)
                {
                    val->setError(EGL_BAD_PARAMETER, "Invalid color buffer type");
                    return false;
                }
                colorBufferType = value;
                break;
            case EGL_YUV_NUMBER_OF_PLANES_EXT:
                // planeCount = -1 is a tag for the default plane count so the value must be checked
                // to be positive here to ensure future logic doesn't break on invalid negative
                // inputs
                if (value < 0)
                {
                    val->setError(EGL_BAD_MATCH, "Invalid plane count");
                    return false;
                }
                planeCount = value;
                break;
            default:
                if (attribute >= EGL_YUV_PLANE0_TEXTURE_UNIT_NV &&
                    attribute <= EGL_YUV_PLANE2_TEXTURE_UNIT_NV)
                {
                    if ((value < 0 ||
                         value >= static_cast<EGLAttrib>(glCaps.maxCombinedTextureImageUnits)) &&
                        value != EGL_NONE)
                    {
                        val->setError(EGL_BAD_ACCESS, "Invalid texture unit");
                        return false;
                    }
                    plane[attribute - EGL_YUV_PLANE0_TEXTURE_UNIT_NV] = value;
                }
                else
                {
                    val->setError(EGL_BAD_ATTRIBUTE, "Invalid attribute");
                    return false;
                }
        }
    }

    if (colorBufferType == EGL_RGB_BUFFER)
    {
        if (planeCount > 0)
        {
            val->setError(EGL_BAD_MATCH, "Plane count must be 0 for RGB buffer");
            return false;
        }
        for (int i = 0; i < 3; i++)
        {
            if (plane[i] != -1)
            {
                val->setError(EGL_BAD_MATCH, "Planes cannot be specified");
                return false;
            }
        }

        // Lookup the texture and ensure it is correct
        gl::Texture *texture = context->getState().getTargetTexture(gl::TextureType::External);
        if (texture == nullptr || texture->id().value == 0)
        {
            val->setError(EGL_BAD_ACCESS, "No external texture bound");
            return false;
        }
    }
    else
    {
        if (planeCount == -1)
        {
            planeCount = 2;
        }
        if (planeCount < 1 || planeCount > 3)
        {
            val->setError(EGL_BAD_MATCH, "Invalid YUV plane count");
            return false;
        }
        for (EGLAttrib i = planeCount; i < 3; i++)
        {
            if (plane[i] != -1)
            {
                val->setError(EGL_BAD_MATCH, "Invalid plane specified");
                return false;
            }
        }

        // Set to ensure no texture is referenced more than once
        std::set<gl::Texture *> textureSet;
        for (EGLAttrib i = 0; i < planeCount; i++)
        {
            if (plane[i] == -1)
            {
                val->setError(EGL_BAD_MATCH, "Not all planes specified");
                return false;
            }
            if (plane[i] != EGL_NONE)
            {
                gl::Texture *texture = context->getState().getSamplerTexture(
                    static_cast<unsigned int>(plane[i]), gl::TextureType::External);
                if (texture == nullptr || texture->id().value == 0)
                {
                    val->setError(
                        EGL_BAD_ACCESS,
                        "No external texture bound at one or more specified texture units");
                    return false;
                }
                if (textureSet.find(texture) != textureSet.end())
                {
                    val->setError(EGL_BAD_ACCESS, "Multiple planes bound to same texture object");
                    return false;
                }
                textureSet.insert(texture);
            }
        }
    }

    return true;
}

bool ValidateCreateStreamProducerD3DTextureANGLE(const ValidationContext *val,
                                                 const Display *display,
                                                 const Stream *stream,
                                                 const AttributeMap &attribs)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));

    const DisplayExtensions &displayExtensions = display->getExtensions();
    if (!displayExtensions.streamProducerD3DTexture)
    {
        val->setError(EGL_BAD_ACCESS, "Stream producer extension not active");
        return false;
    }

    ANGLE_VALIDATION_TRY(ValidateStream(val, display, stream));

    attribs.initializeWithoutValidation();

    if (!attribs.isEmpty())
    {
        val->setError(EGL_BAD_ATTRIBUTE, "Invalid attribute");
        return false;
    }

    if (stream->getState() != EGL_STREAM_STATE_CONNECTING_KHR)
    {
        val->setError(EGL_BAD_STATE_KHR, "Stream not in connecting state");
        return false;
    }

    switch (stream->getConsumerType())
    {
        case Stream::ConsumerType::GLTextureYUV:
            if (stream->getPlaneCount() != 2)
            {
                val->setError(EGL_BAD_MATCH, "Incompatible stream consumer type");
                return false;
            }
            break;

        case Stream::ConsumerType::GLTextureRGB:
            if (stream->getPlaneCount() != 1)
            {
                val->setError(EGL_BAD_MATCH, "Incompatible stream consumer type");
                return false;
            }
            break;

        default:
            val->setError(EGL_BAD_MATCH, "Incompatible stream consumer type");
            return false;
    }

    return true;
}

bool ValidateStreamPostD3DTextureANGLE(const ValidationContext *val,
                                       const Display *display,
                                       const Stream *stream,
                                       const void *texture,
                                       const AttributeMap &attribs)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));

    const DisplayExtensions &displayExtensions = display->getExtensions();
    if (!displayExtensions.streamProducerD3DTexture)
    {
        val->setError(EGL_BAD_ACCESS, "Stream producer extension not active");
        return false;
    }

    ANGLE_VALIDATION_TRY(ValidateStream(val, display, stream));

    attribs.initializeWithoutValidation();

    for (auto &attributeIter : attribs)
    {
        EGLAttrib attribute = attributeIter.first;
        EGLAttrib value     = attributeIter.second;

        switch (attribute)
        {
            case EGL_D3D_TEXTURE_SUBRESOURCE_ID_ANGLE:
                if (value < 0)
                {
                    val->setError(EGL_BAD_PARAMETER, "Invalid subresource index");
                    return false;
                }
                break;
            case EGL_NATIVE_BUFFER_PLANE_OFFSET_IMG:
                if (value < 0)
                {
                    val->setError(EGL_BAD_PARAMETER, "Invalid plane offset");
                    return false;
                }
                break;
            default:
                val->setError(EGL_BAD_ATTRIBUTE, "Invalid attribute");
                return false;
        }
    }

    if (stream->getState() != EGL_STREAM_STATE_EMPTY_KHR &&
        stream->getState() != EGL_STREAM_STATE_NEW_FRAME_AVAILABLE_KHR &&
        stream->getState() != EGL_STREAM_STATE_OLD_FRAME_AVAILABLE_KHR)
    {
        val->setError(EGL_BAD_STATE_KHR, "Stream not fully configured");
        return false;
    }

    if (stream->getProducerType() != Stream::ProducerType::D3D11Texture)
    {
        val->setError(EGL_BAD_MATCH, "Incompatible stream producer");
        return false;
    }

    if (texture == nullptr)
    {
        val->setError(EGL_BAD_PARAMETER, "Texture is null");
        return false;
    }

    ANGLE_EGL_TRY_RETURN(val->eglThread, stream->validateD3D11Texture(texture, attribs),
                         val->entryPoint, val->labeledObject, false);

    return true;
}

bool ValidateSyncControlCHROMIUM(const ValidationContext *val,
                                 const Display *display,
                                 SurfaceID surfaceID)
{
    ANGLE_VALIDATION_TRY(ValidateSurface(val, display, surfaceID));

    const DisplayExtensions &displayExtensions = display->getExtensions();
    if (!displayExtensions.syncControlCHROMIUM)
    {
        val->setError(EGL_BAD_ACCESS, "syncControlCHROMIUM extension not active");
        return false;
    }

    return true;
}

bool ValidateSyncControlRateANGLE(const ValidationContext *val,
                                  const Display *display,
                                  SurfaceID surfaceID)
{
    ANGLE_VALIDATION_TRY(ValidateSurface(val, display, surfaceID));

    const DisplayExtensions &displayExtensions = display->getExtensions();
    if (!displayExtensions.syncControlRateANGLE)
    {
        val->setError(EGL_BAD_ACCESS, "syncControlRateANGLE extension not active");
        return false;
    }

    return true;
}

bool ValidateGetMscRateANGLE(const ValidationContext *val,
                             const Display *display,
                             SurfaceID surfaceID,
                             const EGLint *numerator,
                             const EGLint *denominator)
{
    ANGLE_VALIDATION_TRY(ValidateSyncControlRateANGLE(val, display, surfaceID));

    if (numerator == nullptr)
    {
        val->setError(EGL_BAD_PARAMETER, "numerator is null");
        return false;
    }
    if (denominator == nullptr)
    {
        val->setError(EGL_BAD_PARAMETER, "denominator is null");
        return false;
    }

    return true;
}

bool ValidateGetSyncValuesCHROMIUM(const ValidationContext *val,
                                   const Display *display,
                                   SurfaceID surfaceID,
                                   const EGLuint64KHR *ust,
                                   const EGLuint64KHR *msc,
                                   const EGLuint64KHR *sbc)
{
    ANGLE_VALIDATION_TRY(ValidateSyncControlCHROMIUM(val, display, surfaceID));

    if (ust == nullptr)
    {
        val->setError(EGL_BAD_PARAMETER, "ust is null");
        return false;
    }
    if (msc == nullptr)
    {
        val->setError(EGL_BAD_PARAMETER, "msc is null");
        return false;
    }
    if (sbc == nullptr)
    {
        val->setError(EGL_BAD_PARAMETER, "sbc is null");
        return false;
    }

    return true;
}

bool ValidateDestroySurface(const ValidationContext *val,
                            const Display *display,
                            SurfaceID surfaceID)
{
    ANGLE_VALIDATION_TRY(ValidateSurface(val, display, surfaceID));
    return true;
}

bool ValidateDestroyContext(const ValidationContext *val,
                            const Display *display,
                            gl::ContextID contextID)
{
    ANGLE_VALIDATION_TRY(ValidateContext(val, display, contextID));
    return true;
}

bool ValidateSwapBuffers(const ValidationContext *val, const Display *display, SurfaceID surfaceID)
{
    ANGLE_VALIDATION_TRY(ValidateSurface(val, display, surfaceID));

    if (display->isDeviceLost())
    {
        val->setError(EGL_CONTEXT_LOST);
        return false;
    }

    const Surface *eglSurface = display->getSurface(surfaceID);
    if (eglSurface->isLocked())
    {
        val->setError(EGL_BAD_ACCESS);
        return false;
    }

    if (eglSurface == EGL_NO_SURFACE || !val->eglThread->getContext() ||
        val->eglThread->getCurrentDrawSurface() != eglSurface)
    {
        val->setError(EGL_BAD_SURFACE);
        return false;
    }

    return true;
}

bool ValidateSwapBuffersWithDamageKHR(const ValidationContext *val,
                                      const Display *display,
                                      SurfaceID surfaceID,
                                      const EGLint *rects,
                                      EGLint n_rects)
{
    ANGLE_VALIDATION_TRY(ValidateSurface(val, display, surfaceID));

    if (!display->getExtensions().swapBuffersWithDamage)
    {
        // It is out of spec what happens when calling an extension function when the extension is
        // not available. EGL_BAD_DISPLAY seems like a reasonable error.
        val->setError(EGL_BAD_DISPLAY, "EGL_KHR_swap_buffers_with_damage is not available.");
        return false;
    }

    const Surface *surface = display->getSurface(surfaceID);
    if (surface == nullptr)
    {
        val->setError(EGL_BAD_SURFACE, "Swap surface cannot be EGL_NO_SURFACE.");
        return false;
    }

    if (n_rects < 0)
    {
        val->setError(EGL_BAD_PARAMETER, "n_rects cannot be negative.");
        return false;
    }

    if (n_rects > 0 && rects == nullptr)
    {
        val->setError(EGL_BAD_PARAMETER, "n_rects cannot be greater than zero when rects is NULL.");
        return false;
    }

    if (surface->isLocked())
    {
        val->setError(EGL_BAD_ACCESS);
        return false;
    }

    // TODO(jmadill): Validate Surface is bound to the thread.

    return true;
}

bool ValidateWaitNative(const ValidationContext *val, const EGLint engine)
{
    if (val->eglThread->getDisplay() == nullptr)
    {
        // EGL spec says this about eglWaitNative -
        //    eglWaitNative is ignored if there is no current EGL rendering context.
        return true;
    }

    ANGLE_VALIDATION_TRY(ValidateDisplay(val, val->eglThread->getDisplay()));

    if (engine != EGL_CORE_NATIVE_ENGINE)
    {
        val->setError(EGL_BAD_PARAMETER, "the 'engine' parameter has an unrecognized value");
        return false;
    }

    return true;
}

bool ValidateCopyBuffers(const ValidationContext *val,
                         const Display *display,
                         SurfaceID surfaceID,
                         EGLNativePixmapType target)
{
    ANGLE_VALIDATION_TRY(ValidateSurface(val, display, surfaceID));

    if (display->isDeviceLost())
    {
        val->setError(EGL_CONTEXT_LOST);
        return false;
    }

    return true;
}

bool ValidateBindTexImage(const ValidationContext *val,
                          const Display *display,
                          SurfaceID surfaceID,
                          const EGLint buffer)
{
    ANGLE_VALIDATION_TRY(ValidateSurface(val, display, surfaceID));

    if (buffer != EGL_BACK_BUFFER)
    {
        val->setError(EGL_BAD_PARAMETER);
        return false;
    }

    const Surface *surface = display->getSurface(surfaceID);
    if (surface->getType() == EGL_WINDOW_BIT)
    {
        val->setError(EGL_BAD_SURFACE);
        return false;
    }

    if (surface->getBoundTexture())
    {
        val->setError(EGL_BAD_ACCESS);
        return false;
    }

    if (surface->getTextureFormat() == TextureFormat::NoTexture)
    {
        val->setError(EGL_BAD_MATCH);
        return false;
    }

    if (surface->isLocked())
    {
        val->setError(EGL_BAD_ACCESS);
        return false;
    }

    gl::Context *context = val->eglThread->getContext();
    if (context && !context->isContextLost())
    {
        gl::TextureType type = egl_gl::EGLTextureTargetToTextureType(surface->getTextureTarget());
        gl::Texture *textureObject = context->getTextureByType(type);
        ASSERT(textureObject != nullptr);

        if (textureObject->getImmutableFormat())
        {
            val->setError(EGL_BAD_MATCH);
            return false;
        }
    }

    return true;
}

bool ValidateReleaseTexImage(const ValidationContext *val,
                             const Display *display,
                             SurfaceID surfaceID,
                             const EGLint buffer)
{
    ANGLE_VALIDATION_TRY(ValidateSurface(val, display, surfaceID));

    if (buffer != EGL_BACK_BUFFER)
    {
        val->setError(EGL_BAD_PARAMETER);
        return false;
    }

    const Surface *surface = display->getSurface(surfaceID);
    if (surface->getType() == EGL_WINDOW_BIT)
    {
        val->setError(EGL_BAD_SURFACE);
        return false;
    }

    if (surface->getTextureFormat() == TextureFormat::NoTexture)
    {
        val->setError(EGL_BAD_MATCH);
        return false;
    }

    return true;
}

bool ValidateSwapInterval(const ValidationContext *val, const Display *display, EGLint interval)
{
    ANGLE_VALIDATION_TRY(ValidateThreadContext(val, display, EGL_BAD_CONTEXT));

    Surface *drawSurface = val->eglThread->getCurrentDrawSurface();
    if (drawSurface == nullptr)
    {
        val->setError(EGL_BAD_SURFACE);
        return false;
    }

    return true;
}

bool ValidateBindAPI(const ValidationContext *val, const EGLenum api)
{
    switch (api)
    {
        case EGL_OPENGL_ES_API:
            break;
        case EGL_OPENVG_API:
            val->setError(EGL_BAD_PARAMETER);
            return false;  // Not supported by this implementation
        default:
            val->setError(EGL_BAD_PARAMETER);
            return false;
    }

    return true;
}

bool ValidatePresentationTimeANDROID(const ValidationContext *val,
                                     const Display *display,
                                     SurfaceID surfaceID,
                                     EGLnsecsANDROID time)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));

    if (!display->getExtensions().presentationTime)
    {
        // It is out of spec what happens when calling an extension function when the extension is
        // not available. EGL_BAD_DISPLAY seems like a reasonable error.
        val->setError(EGL_BAD_DISPLAY, "EGL_ANDROID_presentation_time is not available.");
        return false;
    }

    ANGLE_VALIDATION_TRY(ValidateSurface(val, display, surfaceID));

    return true;
}

bool ValidateSetBlobCacheFuncsANDROID(const ValidationContext *val,
                                      const Display *display,
                                      EGLSetBlobFuncANDROID set,
                                      EGLGetBlobFuncANDROID get)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));

    if (display->areBlobCacheFuncsSet())
    {
        val->setError(EGL_BAD_PARAMETER,
                      "Blob cache functions can only be set once in the lifetime of a Display");
        return false;
    }

    if (set == nullptr || get == nullptr)
    {
        val->setError(EGL_BAD_PARAMETER, "Blob cache callbacks cannot be null.");
        return false;
    }

    return true;
}

bool ValidateGetConfigAttrib(const ValidationContext *val,
                             const Display *display,
                             const Config *config,
                             EGLint attribute,
                             const EGLint *value)
{
    ANGLE_VALIDATION_TRY(ValidateConfig(val, display, config));
    ANGLE_TRY(ValidateConfigAttribute(val, display, static_cast<EGLAttrib>(attribute)));
    return true;
}

bool ValidateChooseConfig(const ValidationContext *val,
                          const Display *display,
                          const AttributeMap &attribs,
                          const EGLConfig *configs,
                          EGLint configSize,
                          const EGLint *numConfig)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));
    ANGLE_VALIDATION_TRY(ValidateConfigAttributes(val, display, attribs));

    if (numConfig == nullptr)
    {
        val->setError(EGL_BAD_PARAMETER, "num_config cannot be null.");
        return false;
    }

    return true;
}

bool ValidateGetConfigs(const ValidationContext *val,
                        const Display *display,
                        const EGLConfig *configs,
                        EGLint configSize,
                        const EGLint *numConfig)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));

    if (numConfig == nullptr)
    {
        val->setError(EGL_BAD_PARAMETER, "num_config cannot be null.");
        return false;
    }

    return true;
}

bool ValidateGetPlatformDisplay(const ValidationContext *val,
                                EGLenum platform,
                                const void *native_display,
                                const AttributeMap &attribMap)
{
    return ValidateGetPlatformDisplayCommon(val, platform, native_display, attribMap);
}

bool ValidateGetPlatformDisplayEXT(const ValidationContext *val,
                                   EGLenum platform,
                                   const void *native_display,
                                   const AttributeMap &attribMap)
{
    return ValidateGetPlatformDisplayCommon(val, platform, native_display, attribMap);
}

bool ValidateCreatePlatformWindowSurfaceEXT(const ValidationContext *val,
                                            const Display *display,
                                            const Config *configuration,
                                            const void *nativeWindow,
                                            const AttributeMap &attributes)
{
    if (!Display::GetClientExtensions().platformBase)
    {
        val->setError(EGL_BAD_ACCESS, "EGL_EXT_platform_base not supported");
        return false;
    }

    const void *actualNativeWindow =
        display->getImplementation()->getWindowSystem() == angle::NativeWindowSystem::X11
            ? *reinterpret_cast<const void *const *>(nativeWindow)
            : nativeWindow;

    return ValidateCreatePlatformWindowSurface(val, display, configuration, actualNativeWindow,
                                               attributes);
}

bool ValidateCreatePlatformPixmapSurfaceEXT(const ValidationContext *val,
                                            const Display *display,
                                            const Config *configuration,
                                            const void *nativePixmap,
                                            const AttributeMap &attributes)
{
    if (!Display::GetClientExtensions().platformBase)
    {
        val->setError(EGL_BAD_ACCESS, "EGL_EXT_platform_base not supported");
        return false;
    }

    ANGLE_VALIDATION_TRY(ValidateConfig(val, display, configuration));

    val->setError(EGL_BAD_DISPLAY, "ValidateCreatePlatformPixmapSurfaceEXT unimplemented.");
    return false;
}

bool ValidateProgramCacheGetAttribANGLE(const ValidationContext *val,
                                        const Display *display,
                                        EGLenum attrib)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));

    if (!display->getExtensions().programCacheControlANGLE)
    {
        val->setError(EGL_BAD_ACCESS, "Extension not supported");
        return false;
    }

    switch (attrib)
    {
        case EGL_PROGRAM_CACHE_KEY_LENGTH_ANGLE:
        case EGL_PROGRAM_CACHE_SIZE_ANGLE:
            break;

        default:
            val->setError(EGL_BAD_PARAMETER, "Invalid program cache attribute.");
            return false;
    }

    return true;
}

bool ValidateQuerySupportedCompressionRatesEXT(const ValidationContext *val,
                                               const Display *display,
                                               const Config *config,
                                               const EGLAttrib *attrib_list,
                                               const EGLint *rates,
                                               EGLint rate_size,
                                               const EGLint *num_rates)
{
    const AttributeMap &attributes = PackParam<const AttributeMap &>((const EGLint *)attrib_list);

    ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));
    if (!display->getExtensions().surfaceCompressionEXT)
    {
        val->setError(EGL_BAD_ACCESS, "EGL_EXT_surface_compression not supported");
        return false;
    }

    ANGLE_VALIDATION_TRY(ValidateConfig(val, display, config));
    attributes.initializeWithoutValidation();
    ANGLE_VALIDATION_TRY(ValidateCreateWindowSurfaceAttributes(val, display, config, attributes));

    if (rate_size < 0)
    {
        val->setError(EGL_BAD_PARAMETER, "rate_size cannot be negative.");
        return false;
    }

    if (rate_size > 0 && rates == nullptr)
    {
        val->setError(EGL_BAD_PARAMETER, "rates cannot be null when rate_size greater than 0.");
        return false;
    }

    if (num_rates == nullptr)
    {
        val->setError(EGL_BAD_PARAMETER, "num_rates cannot be null");
        return false;
    }

    return true;
}

bool ValidateProgramCacheQueryANGLE(const ValidationContext *val,
                                    const Display *display,
                                    EGLint index,
                                    const void *key,
                                    const EGLint *keysize,
                                    const void *binary,
                                    const EGLint *binarysize)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));

    if (!display->getExtensions().programCacheControlANGLE)
    {
        val->setError(EGL_BAD_ACCESS, "Extension not supported");
        return false;
    }

    if (index < 0 || index >= display->programCacheGetAttrib(EGL_PROGRAM_CACHE_SIZE_ANGLE))
    {
        val->setError(EGL_BAD_PARAMETER, "Program index out of range.");
        return false;
    }

    if (keysize == nullptr || binarysize == nullptr)
    {
        val->setError(EGL_BAD_PARAMETER, "keysize and binarysize must always be valid pointers.");
        return false;
    }

    if (binary && *keysize != static_cast<EGLint>(egl::BlobCache::kKeyLength))
    {
        val->setError(EGL_BAD_PARAMETER, "Invalid program key size.");
        return false;
    }

    if ((key == nullptr) != (binary == nullptr))
    {
        val->setError(EGL_BAD_PARAMETER, "key and binary must both be null or both non-null.");
        return false;
    }

    return true;
}

bool ValidateProgramCachePopulateANGLE(const ValidationContext *val,
                                       const Display *display,
                                       const void *key,
                                       EGLint keysize,
                                       const void *binary,
                                       EGLint binarysize)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));

    if (!display->getExtensions().programCacheControlANGLE)
    {
        val->setError(EGL_BAD_ACCESS, "Extension not supported");
        return false;
    }

    if (keysize != static_cast<EGLint>(egl::BlobCache::kKeyLength))
    {
        val->setError(EGL_BAD_PARAMETER, "Invalid program key size.");
        return false;
    }

    if (key == nullptr || binary == nullptr)
    {
        val->setError(EGL_BAD_PARAMETER, "null pointer in arguments.");
        return false;
    }

    // Upper bound for binarysize is arbitrary.
    if (binarysize <= 0 || binarysize > egl::kProgramCacheSizeAbsoluteMax)
    {
        val->setError(EGL_BAD_PARAMETER, "binarysize out of valid range.");
        return false;
    }

    return true;
}

bool ValidateProgramCacheResizeANGLE(const ValidationContext *val,
                                     const Display *display,
                                     EGLint limit,
                                     EGLint mode)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));

    if (!display->getExtensions().programCacheControlANGLE)
    {
        val->setError(EGL_BAD_ACCESS, "Extension not supported");
        return false;
    }

    if (limit < 0)
    {
        val->setError(EGL_BAD_PARAMETER, "limit must be non-negative.");
        return false;
    }

    switch (mode)
    {
        case EGL_PROGRAM_CACHE_RESIZE_ANGLE:
        case EGL_PROGRAM_CACHE_TRIM_ANGLE:
            break;

        default:
            val->setError(EGL_BAD_PARAMETER, "Invalid cache resize mode.");
            return false;
    }

    return true;
}

bool ValidateSurfaceAttrib(const ValidationContext *val,
                           const Display *display,
                           SurfaceID surfaceID,
                           EGLint attribute,
                           EGLint value)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));
    ANGLE_VALIDATION_TRY(ValidateSurface(val, display, surfaceID));

    const Surface *surface = display->getSurface(surfaceID);
    if (surface == EGL_NO_SURFACE)
    {
        val->setError(EGL_BAD_SURFACE, "Surface cannot be EGL_NO_SURFACE.");
        return false;
    }

    switch (attribute)
    {
        case EGL_MIPMAP_LEVEL:
            break;

        case EGL_MULTISAMPLE_RESOLVE:
            switch (value)
            {
                case EGL_MULTISAMPLE_RESOLVE_DEFAULT:
                    break;

                case EGL_MULTISAMPLE_RESOLVE_BOX:
                    if ((surface->getConfig()->surfaceType & EGL_MULTISAMPLE_RESOLVE_BOX_BIT) == 0)
                    {
                        val->setError(EGL_BAD_MATCH,
                                      "Surface does not support EGL_MULTISAMPLE_RESOLVE_BOX.");
                        return false;
                    }
                    break;

                default:
                    val->setError(EGL_BAD_ATTRIBUTE, "Invalid multisample resolve type.");
                    return false;
            }
            break;

        case EGL_SWAP_BEHAVIOR:
            switch (value)
            {
                case EGL_BUFFER_PRESERVED:
                    if ((surface->getConfig()->surfaceType & EGL_SWAP_BEHAVIOR_PRESERVED_BIT) == 0)
                    {
                        val->setError(EGL_BAD_MATCH,
                                      "Surface does not support EGL_SWAP_BEHAVIOR_PRESERVED.");
                        return false;
                    }
                    break;

                case EGL_BUFFER_DESTROYED:
                    break;

                default:
                    val->setError(EGL_BAD_ATTRIBUTE, "Invalid swap behaviour.");
                    return false;
            }
            break;

        case EGL_WIDTH:
        case EGL_HEIGHT:
            if (!display->getExtensions().windowFixedSize)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "EGL_WIDTH or EGL_HEIGHT cannot be set without "
                              "EGL_ANGLE_window_fixed_size support.");
                return false;
            }
            if (!surface->isFixedSize())
            {
                val->setError(EGL_BAD_MATCH,
                              "EGL_WIDTH or EGL_HEIGHT cannot be set without "
                              "EGL_FIXED_SIZE_ANGLE being enabled on the surface.");
                return false;
            }
            break;

        case EGL_TIMESTAMPS_ANDROID:
            if (!display->getExtensions().getFrameTimestamps &&
                !display->getExtensions().timestampSurfaceAttributeANGLE)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "EGL_TIMESTAMPS_ANDROID cannot be used without "
                              "EGL_ANDROID_get_frame_timestamps support.");
                return false;
            }
            switch (value)
            {
                case EGL_TRUE:
                case EGL_FALSE:
                    break;

                default:
                    val->setError(EGL_BAD_ATTRIBUTE, "Invalid value.");
                    return false;
            }
            break;

        case EGL_FRONT_BUFFER_AUTO_REFRESH_ANDROID:
            if (!display->getExtensions().frontBufferAutoRefreshANDROID)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "EGL_FRONT_BUFFER_AUTO_REFRESH_ANDROID cannot be used without "
                              "EGL_ANDROID_front_buffer_auto_refresh support.");
                return false;
            }
            switch (value)
            {
                case EGL_TRUE:
                case EGL_FALSE:
                    break;

                default:
                    val->setError(EGL_BAD_ATTRIBUTE, "Invalid value.");
                    return false;
            }
            break;

        case EGL_RENDER_BUFFER:
            if (value != EGL_BACK_BUFFER && value != EGL_SINGLE_BUFFER)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "EGL_RENDER_BUFFER must be EGL_BACK_BUFFER or EGL_SINGLE_BUFFER.");
                return false;
            }

            if (value == EGL_SINGLE_BUFFER)
            {
                if (!display->getExtensions().mutableRenderBufferKHR)
                {
                    val->setError(
                        EGL_BAD_ATTRIBUTE,
                        "Attribute EGL_RENDER_BUFFER requires EGL_KHR_mutable_render_buffer.");
                    return false;
                }

                if ((surface->getConfig()->surfaceType & EGL_MUTABLE_RENDER_BUFFER_BIT_KHR) == 0)
                {
                    val->setError(EGL_BAD_MATCH,
                                  "EGL_RENDER_BUFFER requires the surface type bit "
                                  "EGL_MUTABLE_RENDER_BUFFER_BIT_KHR.");
                    return false;
                }
            }
            break;

        default:
            val->setError(EGL_BAD_ATTRIBUTE, "Invalid surface attribute: 0x%04X", attribute);
            return false;
    }

    return true;
}

bool ValidateQuerySurface(const ValidationContext *val,
                          const Display *display,
                          SurfaceID surfaceID,
                          EGLint attribute,
                          const EGLint *value)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));
    ANGLE_VALIDATION_TRY(ValidateSurface(val, display, surfaceID));

    const Surface *surface = display->getSurface(surfaceID);
    if (surface == EGL_NO_SURFACE)
    {
        val->setError(EGL_BAD_SURFACE, "Surface cannot be EGL_NO_SURFACE.");
        return false;
    }

    switch (attribute)
    {
        case EGL_GL_COLORSPACE:
        case EGL_VG_ALPHA_FORMAT:
        case EGL_VG_COLORSPACE:
        case EGL_CONFIG_ID:
        case EGL_HEIGHT:
        case EGL_HORIZONTAL_RESOLUTION:
        case EGL_LARGEST_PBUFFER:
        case EGL_MIPMAP_TEXTURE:
        case EGL_MIPMAP_LEVEL:
        case EGL_MULTISAMPLE_RESOLVE:
        case EGL_PIXEL_ASPECT_RATIO:
        case EGL_RENDER_BUFFER:
        case EGL_SWAP_BEHAVIOR:
        case EGL_TEXTURE_FORMAT:
        case EGL_TEXTURE_TARGET:
        case EGL_VERTICAL_RESOLUTION:
        case EGL_WIDTH:
            break;

        case EGL_POST_SUB_BUFFER_SUPPORTED_NV:
            if (!display->getExtensions().postSubBuffer)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "EGL_POST_SUB_BUFFER_SUPPORTED_NV cannot be used "
                              "without EGL_ANGLE_surface_orientation support.");
                return false;
            }
            break;

        case EGL_FIXED_SIZE_ANGLE:
            if (!display->getExtensions().windowFixedSize)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "EGL_FIXED_SIZE_ANGLE cannot be used without "
                              "EGL_ANGLE_window_fixed_size support.");
                return false;
            }
            break;

        case EGL_SURFACE_ORIENTATION_ANGLE:
            if (!display->getExtensions().surfaceOrientation)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "EGL_SURFACE_ORIENTATION_ANGLE cannot be "
                              "queried without "
                              "EGL_ANGLE_surface_orientation support.");
                return false;
            }
            break;

        case EGL_DIRECT_COMPOSITION_ANGLE:
            if (!display->getExtensions().directComposition)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "EGL_DIRECT_COMPOSITION_ANGLE cannot be "
                              "used without "
                              "EGL_ANGLE_direct_composition support.");
                return false;
            }
            break;

        case EGL_ROBUST_RESOURCE_INITIALIZATION_ANGLE:
            if (!display->getExtensions().robustResourceInitializationANGLE)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "EGL_ROBUST_RESOURCE_INITIALIZATION_ANGLE cannot be "
                              "used without EGL_ANGLE_robust_resource_initialization "
                              "support.");
                return false;
            }
            break;

        case EGL_TIMESTAMPS_ANDROID:
            if (!display->getExtensions().getFrameTimestamps &&
                !display->getExtensions().timestampSurfaceAttributeANGLE)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "EGL_TIMESTAMPS_ANDROID cannot be used without "
                              "EGL_ANDROID_get_frame_timestamps support.");
                return false;
            }
            break;

        case EGL_BUFFER_AGE_EXT:
        {
            if (!display->getExtensions().bufferAgeEXT)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "EGL_BUFFER_AGE_EXT cannot be used without "
                              "EGL_EXT_buffer_age support.");
                return false;
            }
            gl::Context *context = val->eglThread->getContext();
            if ((context == nullptr) || (context->getCurrentDrawSurface() != surface))
            {
                val->setError(EGL_BAD_SURFACE,
                              "The surface must be current to the current context "
                              "in order to query buffer age per extension "
                              "EGL_EXT_buffer_age.");
                return false;
            }
        }
        break;

        case EGL_BITMAP_PITCH_KHR:
        case EGL_BITMAP_ORIGIN_KHR:
        case EGL_BITMAP_PIXEL_RED_OFFSET_KHR:
        case EGL_BITMAP_PIXEL_GREEN_OFFSET_KHR:
        case EGL_BITMAP_PIXEL_BLUE_OFFSET_KHR:
        case EGL_BITMAP_PIXEL_ALPHA_OFFSET_KHR:
        case EGL_BITMAP_PIXEL_LUMINANCE_OFFSET_KHR:
        case EGL_BITMAP_PIXEL_SIZE_KHR:
            if (!display->getExtensions().lockSurface3KHR)
            {
                val->setError(EGL_BAD_ATTRIBUTE, "EGL_KHR_lock_surface3 is not supported.");
                return false;
            }
            break;

        case EGL_PROTECTED_CONTENT_EXT:
            if (!display->getExtensions().protectedContentEXT)
            {
                val->setError(EGL_BAD_ATTRIBUTE, "EGL_EXT_protected_content not supported");
                return false;
            }
            break;

        case EGL_SURFACE_COMPRESSION_EXT:
            if (!display->getExtensions().surfaceCompressionEXT)
            {
                val->setError(EGL_BAD_ATTRIBUTE, "EGL_EXT_surface_compression not supported");
                return false;
            }
            break;

        default:
            val->setError(EGL_BAD_ATTRIBUTE, "Invalid query surface attribute: 0x%04X", attribute);
            return false;
    }

    return true;
}

bool ValidateQueryContext(const ValidationContext *val,
                          const Display *display,
                          gl::ContextID contextID,
                          EGLint attribute,
                          const EGLint *value)
{
    ANGLE_VALIDATION_TRY(ValidateContext(val, display, contextID));

    switch (attribute)
    {
        case EGL_CONFIG_ID:
        case EGL_CONTEXT_CLIENT_TYPE:
        case EGL_CONTEXT_MAJOR_VERSION:
        case EGL_CONTEXT_MINOR_VERSION:
        case EGL_RENDER_BUFFER:
            break;

        case EGL_ROBUST_RESOURCE_INITIALIZATION_ANGLE:
            if (!display->getExtensions().robustResourceInitializationANGLE)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "EGL_ROBUST_RESOURCE_INITIALIZATION_ANGLE cannot be "
                              "used without EGL_ANGLE_robust_resource_initialization "
                              "support.");
                return false;
            }
            break;

        case EGL_CONTEXT_PRIORITY_LEVEL_IMG:
            if (!display->getExtensions().contextPriority)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "Attribute EGL_CONTEXT_PRIORITY_LEVEL_IMG requires "
                              "extension EGL_IMG_context_priority.");
                return false;
            }
            break;

        case EGL_PROTECTED_CONTENT_EXT:
            if (!display->getExtensions().protectedContentEXT)
            {
                val->setError(EGL_BAD_ATTRIBUTE, "EGL_EXT_protected_content not supported");
                return false;
            }
            break;

        case EGL_CONTEXT_MEMORY_USAGE_ANGLE:
            if (!display->getExtensions().memoryUsageReportANGLE)
            {
                val->setError(EGL_BAD_ATTRIBUTE,
                              "Attribute EGL_CONTEXT_MEMORY_USAGE_ANGLE requires "
                              "EGL_ANGLE_memory_usage_report.");
                return false;
            }
            break;

        default:
            val->setError(EGL_BAD_ATTRIBUTE, "Invalid context attribute: 0x%04X", attribute);
            return false;
    }

    return true;
}

bool ValidateDebugMessageControlKHR(const ValidationContext *val,
                                    EGLDEBUGPROCKHR callback,
                                    const AttributeMap &attribs)
{
    const ClientExtensions &clientExtensions = Display::GetClientExtensions();
    if (!clientExtensions.debug)
    {
        val->setError(EGL_BAD_ACCESS, "EGL_KHR_debug extension is not available.");
        return false;
    }

    attribs.initializeWithoutValidation();

    for (const auto &attrib : attribs)
    {
        switch (attrib.first)
        {
            case EGL_DEBUG_MSG_CRITICAL_KHR:
            case EGL_DEBUG_MSG_ERROR_KHR:
            case EGL_DEBUG_MSG_WARN_KHR:
            case EGL_DEBUG_MSG_INFO_KHR:
                if (attrib.second != EGL_TRUE && attrib.second != EGL_FALSE)
                {
                    val->setError(EGL_BAD_ATTRIBUTE,
                                  "message controls must be EGL_TRUE or EGL_FALSE.");
                    return false;
                }
                break;
        }
    }

    return true;
}

bool ValidateQueryDebugKHR(const ValidationContext *val, EGLint attribute, const EGLAttrib *value)
{
    const ClientExtensions &clientExtensions = Display::GetClientExtensions();
    if (!clientExtensions.debug)
    {
        val->setError(EGL_BAD_ACCESS, "EGL_KHR_debug extension is not available.");
        return false;
    }

    switch (attribute)
    {
        case EGL_DEBUG_MSG_CRITICAL_KHR:
        case EGL_DEBUG_MSG_ERROR_KHR:
        case EGL_DEBUG_MSG_WARN_KHR:
        case EGL_DEBUG_MSG_INFO_KHR:
        case EGL_DEBUG_CALLBACK_KHR:
            break;

        default:
            val->setError(EGL_BAD_ATTRIBUTE, "Unknown attribute: 0x%04X", attribute);
            return false;
    }

    return true;
}

bool ValidateLabelObjectKHR(const ValidationContext *val,
                            const Display *display,
                            ObjectType objectType,
                            EGLObjectKHR object,
                            EGLLabelKHR label)
{
    const ClientExtensions &clientExtensions = Display::GetClientExtensions();
    if (!clientExtensions.debug)
    {
        val->setError(EGL_BAD_ACCESS, "EGL_KHR_debug extension is not available.");
        return false;
    }

    const LabeledObject *labeledObject = nullptr;
    ANGLE_VALIDATION_TRY(ValidateLabeledObject(val, display, objectType, object, &labeledObject));

    return true;
}

bool ValidateGetCompositorTimingSupportedANDROID(const ValidationContext *val,
                                                 const Display *display,
                                                 SurfaceID surfaceID,
                                                 CompositorTiming name)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));

    if (!display->getExtensions().getFrameTimestamps)
    {
        val->setError(EGL_BAD_DISPLAY,
                      "EGL_ANDROID_get_frame_timestamps extension is not available.");
        return false;
    }

    ANGLE_VALIDATION_TRY(ValidateSurface(val, display, surfaceID));

    if (!ValidCompositorTimingName(name))
    {
        val->setError(EGL_BAD_PARAMETER, "invalid timing name.");
        return false;
    }

    return true;
}

bool ValidateGetCompositorTimingANDROID(const ValidationContext *val,
                                        const Display *display,
                                        SurfaceID surfaceID,
                                        EGLint numTimestamps,
                                        const EGLint *names,
                                        const EGLnsecsANDROID *values)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));

    if (!display->getExtensions().getFrameTimestamps)
    {
        val->setError(EGL_BAD_DISPLAY,
                      "EGL_ANDROID_get_frame_timestamps extension is not available.");
        return false;
    }

    ANGLE_VALIDATION_TRY(ValidateSurface(val, display, surfaceID));

    if (names == nullptr && numTimestamps > 0)
    {
        val->setError(EGL_BAD_PARAMETER, "names is NULL.");
        return false;
    }

    if (values == nullptr && numTimestamps > 0)
    {
        val->setError(EGL_BAD_PARAMETER, "values is NULL.");
        return false;
    }

    if (numTimestamps < 0)
    {
        val->setError(EGL_BAD_PARAMETER, "numTimestamps must be at least 0.");
        return false;
    }

    for (EGLint i = 0; i < numTimestamps; i++)
    {
        CompositorTiming name = FromEGLenum<CompositorTiming>(names[i]);

        if (!ValidCompositorTimingName(name))
        {
            val->setError(EGL_BAD_PARAMETER, "invalid compositor timing.");
            return false;
        }

        const Surface *surface = display->getSurface(surfaceID);
        if (!surface->getSupportedCompositorTimings().test(name))
        {
            val->setError(EGL_BAD_PARAMETER, "compositor timing not supported by surface.");
            return false;
        }
    }

    return true;
}

bool ValidateGetNextFrameIdANDROID(const ValidationContext *val,
                                   const Display *display,
                                   SurfaceID surfaceID,
                                   const EGLuint64KHR *frameId)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));

    if (!display->getExtensions().getFrameTimestamps)
    {
        val->setError(EGL_BAD_DISPLAY,
                      "EGL_ANDROID_get_frame_timestamps extension is not available.");
        return false;
    }

    ANGLE_VALIDATION_TRY(ValidateSurface(val, display, surfaceID));

    if (frameId == nullptr)
    {
        val->setError(EGL_BAD_PARAMETER, "frameId is NULL.");
        return false;
    }

    return true;
}

bool ValidateGetFrameTimestampSupportedANDROID(const ValidationContext *val,
                                               const Display *display,
                                               SurfaceID surfaceID,
                                               Timestamp timestamp)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));

    if (!display->getExtensions().getFrameTimestamps)
    {
        val->setError(EGL_BAD_DISPLAY,
                      "EGL_ANDROID_get_frame_timestamps extension is not available.");
        return false;
    }

    ANGLE_VALIDATION_TRY(ValidateSurface(val, display, surfaceID));

    if (!ValidTimestampType(timestamp))
    {
        val->setError(EGL_BAD_PARAMETER, "invalid timestamp type.");
        return false;
    }

    return true;
}

bool ValidateGetFrameTimestampsANDROID(const ValidationContext *val,
                                       const Display *display,
                                       SurfaceID surfaceID,
                                       EGLuint64KHR frameId,
                                       EGLint numTimestamps,
                                       const EGLint *timestamps,
                                       const EGLnsecsANDROID *values)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));

    if (!display->getExtensions().getFrameTimestamps)
    {
        val->setError(EGL_BAD_DISPLAY,
                      "EGL_ANDROID_get_frame_timestamps extension is not available.");
        return false;
    }

    ANGLE_VALIDATION_TRY(ValidateSurface(val, display, surfaceID));

    const Surface *surface = display->getSurface(surfaceID);
    if (!surface->isTimestampsEnabled())
    {
        val->setError(EGL_BAD_SURFACE, "timestamp collection is not enabled for this surface.");
        return false;
    }

    if (timestamps == nullptr && numTimestamps > 0)
    {
        val->setError(EGL_BAD_PARAMETER, "timestamps is NULL.");
        return false;
    }

    if (values == nullptr && numTimestamps > 0)
    {
        val->setError(EGL_BAD_PARAMETER, "values is NULL.");
        return false;
    }

    if (numTimestamps < 0)
    {
        val->setError(EGL_BAD_PARAMETER, "numTimestamps must be at least 0.");
        return false;
    }

    for (EGLint i = 0; i < numTimestamps; i++)
    {
        Timestamp timestamp = FromEGLenum<Timestamp>(timestamps[i]);

        if (!ValidTimestampType(timestamp))
        {
            val->setError(EGL_BAD_PARAMETER, "invalid timestamp type.");
            return false;
        }

        if (!surface->getSupportedTimestamps().test(timestamp))
        {
            val->setError(EGL_BAD_PARAMETER, "timestamp not supported by surface.");
            return false;
        }
    }

    return true;
}

bool ValidateQueryStringiANGLE(const ValidationContext *val,
                               const Display *display,
                               EGLint name,
                               EGLint index)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));

    if (!Display::GetClientExtensions().featureControlANGLE)
    {
        val->setError(EGL_BAD_DISPLAY, "EGL_ANGLE_feature_control extension is not available.");
        return false;
    }

    if (index < 0)
    {
        val->setError(EGL_BAD_PARAMETER, "index is negative.");
        return false;
    }

    switch (name)
    {
        case EGL_FEATURE_NAME_ANGLE:
        case EGL_FEATURE_CATEGORY_ANGLE:
        case EGL_FEATURE_STATUS_ANGLE:
            break;
        default:
            val->setError(EGL_BAD_PARAMETER, "name is not valid.");
            return false;
    }

    if (static_cast<size_t>(index) >= display->getFeatures().size())
    {
        val->setError(EGL_BAD_PARAMETER, "index is too big.");
        return false;
    }

    return true;
}

bool ValidateQueryDisplayAttribEXT(const ValidationContext *val,
                                   const Display *display,
                                   const EGLint attribute,
                                   const EGLAttrib *value)
{
    ANGLE_VALIDATION_TRY(ValidateQueryDisplayAttribBase(val, display, attribute));
    return true;
}

bool ValidateQueryDisplayAttribANGLE(const ValidationContext *val,
                                     const Display *display,
                                     const EGLint attribute,
                                     const EGLAttrib *value)
{
    ANGLE_VALIDATION_TRY(ValidateQueryDisplayAttribBase(val, display, attribute));
    return true;
}

bool ValidateGetNativeClientBufferANDROID(const ValidationContext *val,
                                          const AHardwareBuffer *buffer)
{
    // No extension check is done because no display is passed to eglGetNativeClientBufferANDROID
    // despite it being a display extension.  No display is needed for the implementation though.
    if (buffer == nullptr)
    {
        val->setError(EGL_BAD_PARAMETER, "NULL buffer.");
        return false;
    }

    return true;
}

bool ValidateCreateNativeClientBufferANDROID(const ValidationContext *val,
                                             const egl::AttributeMap &attribMap)
{
    attribMap.initializeWithoutValidation();

    if (attribMap.isEmpty() || attribMap.begin()->second == EGL_NONE)
    {
        val->setError(EGL_BAD_PARAMETER, "invalid attribute list.");
        return false;
    }

    int width     = attribMap.getAsInt(EGL_WIDTH, 0);
    int height    = attribMap.getAsInt(EGL_HEIGHT, 0);
    int redSize   = attribMap.getAsInt(EGL_RED_SIZE, 0);
    int greenSize = attribMap.getAsInt(EGL_GREEN_SIZE, 0);
    int blueSize  = attribMap.getAsInt(EGL_BLUE_SIZE, 0);
    int alphaSize = attribMap.getAsInt(EGL_ALPHA_SIZE, 0);
    int usage     = attribMap.getAsInt(EGL_NATIVE_BUFFER_USAGE_ANDROID, 0);

    for (AttributeMap::const_iterator attributeIter = attribMap.begin();
         attributeIter != attribMap.end(); attributeIter++)
    {
        EGLAttrib attribute = attributeIter->first;
        switch (attribute)
        {
            case EGL_WIDTH:
            case EGL_HEIGHT:
                // Validation done after the switch statement
                break;
            case EGL_RED_SIZE:
            case EGL_GREEN_SIZE:
            case EGL_BLUE_SIZE:
            case EGL_ALPHA_SIZE:
                if (redSize < 0 || greenSize < 0 || blueSize < 0 || alphaSize < 0)
                {
                    val->setError(EGL_BAD_PARAMETER, "incorrect channel size requested");
                    return false;
                }
                break;
            case EGL_NATIVE_BUFFER_USAGE_ANDROID:
                // The buffer must be used for either a texture or a renderbuffer.
                if ((usage & ~(EGL_NATIVE_BUFFER_USAGE_PROTECTED_BIT_ANDROID |
                               EGL_NATIVE_BUFFER_USAGE_RENDERBUFFER_BIT_ANDROID |
                               EGL_NATIVE_BUFFER_USAGE_TEXTURE_BIT_ANDROID)) != 0)
                {
                    val->setError(EGL_BAD_PARAMETER, "invalid usage flag");
                    return false;
                }
                break;
            case EGL_NONE:
                break;
            default:
                val->setError(EGL_BAD_ATTRIBUTE, "invalid attribute");
                return false;
        }
    }

    // Validate EGL_WIDTH and EGL_HEIGHT values passed in. Done here to account
    // for the case where EGL_WIDTH and EGL_HEIGHT were not part of the attribute list.
    if (width <= 0 || height <= 0)
    {
        val->setError(EGL_BAD_PARAMETER, "incorrect buffer dimensions requested");
        return false;
    }

    if (gl::GetAndroidHardwareBufferFormatFromChannelSizes(attribMap) == 0)
    {
        val->setError(EGL_BAD_PARAMETER, "unsupported format");
        return false;
    }
    return true;
}

bool ValidateCopyMetalSharedEventANGLE(const ValidationContext *val,
                                       const Display *display,
                                       SyncID sync)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));

    if (!display->getExtensions().mtlSyncSharedEventANGLE)
    {
        val->setError(EGL_BAD_DISPLAY, "EGL_ANGLE_metal_shared_event_sync is not available.");
        return false;
    }

    ANGLE_VALIDATION_TRY(ValidateSync(val, display, sync));

    return true;
}

bool ValidateDupNativeFenceFDANDROID(const ValidationContext *val,
                                     const Display *display,
                                     SyncID sync)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));

    if (!display->getExtensions().nativeFenceSyncANDROID)
    {
        val->setError(EGL_BAD_DISPLAY, "EGL_ANDROID_native_fence_sync extension is not available.");
        return false;
    }

    ANGLE_VALIDATION_TRY(ValidateSync(val, display, sync));

    return true;
}

bool ValidateSwapBuffersWithFrameTokenANGLE(const ValidationContext *val,
                                            const Display *display,
                                            SurfaceID surfaceID,
                                            EGLFrameTokenANGLE frametoken)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));

    if (!display->getExtensions().swapWithFrameToken)
    {
        val->setError(EGL_BAD_DISPLAY, "EGL_ANGLE_swap_buffers_with_frame_token is not available.");
        return false;
    }

    ANGLE_VALIDATION_TRY(ValidateSurface(val, display, surfaceID));

    return true;
}

bool ValidatePrepareSwapBuffersANGLE(const ValidationContext *val,
                                     const Display *display,
                                     SurfaceID surfaceID)
{
    return ValidateSwapBuffers(val, display, surfaceID);
}

bool ValidateSignalSyncKHR(const ValidationContext *val,
                           const Display *display,
                           SyncID sync,
                           EGLenum mode)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));

    ANGLE_VALIDATION_TRY(ValidateSync(val, display, sync));

    const Sync *syncObj = display->getSync(sync);

    if (syncObj->getType() == EGL_SYNC_REUSABLE_KHR)
    {
        if (!display->getExtensions().reusableSyncKHR)
        {
            val->setError(EGL_BAD_MATCH, "EGL_KHR_reusable_sync extension is not available.");
            return false;
        }

        if ((mode != EGL_SIGNALED_KHR) && (mode != EGL_UNSIGNALED_KHR))
        {
            val->setError(EGL_BAD_PARAMETER, "eglSignalSyncKHR invalid mode.");
            return false;
        }

        return true;
    }

    val->setError(EGL_BAD_MATCH);
    return false;
}

bool ValidateQuerySurfacePointerANGLE(const ValidationContext *val,
                                      const Display *display,
                                      SurfaceID surfaceID,
                                      EGLint attribute,
                                      void *const *value)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));

    if (!display->getExtensions().querySurfacePointer)
    {
        val->setError(EGL_BAD_ACCESS);
        return false;
    }

    ANGLE_VALIDATION_TRY(ValidateSurface(val, display, surfaceID));

    // validate the attribute parameter
    switch (attribute)
    {
        case EGL_D3D_TEXTURE_2D_SHARE_HANDLE_ANGLE:
            if (!display->getExtensions().surfaceD3DTexture2DShareHandle)
            {
                val->setError(EGL_BAD_ATTRIBUTE);
                return false;
            }
            break;
        case EGL_DXGI_KEYED_MUTEX_ANGLE:
            if (!display->getExtensions().keyedMutex)
            {
                val->setError(EGL_BAD_ATTRIBUTE);
                return false;
            }
            break;
        default:
            val->setError(EGL_BAD_ATTRIBUTE);
            return false;
    }

    return true;
}

bool ValidatePostSubBufferNV(const ValidationContext *val,
                             const Display *display,
                             SurfaceID surfaceID,
                             EGLint x,
                             EGLint y,
                             EGLint width,
                             EGLint height)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));

    if (!display->getExtensions().postSubBuffer)
    {
        val->setError(EGL_BAD_ACCESS);
        return false;
    }

    if (x < 0 || y < 0 || width < 0 || height < 0)
    {
        val->setError(EGL_BAD_PARAMETER);
        return false;
    }

    ANGLE_VALIDATION_TRY(ValidateSurface(val, display, surfaceID));

    if (display->isDeviceLost())
    {
        val->setError(EGL_CONTEXT_LOST);
        return false;
    }

    return true;
}

bool ValidateQueryDeviceAttribEXT(const ValidationContext *val,
                                  const Device *device,
                                  EGLint attribute,
                                  const EGLAttrib *value)
{
    ANGLE_VALIDATION_TRY(ValidateDevice(val, device));

    if (!Display::GetClientExtensions().deviceQueryEXT)
    {
        val->setError(EGL_BAD_ACCESS, "EGL_EXT_device_query not supported.");
        return false;
    }

    // validate the attribute parameter
    switch (attribute)
    {
        case EGL_D3D11_DEVICE_ANGLE:
            if (!device->getExtensions().deviceD3D11)
            {
                val->setError(EGL_BAD_ATTRIBUTE);
                return false;
            }
            break;
        case EGL_D3D9_DEVICE_ANGLE:
            if (!device->getExtensions().deviceD3D9)
            {
                val->setError(EGL_BAD_ATTRIBUTE);
                return false;
            }
            break;
        case EGL_METAL_DEVICE_ANGLE:
            if (!device->getExtensions().deviceMetal)
            {
                val->setError(EGL_BAD_ATTRIBUTE);
                return false;
            }
            break;
        case EGL_VULKAN_VERSION_ANGLE:
        case EGL_VULKAN_INSTANCE_ANGLE:
        case EGL_VULKAN_INSTANCE_EXTENSIONS_ANGLE:
        case EGL_VULKAN_PHYSICAL_DEVICE_ANGLE:
        case EGL_VULKAN_DEVICE_ANGLE:
        case EGL_VULKAN_DEVICE_EXTENSIONS_ANGLE:
        case EGL_VULKAN_FEATURES_ANGLE:
        case EGL_VULKAN_QUEUE_ANGLE:
        case EGL_VULKAN_QUEUE_FAMILIY_INDEX_ANGLE:
        case EGL_VULKAN_GET_INSTANCE_PROC_ADDR:
            if (!device->getExtensions().deviceVulkan)
            {
                val->setError(EGL_BAD_ATTRIBUTE);
                return false;
            }
            break;
        case EGL_CGL_CONTEXT_ANGLE:
        case EGL_CGL_PIXEL_FORMAT_ANGLE:
            if (!device->getExtensions().deviceCGL)
            {
                val->setError(EGL_BAD_ATTRIBUTE);
                return false;
            }
            break;
        default:
            val->setError(EGL_BAD_ATTRIBUTE);
            return false;
    }
    return true;
}

bool ValidateQueryDeviceStringEXT(const ValidationContext *val, const Device *device, EGLint name)
{
    ANGLE_VALIDATION_TRY(ValidateDevice(val, device));
    return true;
}

bool ValidateReleaseHighPowerGPUANGLE(const ValidationContext *val,
                                      const Display *display,
                                      gl::ContextID contextID)
{
    ANGLE_VALIDATION_TRY(ValidateContext(val, display, contextID));
    return true;
}

bool ValidateReacquireHighPowerGPUANGLE(const ValidationContext *val,
                                        const Display *display,
                                        gl::ContextID contextID)
{
    ANGLE_VALIDATION_TRY(ValidateContext(val, display, contextID));
    return true;
}

bool ValidateHandleGPUSwitchANGLE(const ValidationContext *val, const Display *display)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));
    return true;
}

bool ValidateForceGPUSwitchANGLE(const ValidationContext *val,
                                 const Display *display,
                                 EGLint gpuIDHigh,
                                 EGLint gpuIDLow)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));
    return true;
}

bool ValidateWaitUntilWorkScheduledANGLE(const ValidationContext *val, const Display *display)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));
    return true;
}

bool ValidateGetCurrentDisplay(const ValidationContext *val)
{
    return true;
}

bool ValidateGetCurrentSurface(const ValidationContext *val, EGLint readdraw)
{
    switch (readdraw)
    {
        case EGL_READ:
        case EGL_DRAW:
            return true;

        default:
            val->setError(EGL_BAD_PARAMETER, "Invalid surface type");
            return false;
    }
}

bool ValidateGetDisplay(const ValidationContext *val, EGLNativeDisplayType display_id)
{
    return true;
}

bool ValidateGetError(const ValidationContext *val)
{
    return true;
}

bool ValidateGetProcAddress(const ValidationContext *val, const char *procname)
{
    return true;
}

bool ValidateQueryString(const ValidationContext *val, const Display *dpyPacked, EGLint name)
{
    // The only situation where EGL_NO_DISPLAY is allowed is when querying
    // EGL_EXTENSIONS or EGL_VERSION.
    const bool canQueryWithoutDisplay = (name == EGL_VERSION || name == EGL_EXTENSIONS);

    if (dpyPacked != nullptr || !canQueryWithoutDisplay)
    {
        ANGLE_VALIDATION_TRY(ValidateDisplay(val, dpyPacked));
    }

    switch (name)
    {
        case EGL_CLIENT_APIS:
        case EGL_EXTENSIONS:
        case EGL_VENDOR:
        case EGL_VERSION:
            break;
        default:
            val->setError(EGL_BAD_PARAMETER);
            return false;
    }
    return true;
}

bool ValidateWaitGL(const ValidationContext *val)
{
    if (val->eglThread->getDisplay() == nullptr)
    {
        // EGL spec says this about eglWaitGL -
        //    eglWaitGL is ignored if there is no current EGL rendering context for OpenGL ES.
        return true;
    }

    ANGLE_VALIDATION_TRY(ValidateDisplay(val, val->eglThread->getDisplay()));
    return true;
}

bool ValidateQueryAPI(const ValidationContext *val)
{
    return true;
}

bool ValidateReleaseThread(const ValidationContext *val)
{
    return true;
}

bool ValidateWaitClient(const ValidationContext *val)
{
    if (val->eglThread->getDisplay() == nullptr)
    {
        // EGL spec says this about eglWaitClient -
        //    If there is no current context for the current rendering API,
        //    the function has no effect but still returns EGL_TRUE.
        return true;
    }

    ANGLE_VALIDATION_TRY(ValidateDisplay(val, val->eglThread->getDisplay()));
    return true;
}

bool ValidateGetCurrentContext(const ValidationContext *val)
{
    return true;
}

bool ValidateCreatePlatformPixmapSurface(const ValidationContext *val,
                                         const Display *dpyPacked,
                                         const Config *configPacked,
                                         const void *native_pixmap,
                                         const AttributeMap &attrib_listPacked)
{
    EGLNativePixmapType nativePixmap =
        reinterpret_cast<EGLNativePixmapType>(const_cast<void *>(native_pixmap));
    return ValidateCreatePixmapSurface(val, dpyPacked, configPacked, nativePixmap,
                                       attrib_listPacked);
}

bool ValidateCreatePlatformWindowSurface(const ValidationContext *val,
                                         const Display *dpyPacked,
                                         const Config *configPacked,
                                         const void *native_window,
                                         const AttributeMap &attrib_listPacked)
{
    EGLNativeWindowType nativeWindow =
        reinterpret_cast<EGLNativeWindowType>(const_cast<void *>(native_window));
    return ValidateCreateWindowSurface(val, dpyPacked, configPacked, nativeWindow,
                                       attrib_listPacked);
}

bool ValidateLockSurfaceKHR(const ValidationContext *val,
                            const egl::Display *dpy,
                            SurfaceID surfaceID,
                            const AttributeMap &attributes)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, dpy));
    ANGLE_VALIDATION_TRY(ValidateSurface(val, dpy, surfaceID));

    if (!dpy->getExtensions().lockSurface3KHR)
    {
        val->setError(EGL_BAD_ACCESS);
        return false;
    }

    const Surface *surface = dpy->getSurface(surfaceID);
    if (surface->isLocked())
    {
        val->setError(EGL_BAD_ACCESS);
        return false;
    }

    if ((surface->getConfig()->surfaceType & EGL_LOCK_SURFACE_BIT_KHR) == false)
    {
        val->setError(EGL_BAD_ACCESS, "Config does not support EGL_LOCK_SURFACE_BIT");
        return false;
    }

    if (surface->isCurrentOnAnyContext())
    {
        val->setError(EGL_BAD_ACCESS,
                      "Surface cannot be current to a context for eglLockSurface()");
        return false;
    }

    if (surface->hasProtectedContent())
    {
        val->setError(EGL_BAD_ACCESS, "Surface cannot be protected content for eglLockSurface()");
        return false;
    }

    attributes.initializeWithoutValidation();

    for (const auto &attributeIter : attributes)
    {
        EGLAttrib attribute = attributeIter.first;
        EGLAttrib value     = attributeIter.second;

        switch (attribute)
        {
            case EGL_MAP_PRESERVE_PIXELS_KHR:
                if (!((value == EGL_FALSE) || (value == EGL_TRUE)))
                {
                    val->setError(EGL_BAD_ATTRIBUTE, "Invalid EGL_MAP_PRESERVE_PIXELS_KHR value");
                    return false;
                }
                break;
            case EGL_LOCK_USAGE_HINT_KHR:
                if ((value & (EGL_READ_SURFACE_BIT_KHR | EGL_WRITE_SURFACE_BIT_KHR)) != value)
                {
                    val->setError(EGL_BAD_ATTRIBUTE, "Invalid EGL_LOCK_USAGE_HINT_KHR value");
                    return false;
                }
                break;
            default:
                val->setError(EGL_BAD_ATTRIBUTE, "Invalid lock surface attribute");
                return false;
        }
    }

    return true;
}

bool ValidateQuerySurface64KHR(const ValidationContext *val,
                               const egl::Display *dpy,
                               SurfaceID surfaceID,
                               EGLint attribute,
                               const EGLAttribKHR *value)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, dpy));
    ANGLE_VALIDATION_TRY(ValidateSurface(val, dpy, surfaceID));

    if (!dpy->getExtensions().lockSurface3KHR)
    {
        val->setError(EGL_BAD_ACCESS);
        return false;
    }

    switch (attribute)
    {
        case EGL_BITMAP_PITCH_KHR:
        case EGL_BITMAP_ORIGIN_KHR:
        case EGL_BITMAP_PIXEL_RED_OFFSET_KHR:
        case EGL_BITMAP_PIXEL_GREEN_OFFSET_KHR:
        case EGL_BITMAP_PIXEL_BLUE_OFFSET_KHR:
        case EGL_BITMAP_PIXEL_ALPHA_OFFSET_KHR:
        case EGL_BITMAP_PIXEL_LUMINANCE_OFFSET_KHR:
        case EGL_BITMAP_PIXEL_SIZE_KHR:
        case EGL_BITMAP_POINTER_KHR:
            break;
        default:
        {
            EGLint querySurfaceValue;
            ANGLE_VALIDATION_TRY(
                ValidateQuerySurface(val, dpy, surfaceID, attribute, &querySurfaceValue));
        }
        break;
    }

    if (value == nullptr)
    {
        val->setError(EGL_BAD_PARAMETER, "value is NULL.");
        return false;
    }

    // EGL_KHR_lock_surface3
    //  If <attribute> is either EGL_BITMAP_POINTER_KHR or EGL_BITMAP_PITCH_KHR, and either
    //  <surface> is not locked using eglLockSurfaceKHR ... then an EGL_BAD_ACCESS error is
    //  generated.
    const bool surfaceShouldBeLocked =
        (attribute == EGL_BITMAP_POINTER_KHR) || (attribute == EGL_BITMAP_PITCH_KHR);
    const Surface *surface = dpy->getSurface(surfaceID);
    if (surfaceShouldBeLocked && !surface->isLocked())
    {
        val->setError(EGL_BAD_ACCESS, "Surface is not locked");
        return false;
    }

    return true;
}

bool ValidateUnlockSurfaceKHR(const ValidationContext *val,
                              const egl::Display *dpy,
                              SurfaceID surfaceID)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, dpy));
    ANGLE_VALIDATION_TRY(ValidateSurface(val, dpy, surfaceID));

    if (!dpy->getExtensions().lockSurface3KHR)
    {
        val->setError(EGL_BAD_ACCESS);
        return false;
    }

    const Surface *surface = dpy->getSurface(surfaceID);
    if (!surface->isLocked())
    {
        val->setError(EGL_BAD_PARAMETER, "Surface is not locked.");
        return false;
    }

    return true;
}

bool ValidateExportVkImageANGLE(const ValidationContext *val,
                                const Display *dpy,
                                ImageID imageID,
                                const void *vkImage,
                                const void *vkImageCreateInfo)
{
    ANGLE_VALIDATION_TRY(ValidateImage(val, dpy, imageID));

    if (!dpy->getExtensions().vulkanImageANGLE)
    {
        val->setError(EGL_BAD_ACCESS);
        return false;
    }

    if (!vkImage)
    {
        val->setError(EGL_BAD_PARAMETER, "Output VkImage pointer is null.");
        return false;
    }

    if (!vkImageCreateInfo)
    {
        val->setError(EGL_BAD_PARAMETER, "Output VkImageCreateInfo pointer is null.");
        return false;
    }

    return true;
}

bool ValidateSetDamageRegionKHR(const ValidationContext *val,
                                const Display *display,
                                SurfaceID surfaceID,
                                const EGLint *rects,
                                EGLint n_rects)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));
    ANGLE_VALIDATION_TRY(ValidateSurface(val, display, surfaceID));

    const Surface *surface = display->getSurface(surfaceID);
    if (!(surface->getType() & EGL_WINDOW_BIT))
    {
        val->setError(EGL_BAD_MATCH, "surface is not a postable surface");
        return false;
    }

    if (surface != val->eglThread->getCurrentDrawSurface())
    {
        val->setError(EGL_BAD_MATCH,
                      "surface is not the current draw surface for the calling thread");
        return false;
    }

    if (surface->getSwapBehavior() != EGL_BUFFER_DESTROYED)
    {
        val->setError(EGL_BAD_MATCH, "surface's swap behavior is not EGL_BUFFER_DESTROYED");
        return false;
    }

    if (surface->isDamageRegionSet())
    {
        val->setError(
            EGL_BAD_ACCESS,
            "damage region has already been set on surface since the most recent frame boundary");
        return false;
    }

    if (!surface->bufferAgeQueriedSinceLastSwap())
    {
        val->setError(EGL_BAD_ACCESS,
                      "EGL_BUFFER_AGE_KHR attribute of surface has not been queried since the most "
                      "recent frame boundary");
        return false;
    }

    return true;
}

bool ValidateQueryDmaBufFormatsEXT(ValidationContext const *val,
                                   Display const *dpy,
                                   EGLint max_formats,
                                   const EGLint *formats,
                                   const EGLint *num_formats)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, dpy));

    if (!dpy->getExtensions().imageDmaBufImportModifiersEXT)
    {
        val->setError(EGL_BAD_ACCESS, "EGL_EXT_dma_buf_import_modfier not supported");
        return false;
    }

    if (max_formats < 0)
    {
        val->setError(EGL_BAD_PARAMETER, "max_formats should not be negative");
        return false;
    }

    if (max_formats > 0 && formats == nullptr)
    {
        val->setError(EGL_BAD_PARAMETER, "if max_formats is positive, formats should not be NULL");
        return false;
    }

    return true;
}

bool ValidateQueryDmaBufModifiersEXT(ValidationContext const *val,
                                     Display const *dpy,
                                     EGLint format,
                                     EGLint max_modifiers,
                                     const EGLuint64KHR *modifiers,
                                     const EGLBoolean *external_only,
                                     const EGLint *num_modifiers)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, dpy));

    if (!dpy->getExtensions().imageDmaBufImportModifiersEXT)
    {
        val->setError(EGL_BAD_ACCESS, "EGL_EXT_dma_buf_import_modfier not supported");
        return false;
    }

    if (max_modifiers < 0)
    {
        val->setError(EGL_BAD_PARAMETER, "max_modifiers should not be negative");
        return false;
    }

    if (max_modifiers > 0 && modifiers == nullptr)
    {
        val->setError(EGL_BAD_PARAMETER,
                      "if max_modifiers is positive, modifiers should not be NULL");
        return false;
    }

    if (!dpy->supportsDmaBufFormat(format))
    {
        val->setError(EGL_BAD_PARAMETER,
                      "format should be one of the formats advertised by QueryDmaBufFormatsEXT");
        return false;
    }
    return true;
}

bool ValidateLockVulkanQueueANGLE(const ValidationContext *val, const egl::Display *display)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));

    if (!display->getDevice()->getExtensions().deviceVulkan)
    {
        val->setError(EGL_BAD_ACCESS, "EGL_ANGLE_device_vulkan not supported");
        return false;
    }

    return true;
}

bool ValidateUnlockVulkanQueueANGLE(const ValidationContext *val, const egl::Display *display)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));

    if (!display->getDevice()->getExtensions().deviceVulkan)
    {
        val->setError(EGL_BAD_ACCESS, "EGL_ANGLE_device_vulkan not supported");
        return false;
    }

    return true;
}

bool ValidateAcquireExternalContextANGLE(const ValidationContext *val,
                                         const egl::Display *display,
                                         SurfaceID drawAndReadPacked)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));
    ANGLE_VALIDATION_TRY(ValidateSurface(val, display, drawAndReadPacked));

    const DisplayExtensions &displayExtensions = display->getExtensions();
    if (!displayExtensions.externalContextAndSurface)
    {
        val->setError(EGL_BAD_ACCESS, "EGL_ANGLE_external_context_and_surface is not available");
        return false;
    }

    gl::Context *currentContext = val->eglThread->getContext();
    if (currentContext == nullptr || !currentContext->isExternal())
    {
        val->setError(EGL_BAD_CONTEXT, "Current context is not an external context");
        return false;
    }

    return true;
}

bool ValidateReleaseExternalContextANGLE(const ValidationContext *val, const egl::Display *display)
{
    ANGLE_VALIDATION_TRY(ValidateDisplay(val, display));

    const DisplayExtensions &displayExtensions = display->getExtensions();
    if (!displayExtensions.externalContextAndSurface)
    {
        val->setError(EGL_BAD_ACCESS, "EGL_ANGLE_external_context_and_surface is not available");
        return false;
    }

    gl::Context *currentContext = val->eglThread->getContext();
    if (currentContext == nullptr || !currentContext->isExternal())
    {
        val->setError(EGL_BAD_CONTEXT, "Current context is not an external context");
        return false;
    }

    return true;
}

bool ValidateSetValidationEnabledANGLE(const ValidationContext *val, EGLBoolean validationState)
{
    const ClientExtensions &clientExtensions = Display::GetClientExtensions();
    if (!clientExtensions.noErrorANGLE)
    {
        val->setError(EGL_BAD_ACCESS, "EGL_ANGLE_no_error is not available.");
        return false;
    }

    return true;
}

}  // namespace egl
