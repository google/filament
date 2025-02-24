//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// platform_glx.h: Includes specific to GLX.

#ifndef LIBANGLE_RENDERER_GL_GLX_PLATFORMGLX_H_
#define LIBANGLE_RENDERER_GL_GLX_PLATFORMGLX_H_

#if !defined(ANGLE_SKIP_GLX_DEFINES)
// GLX 1.0
#    define GLX_USE_GL 1
#    define GLX_BUFFER_SIZE 2
#    define GLX_LEVEL 3
#    define GLX_RGBA 4
#    define GLX_DOUBLEBUFFER 5
#    define GLX_STEREO 6
#    define GLX_AUX_BUFFERS 7
#    define GLX_RED_SIZE 8
#    define GLX_GREEN_SIZE 9
#    define GLX_BLUE_SIZE 10
#    define GLX_ALPHA_SIZE 11
#    define GLX_DEPTH_SIZE 12
#    define GLX_STENCIL_SIZE 13
#    define GLX_ACCUM_RED_SIZE 14
#    define GLX_ACCUM_GREEN_SIZE 15
#    define GLX_ACCUM_BLUE_SIZE 16
#    define GLX_ACCUM_ALPHA_SIZE 17

#    define GLX_BAD_SCREEN 1
#    define GLX_BAD_ATTRIBUTE 2
#    define GLX_NO_EXTENSION 3
#    define GLX_BAD_VISUAL 4
#    define GLX_BAD_CONTEXT 5
#    define GLX_BAD_VALUE 6
#    define GLX_BAD_ENUM 7

// GLX 1.1
#    define GLX_VENDOR 1
#    define GLX_VERSION 2
#    define GLX_EXTENSIONS 3

// GLX 1.3
#    define GLX_CONFIG_CAVEAT 0x20
#    define GLX_DONT_CARE 0xFFFFFFFF
#    define GLX_X_VISUAL_TYPE 0x22
#    define GLX_TRANSPARENT_TYPE 0x23
#    define GLX_TRANSPARENT_INDEX_VALUE 0x24
#    define GLX_TRANSPARENT_RED_VALUE 0x25
#    define GLX_TRANSPARENT_GREEN_VALUE 0x26
#    define GLX_TRANSPARENT_BLUE_VALUE 0x27
#    define GLX_TRANSPARENT_ALPHA_VALUE 0x28
#    define GLX_WINDOW_BIT 0x00000001
#    define GLX_PIXMAP_BIT 0x00000002
#    define GLX_PBUFFER_BIT 0x00000004
#    define GLX_AUX_BUFFERS_BIT 0x00000010
#    define GLX_FRONT_LEFT_BUFFER_BIT 0x00000001
#    define GLX_FRONT_RIGHT_BUFFER_BIT 0x00000002
#    define GLX_BACK_LEFT_BUFFER_BIT 0x00000004
#    define GLX_BACK_RIGHT_BUFFER_BIT 0x00000008
#    define GLX_DEPTH_BUFFER_BIT 0x00000020
#    define GLX_STENCIL_BUFFER_BIT 0x00000040
#    define GLX_ACCUM_BUFFER_BIT 0x00000080
#    define GLX_NONE 0x8000
#    define GLX_SLOW_CONFIG 0x8001
#    define GLX_TRUE_COLOR 0x8002
#    define GLX_DIRECT_COLOR 0x8003
#    define GLX_PSEUDO_COLOR 0x8004
#    define GLX_STATIC_COLOR 0x8005
#    define GLX_GRAY_SCALE 0x8006
#    define GLX_STATIC_GRAY 0x8007
#    define GLX_TRANSPARENT_RGB 0x8008
#    define GLX_TRANSPARENT_INDEX 0x8009
#    define GLX_VISUAL_ID 0x800B
#    define GLX_SCREEN 0x800C
#    define GLX_NON_CONFORMANT_CONFIG 0x800D
#    define GLX_DRAWABLE_TYPE 0x8010
#    define GLX_RENDER_TYPE 0x8011
#    define GLX_X_RENDERABLE 0x8012
#    define GLX_FBCONFIG_ID 0x8013
#    define GLX_RGBA_TYPE 0x8014
#    define GLX_COLOR_INDEX_TYPE 0x8015
#    define GLX_MAX_PBUFFER_WIDTH 0x8016
#    define GLX_MAX_PBUFFER_HEIGHT 0x8017
#    define GLX_MAX_PBUFFER_PIXELS 0x8018
#    define GLX_PRESERVED_CONTENTS 0x801B
#    define GLX_LARGEST_PBUFFER 0x801C
#    define GLX_WIDTH 0x801D
#    define GLX_HEIGHT 0x801E
#    define GLX_EVENT_MASK 0x801F
#    define GLX_DAMAGED 0x8020
#    define GLX_SAVED 0x8021
#    define GLX_WINDOW 0x8022
#    define GLX_PBUFFER 0x8023
#    define GLX_PBUFFER_HEIGHT 0x8040
#    define GLX_PBUFFER_WIDTH 0x8041
#    define GLX_RGBA_BIT 0x00000001
#    define GLX_COLOR_INDEX_BIT 0x00000002
#    define GLX_PBUFFER_CLOBBER_MASK 0x08000000

// GLX 1.4
#    define GLX_SAMPLE_BUFFERS 0x186a0
#    define GLX_SAMPLES 0x186a1

// GLX_ARB_create_context
#    define GLX_CONTEXT_DEBUG_BIT_ARB 0x00000001
#    define GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0x00000002
#    define GLX_CONTEXT_MAJOR_VERSION_ARB 0x2091
#    define GLX_CONTEXT_MINOR_VERSION_ARB 0x2092
#    define GLX_CONTEXT_FLAGS_ARB 0x2094

// GLX_ARB_create_context_profile
#    define GLX_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#    define GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002
#    define GLX_CONTEXT_PROFILE_MASK_ARB 0x9126

// GLX_ARB_create_context_robustness
#    define GLX_CONTEXT_ROBUST_ACCESS_BIT_ARB 0x00000004
#    define GLX_LOSE_CONTEXT_ON_RESET_ARB 0x8252
#    define GLX_CONTEXT_RESET_NOTIFICATION_STRATEGY_ARB 0x8256
#    define GLX_NO_RESET_NOTIFICATION_ARB 0x8261

// GLX_EXT_create_context_es2_profile
#    define GLX_CONTEXT_ES2_PROFILE_BIT_EXT 0x00000004

// GLX_EXT_texture_from_pixmap
#    define GLX_TEXTURE_1D_BIT_EXT 0x00000001
#    define GLX_TEXTURE_2D_BIT_EXT 0x00000002
#    define GLX_TEXTURE_RECTANGLE_BIT_EXT 0x00000004
#    define GLX_BIND_TO_TEXTURE_RGB_EXT 0x20D0
#    define GLX_BIND_TO_TEXTURE_RGBA_EXT 0x20D1
#    define GLX_BIND_TO_MIPMAP_TEXTURE_EXT 0x20D2
#    define GLX_BIND_TO_TEXTURE_TARGETS_EXT 0x20D3
#    define GLX_Y_INVERTED_EXT 0x20D4
#    define GLX_TEXTURE_FORMAT_EXT 0x20D5
#    define GLX_TEXTURE_TARGET_EXT 0x20D6
#    define GLX_MIPMAP_TEXTURE_EXT 0x20D7
#    define GLX_TEXTURE_FORMAT_NONE_EXT 0x20D8
#    define GLX_TEXTURE_FORMAT_RGB_EXT 0x20D9
#    define GLX_TEXTURE_FORMAT_RGBA_EXT 0x20DA
#    define GLX_TEXTURE_1D_EXT 0x20DB
#    define GLX_TEXTURE_2D_EXT 0x20DC
#    define GLX_TEXTURE_RECTANGLE_EXT 0x20DD
#    define GLX_FRONT_LEFT_EXT 0x20DE
#    define GLX_FRONT_RIGHT_EXT 0x20DF
#    define GLX_BACK_LEFT_EXT 0x20E0
#    define GLX_BACK_RIGHT_EXT 0x20E1
#    define GLX_FRONT_EXT 0x20DE
#    define GLX_BACK_EXT 0x20E0
#    define GLX_AUX0_EXT 0x20E2
#    define GLX_AUX1_EXT 0x20E3
#    define GLX_AUX2_EXT 0x20E4
#    define GLX_AUX3_EXT 0x20E5
#    define GLX_AUX4_EXT 0x20E6
#    define GLX_AUX5_EXT 0x20E7
#    define GLX_AUX6_EXT 0x20E8
#    define GLX_AUX7_EXT 0x20E9
#    define GLX_AUX8_EXT 0x20EA
#    define GLX_AUX9_EXT 0x20EB

// GLX_EXT_swap_control
#    define GLX_SWAP_INTERVAL_EXT 0x20F1
#    define GLX_MAX_SWAP_INTERVAL_EXT 0x20F2

// GLX_NV_robustness_video_memory_purge
#    define GLX_GENERATE_RESET_ON_VIDEO_MEMORY_PURGE_NV 0x20F7

#endif  // !defined(ANGLE_SKIP_GLX_DEFINES)

// GLX typedefs depend on the X headers
#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>

// GLX typedefs
namespace glx
{

typedef void *Context;
typedef void *FBConfig;
typedef XID FBConfigID;
typedef XID ContextID;
typedef XID Window;
typedef XID Pbuffer;
typedef XID Pixmap;
typedef XID Drawable;

}  // namespace glx

typedef void *(*PFNGETPROCPROC)(const char *name);

#endif  // LIBANGLE_RENDERER_GL_GLX_PLATFORMGLX_H_
