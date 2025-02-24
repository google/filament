//
// Copyright 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/DirectiveHandler.h"

#include <sstream>

#include "angle_gl.h"
#include "common/debug.h"
#include "compiler/translator/Common.h"
#include "compiler/translator/Diagnostics.h"

namespace sh
{

static TBehavior getBehavior(const std::string &str)
{
    const char kRequire[] = "require";
    const char kEnable[]  = "enable";
    const char kDisable[] = "disable";
    const char kWarn[]    = "warn";

    if (str == kRequire)
        return EBhRequire;
    else if (str == kEnable)
        return EBhEnable;
    else if (str == kDisable)
        return EBhDisable;
    else if (str == kWarn)
        return EBhWarn;
    return EBhUndefined;
}

TDirectiveHandler::TDirectiveHandler(TExtensionBehavior &extBehavior,
                                     TDiagnostics &diagnostics,
                                     int &shaderVersion,
                                     sh::GLenum shaderType)
    : mExtensionBehavior(extBehavior),
      mDiagnostics(diagnostics),
      mShaderVersion(shaderVersion),
      mShaderType(shaderType)
{}

TDirectiveHandler::~TDirectiveHandler() {}

void TDirectiveHandler::handleError(const angle::pp::SourceLocation &loc, const std::string &msg)
{
    mDiagnostics.error(loc, msg.c_str(), "");
}

void TDirectiveHandler::handlePragma(const angle::pp::SourceLocation &loc,
                                     const std::string &name,
                                     const std::string &value,
                                     bool stdgl)
{
    if (stdgl)
    {
        const char kInvariant[] = "invariant";
        const char kAll[]       = "all";

        if (name == kInvariant && value == kAll)
        {
            if (mShaderVersion == 300 && mShaderType == GL_FRAGMENT_SHADER)
            {
                // ESSL 3.00.4 section 4.6.1
                mDiagnostics.error(
                    loc, "#pragma STDGL invariant(all) can not be used in fragment shader",
                    name.c_str());
            }
            mPragma.stdgl.invariantAll = true;
        }
        // The STDGL pragma is used to reserve pragmas for use by future
        // revisions of GLSL.  Do not generate an error on unexpected
        // name and value.
        return;
    }
    else
    {
        const char kOptimize[] = "optimize";
        const char kDebug[]    = "debug";
        const char kOn[]       = "on";
        const char kOff[]      = "off";

        bool invalidValue = false;
        if (name == kOptimize)
        {
            if (value == kOn)
                mPragma.optimize = true;
            else if (value == kOff)
                mPragma.optimize = false;
            else
                invalidValue = true;
        }
        else if (name == kDebug)
        {
            if (value == kOn)
                mPragma.debug = true;
            else if (value == kOff)
                mPragma.debug = false;
            else
                invalidValue = true;
        }
        else
        {
            mDiagnostics.report(angle::pp::Diagnostics::PP_UNRECOGNIZED_PRAGMA, loc, name);
            return;
        }

        if (invalidValue)
        {
            mDiagnostics.error(loc, "invalid pragma value - 'on' or 'off' expected", value.c_str());
        }
    }
}

void TDirectiveHandler::handleExtension(const angle::pp::SourceLocation &loc,
                                        const std::string &name,
                                        const std::string &behavior)
{
    const char kExtAll[] = "all";

    TBehavior behaviorVal = getBehavior(behavior);
    if (behaviorVal == EBhUndefined)
    {
        mDiagnostics.error(loc, "behavior invalid", name.c_str());
        return;
    }

    if (name == kExtAll)
    {
        if (behaviorVal == EBhRequire)
        {
            mDiagnostics.error(loc, "extension cannot have 'require' behavior", name.c_str());
        }
        else if (behaviorVal == EBhEnable)
        {
            mDiagnostics.error(loc, "extension cannot have 'enable' behavior", name.c_str());
        }
        else
        {
            for (TExtensionBehavior::iterator iter = mExtensionBehavior.begin();
                 iter != mExtensionBehavior.end(); ++iter)
            {
                iter->second = behaviorVal;
            }
        }
        return;
    }

    TExtensionBehavior::iterator iter = mExtensionBehavior.find(GetExtensionByName(name.c_str()));
    if (iter != mExtensionBehavior.end() && CheckExtensionVersion(iter->first, mShaderVersion))
    {
        iter->second = behaviorVal;
        // OVR_multiview is implicitly enabled when OVR_multiview2 is enabled
        if (name == "GL_OVR_multiview2")
        {
            constexpr char kMultiviewExtName[] = "GL_OVR_multiview";
            iter = mExtensionBehavior.find(GetExtensionByName(kMultiviewExtName));
            if (iter != mExtensionBehavior.end())
            {
                iter->second = behaviorVal;
            }
        }
        // All the extensions listed in the spec here:
        // https://www.khronos.org/registry/OpenGL/extensions/ANDROID/ANDROID_extension_pack_es31a.txt
        // are implicitly enabled when GL_ANDROID_extension_pack_es31a is enabled
        if (name == "GL_ANDROID_extension_pack_es31a")
        {
            constexpr char kGeometryShaderExtName[]      = "GL_EXT_geometry_shader";
            constexpr char kTessellationShaderExtName[]  = "GL_EXT_tessellation_shader";
            constexpr char kGpuShader5ExtName[]          = "GL_EXT_gpu_shader5";
            constexpr char kTextureBufferExtName[]       = "GL_EXT_texture_buffer";
            constexpr char kTextureCubeMapArrayExtName[] = "GL_EXT_texture_cube_map_array";
            constexpr char kSampleVariablesExtName[]     = "GL_OES_sample_variables";
            constexpr char kShaderMultisampleInterpolationExtName[] =
                "GL_OES_shader_multisample_interpolation";
            constexpr char kShaderImageAtomicExtName[] = "GL_OES_shader_image_atomic";
            constexpr char kTextureStorageMultisample2dArrayExtName[] =
                "GL_OES_texture_storage_multisample_2d_array";
            iter = mExtensionBehavior.find(GetExtensionByName(kGeometryShaderExtName));
            if (iter != mExtensionBehavior.end())
            {
                iter->second = behaviorVal;
            }

            iter = mExtensionBehavior.find(GetExtensionByName(kTessellationShaderExtName));
            if (iter != mExtensionBehavior.end())
            {
                iter->second = behaviorVal;
            }

            iter = mExtensionBehavior.find(GetExtensionByName(kGpuShader5ExtName));
            if (iter != mExtensionBehavior.end())
            {
                iter->second = behaviorVal;
            }

            iter = mExtensionBehavior.find(GetExtensionByName(kTextureBufferExtName));
            if (iter != mExtensionBehavior.end())
            {
                iter->second = behaviorVal;
            }

            iter = mExtensionBehavior.find(GetExtensionByName(kTextureCubeMapArrayExtName));
            if (iter != mExtensionBehavior.end())
            {
                iter->second = behaviorVal;
            }

            iter = mExtensionBehavior.find(GetExtensionByName(kSampleVariablesExtName));
            if (iter != mExtensionBehavior.end())
            {
                iter->second = behaviorVal;
            }

            iter =
                mExtensionBehavior.find(GetExtensionByName(kShaderMultisampleInterpolationExtName));
            if (iter != mExtensionBehavior.end())
            {
                iter->second = behaviorVal;
            }

            iter = mExtensionBehavior.find(GetExtensionByName(kShaderImageAtomicExtName));
            if (iter != mExtensionBehavior.end())
            {
                iter->second = behaviorVal;
            }

            iter = mExtensionBehavior.find(
                GetExtensionByName(kTextureStorageMultisample2dArrayExtName));
            if (iter != mExtensionBehavior.end())
            {
                iter->second = behaviorVal;
            }
        }
        // EXT_shader_io_blocks is implicitly enabled when EXT_geometry_shader or
        // EXT_tessellation_shader is enabled.
        if (name == "GL_EXT_geometry_shader" || name == "GL_EXT_tessellation_shader")
        {
            constexpr char kIOBlocksExtName[] = "GL_EXT_shader_io_blocks";
            iter = mExtensionBehavior.find(GetExtensionByName(kIOBlocksExtName));
            if (iter != mExtensionBehavior.end())
            {
                iter->second = behaviorVal;
            }
        }
        // OES_shader_io_blocks is implicitly enabled when OES_geometry_shader or
        // OES_tessellation_shader is enabled.
        else if (name == "GL_OES_geometry_shader" || name == "GL_OES_tessellation_shader")
        {
            constexpr char kIOBlocksOESName[] = "GL_OES_shader_io_blocks";
            iter = mExtensionBehavior.find(GetExtensionByName(kIOBlocksOESName));
            if (iter != mExtensionBehavior.end())
            {
                iter->second = behaviorVal;
            }
        }
        // GL_APPLE_clip_distance is implicitly enabled when GL_EXT_clip_cull_distance or
        // GL_ANGLE_clip_cull_distance are enabled.
        else if (name == "GL_EXT_clip_cull_distance" || name == "GL_ANGLE_clip_cull_distance")
        {
            constexpr char kAPPLEClipDistanceEXTName[] = "GL_APPLE_clip_distance";
            iter = mExtensionBehavior.find(GetExtensionByName(kAPPLEClipDistanceEXTName));
            if (iter != mExtensionBehavior.end())
            {
                iter->second = behaviorVal;
            }
        }
        return;
    }

    switch (behaviorVal)
    {
        case EBhRequire:
            mDiagnostics.error(loc, "extension is not supported", name.c_str());
            break;
        case EBhEnable:
        case EBhWarn:
        case EBhDisable:
            mDiagnostics.warning(loc, "extension is not supported", name.c_str());
            break;
        default:
            UNREACHABLE();
            break;
    }
}

void TDirectiveHandler::handleVersion(const angle::pp::SourceLocation &loc,
                                      int version,
                                      ShShaderSpec spec,
                                      angle::pp::MacroSet *macro_set)
{
    if (version == 100 || version == 300 || version == 310 || version == 320)
    {
        mShaderVersion = version;

        // Add macros for supported extensions
        for (const auto &iter : mExtensionBehavior)
        {
            if (CheckExtensionVersion(iter.first, version))
            {
                // OVR_multiview should not be defined for WebGL spec'ed shaders.
                if (IsWebGLBasedSpec(spec) && (iter.first == TExtension::OVR_multiview))
                {
                    continue;
                }
                PredefineMacro(macro_set, GetExtensionNameString(iter.first), 1);
            }
        }
    }
    else
    {
        std::stringstream stream = sh::InitializeStream<std::stringstream>();
        stream << version;
        std::string str = stream.str();
        mDiagnostics.error(loc, "client/version number not supported", str.c_str());
    }
}

}  // namespace sh
