#!/usr/bin/python3
#
# Copyright 2017 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# generate_entry_points.py:
#   Generates the OpenGL bindings and entry point layers for ANGLE.
#   NOTE: don't run this script directly. Run scripts/run_code_generation.py.

import sys, os, pprint, json
import fnmatch
import registry_xml
from registry_xml import apis, script_relative, strip_api_prefix, api_enums

# Paths
CL_STUBS_HEADER_PATH = "../src/libGLESv2/cl_stubs_autogen.h"
EGL_GET_LABELED_OBJECT_DATA_PATH = "../src/libGLESv2/egl_get_labeled_object_data.json"
EGL_STUBS_HEADER_PATH = "../src/libGLESv2/egl_stubs_autogen.h"
EGL_EXT_STUBS_HEADER_PATH = "../src/libGLESv2/egl_ext_stubs_autogen.h"

# List of GLES1 extensions for which we don't need to add Context.h decls.
GLES1_NO_CONTEXT_DECL_EXTENSIONS = [
    "GL_OES_framebuffer_object",
]

# This is a list of exceptions for entry points which don't want to have
# the EVENT macro. This is required for some debug marker entry points.
NO_EVENT_MARKER_EXCEPTIONS_LIST = sorted([
    "glPushGroupMarkerEXT",
    "glPopGroupMarkerEXT",
    "glInsertEventMarkerEXT",
])

ALIASING_EXCEPTIONS = [
    # glRenderbufferStorageMultisampleEXT aliases
    # glRenderbufferStorageMultisample on desktop GL, and is marked as such in
    # the registry.  However, that is not correct for GLES where this entry
    # point comes from GL_EXT_multisampled_render_to_texture which is never
    # promoted to core GLES.
    'renderbufferStorageMultisampleEXT',
    # Other entry points where the extension behavior is not identical to core
    # behavior.
    'drawArraysInstancedBaseInstanceANGLE',
    'drawElementsInstancedBaseVertexBaseInstanceANGLE',
    'logicOpANGLE',
]

# These are the entry points which potentially are used first by an application
# and require that the back ends are initialized before the front end is called.
INIT_DICT = {
    "clGetPlatformIDs": "false",
    "clGetPlatformInfo": "false",
    "clGetDeviceIDs": "false",
    "clCreateContext": "false",
    "clCreateContextFromType": "false",
    "clIcdGetPlatformIDsKHR": "true",
}

# These are the only entry points that are allowed while pixel local storage is active.
PLS_ALLOW_LIST = {
    "ActiveTexture",
    "BindBuffer",
    "BindBufferBase",
    "BindBufferRange",
    "BindFramebuffer",
    "BindSampler",
    "BindTexture",
    "BindVertexArray",
    "BlendEquation",
    "BlendEquationSeparate",
    "BlendFunc",
    "BlendFuncSeparate",
    "BufferData",
    "BufferSubData",
    "CheckFramebufferStatus",
    "ClipControlEXT",
    "ColorMask",
    "CullFace",
    "DepthFunc",
    "DepthMask",
    "DepthRangef",
    "Disable",
    "DisableVertexAttribArray",
    "DispatchComputeIndirect",
    "DrawBuffers",
    "Enable",
    "EnableClientState",
    "EnableVertexAttribArray",
    "EndPixelLocalStorageANGLE",
    "FenceSync",
    "FlushMappedBufferRange",
    "FramebufferMemorylessPixelLocalStorageANGLE",
    "FramebufferPixelLocalStorageInterruptANGLE",
    "FramebufferRenderbuffer",
    "FrontFace",
    "MapBufferRange",
    "PixelLocalStorageBarrierANGLE",
    "ProvokingVertexANGLE",
    "Scissor",
    "StencilFunc",
    "StencilFuncSeparate",
    "StencilMask",
    "StencilMaskSeparate",
    "StencilOp",
    "StencilOpSeparate",
    "UnmapBuffer",
    "UseProgram",
    "ValidateProgram",
    "Viewport",
}
PLS_ALLOW_WILDCARDS = [
    "BlendEquationSeparatei*",
    "BlendEquationi*",
    "BlendFuncSeparatei*",
    "BlendFunci*",
    "ClearBuffer*",
    "ColorMaski*",
    "DebugMessageCallback*",
    "DebugMessageControl*",
    "DebugMessageInsert*",
    "Delete*",
    "Disablei*",
    "DrawArrays*",
    "DrawElements*",
    "DrawRangeElements*",
    "Enablei*",
    "FramebufferParameter*",
    "FramebufferTexture*",
    "Gen*",
    "Get*",
    "Is*",
    "ObjectLabel*",
    "ObjectPtrLabel*",
    "PolygonMode*",
    "PolygonOffset*",
    "PopDebugGroup*",
    "PushDebugGroup*",
    "SamplerParameter*",
    "TexParameter*",
    "Uniform*",
    "VertexAttrib*",
]

# These are the entry points which purely set state in the current context with
# no interaction with the other contexts, including through shared resources.
# As a result, they don't require the share group lock.
CONTEXT_PRIVATE_LIST = [
    'glActiveTexture',
    'glBlendColor',
    'glBlobCacheCallbacksANGLE',
    'glClearColor',
    'glClearDepthf',
    'glClearStencil',
    'glClipControl',
    'glColorMask',
    'glColorMaski',
    'glCoverageModulation',
    'glCullFace',
    'glDepthFunc',
    'glDepthMask',
    'glDepthRangef',
    'glDisable',
    'glDisablei',
    'glEnable',
    'glEnablei',
    'glFrontFace',
    'glHint',
    'glIsEnabled',
    'glIsEnabledi',
    'glLineWidth',
    'glLogicOpANGLE',
    'glMinSampleShading',
    'glPatchParameteri',
    'glPixelStorei',
    'glPolygonMode',
    'glPolygonModeNV',
    'glPolygonOffset',
    'glPolygonOffsetClamp',
    'glPrimitiveBoundingBox',
    'glProvokingVertex',
    'glSampleCoverage',
    'glSampleMaski',
    'glScissor',
    'glShadingRate',
    'glStencilFunc',
    'glStencilFuncSeparate',
    'glStencilMask',
    'glStencilMaskSeparate',
    'glStencilOp',
    'glStencilOpSeparate',
    'glViewport',
    # GLES1 entry points
    'glAlphaFunc',
    'glAlphaFuncx',
    'glClearColorx',
    'glClearDepthx',
    'glColor4f',
    'glColor4ub',
    'glColor4x',
    'glDepthRangex',
    'glLineWidthx',
    'glLoadIdentity',
    'glLogicOp',
    'glMatrixMode',
    'glPointSize',
    'glPointSizex',
    'glPopMatrix',
    'glPolygonOffsetx',
    'glPushMatrix',
    'glSampleCoveragex',
    'glShadeModel',
]
CONTEXT_PRIVATE_WILDCARDS = [
    'glBlendFunc*',
    'glBlendEquation*',
    'glVertexAttrib[1-4]*',
    'glVertexAttribI[1-4]*',
    'glVertexAttribP[1-4]*',
    'glVertexAttribL[1-4]*',
    # GLES1 entry points
    'glClipPlane[fx]',
    'glGetClipPlane[fx]',
    'glFog[fx]*',
    'glFrustum[fx]',
    'glGetLight[fx]v',
    'glGetMaterial[fx]v',
    'glGetTexEnv[fix]v',
    'glLoadMatrix[fx]',
    'glLight[fx]*',
    'glLightModel[fx]*',
    'glMaterial[fx]*',
    'glMultMatrix[fx]',
    'glMultiTexCoord4[fx]',
    'glNormal3[fx]',
    'glOrtho[fx]',
    'glPointParameter[fx]*',
    'glRotate[fx]',
    'glScale[fx]',
    'glTexEnv[fix]*',
    'glTranslate[fx]',
]

TEMPLATE_ENTRY_POINT_HEADER = """\
// GENERATED FILE - DO NOT EDIT.
// Generated by {script_name} using data from {data_source_name}.
//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// entry_points_{annotation_lower}_autogen.h:
//   Defines the {comment} entry points.

#ifndef {lib}_ENTRY_POINTS_{annotation_upper}_AUTOGEN_H_
#define {lib}_ENTRY_POINTS_{annotation_upper}_AUTOGEN_H_

{includes}

{entry_points}

#endif  // {lib}_ENTRY_POINTS_{annotation_upper}_AUTOGEN_H_
"""

TEMPLATE_ENTRY_POINT_SOURCE = """\
// GENERATED FILE - DO NOT EDIT.
// Generated by {script_name} using data from {data_source_name}.
//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// entry_points_{annotation_lower}_autogen.cpp:
//   Defines the {comment} entry points.

{includes}

{entry_points}
"""

TEMPLATE_ENTRY_POINTS_ENUM_HEADER = """\
// GENERATED FILE - DO NOT EDIT.
// Generated by {script_name} using data from {data_source_name}.
//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// entry_points_enum_autogen.h:
//   Defines the {lib} entry points enumeration.

#ifndef COMMON_ENTRYPOINTSENUM_AUTOGEN_H_
#define COMMON_ENTRYPOINTSENUM_AUTOGEN_H_

namespace angle
{{
enum class EntryPoint
{{
{entry_points_list}
}};

const char *GetEntryPointName(EntryPoint ep);
}}  // namespace angle
#endif  // COMMON_ENTRY_POINTS_ENUM_AUTOGEN_H_
"""

TEMPLATE_ENTRY_POINTS_NAME_CASE = """\
        case EntryPoint::{enum}:
            return "{cmd}";"""

TEMPLATE_ENTRY_POINTS_ENUM_SOURCE = """\
// GENERATED FILE - DO NOT EDIT.
// Generated by {script_name} using data from {data_source_name}.
//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// entry_points_enum_autogen.cpp:
//   Helper methods for the {lib} entry points enumeration.

#include "common/entry_points_enum_autogen.h"

#include "common/debug.h"

namespace angle
{{
const char *GetEntryPointName(EntryPoint ep)
{{
    switch (ep)
    {{
{entry_points_name_cases}
        default:
            UNREACHABLE();
            return "error";
    }}
}}
}}  // namespace angle
"""

TEMPLATE_LIB_ENTRY_POINT_SOURCE = """\
// GENERATED FILE - DO NOT EDIT.
// Generated by {script_name} using data from {data_source_name}.
//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// {lib_name}_autogen.cpp: Implements the exported {lib_description} functions.

{includes}
extern "C" {{
{entry_points}
}} // extern "C"
"""

TEMPLATE_ENTRY_POINT_DECL = """{angle_export}{return_type} {export_def} {name}({params});"""

TEMPLATE_GLES_ENTRY_POINT_NO_RETURN = """\
void GL_APIENTRY GL_{name}({params})
{{
    ASSERT(!egl::Display::GetCurrentThreadUnlockedTailCall()->any());
    Context *context = {context_getter};
    {event_comment}EVENT(context, GL{name}, "context = %d{comma_if_needed}{format_params}", CID(context){comma_if_needed}{pass_params});

    if ({valid_context_check})
    {{{packed_gl_enum_conversions}
        {context_lock}
        bool isCallValid = (context->skipValidation() || {validation_expression});
        if (isCallValid)
        {{
            context->{name_lower_no_suffix}({internal_params});
        }}
        ANGLE_CAPTURE_GL({name}, isCallValid, {gl_capture_params});
    }}
    else
    {{
        {constext_lost_error_generator}
    }}
    {epilog}
}}
"""

TEMPLATE_GLES_CONTEXT_PRIVATE_ENTRY_POINT_NO_RETURN = """\
void GL_APIENTRY GL_{name}({params})
{{
    ASSERT(!egl::Display::GetCurrentThreadUnlockedTailCall()->any());
    Context *context = {context_getter};
    {event_comment}EVENT(context, GL{name}, "context = %d{comma_if_needed}{format_params}", CID(context){comma_if_needed}{pass_params});

    if ({valid_context_check})
    {{{packed_gl_enum_conversions}
        bool isCallValid = (context->skipValidation() || {validation_expression});
        if (isCallValid)
        {{
            ContextPrivate{name_no_suffix}({context_private_internal_params});
        }}
        ANGLE_CAPTURE_GL({name}, isCallValid, {gl_capture_params});
    }}
    else
    {{
        {constext_lost_error_generator}
    }}
    ASSERT(!egl::Display::GetCurrentThreadUnlockedTailCall()->any());
}}
"""

TEMPLATE_GLES_ENTRY_POINT_WITH_RETURN = """\
{return_type} GL_APIENTRY GL_{name}({params})
{{
    ASSERT(!egl::Display::GetCurrentThreadUnlockedTailCall()->any());
    Context *context = {context_getter};
    {event_comment}EVENT(context, GL{name}, "context = %d{comma_if_needed}{format_params}", CID(context){comma_if_needed}{pass_params});

    {return_type} returnValue;
    if ({valid_context_check})
    {{{packed_gl_enum_conversions}
        {context_lock}
        bool isCallValid = (context->skipValidation() || {validation_expression});
        if (isCallValid)
        {{
            returnValue = context->{name_lower_no_suffix}({internal_params});
        }}
        else
        {{
            returnValue = GetDefaultReturnValue<angle::EntryPoint::GL{name}, {return_type}>();
        }}
        ANGLE_CAPTURE_GL({name}, isCallValid, {gl_capture_params}, returnValue);
    }}
    else
    {{
        {constext_lost_error_generator}
        returnValue = GetDefaultReturnValue<angle::EntryPoint::GL{name}, {return_type}>();
    }}
    {epilog}
    return returnValue;
}}
"""

TEMPLATE_GLES_CONTEXT_PRIVATE_ENTRY_POINT_WITH_RETURN = """\
{return_type} GL_APIENTRY GL_{name}({params})
{{
    ASSERT(!egl::Display::GetCurrentThreadUnlockedTailCall()->any());
    Context *context = {context_getter};
    {event_comment}EVENT(context, GL{name}, "context = %d{comma_if_needed}{format_params}", CID(context){comma_if_needed}{pass_params});

    {return_type} returnValue;
    if ({valid_context_check})
    {{{packed_gl_enum_conversions}
        bool isCallValid = (context->skipValidation() || {validation_expression});
        if (isCallValid)
        {{
            returnValue = ContextPrivate{name_no_suffix}({context_private_internal_params});
        }}
        else
        {{
            returnValue = GetDefaultReturnValue<angle::EntryPoint::GL{name}, {return_type}>();
        }}
        ANGLE_CAPTURE_GL({name}, isCallValid, {gl_capture_params}, returnValue);
    }}
    else
    {{
        {constext_lost_error_generator}
        returnValue = GetDefaultReturnValue<angle::EntryPoint::GL{name}, {return_type}>();
    }}
    ASSERT(!egl::Display::GetCurrentThreadUnlockedTailCall()->any());
    return returnValue;
}}
"""

TEMPLATE_EGL_ENTRY_POINT_NO_RETURN = """\
void EGLAPIENTRY EGL_{name}({params})
{{
    {preamble}
    Thread *thread = egl::GetCurrentThread();
    ASSERT(!egl::Display::GetCurrentThreadUnlockedTailCall()->any());
    {{
        ANGLE_SCOPED_GLOBAL_LOCK();
        EGL_EVENT({name}, "{format_params}"{comma_if_needed}{pass_params});

        {packed_gl_enum_conversions}

        {{
            ANGLE_EGL_SCOPED_CONTEXT_LOCK({name}, thread{comma_if_needed_context_lock}{internal_context_lock_params});
            if (IsEGLValidationEnabled())
            {{
                ANGLE_EGL_VALIDATE_VOID(thread, {name}, {labeled_object}, {internal_params});
            }}
            else
            {{
                {attrib_map_init}
            }}

            {name}(thread{comma_if_needed}{internal_params});
        }}

        ANGLE_CAPTURE_EGL({name}, true, {egl_capture_params});
    }}
    {epilog}
}}
"""

TEMPLATE_EGL_ENTRY_POINT_NO_RETURN_NO_LOCKS = """\
void EGLAPIENTRY EGL_{name}({params})
{{
    {preamble}
    Thread *thread = egl::GetCurrentThread();
    ASSERT(!egl::Display::GetCurrentThreadUnlockedTailCall()->any());

    EGL_EVENT({name}, "{format_params}"{comma_if_needed}{pass_params});

    {packed_gl_enum_conversions}

    {{
        if (IsEGLValidationEnabled())
        {{
            ANGLE_EGL_VALIDATE_VOID(thread, {name}, {labeled_object}, {internal_params});
        }}
        else
        {{
            {attrib_map_init}
        }}

        {name}(thread{comma_if_needed}{internal_params});
    }}

    ANGLE_CAPTURE_EGL({name}, true, {egl_capture_params});
    {epilog}
}}
"""

TEMPLATE_EGL_ENTRY_POINT_WITH_RETURN = """\
{return_type} EGLAPIENTRY EGL_{name}({params})
{{
    {preamble}
    Thread *thread = egl::GetCurrentThread();
    ASSERT(!egl::Display::GetCurrentThreadUnlockedTailCall()->any());
    {return_type} returnValue;
    {{
        {egl_lock}
        EGL_EVENT({name}, "{format_params}"{comma_if_needed}{pass_params});

        {packed_gl_enum_conversions}

        {{
            ANGLE_EGL_SCOPED_CONTEXT_LOCK({name}, thread{comma_if_needed_context_lock}{internal_context_lock_params});
            if (IsEGLValidationEnabled())
            {{
                ANGLE_EGL_VALIDATE(thread, {name}, {labeled_object}, {return_type}{comma_if_needed}{internal_params});
            }}
            else
            {{
                {attrib_map_init}
            }}

            returnValue = {name}(thread{comma_if_needed}{internal_params});
        }}

        ANGLE_CAPTURE_EGL({name}, true, {egl_capture_params}, returnValue);
    }}
    {epilog}
    return returnValue;
}}
"""

TEMPLATE_EGL_ENTRY_POINT_WITH_RETURN_NO_LOCKS = """\
{return_type} EGLAPIENTRY EGL_{name}({params})
{{
    {preamble}
    Thread *thread = egl::GetCurrentThread();
    ASSERT(!egl::Display::GetCurrentThreadUnlockedTailCall()->any());
    {return_type} returnValue;

    EGL_EVENT({name}, "{format_params}"{comma_if_needed}{pass_params});

    {packed_gl_enum_conversions}

    if (IsEGLValidationEnabled())
    {{
        ANGLE_EGL_VALIDATE(thread, {name}, {labeled_object}, {return_type}{comma_if_needed}{internal_params});
    }}
    else
    {{
        {attrib_map_init}
    }}

    returnValue = {name}(thread{comma_if_needed}{internal_params});

    ANGLE_CAPTURE_EGL({name}, true, {egl_capture_params}, returnValue);

    {epilog}
    return returnValue;
}}
"""

TEMPLATE_CL_ENTRY_POINT_NO_RETURN = """\
void CL_API_CALL cl{name}({params})
{{
    CL_EVENT({name}, "{format_params}"{comma_if_needed}{pass_params});

    {packed_gl_enum_conversions}

    ANGLE_CL_VALIDATE_VOID({name}{comma_if_needed}{internal_params});

    cl::gClErrorTls = CL_SUCCESS;
    {name}({internal_params});
}}
"""

TEMPLATE_CL_ENTRY_POINT_WITH_RETURN_ERROR = """\
cl_int CL_API_CALL cl{name}({params})
{{{initialization}
    CL_EVENT({name}, "{format_params}"{comma_if_needed}{pass_params});

    {packed_gl_enum_conversions}

    ANGLE_CL_VALIDATE_ERROR({name}{comma_if_needed}{internal_params});

    cl::gClErrorTls = CL_SUCCESS;
    return {name}({internal_params});
}}
"""

TEMPLATE_CL_ENTRY_POINT_WITH_ERRCODE_RET = """\
{return_type} CL_API_CALL cl{name}({params})
{{{initialization}
    CL_EVENT({name}, "{format_params}"{comma_if_needed}{pass_params});

    {packed_gl_enum_conversions}

    ANGLE_CL_VALIDATE_ERRCODE_RET({name}{comma_if_needed}{internal_params});

    cl::gClErrorTls      = CL_SUCCESS;
    {return_type} object = {name}({internal_params});

    ASSERT((cl::gClErrorTls == CL_SUCCESS) == (object != nullptr));
    if (errcode_ret != nullptr)
    {{
        *errcode_ret = cl::gClErrorTls;
    }}
    return object;
}}
"""

TEMPLATE_CL_ENTRY_POINT_WITH_RETURN_POINTER = """\
{return_type} CL_API_CALL cl{name}({params})
{{{initialization}
    CL_EVENT({name}, "{format_params}"{comma_if_needed}{pass_params});

    {packed_gl_enum_conversions}

    cl::gClErrorTls = CL_SUCCESS;
    ANGLE_CL_VALIDATE_POINTER({name}{comma_if_needed}{internal_params});

    return {name}({internal_params});
}}
"""

TEMPLATE_CL_STUBS_HEADER = """\
// GENERATED FILE - DO NOT EDIT.
// Generated by {script_name} using data from {data_source_name}.
//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// {annotation_lower}_stubs_autogen.h: Stubs for {title} entry points.

#ifndef LIBGLESV2_{annotation_upper}_STUBS_AUTOGEN_H_
#define LIBGLESV2_{annotation_upper}_STUBS_AUTOGEN_H_

#include "libANGLE/cl_types.h"

namespace cl
{{
{stubs}
}}  // namespace cl
#endif  // LIBGLESV2_{annotation_upper}_STUBS_AUTOGEN_H_
"""

TEMPLATE_EGL_STUBS_HEADER = """\
// GENERATED FILE - DO NOT EDIT.
// Generated by {script_name} using data from {data_source_name}.
//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// {annotation_lower}_stubs_autogen.h: Stubs for {title} entry points.

#ifndef LIBGLESV2_{annotation_upper}_STUBS_AUTOGEN_H_
#define LIBGLESV2_{annotation_upper}_STUBS_AUTOGEN_H_

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "common/PackedEnums.h"
#include "common/PackedEGLEnums_autogen.h"

namespace gl
{{
class Context;
}}  // namespace gl

namespace egl
{{
class AttributeMap;
class Device;
class Display;
class Image;
class Stream;
class Surface;
class Sync;
class Thread;
struct Config;

{stubs}
}}  // namespace egl
#endif  // LIBGLESV2_{annotation_upper}_STUBS_AUTOGEN_H_
"""

CONTEXT_HEADER = """\
// GENERATED FILE - DO NOT EDIT.
// Generated by {script_name} using data from {data_source_name}.
//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Context_{annotation_lower}_autogen.h: Creates a macro for interfaces in Context.

#ifndef ANGLE_CONTEXT_{annotation_upper}_AUTOGEN_H_
#define ANGLE_CONTEXT_{annotation_upper}_AUTOGEN_H_

#define ANGLE_{annotation_upper}_CONTEXT_API \\
{interface}

#endif // ANGLE_CONTEXT_API_{version}_AUTOGEN_H_
"""

CONTEXT_DECL_FORMAT = """    {return_type} {name_lower_no_suffix}({internal_params}){maybe_const}; \\"""

TEMPLATE_CL_ENTRY_POINT_EXPORT = """\
{return_type} CL_API_CALL cl{name}({params})
{{
    return cl::GetDispatch().cl{name}({internal_params});
}}
"""

TEMPLATE_GL_ENTRY_POINT_EXPORT = """\
{return_type} GL_APIENTRY gl{name}({params})
{{
    return GL_{name}({internal_params});
}}
"""

TEMPLATE_EGL_ENTRY_POINT_EXPORT = """\
{return_type} EGLAPIENTRY egl{name}({params})
{{
    EnsureEGLLoaded();
    return EGL_{name}({internal_params});
}}
"""

TEMPLATE_GLEXT_FUNCTION_POINTER = """typedef {return_type}(GL_APIENTRYP PFN{name_upper}PROC)({params});"""
TEMPLATE_GLEXT_FUNCTION_PROTOTYPE = """{apicall} {return_type}GL_APIENTRY {name}({params});"""

TEMPLATE_GL_VALIDATION_HEADER = """\
// GENERATED FILE - DO NOT EDIT.
// Generated by {script_name} using data from {data_source_name}.
//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// validation{annotation}_autogen.h:
//   Validation functions for the OpenGL {comment} entry points.

#ifndef LIBANGLE_VALIDATION_{annotation}_AUTOGEN_H_
#define LIBANGLE_VALIDATION_{annotation}_AUTOGEN_H_

#include "common/entry_points_enum_autogen.h"
#include "common/PackedEnums.h"

namespace gl
{{
class Context;
class PrivateState;
class ErrorSet;

{prototypes}
}}  // namespace gl

#endif  // LIBANGLE_VALIDATION_{annotation}_AUTOGEN_H_
"""

TEMPLATE_CL_VALIDATION_HEADER = """\
// GENERATED FILE - DO NOT EDIT.
// Generated by {script_name} using data from {data_source_name}.
//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// validation{annotation}_autogen.h:
//   Validation functions for the {comment} entry points.

#ifndef LIBANGLE_VALIDATION_{annotation}_AUTOGEN_H_
#define LIBANGLE_VALIDATION_{annotation}_AUTOGEN_H_

#include "libANGLE/validationCL.h"

namespace cl
{{
{prototypes}
}}  // namespace cl

#endif  // LIBANGLE_VALIDATION_{annotation}_AUTOGEN_H_
"""

TEMPLATE_EGL_VALIDATION_HEADER = """\
// GENERATED FILE - DO NOT EDIT.
// Generated by {script_name} using data from {data_source_name}.
//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// validation{annotation}_autogen.h:
//   Validation functions for the {comment} entry points.

#ifndef LIBANGLE_VALIDATION_{annotation}_AUTOGEN_H_
#define LIBANGLE_VALIDATION_{annotation}_AUTOGEN_H_

#include "libANGLE/validationEGL.h"

namespace egl
{{
{prototypes}
}}  // namespace egl

#endif  // LIBANGLE_VALIDATION_{annotation}_AUTOGEN_H_
"""

TEMPLATE_CONTEXT_PRIVATE_CALL_HEADER = """\
// GENERATED FILE - DO NOT EDIT.
// Generated by {script_name} using data from {data_source_name}.
//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// context_private_call_autogen.h:
//   Helpers that set/get state that is entirely privately accessed by the context.

#ifndef LIBANGLE_CONTEXT_PRIVATE_CALL_AUTOGEN_H_
#define LIBANGLE_CONTEXT_PRIVATE_CALL_AUTOGEN_H_

#include "libANGLE/Context.h"

namespace gl
{{
{prototypes}
}}  // namespace gl

#endif  // LIBANGLE_CONTEXT_PRIVATE_CALL_AUTOGEN_H_
"""

TEMPLATE_EGL_CONTEXT_LOCK_HEADER = """\
// GENERATED FILE - DO NOT EDIT.
// Generated by {script_name} using data from {data_source_name}.
//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// {annotation_lower}_context_lock_autogen.h:
//   Context Lock functions for the {comment} entry points.

#ifndef LIBGLESV2_{annotation_upper}_CONTEXT_LOCK_AUTOGEN_H_
#define LIBGLESV2_{annotation_upper}_CONTEXT_LOCK_AUTOGEN_H_

#include "libGLESv2/global_state.h"

namespace egl
{{
{prototypes}
}}  // namespace egl

#endif  // LIBGLESV2_{annotation_upper}_CONTEXT_LOCK_AUTOGEN_H_
"""

TEMPLATE_CAPTURE_HEADER = """\
// GENERATED FILE - DO NOT EDIT.
// Generated by {script_name} using data from {data_source_name}.
//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// capture_{annotation_lower}_autogen.h:
//   Capture functions for the OpenGL ES {comment} entry points.

#ifndef LIBANGLE_CAPTURE_{annotation_upper}_AUTOGEN_H_
#define LIBANGLE_CAPTURE_{annotation_upper}_AUTOGEN_H_

#include "common/PackedEnums.h"
#include "libANGLE/capture/FrameCapture.h"

namespace {namespace}
{{
{prototypes}
}}  // namespace {namespace}

#endif  // LIBANGLE_CAPTURE_{annotation_upper}_AUTOGEN_H_
"""

TEMPLATE_CAPTURE_SOURCE = """\
// GENERATED FILE - DO NOT EDIT.
// Generated by {script_name} using data from {data_source_name}.
//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// capture_{annotation_with_dash}_autogen.cpp:
//   Capture functions for the OpenGL ES {comment} entry points.

#include "libANGLE/capture/capture_{annotation_with_dash}_autogen.h"

#include "common/gl_enum_utils.h"
#include "libANGLE/Context.h"
#include "libANGLE/capture/FrameCapture.h"
#include "libANGLE/validation{annotation_no_dash}.h"

using namespace angle;

namespace {namespace}
{{
{capture_methods}
}}  // namespace {namespace}
"""

TEMPLATE_CAPTURE_METHOD_WITH_RETURN_VALUE = """\
CallCapture Capture{short_name}({params_with_type}, {return_value_type_original} returnValue)
{{
    ParamBuffer paramBuffer;

    {parameter_captures}

    ParamCapture returnValueCapture("returnValue", ParamType::T{return_value_type_custom});
    InitParamValue(ParamType::T{return_value_type_custom}, returnValue, &returnValueCapture.value);
    paramBuffer.addReturnValue(std::move(returnValueCapture));

    return CallCapture(angle::EntryPoint::{api_upper}{short_name}, std::move(paramBuffer));
}}
"""

TEMPLATE_CAPTURE_METHOD_NO_RETURN_VALUE = """\
CallCapture Capture{short_name}({params_with_type})
{{
    ParamBuffer paramBuffer;

    {parameter_captures}

    return CallCapture(angle::EntryPoint::{api_upper}{short_name}, std::move(paramBuffer));
}}
"""

TEMPLATE_PARAMETER_CAPTURE_VALUE = """paramBuffer.addValueParam("{name}", ParamType::T{type}, {name});"""

TEMPLATE_PARAMETER_CAPTURE_GL_ENUM = """paramBuffer.addEnumParam("{name}", {api_enum}::{group}, ParamType::T{type}, {name});"""

TEMPLATE_PARAMETER_CAPTURE_POINTER = """
    if (isCallValid)
    {{
        ParamCapture {name}Param("{name}", ParamType::T{type});
        InitParamValue(ParamType::T{type}, {name}, &{name}Param.value);
        {capture_name}({params}, &{name}Param);
        paramBuffer.addParam(std::move({name}Param));
    }}
    else
    {{
        ParamCapture {name}Param("{name}", ParamType::T{type});
        InitParamValue(ParamType::T{type}, static_cast<{cast_type}>(nullptr), &{name}Param.value);
        paramBuffer.addParam(std::move({name}Param));
    }}
"""

TEMPLATE_PARAMETER_CAPTURE_POINTER_FUNC = """void {name}({params});"""

TEMPLATE_CAPTURE_REPLAY_SOURCE = """\
// GENERATED FILE - DO NOT EDIT.
// Generated by {script_name} using data from {data_source_name}.
//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// frame_capture_replay_autogen.cpp:
//   Replay captured GL calls.

#include "angle_trace_gl.h"
#include "common/debug.h"
#include "common/frame_capture_utils.h"
#include "frame_capture_test_utils.h"

namespace angle
{{
void ReplayTraceFunctionCall(const CallCapture &call, const TraceFunctionMap &customFunctions)
{{
    const ParamBuffer &params = call.params;
    const std::vector<ParamCapture> &captures = params.getParamCaptures();

    switch (call.entryPoint)
    {{
{call_replay_cases}
        default:
            ASSERT(!call.customFunctionName.empty());
            ReplayCustomFunctionCall(call, customFunctions);
            break;
    }}
}}

}}  // namespace angle

"""

TEMPLATE_REPLAY_CALL_CASE = """\
        case angle::EntryPoint::{enum}:
            {call}({params});
            break;
"""

POINTER_FORMAT = "0x%016\" PRIxPTR \""
UNSIGNED_LONG_LONG_FORMAT = "%llu"
HEX_LONG_LONG_FORMAT = "0x%llX"

FORMAT_DICT = {
    "GLbitfield": "%s",
    "GLboolean": "%s",
    "GLbyte": "%d",
    "GLclampx": "0x%X",
    "GLDEBUGPROC": POINTER_FORMAT,
    "GLDEBUGPROCKHR": POINTER_FORMAT,
    "GLdouble": "%f",
    "GLeglClientBufferEXT": POINTER_FORMAT,
    "GLeglImageOES": POINTER_FORMAT,
    "GLenum": "%s",
    "GLfixed": "0x%X",
    "GLfloat": "%f",
    "GLGETBLOBPROCANGLE": POINTER_FORMAT,
    "GLint": "%d",
    "GLintptr": UNSIGNED_LONG_LONG_FORMAT,
    "GLSETBLOBPROCANGLE": POINTER_FORMAT,
    "GLshort": "%d",
    "GLsizei": "%d",
    "GLsizeiptr": UNSIGNED_LONG_LONG_FORMAT,
    "GLsync": POINTER_FORMAT,
    "GLubyte": "%d",
    "GLuint": "%u",
    "GLuint64": UNSIGNED_LONG_LONG_FORMAT,
    "GLushort": "%u",
    "int": "%d",
    # EGL-specific types
    "EGLBoolean": "%u",
    "EGLConfig": POINTER_FORMAT,
    "EGLContext": POINTER_FORMAT,
    "EGLDisplay": POINTER_FORMAT,
    "EGLSurface": POINTER_FORMAT,
    "EGLSync": POINTER_FORMAT,
    "EGLNativeDisplayType": POINTER_FORMAT,
    "EGLNativePixmapType": POINTER_FORMAT,
    "EGLNativeWindowType": POINTER_FORMAT,
    "EGLClientBuffer": POINTER_FORMAT,
    "EGLenum": "0x%X",
    "EGLint": "%d",
    "EGLImage": POINTER_FORMAT,
    "EGLTime": UNSIGNED_LONG_LONG_FORMAT,
    "EGLGetBlobFuncANDROID": POINTER_FORMAT,
    "EGLSetBlobFuncANDROID": POINTER_FORMAT,
    "EGLuint64KHR": UNSIGNED_LONG_LONG_FORMAT,
    "EGLSyncKHR": POINTER_FORMAT,
    "EGLnsecsANDROID": UNSIGNED_LONG_LONG_FORMAT,
    "EGLDeviceEXT": POINTER_FORMAT,
    "EGLDEBUGPROCKHR": POINTER_FORMAT,
    "EGLObjectKHR": POINTER_FORMAT,
    "EGLLabelKHR": POINTER_FORMAT,
    "EGLTimeKHR": UNSIGNED_LONG_LONG_FORMAT,
    "EGLImageKHR": POINTER_FORMAT,
    "EGLStreamKHR": POINTER_FORMAT,
    "EGLFrameTokenANGLE": HEX_LONG_LONG_FORMAT,
    # CL-specific types
    "size_t": "%zu",
    "cl_char": "%hhd",
    "cl_uchar": "%hhu",
    "cl_short": "%hd",
    "cl_ushort": "%hu",
    "cl_int": "%d",
    "cl_uint": "%u",
    "cl_long": "%lld",
    "cl_ulong": "%llu",
    "cl_half": "%hu",
    "cl_float": "%f",
    "cl_double": "%f",
    "cl_platform_id": POINTER_FORMAT,
    "cl_device_id": POINTER_FORMAT,
    "cl_context": POINTER_FORMAT,
    "cl_command_queue": POINTER_FORMAT,
    "cl_mem": POINTER_FORMAT,
    "cl_program": POINTER_FORMAT,
    "cl_kernel": POINTER_FORMAT,
    "cl_event": POINTER_FORMAT,
    "cl_sampler": POINTER_FORMAT,
    "cl_bool": "%u",
    "cl_bitfield": "%llu",
    "cl_properties": "%llu",
    "cl_device_type": "%llu",
    "cl_platform_info": "%u",
    "cl_device_info": "%u",
    "cl_device_fp_config": "%llu",
    "cl_device_mem_cache_type": "%u",
    "cl_device_local_mem_type": "%u",
    "cl_device_exec_capabilities": "%llu",
    "cl_device_svm_capabilities": "%llu",
    "cl_command_queue_properties": "%llu",
    "cl_device_partition_property": "%zu",
    "cl_device_affinity_domain": "%llu",
    "cl_context_properties": "%zu",
    "cl_context_info": "%u",
    "cl_queue_properties": "%llu",
    "cl_command_queue_info": "%u",
    "cl_channel_order": "%u",
    "cl_channel_type": "%u",
    "cl_mem_flags": "%llu",
    "cl_svm_mem_flags": "%llu",
    "cl_mem_object_type": "%u",
    "cl_mem_info": "%u",
    "cl_mem_migration_flags": "%llu",
    "cl_mem_properties": "%llu",
    "cl_image_info": "%u",
    "cl_buffer_create_type": "%u",
    "cl_addressing_mode": "%u",
    "cl_filter_mode": "%u",
    "cl_sampler_info": "%u",
    "cl_map_flags": "%llu",
    "cl_pipe_properties": "%zu",
    "cl_pipe_info": "%u",
    "cl_program_info": "%u",
    "cl_program_build_info": "%u",
    "cl_program_binary_type": "%u",
    "cl_build_status": "%d",
    "cl_kernel_info": "%u",
    "cl_kernel_arg_info": "%u",
    "cl_kernel_arg_address_qualifier": "%u",
    "cl_kernel_arg_access_qualifier": "%u",
    "cl_kernel_arg_type_qualifier": "%llu",
    "cl_kernel_work_group_info": "%u",
    "cl_kernel_sub_group_info": "%u",
    "cl_event_info": "%u",
    "cl_command_type": "%u",
    "cl_profiling_info": "%u",
    "cl_sampler_properties": "%llu",
    "cl_kernel_exec_info": "%u",
    "cl_device_atomic_capabilities": "%llu",
    "cl_khronos_vendor_id": "%u",
    "cl_version": "%u",
    "cl_device_device_enqueue_capabilities": "%llu",
}

TEMPLATE_HEADER_INCLUDES = """\
#include <GLES{major}/gl{major}{minor}.h>
#include <export.h>"""

TEMPLATE_SOURCES_INCLUDES = """\
#include "libGLESv2/entry_points_{header_version}_autogen.h"

#include "common/entry_points_enum_autogen.h"
#include "common/gl_enum_utils.h"
#include "libANGLE/Context.h"
#include "libANGLE/Context.inl.h"
#include "libANGLE/context_private_call_autogen.h"
#include "libANGLE/context_private_call.inl.h"
#include "libANGLE/capture/capture_{header_version}_autogen.h"
#include "libANGLE/validation{validation_header_version}.h"
#include "libANGLE/entry_points_utils.h"
#include "libGLESv2/global_state.h"

using namespace gl;
"""

GLES_EXT_HEADER_INCLUDES = TEMPLATE_HEADER_INCLUDES.format(
    major="", minor="") + """
#include <GLES/glext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl32.h>
"""

GLES_EXT_SOURCE_INCLUDES = TEMPLATE_SOURCES_INCLUDES.format(
    header_version="gles_ext", validation_header_version="ESEXT") + """
#include "libANGLE/capture/capture_gles_1_0_autogen.h"
#include "libANGLE/capture/capture_gles_2_0_autogen.h"
#include "libANGLE/capture/capture_gles_3_0_autogen.h"
#include "libANGLE/capture/capture_gles_3_1_autogen.h"
#include "libANGLE/capture/capture_gles_3_2_autogen.h"
#include "libANGLE/validationES1.h"
#include "libANGLE/validationES2.h"
#include "libANGLE/validationES3.h"
#include "libANGLE/validationES31.h"
#include "libANGLE/validationES32.h"

using namespace gl;
"""

EGL_HEADER_INCLUDES = """\
#include <EGL/egl.h>
#include <export.h>
"""

EGL_SOURCE_INCLUDES = """\
#include "libGLESv2/entry_points_egl_autogen.h"
#include "libGLESv2/entry_points_egl_ext_autogen.h"

#include "libANGLE/capture/capture_egl_autogen.h"
#include "libANGLE/entry_points_utils.h"
#include "libANGLE/validationEGL_autogen.h"
#include "libGLESv2/egl_context_lock_impl.h"
#include "libGLESv2/egl_stubs_autogen.h"
#include "libGLESv2/egl_ext_stubs_autogen.h"
#include "libGLESv2/global_state.h"

using namespace egl;
"""

EGL_EXT_HEADER_INCLUDES = """\
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <export.h>
"""

EGL_EXT_SOURCE_INCLUDES = """\
#include "libGLESv2/entry_points_egl_ext_autogen.h"

#include "libANGLE/capture/capture_egl_autogen.h"
#include "libANGLE/entry_points_utils.h"
#include "libANGLE/validationEGL_autogen.h"
#include "libGLESv2/egl_context_lock_impl.h"
#include "libGLESv2/egl_ext_stubs_autogen.h"
#include "libGLESv2/global_state.h"

using namespace egl;
"""

LIBCL_EXPORT_INCLUDES = """
#include "libOpenCL/dispatch.h"
"""

LIBGLESV2_EXPORT_INCLUDES = """
#include "angle_gl.h"

#include "libGLESv2/entry_points_gles_1_0_autogen.h"
#include "libGLESv2/entry_points_gles_2_0_autogen.h"
#include "libGLESv2/entry_points_gles_3_0_autogen.h"
#include "libGLESv2/entry_points_gles_3_1_autogen.h"
#include "libGLESv2/entry_points_gles_3_2_autogen.h"
#include "libGLESv2/entry_points_gles_ext_autogen.h"

#include "common/event_tracer.h"
"""

LIBEGL_EXPORT_INCLUDES_AND_PREAMBLE = """
#include "anglebase/no_destructor.h"
#include "common/system_utils.h"

#include <memory>

#if defined(ANGLE_USE_EGL_LOADER)
#    include "libEGL/egl_loader_autogen.h"
#else
#    include "libGLESv2/entry_points_egl_autogen.h"
#    include "libGLESv2/entry_points_egl_ext_autogen.h"
#endif  // defined(ANGLE_USE_EGL_LOADER)

namespace
{
#if defined(ANGLE_USE_EGL_LOADER)
bool gLoaded = false;
void *gEntryPointsLib = nullptr;

GenericProc KHRONOS_APIENTRY GlobalLoad(const char *symbol)
{
    return reinterpret_cast<GenericProc>(angle::GetLibrarySymbol(gEntryPointsLib, symbol));
}

void EnsureEGLLoaded()
{
    if (gLoaded)
    {
        return;
    }

    std::string errorOut;
    gEntryPointsLib = OpenSystemLibraryAndGetError(ANGLE_DISPATCH_LIBRARY, angle::SearchType::ModuleDir, &errorOut);
    if (gEntryPointsLib)
    {
        LoadLibEGL_EGL(GlobalLoad);
        gLoaded = true;
    }
    else
    {
        fprintf(stderr, "Error loading EGL entry points: %s\\n", errorOut.c_str());
    }
}
#else
void EnsureEGLLoaded() {}
#endif  // defined(ANGLE_USE_EGL_LOADER)
}  // anonymous namespace
"""

LIBCL_HEADER_INCLUDES = """\
#include "angle_cl.h"
"""

LIBCL_SOURCE_INCLUDES = """\
#include "libGLESv2/entry_points_cl_autogen.h"

#include "libANGLE/validationCL_autogen.h"
#include "libGLESv2/cl_stubs_autogen.h"
#include "libGLESv2/entry_points_cl_utils.h"
"""

TEMPLATE_EVENT_COMMENT = """\
    // Don't run the EVENT() macro on the EXT_debug_marker entry points.
    // It can interfere with the debug events being set by the caller.
    // """

TEMPLATE_CAPTURE_PROTO = "angle::CallCapture Capture%s(%s);"

TEMPLATE_VALIDATION_PROTO = "%s Validate%s(%s);"

TEMPLATE_CONTEXT_PRIVATE_CALL_PROTO = "%s ContextPrivate%s(%s);"

TEMPLATE_CONTEXT_LOCK_PROTO = "ScopedContextMutexLock GetContextLock_%s(%s);"

TEMPLATE_WINDOWS_DEF_FILE = """\
; GENERATED FILE - DO NOT EDIT.
; Generated by {script_name} using data from {data_source_name}.
;
; Copyright 2020 The ANGLE Project Authors. All rights reserved.
; Use of this source code is governed by a BSD-style license that can be
; found in the LICENSE file.
LIBRARY {lib}
EXPORTS
{exports}
"""

TEMPLATE_FRAME_CAPTURE_UTILS_HEADER = """\
// GENERATED FILE - DO NOT EDIT.
// Generated by {script_name} using data from {data_source_name}.
//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// frame_capture_utils_autogen.h:
//   ANGLE Frame capture types and helper functions.

#ifndef COMMON_FRAME_CAPTURE_UTILS_AUTOGEN_H_
#define COMMON_FRAME_CAPTURE_UTILS_AUTOGEN_H_

#include "common/PackedEnums.h"

namespace angle
{{
enum class ParamType
{{
    {param_types}
}};

constexpr uint32_t kParamTypeCount = {param_type_count};

union ParamValue
{{
    {param_union_values}
}};

template <ParamType PType, typename T>
T GetParamVal(const ParamValue &value);

{get_param_val_specializations}

template <ParamType PType, typename T>
T GetParamVal(const ParamValue &value)
{{
    UNREACHABLE();
    return T();
}}

template <typename T>
T AccessParamValue(ParamType paramType, const ParamValue &value)
{{
    switch (paramType)
    {{
{access_param_value_cases}
    }}
    UNREACHABLE();
    return T();
}}

template <ParamType PType, typename T>
void SetParamVal(T valueIn, ParamValue *valueOut);

{set_param_val_specializations}

template <ParamType PType, typename T>
void SetParamVal(T valueIn, ParamValue *valueOut)
{{
    UNREACHABLE();
}}

template <typename T>
void InitParamValue(ParamType paramType, T valueIn, ParamValue *valueOut)
{{
    switch (paramType)
    {{
{init_param_value_cases}
    }}
}}

struct CallCapture;
struct ParamCapture;

void WriteParamCaptureReplay(std::ostream &os, const CallCapture &call, const ParamCapture &param);
const char *ParamTypeToString(ParamType paramType);

enum class ResourceIDType
{{
    {resource_id_types}
}};

ResourceIDType GetResourceIDTypeFromParamType(ParamType paramType);
const char *GetResourceIDTypeName(ResourceIDType resourceIDType);

template <typename ResourceType>
struct GetResourceIDTypeFromType;

{type_to_resource_id_type_structs}
}}  // namespace angle

#endif  // COMMON_FRAME_CAPTURE_UTILS_AUTOGEN_H_
"""

TEMPLATE_FRAME_CAPTURE_UTILS_SOURCE = """\
// GENERATED FILE - DO NOT EDIT.
// Generated by {script_name} using data from {data_source_name}.
//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// frame_capture_utils_autogen.cpp:
//   ANGLE Frame capture types and helper functions.

#include "common/frame_capture_utils_autogen.h"

#include "common/frame_capture_utils.h"

namespace angle
{{
void WriteParamCaptureReplay(std::ostream &os, const CallCapture &call, const ParamCapture &param)
{{
    switch (param.type)
    {{
{write_param_type_to_stream_cases}
        default:
            os << "unknown";
            break;
    }}
}}

const char *ParamTypeToString(ParamType paramType)
{{
    switch (paramType)
    {{
{param_type_to_string_cases}
        default:
            UNREACHABLE();
            return "unknown";
    }}
}}

ResourceIDType GetResourceIDTypeFromParamType(ParamType paramType)
{{
    switch (paramType)
    {{
{param_type_resource_id_cases}
        default:
            return ResourceIDType::InvalidEnum;
    }}
}}

const char *GetResourceIDTypeName(ResourceIDType resourceIDType)
{{
    switch (resourceIDType)
    {{
{resource_id_type_name_cases}
        default:
            UNREACHABLE();
            return "GetResourceIDTypeName error";
    }}
}}
}}  // namespace angle
"""

TEMPLATE_GET_PARAM_VAL_SPECIALIZATION = """\
template <>
inline {type} GetParamVal<ParamType::T{enum}, {type}>(const ParamValue &value)
{{
    return value.{union_name};
}}"""

TEMPLATE_ACCESS_PARAM_VALUE_CASE = """\
        case ParamType::T{enum}:
            return GetParamVal<ParamType::T{enum}, T>(value);"""

TEMPLATE_SET_PARAM_VAL_SPECIALIZATION = """\
template <>
inline void SetParamVal<ParamType::T{enum}>({type} valueIn, ParamValue *valueOut)
{{
    valueOut->{union_name} = valueIn;
}}"""

TEMPLATE_INIT_PARAM_VALUE_CASE = """\
        case ParamType::T{enum}:
            SetParamVal<ParamType::T{enum}>(valueIn, valueOut);
            break;"""

TEMPLATE_WRITE_PARAM_TYPE_TO_STREAM_CASE = """\
        case ParamType::T{enum_in}:
            WriteParamValueReplay<ParamType::T{enum_out}>(os, call, param.value.{union_name});
            break;"""

TEMPLATE_PARAM_TYPE_TO_STRING_CASE = """\
        case ParamType::T{enum}:
            return "{type}";"""

TEMPLATE_PARAM_TYPE_TO_RESOURCE_ID_TYPE_CASE = """\
        case ParamType::T{enum}:
            return ResourceIDType::{resource_id_type};"""

TEMPLATE_RESOURCE_ID_TYPE_NAME_CASE = """\
        case ResourceIDType::{resource_id_type}:
            return "{resource_id_type}";"""

CL_PACKED_TYPES = {
    # Enums
    "cl_platform_info": "PlatformInfo",
    "cl_device_info": "DeviceInfo",
    "cl_context_info": "ContextInfo",
    "cl_command_queue_info": "CommandQueueInfo",
    "cl_mem_object_type": "MemObjectType",
    "cl_mem_info": "MemInfo",
    "cl_image_info": "ImageInfo",
    "cl_pipe_info": "PipeInfo",
    "cl_addressing_mode": "AddressingMode",
    "cl_filter_mode": "FilterMode",
    "cl_sampler_info": "SamplerInfo",
    "cl_program_info": "ProgramInfo",
    "cl_program_build_info": "ProgramBuildInfo",
    "cl_kernel_info": "KernelInfo",
    "cl_kernel_arg_info": "KernelArgInfo",
    "cl_kernel_work_group_info": "KernelWorkGroupInfo",
    "cl_kernel_sub_group_info": "KernelSubGroupInfo",
    "cl_kernel_exec_info": "KernelExecInfo",
    "cl_event_info": "EventInfo",
    "cl_profiling_info": "ProfilingInfo",
    # Bit fields
    "cl_device_type": "DeviceType",
    "cl_device_fp_config": "DeviceFpConfig",
    "cl_device_exec_capabilities": "DeviceExecCapabilities",
    "cl_device_svm_capabilities": "DeviceSvmCapabilities",
    "cl_command_queue_properties": "CommandQueueProperties",
    "cl_device_affinity_domain": "DeviceAffinityDomain",
    "cl_mem_flags": "MemFlags",
    "cl_svm_mem_flags": "SVM_MemFlags",
    "cl_mem_migration_flags": "MemMigrationFlags",
    "cl_map_flags": "MapFlags",
    "cl_kernel_arg_type_qualifier": "KernelArgTypeQualifier",
    "cl_device_atomic_capabilities": "DeviceAtomicCapabilities",
    "cl_device_device_enqueue_capabilities": "DeviceEnqueueCapabilities",
}

EGL_PACKED_TYPES = {
    "EGLContext": "gl::ContextID",
    "EGLConfig": "egl::Config *",
    "EGLDeviceEXT": "egl::Device *",
    "EGLDisplay": "egl::Display *",
    "EGLImage": "ImageID",
    "EGLImageKHR": "ImageID",
    "EGLStreamKHR": "egl::Stream *",
    "EGLSurface": "SurfaceID",
    "EGLSync": "egl::SyncID",
    "EGLSyncKHR": "egl::SyncID",
}

EGL_CONTEXT_LOCK_PARAM_TYPES_FILTER = ["Thread *", "egl::Display *", "gl::ContextID"]
EGL_CONTEXT_LOCK_PARAM_NAMES_FILTER = ["attribute", "flags"]

CAPTURE_BLOCKLIST = ['eglGetProcAddress']


def is_aliasing_excepted(api, cmd_name):
    # For simplicity, strip the prefix gl and lower the case of the first
    # letter.  This makes sure that all variants of the cmd_name that reach
    # here end up looking similar for the sake of looking up in ALIASING_EXCEPTIONS
    cmd_name = cmd_name[2:] if cmd_name.startswith('gl') else cmd_name
    cmd_name = cmd_name[0].lower() + cmd_name[1:]
    return api == apis.GLES and cmd_name in ALIASING_EXCEPTIONS


def is_allowed_with_active_pixel_local_storage(name):
    return name in PLS_ALLOW_LIST or any(
        [fnmatch.fnmatchcase(name, entry) for entry in PLS_ALLOW_WILDCARDS])


def is_context_private_state_command(api, name):
    name = strip_suffix(api, name)
    return name in CONTEXT_PRIVATE_LIST or any(
        [fnmatch.fnmatchcase(name, entry) for entry in CONTEXT_PRIVATE_WILDCARDS])


def is_lockless_egl_entry_point(cmd_name):
    if cmd_name in [
            "eglGetError", "eglGetCurrentContext", "eglGetCurrentSurface", "eglGetCurrentDisplay",
            "eglLockVulkanQueueANGLE", "eglUnlockVulkanQueueANGLE"
    ]:
        return True
    return False


def is_egl_sync_entry_point(cmd_name):
    if cmd_name in [
            "eglClientWaitSync", "eglCreateSync", "eglDestroySync", "eglGetSyncAttrib",
            "eglWaitSync", "eglCreateSyncKHR", "eglClientWaitSyncKHR",
            "eglDupNativeFenceFDANDROID", "eglCopyMetalSharedEventANGLE", "eglDestroySyncKHR",
            "eglGetSyncAttribKHR", "eglSignalSyncKHR", "eglWaitSyncKHR"
    ]:
        return True
    return False


# egl entry points whose code path writes to resources that can be accessed
# by both EGL Sync APIs and EGL Non-Sync APIs
def is_egl_entry_point_accessing_both_sync_and_non_sync_API_resources(cmd_name):
    if cmd_name in ["eglTerminate", "eglLabelObjectKHR", "eglReleaseThread", "eglInitialize"]:
        return True
    return False


def get_validation_expression(api, cmd_name, entry_point_name, internal_params, is_gles1):
    name = strip_api_prefix(cmd_name)
    private_params = ["context->getPrivateState()", "context->getMutableErrorSetForValidation()"]
    extra_params = private_params if is_context_private_state_command(api,
                                                                      cmd_name) else ["context"]
    expr = "Validate{name}({params})".format(
        name=name, params=", ".join(extra_params + [entry_point_name] + internal_params))
    if not is_gles1 and not is_allowed_with_active_pixel_local_storage(name):
        expr = "(ValidatePixelLocalStorageInactive({extra_params}, {entry_point_name}) && {expr})".format(
            extra_params=", ".join(private_params), entry_point_name=entry_point_name, expr=expr)
    return expr


def entry_point_export(api):
    if api == apis.CL:
        return ""
    return "ANGLE_EXPORT "


def entry_point_prefix(api):
    if api == apis.CL:
        return "cl"
    if api == apis.GLES:
        return "GL_"
    return api + "_"


def get_api_entry_def(api):
    if api == apis.EGL:
        return "EGLAPIENTRY"
    elif api == apis.CL:
        return "CL_API_CALL"
    else:
        return "GL_APIENTRY"


def get_stubs_header_template(api):
    if api == apis.CL:
        return TEMPLATE_CL_STUBS_HEADER
    elif api == apis.EGL:
        return TEMPLATE_EGL_STUBS_HEADER
    else:
        return ""


def format_entry_point_decl(api, cmd_name, proto, params):
    comma_if_needed = ", " if len(params) > 0 else ""
    stripped = strip_api_prefix(cmd_name)
    return TEMPLATE_ENTRY_POINT_DECL.format(
        angle_export=entry_point_export(api),
        export_def=get_api_entry_def(api),
        name="%s%s" % (entry_point_prefix(api), stripped),
        return_type=proto[:-len(cmd_name)].strip(),
        params=", ".join(params),
        comma_if_needed=comma_if_needed)


# Returns index range of identifier in function parameter
def find_name_range(param):

    def is_allowed_in_identifier(char):
        return char.isalpha() or char.isdigit() or char == "_"

    # If type is a function declaration, only search in first parentheses
    left_paren = param.find("(")
    if left_paren >= 0:
        min = left_paren + 1
        end = param.index(")")
    else:
        min = 0
        end = len(param)

    # Find last identifier in search range
    while end > min and not is_allowed_in_identifier(param[end - 1]):
        end -= 1
    if end == min:
        raise ValueError
    start = end - 1
    while start > min and is_allowed_in_identifier(param[start - 1]):
        start -= 1
    return start, end


def just_the_type(param):
    start, end = find_name_range(param)
    return param[:start].strip() + param[end:].strip()


def just_the_name(param):
    start, end = find_name_range(param)
    return param[start:end]


def make_param(param_type, param_name):

    def insert_name(param_type, param_name, pos):
        return param_type[:pos] + " " + param_name + param_type[pos:]

    # If type is a function declaration, insert identifier before first closing parentheses
    left_paren = param_type.find("(")
    if left_paren >= 0:
        right_paren = param_type.index(")")
        return insert_name(param_type, param_name, right_paren)

    # If type is an array declaration, insert identifier before brackets
    brackets = param_type.find("[")
    if brackets >= 0:
        return insert_name(param_type, param_name, brackets)

    # Otherwise just append identifier
    return param_type + " " + param_name


def just_the_type_packed(param, entry):
    name = just_the_name(param)
    if name in entry:
        return entry[name]
    else:
        return just_the_type(param)


def just_the_name_packed(param, reserved_set):
    name = just_the_name(param)
    if name in reserved_set:
        return name + 'Packed'
    else:
        return name


def is_unsigned_long_format(fmt):
    return fmt == UNSIGNED_LONG_LONG_FORMAT or fmt == HEX_LONG_LONG_FORMAT


def param_print_argument(api, command_node, param):
    name_only = just_the_name(param)
    type_only = just_the_type(param)

    if "*" not in param and type_only not in FORMAT_DICT:
        print(" ".join(param))
        raise Exception("Missing '%s %s' from '%s' entry point" %
                        (type_only, name_only, registry_xml.get_cmd_name(command_node)))

    if "*" in param or FORMAT_DICT[type_only] == POINTER_FORMAT:
        return "(uintptr_t)%s" % name_only

    if is_unsigned_long_format(FORMAT_DICT[type_only]):
        return "static_cast<unsigned long long>(%s)" % name_only

    if type_only == "GLboolean":
        return "GLbooleanToString(%s)" % name_only

    if type_only == "GLbitfield":
        group_name = find_gl_enum_group_in_command(command_node, name_only)
        return "GLbitfieldToString(%s::%s, %s).c_str()" % (api_enums[api], group_name, name_only)

    if type_only == "GLenum":
        group_name = find_gl_enum_group_in_command(command_node, name_only)
        return "GLenumToString(%s::%s, %s)" % (api_enums[api], group_name, name_only)

    return name_only


def param_format_string(param):
    if "*" in param:
        return just_the_name(param) + " = 0x%016\" PRIxPTR \""
    else:
        type_only = just_the_type(param)
        if type_only not in FORMAT_DICT:
            raise Exception(type_only + " is not a known type in 'FORMAT_DICT'")

        return just_the_name(param) + " = " + FORMAT_DICT[type_only]


def is_context_lost_acceptable_cmd(cmd_name):
    lost_context_acceptable_cmds = [
        "glGetError",
        "glGetSync",
        "glGetQueryObjecti",
        "glGetProgramiv",
        "glGetGraphicsResetStatus",
        "glGetShaderiv",
    ]

    for context_lost_entry_pont in lost_context_acceptable_cmds:
        if cmd_name.startswith(context_lost_entry_pont):
            return True
    return False


def get_context_getter_function(cmd_name):
    if is_context_lost_acceptable_cmd(cmd_name):
        return "GetGlobalContext()"

    return "GetValidGlobalContext()"


def get_valid_context_check(cmd_name):
    return "context"


def get_constext_lost_error_generator(cmd_name):
    # Don't generate context lost errors on commands that accept lost contexts
    if is_context_lost_acceptable_cmd(cmd_name):
        return ""

    return "GenerateContextLostErrorOnCurrentGlobalContext();"


def strip_suffix_always(api, name):
    for suffix in registry_xml.strip_suffixes:
        if name.endswith(suffix):
            name = name[0:-len(suffix)]
    return name


def strip_suffix(api, name):
    # For commands where aliasing is excepted, keep the suffix
    if is_aliasing_excepted(api, name):
        return name

    return strip_suffix_always(api, name)


def find_gl_enum_group_in_command(command_node, param_name):
    group_name = None
    for param_node in command_node.findall('./param'):
        if param_node.find('./name').text == param_name:
            group_name = param_node.attrib.get('group', None)
            break

    if group_name is None or group_name in registry_xml.unsupported_enum_group_names:
        group_name = registry_xml.default_enum_group_name

    return group_name


def get_packed_enums(api, cmd_packed_gl_enums, cmd_name, packed_param_types, params):
    # Always strip the suffix when querying packed enums.
    result = cmd_packed_gl_enums.get(strip_suffix_always(api, cmd_name), {})
    for param in params:
        param_type = just_the_type(param)
        if param_type in packed_param_types:
            result[just_the_name(param)] = packed_param_types[param_type]
    return result


def get_def_template(api, cmd_name, return_type, has_errcode_ret):
    if return_type == "void":
        if api == apis.EGL:
            if is_lockless_egl_entry_point(cmd_name):
                return TEMPLATE_EGL_ENTRY_POINT_NO_RETURN_NO_LOCKS
            else:
                return TEMPLATE_EGL_ENTRY_POINT_NO_RETURN
        elif api == apis.CL:
            return TEMPLATE_CL_ENTRY_POINT_NO_RETURN
        elif is_context_private_state_command(api, cmd_name):
            return TEMPLATE_GLES_CONTEXT_PRIVATE_ENTRY_POINT_NO_RETURN
        else:
            return TEMPLATE_GLES_ENTRY_POINT_NO_RETURN
    elif return_type == "cl_int":
        return TEMPLATE_CL_ENTRY_POINT_WITH_RETURN_ERROR
    else:
        if api == apis.EGL:
            if is_lockless_egl_entry_point(cmd_name):
                return TEMPLATE_EGL_ENTRY_POINT_WITH_RETURN_NO_LOCKS
            else:
                return TEMPLATE_EGL_ENTRY_POINT_WITH_RETURN
        elif api == apis.CL:
            if has_errcode_ret:
                return TEMPLATE_CL_ENTRY_POINT_WITH_ERRCODE_RET
            else:
                return TEMPLATE_CL_ENTRY_POINT_WITH_RETURN_POINTER
        elif is_context_private_state_command(api, cmd_name):
            return TEMPLATE_GLES_CONTEXT_PRIVATE_ENTRY_POINT_WITH_RETURN
        else:
            return TEMPLATE_GLES_ENTRY_POINT_WITH_RETURN


def format_entry_point_def(api, command_node, cmd_name, proto, params, cmd_packed_enums,
                           packed_param_types, ep_to_object, is_gles1):
    packed_enums = get_packed_enums(api, cmd_packed_enums, cmd_name, packed_param_types, params)
    internal_params = [just_the_name_packed(param, packed_enums) for param in params]
    if internal_params and internal_params[-1] == "errcode_ret":
        internal_params.pop()
        has_errcode_ret = True
    else:
        has_errcode_ret = False

    internal_context_lock_params = [
        just_the_name_packed(param, packed_enums)
        for param in params
        if just_the_type_packed(param, packed_enums) in EGL_CONTEXT_LOCK_PARAM_TYPES_FILTER or
        just_the_name_packed(param, packed_enums) in EGL_CONTEXT_LOCK_PARAM_NAMES_FILTER
    ]

    packed_gl_enum_conversions = []

    # egl::AttributeMap objects do not convert the raw input parameters to a map until they are
    # validated because it is possible to have unterminated attribute lists if one of the
    # attributes is invalid.
    # When validation is disabled, force the conversion from attribute list to map using
    # initializeWithoutValidation.
    attrib_map_init = []

    for param in params:
        name = just_the_name(param)

        if name in packed_enums:
            internal_name = name + "Packed"
            internal_type = packed_enums[name]
            packed_gl_enum_conversions += [
                "\n        " + internal_type + " " + internal_name + " = PackParam<" +
                internal_type + ">(" + name + ");"
            ]

            if 'AttributeMap' in internal_type:
                attrib_map_init.append(internal_name + ".initializeWithoutValidation();")

    pass_params = [param_print_argument(api, command_node, param) for param in params]
    format_params = [param_format_string(param) for param in params]
    return_type = proto[:-len(cmd_name)].strip()
    initialization = "InitBackEnds(%s);\n" % INIT_DICT[cmd_name] if cmd_name in INIT_DICT else ""
    event_comment = TEMPLATE_EVENT_COMMENT if cmd_name in NO_EVENT_MARKER_EXCEPTIONS_LIST else ""
    name_no_suffix = strip_suffix(api, cmd_name[2:])
    name_lower_no_suffix = name_no_suffix[0:1].lower() + name_no_suffix[1:]
    entry_point_name = "angle::EntryPoint::GL" + strip_api_prefix(cmd_name)

    format_params = {
        "name":
            strip_api_prefix(cmd_name),
        "name_no_suffix":
            name_no_suffix,
        "name_lower_no_suffix":
            name_lower_no_suffix,
        "return_type":
            return_type,
        "params":
            ", ".join(params),
        "internal_params":
            ", ".join(internal_params),
        "attrib_map_init":
            "\n".join(attrib_map_init),
        "context_private_internal_params":
            ", ".join(
                ["context->getMutablePrivateState()", "context->getMutablePrivateStateCache()"] +
                internal_params),
        "internal_context_lock_params":
            ", ".join(internal_context_lock_params),
        "initialization":
            initialization,
        "packed_gl_enum_conversions":
            "".join(packed_gl_enum_conversions),
        "pass_params":
            ", ".join(pass_params),
        "comma_if_needed":
            ", " if len(params) > 0 else "",
        "comma_if_needed_context_lock":
            ", " if len(internal_context_lock_params) > 0 else "",
        "gl_capture_params":
            ", ".join(["context"] + internal_params),
        "egl_capture_params":
            ", ".join(["thread"] + internal_params),
        "validation_expression":
            get_validation_expression(api, cmd_name, entry_point_name, internal_params, is_gles1),
        "format_params":
            ", ".join(format_params),
        "context_getter":
            get_context_getter_function(cmd_name),
        "valid_context_check":
            get_valid_context_check(cmd_name),
        "constext_lost_error_generator":
            get_constext_lost_error_generator(cmd_name),
        "event_comment":
            event_comment,
        "labeled_object":
            get_egl_entry_point_labeled_object(ep_to_object, cmd_name, params, packed_enums),
        "context_lock":
            get_context_lock(api, cmd_name),
        "preamble":
            get_preamble(api, cmd_name, params),
        "epilog":
            get_epilog(api, cmd_name),
        "egl_lock":
            get_egl_lock(cmd_name),
    }

    template = get_def_template(api, cmd_name, return_type, has_errcode_ret)
    return template.format(**format_params)


def get_capture_param_type_name(param_type):
    pointer_count = param_type.count("*")
    is_const = "const" in param_type.split()

    param_type = param_type.replace("*", "")
    param_type = param_type.replace("&", "")
    param_type = param_type.replace("const", "")
    param_type = param_type.replace("struct", "")
    param_type = param_type.replace("egl::",
                                    "egl_" if pointer_count or param_type == 'egl::SyncID' else "")
    param_type = param_type.replace("gl::", "")
    param_type = param_type.strip()

    if is_const and param_type != 'AttributeMap':
        param_type += "Const"
    for x in range(pointer_count):
        param_type += "Pointer"

    return param_type


def format_capture_method(api, command, cmd_name, proto, params, all_param_types,
                          capture_pointer_funcs, cmd_packed_gl_enums, packed_param_types):

    context_param_typed = 'egl::Thread *thread' if api == apis.EGL else 'const State &glState'
    context_param_name = 'thread' if api == apis.EGL else 'glState'

    packed_gl_enums = get_packed_enums(api, cmd_packed_gl_enums, cmd_name, packed_param_types,
                                       params)

    params_with_type = get_internal_params(api, cmd_name,
                                           [context_param_typed, "bool isCallValid"] + params,
                                           cmd_packed_gl_enums, packed_param_types)
    params_just_name = ", ".join(
        [context_param_name, "isCallValid"] +
        [just_the_name_packed(param, packed_gl_enums) for param in params])

    parameter_captures = []
    for param in params:

        param_name = just_the_name_packed(param, packed_gl_enums)
        param_type = just_the_type_packed(param, packed_gl_enums).strip()

        if 'AttributeMap' in param_type:
            capture = 'paramBuffer.addParam(CaptureAttributeMap(%s));' % param_name
            parameter_captures += [capture]
            continue

        pointer_count = param_type.count("*")
        capture_param_type = get_capture_param_type_name(param_type)

        # With EGL capture, we don't currently support capturing specific pointer params.
        if pointer_count > 0 and api != apis.EGL:
            params = params_just_name
            capture_name = "Capture%s_%s" % (strip_api_prefix(cmd_name), param_name)
            capture = TEMPLATE_PARAMETER_CAPTURE_POINTER.format(
                name=param_name,
                type=capture_param_type,
                capture_name=capture_name,
                params=params,
                cast_type=param_type)

            capture_pointer_func = TEMPLATE_PARAMETER_CAPTURE_POINTER_FUNC.format(
                name=capture_name, params=params_with_type + ", angle::ParamCapture *paramCapture")
            capture_pointer_funcs += [capture_pointer_func]
        elif capture_param_type in ('GLenum', 'GLbitfield'):
            gl_enum_group = find_gl_enum_group_in_command(command, param_name)
            capture = TEMPLATE_PARAMETER_CAPTURE_GL_ENUM.format(
                name=param_name,
                type=capture_param_type,
                api_enum=api_enums[api],
                group=gl_enum_group)
        else:
            capture = TEMPLATE_PARAMETER_CAPTURE_VALUE.format(
                name=param_name, type=capture_param_type)

        # For specific methods we can't easily parse their types. Work around this by omitting
        # parameter captures, but keeping the capture method as a mostly empty stub.
        if cmd_name not in CAPTURE_BLOCKLIST:
            all_param_types.add(capture_param_type)
            parameter_captures += [capture]

    return_type = proto[:-len(cmd_name)].strip()
    capture_return_type = get_capture_param_type_name(return_type)
    if capture_return_type != 'void':
        if cmd_name in CAPTURE_BLOCKLIST:
            params_with_type += ", %s returnValue" % capture_return_type
        else:
            all_param_types.add(capture_return_type)

    format_args = {
        "api_upper": "EGL" if api == apis.EGL else "GL",
        "full_name": cmd_name,
        "short_name": strip_api_prefix(cmd_name),
        "params_with_type": params_with_type,
        "params_just_name": params_just_name,
        "parameter_captures": "\n    ".join(parameter_captures),
        "return_value_type_original": return_type,
        "return_value_type_custom": capture_return_type,
    }

    if return_type == "void" or cmd_name in CAPTURE_BLOCKLIST:
        return TEMPLATE_CAPTURE_METHOD_NO_RETURN_VALUE.format(**format_args)
    else:
        return TEMPLATE_CAPTURE_METHOD_WITH_RETURN_VALUE.format(**format_args)


def const_pointer_type(param, packed_gl_enums):
    type = just_the_type_packed(param, packed_gl_enums)
    if just_the_name(param) == "errcode_ret" or type == "ErrorSet *" or "(" in type:
        return type
    elif "**" in type and "const" not in type:
        return type.replace("**", "* const *")
    elif "*" in type and "const" not in type:
        return type.replace("*", "*const ") if "[]" in type else "const " + type
    else:
        return type


def get_internal_params(api, cmd_name, params, cmd_packed_gl_enums, packed_param_types):
    packed_gl_enums = get_packed_enums(api, cmd_packed_gl_enums, cmd_name, packed_param_types,
                                       params)
    return ", ".join([
        make_param(
            just_the_type_packed(param, packed_gl_enums),
            just_the_name_packed(param, packed_gl_enums)) for param in params
    ])


def get_validation_params(api, cmd_name, params, cmd_packed_gl_enums, packed_param_types):
    packed_gl_enums = get_packed_enums(api, cmd_packed_gl_enums, cmd_name, packed_param_types,
                                       params)
    last = -1 if params and just_the_name(params[-1]) == "errcode_ret" else None
    return ", ".join([
        make_param(
            const_pointer_type(param, packed_gl_enums),
            just_the_name_packed(param, packed_gl_enums)) for param in params[:last]
    ])


def get_context_private_call_params(api, cmd_name, params, cmd_packed_gl_enums,
                                    packed_param_types):
    packed_gl_enums = get_packed_enums(api, cmd_packed_gl_enums, cmd_name, packed_param_types,
                                       params)
    return ", ".join([
        make_param(
            just_the_type_packed(param, packed_gl_enums),
            just_the_name_packed(param, packed_gl_enums)) for param in params
    ])


def get_context_lock_params(api, cmd_name, params, cmd_packed_gl_enums, packed_param_types):
    packed_gl_enums = get_packed_enums(api, cmd_packed_gl_enums, cmd_name, packed_param_types,
                                       params)
    return ", ".join([
        make_param(
            just_the_type_packed(param, packed_gl_enums),
            just_the_name_packed(param, packed_gl_enums))
        for param in params
        if just_the_type_packed(param, packed_gl_enums) in EGL_CONTEXT_LOCK_PARAM_TYPES_FILTER or
        just_the_name_packed(param, packed_gl_enums) in EGL_CONTEXT_LOCK_PARAM_NAMES_FILTER
    ])


def format_context_decl(api, cmd_name, proto, params, template, cmd_packed_gl_enums,
                        packed_param_types):
    internal_params = get_internal_params(api, cmd_name, params, cmd_packed_gl_enums,
                                          packed_param_types)

    return_type = proto[:-len(cmd_name)].strip()
    name_lower_no_suffix = cmd_name[2:3].lower() + cmd_name[3:]
    name_lower_no_suffix = strip_suffix(api, name_lower_no_suffix)
    maybe_const = " const" if name_lower_no_suffix.startswith(
        "is") and name_lower_no_suffix[2].isupper() else ""

    return template.format(
        return_type=return_type,
        name_lower_no_suffix=name_lower_no_suffix,
        internal_params=internal_params,
        maybe_const=maybe_const)


def format_entry_point_export(cmd_name, proto, params, template):
    internal_params = [just_the_name(param) for param in params]
    return_type = proto[:-len(cmd_name)].strip()

    return template.format(
        name=strip_api_prefix(cmd_name),
        return_type=return_type,
        params=", ".join(params),
        internal_params=", ".join(internal_params))


def format_validation_proto(api, cmd_name, params, cmd_packed_gl_enums, packed_param_types):
    if api == apis.CL:
        return_type = "cl_int"
    else:
        return_type = "bool"
    if api in [apis.GL, apis.GLES]:
        with_extra_params = ["const PrivateState &state",
                             "ErrorSet *errors"] if is_context_private_state_command(
                                 api, cmd_name) else ["Context *context"]
        with_extra_params += ["angle::EntryPoint entryPoint"] + params
    elif api == apis.EGL:
        with_extra_params = ["ValidationContext *val"] + params
    else:
        with_extra_params = params
    internal_params = get_validation_params(api, cmd_name, with_extra_params, cmd_packed_gl_enums,
                                            packed_param_types)
    return TEMPLATE_VALIDATION_PROTO % (return_type, strip_api_prefix(cmd_name), internal_params)


def format_context_private_call_proto(api, cmd_name, proto, params, cmd_packed_gl_enums,
                                      packed_param_types):
    with_extra_params = ["PrivateState *privateState", "PrivateStateCache *privateStateCache"
                        ] + params
    packed_enums = get_packed_enums(api, cmd_packed_gl_enums, cmd_name, packed_param_types,
                                    with_extra_params)
    internal_params = get_context_private_call_params(api, cmd_name, with_extra_params,
                                                      cmd_packed_gl_enums, packed_param_types)
    stripped_name = strip_suffix(api, strip_api_prefix(cmd_name))
    return_type = proto[:-len(cmd_name)].strip()
    return TEMPLATE_CONTEXT_PRIVATE_CALL_PROTO % (return_type, stripped_name,
                                                  internal_params), stripped_name


def format_context_lock_proto(api, cmd_name, params, cmd_packed_gl_enums, packed_param_types):
    with_extra_params = ["Thread *thread"] + params
    internal_params = get_context_lock_params(api, cmd_name, with_extra_params,
                                              cmd_packed_gl_enums, packed_param_types)
    return TEMPLATE_CONTEXT_LOCK_PROTO % (strip_api_prefix(cmd_name), internal_params)


def format_capture_proto(api, cmd_name, proto, params, cmd_packed_gl_enums, packed_param_types):
    context_param_typed = 'egl::Thread *thread' if api == apis.EGL else 'const State &glState'
    internal_params = get_internal_params(api, cmd_name,
                                          [context_param_typed, "bool isCallValid"] + params,
                                          cmd_packed_gl_enums, packed_param_types)
    return_type = proto[:-len(cmd_name)].strip()
    if return_type != "void":
        internal_params += ", %s returnValue" % return_type
    return TEMPLATE_CAPTURE_PROTO % (strip_api_prefix(cmd_name), internal_params)


def path_to(folder, file):
    return os.path.join(script_relative(".."), "src", folder, file)


class ANGLEEntryPoints(registry_xml.EntryPoints):

    def __init__(self,
                 api,
                 xml,
                 commands,
                 all_param_types,
                 cmd_packed_enums,
                 export_template=TEMPLATE_GL_ENTRY_POINT_EXPORT,
                 packed_param_types=[],
                 ep_to_object={},
                 is_gles1=False):
        super().__init__(api, xml, commands)

        self.decls = []
        self.defs = []
        self.export_defs = []
        self.validation_protos = []
        self.context_private_call_protos = []
        self.context_private_call_functions = []
        self.context_lock_protos = []
        self.capture_protos = []
        self.capture_methods = []
        self.capture_pointer_funcs = []

        for (cmd_name, command_node, param_text, proto_text) in self.get_infos():
            self.decls.append(format_entry_point_decl(self.api, cmd_name, proto_text, param_text))
            self.defs.append(
                format_entry_point_def(self.api, command_node, cmd_name, proto_text, param_text,
                                       cmd_packed_enums, packed_param_types, ep_to_object,
                                       is_gles1))

            self.export_defs.append(
                format_entry_point_export(cmd_name, proto_text, param_text, export_template))

            self.validation_protos.append(
                format_validation_proto(self.api, cmd_name, param_text, cmd_packed_enums,
                                        packed_param_types))

            if is_context_private_state_command(self.api, cmd_name):
                proto, function = format_context_private_call_proto(self.api, cmd_name, proto_text,
                                                                    param_text, cmd_packed_enums,
                                                                    packed_param_types)
                self.context_private_call_protos.append(proto)
                self.context_private_call_functions.append(function)

            if api == apis.EGL:
                self.context_lock_protos.append(
                    format_context_lock_proto(api, cmd_name, param_text, cmd_packed_enums,
                                              packed_param_types))

            self.capture_protos.append(
                format_capture_proto(self.api, cmd_name, proto_text, param_text, cmd_packed_enums,
                                     packed_param_types))
            self.capture_methods.append(
                format_capture_method(self.api, command_node, cmd_name, proto_text, param_text,
                                      all_param_types, self.capture_pointer_funcs,
                                      cmd_packed_enums, packed_param_types))

        # Ensure we store GLint64 in the param types for use with the replay interpreter.
        all_param_types.add('GLint64')


class GLEntryPoints(ANGLEEntryPoints):

    all_param_types = set()

    def __init__(self, api, xml, commands, is_gles1=False):
        super().__init__(
            api,
            xml,
            commands,
            GLEntryPoints.all_param_types,
            GLEntryPoints.get_packed_enums(),
            is_gles1=is_gles1)

    _packed_enums = None

    @classmethod
    def get_packed_enums(cls):
        if not cls._packed_enums:
            with open(script_relative('entry_point_packed_gl_enums.json')) as f:
                cls._packed_enums = json.loads(f.read())
        return cls._packed_enums


class EGLEntryPoints(ANGLEEntryPoints):

    all_param_types = set()

    def __init__(self, xml, commands):
        super().__init__(
            apis.EGL,
            xml,
            commands,
            EGLEntryPoints.all_param_types,
            EGLEntryPoints.get_packed_enums(),
            export_template=TEMPLATE_EGL_ENTRY_POINT_EXPORT,
            packed_param_types=EGL_PACKED_TYPES,
            ep_to_object=EGLEntryPoints._get_ep_to_object())

    _ep_to_object = None

    @classmethod
    def _get_ep_to_object(cls):

        if cls._ep_to_object:
            return cls._ep_to_object

        with open(EGL_GET_LABELED_OBJECT_DATA_PATH) as f:
            try:
                spec_json = json.loads(f.read())
            except ValueError:
                raise Exception("Could not decode JSON from %s" % EGL_GET_LABELED_OBJECT_DATA_PATH)

        # Construct a mapping from EP to type. Fill in the gaps with Display/None.
        cls._ep_to_object = {}

        for category, eps in spec_json.items():
            if category == 'description':
                continue
            for ep in eps:
                cls._ep_to_object[ep] = category

        return cls._ep_to_object

    _packed_enums = None

    @classmethod
    def get_packed_enums(cls):
        if not cls._packed_enums:
            with open(script_relative('entry_point_packed_egl_enums.json')) as f:
                cls._packed_enums = json.loads(f.read())
        return cls._packed_enums


class CLEntryPoints(ANGLEEntryPoints):

    all_param_types = set()

    def __init__(self, xml, commands):
        super().__init__(
            apis.CL,
            xml,
            commands,
            CLEntryPoints.all_param_types,
            CLEntryPoints.get_packed_enums(),
            export_template=TEMPLATE_CL_ENTRY_POINT_EXPORT,
            packed_param_types=CL_PACKED_TYPES)

    @classmethod
    def get_packed_enums(cls):
        return {}


def get_decls(api,
              formatter,
              all_commands,
              gles_commands,
              already_included,
              cmd_packed_gl_enums,
              packed_param_types=[]):
    decls = []
    for command in all_commands:
        proto = command.find('proto')
        cmd_name = proto.find('name').text

        if cmd_name not in gles_commands:
            continue

        name_no_suffix = strip_suffix(api, cmd_name)
        if name_no_suffix in already_included:
            continue

        # Don't generate Context::entryPoint declarations for entry points that
        # directly access the context-private state.
        if is_context_private_state_command(api, cmd_name):
            continue

        param_text = ["".join(param.itertext()) for param in command.findall('param')]
        proto_text = "".join(proto.itertext())
        decls.append(
            format_context_decl(api, cmd_name, proto_text, param_text, formatter,
                                cmd_packed_gl_enums, packed_param_types))

    return decls


def get_glext_decls(all_commands, gles_commands, version):
    glext_ptrs = []
    glext_protos = []
    is_gles1 = False

    if (version == ""):
        is_gles1 = True

    for command in all_commands:
        proto = command.find('proto')
        cmd_name = proto.find('name').text

        if cmd_name not in gles_commands:
            continue

        param_text = ["".join(param.itertext()) for param in command.findall('param')]
        proto_text = "".join(proto.itertext())

        return_type = proto_text[:-len(cmd_name)]
        params = ", ".join(param_text)

        format_params = {
            "apicall": "GL_API" if is_gles1 else "GL_APICALL",
            "name": cmd_name,
            "name_upper": cmd_name.upper(),
            "return_type": return_type,
            "params": params,
        }

        glext_ptrs.append(TEMPLATE_GLEXT_FUNCTION_POINTER.format(**format_params))
        glext_protos.append(TEMPLATE_GLEXT_FUNCTION_PROTOTYPE.format(**format_params))

    return glext_ptrs, glext_protos


def write_file(annotation, comment, template, entry_points, suffix, includes, lib, file):

    content = template.format(
        script_name=os.path.basename(sys.argv[0]),
        data_source_name=file,
        annotation_lower=annotation.lower(),
        annotation_upper=annotation.upper(),
        comment=comment,
        lib=lib.upper(),
        includes=includes,
        entry_points=entry_points)

    path = path_to(lib, "entry_points_{}_autogen.{}".format(annotation.lower(), suffix))

    with open(path, "w") as out:
        out.write(content)
        out.close()


def write_export_files(entry_points, includes, source, lib_name, lib_description, lib_dir=None):
    content = TEMPLATE_LIB_ENTRY_POINT_SOURCE.format(
        script_name=os.path.basename(sys.argv[0]),
        data_source_name=source,
        lib_name=lib_name,
        lib_description=lib_description,
        includes=includes,
        entry_points=entry_points,
    )

    path = path_to(lib_name if not lib_dir else lib_dir, "{}_autogen.cpp".format(lib_name))

    with open(path, "w") as out:
        out.write(content)
        out.close()


def write_context_api_decls(decls, api):
    for (major, minor), version_decls in sorted(decls['core'].items()):
        if minor == "X":
            annotation = '{}_{}'.format(api, major)
            version = str(major)
        else:
            annotation = '{}_{}_{}'.format(api, major, minor)
            version = '{}_{}'.format(major, minor)
        content = CONTEXT_HEADER.format(
            annotation_lower=annotation.lower(),
            annotation_upper=annotation.upper(),
            script_name=os.path.basename(sys.argv[0]),
            data_source_name="gl.xml",
            version=version,
            interface="\n".join(version_decls))

        path = path_to("libANGLE", "Context_%s_autogen.h" % annotation.lower())

        with open(path, "w") as out:
            out.write(content)
            out.close()

    if 'exts' in decls.keys():
        interface_lines = []
        for annotation in decls['exts'].keys():
            interface_lines.append("\\\n    /* " + annotation + " */ \\\n\\")

            for extname in sorted(decls['exts'][annotation].keys()):
                interface_lines.append("    /* " + extname + " */ \\")
                interface_lines.extend(decls['exts'][annotation][extname])

        content = CONTEXT_HEADER.format(
            annotation_lower='gles_ext',
            annotation_upper='GLES_EXT',
            script_name=os.path.basename(sys.argv[0]),
            data_source_name="gl.xml",
            version='EXT',
            interface="\n".join(interface_lines))

        path = path_to("libANGLE", "Context_gles_ext_autogen.h")

        with open(path, "w") as out:
            out.write(content)
            out.close()


def write_validation_header(annotation, comment, protos, source, template):
    content = template.format(
        script_name=os.path.basename(sys.argv[0]),
        data_source_name=source,
        annotation=annotation,
        comment=comment,
        prototypes="\n".join(protos))

    path = path_to("libANGLE", "validation%s_autogen.h" % annotation)

    with open(path, "w") as out:
        out.write(content)
        out.close()


def write_context_private_call_header(protos, source, template):
    content = TEMPLATE_CONTEXT_PRIVATE_CALL_HEADER.format(
        script_name=os.path.basename(sys.argv[0]),
        data_source_name=source,
        prototypes="\n".join(protos))

    path = path_to("libANGLE", "context_private_call_autogen.h")

    with open(path, "w") as out:
        out.write(content)
        out.close()


def write_context_lock_header(annotation, comment, protos, source, template):
    content = template.format(
        script_name=os.path.basename(sys.argv[0]),
        data_source_name=source,
        annotation_lower=annotation.lower(),
        annotation_upper=annotation.upper(),
        comment=comment,
        prototypes="\n".join(protos))

    path = path_to("libGLESv2", "%s_context_lock_autogen.h" % annotation.lower())

    with open(path, "w") as out:
        out.write(content)
        out.close()


def write_gl_validation_header(annotation, comment, protos, source):
    return write_validation_header(annotation, comment, protos, source,
                                   TEMPLATE_GL_VALIDATION_HEADER)


def write_capture_header(api, annotation, comment, protos, capture_pointer_funcs):
    ns = 'egl' if api == apis.EGL else 'gl'
    combined_protos = ["\n// Method Captures\n"] + protos
    if capture_pointer_funcs:
        combined_protos += ["\n// Parameter Captures\n"] + capture_pointer_funcs
    content = TEMPLATE_CAPTURE_HEADER.format(
        script_name=os.path.basename(sys.argv[0]),
        data_source_name="%s.xml and %s_angle_ext.xml" % (ns, ns),
        annotation_lower=annotation.lower(),
        annotation_upper=annotation.upper(),
        comment=comment,
        namespace=ns,
        prototypes="\n".join(combined_protos))

    path = path_to(os.path.join("libANGLE", "capture"), "capture_%s_autogen.h" % annotation)

    with open(path, "w") as out:
        out.write(content)
        out.close()


def write_capture_source(api, annotation_with_dash, annotation_no_dash, comment, capture_methods):
    ns = 'egl' if api == apis.EGL else 'gl'
    content = TEMPLATE_CAPTURE_SOURCE.format(
        script_name=os.path.basename(sys.argv[0]),
        data_source_name="%s.xml and %s_angle_ext.xml" % (ns, ns),
        annotation_with_dash=annotation_with_dash,
        annotation_no_dash=annotation_no_dash,
        comment=comment,
        namespace=ns,
        capture_methods="\n".join(capture_methods))

    path = path_to(
        os.path.join("libANGLE", "capture"), "capture_%s_autogen.cpp" % annotation_with_dash)

    with open(path, "w") as out:
        out.write(content)
        out.close()


def is_packed_enum_param_type(param_type):
    return not param_type.startswith("GL") and not param_type.startswith(
        "EGL") and "void" not in param_type


def add_namespace(param_type):
    param_type = param_type.strip()

    if param_type == 'AHardwareBufferConstPointer' or param_type == 'charConstPointer':
        return param_type

    # ANGLE namespaced EGL types
    egl_namespace = [
        'CompositorTiming',
        'ObjectType',
        'Timestamp',
    ] + list(EGL_PACKED_TYPES.values())

    if param_type[0:2] == "GL" or param_type[0:3] == "EGL" or "void" in param_type:
        return param_type

    if param_type.startswith('gl_'):
        return param_type.replace('gl_', 'gl::')
    elif param_type.startswith('egl_'):
        return param_type.replace('egl_', 'egl::')
    elif param_type.startswith('wl_'):
        return param_type
    elif param_type in egl_namespace:
        return "egl::" + param_type
    else:
        return "gl::" + param_type


def get_gl_pointer_type(param_type):

    if "ConstPointerPointer" in param_type:
        return "const " + param_type.replace("ConstPointerPointer", "") + " * const *"

    if "ConstPointer" in param_type:
        return "const " + param_type.replace("ConstPointer", "") + " *"

    if "PointerPointer" in param_type:
        return param_type.replace("PointerPointer", "") + " **"

    if "Pointer" in param_type:
        return param_type.replace("Pointer", "") + " *"

    return param_type


def get_param_type_type(param_type):
    param_type = add_namespace(param_type)
    return get_gl_pointer_type(param_type)


def is_id_type(t):
    return t.endswith('ID') and not t.endswith('ANDROID')


def is_id_pointer_type(t):
    return t.endswith("IDConstPointer") or t.endswith("IDPointer") and not 'ANDROID' in t


def get_gl_param_type_type(param_type):
    if is_packed_enum_param_type(param_type):
        base_type = param_type.replace("Pointer", "").replace("Const", "")
        if is_id_type(base_type):
            replace_type = "GLuint"
        else:
            replace_type = "GLenum"
        param_type = param_type.replace(base_type, replace_type)
    return get_gl_pointer_type(param_type)


def get_param_type_union_name(param_type):
    return param_type + "Val"


def format_param_type_union_type(param_type):
    return "%s %s;" % (get_param_type_type(param_type), get_param_type_union_name(param_type))


def format_get_param_val_specialization(param_type):
    return TEMPLATE_GET_PARAM_VAL_SPECIALIZATION.format(
        enum=param_type,
        type=get_param_type_type(param_type),
        union_name=get_param_type_union_name(param_type))


def format_access_param_value_case(param_type):
    return TEMPLATE_ACCESS_PARAM_VALUE_CASE.format(enum=param_type)


def format_set_param_val_specialization(param_type):
    return TEMPLATE_SET_PARAM_VAL_SPECIALIZATION.format(
        enum=param_type,
        type=get_param_type_type(param_type),
        union_name=get_param_type_union_name(param_type))


def format_init_param_value_case(param_type):
    return TEMPLATE_INIT_PARAM_VALUE_CASE.format(enum=param_type)


def format_write_param_type_to_stream_case(param_type):
    return TEMPLATE_WRITE_PARAM_TYPE_TO_STREAM_CASE.format(
        enum_in=param_type, enum_out=param_type, union_name=get_param_type_union_name(param_type))


def get_resource_id_types(all_param_types):
    return [t[:-2] for t in filter(lambda t: is_id_type(t), all_param_types)]


def format_resource_id_types(all_param_types):
    resource_id_types = get_resource_id_types(all_param_types)
    resource_id_types += ["EnumCount", "InvalidEnum = EnumCount"]
    resource_id_types = ",\n    ".join(resource_id_types)
    return resource_id_types


def format_resource_id_convert_structs(all_param_types):
    templ = """\
template <>
struct GetResourceIDTypeFromType<%s>
{
    static constexpr ResourceIDType IDType = ResourceIDType::%s;
};
"""
    resource_id_types = get_resource_id_types(all_param_types)
    convert_struct_strings = [templ % (add_namespace('%sID' % id), id) for id in resource_id_types]
    return "\n".join(convert_struct_strings)


def write_capture_helper_header(all_param_types):

    param_types = "\n    ".join(["T%s," % t for t in all_param_types])
    param_union_values = "\n    ".join([format_param_type_union_type(t) for t in all_param_types])
    get_param_val_specializations = "\n\n".join(
        [format_get_param_val_specialization(t) for t in all_param_types])
    access_param_value_cases = "\n".join(
        [format_access_param_value_case(t) for t in all_param_types])
    set_param_val_specializations = "\n\n".join(
        [format_set_param_val_specialization(t) for t in all_param_types])
    init_param_value_cases = "\n".join([format_init_param_value_case(t) for t in all_param_types])
    resource_id_types = format_resource_id_types(all_param_types)
    convert_structs = format_resource_id_convert_structs(all_param_types)

    content = TEMPLATE_FRAME_CAPTURE_UTILS_HEADER.format(
        script_name=os.path.basename(sys.argv[0]),
        data_source_name="gl.xml and gl_angle_ext.xml",
        param_types=param_types,
        param_type_count=len(all_param_types),
        param_union_values=param_union_values,
        get_param_val_specializations=get_param_val_specializations,
        access_param_value_cases=access_param_value_cases,
        set_param_val_specializations=set_param_val_specializations,
        init_param_value_cases=init_param_value_cases,
        resource_id_types=resource_id_types,
        type_to_resource_id_type_structs=convert_structs)

    path = path_to("common", "frame_capture_utils_autogen.h")

    with open(path, "w") as out:
        out.write(content)
        out.close()


def format_param_type_to_string_case(param_type):
    return TEMPLATE_PARAM_TYPE_TO_STRING_CASE.format(
        enum=param_type, type=get_gl_param_type_type(param_type))


def get_resource_id_type_from_param_type(param_type):
    if param_type.endswith("ConstPointer"):
        return param_type.replace("ConstPointer", "")[:-2]
    if param_type.endswith("Pointer"):
        return param_type.replace("Pointer", "")[:-2]
    return param_type[:-2]


def format_param_type_to_resource_id_type_case(param_type):
    return TEMPLATE_PARAM_TYPE_TO_RESOURCE_ID_TYPE_CASE.format(
        enum=param_type, resource_id_type=get_resource_id_type_from_param_type(param_type))


def format_param_type_resource_id_cases(all_param_types):
    id_types = filter(lambda t: is_id_type(t) or is_id_pointer_type(t), all_param_types)
    return "\n".join([format_param_type_to_resource_id_type_case(t) for t in id_types])


def format_resource_id_type_name_case(resource_id_type):
    return TEMPLATE_RESOURCE_ID_TYPE_NAME_CASE.format(resource_id_type=resource_id_type)


def write_capture_helper_source(all_param_types):

    write_param_type_to_stream_cases = "\n".join(
        [format_write_param_type_to_stream_case(t) for t in all_param_types])
    param_type_to_string_cases = "\n".join(
        [format_param_type_to_string_case(t) for t in all_param_types])

    param_type_resource_id_cases = format_param_type_resource_id_cases(all_param_types)

    resource_id_types = get_resource_id_types(all_param_types)
    resource_id_type_name_cases = "\n".join(
        [format_resource_id_type_name_case(t) for t in resource_id_types])

    content = TEMPLATE_FRAME_CAPTURE_UTILS_SOURCE.format(
        script_name=os.path.basename(sys.argv[0]),
        data_source_name="gl.xml and gl_angle_ext.xml",
        write_param_type_to_stream_cases=write_param_type_to_stream_cases,
        param_type_to_string_cases=param_type_to_string_cases,
        param_type_resource_id_cases=param_type_resource_id_cases,
        resource_id_type_name_cases=resource_id_type_name_cases)

    path = path_to("common", "frame_capture_utils_autogen.cpp")

    with open(path, "w") as out:
        out.write(content)
        out.close()


def get_command_params_text(command_node, cmd_name):
    param_text_list = list()
    for param_node in command_node.findall('param'):
        param_text_list.append("".join(param_node.itertext()))
    return param_text_list


def is_get_pointer_command(command_name):
    return command_name.endswith('Pointerv') and command_name.startswith('glGet')


def remove_id_suffix(t):
    return t[:-2] if is_id_type(t) else t


def format_replay_params(api, command_name, param_text_list, packed_enums, resource_id_types):
    param_access_strs = list()
    for i, param_text in enumerate(param_text_list):
        param_type = just_the_type(param_text)
        if param_type in EGL_PACKED_TYPES:
            param_type = 'void *'
        param_name = just_the_name(param_text)
        capture_type = get_capture_param_type_name(param_type)
        union_name = get_param_type_union_name(capture_type)
        param_access = 'captures[%d].value.%s' % (i, union_name)
        # Workaround for https://github.com/KhronosGroup/OpenGL-Registry/issues/545
        if command_name == 'glCreateShaderProgramvEXT' and i == 2:
            param_access = 'const_cast<const char **>(%s)' % param_access
        else:
            cmd_no_suffix = strip_suffix(api, command_name)
            if cmd_no_suffix in packed_enums and param_name in packed_enums[cmd_no_suffix]:
                packed_type = remove_id_suffix(packed_enums[cmd_no_suffix][param_name])
                if packed_type == 'Sync':
                    param_access = 'gSyncMap2[captures[%d].value.GLuintVal]' % i
                elif packed_type in resource_id_types:
                    param_access = 'g%sMap[%s]' % (packed_type, param_access)
                elif packed_type == 'UniformLocation':
                    param_access = 'gUniformLocations[gCurrentProgram][%s]' % param_access
                elif packed_type == 'egl::Image':
                    param_access = 'gEGLImageMap2[captures[%d].value.GLuintVal]' % i
                elif packed_type == 'egl::Sync':
                    param_access = 'gEGLSyncMap[captures[%d].value.egl_SyncIDVal]' % i
        param_access_strs.append(param_access)
    return ', '.join(param_access_strs)


def format_capture_replay_call_case(api, command_to_param_types_mapping, gl_packed_enums,
                                    resource_id_types):
    call_list = list()
    for command_name, cmd_param_texts in sorted(command_to_param_types_mapping.items()):
        entry_point_name = strip_api_prefix(command_name)

        call_list.append(
            TEMPLATE_REPLAY_CALL_CASE.format(
                enum=('EGL' if api == 'EGL' else 'GL') + entry_point_name,
                params=format_replay_params(api, command_name, cmd_param_texts, gl_packed_enums,
                                            resource_id_types),
                call=command_name,
            ))

    return ''.join(call_list)


def write_capture_replay_source(gl_command_nodes, gl_command_names, gl_packed_enums,
                                egl_command_nodes, egl_command_names, egl_packed_enums,
                                resource_id_types):

    call_replay_cases = ''

    for api, nodes, names, packed_enums in [
        (apis.GLES, gl_command_nodes, gl_command_names, gl_packed_enums),
        (apis.EGL, egl_command_nodes, egl_command_names, egl_packed_enums)
    ]:
        command_to_param_types_mapping = dict()
        all_commands_names = set(names)
        for command_node in nodes:
            command_name = command_node.find('proto').find('name').text
            if command_name not in all_commands_names:
                continue
            command_to_param_types_mapping[command_name] = get_command_params_text(
                command_node, command_name)

        call_replay_cases += format_capture_replay_call_case(api, command_to_param_types_mapping,
                                                             packed_enums, resource_id_types)

    source_content = TEMPLATE_CAPTURE_REPLAY_SOURCE.format(
        script_name=os.path.basename(sys.argv[0]),
        data_source_name="gl.xml and gl_angle_ext.xml",
        call_replay_cases=call_replay_cases,
    )
    source_file_path = registry_xml.script_relative(
        "../util/capture/frame_capture_replay_autogen.cpp")
    with open(source_file_path, 'w') as f:
        f.write(source_content)


def write_windows_def_file(data_source_name, lib, libexport, folder, exports):

    content = TEMPLATE_WINDOWS_DEF_FILE.format(
        script_name=os.path.basename(sys.argv[0]),
        data_source_name=data_source_name,
        exports="\n".join(exports),
        lib=libexport)

    path = path_to(folder, "%s_autogen.def" % lib)

    with open(path, "w") as out:
        out.write(content)
        out.close()


def get_exports(commands, fmt=None):
    if fmt:
        return ["    %s" % fmt(cmd) for cmd in sorted(commands)]
    else:
        return ["    %s" % cmd for cmd in sorted(commands)]


# Get EGL exports
def get_egl_exports():

    egl = registry_xml.RegistryXML('egl.xml', 'egl_angle_ext.xml')
    exports = []

    capser = lambda fn: "EGL_" + fn[3:]

    for major, minor in registry_xml.EGL_VERSIONS:
        annotation = "{}_{}".format(major, minor)
        name_prefix = "EGL_VERSION_"

        feature_name = "{}{}".format(name_prefix, annotation)

        egl.AddCommands(feature_name, annotation)

        commands = egl.commands[annotation]

        if len(commands) == 0:
            continue

        exports.append("\n    ; EGL %d.%d" % (major, minor))
        exports += get_exports(commands, capser)

    egl.AddExtensionCommands(registry_xml.supported_egl_extensions, ['egl'])

    for extension_name, ext_cmd_names in sorted(egl.ext_data.items()):

        if len(ext_cmd_names) == 0:
            continue

        exports.append("\n    ; %s" % extension_name)
        exports += get_exports(ext_cmd_names, capser)

    return exports


# Construct a mapping from an EGL EP to object function
def get_egl_entry_point_labeled_object(ep_to_object, cmd_stripped, params, packed_enums):

    if not ep_to_object:
        return ""

    # Finds a packed parameter name in a list of params
    def find_param(params, type_name, packed_enums):
        for param in params:
            if just_the_type_packed(param, packed_enums).split(' ')[0] == type_name:
                return just_the_name_packed(param, packed_enums)
        return None

    display_param = find_param(params, "egl::Display", packed_enums)

    # For entry points not listed in the JSON file, they default to an EGLDisplay or nothing.
    if cmd_stripped not in ep_to_object:
        if display_param:
            return "GetDisplayIfValid(%s)" % display_param
        return "nullptr"

    # We first handle a few special cases for certain type categories.
    category = ep_to_object[cmd_stripped]
    if category == "Thread":
        return "GetThreadIfValid(thread)"
    found_param = find_param(params, category, packed_enums)
    if category == "Context" and not found_param:
        return "GetContextIfValid(thread->getDisplay(), thread->getContext())"
    assert found_param, "Did not find %s for %s: %s" % (category, cmd_stripped, str(params))
    if category == "Device":
        return "GetDeviceIfValid(%s)" % found_param
    if category == "LabeledObject":
        object_type_param = find_param(params, "ObjectType", packed_enums)
        return "GetLabeledObjectIfValid(thread, %s, %s, %s)" % (display_param, object_type_param,
                                                                found_param)

    # We then handle the general case which handles the rest of the type categories.
    return "Get%sIfValid(%s, %s)" % (category, display_param, found_param)


def disable_share_group_lock(api, cmd_name):
    if cmd_name == 'glBindBuffer':
        # This function looks up the ID in the buffer manager,
        # access to which is thread-safe for buffers.
        return True

    if api == apis.GLES and cmd_name.startswith('glUniform'):
        # Thread safety of glUniform1/2/3/4 and glUniformMatrix* calls is defined by the backend,
        # frontend only does validation.
        keep_locked = [
            # Might set samplers:
            'glUniform1i',
            'glUniform1iv',
            # More complex state change with notifications:
            'glUniformBlockBinding',
        ]
        return cmd_name not in keep_locked

    return False


def get_context_lock(api, cmd_name):
    # EGLImage related commands need to access EGLImage and Display which should
    # be protected with global lock
    # Also handles ContextMutex merging when "angle_enable_context_mutex" is true.
    if api == apis.GLES and cmd_name.startswith("glEGLImage"):
        return "SCOPED_EGL_IMAGE_SHARE_CONTEXT_LOCK(context, imagePacked);"

    # Certain commands do not need to hold the share group lock.  Both
    # validation and their implementation in the context are limited to
    # context-local state.
    if disable_share_group_lock(api, cmd_name):
        return ""

    return "SCOPED_SHARE_CONTEXT_LOCK(context);"


def get_egl_lock(cmd_name):
    if is_egl_sync_entry_point(cmd_name):
        return "ANGLE_SCOPED_GLOBAL_EGL_SYNC_LOCK();"
    if is_egl_entry_point_accessing_both_sync_and_non_sync_API_resources(cmd_name):
        return "ANGLE_SCOPED_GLOBAL_EGL_AND_EGL_SYNC_LOCK();"
    else:
        return "ANGLE_SCOPED_GLOBAL_LOCK();"


def get_prepare_swap_buffers_call(api, cmd_name, params):
    if cmd_name not in [
            "eglSwapBuffers",
            "eglSwapBuffersWithDamageKHR",
            "eglSwapBuffersWithFrameTokenANGLE",
            "eglQuerySurface",
            "eglQuerySurface64KHR",
    ]:
        return ""

    passed_params = [None, None]

    for param in params:
        param_type = just_the_type(param)
        if param_type == "EGLDisplay":
            passed_params[0] = param
        if param_type == "EGLSurface":
            passed_params[1] = param

    prepareCall = "ANGLE_EGLBOOLEAN_TRY(EGL_PrepareSwapBuffersANGLE(%s));" % (", ".join(
        [just_the_name(param) for param in passed_params]))

    # For eglQuerySurface, the prepare call is only needed for EGL_BUFFER_AGE
    if cmd_name in ["eglQuerySurface", "eglQuerySurface64KHR"]:
        prepareCall = "if (attribute == EGL_BUFFER_AGE_EXT) {" + prepareCall + "}"

    return prepareCall


def get_preamble(api, cmd_name, params):
    preamble = ""
    preamble += get_prepare_swap_buffers_call(api, cmd_name, params)
    # TODO: others?
    return preamble


def get_unlocked_tail_call(api, cmd_name):
    # Only the following can generate tail calls:
    #
    # - eglDestroySurface, eglMakeCurrent and eglReleaseThread -> May destroy
    #   VkSurfaceKHR in tail call
    # - eglCreateWindowSurface and eglCreatePlatformWindowSurface[EXT] -> May
    #   destroy VkSurfaceKHR in tail call if surface initialization fails
    #
    # - eglPrepareSwapBuffersANGLE -> Calls vkAcquireNextImageKHR in tail call
    #
    # - eglSwapBuffers, eglSwapBuffersWithDamageKHR and
    #   eglSwapBuffersWithFrameTokenANGLE -> May throttle the CPU in tail call or
    #   calls native EGL function
    #
    # - eglClientWaitSyncKHR, eglClientWaitSync, glClientWaitSync,
    #   glFinishFenceNV -> May wait on fence in tail call or call native EGL function
    #
    # - glTexImage2D, glTexImage3D, glTexSubImage2D, glTexSubImage3D,
    #   glCompressedTexImage2D, glCompressedTexImage3D,
    #   glCompressedTexSubImage2D, glCompressedTexSubImage3D -> May perform the
    #   data upload on the host in tail call
    #
    # - glCompileShader, glShaderBinary, glLinkProgram -> May perform the compilation /
    #   link in tail call
    #
    # - eglCreateSync, eglCreateImage, eglDestroySync, eglDestroyImage,
    #   eglGetCompositorTimingANDROID, eglGetFrameTimestampsANDROID -> Calls
    #   native EGL function in tail call
    #
    # - glFlush, glFinish -> May perform the CPU throttling from the implicit swap buffers call when
    #   the current Window Surface is in the single buffer mode.
    #
    if (cmd_name in [
            'eglDestroySurface', 'eglMakeCurrent', 'eglReleaseThread', 'eglCreateWindowSurface',
            'eglCreatePlatformWindowSurface', 'eglCreatePlatformWindowSurfaceEXT',
            'eglPrepareSwapBuffersANGLE', 'eglSwapBuffersWithFrameTokenANGLE', 'glFinishFenceNV',
            'glCompileShader', 'glLinkProgram', 'glShaderBinary', 'glFlush', 'glFinish'
    ] or cmd_name.startswith('glTexImage2D') or cmd_name.startswith('glTexImage3D') or
            cmd_name.startswith('glTexSubImage2D') or cmd_name.startswith('glTexSubImage3D') or
            cmd_name.startswith('glCompressedTexImage2D') or
            cmd_name.startswith('glCompressedTexImage3D') or
            cmd_name.startswith('glCompressedTexSubImage2D') or
            cmd_name.startswith('glCompressedTexSubImage3D')):
        return 'egl::Display::GetCurrentThreadUnlockedTailCall()->run(nullptr);'

    if cmd_name in [
            'eglClientWaitSyncKHR',
            'eglClientWaitSync',
            'eglCreateImageKHR',
            'eglCreateImage',
            'eglCreateSyncKHR',
            'eglCreateSync',
            'eglDestroySyncKHR',
            'eglDestroySync',
            'eglGetCompositorTimingANDROID',
            'eglGetFrameTimestampsANDROID',
            'eglSwapBuffers',
            'eglSwapBuffersWithDamageKHR',
            'eglWaitSyncKHR',
            'eglWaitSync',
            'glClientWaitSync',
    ]:
        return 'egl::Display::GetCurrentThreadUnlockedTailCall()->run(&returnValue);'

    # Otherwise assert that no tail calls where generated
    return 'ASSERT(!egl::Display::GetCurrentThreadUnlockedTailCall()->any());'


def get_epilog(api, cmd_name):
    epilog = get_unlocked_tail_call(api, cmd_name)
    return epilog


def write_stubs_header(api, annotation, title, data_source, out_file, all_commands, commands,
                       cmd_packed_egl_enums, packed_param_types):

    stubs = []

    for command in all_commands:
        proto = command.find('proto')
        cmd_name = proto.find('name').text

        if cmd_name not in commands:
            continue

        proto_text = "".join(proto.itertext())

        params = [] if api == apis.CL else ["Thread *thread"]
        params += ["".join(param.itertext()) for param in command.findall('param')]
        if params and just_the_name(params[-1]) == "errcode_ret":
            # Using TLS object for CL error handling, no longer a need for errcode_ret
            del params[-1]
        return_type = proto_text[:-len(cmd_name)].strip()

        internal_params = get_internal_params(api, cmd_name, params, cmd_packed_egl_enums,
                                              packed_param_types)
        stubs.append("%s %s(%s);" % (return_type, strip_api_prefix(cmd_name), internal_params))

    args = {
        "annotation_lower": annotation.lower(),
        "annotation_upper": annotation.upper(),
        "data_source_name": data_source,
        "script_name": os.path.basename(sys.argv[0]),
        "stubs": "\n".join(stubs),
        "title": title,
    }

    output = get_stubs_header_template(api).format(**args)

    with open(out_file, "w") as f:
        f.write(output)


def main():

    # auto_script parameters.
    if len(sys.argv) > 1:
        inputs = [
            'entry_point_packed_egl_enums.json', 'entry_point_packed_gl_enums.json',
            EGL_GET_LABELED_OBJECT_DATA_PATH
        ] + registry_xml.xml_inputs
        outputs = [
            CL_STUBS_HEADER_PATH,
            EGL_STUBS_HEADER_PATH,
            EGL_EXT_STUBS_HEADER_PATH,
            '../src/libOpenCL/libOpenCL_autogen.cpp',
            '../src/common/entry_points_enum_autogen.cpp',
            '../src/common/entry_points_enum_autogen.h',
            '../src/common/frame_capture_utils_autogen.cpp',
            '../src/common/frame_capture_utils_autogen.h',
            '../src/libANGLE/Context_gles_1_0_autogen.h',
            '../src/libANGLE/Context_gles_2_0_autogen.h',
            '../src/libANGLE/Context_gles_3_0_autogen.h',
            '../src/libANGLE/Context_gles_3_1_autogen.h',
            '../src/libANGLE/Context_gles_3_2_autogen.h',
            '../src/libANGLE/Context_gles_ext_autogen.h',
            '../src/libANGLE/context_private_call_autogen.h',
            '../src/libANGLE/capture/capture_egl_autogen.cpp',
            '../src/libANGLE/capture/capture_egl_autogen.h',
            '../src/libANGLE/capture/capture_gles_1_0_autogen.cpp',
            '../src/libANGLE/capture/capture_gles_1_0_autogen.h',
            '../src/libANGLE/capture/capture_gles_2_0_autogen.cpp',
            '../src/libANGLE/capture/capture_gles_2_0_autogen.h',
            '../src/libANGLE/capture/capture_gles_3_0_autogen.cpp',
            '../src/libANGLE/capture/capture_gles_3_0_autogen.h',
            '../src/libANGLE/capture/capture_gles_3_1_autogen.cpp',
            '../src/libANGLE/capture/capture_gles_3_1_autogen.h',
            '../src/libANGLE/capture/capture_gles_3_2_autogen.cpp',
            '../src/libANGLE/capture/capture_gles_3_2_autogen.h',
            '../src/libANGLE/capture/capture_gles_ext_autogen.cpp',
            '../src/libANGLE/capture/capture_gles_ext_autogen.h',
            '../src/libANGLE/validationCL_autogen.h',
            '../src/libANGLE/validationEGL_autogen.h',
            '../src/libANGLE/validationES1_autogen.h',
            '../src/libANGLE/validationES2_autogen.h',
            '../src/libANGLE/validationES31_autogen.h',
            '../src/libANGLE/validationES32_autogen.h',
            '../src/libANGLE/validationES3_autogen.h',
            '../src/libANGLE/validationESEXT_autogen.h',
            '../src/libEGL/libEGL_autogen.cpp',
            '../src/libEGL/libEGL_autogen.def',
            '../src/libGLESv2/entry_points_cl_autogen.cpp',
            '../src/libGLESv2/entry_points_cl_autogen.h',
            '../src/libGLESv2/entry_points_egl_autogen.cpp',
            '../src/libGLESv2/entry_points_egl_autogen.h',
            '../src/libGLESv2/entry_points_egl_ext_autogen.cpp',
            '../src/libGLESv2/entry_points_egl_ext_autogen.h',
            '../src/libGLESv2/entry_points_gles_1_0_autogen.cpp',
            '../src/libGLESv2/entry_points_gles_1_0_autogen.h',
            '../src/libGLESv2/entry_points_gles_2_0_autogen.cpp',
            '../src/libGLESv2/entry_points_gles_2_0_autogen.h',
            '../src/libGLESv2/entry_points_gles_3_0_autogen.cpp',
            '../src/libGLESv2/entry_points_gles_3_0_autogen.h',
            '../src/libGLESv2/entry_points_gles_3_1_autogen.cpp',
            '../src/libGLESv2/entry_points_gles_3_1_autogen.h',
            '../src/libGLESv2/entry_points_gles_3_2_autogen.cpp',
            '../src/libGLESv2/entry_points_gles_3_2_autogen.h',
            '../src/libGLESv2/entry_points_gles_ext_autogen.cpp',
            '../src/libGLESv2/entry_points_gles_ext_autogen.h',
            '../src/libGLESv2/libGLESv2_autogen.cpp',
            '../src/libGLESv2/libGLESv2_autogen.def',
            '../src/libGLESv2/libGLESv2_no_capture_autogen.def',
            '../src/libGLESv2/libGLESv2_with_capture_autogen.def',
            '../src/libGLESv2/egl_context_lock_autogen.h',
            '../util/capture/frame_capture_replay_autogen.cpp',
        ]

        if sys.argv[1] == 'inputs':
            print(','.join(inputs))
        elif sys.argv[1] == 'outputs':
            print(','.join(outputs))
        else:
            print('Invalid script parameters')
            return 1
        return 0

    glesdecls = {}
    glesdecls['core'] = {}
    glesdecls['exts'] = {}
    for ver in registry_xml.GLES_VERSIONS:
        glesdecls['core'][ver] = []
    for ver in ['GLES1 Extensions', 'GLES2+ Extensions', 'ANGLE Extensions']:
        glesdecls['exts'][ver] = {}

    libgles_ep_defs = []
    libgles_ep_exports = []

    xml = registry_xml.RegistryXML('gl.xml', 'gl_angle_ext.xml')

    # Stores core commands to keep track of duplicates
    all_commands_no_suffix = []
    all_commands_with_suffix = []

    # Collect all context-private-state-accessing helper declarations
    context_private_call_protos = []
    context_private_call_functions = set()

    # First run through the main GLES entry points.  Since ES2+ is the primary use
    # case, we go through those first and then add ES1-only APIs at the end.
    for major_version, minor_version in registry_xml.GLES_VERSIONS:
        version = "{}_{}".format(major_version, minor_version)
        annotation = "GLES_{}".format(version)
        name_prefix = "GL_ES_VERSION_"

        if major_version == 1:
            name_prefix = "GL_VERSION_ES_CM_"

        comment = version.replace("_", ".")
        feature_name = "{}{}".format(name_prefix, version)

        xml.AddCommands(feature_name, version)

        version_commands = xml.commands[version]
        all_commands_no_suffix.extend(xml.commands[version])
        all_commands_with_suffix.extend(xml.commands[version])

        eps = GLEntryPoints(apis.GLES, xml, version_commands, is_gles1=(major_version == 1))
        eps.decls.insert(0, "extern \"C\" {")
        eps.decls.append("} // extern \"C\"")
        eps.defs.insert(0, "extern \"C\" {")
        eps.defs.append("} // extern \"C\"")

        # Write the version as a comment before the first EP.
        libgles_ep_exports.append("\n    ; OpenGL ES %s" % comment)

        libgles_ep_defs += ["\n// OpenGL ES %s" % comment] + eps.export_defs
        libgles_ep_exports += get_exports(version_commands)

        major_if_not_one = major_version if major_version != 1 else ""
        minor_if_not_zero = minor_version if minor_version != 0 else ""

        header_includes = TEMPLATE_HEADER_INCLUDES.format(
            major=major_if_not_one, minor=minor_if_not_zero)

        version_annotation = "%s%s" % (major_version, minor_if_not_zero)
        source_includes = TEMPLATE_SOURCES_INCLUDES.format(
            header_version=annotation.lower(), validation_header_version="ES" + version_annotation)

        write_file(annotation, "GLES " + comment, TEMPLATE_ENTRY_POINT_HEADER,
                   "\n".join(eps.decls), "h", header_includes, "libGLESv2", "gl.xml")
        write_file(annotation, "GLES " + comment, TEMPLATE_ENTRY_POINT_SOURCE, "\n".join(eps.defs),
                   "cpp", source_includes, "libGLESv2", "gl.xml")

        glesdecls['core'][(major_version,
                           minor_version)] = get_decls(apis.GLES, CONTEXT_DECL_FORMAT,
                                                       xml.all_commands, version_commands, [],
                                                       GLEntryPoints.get_packed_enums())

        validation_annotation = "ES%s%s" % (major_version, minor_if_not_zero)
        write_gl_validation_header(validation_annotation, "ES %s" % comment, eps.validation_protos,
                                   "gl.xml and gl_angle_ext.xml")

        context_private_call_protos += eps.context_private_call_protos
        context_private_call_functions.update(eps.context_private_call_functions)

        write_capture_header(apis.GLES, 'gles_' + version, comment, eps.capture_protos,
                             eps.capture_pointer_funcs)
        write_capture_source(apis.GLES, 'gles_' + version, validation_annotation, comment,
                             eps.capture_methods)

    # After we finish with the main entry points, we process the extensions.
    extension_decls = ["extern \"C\" {"]
    extension_defs = ["extern \"C\" {"]
    extension_commands = []

    # Accumulated validation prototypes.
    ext_validation_protos = []
    ext_capture_protos = []
    ext_capture_methods = []
    ext_capture_pointer_funcs = []

    for gles1ext in registry_xml.gles1_extensions:
        glesdecls['exts']['GLES1 Extensions'][gles1ext] = []
    for glesext in registry_xml.gles_extensions:
        glesdecls['exts']['GLES2+ Extensions'][glesext] = []
    for angle_ext in registry_xml.angle_extensions:
        glesdecls['exts']['ANGLE Extensions'][angle_ext] = []

    xml.AddExtensionCommands(registry_xml.supported_extensions, ['gles2', 'gles1'])

    for extension_name, ext_cmd_names in sorted(xml.ext_data.items()):
        extension_commands.extend(xml.ext_data[extension_name])

        # Detect and filter duplicate extensions.
        is_gles1 = extension_name in registry_xml.gles1_extensions
        eps = GLEntryPoints(apis.GLES, xml, ext_cmd_names, is_gles1=is_gles1)

        # Write the extension name as a comment before the first EP.
        comment = "\n// {}".format(extension_name)
        libgles_ep_exports.append("\n    ; %s" % extension_name)

        extension_defs += [comment] + eps.defs
        extension_decls += [comment] + eps.decls

        # Avoid writing out entry points defined by a prior extension.
        for dupe in xml.ext_dupes[extension_name]:
            msg = "// {} is already defined.\n".format(strip_api_prefix(dupe))
            extension_defs.append(msg)

        ext_validation_protos += [comment] + eps.validation_protos
        ext_capture_protos += [comment] + eps.capture_protos
        ext_capture_methods += eps.capture_methods
        ext_capture_pointer_funcs += eps.capture_pointer_funcs

        for proto, function in zip(eps.context_private_call_protos,
                                   eps.context_private_call_functions):
            if function not in context_private_call_functions:
                context_private_call_protos.append(proto)
        context_private_call_functions.update(eps.context_private_call_functions)

        libgles_ep_defs += [comment] + eps.export_defs
        libgles_ep_exports += get_exports(ext_cmd_names)

        if (extension_name in registry_xml.gles1_extensions and
                extension_name not in GLES1_NO_CONTEXT_DECL_EXTENSIONS):
            glesdecls['exts']['GLES1 Extensions'][extension_name] = get_decls(
                apis.GLES, CONTEXT_DECL_FORMAT, xml.all_commands, ext_cmd_names,
                all_commands_no_suffix, GLEntryPoints.get_packed_enums())
        if extension_name in registry_xml.gles_extensions:
            glesdecls['exts']['GLES2+ Extensions'][extension_name] = get_decls(
                apis.GLES, CONTEXT_DECL_FORMAT, xml.all_commands, ext_cmd_names,
                all_commands_no_suffix, GLEntryPoints.get_packed_enums())
        if extension_name in registry_xml.angle_extensions:
            glesdecls['exts']['ANGLE Extensions'][extension_name] = get_decls(
                apis.GLES, CONTEXT_DECL_FORMAT, xml.all_commands, ext_cmd_names,
                all_commands_no_suffix, GLEntryPoints.get_packed_enums())

    write_context_private_call_header(context_private_call_protos, "gl.xml and gl_angle_ext.xml",
                                      TEMPLATE_CONTEXT_PRIVATE_CALL_HEADER)

    for name in extension_commands:
        all_commands_with_suffix.append(name)
        all_commands_no_suffix.append(strip_suffix(apis.GLES, name))

    # OpenCL
    clxml = registry_xml.RegistryXML('cl.xml')

    cl_validation_protos = []
    cl_decls = ["namespace cl\n{"]
    cl_defs = ["namespace cl\n{"]
    libcl_ep_defs = []
    libcl_windows_def_exports = []
    cl_commands = []

    for major_version, minor_version in registry_xml.CL_VERSIONS:
        version = "%d_%d" % (major_version, minor_version)
        annotation = "CL_%s" % version
        name_prefix = "CL_VERSION_"

        comment = version.replace("_", ".")
        feature_name = "%s%s" % (name_prefix, version)

        clxml.AddCommands(feature_name, version)

        cl_version_commands = clxml.commands[version]
        cl_commands += cl_version_commands

        # Spec revs may have no new commands.
        if not cl_version_commands:
            continue

        eps = CLEntryPoints(clxml, cl_version_commands)

        comment = "\n// CL %d.%d" % (major_version, minor_version)
        win_def_comment = "\n    ; CL %d.%d" % (major_version, minor_version)

        cl_decls += [comment] + eps.decls
        cl_defs += [comment] + eps.defs
        libcl_ep_defs += [comment] + eps.export_defs
        cl_validation_protos += [comment] + eps.validation_protos
        libcl_windows_def_exports += [win_def_comment] + get_exports(clxml.commands[version])

    clxml.AddExtensionCommands(registry_xml.supported_cl_extensions, ['cl'])
    for extension_name, ext_cmd_names in sorted(clxml.ext_data.items()):

        # Extensions may have no new commands.
        if not ext_cmd_names:
            continue

        # Detect and filter duplicate extensions.
        eps = CLEntryPoints(clxml, ext_cmd_names)

        comment = "\n// %s" % extension_name
        win_def_comment = "\n    ; %s" % (extension_name)

        cl_commands += ext_cmd_names

        cl_decls += [comment] + eps.decls
        cl_defs += [comment] + eps.defs
        libcl_ep_defs += [comment] + eps.export_defs
        cl_validation_protos += [comment] + eps.validation_protos
        libcl_windows_def_exports += [win_def_comment] + get_exports(ext_cmd_names)

        # Avoid writing out entry points defined by a prior extension.
        for dupe in clxml.ext_dupes[extension_name]:
            msg = "// %s is already defined.\n" % strip_api_prefix(dupe)
            cl_defs.append(msg)

    cl_decls.append("}  // namespace cl")
    cl_defs.append("}  // namespace cl")

    write_file("cl", "CL", TEMPLATE_ENTRY_POINT_HEADER, "\n".join(cl_decls), "h",
               LIBCL_HEADER_INCLUDES, "libGLESv2", "cl.xml")
    write_file("cl", "CL", TEMPLATE_ENTRY_POINT_SOURCE, "\n".join(cl_defs), "cpp",
               LIBCL_SOURCE_INCLUDES, "libGLESv2", "cl.xml")
    write_validation_header("CL", "CL", cl_validation_protos, "cl.xml",
                            TEMPLATE_CL_VALIDATION_HEADER)
    write_stubs_header("CL", "cl", "CL", "cl.xml", CL_STUBS_HEADER_PATH, clxml.all_commands,
                       cl_commands, CLEntryPoints.get_packed_enums(), CL_PACKED_TYPES)

    # EGL
    eglxml = registry_xml.RegistryXML('egl.xml', 'egl_angle_ext.xml')

    egl_validation_protos = []
    egl_context_lock_protos = []
    egl_decls = ["extern \"C\" {"]
    egl_defs = ["extern \"C\" {"]
    libegl_ep_defs = []
    libegl_windows_def_exports = []
    egl_commands = []
    egl_capture_protos = []
    egl_capture_methods = []

    for major_version, minor_version in registry_xml.EGL_VERSIONS:
        version = "%d_%d" % (major_version, minor_version)
        annotation = "EGL_%s" % version
        name_prefix = "EGL_VERSION_"

        comment = version.replace("_", ".")
        feature_name = "%s%s" % (name_prefix, version)

        eglxml.AddCommands(feature_name, version)

        egl_version_commands = eglxml.commands[version]
        egl_commands += egl_version_commands

        # Spec revs may have no new commands.
        if not egl_version_commands:
            continue

        eps = EGLEntryPoints(eglxml, egl_version_commands)

        comment = "\n// EGL %d.%d" % (major_version, minor_version)
        win_def_comment = "\n    ; EGL %d.%d" % (major_version, minor_version)

        egl_decls += [comment] + eps.decls
        egl_defs += [comment] + eps.defs
        libegl_ep_defs += [comment] + eps.export_defs
        egl_validation_protos += [comment] + eps.validation_protos
        egl_context_lock_protos += [comment] + eps.context_lock_protos
        libegl_windows_def_exports += [win_def_comment] + get_exports(eglxml.commands[version])
        egl_capture_protos += eps.capture_protos
        egl_capture_methods += eps.capture_methods

    egl_decls.append("} // extern \"C\"")
    egl_defs.append("} // extern \"C\"")

    write_file("egl", "EGL", TEMPLATE_ENTRY_POINT_HEADER, "\n".join(egl_decls), "h",
               EGL_HEADER_INCLUDES, "libGLESv2", "egl.xml")
    write_file("egl", "EGL", TEMPLATE_ENTRY_POINT_SOURCE, "\n".join(egl_defs), "cpp",
               EGL_SOURCE_INCLUDES, "libGLESv2", "egl.xml")
    write_stubs_header("EGL", "egl", "EGL", "egl.xml", EGL_STUBS_HEADER_PATH, eglxml.all_commands,
                       egl_commands, EGLEntryPoints.get_packed_enums(), EGL_PACKED_TYPES)

    eglxml.AddExtensionCommands(registry_xml.supported_egl_extensions, ['egl'])
    egl_ext_decls = ["extern \"C\" {"]
    egl_ext_defs = ["extern \"C\" {"]
    egl_ext_commands = []

    for extension_name, ext_cmd_names in sorted(eglxml.ext_data.items()):

        # Extensions may have no new commands.
        if not ext_cmd_names:
            continue

        # Detect and filter duplicate extensions.
        eps = EGLEntryPoints(eglxml, ext_cmd_names)

        comment = "\n// %s" % extension_name
        win_def_comment = "\n    ; %s" % (extension_name)

        egl_ext_commands += ext_cmd_names

        egl_ext_decls += [comment] + eps.decls
        egl_ext_defs += [comment] + eps.defs
        libegl_ep_defs += [comment] + eps.export_defs
        egl_validation_protos += [comment] + eps.validation_protos
        egl_context_lock_protos += [comment] + eps.context_lock_protos
        libegl_windows_def_exports += [win_def_comment] + get_exports(ext_cmd_names)
        egl_capture_protos += eps.capture_protos
        egl_capture_methods += eps.capture_methods

        # Avoid writing out entry points defined by a prior extension.
        for dupe in eglxml.ext_dupes[extension_name]:
            msg = "// %s is already defined.\n" % strip_api_prefix(dupe)
            egl_ext_defs.append(msg)

    egl_ext_decls.append("} // extern \"C\"")
    egl_ext_defs.append("} // extern \"C\"")

    write_file("egl_ext", "EGL Extension", TEMPLATE_ENTRY_POINT_HEADER, "\n".join(egl_ext_decls),
               "h", EGL_EXT_HEADER_INCLUDES, "libGLESv2", "egl.xml and egl_angle_ext.xml")
    write_file("egl_ext", "EGL Extension", TEMPLATE_ENTRY_POINT_SOURCE, "\n".join(egl_ext_defs),
               "cpp", EGL_EXT_SOURCE_INCLUDES, "libGLESv2", "egl.xml and egl_angle_ext.xml")
    write_validation_header("EGL", "EGL", egl_validation_protos, "egl.xml and egl_angle_ext.xml",
                            TEMPLATE_EGL_VALIDATION_HEADER)
    write_context_lock_header("EGL", "EGL", egl_context_lock_protos,
                              "egl.xml and egl_angle_ext.xml", TEMPLATE_EGL_CONTEXT_LOCK_HEADER)
    write_stubs_header("EGL", "egl_ext", "EXT extension", "egl.xml and egl_angle_ext.xml",
                       EGL_EXT_STUBS_HEADER_PATH, eglxml.all_commands, egl_ext_commands,
                       EGLEntryPoints.get_packed_enums(), EGL_PACKED_TYPES)

    write_capture_header(apis.EGL, 'egl', 'EGL', egl_capture_protos, [])
    write_capture_source(apis.EGL, 'egl', 'EGL', 'all', egl_capture_methods)

    extension_decls.append("} // extern \"C\"")
    extension_defs.append("} // extern \"C\"")

    write_file("gles_ext", "GLES extension", TEMPLATE_ENTRY_POINT_HEADER,
               "\n".join([item for item in extension_decls]), "h", GLES_EXT_HEADER_INCLUDES,
               "libGLESv2", "gl.xml and gl_angle_ext.xml")
    write_file("gles_ext", "GLES extension", TEMPLATE_ENTRY_POINT_SOURCE,
               "\n".join([item for item in extension_defs]), "cpp", GLES_EXT_SOURCE_INCLUDES,
               "libGLESv2", "gl.xml and gl_angle_ext.xml")

    write_gl_validation_header("ESEXT", "ES extension", ext_validation_protos,
                               "gl.xml and gl_angle_ext.xml")
    write_capture_header(apis.GLES, "gles_ext", "extension", ext_capture_protos,
                         ext_capture_pointer_funcs)
    write_capture_source(apis.GLES, "gles_ext", "ESEXT", "extension", ext_capture_methods)

    write_context_api_decls(glesdecls, "gles")

    # Entry point enum
    unsorted_enums = clxml.GetEnums() + eglxml.GetEnums() + xml.GetEnums()
    all_enums = [('Invalid', 'Invalid')] + sorted(list(set(unsorted_enums)))

    entry_points_enum_header = TEMPLATE_ENTRY_POINTS_ENUM_HEADER.format(
        script_name=os.path.basename(sys.argv[0]),
        data_source_name="gl.xml and gl_angle_ext.xml",
        lib="GL/GLES",
        entry_points_list=",\n".join(["    " + enum for (enum, _) in all_enums]))

    entry_points_enum_header_path = path_to("common", "entry_points_enum_autogen.h")
    with open(entry_points_enum_header_path, "w") as out:
        out.write(entry_points_enum_header)
        out.close()

    entry_points_cases = [
        TEMPLATE_ENTRY_POINTS_NAME_CASE.format(enum=enum, cmd=cmd) for (enum, cmd) in all_enums
    ]
    entry_points_enum_source = TEMPLATE_ENTRY_POINTS_ENUM_SOURCE.format(
        script_name=os.path.basename(sys.argv[0]),
        data_source_name="gl.xml and gl_angle_ext.xml",
        lib="GL/GLES",
        entry_points_name_cases="\n".join(entry_points_cases))

    entry_points_enum_source_path = path_to("common", "entry_points_enum_autogen.cpp")
    with open(entry_points_enum_source_path, "w") as out:
        out.write(entry_points_enum_source)
        out.close()

    write_export_files("\n".join([item for item in libgles_ep_defs]), LIBGLESV2_EXPORT_INCLUDES,
                       "gl.xml and gl_angle_ext.xml", "libGLESv2", "OpenGL ES")
    write_export_files("\n".join([item for item in libegl_ep_defs]),
                       LIBEGL_EXPORT_INCLUDES_AND_PREAMBLE, "egl.xml and egl_angle_ext.xml",
                       "libEGL", "EGL")
    write_export_files("\n".join([item for item in libcl_ep_defs]), LIBCL_EXPORT_INCLUDES,
                       "cl.xml", "libOpenCL", "CL")

    libgles_ep_exports += get_egl_exports()

    everything = "Khronos and ANGLE XML files"

    for lib in [
            "libGLESv2" + suffix
            for suffix in ["", "_no_capture", "_with_capture", "_vulkan_secondaries"]
    ]:
        write_windows_def_file(everything, lib, lib, "libGLESv2", libgles_ep_exports)

    for lib in ["libEGL" + suffix for suffix in ["", "_vulkan_secondaries"]]:
        write_windows_def_file("egl.xml and egl_angle_ext.xml", lib, lib, "libEGL",
                               libegl_windows_def_exports)

    all_gles_param_types = sorted(GLEntryPoints.all_param_types)
    all_egl_param_types = sorted(EGLEntryPoints.all_param_types)
    resource_id_types = get_resource_id_types(GLEntryPoints.all_param_types)
    # Get a sorted list of param types without duplicates
    all_param_types = sorted(list(set(all_gles_param_types + all_egl_param_types)))
    write_capture_helper_header(all_param_types)
    write_capture_helper_source(all_param_types)
    write_capture_replay_source(xml.all_commands, all_commands_with_suffix,
                                GLEntryPoints.get_packed_enums(), eglxml.all_commands,
                                egl_commands, EGLEntryPoints.get_packed_enums(), resource_id_types)


if __name__ == '__main__':
    sys.exit(main())
