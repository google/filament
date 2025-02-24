//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_BASETYPES_H_
#define COMPILER_TRANSLATOR_BASETYPES_H_

#include <algorithm>
#include <array>

#include "GLSLANG/ShaderLang.h"
#include "common/debug.h"
#include "compiler/translator/ImmutableString.h"

namespace sh
{

//
// Precision qualifiers
//
enum TPrecision
{
    // These need to be kept sorted
    EbpUndefined,
    EbpLow,
    EbpMedium,
    EbpHigh,

    // end of list
    EbpLast
};

inline const char *getPrecisionString(TPrecision p)
{
    switch (p)
    {
        case EbpHigh:
            return "highp";
        case EbpMedium:
            return "mediump";
        case EbpLow:
            return "lowp";
        default:
            return "mediump";  // Safest fallback
    }
}

//
// Basic type.  Arrays, vectors, etc., are orthogonal to this.
//
enum TBasicType
{
    EbtVoid,
    EbtFloat,
    EbtInt,
    EbtUInt,
    EbtBool,

    EbtAtomicCounter,
    EbtYuvCscStandardEXT,  // Only valid if EXT_YUV_target exists.

    EbtGuardSamplerBegin,  // non type: see implementation of IsSampler()
    EbtSampler2D = EbtGuardSamplerBegin,
    EbtSampler3D,
    EbtSamplerCube,
    EbtSampler2DArray,
    EbtSamplerExternalOES,       // Only valid if OES_EGL_image_external exists.
    EbtSamplerExternal2DY2YEXT,  // Only valid if GL_EXT_YUV_target exists.
    EbtSampler2DRect,            // Only valid if GL_ARB_texture_rectangle exists.
    EbtSampler2DMS,
    EbtSampler2DMSArray,
    EbtISampler2D,
    EbtISampler3D,
    EbtISamplerCube,
    EbtISampler2DArray,
    EbtISampler2DMS,
    EbtISampler2DMSArray,
    EbtUSampler2D,
    EbtUSampler3D,
    EbtUSamplerCube,
    EbtUSampler2DArray,
    EbtUSampler2DMS,
    EbtUSampler2DMSArray,
    EbtSampler2DShadow,
    EbtSamplerCubeShadow,
    EbtSampler2DArrayShadow,
    EbtSamplerBuffer,
    EbtSamplerCubeArray,
    EbtSamplerCubeArrayShadow,
    EbtSampler2DRectShadow,
    EbtISampler2DRect,
    EbtISamplerBuffer,
    EbtISamplerCubeArray,
    EbtUSampler2DRect,
    EbtUSamplerBuffer,
    EbtUSamplerCubeArray,
    EbtSamplerVideoWEBGL,
    EbtGuardSamplerEnd = EbtSamplerVideoWEBGL,  // non type: see implementation of IsSampler()

    // images
    EbtGuardImageBegin,
    EbtImage2D = EbtGuardImageBegin,
    EbtImage3D,
    EbtImage2DArray,
    EbtImageCube,
    EbtImage2DMS,
    EbtImage2DMSArray,
    EbtImageCubeArray,
    EbtImageRect,
    EbtImageBuffer,
    EbtIImage2D,
    EbtIImage3D,
    EbtIImage2DArray,
    EbtIImageCube,
    EbtIImage2DMS,
    EbtIImage2DMSArray,
    EbtIImageCubeArray,
    EbtIImageRect,
    EbtIImageBuffer,
    EbtGuardUIntImageBegin,
    EbtUImage2D = EbtGuardUIntImageBegin,
    EbtUImage3D,
    EbtUImage2DArray,
    EbtUImageCube,
    EbtUImage2DMS,
    EbtUImage2DMSArray,
    EbtUImageCubeArray,
    EbtUImageRect,
    EbtUImageBuffer,
    EbtGuardUIntImageEnd = EbtUImageBuffer,
    EbtGuardImageEnd     = EbtGuardUIntImageEnd,

    // ANGLE_shader_pixel_local_storage
    EbtGuardPixelLocalBegin,
    EbtPixelLocalANGLE = EbtGuardPixelLocalBegin,
    EbtIPixelLocalANGLE,
    EbtUPixelLocalANGLE,
    EbtGuardPixelLocalEnd = EbtUPixelLocalANGLE,

    // Subpass Input
    EbtGuardSubpassInputBegin,
    EbtSubpassInput = EbtGuardSubpassInputBegin,
    EbtISubpassInput,
    EbtUSubpassInput,
    EbtGuardSubpassInputEnd = EbtUSubpassInput,

    EbtLastSimpleType = EbtGuardSubpassInputEnd,

    EbtStruct,
    EbtInterfaceBlock,

    // end of list
    EbtLast = EbtInterfaceBlock
};

class TBasicMangledName
{
  public:
    constexpr TBasicMangledName(TBasicType t) : mName{'\0', '\0'}
    {
        if (t > EbtLastSimpleType)
        {
            mName[0] = '{';
            mName[1] = '\0';
        }
        else if (t < 26)
        {
            mName[0] = '0';
            mName[1] = static_cast<char>('A' + t);
        }
        else if (t < 52)
        {
            mName[0] = '0';
            mName[1] = static_cast<char>('a' - 26 + t);
        }
        else if (t < 78)
        {
            mName[0] = '1';
            mName[1] = static_cast<char>('A' - 52 + t);
        }
        else if (t < 104)
        {
            mName[0] = '1';
            mName[1] = static_cast<char>('a' - 78 + t);
        }
    }

    constexpr char *getName() { return mName; }

    static constexpr int mangledNameSize = 2;

  private:
    char mName[mangledNameSize];
};

const char *getBasicString(TBasicType t);

inline bool IsSampler(TBasicType type)
{
    return type >= EbtGuardSamplerBegin && type <= EbtGuardSamplerEnd;
}

inline bool IsImage(TBasicType type)
{
    return type >= EbtGuardImageBegin && type <= EbtGuardImageEnd;
}

inline bool IsUIntImage(TBasicType type)
{
    return type >= EbtGuardUIntImageBegin && type <= EbtGuardUIntImageEnd;
}

inline bool IsAtomicCounter(TBasicType type)
{
    return type == EbtAtomicCounter;
}

inline bool IsPixelLocal(TBasicType type)
{
    return type >= EbtGuardPixelLocalBegin && type <= EbtGuardPixelLocalEnd;
}

inline bool IsSubpassInputType(TBasicType type)
{
    return type >= EbtGuardSubpassInputBegin && type <= EbtGuardSubpassInputEnd;
}

inline bool IsOpaqueType(TBasicType type)
{
    return IsSampler(type) || IsImage(type) || IsAtomicCounter(type) || IsPixelLocal(type) ||
           IsSubpassInputType(type);
}

inline bool IsIntegerSampler(TBasicType type)
{
    switch (type)
    {
        case EbtISampler2D:
        case EbtISampler3D:
        case EbtISamplerCube:
        case EbtISampler2DArray:
        case EbtISampler2DMS:
        case EbtISampler2DMSArray:
        case EbtUSampler2D:
        case EbtUSampler3D:
        case EbtUSamplerCube:
        case EbtUSampler2DArray:
        case EbtUSampler2DMS:
        case EbtUSampler2DMSArray:
        case EbtISampler2DRect:
        case EbtISamplerBuffer:
        case EbtISamplerCubeArray:
        case EbtUSampler2DRect:
        case EbtUSamplerBuffer:
        case EbtUSamplerCubeArray:
            return true;
        case EbtSampler2D:
        case EbtSampler3D:
        case EbtSamplerCube:
        case EbtSamplerExternalOES:
        case EbtSamplerExternal2DY2YEXT:
        case EbtSampler2DRect:
        case EbtSampler2DArray:
        case EbtSampler2DShadow:
        case EbtSamplerCubeShadow:
        case EbtSampler2DArrayShadow:
        case EbtSampler2DMS:
        case EbtSampler2DMSArray:
        case EbtSamplerBuffer:
        case EbtSamplerCubeArray:
        case EbtSamplerCubeArrayShadow:
        case EbtSampler2DRectShadow:
        case EbtSamplerVideoWEBGL:
            return false;
        default:
            ASSERT(!IsSampler(type));
    }

    return false;
}

inline bool IsIntegerSamplerUnsigned(TBasicType type)
{
    switch (type)
    {
        case EbtISampler2D:
        case EbtISampler3D:
        case EbtISamplerCube:
        case EbtISampler2DArray:
        case EbtISampler2DMS:
        case EbtISampler2DMSArray:
        case EbtISampler2DRect:
        case EbtISamplerBuffer:
        case EbtISamplerCubeArray:
            return false;
        case EbtUSampler2D:
        case EbtUSampler3D:
        case EbtUSamplerCube:
        case EbtUSampler2DArray:
        case EbtUSampler2DMS:
        case EbtUSampler2DMSArray:
        case EbtUSampler2DRect:
        case EbtUSamplerBuffer:
        case EbtUSamplerCubeArray:
            return true;
        default:
            ASSERT(!IsIntegerSampler(type));
    }

    return false;
}

inline bool IsSampler2DMS(TBasicType type)
{
    switch (type)
    {
        case EbtSampler2DMS:
        case EbtISampler2DMS:
        case EbtUSampler2DMS:
            return true;
        default:
            return false;
    }
}

inline bool IsSampler2DMSArray(TBasicType type)
{
    switch (type)
    {
        case EbtSampler2DMSArray:
        case EbtISampler2DMSArray:
        case EbtUSampler2DMSArray:
            return true;
        default:
            return false;
    }
}

inline bool IsSamplerMS(TBasicType type)
{
    return IsSampler2DMS(type) || IsSampler2DMSArray(type);
}

inline bool IsImageMS(TBasicType type)
{
    switch (type)
    {
        case EbtImage2DMS:
        case EbtImage2DMSArray:
        case EbtIImage2DMS:
        case EbtIImage2DMSArray:
        case EbtUImage2DMS:
        case EbtUImage2DMSArray:
            return true;
        default:
            return false;
    }
}

inline bool IsFloatImage(TBasicType type)
{
    switch (type)
    {
        case EbtImage2D:
        case EbtImage3D:
        case EbtImage2DArray:
        case EbtImageCube:
        case EbtImage2DMS:
        case EbtImage2DMSArray:
        case EbtImageCubeArray:
        case EbtImageRect:
        case EbtImageBuffer:
            return true;
        default:
            break;
    }

    return false;
}

inline bool IsIntegerImage(TBasicType type)
{

    switch (type)
    {
        case EbtIImage2D:
        case EbtIImage3D:
        case EbtIImage2DArray:
        case EbtIImageCube:
        case EbtIImage2DMS:
        case EbtIImage2DMSArray:
        case EbtIImageCubeArray:
        case EbtIImageRect:
        case EbtIImageBuffer:
            return true;
        default:
            break;
    }

    return false;
}

inline bool IsUnsignedImage(TBasicType type)
{

    switch (type)
    {
        case EbtUImage2D:
        case EbtUImage3D:
        case EbtUImage2DArray:
        case EbtUImageCube:
        case EbtUImage2DMS:
        case EbtUImage2DMSArray:
        case EbtUImageCubeArray:
        case EbtUImageRect:
        case EbtUImageBuffer:
            return true;
        default:
            break;
    }

    return false;
}

// Samplers are divided into 4 disjoint categories: 2D, cube, 3D, and array.
// Array samplers are not 2D samplers.
inline bool IsSampler2D(TBasicType type)
{
    switch (type)
    {
        case EbtSampler2D:
        case EbtISampler2D:
        case EbtUSampler2D:
        case EbtSampler2DRect:
        case EbtISampler2DRect:
        case EbtUSampler2DRect:
        case EbtSampler2DRectShadow:
        case EbtSamplerExternalOES:
        case EbtSamplerExternal2DY2YEXT:
        case EbtSampler2DShadow:
        case EbtSampler2DMS:
        case EbtISampler2DMS:
        case EbtUSampler2DMS:
        case EbtSamplerVideoWEBGL:
            return true;
        case EbtSampler2DArray:
        case EbtISampler2DArray:
        case EbtUSampler2DArray:
        case EbtSampler2DMSArray:
        case EbtISampler2DMSArray:
        case EbtUSampler2DMSArray:
        case EbtSampler2DArrayShadow:
        case EbtSampler3D:
        case EbtISampler3D:
        case EbtUSampler3D:
        case EbtISamplerCube:
        case EbtUSamplerCube:
        case EbtSamplerCube:
        case EbtSamplerCubeShadow:
        case EbtSamplerBuffer:
        case EbtSamplerCubeArray:
        case EbtSamplerCubeArrayShadow:
        case EbtISamplerBuffer:
        case EbtISamplerCubeArray:
        case EbtUSamplerBuffer:
        case EbtUSamplerCubeArray:
            return false;
        default:
            ASSERT(!IsSampler(type));
    }

    return false;
}

inline bool IsSamplerCube(TBasicType type)
{
    switch (type)
    {
        case EbtSamplerCube:
        case EbtISamplerCube:
        case EbtUSamplerCube:
        case EbtSamplerCubeShadow:
            return true;
        case EbtSampler2D:
        case EbtSampler3D:
        case EbtSamplerExternalOES:
        case EbtSamplerExternal2DY2YEXT:
        case EbtSampler2DRect:
        case EbtSampler2DArray:
        case EbtSampler2DMS:
        case EbtSampler2DMSArray:
        case EbtISampler2D:
        case EbtISampler3D:
        case EbtISampler2DArray:
        case EbtISampler2DMS:
        case EbtISampler2DMSArray:
        case EbtUSampler2D:
        case EbtUSampler3D:
        case EbtUSampler2DArray:
        case EbtUSampler2DMS:
        case EbtUSampler2DMSArray:
        case EbtSampler2DShadow:
        case EbtSampler2DArrayShadow:
        case EbtSamplerBuffer:
        case EbtSamplerCubeArray:
        case EbtSamplerCubeArrayShadow:
        case EbtSampler2DRectShadow:
        case EbtISampler2DRect:
        case EbtISamplerBuffer:
        case EbtISamplerCubeArray:
        case EbtUSampler2DRect:
        case EbtUSamplerBuffer:
        case EbtUSamplerCubeArray:
        case EbtSamplerVideoWEBGL:
            return false;
        default:
            ASSERT(!IsSampler(type));
    }

    return false;
}

inline bool IsSampler3D(TBasicType type)
{
    switch (type)
    {
        case EbtSampler3D:
        case EbtISampler3D:
        case EbtUSampler3D:
            return true;
        case EbtSampler2D:
        case EbtSamplerCube:
        case EbtSamplerExternalOES:
        case EbtSamplerExternal2DY2YEXT:
        case EbtSampler2DRect:
        case EbtSampler2DArray:
        case EbtSampler2DMS:
        case EbtSampler2DMSArray:
        case EbtISampler2D:
        case EbtISamplerCube:
        case EbtISampler2DArray:
        case EbtISampler2DMS:
        case EbtISampler2DMSArray:
        case EbtUSampler2D:
        case EbtUSamplerCube:
        case EbtUSampler2DArray:
        case EbtUSampler2DMS:
        case EbtUSampler2DMSArray:
        case EbtSampler2DShadow:
        case EbtSamplerCubeShadow:
        case EbtSampler2DArrayShadow:
        case EbtSamplerBuffer:
        case EbtSamplerCubeArray:
        case EbtSamplerCubeArrayShadow:
        case EbtSampler2DRectShadow:
        case EbtISampler2DRect:
        case EbtISamplerBuffer:
        case EbtISamplerCubeArray:
        case EbtUSampler2DRect:
        case EbtUSamplerBuffer:
        case EbtUSamplerCubeArray:
        case EbtSamplerVideoWEBGL:
            return false;
        default:
            ASSERT(!IsSampler(type));
    }

    return false;
}

inline bool IsSamplerArray(TBasicType type)
{
    switch (type)
    {
        case EbtSampler2DArray:
        case EbtISampler2DArray:
        case EbtUSampler2DArray:
        case EbtSampler2DMSArray:
        case EbtISampler2DMSArray:
        case EbtUSampler2DMSArray:
        case EbtSampler2DArrayShadow:
        case EbtSamplerCubeArray:
        case EbtISamplerCubeArray:
        case EbtUSamplerCubeArray:
        case EbtSamplerCubeArrayShadow:
            return true;
        case EbtSampler2D:
        case EbtISampler2D:
        case EbtUSampler2D:
        case EbtSampler2DRect:
        case EbtSamplerExternalOES:
        case EbtSamplerExternal2DY2YEXT:
        case EbtSampler3D:
        case EbtISampler3D:
        case EbtUSampler3D:
        case EbtISamplerCube:
        case EbtUSamplerCube:
        case EbtSamplerCube:
        case EbtSampler2DShadow:
        case EbtSamplerCubeShadow:
        case EbtSampler2DMS:
        case EbtISampler2DMS:
        case EbtUSampler2DMS:
        case EbtSamplerBuffer:
        case EbtSampler2DRectShadow:
        case EbtISampler2DRect:
        case EbtISamplerBuffer:
        case EbtUSampler2DRect:
        case EbtUSamplerBuffer:
        case EbtSamplerVideoWEBGL:
            return false;
        default:
            ASSERT(!IsSampler(type));
    }

    return false;
}

inline bool IsSamplerBuffer(TBasicType type)
{
    switch (type)
    {
        case EbtSamplerBuffer:
        case EbtISamplerBuffer:
        case EbtUSamplerBuffer:
            return true;
        default:
            return false;
    }
}

inline bool IsShadowSampler(TBasicType type)
{
    switch (type)
    {
        case EbtSampler2DShadow:
        case EbtSamplerCubeShadow:
        case EbtSampler2DArrayShadow:
        case EbtSamplerCubeArrayShadow:
        case EbtSampler2DRectShadow:
            return true;
        case EbtISampler2D:
        case EbtISampler3D:
        case EbtISamplerCube:
        case EbtISampler2DArray:
        case EbtISampler2DMS:
        case EbtISampler2DMSArray:
        case EbtUSampler2D:
        case EbtUSampler3D:
        case EbtUSamplerCube:
        case EbtUSampler2DArray:
        case EbtUSampler2DMS:
        case EbtUSampler2DMSArray:
        case EbtSampler2D:
        case EbtSampler3D:
        case EbtSamplerCube:
        case EbtSamplerExternalOES:
        case EbtSamplerExternal2DY2YEXT:
        case EbtSampler2DRect:
        case EbtSampler2DArray:
        case EbtSampler2DMS:
        case EbtSampler2DMSArray:
        case EbtSamplerBuffer:
        case EbtSamplerCubeArray:
        case EbtISampler2DRect:
        case EbtISamplerBuffer:
        case EbtISamplerCubeArray:
        case EbtUSampler2DRect:
        case EbtUSamplerBuffer:
        case EbtUSamplerCubeArray:
        case EbtSamplerVideoWEBGL:
            return false;
        default:
            ASSERT(!IsSampler(type));
    }

    return false;
}

inline bool IsImage2D(TBasicType type)
{
    switch (type)
    {
        case EbtImage2D:
        case EbtIImage2D:
        case EbtUImage2D:
        case EbtImage2DMS:
        case EbtIImage2DMS:
        case EbtUImage2DMS:
            return true;
        case EbtImage3D:
        case EbtIImage3D:
        case EbtUImage3D:
        case EbtImage2DArray:
        case EbtIImage2DArray:
        case EbtUImage2DArray:
        case EbtImageCube:
        case EbtIImageCube:
        case EbtUImageCube:
        case EbtImage2DMSArray:
        case EbtIImage2DMSArray:
        case EbtUImage2DMSArray:
        case EbtImageCubeArray:
        case EbtIImageCubeArray:
        case EbtUImageCubeArray:
        case EbtImageRect:
        case EbtIImageRect:
        case EbtUImageRect:
        case EbtImageBuffer:
        case EbtIImageBuffer:
        case EbtUImageBuffer:
            return false;
        default:
            ASSERT(!IsImage(type));
    }

    return false;
}

inline bool IsImage3D(TBasicType type)
{
    switch (type)
    {
        case EbtImage3D:
        case EbtIImage3D:
        case EbtUImage3D:
            return true;
        case EbtImage2D:
        case EbtIImage2D:
        case EbtUImage2D:
        case EbtImage2DArray:
        case EbtIImage2DArray:
        case EbtUImage2DArray:
        case EbtImageCube:
        case EbtIImageCube:
        case EbtUImageCube:
        case EbtImage2DMS:
        case EbtIImage2DMS:
        case EbtUImage2DMS:
        case EbtImage2DMSArray:
        case EbtIImage2DMSArray:
        case EbtUImage2DMSArray:
        case EbtImageCubeArray:
        case EbtIImageCubeArray:
        case EbtUImageCubeArray:
        case EbtImageRect:
        case EbtIImageRect:
        case EbtUImageRect:
        case EbtImageBuffer:
        case EbtIImageBuffer:
        case EbtUImageBuffer:
            return false;
        default:
            ASSERT(!IsImage(type));
    }

    return false;
}

inline bool IsImage2DArray(TBasicType type)
{
    switch (type)
    {
        case EbtImage2DArray:
        case EbtIImage2DArray:
        case EbtUImage2DArray:
        case EbtImage2DMSArray:
        case EbtIImage2DMSArray:
        case EbtUImage2DMSArray:
            return true;
        case EbtImage2D:
        case EbtIImage2D:
        case EbtUImage2D:
        case EbtImage3D:
        case EbtIImage3D:
        case EbtUImage3D:
        case EbtImageCube:
        case EbtIImageCube:
        case EbtUImageCube:
        case EbtImage2DMS:
        case EbtIImage2DMS:
        case EbtUImage2DMS:
        case EbtImageCubeArray:
        case EbtIImageCubeArray:
        case EbtUImageCubeArray:
        case EbtImageRect:
        case EbtIImageRect:
        case EbtUImageRect:
        case EbtImageBuffer:
        case EbtIImageBuffer:
        case EbtUImageBuffer:
            return false;
        default:
            ASSERT(!IsImage(type));
    }

    return false;
}

inline bool IsImageCube(TBasicType type)
{
    switch (type)
    {
        case EbtImageCube:
        case EbtIImageCube:
        case EbtUImageCube:
            return true;
        case EbtImage2D:
        case EbtIImage2D:
        case EbtUImage2D:
        case EbtImage3D:
        case EbtIImage3D:
        case EbtUImage3D:
        case EbtImage2DArray:
        case EbtIImage2DArray:
        case EbtUImage2DArray:
        case EbtImage2DMS:
        case EbtIImage2DMS:
        case EbtUImage2DMS:
        case EbtImage2DMSArray:
        case EbtIImage2DMSArray:
        case EbtUImage2DMSArray:
        case EbtImageCubeArray:
        case EbtIImageCubeArray:
        case EbtUImageCubeArray:
        case EbtImageRect:
        case EbtIImageRect:
        case EbtUImageRect:
        case EbtImageBuffer:
        case EbtIImageBuffer:
        case EbtUImageBuffer:
            return false;
        default:
            ASSERT(!IsImage(type));
    }

    return false;
}

inline bool IsImageBuffer(TBasicType type)
{
    switch (type)
    {
        case EbtImageBuffer:
        case EbtIImageBuffer:
        case EbtUImageBuffer:
            return true;
        default:
            return false;
    }
}

inline bool IsInteger(TBasicType type)
{
    return type == EbtInt || type == EbtUInt;
}

inline bool SupportsPrecision(TBasicType type)
{
    return type == EbtFloat || type == EbtInt || type == EbtUInt || IsOpaqueType(type);
}

//
// Qualifiers and built-ins.  These are mainly used to see what can be read
// or written, and by the machine dependent translator to know which registers
// to allocate variables in.  Since built-ins tend to go to different registers
// than varying or uniform, it makes sense they are peers, not sub-classes.
//
enum TQualifier
{
    EvqTemporary,   // For temporaries (within a function), read/write
    EvqGlobal,      // For globals read/write
    EvqConst,       // User defined constants
    EvqAttribute,   // Readonly
    EvqVaryingIn,   // readonly, fragment shaders only
    EvqVaryingOut,  // vertex shaders only  read/write
    EvqUniform,     // Readonly, vertex and fragment
    EvqBuffer,      // read/write, vertex, fragment and compute shader
    EvqPatch,       // EXT_tessellation_shader storage qualifier

    EvqVertexIn,     // Vertex shader input
    EvqFragmentOut,  // Fragment shader output
    EvqVertexOut,    // Vertex shader output
    EvqFragmentIn,   // Fragment shader input

    EvqFragmentInOut,  // EXT_shader_framebuffer_fetch qualifier

    // parameters
    EvqParamIn,
    EvqParamOut,
    EvqParamInOut,
    EvqParamConst,

    // built-ins read by vertex shader
    EvqInstanceID,
    EvqVertexID,

    // built-ins written by vertex shader
    EvqPosition,
    EvqPointSize,

    EvqDrawID,  // ANGLE_multi_draw

    // built-ins read by fragment shader
    EvqFragCoord,
    EvqFrontFacing,
    EvqPointCoord,
    EvqHelperInvocation,

    // built-ins written by fragment shader
    EvqFragColor,
    EvqFragData,
    EvqFragDepth,  // gl_FragDepth for ESSL300, or gl_FragDepthEXT for ESSL100, EXT_frag_depth.

    EvqSecondaryFragColorEXT,  // EXT_blend_func_extended
    EvqSecondaryFragDataEXT,   // EXT_blend_func_extended

    EvqViewIDOVR,  // OVR_multiview

    EvqClipDistance,  // APPLE_clip_distance / EXT_clip_cull_distance / ANGLE_clip_cull_distance
    EvqCullDistance,  // EXT_clip_cull_distance / ANGLE_clip_cull_distance

    // built-ins written by the shader_framebuffer_fetch extension(s)
    EvqLastFragColor,
    EvqLastFragData,
    // built-ins written by the shader_framebuffer_fetch_depth_stencil extension
    EvqLastFragDepth,
    EvqLastFragStencil,

    // GLSL ES 3.0 vertex output and fragment input

    // This section combines
    // * storage (in/out),
    // * auxiliary storage (<none>/centroid/sample), and
    // * interpolation (<none>/smooth/flat/noperspective)
    // qualifiers into a flat list.

    // Qualifiers not ending with 'In' or 'Out' are considered incomplete
    // and are used only internally. Some combinations are redundant
    // because they represent the same effective qualifiers. Specifically:
    // * 'smooth' is implied when an interpolation qualifier is omitted
    // * 'flat' makes 'centroid' and 'sample' irrelevant

    // <none>        <none>   -> original storage qualifier, e.g., EvqFragmentIn, implies smooth
    // <none>        centroid -> EvqCentroid
    // <none>        sample   -> EvqSample
    // smooth        <none>   -> EvqSmooth
    // smooth        centroid -> EvqCentroid
    // smooth        sample   -> EvqSample
    // flat          <none>   -> EvqFlat
    // flat          centroid -> EvqFlat
    // flat          sample   -> EvqFlat
    // noperspective <none>   -> EvqNoPerspective
    // noperspective centroid -> EvqNoPerspectiveCentroid
    // noperspective sample   -> EvqNoPerspectiveSample

    EvqSmooth,                 // Incomplete
    EvqFlat,                   // Incomplete
    EvqNoPerspective,          // Incomplete
    EvqCentroid,               // Incomplete
    EvqSample,                 // Incomplete
    EvqNoPerspectiveCentroid,  // Incomplete
    EvqNoPerspectiveSample,    // Incomplete
    EvqSmoothOut,
    EvqFlatOut,
    EvqNoPerspectiveOut,
    EvqCentroidOut,  // Implies smooth
    EvqSampleOut,    // Implies smooth
    EvqNoPerspectiveCentroidOut,
    EvqNoPerspectiveSampleOut,
    EvqSmoothIn,
    EvqFlatIn,
    EvqNoPerspectiveIn,
    EvqCentroidIn,  // Implies smooth
    EvqSampleIn,    // Implies smooth
    EvqNoPerspectiveCentroidIn,
    EvqNoPerspectiveSampleIn,

    // GLSL ES 3.0 extension OES_sample_variables
    EvqSampleID,
    EvqSamplePosition,
    EvqSampleMaskIn,
    EvqSampleMask,
    EvqNumSamples,

    // GLSL ES 3.1 compute shader special variables
    EvqShared,
    EvqComputeIn,
    EvqNumWorkGroups,
    EvqWorkGroupSize,
    EvqWorkGroupID,
    EvqLocalInvocationID,
    EvqGlobalInvocationID,
    EvqLocalInvocationIndex,

    // GLSL ES 3.1 memory qualifiers
    EvqReadOnly,
    EvqWriteOnly,
    EvqCoherent,
    EvqRestrict,
    EvqVolatile,

    // GLSL ES 3.1 extension EXT_geometry_shader qualifiers
    EvqGeometryIn,
    EvqGeometryOut,
    EvqPerVertexIn,    // gl_in
    EvqPrimitiveIDIn,  // gl_PrimitiveIDIn
    EvqInvocationID,   // gl_InvocationID
    EvqPrimitiveID,    // gl_PrimitiveID
    EvqLayerOut,       // gl_Layer (GS output)
    EvqLayerIn,        // gl_Layer (FS input)

    // GLSL ES 3.1 extension EXT_gpu_shader5 qualifiers
    EvqPrecise,

    // GLES ES 3.1 extension EXT_tessellation_shader qualifiers
    EvqPatchIn,
    EvqPatchOut,

    EvqTessControlIn,
    EvqTessControlOut,
    EvqPerVertexOut,
    EvqPatchVerticesIn,
    EvqTessLevelOuter,
    EvqTessLevelInner,

    // GLES ES 3.1 extension EXT_primitive_bounding_box/OES_primitive_bounding_box
    EvqBoundingBox,

    EvqTessEvaluationIn,
    EvqTessEvaluationOut,
    EvqTessCoord,

    // A specialization constant, which is not valid GLSL ES, but is there to support Vulkan output
    // generation.  In that case, TLayoutQualifier::location will contain the somewhat equivalent
    // constant_id.
    EvqSpecConst,

    // __pixel_localEXT from EXT_shader_pixel_local_storage.
    EvqPixelLocalEXT,

    // end of list
    EvqLast
};

inline bool IsQualifierUnspecified(TQualifier qualifier)
{
    return (qualifier == EvqTemporary || qualifier == EvqGlobal);
}

inline bool IsStorageBuffer(TQualifier qualifier)
{
    return qualifier == EvqBuffer;
}

inline bool IsShaderIn(TQualifier qualifier)
{
    switch (qualifier)
    {
        case EvqVertexIn:
        case EvqTessControlIn:
        case EvqTessEvaluationIn:
        case EvqGeometryIn:
        case EvqFragmentIn:
        case EvqPerVertexIn:
        case EvqAttribute:
        case EvqVaryingIn:
        case EvqSmoothIn:
        case EvqFlatIn:
        case EvqNoPerspectiveIn:
        case EvqCentroidIn:
        case EvqSampleIn:
        case EvqNoPerspectiveCentroidIn:
        case EvqNoPerspectiveSampleIn:
        case EvqPatchIn:
            return true;
        default:
            return false;
    }
}

inline bool IsShaderOut(TQualifier qualifier)
{
    switch (qualifier)
    {
        case EvqVertexOut:
        case EvqTessControlOut:
        case EvqTessEvaluationOut:
        case EvqGeometryOut:
        case EvqFragmentOut:
        case EvqPerVertexOut:
        case EvqVaryingOut:
        case EvqSmoothOut:
        case EvqFlatOut:
        case EvqNoPerspectiveOut:
        case EvqCentroidOut:
        case EvqSampleOut:
        case EvqNoPerspectiveCentroidOut:
        case EvqNoPerspectiveSampleOut:
        case EvqPatchOut:
        case EvqFragmentInOut:
            return true;
        default:
            return false;
    }
}

inline bool IsShaderIoBlock(TQualifier qualifier)
{
    switch (qualifier)
    {
        case EvqPerVertexIn:
        case EvqPerVertexOut:
        case EvqVertexOut:
        case EvqTessControlIn:
        case EvqTessControlOut:
        case EvqTessEvaluationIn:
        case EvqTessEvaluationOut:
        case EvqPatchIn:
        case EvqPatchOut:
        case EvqGeometryIn:
        case EvqGeometryOut:
        case EvqFragmentIn:
            return true;
        default:
            return false;
    }
}

enum TLayoutImageInternalFormat
{
    EiifUnspecified,
    EiifRGBA32F,
    EiifRGBA16F,
    EiifR32F,
    EiifRGBA32UI,
    EiifRGBA16UI,
    EiifRGBA8UI,
    EiifR32UI,
    EiifRGBA32I,
    EiifRGBA16I,
    EiifRGBA8I,
    EiifR32I,
    EiifRGBA8,
    EiifRGBA8_SNORM,

    EiifLast = EiifRGBA8_SNORM,
};

enum TLayoutMatrixPacking
{
    EmpUnspecified,
    EmpRowMajor,
    EmpColumnMajor,

    EmpLast = EmpColumnMajor,
};

enum TLayoutBlockStorage
{
    EbsUnspecified,
    EbsShared,
    EbsPacked,
    EbsStd140,
    EbsStd430,

    EbsLast = EbsStd430,
};

enum TLayoutDepth
{
    EdUnspecified,
    EdAny,
    EdGreater,
    EdLess,
    EdUnchanged,
};

enum TYuvCscStandardEXT
{
    EycsUndefined,
    EycsItu601,
    EycsItu601FullRange,
    EycsItu709
};

enum TLayoutPrimitiveType
{
    EptUndefined,
    EptPoints,
    EptLines,
    EptLinesAdjacency,
    EptTriangles,
    EptTrianglesAdjacency,
    EptLineStrip,
    EptTriangleStrip
};

enum TLayoutTessEvaluationType
{
    EtetUndefined,
    EtetTriangles,
    EtetQuads,
    EtetIsolines,
    EtetEqualSpacing,
    EtetFractionalEvenSpacing,
    EtetFractionalOddSpacing,
    EtetCw,
    EtetCcw,
    EtetPointMode
};

class AdvancedBlendEquations
{
  public:
    // Must have a trivial default constructor since it is used in YYSTYPE.
    AdvancedBlendEquations() = default;
    explicit constexpr AdvancedBlendEquations(uint32_t initialState)
        : mEnabledBlendEquations(initialState)
    {}

    bool any() const;
    bool all() const;
    bool anyHsl() const;

    void setAll();
    void reset() { mEnabledBlendEquations = 0; }

    // Parameter is gl::BlendEquationType, but PackedEnums.h include is not possible here.
    void set(uint32_t blendEquation);

    uint32_t bits() const { return mEnabledBlendEquations; }

    AdvancedBlendEquations operator|=(AdvancedBlendEquations other)
    {
        mEnabledBlendEquations |= other.mEnabledBlendEquations;
        return *this;
    }

    static const char *GetLayoutString(uint32_t blendEquation);
    static const char *GetAllEquationsLayoutString();

  private:
    uint32_t mEnabledBlendEquations;
};

struct TLayoutQualifier
{
    // Must have a trivial default constructor since it is used in YYSTYPE.
    TLayoutQualifier() = default;

    constexpr static TLayoutQualifier Create() { return TLayoutQualifier(0); }

    bool isEmpty() const
    {
        return location == -1 && binding == -1 && offset == -1 && numViews == -1 && yuv == false &&
               earlyFragmentTests == false && matrixPacking == EmpUnspecified &&
               blockStorage == EbsUnspecified && !localSize.isAnyValueSet() &&
               imageInternalFormat == EiifUnspecified && primitiveType == EptUndefined &&
               invocations == 0 && maxVertices == -1 && vertices == 0 && depth == EdUnspecified &&
               tesPrimitiveType == EtetUndefined && tesVertexSpacingType == EtetUndefined &&
               tesOrderingType == EtetUndefined && tesPointType == EtetUndefined && index == -1 &&
               inputAttachmentIndex == -1 && noncoherent == false &&
               !advancedBlendEquations.any() && !pushConstant;
    }

    bool isCombinationValid() const
    {
        bool workGroupSizeSpecified = localSize.isAnyValueSet();
        bool numViewsSet            = (numViews != -1);
        bool geometryShaderSpecified =
            (primitiveType != EptUndefined) || (invocations != 0) || (maxVertices != -1);
        bool subpassInputSpecified = (inputAttachmentIndex != -1);
        bool otherLayoutQualifiersSpecified =
            (location != -1 || binding != -1 || index != -1 || matrixPacking != EmpUnspecified ||
             blockStorage != EbsUnspecified || imageInternalFormat != EiifUnspecified);
        bool blendEquationSpecified = advancedBlendEquations.any();

        // we can have either the work group size specified, or number of views,
        // or yuv layout qualifier, or early_fragment_tests layout qualifier, or the other layout
        // qualifiers.
        return (workGroupSizeSpecified ? 1 : 0) + (numViewsSet ? 1 : 0) + (yuv ? 1 : 0) +
                   (earlyFragmentTests ? 1 : 0) + (otherLayoutQualifiersSpecified ? 1 : 0) +
                   (geometryShaderSpecified ? 1 : 0) + (subpassInputSpecified ? 1 : 0) +
                   (noncoherent ? 1 : 0) + (blendEquationSpecified ? 1 : 0) <=
               1;
    }

    bool isLocalSizeEqual(const WorkGroupSize &localSizeIn) const
    {
        return localSize.isWorkGroupSizeMatching(localSizeIn);
    }

    int location;
    unsigned int locationsSpecified;
    TLayoutMatrixPacking matrixPacking;
    TLayoutBlockStorage blockStorage;

    // Compute shader layout qualifiers.
    WorkGroupSize localSize;

    int binding;
    int offset;

    bool pushConstant;

    // Depth layout qualifier
    TLayoutDepth depth;

    // Image format layout qualifier
    TLayoutImageInternalFormat imageInternalFormat;

    // OVR_multiview num_views.
    int numViews;

    // EXT_YUV_target yuv layout qualifier.
    bool yuv;

    // early_fragment_tests qualifier.
    bool earlyFragmentTests;

    // OES_geometry_shader layout qualifiers.
    TLayoutPrimitiveType primitiveType;
    int invocations;
    int maxVertices;

    // EXT_tessellation_shader shader layout qualifiers
    int vertices;
    TLayoutTessEvaluationType tesPrimitiveType;
    TLayoutTessEvaluationType tesVertexSpacingType;
    TLayoutTessEvaluationType tesOrderingType;
    TLayoutTessEvaluationType tesPointType;

    // EXT_blend_func_extended fragment output layout qualifier
    int index;

    // EXT_shader_framebuffer_fetch layout qualifiers.
    int inputAttachmentIndex;
    bool noncoherent;

    // KHR_blend_equation_advanced layout qualifiers.
    AdvancedBlendEquations advancedBlendEquations;

    // D3D 11.3 Rasterizer Order Views (ROVs).
    // This qualifier is only used internally by ANGLE; it is not visible to the application.
    bool rasterOrdered;

  private:
    explicit constexpr TLayoutQualifier(int /*placeholder*/)
        : location(-1),
          locationsSpecified(0),
          matrixPacking(EmpUnspecified),
          blockStorage(EbsUnspecified),
          localSize(-1),
          binding(-1),
          offset(-1),
          pushConstant(false),
          depth(EdUnspecified),
          imageInternalFormat(EiifUnspecified),
          numViews(-1),
          yuv(false),
          earlyFragmentTests(false),
          primitiveType(EptUndefined),
          invocations(0),
          maxVertices(-1),
          vertices(0),
          tesPrimitiveType(EtetUndefined),
          tesVertexSpacingType(EtetUndefined),
          tesOrderingType(EtetUndefined),
          tesPointType(EtetUndefined),
          index(-1),
          inputAttachmentIndex(-1),
          noncoherent(false),
          advancedBlendEquations(0),
          rasterOrdered(false)
    {}
};

struct TMemoryQualifier
{
    // Must have a trivial default constructor since it is used in YYSTYPE.
    TMemoryQualifier() = default;

    bool isEmpty() const
    {
        return !readonly && !writeonly && !coherent && !restrictQualifier && !volatileQualifier;
    }

    constexpr static TMemoryQualifier Create() { return TMemoryQualifier(0); }

    // Used for GLSL generation, debugging and error messages.
    inline const char *getAnyQualifierString() const
    {
        if (readonly)
        {
            return "readonly";
        }
        if (writeonly)
        {
            return "writeonly";
        }
        if (coherent)
        {
            return "coherent";
        }
        if (restrictQualifier)
        {
            return "restrict";
        }
        if (volatileQualifier)
        {
            return "volatile";
        }
        ASSERT(isEmpty());
        return "";
    }

    // GLSL ES 3.10 Revision 4, 4.9 Memory Access Qualifiers
    // An image can be qualified as both readonly and writeonly. It still can be can be used with
    // imageSize().
    bool readonly;
    bool writeonly;
    bool coherent;

    // restrict and volatile are reserved keywords in C/C++
    bool restrictQualifier;
    bool volatileQualifier;

  private:
    explicit constexpr TMemoryQualifier(int /*placeholder*/)
        : readonly(false),
          writeonly(false),
          coherent(false),
          restrictQualifier(false),
          volatileQualifier(false)
    {}
};

inline const char *getWorkGroupSizeString(size_t dimension)
{
    switch (dimension)
    {
        case 0u:
            return "local_size_x";
        case 1u:
            return "local_size_y";
        case 2u:
            return "local_size_z";
        default:
            UNREACHABLE();
            return "dimension out of bounds";
    }
}

// Used for GLSL generation, debugging and error messages.
inline const char *getQualifierString(TQualifier q)
{
    // clang-format off
    switch(q)
    {
    case EvqTemporary:                 return "Temporary";
    case EvqGlobal:                    return "Global";
    case EvqConst:                     return "const";
    case EvqAttribute:                 return "attribute";
    case EvqVaryingIn:                 return "varying";
    case EvqVaryingOut:                return "varying";
    case EvqUniform:                   return "uniform";
    case EvqBuffer:                    return "buffer";
    case EvqPatch:                     return "patch";
    case EvqVertexIn:                  return "in";
    case EvqFragmentOut:               return "out";
    case EvqVertexOut:                 return "out";
    case EvqFragmentIn:                return "in";
    case EvqParamIn:                   return "in";
    case EvqParamOut:                  return "out";
    case EvqParamInOut:                return "inout";
    case EvqParamConst:                return "const";
    case EvqInstanceID:                return "InstanceID";
    case EvqVertexID:                  return "VertexID";
    case EvqPosition:                  return "Position";
    case EvqPointSize:                 return "PointSize";
    case EvqDrawID:                    return "DrawID";
    case EvqFragCoord:                 return "FragCoord";
    case EvqFrontFacing:               return "FrontFacing";
    case EvqHelperInvocation:          return "HelperInvocation";
    case EvqPointCoord:                return "PointCoord";
    case EvqFragColor:                 return "FragColor";
    case EvqFragData:                  return "FragData";
    case EvqFragDepth:                 return "FragDepth";
    case EvqSecondaryFragColorEXT:     return "SecondaryFragColorEXT";
    case EvqSecondaryFragDataEXT:      return "SecondaryFragDataEXT";
    case EvqViewIDOVR:                 return "ViewIDOVR";
    case EvqLayerOut:                  return "LayerOut";
    case EvqLayerIn:                   return "LayerIn";
    case EvqLastFragColor:             return "LastFragColor";
    case EvqLastFragData:              return "LastFragData";
    case EvqLastFragDepth:             return "LastFragDepthARM";
    case EvqLastFragStencil:           return "LastFragStencilARM";
    case EvqFragmentInOut:             return "inout";
    case EvqSmoothOut:                 return "smooth out";
    case EvqCentroidOut:               return "smooth centroid out";
    case EvqFlatOut:                   return "flat out";
    case EvqNoPerspectiveOut:          return "noperspective out";
    case EvqNoPerspectiveCentroidOut:  return "noperspective centroid out";
    case EvqNoPerspectiveSampleOut:    return "noperspective sample out";
    case EvqSmoothIn:                  return "smooth in";
    case EvqFlatIn:                    return "flat in";
    case EvqNoPerspectiveIn:           return "noperspective in";
    case EvqNoPerspectiveCentroidIn:   return "noperspective centroid in";
    case EvqNoPerspectiveSampleIn:     return "noperspective sample in";
    case EvqCentroidIn:                return "smooth centroid in";
    case EvqCentroid:                  return "centroid";
    case EvqFlat:                      return "flat";
    case EvqNoPerspective:             return "noperspective";
    case EvqNoPerspectiveCentroid:     return "noperspective centroid";
    case EvqNoPerspectiveSample:       return "noperspective sample";
    case EvqSmooth:                    return "smooth";
    case EvqShared:                    return "shared";
    case EvqComputeIn:                 return "in";
    case EvqNumWorkGroups:             return "NumWorkGroups";
    case EvqWorkGroupSize:             return "WorkGroupSize";
    case EvqWorkGroupID:               return "WorkGroupID";
    case EvqLocalInvocationID:         return "LocalInvocationID";
    case EvqGlobalInvocationID:        return "GlobalInvocationID";
    case EvqLocalInvocationIndex:      return "LocalInvocationIndex";
    case EvqReadOnly:                  return "readonly";
    case EvqWriteOnly:                 return "writeonly";
    case EvqCoherent:                  return "coherent";
    case EvqRestrict:                  return "restrict";
    case EvqVolatile:                  return "volatile";
    case EvqGeometryIn:                return "in";
    case EvqGeometryOut:               return "out";
    case EvqPerVertexIn:               return "gl_in";
    case EvqPrimitiveIDIn:             return "gl_PrimitiveIDIn";
    case EvqInvocationID:              return "gl_InvocationID";
    case EvqPrimitiveID:               return "gl_PrimitiveID";
    case EvqPrecise:                   return "precise";
    case EvqClipDistance:              return "ClipDistance";
    case EvqCullDistance:              return "CullDistance";
    case EvqSample:                    return "sample";
    case EvqSampleIn:                  return "sample in";
    case EvqSampleOut:                 return "sample out";
    case EvqSampleID:                  return "SampleID";
    case EvqSamplePosition:            return "SamplePosition";
    case EvqSampleMaskIn:              return "SampleMaskIn";
    case EvqSampleMask:                return "SampleMask";
    case EvqNumSamples:                return "NumSamples";
    case EvqPatchIn:                   return "patch in";
    case EvqPatchOut:                  return "patch out";
    case EvqTessControlIn:             return "in";
    case EvqTessControlOut:            return "out";
    case EvqPerVertexOut:              return "out";
    case EvqPatchVerticesIn:           return "PatchVerticesIn";
    case EvqTessLevelOuter:            return "TessLevelOuter";
    case EvqTessLevelInner:            return "TessLevelInner";
    case EvqBoundingBox:               return "BoundingBox";
    case EvqTessEvaluationIn:          return "in";
    case EvqTessEvaluationOut:         return "out";
    case EvqTessCoord:                 return "TessCoord";
    case EvqSpecConst:                 return "const";
    case EvqPixelLocalEXT:             return "__pixel_localEXT";
    default: UNREACHABLE();            return "unknown qualifier";
    }
    // clang-format on
}

inline const char *getMatrixPackingString(TLayoutMatrixPacking mpq)
{
    switch (mpq)
    {
        case EmpUnspecified:
            return "mp_unspecified";
        case EmpRowMajor:
            return "row_major";
        case EmpColumnMajor:
            return "column_major";
        default:
            UNREACHABLE();
            return "unknown matrix packing";
    }
}

inline const char *getBlockStorageString(TLayoutBlockStorage bsq)
{
    switch (bsq)
    {
        case EbsUnspecified:
            return "bs_unspecified";
        case EbsShared:
            return "shared";
        case EbsPacked:
            return "packed";
        case EbsStd140:
            return "std140";
        case EbsStd430:
            return "std430";
        default:
            UNREACHABLE();
            return "unknown block storage";
    }
}

inline const char *getImageInternalFormatString(TLayoutImageInternalFormat iifq)
{
    switch (iifq)
    {
        case EiifRGBA32F:
            return "rgba32f";
        case EiifRGBA16F:
            return "rgba16f";
        case EiifR32F:
            return "r32f";
        case EiifRGBA32UI:
            return "rgba32ui";
        case EiifRGBA16UI:
            return "rgba16ui";
        case EiifRGBA8UI:
            return "rgba8ui";
        case EiifR32UI:
            return "r32ui";
        case EiifRGBA32I:
            return "rgba32i";
        case EiifRGBA16I:
            return "rgba16i";
        case EiifRGBA8I:
            return "rgba8i";
        case EiifR32I:
            return "r32i";
        case EiifRGBA8:
            return "rgba8";
        case EiifRGBA8_SNORM:
            return "rgba8_snorm";
        default:
            UNREACHABLE();
            return "unknown internal image format";
    }
}

inline const char *getDepthString(TLayoutDepth depth)
{
    switch (depth)
    {
        case EdUnspecified:
            return "depth_unspecified";
        case EdAny:
            return "depth_any";
        case EdGreater:
            return "depth_greater";
        case EdLess:
            return "depth_less";
        case EdUnchanged:
            return "depth_unchanged";
        default:
            UNREACHABLE();
            return "unknown depth";
    }
}

inline TYuvCscStandardEXT getYuvCscStandardEXT(const ImmutableString &str)
{
    if (str == "itu_601")
        return EycsItu601;
    else if (str == "itu_601_full_range")
        return EycsItu601FullRange;
    else if (str == "itu_709")
        return EycsItu709;
    return EycsUndefined;
}

inline const char *getYuvCscStandardEXTString(TYuvCscStandardEXT ycsq)
{
    switch (ycsq)
    {
        case EycsItu601:
            return "itu_601";
        case EycsItu601FullRange:
            return "itu_601_full_range";
        case EycsItu709:
            return "itu_709";
        default:
            UNREACHABLE();
            return "unknown color space conversion standard";
    }
}

inline const char *getGeometryShaderPrimitiveTypeString(TLayoutPrimitiveType primitiveType)
{
    switch (primitiveType)
    {
        case EptPoints:
            return "points";
        case EptLines:
            return "lines";
        case EptTriangles:
            return "triangles";
        case EptLinesAdjacency:
            return "lines_adjacency";
        case EptTrianglesAdjacency:
            return "triangles_adjacency";
        case EptLineStrip:
            return "line_strip";
        case EptTriangleStrip:
            return "triangle_strip";
        default:
            UNREACHABLE();
            return "unknown geometry shader primitive type";
    }
}

inline const char *getTessEvaluationShaderTypeString(TLayoutTessEvaluationType type)
{
    switch (type)
    {
        case EtetTriangles:
            return "triangles";
        case EtetQuads:
            return "quads";
        case EtetIsolines:
            return "isolines";
        case EtetEqualSpacing:
            return "equal_spacing";
        case EtetFractionalEvenSpacing:
            return "fractional_even_spacing";
        case EtetFractionalOddSpacing:
            return "fractional_odd_spacing";
        case EtetCw:
            return "cw";
        case EtetCcw:
            return "ccw";
        case EtetPointMode:
            return "point_mode";
        default:
            UNREACHABLE();
            return "unknown tessellation evaluation shader variable type";
    }
}

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_BASETYPES_H_
