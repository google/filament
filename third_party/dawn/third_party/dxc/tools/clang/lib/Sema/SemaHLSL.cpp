//===--- SemaHLSL.cpp       - HLSL support for AST nodes and operations ---===//
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// SemaHLSL.cpp                                                              //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
//  This file implements the semantic support for HLSL.                      //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "clang/Sema/SemaHLSL.h"
#include "VkConstantsTables.h"
#include "dxc/DXIL/DxilConstants.h"
#include "dxc/DXIL/DxilFunctionProps.h"
#include "dxc/DXIL/DxilShaderModel.h"
#include "dxc/DXIL/DxilUtil.h"
#include "dxc/HLSL/HLOperations.h"
#include "dxc/HlslIntrinsicOp.h"
#include "dxc/Support/Global.h"
#include "dxc/Support/WinIncludes.h"
#include "dxc/dxcapi.internal.h"
#include "gen_intrin_main_tables_15.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Attr.h"
#include "clang/AST/DeclBase.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/ExternalASTSource.h"
#include "clang/AST/HlslTypes.h"
#include "clang/AST/TypeLoc.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/Specifiers.h"
#include "clang/Parse/ParseDiagnostic.h"
#include "clang/Sema/ExternalSemaSource.h"
#include "clang/Sema/Initialization.h"
#include "clang/Sema/Lookup.h"
#include "clang/Sema/Overload.h"
#include "clang/Sema/SemaDiagnostic.h"
#include "clang/Sema/Template.h"
#include "clang/Sema/TemplateDeduction.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <array>
#include <bitset>
#include <float.h>

enum ArBasicKind {
  AR_BASIC_BOOL,
  AR_BASIC_LITERAL_FLOAT,
  AR_BASIC_FLOAT16,
  AR_BASIC_FLOAT32_PARTIAL_PRECISION,
  AR_BASIC_FLOAT32,
  AR_BASIC_FLOAT64,
  AR_BASIC_LITERAL_INT,
  AR_BASIC_INT8,
  AR_BASIC_UINT8,
  AR_BASIC_INT16,
  AR_BASIC_UINT16,
  AR_BASIC_INT32,
  AR_BASIC_UINT32,
  AR_BASIC_INT64,
  AR_BASIC_UINT64,

  AR_BASIC_MIN10FLOAT,
  AR_BASIC_MIN16FLOAT,
  AR_BASIC_MIN12INT,
  AR_BASIC_MIN16INT,
  AR_BASIC_MIN16UINT,
  AR_BASIC_INT8_4PACKED,
  AR_BASIC_UINT8_4PACKED,
  AR_BASIC_ENUM,

  AR_BASIC_COUNT,

  //
  // Pseudo-entries for intrinsic tables and such.
  //

  AR_BASIC_NONE,
  AR_BASIC_UNKNOWN,
  AR_BASIC_NOCAST,
  AR_BASIC_DEPENDENT,
  //
  // The following pseudo-entries represent higher-level
  // object types that are treated as units.
  //

  AR_BASIC_POINTER,
  AR_BASIC_ENUM_CLASS,

  AR_OBJECT_NULL,
  AR_OBJECT_STRING_LITERAL,
  AR_OBJECT_STRING,

  // AR_OBJECT_TEXTURE,
  AR_OBJECT_TEXTURE1D,
  AR_OBJECT_TEXTURE1D_ARRAY,
  AR_OBJECT_TEXTURE2D,
  AR_OBJECT_TEXTURE2D_ARRAY,
  AR_OBJECT_TEXTURE3D,
  AR_OBJECT_TEXTURECUBE,
  AR_OBJECT_TEXTURECUBE_ARRAY,
  AR_OBJECT_TEXTURE2DMS,
  AR_OBJECT_TEXTURE2DMS_ARRAY,

  AR_OBJECT_SAMPLER,
  AR_OBJECT_SAMPLER1D,
  AR_OBJECT_SAMPLER2D,
  AR_OBJECT_SAMPLER3D,
  AR_OBJECT_SAMPLERCUBE,
  AR_OBJECT_SAMPLERCOMPARISON,

  AR_OBJECT_BUFFER,

  //
  // View objects are only used as variable/types within the Effects
  // framework, for example in calls to OMSetRenderTargets.
  //

  AR_OBJECT_RENDERTARGETVIEW,
  AR_OBJECT_DEPTHSTENCILVIEW,

  //
  // Shader objects are only used as variable/types within the Effects
  // framework, for example as a result of CompileShader().
  //

  AR_OBJECT_COMPUTESHADER,
  AR_OBJECT_DOMAINSHADER,
  AR_OBJECT_GEOMETRYSHADER,
  AR_OBJECT_HULLSHADER,
  AR_OBJECT_PIXELSHADER,
  AR_OBJECT_VERTEXSHADER,
  AR_OBJECT_PIXELFRAGMENT,
  AR_OBJECT_VERTEXFRAGMENT,

  AR_OBJECT_STATEBLOCK,

  AR_OBJECT_RASTERIZER,
  AR_OBJECT_DEPTHSTENCIL,
  AR_OBJECT_BLEND,

  AR_OBJECT_POINTSTREAM,
  AR_OBJECT_LINESTREAM,
  AR_OBJECT_TRIANGLESTREAM,

  AR_OBJECT_INPUTPATCH,
  AR_OBJECT_OUTPUTPATCH,

  AR_OBJECT_RWTEXTURE1D,
  AR_OBJECT_RWTEXTURE1D_ARRAY,
  AR_OBJECT_RWTEXTURE2D,
  AR_OBJECT_RWTEXTURE2D_ARRAY,
  AR_OBJECT_RWTEXTURE3D,
  AR_OBJECT_RWBUFFER,

  AR_OBJECT_BYTEADDRESS_BUFFER,
  AR_OBJECT_RWBYTEADDRESS_BUFFER,
  AR_OBJECT_STRUCTURED_BUFFER,
  AR_OBJECT_RWSTRUCTURED_BUFFER,
  AR_OBJECT_RWSTRUCTURED_BUFFER_ALLOC,
  AR_OBJECT_RWSTRUCTURED_BUFFER_CONSUME,
  AR_OBJECT_APPEND_STRUCTURED_BUFFER,
  AR_OBJECT_CONSUME_STRUCTURED_BUFFER,

  AR_OBJECT_CONSTANT_BUFFER,
  AR_OBJECT_TEXTURE_BUFFER,

  AR_OBJECT_ROVBUFFER,
  AR_OBJECT_ROVBYTEADDRESS_BUFFER,
  AR_OBJECT_ROVSTRUCTURED_BUFFER,
  AR_OBJECT_ROVTEXTURE1D,
  AR_OBJECT_ROVTEXTURE1D_ARRAY,
  AR_OBJECT_ROVTEXTURE2D,
  AR_OBJECT_ROVTEXTURE2D_ARRAY,
  AR_OBJECT_ROVTEXTURE3D,

  AR_OBJECT_FEEDBACKTEXTURE2D,
  AR_OBJECT_FEEDBACKTEXTURE2D_ARRAY,

// SPIRV change starts
#ifdef ENABLE_SPIRV_CODEGEN
  AR_OBJECT_VK_SUBPASS_INPUT,
  AR_OBJECT_VK_SUBPASS_INPUT_MS,
  AR_OBJECT_VK_SPIRV_TYPE,
  AR_OBJECT_VK_SPIRV_OPAQUE_TYPE,
  AR_OBJECT_VK_INTEGRAL_CONSTANT,
  AR_OBJECT_VK_LITERAL,
  AR_OBJECT_VK_SPV_INTRINSIC_TYPE,
  AR_OBJECT_VK_SPV_INTRINSIC_RESULT_ID,
  AR_OBJECT_VK_BUFFER_POINTER,
#endif // ENABLE_SPIRV_CODEGEN
  // SPIRV change ends

  AR_OBJECT_INNER, // Used for internal type object

  AR_OBJECT_LEGACY_EFFECT,

  AR_OBJECT_WAVE,

  AR_OBJECT_RAY_DESC,
  AR_OBJECT_ACCELERATION_STRUCT,
  AR_OBJECT_USER_DEFINED_TYPE,
  AR_OBJECT_TRIANGLE_INTERSECTION_ATTRIBUTES,

  // subobjects
  AR_OBJECT_STATE_OBJECT_CONFIG,
  AR_OBJECT_GLOBAL_ROOT_SIGNATURE,
  AR_OBJECT_LOCAL_ROOT_SIGNATURE,
  AR_OBJECT_SUBOBJECT_TO_EXPORTS_ASSOC,
  AR_OBJECT_RAYTRACING_SHADER_CONFIG,
  AR_OBJECT_RAYTRACING_PIPELINE_CONFIG,
  AR_OBJECT_TRIANGLE_HIT_GROUP,
  AR_OBJECT_PROCEDURAL_PRIMITIVE_HIT_GROUP,
  AR_OBJECT_RAYTRACING_PIPELINE_CONFIG1,

  // RayQuery
  AR_OBJECT_RAY_QUERY,

  // Heap Resource
  AR_OBJECT_HEAP_RESOURCE,
  AR_OBJECT_HEAP_SAMPLER,

  AR_OBJECT_RWTEXTURE2DMS,
  AR_OBJECT_RWTEXTURE2DMS_ARRAY,

  // Work Graphs
  AR_OBJECT_EMPTY_NODE_INPUT,
  AR_OBJECT_DISPATCH_NODE_INPUT_RECORD,
  AR_OBJECT_RWDISPATCH_NODE_INPUT_RECORD,
  AR_OBJECT_GROUP_NODE_INPUT_RECORDS,
  AR_OBJECT_RWGROUP_NODE_INPUT_RECORDS,
  AR_OBJECT_THREAD_NODE_INPUT_RECORD,
  AR_OBJECT_RWTHREAD_NODE_INPUT_RECORD,

  AR_OBJECT_NODE_OUTPUT,
  AR_OBJECT_EMPTY_NODE_OUTPUT,
  AR_OBJECT_NODE_OUTPUT_ARRAY,
  AR_OBJECT_EMPTY_NODE_OUTPUT_ARRAY,

  AR_OBJECT_THREAD_NODE_OUTPUT_RECORDS,
  AR_OBJECT_GROUP_NODE_OUTPUT_RECORDS,

  // Shader Execution Reordering
  AR_OBJECT_HIT_OBJECT,

  AR_BASIC_MAXIMUM_COUNT
};

#define AR_BASIC_TEXTURE_MS_CASES                                              \
  case AR_OBJECT_TEXTURE2DMS:                                                  \
  case AR_OBJECT_TEXTURE2DMS_ARRAY:                                            \
  case AR_OBJECT_RWTEXTURE2DMS:                                                \
  case AR_OBJECT_RWTEXTURE2DMS_ARRAY

#define AR_BASIC_NON_TEXTURE_MS_CASES                                          \
  case AR_OBJECT_TEXTURE1D:                                                    \
  case AR_OBJECT_TEXTURE1D_ARRAY:                                              \
  case AR_OBJECT_TEXTURE2D:                                                    \
  case AR_OBJECT_TEXTURE2D_ARRAY:                                              \
  case AR_OBJECT_TEXTURE3D:                                                    \
  case AR_OBJECT_TEXTURECUBE:                                                  \
  case AR_OBJECT_TEXTURECUBE_ARRAY

#define AR_BASIC_TEXTURE_CASES                                                 \
  AR_BASIC_TEXTURE_MS_CASES:                                                   \
  AR_BASIC_NON_TEXTURE_MS_CASES

#define AR_BASIC_NON_CMP_SAMPLER_CASES                                         \
  case AR_OBJECT_SAMPLER:                                                      \
  case AR_OBJECT_SAMPLER1D:                                                    \
  case AR_OBJECT_SAMPLER2D:                                                    \
  case AR_OBJECT_SAMPLER3D:                                                    \
  case AR_OBJECT_SAMPLERCUBE

#define AR_BASIC_ROBJECT_CASES                                                 \
  case AR_OBJECT_BLEND:                                                        \
  case AR_OBJECT_RASTERIZER:                                                   \
  case AR_OBJECT_DEPTHSTENCIL:                                                 \
  case AR_OBJECT_STATEBLOCK

//
// Properties of entries in the ArBasicKind enumeration.
// These properties are intended to allow easy identification
// of classes of basic kinds.  More specific checks on the
// actual kind values could then be done.
//

// The first four bits are used as a subtype indicator,
// such as bit count for primitive kinds or specific
// types for non-primitive-data kinds.
#define BPROP_SUBTYPE_MASK 0x0000000f

// Bit counts must be ordered from smaller to larger.
#define BPROP_BITS0 0x00000000
#define BPROP_BITS8 0x00000001
#define BPROP_BITS10 0x00000002
#define BPROP_BITS12 0x00000003
#define BPROP_BITS16 0x00000004
#define BPROP_BITS32 0x00000005
#define BPROP_BITS64 0x00000006
#define BPROP_BITS_NON_PRIM 0x00000007

#define GET_BPROP_SUBTYPE(_Props) ((_Props)&BPROP_SUBTYPE_MASK)
#define GET_BPROP_BITS(_Props) ((_Props)&BPROP_SUBTYPE_MASK)

#define BPROP_BOOLEAN 0x00000010 // Whether the type is bool
#define BPROP_INTEGER 0x00000020 // Whether the type is an integer
#define BPROP_UNSIGNED                                                         \
  0x00000040 // Whether the type is an unsigned numeric (its absence implies
             // signed)
#define BPROP_NUMERIC 0x00000080 // Whether the type is numeric or boolean
#define BPROP_LITERAL                                                          \
  0x00000100 // Whether the type is a literal float or integer
#define BPROP_FLOATING 0x00000200 // Whether the type is a float
#define BPROP_OBJECT                                                           \
  0x00000400 // Whether the type is an object (including null or stream)
#define BPROP_OTHER                                                            \
  0x00000800 // Whether the type is a pseudo-entry in another table.
#define BPROP_PARTIAL_PRECISION                                                \
  0x00001000 // Whether the type has partial precision for calculations (i.e.,
             // is this 'half')
#define BPROP_POINTER 0x00002000 // Whether the type is a basic pointer.
#define BPROP_TEXTURE 0x00004000 // Whether the type is any kind of texture.
#define BPROP_SAMPLER                                                          \
  0x00008000 // Whether the type is any kind of sampler object.
#define BPROP_STREAM                                                           \
  0x00010000 // Whether the type is a point, line or triangle stream.
#define BPROP_PATCH 0x00020000 // Whether the type is an input or output patch.
#define BPROP_RBUFFER 0x00040000 // Whether the type acts as a read-only buffer.
#define BPROP_RWBUFFER                                                         \
  0x00080000 // Whether the type acts as a read-write buffer.
#define BPROP_PRIMITIVE                                                        \
  0x00100000 // Whether the type is a primitive scalar type.
#define BPROP_MIN_PRECISION                                                    \
  0x00200000 // Whether the type is qualified with a minimum precision.
#define BPROP_ROVBUFFER 0x00400000 // Whether the type is a ROV object.
#define BPROP_FEEDBACKTEXTURE                                                  \
  0x00800000                  // Whether the type is a feedback texture.
#define BPROP_ENUM 0x01000000 // Whether the type is a enum

#define GET_BPROP_PRIM_KIND(_Props)                                            \
  ((_Props) & (BPROP_BOOLEAN | BPROP_INTEGER | BPROP_FLOATING))

#define GET_BPROP_PRIM_KIND_SU(_Props)                                         \
  ((_Props) & (BPROP_BOOLEAN | BPROP_INTEGER | BPROP_FLOATING | BPROP_UNSIGNED))

#define IS_BPROP_PRIMITIVE(_Props) (((_Props)&BPROP_PRIMITIVE) != 0)

#define IS_BPROP_BOOL(_Props) (((_Props)&BPROP_BOOLEAN) != 0)

#define IS_BPROP_FLOAT(_Props) (((_Props)&BPROP_FLOATING) != 0)

#define IS_BPROP_SINT(_Props)                                                  \
  (((_Props) & (BPROP_INTEGER | BPROP_UNSIGNED | BPROP_BOOLEAN)) ==            \
   BPROP_INTEGER)

#define IS_BPROP_UINT(_Props)                                                  \
  (((_Props) & (BPROP_INTEGER | BPROP_UNSIGNED | BPROP_BOOLEAN)) ==            \
   (BPROP_INTEGER | BPROP_UNSIGNED))

#define IS_BPROP_AINT(_Props)                                                  \
  (((_Props) & (BPROP_INTEGER | BPROP_BOOLEAN)) == BPROP_INTEGER)

#define IS_BPROP_STREAM(_Props) (((_Props)&BPROP_STREAM) != 0)

#define IS_BPROP_PATCH(_Props) (((_Props) & BPROP_PATCH) != 0)

#define IS_BPROP_SAMPLER(_Props) (((_Props)&BPROP_SAMPLER) != 0)

#define IS_BPROP_TEXTURE(_Props) (((_Props)&BPROP_TEXTURE) != 0)

#define IS_BPROP_OBJECT(_Props) (((_Props)&BPROP_OBJECT) != 0)

#define IS_BPROP_MIN_PRECISION(_Props) (((_Props)&BPROP_MIN_PRECISION) != 0)

#define IS_BPROP_UNSIGNABLE(_Props)                                            \
  (IS_BPROP_AINT(_Props) && GET_BPROP_BITS(_Props) != BPROP_BITS12)

#define IS_BPROP_ENUM(_Props) (((_Props)&BPROP_ENUM) != 0)

const UINT g_uBasicKindProps[] = {
    BPROP_PRIMITIVE | BPROP_BOOLEAN | BPROP_INTEGER | BPROP_NUMERIC |
        BPROP_BITS0, // AR_BASIC_BOOL

    BPROP_PRIMITIVE | BPROP_NUMERIC | BPROP_FLOATING | BPROP_LITERAL |
        BPROP_BITS0, // AR_BASIC_LITERAL_FLOAT
    BPROP_PRIMITIVE | BPROP_NUMERIC | BPROP_FLOATING |
        BPROP_BITS16, // AR_BASIC_FLOAT16
    BPROP_PRIMITIVE | BPROP_NUMERIC | BPROP_FLOATING | BPROP_BITS32 |
        BPROP_PARTIAL_PRECISION, // AR_BASIC_FLOAT32_PARTIAL_PRECISION
    BPROP_PRIMITIVE | BPROP_NUMERIC | BPROP_FLOATING |
        BPROP_BITS32, // AR_BASIC_FLOAT32
    BPROP_PRIMITIVE | BPROP_NUMERIC | BPROP_FLOATING |
        BPROP_BITS64, // AR_BASIC_FLOAT64

    BPROP_PRIMITIVE | BPROP_NUMERIC | BPROP_INTEGER | BPROP_LITERAL |
        BPROP_BITS0, // AR_BASIC_LITERAL_INT
    BPROP_PRIMITIVE | BPROP_NUMERIC | BPROP_INTEGER |
        BPROP_BITS8, // AR_BASIC_INT8
    BPROP_PRIMITIVE | BPROP_NUMERIC | BPROP_INTEGER | BPROP_UNSIGNED |
        BPROP_BITS8, // AR_BASIC_UINT8
    BPROP_PRIMITIVE | BPROP_NUMERIC | BPROP_INTEGER |
        BPROP_BITS16, // AR_BASIC_INT16
    BPROP_PRIMITIVE | BPROP_NUMERIC | BPROP_INTEGER | BPROP_UNSIGNED |
        BPROP_BITS16, // AR_BASIC_UINT16
    BPROP_PRIMITIVE | BPROP_NUMERIC | BPROP_INTEGER |
        BPROP_BITS32, // AR_BASIC_INT32
    BPROP_PRIMITIVE | BPROP_NUMERIC | BPROP_INTEGER | BPROP_UNSIGNED |
        BPROP_BITS32, // AR_BASIC_UINT32
    BPROP_PRIMITIVE | BPROP_NUMERIC | BPROP_INTEGER |
        BPROP_BITS64, // AR_BASIC_INT64
    BPROP_PRIMITIVE | BPROP_NUMERIC | BPROP_INTEGER | BPROP_UNSIGNED |
        BPROP_BITS64, // AR_BASIC_UINT64

    BPROP_PRIMITIVE | BPROP_NUMERIC | BPROP_FLOATING | BPROP_BITS10 |
        BPROP_MIN_PRECISION, // AR_BASIC_MIN10FLOAT
    BPROP_PRIMITIVE | BPROP_NUMERIC | BPROP_FLOATING | BPROP_BITS16 |
        BPROP_MIN_PRECISION, // AR_BASIC_MIN16FLOAT

    BPROP_PRIMITIVE | BPROP_NUMERIC | BPROP_INTEGER | BPROP_BITS12 |
        BPROP_MIN_PRECISION, // AR_BASIC_MIN12INT
    BPROP_PRIMITIVE | BPROP_NUMERIC | BPROP_INTEGER | BPROP_BITS16 |
        BPROP_MIN_PRECISION, // AR_BASIC_MIN16INT
    BPROP_PRIMITIVE | BPROP_NUMERIC | BPROP_INTEGER | BPROP_UNSIGNED |
        BPROP_BITS16 | BPROP_MIN_PRECISION, // AR_BASIC_MIN16UINT
    BPROP_PRIMITIVE | BPROP_NUMERIC | BPROP_INTEGER | BPROP_UNSIGNED |
        BPROP_BITS32, // AR_BASIC_INT8_4PACKED
    BPROP_PRIMITIVE | BPROP_NUMERIC | BPROP_INTEGER | BPROP_UNSIGNED |
        BPROP_BITS32, // AR_BASIC_UINT8_4PACKED

    BPROP_ENUM | BPROP_NUMERIC | BPROP_INTEGER, // AR_BASIC_ENUM
    BPROP_OTHER,                                // AR_BASIC_COUNT

    //
    // Pseudo-entries for intrinsic tables and such.
    //

    0,           // AR_BASIC_NONE
    BPROP_OTHER, // AR_BASIC_UNKNOWN
    BPROP_OTHER, // AR_BASIC_NOCAST
    0,           // AR_BASIC_DEPENDENT

    //
    // The following pseudo-entries represent higher-level
    // object types that are treated as units.
    //

    BPROP_POINTER, // AR_BASIC_POINTER
    BPROP_ENUM,    // AR_BASIC_ENUM_CLASS

    BPROP_OBJECT | BPROP_RBUFFER, // AR_OBJECT_NULL
    BPROP_OBJECT | BPROP_RBUFFER, // AR_OBJECT_STRING_LITERAL
    BPROP_OBJECT | BPROP_RBUFFER, // AR_OBJECT_STRING

    // BPROP_OBJECT | BPROP_TEXTURE, // AR_OBJECT_TEXTURE
    BPROP_OBJECT | BPROP_TEXTURE, // AR_OBJECT_TEXTURE1D
    BPROP_OBJECT | BPROP_TEXTURE, // AR_OBJECT_TEXTURE1D_ARRAY
    BPROP_OBJECT | BPROP_TEXTURE, // AR_OBJECT_TEXTURE2D
    BPROP_OBJECT | BPROP_TEXTURE, // AR_OBJECT_TEXTURE2D_ARRAY
    BPROP_OBJECT | BPROP_TEXTURE, // AR_OBJECT_TEXTURE3D
    BPROP_OBJECT | BPROP_TEXTURE, // AR_OBJECT_TEXTURECUBE
    BPROP_OBJECT | BPROP_TEXTURE, // AR_OBJECT_TEXTURECUBE_ARRAY
    BPROP_OBJECT | BPROP_TEXTURE, // AR_OBJECT_TEXTURE2DMS
    BPROP_OBJECT | BPROP_TEXTURE, // AR_OBJECT_TEXTURE2DMS_ARRAY

    BPROP_OBJECT | BPROP_SAMPLER, // AR_OBJECT_SAMPLER
    BPROP_OBJECT | BPROP_SAMPLER, // AR_OBJECT_SAMPLER1D
    BPROP_OBJECT | BPROP_SAMPLER, // AR_OBJECT_SAMPLER2D
    BPROP_OBJECT | BPROP_SAMPLER, // AR_OBJECT_SAMPLER3D
    BPROP_OBJECT | BPROP_SAMPLER, // AR_OBJECT_SAMPLERCUBE
    BPROP_OBJECT | BPROP_SAMPLER, // AR_OBJECT_SAMPLERCOMPARISON

    BPROP_OBJECT | BPROP_RBUFFER, // AR_OBJECT_BUFFER
    BPROP_OBJECT,                 // AR_OBJECT_RENDERTARGETVIEW
    BPROP_OBJECT,                 // AR_OBJECT_DEPTHSTENCILVIEW

    BPROP_OBJECT, // AR_OBJECT_COMPUTESHADER
    BPROP_OBJECT, // AR_OBJECT_DOMAINSHADER
    BPROP_OBJECT, // AR_OBJECT_GEOMETRYSHADER
    BPROP_OBJECT, // AR_OBJECT_HULLSHADER
    BPROP_OBJECT, // AR_OBJECT_PIXELSHADER
    BPROP_OBJECT, // AR_OBJECT_VERTEXSHADER
    BPROP_OBJECT, // AR_OBJECT_PIXELFRAGMENT
    BPROP_OBJECT, // AR_OBJECT_VERTEXFRAGMENT

    BPROP_OBJECT, // AR_OBJECT_STATEBLOCK

    BPROP_OBJECT, // AR_OBJECT_RASTERIZER
    BPROP_OBJECT, // AR_OBJECT_DEPTHSTENCIL
    BPROP_OBJECT, // AR_OBJECT_BLEND

    BPROP_OBJECT | BPROP_STREAM, // AR_OBJECT_POINTSTREAM
    BPROP_OBJECT | BPROP_STREAM, // AR_OBJECT_LINESTREAM
    BPROP_OBJECT | BPROP_STREAM, // AR_OBJECT_TRIANGLESTREAM

    BPROP_OBJECT | BPROP_PATCH, // AR_OBJECT_INPUTPATCH
    BPROP_OBJECT | BPROP_PATCH, // AR_OBJECT_OUTPUTPATCH

    BPROP_OBJECT | BPROP_RWBUFFER | BPROP_TEXTURE, // AR_OBJECT_RWTEXTURE1D
    BPROP_OBJECT | BPROP_RWBUFFER |
        BPROP_TEXTURE, // AR_OBJECT_RWTEXTURE1D_ARRAY
    BPROP_OBJECT | BPROP_RWBUFFER | BPROP_TEXTURE, // AR_OBJECT_RWTEXTURE2D
    BPROP_OBJECT | BPROP_RWBUFFER |
        BPROP_TEXTURE, // AR_OBJECT_RWTEXTURE2D_ARRAY
    BPROP_OBJECT | BPROP_RWBUFFER | BPROP_TEXTURE, // AR_OBJECT_RWTEXTURE3D
    BPROP_OBJECT | BPROP_RWBUFFER,                 // AR_OBJECT_RWBUFFER

    BPROP_OBJECT | BPROP_RBUFFER,  // AR_OBJECT_BYTEADDRESS_BUFFER
    BPROP_OBJECT | BPROP_RWBUFFER, // AR_OBJECT_RWBYTEADDRESS_BUFFER
    BPROP_OBJECT | BPROP_RBUFFER,  // AR_OBJECT_STRUCTURED_BUFFER
    BPROP_OBJECT | BPROP_RWBUFFER, // AR_OBJECT_RWSTRUCTURED_BUFFER
    BPROP_OBJECT | BPROP_RWBUFFER, // AR_OBJECT_RWSTRUCTURED_BUFFER_ALLOC
    BPROP_OBJECT | BPROP_RWBUFFER, // AR_OBJECT_RWSTRUCTURED_BUFFER_CONSUME
    BPROP_OBJECT | BPROP_RWBUFFER, // AR_OBJECT_APPEND_STRUCTURED_BUFFER
    BPROP_OBJECT | BPROP_RWBUFFER, // AR_OBJECT_CONSUME_STRUCTURED_BUFFER

    BPROP_OBJECT | BPROP_RBUFFER, // AR_OBJECT_CONSTANT_BUFFER
    BPROP_OBJECT | BPROP_RBUFFER, // AR_OBJECT_TEXTURE_BUFFER

    BPROP_OBJECT | BPROP_RWBUFFER | BPROP_ROVBUFFER, // AR_OBJECT_ROVBUFFER
    BPROP_OBJECT | BPROP_RWBUFFER |
        BPROP_ROVBUFFER, // AR_OBJECT_ROVBYTEADDRESS_BUFFER
    BPROP_OBJECT | BPROP_RWBUFFER |
        BPROP_ROVBUFFER, // AR_OBJECT_ROVSTRUCTURED_BUFFER
    BPROP_OBJECT | BPROP_RWBUFFER | BPROP_ROVBUFFER, // AR_OBJECT_ROVTEXTURE1D
    BPROP_OBJECT | BPROP_RWBUFFER |
        BPROP_ROVBUFFER, // AR_OBJECT_ROVTEXTURE1D_ARRAY
    BPROP_OBJECT | BPROP_RWBUFFER | BPROP_ROVBUFFER, // AR_OBJECT_ROVTEXTURE2D
    BPROP_OBJECT | BPROP_RWBUFFER |
        BPROP_ROVBUFFER, // AR_OBJECT_ROVTEXTURE2D_ARRAY
    BPROP_OBJECT | BPROP_RWBUFFER | BPROP_ROVBUFFER, // AR_OBJECT_ROVTEXTURE3D

    BPROP_OBJECT | BPROP_FEEDBACKTEXTURE, // AR_OBJECT_FEEDBACKTEXTURE2D
    BPROP_OBJECT | BPROP_FEEDBACKTEXTURE, // AR_OBJECT_FEEDBACKTEXTURE2D_ARRAY

// SPIRV change starts
#ifdef ENABLE_SPIRV_CODEGEN
    BPROP_OBJECT | BPROP_RBUFFER, // AR_OBJECT_VK_SUBPASS_INPUT
    BPROP_OBJECT | BPROP_RBUFFER, // AR_OBJECT_VK_SUBPASS_INPUT_MS
    BPROP_OBJECT,                 // AR_OBJECT_VK_SPIRV_TYPE
    BPROP_OBJECT,                 // AR_OBJECT_VK_SPIRV_OPAQUE_TYPE
    BPROP_OBJECT,                 // AR_OBJECT_VK_INTEGRAL_CONSTANT,
    BPROP_OBJECT,                 // AR_OBJECT_VK_LITERAL,
    BPROP_OBJECT, // AR_OBJECT_VK_SPV_INTRINSIC_TYPE use recordType
    BPROP_OBJECT, // AR_OBJECT_VK_SPV_INTRINSIC_RESULT_ID use recordType
    BPROP_OBJECT, // AR_OBJECT_VK_BUFFER_POINTER use recordType
#endif            // ENABLE_SPIRV_CODEGEN
    // SPIRV change ends

    BPROP_OBJECT, // AR_OBJECT_INNER

    BPROP_OBJECT, // AR_OBJECT_LEGACY_EFFECT

    BPROP_OBJECT, // AR_OBJECT_WAVE

    LICOMPTYPE_RAYDESC,             // AR_OBJECT_RAY_DESC
    LICOMPTYPE_ACCELERATION_STRUCT, // AR_OBJECT_ACCELERATION_STRUCT
    LICOMPTYPE_USER_DEFINED_TYPE,   // AR_OBJECT_USER_DEFINED_TYPE
    0, // AR_OBJECT_TRIANGLE_INTERSECTION_ATTRIBUTES

    // subobjects
    0, // AR_OBJECT_STATE_OBJECT_CONFIG,
    0, // AR_OBJECT_GLOBAL_ROOT_SIGNATURE,
    0, // AR_OBJECT_LOCAL_ROOT_SIGNATURE,
    0, // AR_OBJECT_SUBOBJECT_TO_EXPORTS_ASSOC,
    0, // AR_OBJECT_RAYTRACING_SHADER_CONFIG,
    0, // AR_OBJECT_RAYTRACING_PIPELINE_CONFIG,
    0, // AR_OBJECT_TRIANGLE_HIT_GROUP,
    0, // AR_OBJECT_PROCEDURAL_PRIMITIVE_HIT_GROUP,
    0, // AR_OBJECT_RAYTRACING_PIPELINE_CONFIG1,

    LICOMPTYPE_RAY_QUERY, // AR_OBJECT_RAY_QUERY,
    BPROP_OBJECT,         // AR_OBJECT_HEAP_RESOURCE,
    BPROP_OBJECT,         // AR_OBJECT_HEAP_SAMPLER,

    BPROP_OBJECT | BPROP_RWBUFFER | BPROP_TEXTURE, // AR_OBJECT_RWTEXTURE2DMS
    BPROP_OBJECT | BPROP_RWBUFFER |
        BPROP_TEXTURE, // AR_OBJECT_RWTEXTURE2DMS_ARRAY

    // WorkGraphs
    BPROP_OBJECT,                  // AR_OBJECT_EMPTY_NODE_INPUT
    BPROP_OBJECT,                  // AR_OBJECT_DISPATCH_NODE_INPUT_RECORD
    BPROP_OBJECT | BPROP_RWBUFFER, // AR_OBJECT_RWDISPATCH_NODE_INPUT_RECORD
    BPROP_OBJECT,                  // AR_OBJECT_GROUP_NODE_INPUT_RECORDS
    BPROP_OBJECT | BPROP_RWBUFFER, // AR_OBJECT_RWGROUP_NODE_INPUT_RECORDS
    BPROP_OBJECT,                  // AR_OBJECT_THREAD_NODE_INPUT_RECORD
    BPROP_OBJECT | BPROP_RWBUFFER, // AR_OBJECT_RWTHREAD_NODE_INPUT_RECORD

    BPROP_OBJECT, // AR_OBJECT_NODE_OUTPUT
    BPROP_OBJECT, // AR_OBJECT_EMPTY_NODE_OUTPUT
    BPROP_OBJECT, // AR_OBJECT_NODE_OUTPUT_ARRAY
    BPROP_OBJECT, // AR_OBJECT_EMPTY_NODE_OUTPUT_ARRAY

    BPROP_OBJECT | BPROP_RWBUFFER, // AR_OBJECT_THREAD_NODE_OUTPUT_RECORDS,
    BPROP_OBJECT | BPROP_RWBUFFER, // AR_OBJECT_GROUP_NODE_OUTPUT_RECORDS,

    // Shader Execution Reordering
    LICOMPTYPE_HIT_OBJECT, // AR_OBJECT_HIT_OBJECT,

    // AR_BASIC_MAXIMUM_COUNT
};

C_ASSERT(ARRAYSIZE(g_uBasicKindProps) == AR_BASIC_MAXIMUM_COUNT);

#define GetBasicKindProps(_Kind) g_uBasicKindProps[(_Kind)]

#define GET_BASIC_BITS(_Kind) GET_BPROP_BITS(GetBasicKindProps(_Kind))

#define GET_BASIC_PRIM_KIND(_Kind) GET_BPROP_PRIM_KIND(GetBasicKindProps(_Kind))
#define GET_BASIC_PRIM_KIND_SU(_Kind)                                          \
  GET_BPROP_PRIM_KIND_SU(GetBasicKindProps(_Kind))

#define IS_BASIC_PRIMITIVE(_Kind) IS_BPROP_PRIMITIVE(GetBasicKindProps(_Kind))

#define IS_BASIC_BOOL(_Kind) IS_BPROP_BOOL(GetBasicKindProps(_Kind))

#define IS_BASIC_FLOAT(_Kind) IS_BPROP_FLOAT(GetBasicKindProps(_Kind))

#define IS_BASIC_SINT(_Kind) IS_BPROP_SINT(GetBasicKindProps(_Kind))
#define IS_BASIC_UINT(_Kind) IS_BPROP_UINT(GetBasicKindProps(_Kind))
#define IS_BASIC_AINT(_Kind) IS_BPROP_AINT(GetBasicKindProps(_Kind))

#define IS_BASIC_STREAM(_Kind) IS_BPROP_STREAM(GetBasicKindProps(_Kind))

#define IS_BASIC_PATCH(_Kind) IS_BPROP_PATCH(GetBasicKindProps(_Kind))

#define IS_BASIC_SAMPLER(_Kind) IS_BPROP_SAMPLER(GetBasicKindProps(_Kind))
#define IS_BASIC_TEXTURE(_Kind) IS_BPROP_TEXTURE(GetBasicKindProps(_Kind))
#define IS_BASIC_OBJECT(_Kind) IS_BPROP_OBJECT(GetBasicKindProps(_Kind))

#define IS_BASIC_MIN_PRECISION(_Kind)                                          \
  IS_BPROP_MIN_PRECISION(GetBasicKindProps(_Kind))

#define IS_BASIC_UNSIGNABLE(_Kind) IS_BPROP_UNSIGNABLE(GetBasicKindProps(_Kind))

#define IS_BASIC_ENUM(_Kind) IS_BPROP_ENUM(GetBasicKindProps(_Kind))

#define BITWISE_ENUM_OPS(_Type)                                                \
  inline _Type operator|(_Type F1, _Type F2) {                                 \
    return (_Type)((UINT)F1 | (UINT)F2);                                       \
  }                                                                            \
  inline _Type operator&(_Type F1, _Type F2) {                                 \
    return (_Type)((UINT)F1 & (UINT)F2);                                       \
  }                                                                            \
  inline _Type &operator|=(_Type &F1, _Type F2) {                              \
    F1 = F1 | F2;                                                              \
    return F1;                                                                 \
  }                                                                            \
  inline _Type &operator&=(_Type &F1, _Type F2) {                              \
    F1 = F1 & F2;                                                              \
    return F1;                                                                 \
  }                                                                            \
  inline _Type &operator&=(_Type &F1, UINT F2) {                               \
    F1 = (_Type)((UINT)F1 & F2);                                               \
    return F1;                                                                 \
  }

enum ArTypeObjectKind {
  AR_TOBJ_INVALID, // Flag for an unassigned / unavailable object type.
  AR_TOBJ_VOID,  // Represents the type for functions with not returned valued.
  AR_TOBJ_BASIC, // Represents a primitive type.
  AR_TOBJ_COMPOUND,  // Represents a struct or class.
  AR_TOBJ_INTERFACE, // Represents an interface.
  AR_TOBJ_POINTER,   // Represents a pointer to another type.
  AR_TOBJ_OBJECT,    // Represents a built-in object.
  AR_TOBJ_ARRAY,     // Represents an array of other types.
  AR_TOBJ_MATRIX,    // Represents a matrix of basic types.
  AR_TOBJ_VECTOR,    // Represents a vector of basic types.
  AR_TOBJ_QUALIFIER, // Represents another type plus an ArTypeQualifier.
  AR_TOBJ_INNER_OBJ, // Represents a built-in inner object, such as an
                     // indexer object used to implement .mips[1].
  AR_TOBJ_STRING,    // Represents a string
  AR_TOBJ_DEPENDENT, // Dependent type for template.
};

enum TYPE_CONVERSION_FLAGS {
  TYPE_CONVERSION_DEFAULT =
      0x00000000, // Indicates an implicit conversion is done.
  TYPE_CONVERSION_EXPLICIT =
      0x00000001, // Indicates a conversion is done through an explicit cast.
  TYPE_CONVERSION_BY_REFERENCE =
      0x00000002, // Indicates a conversion is done to an output parameter.
};

enum TYPE_CONVERSION_REMARKS {
  TYPE_CONVERSION_NONE = 0x00000000,
  TYPE_CONVERSION_PRECISION_LOSS = 0x00000001,
  TYPE_CONVERSION_IDENTICAL = 0x00000002,
  TYPE_CONVERSION_TO_VOID = 0x00000004,
  TYPE_CONVERSION_ELT_TRUNCATION = 0x00000008,
};

BITWISE_ENUM_OPS(TYPE_CONVERSION_REMARKS)

#define AR_TOBJ_SCALAR AR_TOBJ_BASIC
#define AR_TOBJ_UNKNOWN AR_TOBJ_INVALID

#define AR_TPROP_VOID 0x0000000000000001
#define AR_TPROP_CONST 0x0000000000000002
#define AR_TPROP_IMP_CONST 0x0000000000000004
#define AR_TPROP_OBJECT 0x0000000000000008
#define AR_TPROP_SCALAR 0x0000000000000010
#define AR_TPROP_UNSIGNED 0x0000000000000020
#define AR_TPROP_NUMERIC 0x0000000000000040
#define AR_TPROP_INTEGRAL 0x0000000000000080
#define AR_TPROP_FLOATING 0x0000000000000100
#define AR_TPROP_LITERAL 0x0000000000000200
#define AR_TPROP_POINTER 0x0000000000000400
#define AR_TPROP_INPUT_PATCH 0x0000000000000800
#define AR_TPROP_OUTPUT_PATCH 0x0000000000001000
#define AR_TPROP_INH_IFACE 0x0000000000002000
#define AR_TPROP_HAS_COMPOUND 0x0000000000004000
#define AR_TPROP_HAS_TEXTURES 0x0000000000008000
#define AR_TPROP_HAS_SAMPLERS 0x0000000000010000
#define AR_TPROP_HAS_SAMPLER_CMPS 0x0000000000020000
#define AR_TPROP_HAS_STREAMS 0x0000000000040000
#define AR_TPROP_HAS_OTHER_OBJECTS 0x0000000000080000
#define AR_TPROP_HAS_BASIC 0x0000000000100000
#define AR_TPROP_HAS_BUFFERS 0x0000000000200000
#define AR_TPROP_HAS_ROBJECTS 0x0000000000400000
#define AR_TPROP_HAS_POINTERS 0x0000000000800000
#define AR_TPROP_INDEXABLE 0x0000000001000000
#define AR_TPROP_HAS_MIPS 0x0000000002000000
#define AR_TPROP_WRITABLE_GLOBAL 0x0000000004000000
#define AR_TPROP_HAS_UAVS 0x0000000008000000
#define AR_TPROP_HAS_BYTEADDRESS 0x0000000010000000
#define AR_TPROP_HAS_STRUCTURED 0x0000000020000000
#define AR_TPROP_HAS_SAMPLE 0x0000000040000000
#define AR_TPROP_MIN_PRECISION 0x0000000080000000
#define AR_TPROP_HAS_CBUFFERS 0x0000000100008000
#define AR_TPROP_HAS_TBUFFERS 0x0000000200008000

#define AR_TPROP_ALL 0xffffffffffffffff

#define AR_TPROP_HAS_OBJECTS                                                   \
  (AR_TPROP_HAS_TEXTURES | AR_TPROP_HAS_SAMPLERS | AR_TPROP_HAS_SAMPLER_CMPS | \
   AR_TPROP_HAS_STREAMS | AR_TPROP_HAS_OTHER_OBJECTS | AR_TPROP_HAS_BUFFERS |  \
   AR_TPROP_HAS_ROBJECTS | AR_TPROP_HAS_UAVS | AR_TPROP_HAS_BYTEADDRESS |      \
   AR_TPROP_HAS_STRUCTURED)

#define AR_TPROP_HAS_BASIC_RESOURCES                                           \
  (AR_TPROP_HAS_TEXTURES | AR_TPROP_HAS_SAMPLERS | AR_TPROP_HAS_SAMPLER_CMPS | \
   AR_TPROP_HAS_BUFFERS | AR_TPROP_HAS_UAVS)

#define AR_TPROP_UNION_BITS                                                    \
  (AR_TPROP_INH_IFACE | AR_TPROP_HAS_COMPOUND | AR_TPROP_HAS_TEXTURES |        \
   AR_TPROP_HAS_SAMPLERS | AR_TPROP_HAS_SAMPLER_CMPS | AR_TPROP_HAS_STREAMS |  \
   AR_TPROP_HAS_OTHER_OBJECTS | AR_TPROP_HAS_BASIC | AR_TPROP_HAS_BUFFERS |    \
   AR_TPROP_HAS_ROBJECTS | AR_TPROP_HAS_POINTERS | AR_TPROP_WRITABLE_GLOBAL |  \
   AR_TPROP_HAS_UAVS | AR_TPROP_HAS_BYTEADDRESS | AR_TPROP_HAS_STRUCTURED |    \
   AR_TPROP_MIN_PRECISION)

#define AR_TINFO_ALLOW_COMPLEX 0x00000001
#define AR_TINFO_ALLOW_OBJECTS 0x00000002
#define AR_TINFO_IGNORE_QUALIFIERS 0x00000004
#define AR_TINFO_OBJECTS_AS_ELEMENTS 0x00000008
#define AR_TINFO_PACK_SCALAR 0x00000010
#define AR_TINFO_PACK_ROW_MAJOR 0x00000020
#define AR_TINFO_PACK_TEMP_ARRAY 0x00000040
#define AR_TINFO_ALL_VAR_INFO 0x00000080

#define AR_TINFO_ALLOW_ALL (AR_TINFO_ALLOW_COMPLEX | AR_TINFO_ALLOW_OBJECTS)

#define AR_TINFO_PACK_CBUFFER 0
#define AR_TINFO_LAYOUT_PACK_ALL                                               \
  (AR_TINFO_PACK_SCALAR | AR_TINFO_PACK_TEMP_ARRAY)

#define AR_TINFO_SIMPLE_OBJECTS                                                \
  (AR_TINFO_ALLOW_OBJECTS | AR_TINFO_OBJECTS_AS_ELEMENTS)

struct ArTypeInfo {
  ArTypeObjectKind ShapeKind; // The shape of the type (basic, matrix, etc.)
  ArBasicKind EltKind;        // The primitive type of elements in this type.
  const clang::Type *EltTy;   // Canonical element type ptr
  ArBasicKind
      ObjKind; // The object type for this type (textures, buffers, etc.)
  UINT uRows;
  UINT uCols;
  UINT uTotalElts;
};

using namespace clang;
using namespace clang::sema;
using namespace hlsl;

extern const char *HLSLScalarTypeNames[];

static const bool ExplicitConversionFalse =
    false; // a conversion operation is not the result of an explicit cast
static const bool ParameterPackFalse =
    false; // template parameter is not an ellipsis.
static const bool TypenameTrue =
    false; // 'typename' specified rather than 'class' for a template argument.
static const bool DelayTypeCreationTrue =
    true;                          // delay type creation for a declaration
static const SourceLocation NoLoc; // no source location attribution available
static const SourceRange NoRange;  // no source range attribution available
static const bool HasWrittenPrototypeTrue =
    true; // function had the prototype written
static const bool InlineSpecifiedFalse =
    false; // function was not specified as inline
static const bool IsConstexprFalse = false; // function is not constexpr
static const bool ListInitializationFalse =
    false; // not performing a list initialization
static const bool SuppressWarningsFalse =
    false; // do not suppress warning diagnostics
static const bool SuppressErrorsTrue = true; // suppress error diagnostics
static const bool SuppressErrorsFalse =
    false;                           // do not suppress error diagnostics
static const int OneRow = 1;         // a single row for a type
static const bool MipsFalse = false; // a type does not support the .mips member
static const bool MipsTrue = true;   // a type supports the .mips member
static const bool SampleFalse =
    false; // a type does not support the .sample member
static const bool SampleTrue = true;   // a type supports the .sample member
static const size_t MaxVectorSize = 4; // maximum size for a vector

static QualType
GetOrCreateTemplateSpecialization(ASTContext &context, Sema &sema,
                                  ClassTemplateDecl *templateDecl,
                                  ArrayRef<TemplateArgument> templateArgs) {
  DXASSERT_NOMSG(templateDecl);
  DeclContext *currentDeclContext = context.getTranslationUnitDecl();
  SmallVector<TemplateArgument, 3> templateArgsForDecl;
  for (const TemplateArgument &Arg : templateArgs) {
    if (Arg.getKind() == TemplateArgument::Type) {
      // the class template need to use CanonicalType
      templateArgsForDecl.emplace_back(
          TemplateArgument(Arg.getAsType().getCanonicalType()));
    } else
      templateArgsForDecl.emplace_back(Arg);
  }
  // First, try looking up existing specialization
  void *InsertPos = nullptr;
  ClassTemplateSpecializationDecl *specializationDecl =
      templateDecl->findSpecialization(templateArgsForDecl, InsertPos);
  if (specializationDecl) {
    // Instantiate the class template if not yet.
    if (specializationDecl->getInstantiatedFrom().isNull()) {
      // InstantiateClassTemplateSpecialization returns true if it finds an
      // error.
      DXVERIFY_NOMSG(false ==
                     sema.InstantiateClassTemplateSpecialization(
                         NoLoc, specializationDecl,
                         TemplateSpecializationKind::TSK_ImplicitInstantiation,
                         true));
    }
    return context.getTemplateSpecializationType(
        TemplateName(templateDecl), templateArgs.data(), templateArgs.size(),
        context.getTypeDeclType(specializationDecl));
  }

  specializationDecl = ClassTemplateSpecializationDecl::Create(
      context, TagDecl::TagKind::TTK_Class, currentDeclContext, NoLoc, NoLoc,
      templateDecl, templateArgsForDecl.data(), templateArgsForDecl.size(),
      nullptr);
  // InstantiateClassTemplateSpecialization returns true if it finds an error.
  DXVERIFY_NOMSG(false ==
                 sema.InstantiateClassTemplateSpecialization(
                     NoLoc, specializationDecl,
                     TemplateSpecializationKind::TSK_ImplicitInstantiation,
                     true));
  templateDecl->AddSpecialization(specializationDecl, InsertPos);
  specializationDecl->setImplicit(true);

  QualType canonType = context.getTypeDeclType(specializationDecl);
  DXASSERT(isa<RecordType>(canonType),
           "type of non-dependent specialization is not a RecordType");
  TemplateArgumentListInfo templateArgumentList(NoLoc, NoLoc);
  TemplateArgumentLocInfo NoTemplateArgumentLocInfo;
  for (unsigned i = 0; i < templateArgs.size(); i++) {
    templateArgumentList.addArgument(
        TemplateArgumentLoc(templateArgs[i], NoTemplateArgumentLocInfo));
  }
  return context.getTemplateSpecializationType(TemplateName(templateDecl),
                                               templateArgumentList, canonType);
}

/// <summary>Instantiates a new matrix type specialization or gets an existing
/// one from the AST.</summary>
static QualType GetOrCreateMatrixSpecialization(
    ASTContext &context, Sema *sema, ClassTemplateDecl *matrixTemplateDecl,
    QualType elementType, uint64_t rowCount, uint64_t colCount) {
  DXASSERT_NOMSG(sema);

  TemplateArgument templateArgs[3] = {
      TemplateArgument(elementType),
      TemplateArgument(
          context,
          llvm::APSInt(
              llvm::APInt(context.getIntWidth(context.IntTy), rowCount), false),
          context.IntTy),
      TemplateArgument(
          context,
          llvm::APSInt(
              llvm::APInt(context.getIntWidth(context.IntTy), colCount), false),
          context.IntTy)};

  QualType matrixSpecializationType = GetOrCreateTemplateSpecialization(
      context, *sema, matrixTemplateDecl,
      ArrayRef<TemplateArgument>(templateArgs));

#ifndef NDEBUG
  // Verify that we can read the field member from the template record.
  DXASSERT(matrixSpecializationType->getAsCXXRecordDecl(),
           "type of non-dependent specialization is not a RecordType");
  DeclContext::lookup_result lookupResult =
      matrixSpecializationType->getAsCXXRecordDecl()->lookup(
          DeclarationName(&context.Idents.get(StringRef("h"))));
  DXASSERT(!lookupResult.empty(),
           "otherwise matrix handle cannot be looked up");
#endif

  return matrixSpecializationType;
}

/// <summary>Instantiates a new vector type specialization or gets an existing
/// one from the AST.</summary>
static QualType
GetOrCreateVectorSpecialization(ASTContext &context, Sema *sema,
                                ClassTemplateDecl *vectorTemplateDecl,
                                QualType elementType, uint64_t colCount) {
  DXASSERT_NOMSG(sema);
  DXASSERT_NOMSG(vectorTemplateDecl);

  TemplateArgument templateArgs[2] = {
      TemplateArgument(elementType),
      TemplateArgument(
          context,
          llvm::APSInt(
              llvm::APInt(context.getIntWidth(context.IntTy), colCount), false),
          context.IntTy)};

  QualType vectorSpecializationType = GetOrCreateTemplateSpecialization(
      context, *sema, vectorTemplateDecl,
      ArrayRef<TemplateArgument>(templateArgs));

#ifndef NDEBUG
  // Verify that we can read the field member from the template record.
  DXASSERT(vectorSpecializationType->getAsCXXRecordDecl(),
           "type of non-dependent specialization is not a RecordType");
  DeclContext::lookup_result lookupResult =
      vectorSpecializationType->getAsCXXRecordDecl()->lookup(
          DeclarationName(&context.Idents.get(StringRef("h"))));
  DXASSERT(!lookupResult.empty(),
           "otherwise vector handle cannot be looked up");
#endif

  return vectorSpecializationType;
}

/// <summary>Instantiates a new *NodeOutputRecords type specialization or gets
/// an existing one from the AST.</summary>
static QualType
GetOrCreateNodeOutputRecordSpecialization(ASTContext &context, Sema *sema,
                                          _In_ ClassTemplateDecl *templateDecl,
                                          QualType elementType) {
  DXASSERT_NOMSG(sema);
  DXASSERT_NOMSG(templateDecl);

  TemplateArgument templateArgs[1] = {TemplateArgument(elementType)};

  QualType specializationType = GetOrCreateTemplateSpecialization(
      context, *sema, templateDecl, ArrayRef<TemplateArgument>(templateArgs));

#ifdef DBG
  // Verify that we can read the field member from the template record.
  DXASSERT(specializationType->getAsCXXRecordDecl(),
           "type of non-dependent specialization is not a RecordType");
  DeclContext::lookup_result lookupResult =
      specializationType->getAsCXXRecordDecl()->lookup(
          DeclarationName(&context.Idents.get(StringRef("h"))));
  DXASSERT(!lookupResult.empty(),
           "otherwise *NodeOutputRecords handle cannot be looked up");
#endif

  return specializationType;
}

// Decls.cpp constants start here - these should be refactored or, better,
// replaced with clang::Type-based constructs.

static const LPCSTR kBuiltinIntrinsicTableName = "op";

static const ArTypeObjectKind g_ScalarTT[] = {AR_TOBJ_SCALAR, AR_TOBJ_UNKNOWN};

static const ArTypeObjectKind g_VectorTT[] = {AR_TOBJ_VECTOR, AR_TOBJ_UNKNOWN};

static const ArTypeObjectKind g_MatrixTT[] = {AR_TOBJ_MATRIX, AR_TOBJ_UNKNOWN};

static const ArTypeObjectKind g_AnyTT[] = {AR_TOBJ_SCALAR, AR_TOBJ_VECTOR,
                                           AR_TOBJ_MATRIX, AR_TOBJ_UNKNOWN};

static const ArTypeObjectKind g_ObjectTT[] = {AR_TOBJ_OBJECT, AR_TOBJ_STRING,
                                              AR_TOBJ_UNKNOWN};

static const ArTypeObjectKind g_NullTT[] = {AR_TOBJ_VOID, AR_TOBJ_UNKNOWN};

static const ArTypeObjectKind g_ArrayTT[] = {AR_TOBJ_ARRAY, AR_TOBJ_UNKNOWN};

const ArTypeObjectKind *g_LegalIntrinsicTemplates[] = {
    g_NullTT, g_ScalarTT, g_VectorTT, g_MatrixTT,
    g_AnyTT,  g_ObjectTT, g_ArrayTT,
};
C_ASSERT(ARRAYSIZE(g_LegalIntrinsicTemplates) == LITEMPLATE_COUNT);

//
// The first one is used to name the representative group, so make
// sure its name will make sense in error messages.
//

static const ArBasicKind g_BoolCT[] = {AR_BASIC_BOOL, AR_BASIC_UNKNOWN};

static const ArBasicKind g_IntCT[] = {AR_BASIC_INT32, AR_BASIC_LITERAL_INT,
                                      AR_BASIC_UNKNOWN};

static const ArBasicKind g_UIntCT[] = {AR_BASIC_UINT32, AR_BASIC_LITERAL_INT,
                                       AR_BASIC_UNKNOWN};

// We use the first element for default if matching kind is missing in the list.
// AR_BASIC_INT32 should be the default for any int since min precision integers
// should map to int32, not int16 or int64
static const ArBasicKind g_AnyIntCT[] = {
    AR_BASIC_INT32, AR_BASIC_INT16,  AR_BASIC_UINT32,      AR_BASIC_UINT16,
    AR_BASIC_INT64, AR_BASIC_UINT64, AR_BASIC_LITERAL_INT, AR_BASIC_UNKNOWN};

static const ArBasicKind g_AnyInt32CT[] = {
    AR_BASIC_INT32, AR_BASIC_UINT32, AR_BASIC_LITERAL_INT, AR_BASIC_UNKNOWN};

static const ArBasicKind g_UIntOnlyCT[] = {AR_BASIC_UINT32, AR_BASIC_UINT64,
                                           AR_BASIC_LITERAL_INT,
                                           AR_BASIC_NOCAST, AR_BASIC_UNKNOWN};

static const ArBasicKind g_FloatCT[] = {
    AR_BASIC_FLOAT32, AR_BASIC_FLOAT32_PARTIAL_PRECISION,
    AR_BASIC_LITERAL_FLOAT, AR_BASIC_UNKNOWN};

static const ArBasicKind g_AnyFloatCT[] = {
    AR_BASIC_FLOAT32,       AR_BASIC_FLOAT32_PARTIAL_PRECISION,
    AR_BASIC_FLOAT16,       AR_BASIC_FLOAT64,
    AR_BASIC_LITERAL_FLOAT, AR_BASIC_MIN10FLOAT,
    AR_BASIC_MIN16FLOAT,    AR_BASIC_UNKNOWN};

static const ArBasicKind g_FloatLikeCT[] = {
    AR_BASIC_FLOAT32,    AR_BASIC_FLOAT32_PARTIAL_PRECISION,
    AR_BASIC_FLOAT16,    AR_BASIC_LITERAL_FLOAT,
    AR_BASIC_MIN10FLOAT, AR_BASIC_MIN16FLOAT,
    AR_BASIC_UNKNOWN};

static const ArBasicKind g_FloatDoubleCT[] = {
    AR_BASIC_FLOAT32, AR_BASIC_FLOAT32_PARTIAL_PRECISION, AR_BASIC_FLOAT64,
    AR_BASIC_LITERAL_FLOAT, AR_BASIC_UNKNOWN};

static const ArBasicKind g_DoubleCT[] = {
    AR_BASIC_FLOAT64, AR_BASIC_LITERAL_FLOAT, AR_BASIC_UNKNOWN};

static const ArBasicKind g_DoubleOnlyCT[] = {AR_BASIC_FLOAT64,
                                             AR_BASIC_LITERAL_FLOAT,
                                             AR_BASIC_NOCAST, AR_BASIC_UNKNOWN};

static const ArBasicKind g_NumericCT[] = {
    AR_BASIC_FLOAT32,       AR_BASIC_FLOAT32_PARTIAL_PRECISION,
    AR_BASIC_FLOAT16,       AR_BASIC_FLOAT64,
    AR_BASIC_LITERAL_FLOAT, AR_BASIC_MIN10FLOAT,
    AR_BASIC_MIN16FLOAT,    AR_BASIC_LITERAL_INT,
    AR_BASIC_INT16,         AR_BASIC_INT32,
    AR_BASIC_UINT16,        AR_BASIC_UINT32,
    AR_BASIC_MIN12INT,      AR_BASIC_MIN16INT,
    AR_BASIC_MIN16UINT,     AR_BASIC_INT64,
    AR_BASIC_UINT64,        AR_BASIC_UNKNOWN};

static const ArBasicKind g_Numeric32CT[] = {
    AR_BASIC_FLOAT32,       AR_BASIC_FLOAT32_PARTIAL_PRECISION,
    AR_BASIC_LITERAL_FLOAT, AR_BASIC_LITERAL_INT,
    AR_BASIC_INT32,         AR_BASIC_UINT32,
    AR_BASIC_UNKNOWN};

static const ArBasicKind g_Numeric32OnlyCT[] = {
    AR_BASIC_FLOAT32,       AR_BASIC_FLOAT32_PARTIAL_PRECISION,
    AR_BASIC_LITERAL_FLOAT, AR_BASIC_LITERAL_INT,
    AR_BASIC_INT32,         AR_BASIC_UINT32,
    AR_BASIC_NOCAST,        AR_BASIC_UNKNOWN};

static const ArBasicKind g_AnyCT[] = {
    AR_BASIC_FLOAT32,       AR_BASIC_FLOAT32_PARTIAL_PRECISION,
    AR_BASIC_FLOAT16,       AR_BASIC_FLOAT64,
    AR_BASIC_LITERAL_FLOAT, AR_BASIC_MIN10FLOAT,
    AR_BASIC_MIN16FLOAT,    AR_BASIC_INT16,
    AR_BASIC_UINT16,        AR_BASIC_LITERAL_INT,
    AR_BASIC_INT32,         AR_BASIC_UINT32,
    AR_BASIC_MIN12INT,      AR_BASIC_MIN16INT,
    AR_BASIC_MIN16UINT,     AR_BASIC_BOOL,
    AR_BASIC_INT64,         AR_BASIC_UINT64,
    AR_BASIC_UNKNOWN};

static const ArBasicKind g_AnySamplerCT[] = {
    AR_OBJECT_SAMPLER, AR_OBJECT_SAMPLERCOMPARISON, AR_BASIC_UNKNOWN};

static const ArBasicKind g_Sampler1DCT[] = {AR_OBJECT_SAMPLER1D,
                                            AR_BASIC_UNKNOWN};

static const ArBasicKind g_Sampler2DCT[] = {AR_OBJECT_SAMPLER2D,
                                            AR_BASIC_UNKNOWN};

static const ArBasicKind g_Sampler3DCT[] = {AR_OBJECT_SAMPLER3D,
                                            AR_BASIC_UNKNOWN};

static const ArBasicKind g_SamplerCUBECT[] = {AR_OBJECT_SAMPLERCUBE,
                                              AR_BASIC_UNKNOWN};

static const ArBasicKind g_SamplerCmpCT[] = {AR_OBJECT_SAMPLERCOMPARISON,
                                             AR_BASIC_UNKNOWN};

static const ArBasicKind g_SamplerCT[] = {AR_OBJECT_SAMPLER, AR_BASIC_UNKNOWN};

static const ArBasicKind g_Texture2DCT[] = {AR_OBJECT_TEXTURE2D,
                                            AR_BASIC_UNKNOWN};

static const ArBasicKind g_Texture2DArrayCT[] = {AR_OBJECT_TEXTURE2D_ARRAY,
                                                 AR_BASIC_UNKNOWN};

static const ArBasicKind g_ResourceCT[] = {AR_OBJECT_HEAP_RESOURCE,
                                           AR_BASIC_UNKNOWN};

static const ArBasicKind g_RayDescCT[] = {AR_OBJECT_RAY_DESC, AR_BASIC_UNKNOWN};

static const ArBasicKind g_RayQueryCT[] = {AR_OBJECT_RAY_QUERY,
                                           AR_BASIC_UNKNOWN};

static const ArBasicKind g_LinAlgCT[] = {
    AR_BASIC_FLOAT32,       AR_BASIC_FLOAT32_PARTIAL_PRECISION,
    AR_BASIC_FLOAT16,       AR_BASIC_INT32,
    AR_BASIC_INT16,         AR_BASIC_UINT32,
    AR_BASIC_UINT16,        AR_BASIC_INT8_4PACKED,
    AR_BASIC_UINT8_4PACKED, AR_BASIC_NOCAST,
    AR_BASIC_UNKNOWN};

static const ArBasicKind g_AccelerationStructCT[] = {
    AR_OBJECT_ACCELERATION_STRUCT, AR_BASIC_UNKNOWN};

static const ArBasicKind g_UDTCT[] = {AR_OBJECT_USER_DEFINED_TYPE,
                                      AR_BASIC_UNKNOWN};

static const ArBasicKind g_StringCT[] = {AR_OBJECT_STRING_LITERAL,
                                         AR_OBJECT_STRING, AR_BASIC_UNKNOWN};

static const ArBasicKind g_NullCT[] = {AR_OBJECT_NULL, AR_BASIC_UNKNOWN};

static const ArBasicKind g_WaveCT[] = {AR_OBJECT_WAVE, AR_BASIC_UNKNOWN};

static const ArBasicKind g_UInt64CT[] = {AR_BASIC_UINT64, AR_BASIC_UNKNOWN};

static const ArBasicKind g_Float16CT[] = {
    AR_BASIC_FLOAT16, AR_BASIC_LITERAL_FLOAT, AR_BASIC_UNKNOWN};

static const ArBasicKind g_Int16CT[] = {AR_BASIC_INT16, AR_BASIC_LITERAL_INT,
                                        AR_BASIC_UNKNOWN};

static const ArBasicKind g_UInt16CT[] = {AR_BASIC_UINT16, AR_BASIC_LITERAL_INT,
                                         AR_BASIC_UNKNOWN};

static const ArBasicKind g_Numeric16OnlyCT[] = {
    AR_BASIC_FLOAT16,       AR_BASIC_INT16,       AR_BASIC_UINT16,
    AR_BASIC_LITERAL_FLOAT, AR_BASIC_LITERAL_INT, AR_BASIC_NOCAST,
    AR_BASIC_UNKNOWN};

static const ArBasicKind g_Int32OnlyCT[] = {AR_BASIC_INT32, AR_BASIC_UINT32,
                                            AR_BASIC_LITERAL_INT,
                                            AR_BASIC_NOCAST, AR_BASIC_UNKNOWN};

static const ArBasicKind g_Float32OnlyCT[] = {
    AR_BASIC_FLOAT32, AR_BASIC_LITERAL_FLOAT, AR_BASIC_NOCAST,
    AR_BASIC_UNKNOWN};

static const ArBasicKind g_Int64OnlyCT[] = {AR_BASIC_UINT64, AR_BASIC_INT64,
                                            AR_BASIC_LITERAL_INT,
                                            AR_BASIC_NOCAST, AR_BASIC_UNKNOWN};

static const ArBasicKind g_AnyInt64CT[] = {
    AR_BASIC_INT64, AR_BASIC_UINT64, AR_BASIC_LITERAL_INT, AR_BASIC_UNKNOWN};

static const ArBasicKind g_Int8_4PackedCT[] = {
    AR_BASIC_INT8_4PACKED, AR_BASIC_UINT32, AR_BASIC_LITERAL_INT,
    AR_BASIC_UNKNOWN};

static const ArBasicKind g_UInt8_4PackedCT[] = {
    AR_BASIC_UINT8_4PACKED, AR_BASIC_UINT32, AR_BASIC_LITERAL_INT,
    AR_BASIC_UNKNOWN};

static const ArBasicKind g_AnyInt16Or32CT[] = {
    AR_BASIC_INT32,  AR_BASIC_UINT32,      AR_BASIC_INT16,
    AR_BASIC_UINT16, AR_BASIC_LITERAL_INT, AR_BASIC_UNKNOWN};

static const ArBasicKind g_SInt16Or32OnlyCT[] = {
    AR_BASIC_INT32, AR_BASIC_INT16, AR_BASIC_LITERAL_INT, AR_BASIC_NOCAST,
    AR_BASIC_UNKNOWN};

static const ArBasicKind g_ByteAddressBufferCT[] = {
    AR_OBJECT_BYTEADDRESS_BUFFER, AR_BASIC_UNKNOWN};

static const ArBasicKind g_RWByteAddressBufferCT[] = {
    AR_OBJECT_RWBYTEADDRESS_BUFFER, AR_BASIC_UNKNOWN};

static const ArBasicKind g_NodeRecordOrUAVCT[] = {
    AR_OBJECT_DISPATCH_NODE_INPUT_RECORD,
    AR_OBJECT_RWDISPATCH_NODE_INPUT_RECORD,
    AR_OBJECT_GROUP_NODE_INPUT_RECORDS,
    AR_OBJECT_RWGROUP_NODE_INPUT_RECORDS,
    AR_OBJECT_THREAD_NODE_INPUT_RECORD,
    AR_OBJECT_RWTHREAD_NODE_INPUT_RECORD,
    AR_OBJECT_NODE_OUTPUT,
    AR_OBJECT_THREAD_NODE_OUTPUT_RECORDS,
    AR_OBJECT_GROUP_NODE_OUTPUT_RECORDS,

    AR_OBJECT_RWBUFFER,
    AR_OBJECT_RWTEXTURE1D,
    AR_OBJECT_RWTEXTURE1D_ARRAY,
    AR_OBJECT_RWTEXTURE2D,
    AR_OBJECT_RWTEXTURE2D_ARRAY,
    AR_OBJECT_RWTEXTURE3D,
    AR_OBJECT_RWSTRUCTURED_BUFFER,
    AR_OBJECT_RWBYTEADDRESS_BUFFER,
    AR_OBJECT_APPEND_STRUCTURED_BUFFER,
    AR_BASIC_UNKNOWN};

static const ArBasicKind g_GroupNodeOutputRecordsCT[] = {
    AR_OBJECT_GROUP_NODE_OUTPUT_RECORDS, AR_BASIC_UNKNOWN};

static const ArBasicKind g_ThreadNodeOutputRecordsCT[] = {
    AR_OBJECT_THREAD_NODE_OUTPUT_RECORDS, AR_BASIC_UNKNOWN};

static const ArBasicKind g_AnyOutputRecordCT[] = {
    AR_OBJECT_GROUP_NODE_OUTPUT_RECORDS, AR_OBJECT_THREAD_NODE_OUTPUT_RECORDS,
    AR_BASIC_UNKNOWN};

// Shader Execution Reordering
static const ArBasicKind g_DxHitObjectCT[] = {AR_OBJECT_HIT_OBJECT,
                                              AR_BASIC_UNKNOWN};

#ifdef ENABLE_SPIRV_CODEGEN
static const ArBasicKind g_VKBufferPointerCT[] = {AR_OBJECT_VK_BUFFER_POINTER,
                                                  AR_BASIC_UNKNOWN};
#endif

// Basic kinds, indexed by a LEGAL_INTRINSIC_COMPTYPES value.
const ArBasicKind *g_LegalIntrinsicCompTypes[] = {
    g_NullCT,               // LICOMPTYPE_VOID
    g_BoolCT,               // LICOMPTYPE_BOOL
    g_IntCT,                // LICOMPTYPE_INT
    g_UIntCT,               // LICOMPTYPE_UINT
    g_AnyIntCT,             // LICOMPTYPE_ANY_INT
    g_AnyInt32CT,           // LICOMPTYPE_ANY_INT32
    g_UIntOnlyCT,           // LICOMPTYPE_UINT_ONLY
    g_FloatCT,              // LICOMPTYPE_FLOAT
    g_AnyFloatCT,           // LICOMPTYPE_ANY_FLOAT
    g_FloatLikeCT,          // LICOMPTYPE_FLOAT_LIKE
    g_FloatDoubleCT,        // LICOMPTYPE_FLOAT_DOUBLE
    g_DoubleCT,             // LICOMPTYPE_DOUBLE
    g_DoubleOnlyCT,         // LICOMPTYPE_DOUBLE_ONLY
    g_NumericCT,            // LICOMPTYPE_NUMERIC
    g_Numeric32CT,          // LICOMPTYPE_NUMERIC32
    g_Numeric32OnlyCT,      // LICOMPTYPE_NUMERIC32_ONLY
    g_AnyCT,                // LICOMPTYPE_ANY
    g_Sampler1DCT,          // LICOMPTYPE_SAMPLER1D
    g_Sampler2DCT,          // LICOMPTYPE_SAMPLER2D
    g_Sampler3DCT,          // LICOMPTYPE_SAMPLER3D
    g_SamplerCUBECT,        // LICOMPTYPE_SAMPLERCUBE
    g_SamplerCmpCT,         // LICOMPTYPE_SAMPLERCMP
    g_SamplerCT,            // LICOMPTYPE_SAMPLER
    g_StringCT,             // LICOMPTYPE_STRING
    g_WaveCT,               // LICOMPTYPE_WAVE
    g_UInt64CT,             // LICOMPTYPE_UINT64
    g_Float16CT,            // LICOMPTYPE_FLOAT16
    g_Int16CT,              // LICOMPTYPE_INT16
    g_UInt16CT,             // LICOMPTYPE_UINT16
    g_Numeric16OnlyCT,      // LICOMPTYPE_NUMERIC16_ONLY
    g_RayDescCT,            // LICOMPTYPE_RAYDESC
    g_AccelerationStructCT, // LICOMPTYPE_ACCELERATION_STRUCT,
    g_UDTCT,                // LICOMPTYPE_USER_DEFINED_TYPE
    g_Texture2DCT,          // LICOMPTYPE_TEXTURE2D
    g_Texture2DArrayCT,     // LICOMPTYPE_TEXTURE2DARRAY
    g_ResourceCT,           // LICOMPTYPE_RESOURCE
    g_Int32OnlyCT,          // LICOMPTYPE_INT32_ONLY
    g_Int64OnlyCT,          // LICOMPTYPE_INT64_ONLY
    g_AnyInt64CT,           // LICOMPTYPE_ANY_INT64
    g_Float32OnlyCT,        // LICOMPTYPE_FLOAT32_ONLY
    g_Int8_4PackedCT,       // LICOMPTYPE_INT8_4PACKED
    g_UInt8_4PackedCT,      // LICOMPTYPE_UINT8_4PACKED
    g_AnyInt16Or32CT,       // LICOMPTYPE_ANY_INT16_OR_32
    g_SInt16Or32OnlyCT,     // LICOMPTYPE_SINT16_OR_32_ONLY
    g_AnySamplerCT,         // LICOMPTYPE_ANY_SAMPLER

    g_ByteAddressBufferCT,       // LICOMPTYPE_BYTEADDRESSBUFFER
    g_RWByteAddressBufferCT,     // LICOMPTYPE_RWBYTEADDRESSBUFFER
    g_NodeRecordOrUAVCT,         // LICOMPTYPE_NODE_RECORD_OR_UAV
    g_AnyOutputRecordCT,         // LICOMPTYPE_ANY_NODE_OUTPUT_RECORD
    g_GroupNodeOutputRecordsCT,  // LICOMPTYPE_GROUP_NODE_OUTPUT_RECORDS
    g_ThreadNodeOutputRecordsCT, // LICOMPTYPE_THREAD_NODE_OUTPUT_RECORDS
    g_DxHitObjectCT,             // LICOMPTYPE_HIT_OBJECT
    g_RayQueryCT,                // LICOMPTYPE_RAY_QUERY
    g_LinAlgCT,                  // LICOMPTYPE_LINALG
#ifdef ENABLE_SPIRV_CODEGEN
    g_VKBufferPointerCT, // LICOMPTYPE_VK_BUFFER_POINTER
#endif
};
static_assert(
    ARRAYSIZE(g_LegalIntrinsicCompTypes) == LICOMPTYPE_COUNT,
    "Intrinsic comp type table must be updated when new enumerants are added.");

// Decls.cpp constants ends here - these should be refactored or, better,
// replaced with clang::Type-based constructs.

// Basic kind objects that are represented as HLSL structures or templates.
static const ArBasicKind g_ArBasicKindsAsTypes[] = {
    AR_OBJECT_BUFFER, // Buffer

    // AR_OBJECT_TEXTURE,
    AR_OBJECT_TEXTURE1D,         // Texture1D
    AR_OBJECT_TEXTURE1D_ARRAY,   // Texture1DArray
    AR_OBJECT_TEXTURE2D,         // Texture2D
    AR_OBJECT_TEXTURE2D_ARRAY,   // Texture2DArray
    AR_OBJECT_TEXTURE3D,         // Texture3D
    AR_OBJECT_TEXTURECUBE,       // TextureCube
    AR_OBJECT_TEXTURECUBE_ARRAY, // TextureCubeArray
    AR_OBJECT_TEXTURE2DMS,       // Texture2DMS
    AR_OBJECT_TEXTURE2DMS_ARRAY, // Texture2DMSArray

    AR_OBJECT_SAMPLER,
    // AR_OBJECT_SAMPLER1D,
    // AR_OBJECT_SAMPLER2D,
    // AR_OBJECT_SAMPLER3D,
    // AR_OBJECT_SAMPLERCUBE,
    AR_OBJECT_SAMPLERCOMPARISON,

    AR_OBJECT_CONSTANT_BUFFER, AR_OBJECT_TEXTURE_BUFFER,

    AR_OBJECT_POINTSTREAM, AR_OBJECT_LINESTREAM, AR_OBJECT_TRIANGLESTREAM,

    AR_OBJECT_INPUTPATCH, AR_OBJECT_OUTPUTPATCH,

    AR_OBJECT_RWTEXTURE1D, AR_OBJECT_RWTEXTURE1D_ARRAY, AR_OBJECT_RWTEXTURE2D,
    AR_OBJECT_RWTEXTURE2D_ARRAY, AR_OBJECT_RWTEXTURE3D, AR_OBJECT_RWBUFFER,

    AR_OBJECT_BYTEADDRESS_BUFFER, AR_OBJECT_RWBYTEADDRESS_BUFFER,
    AR_OBJECT_STRUCTURED_BUFFER, AR_OBJECT_RWSTRUCTURED_BUFFER,
    // AR_OBJECT_RWSTRUCTURED_BUFFER_ALLOC,
    // AR_OBJECT_RWSTRUCTURED_BUFFER_CONSUME,
    AR_OBJECT_APPEND_STRUCTURED_BUFFER, AR_OBJECT_CONSUME_STRUCTURED_BUFFER,

    AR_OBJECT_ROVBUFFER, AR_OBJECT_ROVBYTEADDRESS_BUFFER,
    AR_OBJECT_ROVSTRUCTURED_BUFFER, AR_OBJECT_ROVTEXTURE1D,
    AR_OBJECT_ROVTEXTURE1D_ARRAY, AR_OBJECT_ROVTEXTURE2D,
    AR_OBJECT_ROVTEXTURE2D_ARRAY, AR_OBJECT_ROVTEXTURE3D,

    AR_OBJECT_FEEDBACKTEXTURE2D, AR_OBJECT_FEEDBACKTEXTURE2D_ARRAY,

// SPIRV change starts
#ifdef ENABLE_SPIRV_CODEGEN
    AR_OBJECT_VK_SUBPASS_INPUT, AR_OBJECT_VK_SUBPASS_INPUT_MS,
    AR_OBJECT_VK_SPIRV_TYPE, AR_OBJECT_VK_SPIRV_OPAQUE_TYPE,
    AR_OBJECT_VK_INTEGRAL_CONSTANT, AR_OBJECT_VK_LITERAL,
    AR_OBJECT_VK_SPV_INTRINSIC_TYPE, AR_OBJECT_VK_SPV_INTRINSIC_RESULT_ID,
    AR_OBJECT_VK_BUFFER_POINTER,
#endif // ENABLE_SPIRV_CODEGEN
    // SPIRV change ends

    AR_OBJECT_LEGACY_EFFECT, // Used for all unsupported but ignored legacy
                             // effect types

    AR_OBJECT_WAVE, AR_OBJECT_RAY_DESC, AR_OBJECT_ACCELERATION_STRUCT,
    AR_OBJECT_TRIANGLE_INTERSECTION_ATTRIBUTES,

    // subobjects
    AR_OBJECT_STATE_OBJECT_CONFIG, AR_OBJECT_GLOBAL_ROOT_SIGNATURE,
    AR_OBJECT_LOCAL_ROOT_SIGNATURE, AR_OBJECT_SUBOBJECT_TO_EXPORTS_ASSOC,
    AR_OBJECT_RAYTRACING_SHADER_CONFIG, AR_OBJECT_RAYTRACING_PIPELINE_CONFIG,
    AR_OBJECT_TRIANGLE_HIT_GROUP, AR_OBJECT_PROCEDURAL_PRIMITIVE_HIT_GROUP,
    AR_OBJECT_RAYTRACING_PIPELINE_CONFIG1,

    AR_OBJECT_RAY_QUERY, AR_OBJECT_HEAP_RESOURCE, AR_OBJECT_HEAP_SAMPLER,

    AR_OBJECT_RWTEXTURE2DMS,       // RWTexture2DMS
    AR_OBJECT_RWTEXTURE2DMS_ARRAY, // RWTexture2DMSArray

    // Work Graphs
    AR_OBJECT_EMPTY_NODE_INPUT, AR_OBJECT_DISPATCH_NODE_INPUT_RECORD,
    AR_OBJECT_RWDISPATCH_NODE_INPUT_RECORD, AR_OBJECT_GROUP_NODE_INPUT_RECORDS,
    AR_OBJECT_RWGROUP_NODE_INPUT_RECORDS, AR_OBJECT_THREAD_NODE_INPUT_RECORD,
    AR_OBJECT_RWTHREAD_NODE_INPUT_RECORD,

    AR_OBJECT_NODE_OUTPUT, AR_OBJECT_EMPTY_NODE_OUTPUT,
    AR_OBJECT_NODE_OUTPUT_ARRAY, AR_OBJECT_EMPTY_NODE_OUTPUT_ARRAY,

    AR_OBJECT_THREAD_NODE_OUTPUT_RECORDS, AR_OBJECT_GROUP_NODE_OUTPUT_RECORDS,

    // Shader Execution Reordering
    AR_OBJECT_HIT_OBJECT};

// Count of template arguments for basic kind of objects that look like
// templates (one or more type arguments).
static const uint8_t g_ArBasicKindsTemplateCount[] = {
    1, // AR_OBJECT_BUFFER

    // AR_OBJECT_TEXTURE,
    1, // AR_OBJECT_TEXTURE1D
    1, // AR_OBJECT_TEXTURE1D_ARRAY
    1, // AR_OBJECT_TEXTURE2D
    1, // AR_OBJECT_TEXTURE2D_ARRAY
    1, // AR_OBJECT_TEXTURE3D
    1, // AR_OBJECT_TEXTURECUBE
    1, // AR_OBJECT_TEXTURECUBE_ARRAY
    2, // AR_OBJECT_TEXTURE2DMS
    2, // AR_OBJECT_TEXTURE2DMS_ARRAY

    0, // AR_OBJECT_SAMPLER
    // AR_OBJECT_SAMPLER1D,
    // AR_OBJECT_SAMPLER2D,
    // AR_OBJECT_SAMPLER3D,
    // AR_OBJECT_SAMPLERCUBE,
    0, // AR_OBJECT_SAMPLERCOMPARISON

    1, // AR_OBJECT_CONSTANT_BUFFER,
    1, // AR_OBJECT_TEXTURE_BUFFER,

    1, // AR_OBJECT_POINTSTREAM
    1, // AR_OBJECT_LINESTREAM
    1, // AR_OBJECT_TRIANGLESTREAM

    2, // AR_OBJECT_INPUTPATCH
    2, // AR_OBJECT_OUTPUTPATCH

    1, // AR_OBJECT_RWTEXTURE1D
    1, // AR_OBJECT_RWTEXTURE1D_ARRAY
    1, // AR_OBJECT_RWTEXTURE2D
    1, // AR_OBJECT_RWTEXTURE2D_ARRAY
    1, // AR_OBJECT_RWTEXTURE3D
    1, // AR_OBJECT_RWBUFFER

    0, // AR_OBJECT_BYTEADDRESS_BUFFER
    0, // AR_OBJECT_RWBYTEADDRESS_BUFFER
    1, // AR_OBJECT_STRUCTURED_BUFFER
    1, // AR_OBJECT_RWSTRUCTURED_BUFFER
    // 1, // AR_OBJECT_RWSTRUCTURED_BUFFER_ALLOC
    // 1, // AR_OBJECT_RWSTRUCTURED_BUFFER_CONSUME
    1, // AR_OBJECT_APPEND_STRUCTURED_BUFFER
    1, // AR_OBJECT_CONSUME_STRUCTURED_BUFFER

    1, // AR_OBJECT_ROVBUFFER
    0, // AR_OBJECT_ROVBYTEADDRESS_BUFFER
    1, // AR_OBJECT_ROVSTRUCTURED_BUFFER
    1, // AR_OBJECT_ROVTEXTURE1D
    1, // AR_OBJECT_ROVTEXTURE1D_ARRAY
    1, // AR_OBJECT_ROVTEXTURE2D
    1, // AR_OBJECT_ROVTEXTURE2D_ARRAY
    1, // AR_OBJECT_ROVTEXTURE3D

    1, // AR_OBJECT_FEEDBACKTEXTURE2D
    1, // AR_OBJECT_FEEDBACKTEXTURE2D_ARRAY

// SPIRV change starts
#ifdef ENABLE_SPIRV_CODEGEN
    1, // AR_OBJECT_VK_SUBPASS_INPUT
    1, // AR_OBJECT_VK_SUBPASS_INPUT_MS,
    1, // AR_OBJECT_VK_SPIRV_TYPE
    1, // AR_OBJECT_VK_SPIRV_OPAQUE_TYPE
    1, // AR_OBJECT_VK_INTEGRAL_CONSTANT,
    1, // AR_OBJECT_VK_LITERAL,
    1, // AR_OBJECT_VK_SPV_INTRINSIC_TYPE
    1, // AR_OBJECT_VK_SPV_INTRINSIC_RESULT_ID
    2, // AR_OBJECT_VK_BUFFER_POINTER
#endif // ENABLE_SPIRV_CODEGEN
    // SPIRV change ends

    0, // AR_OBJECT_LEGACY_EFFECT   // Used for all unsupported but ignored
       // legacy effect types
    0, // AR_OBJECT_WAVE
    0, // AR_OBJECT_RAY_DESC
    0, // AR_OBJECT_ACCELERATION_STRUCT
    0, // AR_OBJECT_TRIANGLE_INTERSECTION_ATTRIBUTES

    0, // AR_OBJECT_STATE_OBJECT_CONFIG,
    0, // AR_OBJECT_GLOBAL_ROOT_SIGNATURE,
    0, // AR_OBJECT_LOCAL_ROOT_SIGNATURE,
    0, // AR_OBJECT_SUBOBJECT_TO_EXPORTS_ASSOC,
    0, // AR_OBJECT_RAYTRACING_SHADER_CONFIG,
    0, // AR_OBJECT_RAYTRACING_PIPELINE_CONFIG,
    0, // AR_OBJECT_TRIANGLE_HIT_GROUP,
    0, // AR_OBJECT_PROCEDURAL_PRIMITIVE_HIT_GROUP,
    0, // AR_OBJECT_RAYTRACING_PIPELINE_CONFIG1,

    1, // AR_OBJECT_RAY_QUERY,
    0, // AR_OBJECT_HEAP_RESOURCE,
    0, // AR_OBJECT_HEAP_SAMPLER,

    2, // AR_OBJECT_RWTEXTURE2DMS
    2, // AR_OBJECT_RWTEXTURE2DMS_ARRAY

    // WorkGraphs
    0, // AR_OBJECT_EMPTY_NODE_INPUT,
    1, // AR_OBJECT_DISPATCH_NODE_INPUT_RECORD,
    1, // AR_OBJECT_RWDISPATCH_NODE_INPUT_RECORD,
    1, // AR_OBJECT_GROUP_NODE_INPUT_RECORDS,
    1, // AR_OBJECT_RWGROUP_NODE_INPUT_RECORDS,
    1, // AR_OBJECT_THREAD_NODE_INPUT_RECORD,
    1, // AR_OBJECT_RWTHREAD_NODE_INPUT_RECORD,

    1, // AR_OBJECT_NODE_OUTPUT,
    0, // AR_OBJECT_EMPTY_NODE_OUTPUT,
    1, // AR_OBJECT_NODE_OUTPUT_ARRAY,
    0, // AR_OBJECT_EMPTY_NODE_OUTPUT_ARRAY,

    1, // AR_OBJECT_THREAD_NODE_OUTPUT_RECORDS,
    1, // AR_OBJECT_GROUP_NODE_OUTPUT_RECORDS

    // Shader Execution Reordering
    0, // AR_OBJECT_HIT_OBJECT,
};

C_ASSERT(_countof(g_ArBasicKindsAsTypes) ==
         _countof(g_ArBasicKindsTemplateCount));

/// <summary>Describes the how the subscript or indexing operators work on a
/// given type.</summary>
struct SubscriptOperatorRecord {
  unsigned int
      SubscriptCardinality : 4; // Number of elements expected in subscript -
                                // zero if operator not supported.
  bool HasMips : 1;   // true if the kind has a mips member; false otherwise
  bool HasSample : 1; // true if the kind has a sample member; false otherwise
};

// Subscript operators for objects that are represented as HLSL structures or
// templates.
static const SubscriptOperatorRecord g_ArBasicKindsSubscripts[] = {
    {1, MipsFalse, SampleFalse}, // AR_OBJECT_BUFFER (Buffer)

    // AR_OBJECT_TEXTURE,
    {1, MipsTrue, SampleFalse},  // AR_OBJECT_TEXTURE1D (Texture1D)
    {2, MipsTrue, SampleFalse},  // AR_OBJECT_TEXTURE1D_ARRAY (Texture1DArray)
    {2, MipsTrue, SampleFalse},  // AR_OBJECT_TEXTURE2D (Texture2D)
    {3, MipsTrue, SampleFalse},  // AR_OBJECT_TEXTURE2D_ARRAY (Texture2DArray)
    {3, MipsTrue, SampleFalse},  // AR_OBJECT_TEXTURE3D (Texture3D)
    {0, MipsFalse, SampleFalse}, // AR_OBJECT_TEXTURECUBE (TextureCube)
    {0, MipsFalse,
     SampleFalse}, // AR_OBJECT_TEXTURECUBE_ARRAY (TextureCubeArray)
    {2, MipsFalse, SampleTrue}, // AR_OBJECT_TEXTURE2DMS (Texture2DMS)
    {3, MipsFalse,
     SampleTrue}, // AR_OBJECT_TEXTURE2DMS_ARRAY (Texture2DMSArray)

    {0, MipsFalse, SampleFalse}, // AR_OBJECT_SAMPLER (SamplerState)
    // AR_OBJECT_SAMPLER1D,
    // AR_OBJECT_SAMPLER2D,
    // AR_OBJECT_SAMPLER3D,
    // AR_OBJECT_SAMPLERCUBE,
    {0, MipsFalse,
     SampleFalse}, // AR_OBJECT_SAMPLERCOMPARISON (SamplerComparison)

    {0, MipsFalse, SampleFalse}, // AR_OBJECT_CONSTANT_BUFFER
    {0, MipsFalse, SampleFalse}, // AR_OBJECT_TEXTURE_BUFFER

    {0, MipsFalse, SampleFalse}, // AR_OBJECT_POINTSTREAM (PointStream)
    {0, MipsFalse, SampleFalse}, // AR_OBJECT_LINESTREAM (LineStream)
    {0, MipsFalse, SampleFalse}, // AR_OBJECT_TRIANGLESTREAM (TriangleStream)

    {1, MipsFalse, SampleFalse}, // AR_OBJECT_INPUTPATCH (InputPatch)
    {1, MipsFalse, SampleFalse}, // AR_OBJECT_OUTPUTPATCH (OutputPatch)

    {1, MipsFalse, SampleFalse}, // AR_OBJECT_RWTEXTURE1D (RWTexture1D)
    {2, MipsFalse,
     SampleFalse}, // AR_OBJECT_RWTEXTURE1D_ARRAY (RWTexture1DArray)
    {2, MipsFalse, SampleFalse}, // AR_OBJECT_RWTEXTURE2D (RWTexture2D)
    {3, MipsFalse,
     SampleFalse}, // AR_OBJECT_RWTEXTURE2D_ARRAY (RWTexture2DArray)
    {3, MipsFalse, SampleFalse}, // AR_OBJECT_RWTEXTURE3D (RWTexture3D)
    {1, MipsFalse, SampleFalse}, // AR_OBJECT_RWBUFFER (RWBuffer)

    {0, MipsFalse,
     SampleFalse}, // AR_OBJECT_BYTEADDRESS_BUFFER (ByteAddressBuffer)
    {0, MipsFalse,
     SampleFalse}, // AR_OBJECT_RWBYTEADDRESS_BUFFER (RWByteAddressBuffer)
    {1, MipsFalse,
     SampleFalse}, // AR_OBJECT_STRUCTURED_BUFFER (StructuredBuffer)
    {1, MipsFalse,
     SampleFalse}, // AR_OBJECT_RWSTRUCTURED_BUFFER (RWStructuredBuffer)
    // AR_OBJECT_RWSTRUCTURED_BUFFER_ALLOC,
    // AR_OBJECT_RWSTRUCTURED_BUFFER_CONSUME,
    {0, MipsFalse, SampleFalse}, // AR_OBJECT_APPEND_STRUCTURED_BUFFER
                                 // (AppendStructuredBuffer)
    {0, MipsFalse, SampleFalse}, // AR_OBJECT_CONSUME_STRUCTURED_BUFFER
                                 // (ConsumeStructuredBuffer)

    {1, MipsFalse, SampleFalse}, // AR_OBJECT_ROVBUFFER (ROVBuffer)
    {0, MipsFalse,
     SampleFalse}, // AR_OBJECT_ROVBYTEADDRESS_BUFFER (ROVByteAddressBuffer)
    {1, MipsFalse,
     SampleFalse}, // AR_OBJECT_ROVSTRUCTURED_BUFFER (ROVStructuredBuffer)
    {1, MipsFalse, SampleFalse}, // AR_OBJECT_ROVTEXTURE1D (ROVTexture1D)
    {2, MipsFalse,
     SampleFalse}, // AR_OBJECT_ROVTEXTURE1D_ARRAY (ROVTexture1DArray)
    {2, MipsFalse, SampleFalse}, // AR_OBJECT_ROVTEXTURE2D (ROVTexture2D)
    {3, MipsFalse,
     SampleFalse}, // AR_OBJECT_ROVTEXTURE2D_ARRAY (ROVTexture2DArray)
    {3, MipsFalse, SampleFalse}, // AR_OBJECT_ROVTEXTURE3D (ROVTexture3D)

    {0, MipsFalse, SampleFalse}, // AR_OBJECT_FEEDBACKTEXTURE2D
    {0, MipsFalse, SampleFalse}, // AR_OBJECT_FEEDBACKTEXTURE2D_ARRAY

// SPIRV change starts
#ifdef ENABLE_SPIRV_CODEGEN
    {0, MipsFalse, SampleFalse}, // AR_OBJECT_VK_SUBPASS_INPUT (SubpassInput)
    {0, MipsFalse,
     SampleFalse}, // AR_OBJECT_VK_SUBPASS_INPUT_MS (SubpassInputMS)
    {0, MipsFalse, SampleFalse}, // AR_OBJECT_VK_SPIRV_TYPE
    {0, MipsFalse, SampleFalse}, // AR_OBJECT_VK_SPIRV_OPAQUE_TYPE
    {0, MipsFalse, SampleFalse}, // AR_OBJECT_VK_INTEGRAL_CONSTANT,
    {0, MipsFalse, SampleFalse}, // AR_OBJECT_VK_LITERAL,
    {0, MipsFalse, SampleFalse}, // AR_OBJECT_VK_SPV_INTRINSIC_TYPE
    {0, MipsFalse, SampleFalse}, // AR_OBJECT_VK_SPV_INTRINSIC_RESULT_ID
    {0, MipsFalse, SampleFalse}, // AR_OBJECT_VK_BUFFER_POINTER
#endif                           // ENABLE_SPIRV_CODEGEN
    // SPIRV change ends

    {0, MipsFalse,
     SampleFalse}, // AR_OBJECT_LEGACY_EFFECT (legacy effect objects)
    {0, MipsFalse, SampleFalse}, // AR_OBJECT_WAVE
    {0, MipsFalse, SampleFalse}, // AR_OBJECT_RAY_DESC
    {0, MipsFalse, SampleFalse}, // AR_OBJECT_ACCELERATION_STRUCT
    {0, MipsFalse, SampleFalse}, // AR_OBJECT_TRIANGLE_INTERSECTION_ATTRIBUTES

    {0, MipsFalse, SampleFalse}, // AR_OBJECT_STATE_OBJECT_CONFIG,
    {0, MipsFalse, SampleFalse}, // AR_OBJECT_GLOBAL_ROOT_SIGNATURE,
    {0, MipsFalse, SampleFalse}, // AR_OBJECT_LOCAL_ROOT_SIGNATURE,
    {0, MipsFalse, SampleFalse}, // AR_OBJECT_SUBOBJECT_TO_EXPORTS_ASSOC,
    {0, MipsFalse, SampleFalse}, // AR_OBJECT_RAYTRACING_SHADER_CONFIG,
    {0, MipsFalse, SampleFalse}, // AR_OBJECT_RAYTRACING_PIPELINE_CONFIG,
    {0, MipsFalse, SampleFalse}, // AR_OBJECT_TRIANGLE_HIT_GROUP,
    {0, MipsFalse, SampleFalse}, // AR_OBJECT_PROCEDURAL_PRIMITIVE_HIT_GROUP,
    {0, MipsFalse, SampleFalse}, // AR_OBJECT_RAYTRACING_PIPELINE_CONFIG1,

    {0, MipsFalse, SampleFalse}, // AR_OBJECT_RAY_QUERY,
    {0, MipsFalse, SampleFalse}, // AR_OBJECT_HEAP_RESOURCE,
    {0, MipsFalse, SampleFalse}, // AR_OBJECT_HEAP_SAMPLER,

    {2, MipsFalse, SampleTrue}, // AR_OBJECT_RWTEXTURE2DMS (RWTexture2DMS)
    {3, MipsFalse,
     SampleTrue}, // AR_OBJECT_RWTEXTURE2DMS_ARRAY (RWTexture2DMSArray)

    // WorkGraphs
    {0, MipsFalse, SampleFalse}, // AR_OBJECT_EMPTY_NODE_INPUT
    {0, MipsFalse, SampleFalse}, // AR_OBJECT_DISPATCH_NODE_INPUT_RECORD
    {0, MipsFalse, SampleFalse}, // AR_OBJECT_RWDISPATCH_NODE_INPUT_RECORD
    {1, MipsFalse, SampleFalse}, // AR_OBJECT_GROUP_NODE_INPUT_RECORDS
    {1, MipsFalse, SampleFalse}, // AR_OBJECT_RWGROUP_NODE_INPUT_RECORDS
    {0, MipsFalse, SampleFalse}, // AR_OBJECT_GROUP_NODE_INPUT_RECORD
    {0, MipsFalse, SampleFalse}, // AR_OBJECT_RWGROUP_NODE_INPUT_RECORD

    {0, MipsFalse, SampleFalse}, // AR_OBJECT_NODE_OUTPUT
    {0, MipsFalse, SampleFalse}, // AR_OBJECT_EMPTY_NODE_OUTPUT
    {1, MipsFalse, SampleFalse}, // AR_OBJECT_NODE_OUTPUT_ARRAY
    {1, MipsFalse, SampleFalse}, // AR_OBJECT_EMPTY_NODE_OUTPUT_ARRAY

    {1, MipsFalse, SampleFalse}, // AR_OBJECT_THREAD_NODE_OUTPUT_RECORDS
    {1, MipsFalse, SampleFalse}, // AR_OBJECT_GROUP_NODE_OUTPUT_RECORDS

    // Shader Execution Reordering
    {0, MipsFalse, SampleFalse}, // AR_OBJECT_HIT_OBJECT,
};

C_ASSERT(_countof(g_ArBasicKindsAsTypes) == _countof(g_ArBasicKindsSubscripts));

// Type names for ArBasicKind values.
static const char *g_ArBasicTypeNames[] = {
    "bool",
    "float",
    "half",
    "half",
    "float",
    "double",
    "int",
    "sbyte",
    "byte",
    "short",
    "ushort",
    "int",
    "uint",
    "long",
    "ulong",
    "min10float",
    "min16float",
    "min12int",
    "min16int",
    "min16uint",
    "int8_t4_packed",
    "uint8_t4_packed",
    "enum",

    "<count>",
    "<none>",
    "<unknown>",
    "<nocast>",
    "<dependent>",
    "<pointer>",
    "enum class",

    "null",
    "literal string",
    "string",
    // "texture",
    "Texture1D",
    "Texture1DArray",
    "Texture2D",
    "Texture2DArray",
    "Texture3D",
    "TextureCube",
    "TextureCubeArray",
    "Texture2DMS",
    "Texture2DMSArray",
    "SamplerState",
    "sampler1D",
    "sampler2D",
    "sampler3D",
    "samplerCUBE",
    "SamplerComparisonState",
    "Buffer",
    "RenderTargetView",
    "DepthStencilView",
    "ComputeShader",
    "DomainShader",
    "GeometryShader",
    "HullShader",
    "PixelShader",
    "VertexShader",
    "pixelfragment",
    "vertexfragment",
    "StateBlock",
    "Rasterizer",
    "DepthStencil",
    "Blend",
    "PointStream",
    "LineStream",
    "TriangleStream",
    "InputPatch",
    "OutputPatch",
    "RWTexture1D",
    "RWTexture1DArray",
    "RWTexture2D",
    "RWTexture2DArray",
    "RWTexture3D",
    "RWBuffer",
    "ByteAddressBuffer",
    "RWByteAddressBuffer",
    "StructuredBuffer",
    "RWStructuredBuffer",
    "RWStructuredBuffer(Incrementable)",
    "RWStructuredBuffer(Decrementable)",
    "AppendStructuredBuffer",
    "ConsumeStructuredBuffer",

    "ConstantBuffer",
    "TextureBuffer",

    "RasterizerOrderedBuffer",
    "RasterizerOrderedByteAddressBuffer",
    "RasterizerOrderedStructuredBuffer",
    "RasterizerOrderedTexture1D",
    "RasterizerOrderedTexture1DArray",
    "RasterizerOrderedTexture2D",
    "RasterizerOrderedTexture2DArray",
    "RasterizerOrderedTexture3D",

    "FeedbackTexture2D",
    "FeedbackTexture2DArray",

// SPIRV change starts
#ifdef ENABLE_SPIRV_CODEGEN
    "SubpassInput",
    "SubpassInputMS",
    "SpirvType",
    "SpirvOpaqueType",
    "integral_constant",
    "Literal",
    "ext_type",
    "ext_result_id",
    "BufferPointer",
#endif // ENABLE_SPIRV_CODEGEN
    // SPIRV change ends

    "<internal inner type object>",

    "deprecated effect object",
    "wave_t",
    "RayDesc",
    "RaytracingAccelerationStructure",
    "user defined type",
    "BuiltInTriangleIntersectionAttributes",

    // subobjects
    "StateObjectConfig",
    "GlobalRootSignature",
    "LocalRootSignature",
    "SubobjectToExportsAssociation",
    "RaytracingShaderConfig",
    "RaytracingPipelineConfig",
    "TriangleHitGroup",
    "ProceduralPrimitiveHitGroup",
    "RaytracingPipelineConfig1",

    "RayQuery",
    "HEAP_Resource",
    "HEAP_Sampler",

    "RWTexture2DMS",
    "RWTexture2DMSArray",

    // Workgraphs
    "EmptyNodeInput",
    "DispatchNodeInputRecord",
    "RWDispatchNodeInputRecord",
    "GroupNodeInputRecords",
    "RWGroupNodeInputRecords",
    "ThreadNodeInputRecord",
    "RWThreadNodeInputRecord",

    "NodeOutput",
    "EmptyNodeOutput",
    "NodeOutputArray",
    "EmptyNodeOutputArray",

    "ThreadNodeOutputRecords",
    "GroupNodeOutputRecords",

    // Shader Execution Reordering
    "HitObject",
};

C_ASSERT(_countof(g_ArBasicTypeNames) == AR_BASIC_MAXIMUM_COUNT);

static bool IsValidBasicKind(ArBasicKind kind) {
  return kind != AR_BASIC_COUNT && kind != AR_BASIC_NONE &&
         kind != AR_BASIC_UNKNOWN && kind != AR_BASIC_NOCAST &&
         kind != AR_BASIC_POINTER && kind != AR_OBJECT_RENDERTARGETVIEW &&
         kind != AR_OBJECT_DEPTHSTENCILVIEW &&
         kind != AR_OBJECT_COMPUTESHADER && kind != AR_OBJECT_DOMAINSHADER &&
         kind != AR_OBJECT_GEOMETRYSHADER && kind != AR_OBJECT_HULLSHADER &&
         kind != AR_OBJECT_PIXELSHADER && kind != AR_OBJECT_VERTEXSHADER &&
         kind != AR_OBJECT_PIXELFRAGMENT && kind != AR_OBJECT_VERTEXFRAGMENT;
}
// kind should never be a flag value or effects framework type - we simply do
// not expect to deal with these
#define DXASSERT_VALIDBASICKIND(kind)                                          \
  DXASSERT(IsValidBasicKind(kind), "otherwise caller is using a special flag " \
                                   "or an unsupported kind value");

static const char *g_DeprecatedEffectObjectNames[] = {
    // These are case insensitive in fxc, but we'll just create two case aliases
    // to capture the majority of cases
    "texture", "Texture", "pixelshader", "PixelShader", "vertexshader",
    "VertexShader",

    // These are case sensitive in fxc
    "pixelfragment",     // 13
    "vertexfragment",    // 14
    "ComputeShader",     // 13
    "DomainShader",      // 12
    "GeometryShader",    // 14
    "HullShader",        // 10
    "BlendState",        // 10
    "DepthStencilState", // 17
    "DepthStencilView",  // 16
    "RasterizerState",   // 15
    "RenderTargetView",  // 16
};

static bool IsStaticMember(const HLSL_INTRINSIC *fn) {
  return fn->Flags & INTRIN_FLAG_STATIC_MEMBER;
}

static bool IsVariadicIntrinsicFunction(const HLSL_INTRINSIC *fn) {
  return fn->pArgs[fn->uNumArgs - 1].uTemplateId == INTRIN_TEMPLATE_VARARGS;
}

static bool IsVariadicArgument(const HLSL_INTRINSIC_ARGUMENT &arg) {
  return arg.uTemplateId == INTRIN_TEMPLATE_VARARGS;
}

static hlsl::ParameterModifier
ParamModsFromIntrinsicArg(const HLSL_INTRINSIC_ARGUMENT *pArg) {
  UINT64 qwUsage = pArg->qwUsage & AR_QUAL_IN_OUT;
  if (qwUsage == AR_QUAL_IN_OUT) {
    return hlsl::ParameterModifier(hlsl::ParameterModifier::Kind::InOut);
  }
  if (qwUsage == AR_QUAL_OUT) {
    return hlsl::ParameterModifier(hlsl::ParameterModifier::Kind::Out);
  }
  if (pArg->qwUsage == AR_QUAL_REF)
    return hlsl::ParameterModifier(hlsl::ParameterModifier::Kind::Ref);
  DXASSERT(qwUsage & AR_QUAL_IN, "else usage is incorrect");
  return hlsl::ParameterModifier(hlsl::ParameterModifier::Kind::In);
}
static void InitParamMods(const HLSL_INTRINSIC *pIntrinsic,
                          SmallVectorImpl<hlsl::ParameterModifier> &paramMods,
                          size_t VariadicArgumentCount = 0u) {
  // The first argument is the return value, which isn't included.
  for (unsigned i = 1; i < pIntrinsic->uNumArgs; ++i) {
    // Once we reach varargs we can break out of this loop.
    if (IsVariadicArgument(pIntrinsic->pArgs[i]))
      break;
    paramMods.push_back(ParamModsFromIntrinsicArg(&pIntrinsic->pArgs[i]));
  }
  // For variadic functions, any argument not explicitly specified will be
  // considered an input argument.
  for (unsigned i = 0; i < VariadicArgumentCount; ++i) {
    paramMods.push_back(
        hlsl::ParameterModifier(hlsl::ParameterModifier::Kind::In));
  }
}

static bool IsBuiltinTable(StringRef tableName) {
  return tableName.compare(kBuiltinIntrinsicTableName) == 0;
}

static bool HasUnsignedOpcode(LPCSTR tableName, IntrinsicOp opcode) {
  return IsBuiltinTable(tableName) && HasUnsignedIntrinsicOpcode(opcode);
}

static void AddHLSLIntrinsicAttr(FunctionDecl *FD, ASTContext &context,
                                 LPCSTR tableName, LPCSTR lowering,
                                 const HLSL_INTRINSIC *pIntrinsic) {
  unsigned opcode = pIntrinsic->Op;
  if (HasUnsignedOpcode(tableName, static_cast<IntrinsicOp>(opcode))) {
    QualType Ty = FD->getReturnType();
    if (pIntrinsic->iOverloadParamIndex != -1) {
      const FunctionProtoType *FT =
          FD->getFunctionType()->getAs<FunctionProtoType>();
      Ty = FT->getParamType(pIntrinsic->iOverloadParamIndex);
      // To go thru reference type.
      if (Ty->isReferenceType())
        Ty = Ty.getNonReferenceType();
    }

    // TODO: refine the code for getting element type
    if (const ExtVectorType *VecTy =
            hlsl::ConvertHLSLVecMatTypeToExtVectorType(context, Ty)) {
      Ty = VecTy->getElementType();
    }

    // Make sure to use unsigned op when return type is 'unsigned' matrix
    bool isUnsignedMatOp =
        IsHLSLMatType(Ty) && GetHLSLMatElementType(Ty)->isUnsignedIntegerType();

    if (Ty->isUnsignedIntegerType() || isUnsignedMatOp) {
      opcode = hlsl::GetUnsignedOpcode(opcode);
    }
  }
  FD->addAttr(
      HLSLIntrinsicAttr::CreateImplicit(context, tableName, lowering, opcode));
  if (pIntrinsic->Flags & INTRIN_FLAG_READ_NONE)
    FD->addAttr(ConstAttr::CreateImplicit(context));
  if (pIntrinsic->Flags & INTRIN_FLAG_READ_ONLY)
    FD->addAttr(PureAttr::CreateImplicit(context));
  if (pIntrinsic->Flags & INTRIN_FLAG_IS_WAVE)
    FD->addAttr(HLSLWaveSensitiveAttr::CreateImplicit(context));
  if (pIntrinsic->MinShaderModel) {
    unsigned Major = pIntrinsic->MinShaderModel >> 4;
    unsigned Minor = pIntrinsic->MinShaderModel & 0xF;
    FD->addAttr(AvailabilityAttr::CreateImplicit(
        context, &context.Idents.get(""), clang::VersionTuple(Major, Minor),
        clang::VersionTuple(), clang::VersionTuple(), false, ""));
  }
}

static FunctionDecl *
AddHLSLIntrinsicFunction(ASTContext &context, NamespaceDecl *NS,
                         LPCSTR tableName, LPCSTR lowering,
                         const HLSL_INTRINSIC *pIntrinsic,
                         std::vector<QualType> *functionArgQualTypesVector) {
  DeclContext *currentDeclContext =
      NS ? static_cast<DeclContext *>(NS) : context.getTranslationUnitDecl();

  std::vector<QualType> &functionArgQualTypes = *functionArgQualTypesVector;
  const size_t functionArgTypeCount = functionArgQualTypes.size();
  const bool isVariadic = IsVariadicIntrinsicFunction(pIntrinsic);
  // For variadic functions, the number of arguments is larger than the
  // function declaration signature.
  const size_t VariadicArgumentCount =
      isVariadic ? (functionArgTypeCount - (pIntrinsic->uNumArgs - 1)) : 0;
  DXASSERT(isVariadic || functionArgTypeCount - 1 <= g_MaxIntrinsicParamCount,
           "otherwise g_MaxIntrinsicParamCount should be larger");

  SmallVector<hlsl::ParameterModifier, g_MaxIntrinsicParamCount> paramMods;
  InitParamMods(pIntrinsic, paramMods, VariadicArgumentCount);

  for (size_t i = 1; i < functionArgTypeCount; i++) {
    // Change out/inout param to reference type.
    if (paramMods[i - 1].isAnyOut() ||
        paramMods[i - 1].GetKind() == hlsl::ParameterModifier::Kind::Ref) {
      QualType Ty = functionArgQualTypes[i];
      // Aggregate type will be indirect param convert to pointer type.
      // Don't need add reference for it.
      if ((!Ty->isArrayType() && !Ty->isRecordType()) ||
          hlsl::IsHLSLVecMatType(Ty)) {
        functionArgQualTypes[i] = context.getLValueReferenceType(Ty);
      }
    }
  }

  IdentifierInfo &functionId = context.Idents.get(
      StringRef(pIntrinsic->pArgs[0].pName), tok::TokenKind::identifier);
  DeclarationName functionName(&functionId);
  auto protoInfo = clang::FunctionProtoType::ExtProtoInfo();
  protoInfo.Variadic = isVariadic;
  // functionArgQualTypes first element is the function return type, and
  // function argument types start at index 1.
  const QualType fnReturnType = functionArgQualTypes[0];
  std::vector<QualType> fnArgTypes(functionArgQualTypes.begin() + 1,
                                   functionArgQualTypes.end());

  StorageClass SC = IsStaticMember(pIntrinsic) ? SC_Static : SC_Extern;
  QualType functionType =
      context.getFunctionType(fnReturnType, fnArgTypes, protoInfo, paramMods);
  FunctionDecl *functionDecl = FunctionDecl::Create(
      context, currentDeclContext, NoLoc,
      DeclarationNameInfo(functionName, NoLoc), functionType, nullptr, SC,
      InlineSpecifiedFalse, HasWrittenPrototypeTrue);
  currentDeclContext->addDecl(functionDecl);

  // Add intrinsic attribute
  AddHLSLIntrinsicAttr(functionDecl, context, tableName, lowering, pIntrinsic);

  llvm::SmallVector<ParmVarDecl *, 4> paramDecls;
  for (size_t i = 1; i < functionArgTypeCount; i++) {
    // For variadic functions all non-explicit arguments will have the same
    // name: "..."
    std::string name = i < pIntrinsic->uNumArgs - 1
                           ? pIntrinsic->pArgs[i].pName
                           : pIntrinsic->pArgs[pIntrinsic->uNumArgs - 1].pName;
    IdentifierInfo &parameterId =
        context.Idents.get(name, tok::TokenKind::identifier);
    ParmVarDecl *paramDecl =
        ParmVarDecl::Create(context, functionDecl, NoLoc, NoLoc, &parameterId,
                            functionArgQualTypes[i], nullptr,
                            StorageClass::SC_None, nullptr, paramMods[i - 1]);
    functionDecl->addDecl(paramDecl);
    paramDecls.push_back(paramDecl);
  }

  functionDecl->setParams(paramDecls);
  functionDecl->setImplicit(true);

  if (!NS)
    functionDecl->addAttr(HLSLBuiltinCallAttr::CreateImplicit(context));

  return functionDecl;
}

/// <summary>
/// Checks whether the specified expression is a (possibly parenthesized) comma
/// operator.
/// </summary>
static bool IsExpressionBinaryComma(const Expr *expr) {
  DXASSERT_NOMSG(expr != nullptr);
  expr = expr->IgnoreParens();
  return expr->getStmtClass() == Expr::StmtClass::BinaryOperatorClass &&
         cast<BinaryOperator>(expr)->getOpcode() ==
             BinaryOperatorKind::BO_Comma;
}

/// <summary>
/// Silences diagnostics for the initialization sequence, typically because they
/// have already been emitted.
/// </summary>
static void SilenceSequenceDiagnostics(InitializationSequence *initSequence) {
  DXASSERT_NOMSG(initSequence != nullptr);
  initSequence->SetFailed(InitializationSequence::FK_ListInitializationFailed);
}

class UsedIntrinsic {
public:
  static int compareArgs(const QualType &LHS, const QualType &RHS) {
    // The canonical representations are unique'd in an ASTContext, and so these
    // should be stable.
    return RHS.getTypePtr() - LHS.getTypePtr();
  }

  static int compareIntrinsic(const HLSL_INTRINSIC *LHS,
                              const HLSL_INTRINSIC *RHS) {
    // The intrinsics are defined in a single static table, and so should be
    // stable.
    return RHS - LHS;
  }

  int compare(const UsedIntrinsic &other) const {
    // Check whether it's the same instance.
    if (this == &other)
      return 0;

    int result = compareIntrinsic(m_intrinsicSource, other.m_intrinsicSource);
    if (result != 0)
      return result;

    // At this point, it's the exact same intrinsic name.
    // Compare the arguments for ordering then.

    DXASSERT(IsVariadicIntrinsicFunction(m_intrinsicSource) ||
                 m_args.size() == other.m_args.size(),
             "only variadic intrinsics can be overloaded on argument count");

    // For variadic functions with different number of args, order by number of
    // arguments.
    if (m_args.size() != other.m_args.size())
      return m_args.size() - other.m_args.size();

    for (size_t i = 0; i < m_args.size(); i++) {
      int argComparison = compareArgs(m_args[i], other.m_args[i]);
      if (argComparison != 0)
        return argComparison;
    }

    // Exactly the same.
    return 0;
  }

public:
  UsedIntrinsic(const HLSL_INTRINSIC *intrinsicSource,
                llvm::ArrayRef<QualType> args)
      : m_args(args.begin(), args.end()), m_intrinsicSource(intrinsicSource),
        m_functionDecl(nullptr) {}

  void setFunctionDecl(FunctionDecl *value) const {
    DXASSERT(value != nullptr, "no reason to clear this out");
    DXASSERT(m_functionDecl == nullptr,
             "otherwise cached value is being invaldiated");
    m_functionDecl = value;
  }
  FunctionDecl *getFunctionDecl() const { return m_functionDecl; }

  bool operator==(const UsedIntrinsic &other) const {
    return compare(other) == 0;
  }

  bool operator<(const UsedIntrinsic &other) const {
    return compare(other) < 0;
  }

private:
  std::vector<QualType> m_args;
  const HLSL_INTRINSIC *m_intrinsicSource;
  mutable FunctionDecl *m_functionDecl;
};

template <typename T> inline void AssignOpt(T value, T *ptr) {
  if (ptr != nullptr) {
    *ptr = value;
  }
}

static bool CombineBasicTypes(ArBasicKind LeftKind, ArBasicKind RightKind,
                              ArBasicKind *pOutKind) {
  // Make sure the kinds are both valid
  if ((LeftKind < 0 || LeftKind >= AR_BASIC_MAXIMUM_COUNT) ||
      (RightKind < 0 || RightKind >= AR_BASIC_MAXIMUM_COUNT)) {
    return false;
  }

  // If kinds match perfectly, succeed without requiring they be basic
  if (LeftKind == RightKind) {
    *pOutKind = LeftKind;
    return true;
  }

  // More complicated combination requires that the kinds be basic
  if (LeftKind >= AR_BASIC_COUNT || RightKind >= AR_BASIC_COUNT) {
    return false;
  }

  UINT uLeftProps = GetBasicKindProps(LeftKind);
  UINT uRightProps = GetBasicKindProps(RightKind);
  UINT uBits = GET_BPROP_BITS(uLeftProps) > GET_BPROP_BITS(uRightProps)
                   ? GET_BPROP_BITS(uLeftProps)
                   : GET_BPROP_BITS(uRightProps);
  UINT uBothFlags = uLeftProps & uRightProps;
  UINT uEitherFlags = uLeftProps | uRightProps;

  // Notes: all numeric types have either BPROP_FLOATING or BPROP_INTEGER (even
  // bool)
  //        unsigned only applies to non-literal ints, not bool or enum
  //        literals, bool, and enum are all BPROP_BITS0
  if (uBothFlags & BPROP_BOOLEAN) {
    *pOutKind = AR_BASIC_BOOL;
    return true;
  }

  bool bFloatResult = 0 != (uEitherFlags & BPROP_FLOATING);
  if (uBothFlags & BPROP_LITERAL) {
    *pOutKind = bFloatResult ? AR_BASIC_LITERAL_FLOAT : AR_BASIC_LITERAL_INT;
    return true;
  }

  // Starting approximation of result properties:
  // - float if either are float, otherwise int (see Notes above)
  // - min/partial precision if both have same flag
  // - if not float, add unsigned if either is unsigned
  UINT uResultFlags = (uBothFlags & (BPROP_INTEGER | BPROP_MIN_PRECISION |
                                     BPROP_PARTIAL_PRECISION)) |
                      (uEitherFlags & BPROP_FLOATING) |
                      (!bFloatResult ? (uEitherFlags & BPROP_UNSIGNED) : 0);

  // If one is literal/bool/enum, use min/partial precision from the other
  if (uEitherFlags & (BPROP_LITERAL | BPROP_BOOLEAN | BPROP_ENUM)) {
    uResultFlags |=
        uEitherFlags & (BPROP_MIN_PRECISION | BPROP_PARTIAL_PRECISION);
  }

  // Now if we have partial precision, we know the result must be half
  if (uResultFlags & BPROP_PARTIAL_PRECISION) {
    *pOutKind = AR_BASIC_FLOAT32_PARTIAL_PRECISION;
    return true;
  }

  // uBits are already initialized to max of either side, so now:
  // if only one is float, get result props from float side
  //  min16float + int -> min16float
  //  also take min precision from that side
  if (bFloatResult && 0 == (uBothFlags & BPROP_FLOATING)) {
    uResultFlags = (uLeftProps & BPROP_FLOATING) ? uLeftProps : uRightProps;
    uBits = GET_BPROP_BITS(uResultFlags);
    uResultFlags &= ~BPROP_LITERAL;
  }

  bool bMinPrecisionResult = uResultFlags & BPROP_MIN_PRECISION;

  // if uBits is 0 here, upgrade to 32-bits
  // this happens if bool, literal or enum on both sides,
  // or if float came from literal side
  if (uBits == BPROP_BITS0)
    uBits = BPROP_BITS32;

  DXASSERT(uBits != BPROP_BITS0,
           "CombineBasicTypes: uBits should not be zero at this point");
  DXASSERT(uBits != BPROP_BITS8,
           "CombineBasicTypes: 8-bit types not supported at this time");

  if (bMinPrecisionResult) {
    DXASSERT(
        uBits < BPROP_BITS32,
        "CombineBasicTypes: min-precision result must be less than 32-bits");
  } else {
    DXASSERT(uBits > BPROP_BITS12,
             "CombineBasicTypes: 10 or 12 bit result must be min precision");
  }
  if (bFloatResult) {
    DXASSERT(uBits != BPROP_BITS12,
             "CombineBasicTypes: 12-bit result must be int");
  } else {
    DXASSERT(uBits != BPROP_BITS10,
             "CombineBasicTypes: 10-bit result must be float");
  }
  if (uBits == BPROP_BITS12) {
    DXASSERT(!(uResultFlags & BPROP_UNSIGNED),
             "CombineBasicTypes: 12-bit result must not be unsigned");
  }

  if (bFloatResult) {
    switch (uBits) {
    case BPROP_BITS10:
      *pOutKind = AR_BASIC_MIN10FLOAT;
      break;
    case BPROP_BITS16:
      *pOutKind = bMinPrecisionResult ? AR_BASIC_MIN16FLOAT : AR_BASIC_FLOAT16;
      break;
    case BPROP_BITS32:
      *pOutKind = AR_BASIC_FLOAT32;
      break;
    case BPROP_BITS64:
      *pOutKind = AR_BASIC_FLOAT64;
      break;
    default:
      DXASSERT(false, "Unexpected bit count for float result");
      break;
    }
  } else {
    // int or unsigned int
    switch (uBits) {
    case BPROP_BITS12:
      *pOutKind = AR_BASIC_MIN12INT;
      break;
    case BPROP_BITS16:
      if (uResultFlags & BPROP_UNSIGNED)
        *pOutKind = bMinPrecisionResult ? AR_BASIC_MIN16UINT : AR_BASIC_UINT16;
      else
        *pOutKind = bMinPrecisionResult ? AR_BASIC_MIN16INT : AR_BASIC_INT16;
      break;
    case BPROP_BITS32:
      *pOutKind =
          (uResultFlags & BPROP_UNSIGNED) ? AR_BASIC_UINT32 : AR_BASIC_INT32;
      break;
    case BPROP_BITS64:
      *pOutKind =
          (uResultFlags & BPROP_UNSIGNED) ? AR_BASIC_UINT64 : AR_BASIC_INT64;
      break;
    default:
      DXASSERT(false, "Unexpected bit count for int result");
      break;
    }
  }

  return true;
}

class UsedIntrinsicStore : public std::set<UsedIntrinsic> {};

static void GetIntrinsicMethods(ArBasicKind kind,
                                const HLSL_INTRINSIC **intrinsics,
                                size_t *intrinsicCount) {
  DXASSERT_NOMSG(intrinsics != nullptr);
  DXASSERT_NOMSG(intrinsicCount != nullptr);

  switch (kind) {
  case AR_OBJECT_TRIANGLESTREAM:
  case AR_OBJECT_POINTSTREAM:
  case AR_OBJECT_LINESTREAM:
    *intrinsics = g_StreamMethods;
    *intrinsicCount = _countof(g_StreamMethods);
    break;
  case AR_OBJECT_TEXTURE1D:
    *intrinsics = g_Texture1DMethods;
    *intrinsicCount = _countof(g_Texture1DMethods);
    break;
  case AR_OBJECT_TEXTURE1D_ARRAY:
    *intrinsics = g_Texture1DArrayMethods;
    *intrinsicCount = _countof(g_Texture1DArrayMethods);
    break;
  case AR_OBJECT_TEXTURE2D:
    *intrinsics = g_Texture2DMethods;
    *intrinsicCount = _countof(g_Texture2DMethods);
    break;
  case AR_OBJECT_TEXTURE2DMS:
    *intrinsics = g_Texture2DMSMethods;
    *intrinsicCount = _countof(g_Texture2DMSMethods);
    break;
  case AR_OBJECT_TEXTURE2D_ARRAY:
    *intrinsics = g_Texture2DArrayMethods;
    *intrinsicCount = _countof(g_Texture2DArrayMethods);
    break;
  case AR_OBJECT_TEXTURE2DMS_ARRAY:
    *intrinsics = g_Texture2DArrayMSMethods;
    *intrinsicCount = _countof(g_Texture2DArrayMSMethods);
    break;
  case AR_OBJECT_TEXTURE3D:
    *intrinsics = g_Texture3DMethods;
    *intrinsicCount = _countof(g_Texture3DMethods);
    break;
  case AR_OBJECT_TEXTURECUBE:
    *intrinsics = g_TextureCUBEMethods;
    *intrinsicCount = _countof(g_TextureCUBEMethods);
    break;
  case AR_OBJECT_TEXTURECUBE_ARRAY:
    *intrinsics = g_TextureCUBEArrayMethods;
    *intrinsicCount = _countof(g_TextureCUBEArrayMethods);
    break;
  case AR_OBJECT_BUFFER:
    *intrinsics = g_BufferMethods;
    *intrinsicCount = _countof(g_BufferMethods);
    break;
  case AR_OBJECT_RWTEXTURE1D:
  case AR_OBJECT_ROVTEXTURE1D:
    *intrinsics = g_RWTexture1DMethods;
    *intrinsicCount = _countof(g_RWTexture1DMethods);
    break;
  case AR_OBJECT_RWTEXTURE1D_ARRAY:
  case AR_OBJECT_ROVTEXTURE1D_ARRAY:
    *intrinsics = g_RWTexture1DArrayMethods;
    *intrinsicCount = _countof(g_RWTexture1DArrayMethods);
    break;
  case AR_OBJECT_RWTEXTURE2D:
  case AR_OBJECT_ROVTEXTURE2D:
    *intrinsics = g_RWTexture2DMethods;
    *intrinsicCount = _countof(g_RWTexture2DMethods);
    break;
  case AR_OBJECT_RWTEXTURE2D_ARRAY:
  case AR_OBJECT_ROVTEXTURE2D_ARRAY:
    *intrinsics = g_RWTexture2DArrayMethods;
    *intrinsicCount = _countof(g_RWTexture2DArrayMethods);
    break;
  case AR_OBJECT_RWTEXTURE3D:
  case AR_OBJECT_ROVTEXTURE3D:
    *intrinsics = g_RWTexture3DMethods;
    *intrinsicCount = _countof(g_RWTexture3DMethods);
    break;
  case AR_OBJECT_FEEDBACKTEXTURE2D:
    *intrinsics = g_FeedbackTexture2DMethods;
    *intrinsicCount = _countof(g_FeedbackTexture2DMethods);
    break;
  case AR_OBJECT_FEEDBACKTEXTURE2D_ARRAY:
    *intrinsics = g_FeedbackTexture2DArrayMethods;
    *intrinsicCount = _countof(g_FeedbackTexture2DArrayMethods);
    break;
  case AR_OBJECT_RWBUFFER:
  case AR_OBJECT_ROVBUFFER:
    *intrinsics = g_RWBufferMethods;
    *intrinsicCount = _countof(g_RWBufferMethods);
    break;
  case AR_OBJECT_BYTEADDRESS_BUFFER:
    *intrinsics = g_ByteAddressBufferMethods;
    *intrinsicCount = _countof(g_ByteAddressBufferMethods);
    break;
  case AR_OBJECT_RWBYTEADDRESS_BUFFER:
  case AR_OBJECT_ROVBYTEADDRESS_BUFFER:
    *intrinsics = g_RWByteAddressBufferMethods;
    *intrinsicCount = _countof(g_RWByteAddressBufferMethods);
    break;
  case AR_OBJECT_STRUCTURED_BUFFER:
    *intrinsics = g_StructuredBufferMethods;
    *intrinsicCount = _countof(g_StructuredBufferMethods);
    break;
  case AR_OBJECT_RWSTRUCTURED_BUFFER:
  case AR_OBJECT_ROVSTRUCTURED_BUFFER:
    *intrinsics = g_RWStructuredBufferMethods;
    *intrinsicCount = _countof(g_RWStructuredBufferMethods);
    break;
  case AR_OBJECT_APPEND_STRUCTURED_BUFFER:
    *intrinsics = g_AppendStructuredBufferMethods;
    *intrinsicCount = _countof(g_AppendStructuredBufferMethods);
    break;
  case AR_OBJECT_CONSUME_STRUCTURED_BUFFER:
    *intrinsics = g_ConsumeStructuredBufferMethods;
    *intrinsicCount = _countof(g_ConsumeStructuredBufferMethods);
    break;
  case AR_OBJECT_RAY_QUERY:
    *intrinsics = g_RayQueryMethods;
    *intrinsicCount = _countof(g_RayQueryMethods);
    break;
  case AR_OBJECT_HIT_OBJECT:
    *intrinsics = g_DxHitObjectMethods;
    *intrinsicCount = _countof(g_DxHitObjectMethods);
    break;
  case AR_OBJECT_RWTEXTURE2DMS:
    *intrinsics = g_RWTexture2DMSMethods;
    *intrinsicCount = _countof(g_RWTexture2DMSMethods);
    break;
  case AR_OBJECT_RWTEXTURE2DMS_ARRAY:
    *intrinsics = g_RWTexture2DMSArrayMethods;
    *intrinsicCount = _countof(g_RWTexture2DMSArrayMethods);
    break;
  case AR_OBJECT_EMPTY_NODE_INPUT:
    *intrinsics = g_EmptyNodeInputMethods;
    *intrinsicCount = _countof(g_EmptyNodeInputMethods);
    break;
  case AR_OBJECT_RWDISPATCH_NODE_INPUT_RECORD:
    *intrinsics = g_RWDispatchNodeInputRecordMethods;
    *intrinsicCount = _countof(g_RWDispatchNodeInputRecordMethods);
    break;
  case AR_OBJECT_GROUP_NODE_INPUT_RECORDS:
  case AR_OBJECT_RWGROUP_NODE_INPUT_RECORDS:
    *intrinsics = g_GroupNodeInputRecordsMethods;
    *intrinsicCount = _countof(g_GroupNodeInputRecordsMethods);
    break;
  case AR_OBJECT_NODE_OUTPUT:
    *intrinsics = g_NodeOutputMethods;
    *intrinsicCount = _countof(g_NodeOutputMethods);
    break;
  case AR_OBJECT_EMPTY_NODE_OUTPUT:
    *intrinsics = g_EmptyNodeOutputMethods;
    *intrinsicCount = _countof(g_EmptyNodeOutputMethods);
    break;
  case AR_OBJECT_THREAD_NODE_OUTPUT_RECORDS:
  case AR_OBJECT_GROUP_NODE_OUTPUT_RECORDS:
    *intrinsics = g_GroupOrThreadNodeOutputRecordsMethods;
    *intrinsicCount = _countof(g_GroupOrThreadNodeOutputRecordsMethods);
    break;

    // SPIRV change starts
#ifdef ENABLE_SPIRV_CODEGEN
  case AR_OBJECT_VK_SUBPASS_INPUT:
    *intrinsics = g_VkSubpassInputMethods;
    *intrinsicCount = _countof(g_VkSubpassInputMethods);
    break;
  case AR_OBJECT_VK_SUBPASS_INPUT_MS:
    *intrinsics = g_VkSubpassInputMSMethods;
    *intrinsicCount = _countof(g_VkSubpassInputMSMethods);
    break;
#endif // ENABLE_SPIRV_CODEGEN
  // SPIRV change ends
  default:
    *intrinsics = nullptr;
    *intrinsicCount = 0;
    break;
  }
}

static bool IsRowOrColumnVariable(size_t value) {
  return IA_SPECIAL_BASE <= value &&
         value <= (IA_SPECIAL_BASE + IA_SPECIAL_SLOTS - 1);
}

static bool
DoesComponentTypeAcceptMultipleTypes(LEGAL_INTRINSIC_COMPTYPES value) {
  return value == LICOMPTYPE_ANY_INT ||        // signed or unsigned ints
         value == LICOMPTYPE_ANY_INT32 ||      // signed or unsigned ints
         value == LICOMPTYPE_ANY_FLOAT ||      // float or double
         value == LICOMPTYPE_FLOAT_LIKE ||     // float or min16
         value == LICOMPTYPE_FLOAT_DOUBLE ||   // float or double
         value == LICOMPTYPE_NUMERIC ||        // all sorts of numbers
         value == LICOMPTYPE_NUMERIC32 ||      // all sorts of numbers
         value == LICOMPTYPE_NUMERIC32_ONLY || // all sorts of numbers
         value == LICOMPTYPE_ANY;              // any time
}

static bool DoesComponentTypeAcceptMultipleTypes(BYTE value) {
  return DoesComponentTypeAcceptMultipleTypes(
      static_cast<LEGAL_INTRINSIC_COMPTYPES>(value));
}

static bool
DoesLegalTemplateAcceptMultipleTypes(LEGAL_INTRINSIC_TEMPLATES value) {
  // Note that LITEMPLATE_OBJECT can accept different types, but it
  // specifies a single 'layout'. In practice, this information is used
  // together with a component type that specifies a single object.

  return value == LITEMPLATE_ANY; // Any layout
}

static bool DoesLegalTemplateAcceptMultipleTypes(BYTE value) {
  return DoesLegalTemplateAcceptMultipleTypes(
      static_cast<LEGAL_INTRINSIC_TEMPLATES>(value));
}

static bool TemplateHasDefaultType(ArBasicKind kind) {
  switch (kind) {
  case AR_OBJECT_BUFFER:
  case AR_OBJECT_TEXTURE1D:
  case AR_OBJECT_TEXTURE2D:
  case AR_OBJECT_TEXTURE3D:
  case AR_OBJECT_TEXTURE1D_ARRAY:
  case AR_OBJECT_TEXTURE2D_ARRAY:
  case AR_OBJECT_TEXTURECUBE:
  case AR_OBJECT_TEXTURECUBE_ARRAY:
    // SPIRV change starts
#ifdef ENABLE_SPIRV_CODEGEN
  case AR_OBJECT_VK_SUBPASS_INPUT:
  case AR_OBJECT_VK_SUBPASS_INPUT_MS:
#endif // ENABLE_SPIRV_CODEGEN
       // SPIRV change ends
    return true;
  default:
    // Objects with default types return true. Everything else is false.
    return false;
  }
}

/// <summary>
/// Use this class to iterate over intrinsic definitions that come from an
/// external source.
/// </summary>
class IntrinsicTableDefIter {
private:
  StringRef _typeName;
  StringRef _functionName;
  llvm::SmallVector<CComPtr<IDxcIntrinsicTable>, 2> &_tables;
  const HLSL_INTRINSIC *_tableIntrinsic;
  UINT64 _tableLookupCookie;
  unsigned _tableIndex;
  unsigned _argCount;
  bool _firstChecked;

  IntrinsicTableDefIter(
      llvm::SmallVector<CComPtr<IDxcIntrinsicTable>, 2> &tables,
      StringRef typeName, StringRef functionName, unsigned argCount)
      : _typeName(typeName), _functionName(functionName), _tables(tables),
        _tableIntrinsic(nullptr), _tableLookupCookie(0), _tableIndex(0),
        _argCount(argCount), _firstChecked(false) {}

  void CheckForIntrinsic() {
    if (_tableIndex >= _tables.size()) {
      return;
    }

    _firstChecked = true;

    // TODO: review this - this will allocate at least once per string
    CA2WEX<> typeName(_typeName.str().c_str());
    CA2WEX<> functionName(_functionName.str().c_str());

    if (FAILED(_tables[_tableIndex]->LookupIntrinsic(
            typeName, functionName, &_tableIntrinsic, &_tableLookupCookie))) {
      _tableLookupCookie = 0;
      _tableIntrinsic = nullptr;
    }
  }

  void MoveToNext() {
    for (;;) {
      // If we don't have an intrinsic, try the following table.
      if (_firstChecked && _tableIntrinsic == nullptr) {
        _tableIndex++;
      }

      CheckForIntrinsic();
      if (_tableIndex == _tables.size() ||
          (_tableIntrinsic != nullptr &&
           _tableIntrinsic->uNumArgs ==
               (_argCount + 1))) // uNumArgs includes return
        break;
    }
  }

public:
  static IntrinsicTableDefIter
  CreateStart(llvm::SmallVector<CComPtr<IDxcIntrinsicTable>, 2> &tables,
              StringRef typeName, StringRef functionName, unsigned argCount) {
    IntrinsicTableDefIter result(tables, typeName, functionName, argCount);
    return result;
  }

  static IntrinsicTableDefIter
  CreateEnd(llvm::SmallVector<CComPtr<IDxcIntrinsicTable>, 2> &tables) {
    IntrinsicTableDefIter result(tables, StringRef(), StringRef(), 0);
    result._tableIndex = tables.size();
    return result;
  }

  bool operator!=(const IntrinsicTableDefIter &other) {
    if (!_firstChecked) {
      MoveToNext();
    }
    return _tableIndex != other._tableIndex; // More things could be compared
                                             // but we only match end.
  }

  const HLSL_INTRINSIC *operator*() const {
    DXASSERT(_firstChecked, "otherwise deref without comparing to end");
    return _tableIntrinsic;
  }

  LPCSTR GetTableName() const {
    LPCSTR tableName = nullptr;
    if (FAILED(_tables[_tableIndex]->GetTableName(&tableName))) {
      return nullptr;
    }
    return tableName;
  }

  LPCSTR GetLoweringStrategy() const {
    LPCSTR lowering = nullptr;
    if (FAILED(_tables[_tableIndex]->GetLoweringStrategy(_tableIntrinsic->Op,
                                                         &lowering))) {
      return nullptr;
    }
    return lowering;
  }

  IntrinsicTableDefIter &operator++() {
    MoveToNext();
    return *this;
  }
};

/// <summary>
/// Use this class to iterate over intrinsic definitions that have the same name
/// and parameter count.
/// </summary>
class IntrinsicDefIter {
  const HLSL_INTRINSIC *_current;
  const HLSL_INTRINSIC *_end;
  IntrinsicTableDefIter _tableIter;

  IntrinsicDefIter(const HLSL_INTRINSIC *value, const HLSL_INTRINSIC *end,
                   IntrinsicTableDefIter tableIter)
      : _current(value), _end(end), _tableIter(tableIter) {}

public:
  static IntrinsicDefIter CreateStart(const HLSL_INTRINSIC *table, size_t count,
                                      const HLSL_INTRINSIC *start,
                                      IntrinsicTableDefIter tableIter) {
    return IntrinsicDefIter(start, table + count, tableIter);
  }

  static IntrinsicDefIter CreateEnd(const HLSL_INTRINSIC *table, size_t count,
                                    IntrinsicTableDefIter tableIter) {
    return IntrinsicDefIter(table + count, table + count, tableIter);
  }

  bool operator!=(const IntrinsicDefIter &other) {
    return _current != other._current ||
           _tableIter.operator!=(other._tableIter);
  }

  const HLSL_INTRINSIC *operator*() const {
    return (_current != _end) ? _current : *_tableIter;
  }

  LPCSTR GetTableName() const {
    return (_current != _end) ? kBuiltinIntrinsicTableName
                              : _tableIter.GetTableName();
  }

  LPCSTR GetLoweringStrategy() const {
    return (_current != _end) ? "" : _tableIter.GetLoweringStrategy();
  }

  IntrinsicDefIter &operator++() {
    if (_current != _end) {
      const HLSL_INTRINSIC *next = _current + 1;
      if (next != _end && _current->uNumArgs == next->uNumArgs &&
          0 == strcmp(_current->pArgs[0].pName, next->pArgs[0].pName)) {
        _current = next;
      } else {
        _current = _end;
      }
    } else {
      ++_tableIter;
    }

    return *this;
  }
};

static void AddHLSLSubscriptAttr(Decl *D, ASTContext &context,
                                 HLSubscriptOpcode opcode) {
  StringRef group = GetHLOpcodeGroupName(HLOpcodeGroup::HLSubscript);
  D->addAttr(HLSLIntrinsicAttr::CreateImplicit(context, group, "",
                                               static_cast<unsigned>(opcode)));
  D->addAttr(HLSLCXXOverloadAttr::CreateImplicit(context));
}

static void
CreateSimpleField(clang::ASTContext &context, CXXRecordDecl *recordDecl,
                  StringRef Name, QualType Ty,
                  AccessSpecifier access = AccessSpecifier::AS_public) {
  IdentifierInfo &fieldId =
      context.Idents.get(Name, tok::TokenKind::identifier);
  TypeSourceInfo *filedTypeSource = context.getTrivialTypeSourceInfo(Ty, NoLoc);
  const bool MutableFalse = false;
  const InClassInitStyle initStyle = InClassInitStyle::ICIS_NoInit;

  FieldDecl *fieldDecl =
      FieldDecl::Create(context, recordDecl, NoLoc, NoLoc, &fieldId, Ty,
                        filedTypeSource, nullptr, MutableFalse, initStyle);
  fieldDecl->setAccess(access);
  fieldDecl->setImplicit(true);

  recordDecl->addDecl(fieldDecl);
}

// struct RayDesc
//{
//    float3 Origin;
//    float  TMin;
//    float3 Direction;
//    float  TMax;
//};
static CXXRecordDecl *CreateRayDescStruct(clang::ASTContext &context,
                                          QualType float3Ty) {
  DeclContext *currentDeclContext = context.getTranslationUnitDecl();
  IdentifierInfo &rayDesc =
      context.Idents.get(StringRef("RayDesc"), tok::TokenKind::identifier);
  CXXRecordDecl *rayDescDecl = CXXRecordDecl::Create(
      context, TagTypeKind::TTK_Struct, currentDeclContext, NoLoc, NoLoc,
      &rayDesc, nullptr, DelayTypeCreationTrue);
  rayDescDecl->addAttr(
      FinalAttr::CreateImplicit(context, FinalAttr::Keyword_final));
  rayDescDecl->startDefinition();

  QualType floatTy = context.FloatTy;
  // float3 Origin;
  CreateSimpleField(context, rayDescDecl, "Origin", float3Ty);
  // float TMin;
  CreateSimpleField(context, rayDescDecl, "TMin", floatTy);
  // float3 Direction;
  CreateSimpleField(context, rayDescDecl, "Direction", float3Ty);
  // float  TMax;
  CreateSimpleField(context, rayDescDecl, "TMax", floatTy);

  rayDescDecl->completeDefinition();
  // Both declarations need to be present for correct handling.
  currentDeclContext->addDecl(rayDescDecl);
  rayDescDecl->setImplicit(true);
  return rayDescDecl;
}

// struct BuiltInTriangleIntersectionAttributes
// {
//   float2 barycentrics;
// };
static CXXRecordDecl *
AddBuiltInTriangleIntersectionAttributes(ASTContext &context,
                                         QualType baryType) {
  DeclContext *curDC = context.getTranslationUnitDecl();
  IdentifierInfo &attributesId =
      context.Idents.get(StringRef("BuiltInTriangleIntersectionAttributes"),
                         tok::TokenKind::identifier);
  CXXRecordDecl *attributesDecl = CXXRecordDecl::Create(
      context, TagTypeKind::TTK_Struct, curDC, NoLoc, NoLoc, &attributesId,
      nullptr, DelayTypeCreationTrue);
  attributesDecl->addAttr(
      FinalAttr::CreateImplicit(context, FinalAttr::Keyword_final));
  attributesDecl->startDefinition();
  // float2 barycentrics;
  CreateSimpleField(context, attributesDecl, "barycentrics", baryType);
  attributesDecl->completeDefinition();
  attributesDecl->setImplicit(true);
  curDC->addDecl(attributesDecl);
  return attributesDecl;
}

//
// Subobjects

static CXXRecordDecl *
StartSubobjectDecl(ASTContext &context, const char *name,
                   DXIL::SubobjectKind Kind,
                   DXIL::HitGroupType HGT = DXIL::HitGroupType::LastEntry) {
  IdentifierInfo &id =
      context.Idents.get(StringRef(name), tok::TokenKind::identifier);
  CXXRecordDecl *decl = CXXRecordDecl::Create(
      context, TagTypeKind::TTK_Struct, context.getTranslationUnitDecl(), NoLoc,
      NoLoc, &id, nullptr, DelayTypeCreationTrue);
  decl->addAttr(HLSLSubObjectAttr::CreateImplicit(
      context, static_cast<unsigned>(Kind), static_cast<unsigned>(HGT)));
  decl->addAttr(FinalAttr::CreateImplicit(context, FinalAttr::Keyword_final));
  decl->startDefinition();
  return decl;
}

void FinishSubobjectDecl(ASTContext &context, CXXRecordDecl *decl) {
  decl->completeDefinition();
  context.getTranslationUnitDecl()->addDecl(decl);
  decl->setImplicit(true);
}

// struct StateObjectConfig
// {
//   uint32_t Flags;
// };
static CXXRecordDecl *CreateSubobjectStateObjectConfig(ASTContext &context) {
  CXXRecordDecl *decl = StartSubobjectDecl(
      context, "StateObjectConfig", DXIL::SubobjectKind::StateObjectConfig);
  CreateSimpleField(context, decl, "Flags", context.UnsignedIntTy,
                    AccessSpecifier::AS_private);
  FinishSubobjectDecl(context, decl);
  return decl;
}

// struct GlobalRootSignature
// {
//   string signature;
// };
static CXXRecordDecl *CreateSubobjectRootSignature(ASTContext &context,
                                                   bool global) {
  CXXRecordDecl *decl = StartSubobjectDecl(
      context, global ? "GlobalRootSignature" : "LocalRootSignature",
      global ? DXIL::SubobjectKind::GlobalRootSignature
             : DXIL::SubobjectKind::LocalRootSignature);

  CreateSimpleField(context, decl, "Data", context.HLSLStringTy,
                    AccessSpecifier::AS_private);
  FinishSubobjectDecl(context, decl);
  return decl;
}

// struct SubobjectToExportsAssociation
// {
//   string Subobject;
//   string Exports;
// };
static CXXRecordDecl *
CreateSubobjectSubobjectToExportsAssoc(ASTContext &context) {
  CXXRecordDecl *decl =
      StartSubobjectDecl(context, "SubobjectToExportsAssociation",
                         DXIL::SubobjectKind::SubobjectToExportsAssociation);
  CreateSimpleField(context, decl, "Subobject", context.HLSLStringTy,
                    AccessSpecifier::AS_private);
  CreateSimpleField(context, decl, "Exports", context.HLSLStringTy,
                    AccessSpecifier::AS_private);
  FinishSubobjectDecl(context, decl);
  return decl;
}

// struct RaytracingShaderConfig
// {
//   uint32_t MaxPayloadSizeInBytes;
//   uint32_t MaxAttributeSizeInBytes;
// };
static CXXRecordDecl *
CreateSubobjectRaytracingShaderConfig(ASTContext &context) {
  CXXRecordDecl *decl =
      StartSubobjectDecl(context, "RaytracingShaderConfig",
                         DXIL::SubobjectKind::RaytracingShaderConfig);
  CreateSimpleField(context, decl, "MaxPayloadSizeInBytes",
                    context.UnsignedIntTy, AccessSpecifier::AS_private);
  CreateSimpleField(context, decl, "MaxAttributeSizeInBytes",
                    context.UnsignedIntTy, AccessSpecifier::AS_private);
  FinishSubobjectDecl(context, decl);
  return decl;
}

// struct RaytracingPipelineConfig
// {
//   uint32_t MaxTraceRecursionDepth;
// };
static CXXRecordDecl *
CreateSubobjectRaytracingPipelineConfig(ASTContext &context) {
  CXXRecordDecl *decl =
      StartSubobjectDecl(context, "RaytracingPipelineConfig",
                         DXIL::SubobjectKind::RaytracingPipelineConfig);
  CreateSimpleField(context, decl, "MaxTraceRecursionDepth",
                    context.UnsignedIntTy, AccessSpecifier::AS_private);
  FinishSubobjectDecl(context, decl);
  return decl;
}

// struct RaytracingPipelineConfig1
// {
//   uint32_t MaxTraceRecursionDepth;
//   uint32_t Flags;
// };
static CXXRecordDecl *
CreateSubobjectRaytracingPipelineConfig1(ASTContext &context) {
  CXXRecordDecl *decl =
      StartSubobjectDecl(context, "RaytracingPipelineConfig1",
                         DXIL::SubobjectKind::RaytracingPipelineConfig1);
  CreateSimpleField(context, decl, "MaxTraceRecursionDepth",
                    context.UnsignedIntTy, AccessSpecifier::AS_private);
  CreateSimpleField(context, decl, "Flags", context.UnsignedIntTy,
                    AccessSpecifier::AS_private);
  FinishSubobjectDecl(context, decl);
  return decl;
}

// struct TriangleHitGroup
// {
//   string AnyHit;
//   string ClosestHit;
// };
static CXXRecordDecl *CreateSubobjectTriangleHitGroup(ASTContext &context) {
  CXXRecordDecl *decl = StartSubobjectDecl(context, "TriangleHitGroup",
                                           DXIL::SubobjectKind::HitGroup,
                                           DXIL::HitGroupType::Triangle);
  CreateSimpleField(context, decl, "AnyHit", context.HLSLStringTy,
                    AccessSpecifier::AS_private);
  CreateSimpleField(context, decl, "ClosestHit", context.HLSLStringTy,
                    AccessSpecifier::AS_private);
  FinishSubobjectDecl(context, decl);
  return decl;
}

// struct ProceduralPrimitiveHitGroup
// {
//   string AnyHit;
//   string ClosestHit;
//   string Intersection;
// };
static CXXRecordDecl *
CreateSubobjectProceduralPrimitiveHitGroup(ASTContext &context) {
  CXXRecordDecl *decl = StartSubobjectDecl(
      context, "ProceduralPrimitiveHitGroup", DXIL::SubobjectKind::HitGroup,
      DXIL::HitGroupType::ProceduralPrimitive);
  CreateSimpleField(context, decl, "AnyHit", context.HLSLStringTy,
                    AccessSpecifier::AS_private);
  CreateSimpleField(context, decl, "ClosestHit", context.HLSLStringTy,
                    AccessSpecifier::AS_private);
  CreateSimpleField(context, decl, "Intersection", context.HLSLStringTy,
                    AccessSpecifier::AS_private);
  FinishSubobjectDecl(context, decl);
  return decl;
}

/// <summary>Creates a Typedef in the specified ASTContext.</summary>
static TypedefDecl *CreateGlobalTypedef(ASTContext *context, const char *ident,
                                        QualType baseType) {
  DXASSERT_NOMSG(context != nullptr);
  DXASSERT_NOMSG(ident != nullptr);
  DXASSERT_NOMSG(!baseType.isNull());

  DeclContext *declContext = context->getTranslationUnitDecl();
  TypeSourceInfo *typeSource = context->getTrivialTypeSourceInfo(baseType);
  TypedefDecl *decl =
      TypedefDecl::Create(*context, declContext, NoLoc, NoLoc,
                          &context->Idents.get(ident), typeSource);
  declContext->addDecl(decl);
  decl->setImplicit(true);
  return decl;
}

class HLSLExternalSource : public ExternalSemaSource {
private:
  // Inner types.
  struct FindStructBasicTypeResult {
    ArBasicKind Kind; // Kind of struct (eg, AR_OBJECT_TEXTURE2D)
    unsigned int BasicKindsAsTypeIndex; // Index into g_ArBasicKinds*

    FindStructBasicTypeResult(ArBasicKind kind,
                              unsigned int basicKindAsTypeIndex)
        : Kind(kind), BasicKindsAsTypeIndex(basicKindAsTypeIndex) {}

    bool Found() const { return Kind != AR_BASIC_UNKNOWN; }
  };

  // Declaration for matrix and vector templates.
  ClassTemplateDecl *m_matrixTemplateDecl;
  ClassTemplateDecl *m_vectorTemplateDecl;

  ClassTemplateDecl *m_vkIntegralConstantTemplateDecl;
  ClassTemplateDecl *m_vkLiteralTemplateDecl;
  ClassTemplateDecl *m_vkBufferPointerTemplateDecl;

  // Declarations for Work Graph Output Record types
  ClassTemplateDecl *m_GroupNodeOutputRecordsTemplateDecl;
  ClassTemplateDecl *m_ThreadNodeOutputRecordsTemplateDecl;

  // Namespace decl for hlsl intrinsic functions
  NamespaceDecl *m_hlslNSDecl;

  // Namespace decl for Vulkan-specific intrinsic functions
  NamespaceDecl *m_vkNSDecl;

  // Namespace decl for dx intrinsic functions
  NamespaceDecl *m_dxNSDecl;

  // Context being processed.
  ASTContext *m_context;

  // Semantic analyzer being processed.
  Sema *m_sema;

  // Intrinsic tables available externally.
  llvm::SmallVector<CComPtr<IDxcIntrinsicTable>, 2> m_intrinsicTables;

  // Scalar types indexed by HLSLScalarType.
  QualType m_scalarTypes[HLSLScalarTypeCount];

  // Scalar types already built.
  TypedefDecl *m_scalarTypeDefs[HLSLScalarTypeCount];

  // Matrix types already built indexed by type, row-count, col-count. Should
  // probably move to a sparse map. Instrument to figure out best initial size.
  QualType m_matrixTypes[HLSLScalarTypeCount][4][4];

  // Matrix types already built, in shorthand form.
  TypedefDecl *m_matrixShorthandTypes[HLSLScalarTypeCount][4][4];

  // Vector types already built.
  QualType m_vectorTypes[HLSLScalarTypeCount][DXIL::kDefaultMaxVectorLength];
  TypedefDecl
      *m_vectorTypedefs[HLSLScalarTypeCount][DXIL::kDefaultMaxVectorLength];

  // BuiltinType for each scalar type.
  QualType m_baseTypes[HLSLScalarTypeCount];

  // String type
  QualType m_hlslStringType;
  TypedefDecl *m_hlslStringTypedef;

  // Built-in object types declarations, indexed by basic kind constant.
  CXXRecordDecl *m_objectTypeDecls[_countof(g_ArBasicKindsAsTypes)];
  // Map from object decl to the object index.
  using ObjectTypeDeclMapType =
      std::array<std::pair<CXXRecordDecl *, unsigned>,
                 _countof(g_ArBasicKindsAsTypes) +
                     _countof(g_DeprecatedEffectObjectNames)>;
  ObjectTypeDeclMapType m_objectTypeDeclsMap;

  UsedIntrinsicStore m_usedIntrinsics;

  /// <summary>Add all base QualTypes for each hlsl scalar types.</summary>
  void AddBaseTypes();

  /// <summary>Adds all supporting declarations to reference scalar
  /// types.</summary>
  void AddHLSLScalarTypes();

  /// <summary>Adds string type QualType for HLSL string declarations</summary>
  void AddHLSLStringType();

  QualType GetTemplateObjectDataType(CXXRecordDecl *recordDecl) {
    DXASSERT_NOMSG(recordDecl != nullptr);
    TemplateParameterList *parameterList =
        recordDecl->getTemplateParameterList(0);
    NamedDecl *parameterDecl = parameterList->getParam(0);

    DXASSERT(parameterDecl->getKind() == Decl::Kind::TemplateTypeParm,
             "otherwise recordDecl isn't one of the built-in objects with "
             "templates");
    TemplateTypeParmDecl *parmDecl =
        dyn_cast<TemplateTypeParmDecl>(parameterDecl);
    return QualType(parmDecl->getTypeForDecl(), 0);
  }

  // Determines whether the given intrinsic parameter type has a single QualType
  // mapping.
  QualType GetSingleQualTypeForMapping(const HLSL_INTRINSIC *intrinsic,
                                       int index) {
    int templateRef = intrinsic->pArgs[index].uTemplateId;
    int componentRef = intrinsic->pArgs[index].uComponentTypeId;
    const HLSL_INTRINSIC_ARGUMENT *templateArg = &intrinsic->pArgs[templateRef];
    const HLSL_INTRINSIC_ARGUMENT *componentArg =
        &intrinsic->pArgs[componentRef];
    const HLSL_INTRINSIC_ARGUMENT *matrixArg = &intrinsic->pArgs[index];

    if (templateRef >= 0 && templateArg->uTemplateId == templateRef &&
        !DoesLegalTemplateAcceptMultipleTypes(templateArg->uLegalTemplates) &&
        componentRef >= 0 && componentRef != INTRIN_COMPTYPE_FROM_TYPE_ELT0 &&
        componentRef != INTRIN_COMPTYPE_FROM_NODEOUTPUT &&
        componentArg->uComponentTypeId == 0 &&
        !DoesComponentTypeAcceptMultipleTypes(
            componentArg->uLegalComponentTypes) &&
        !IsRowOrColumnVariable(matrixArg->uCols) &&
        !IsRowOrColumnVariable(matrixArg->uRows)) {
      ArTypeObjectKind templateKind =
          g_LegalIntrinsicTemplates[templateArg->uLegalTemplates][0];
      ArBasicKind elementKind =
          g_LegalIntrinsicCompTypes[componentArg->uLegalComponentTypes][0];
      return NewSimpleAggregateType(templateKind, elementKind, 0,
                                    matrixArg->uRows, matrixArg->uCols);
    }

    return QualType();
  }

  // Adds a new template parameter declaration to the specified array and
  // returns the type for the parameter.
  QualType AddTemplateParamToArray(
      const char *name, CXXRecordDecl *recordDecl, int templateDepth,
      NamedDecl *(&templateParamNamedDecls)[g_MaxIntrinsicParamCount + 1],
      size_t *templateParamNamedDeclsCount) {
    DXASSERT_NOMSG(name != nullptr);
    DXASSERT_NOMSG(recordDecl != nullptr);
    DXASSERT_NOMSG(templateParamNamedDecls != nullptr);
    DXASSERT_NOMSG(templateParamNamedDeclsCount != nullptr);
    DXASSERT(*templateParamNamedDeclsCount < _countof(templateParamNamedDecls),
             "otherwise constants should be updated");
    assert(*templateParamNamedDeclsCount < _countof(templateParamNamedDecls));

    // Create the declaration for the template parameter.
    IdentifierInfo *id = &m_context->Idents.get(StringRef(name));
    TemplateTypeParmDecl *templateTypeParmDecl = TemplateTypeParmDecl::Create(
        *m_context, recordDecl, NoLoc, NoLoc, templateDepth,
        *templateParamNamedDeclsCount, id, TypenameTrue, ParameterPackFalse);
    templateParamNamedDecls[*templateParamNamedDeclsCount] =
        templateTypeParmDecl;

    // Create the type that the parameter represents.
    QualType result = m_context->getTemplateTypeParmType(
        templateDepth, *templateParamNamedDeclsCount, ParameterPackFalse,
        templateTypeParmDecl);

    // Increment the declaration count for the array; as long as caller passes
    // in both arguments, it need not concern itself with maintaining this
    // value.
    (*templateParamNamedDeclsCount)++;

    return result;
  }

  // Adds a function specified by the given intrinsic to a record declaration.
  // The template depth will be zero for records that don't have a "template<>"
  // line even if conceptual; or one if it does have one.
  void AddObjectIntrinsicTemplate(CXXRecordDecl *recordDecl, int templateDepth,
                                  const HLSL_INTRINSIC *intrinsic) {
    DXASSERT_NOMSG(recordDecl != nullptr);
    DXASSERT_NOMSG(intrinsic != nullptr);
    DXASSERT(intrinsic->uNumArgs > 0,
             "otherwise there isn't even an intrinsic name");
    DXASSERT(intrinsic->uNumArgs <= (g_MaxIntrinsicParamCount + 1),
             "otherwise g_MaxIntrinsicParamCount should be updated");

    // uNumArgs includes the result type, g_MaxIntrinsicParamCount doesn't, thus
    // the +1.
    assert(intrinsic->uNumArgs <= (g_MaxIntrinsicParamCount + 1));

    // TODO: implement template parameter constraints for HLSL intrinsic methods
    // in declarations

    //
    // Build template parameters, parameter types, and the return type.
    // Parameter declarations are built after the function is created, to use it
    // as their scope.
    //
    unsigned int numParams = intrinsic->uNumArgs - 1;
    NamedDecl *templateParamNamedDecls[g_MaxIntrinsicParamCount + 1];
    size_t templateParamNamedDeclsCount = 0;
    QualType argsQTs[g_MaxIntrinsicParamCount];
    StringRef argNames[g_MaxIntrinsicParamCount];
    QualType functionResultQT = recordDecl->getASTContext().VoidTy;

    DXASSERT(_countof(templateParamNamedDecls) >= numParams + 1,
             "need enough templates for all parameters and the return type, "
             "otherwise constants need updating");

    // Handle the return type.
    // functionResultQT = GetSingleQualTypeForMapping(intrinsic, 0);
    // if (functionResultQT.isNull()) {
    // Workaround for template parameter argument count mismatch.
    // Create template parameter for return type always
    // TODO: reenable the check and skip template argument.
    functionResultQT = AddTemplateParamToArray(
        "TResult", recordDecl, templateDepth, templateParamNamedDecls,
        &templateParamNamedDeclsCount);
    // }

    SmallVector<hlsl::ParameterModifier, g_MaxIntrinsicParamCount> paramMods;
    InitParamMods(intrinsic, paramMods);

    // Consider adding more cases where return type can be handled a priori.
    // Ultimately #260431 should do significantly better.

    // Handle parameters.
    for (unsigned int i = 1; i < intrinsic->uNumArgs; i++) {
      //
      // GetSingleQualTypeForMapping can be used here to remove unnecessary
      // template arguments.
      //
      // However this may produce template instantiations with equivalent
      // template arguments for overloaded methods. It's possible to resolve
      // some of these by generating specializations, but the current intrinsic
      // table has rules that are hard to process in their current form to find
      // all cases.
      //
      char name[g_MaxIntrinsicParamName + 2];
      name[0] = 'T';
      name[1] = '\0';
      strcat_s(name, intrinsic->pArgs[i].pName);
      argsQTs[i - 1] = AddTemplateParamToArray(name, recordDecl, templateDepth,
                                               templateParamNamedDecls,
                                               &templateParamNamedDeclsCount);
      // Change out/inout param to reference type.
      if (paramMods[i - 1].isAnyOut())
        argsQTs[i - 1] = m_context->getLValueReferenceType(argsQTs[i - 1]);

      argNames[i - 1] = StringRef(intrinsic->pArgs[i].pName);
    }

    // Create the declaration.
    IdentifierInfo *ii =
        &m_context->Idents.get(StringRef(intrinsic->pArgs[0].pName));
    DeclarationName declarationName = DeclarationName(ii);

    StorageClass SC = IsStaticMember(intrinsic) ? SC_Static : SC_None;

    CXXMethodDecl *functionDecl = CreateObjectFunctionDeclarationWithParams(
        *m_context, recordDecl, functionResultQT,
        ArrayRef<QualType>(argsQTs, numParams),
        ArrayRef<StringRef>(argNames, numParams), declarationName, true, SC,
        templateParamNamedDeclsCount > 0);
    functionDecl->setImplicit(true);

    // If the function is a template function, create the declaration and
    // cross-reference.
    if (templateParamNamedDeclsCount > 0) {
      hlsl::CreateFunctionTemplateDecl(*m_context, recordDecl, functionDecl,
                                       templateParamNamedDecls,
                                       templateParamNamedDeclsCount);
    }
  }

  // Checks whether the two specified intrinsics generate equivalent templates.
  // For example: foo (any_int) and foo (any_float) are only unambiguous in the
  // context of HLSL intrinsic rules, and their difference can't be expressed
  // with C++ templates.
  bool AreIntrinsicTemplatesEquivalent(const HLSL_INTRINSIC *left,
                                       const HLSL_INTRINSIC *right) {
    if (left == right) {
      return true;
    }
    if (left == nullptr || right == nullptr) {
      return false;
    }

    return (left->uNumArgs == right->uNumArgs &&
            0 == strcmp(left->pArgs[0].pName, right->pArgs[0].pName));
  }

  // Adds all the intrinsic methods that correspond to the specified type.
  void AddObjectMethods(ArBasicKind kind, CXXRecordDecl *recordDecl,
                        int templateDepth) {
    DXASSERT_NOMSG(recordDecl != nullptr);
    DXASSERT_NOMSG(templateDepth >= 0);

    const HLSL_INTRINSIC *intrinsics;
    const HLSL_INTRINSIC *prior = nullptr;
    size_t intrinsicCount;

    GetIntrinsicMethods(kind, &intrinsics, &intrinsicCount);
    DXASSERT((intrinsics == nullptr) == (intrinsicCount == 0),
             "intrinsic table pointer must match count (null for zero, "
             "something valid otherwise");

    while (intrinsicCount--) {
      if (!AreIntrinsicTemplatesEquivalent(intrinsics, prior)) {
        AddObjectIntrinsicTemplate(recordDecl, templateDepth, intrinsics);
        prior = intrinsics;
      }

      intrinsics++;
    }
  }

  void AddDoubleSubscriptSupport(
      ClassTemplateDecl *typeDecl, CXXRecordDecl *recordDecl,
      const char *memberName, QualType elementType,
      TemplateTypeParmDecl *templateTypeParmDecl, const char *type0Name,
      const char *type1Name, const char *indexer0Name, QualType indexer0Type,
      const char *indexer1Name, QualType indexer1Type) {
    DXASSERT_NOMSG(typeDecl != nullptr);
    DXASSERT_NOMSG(recordDecl != nullptr);
    DXASSERT_NOMSG(memberName != nullptr);
    DXASSERT_NOMSG(!elementType.isNull());
    DXASSERT_NOMSG(templateTypeParmDecl != nullptr);
    DXASSERT_NOMSG(type0Name != nullptr);
    DXASSERT_NOMSG(type1Name != nullptr);
    DXASSERT_NOMSG(indexer0Name != nullptr);
    DXASSERT_NOMSG(!indexer0Type.isNull());
    DXASSERT_NOMSG(indexer1Name != nullptr);
    DXASSERT_NOMSG(!indexer1Type.isNull());

    //
    // Add inner types to the templates to represent the following C++ code
    // inside the class. public:
    //  class sample_slice_type
    //  {
    //  public: TElement operator[](uint3 index);
    //  };
    //  class sample_type
    //  {
    //  public: sample_slice_type operator[](uint slice);
    //  };
    //  sample_type sample;
    //
    // Variable names reflect this structure, but this code will also produce
    // the types for .mips access.
    //
    const bool MutableTrue = true;
    DeclarationName subscriptName =
        m_context->DeclarationNames.getCXXOperatorName(OO_Subscript);
    CXXRecordDecl *sampleSliceTypeDecl =
        CXXRecordDecl::Create(*m_context, TTK_Class, recordDecl, NoLoc, NoLoc,
                              &m_context->Idents.get(StringRef(type1Name)));
    sampleSliceTypeDecl->setAccess(AS_public);
    sampleSliceTypeDecl->setImplicit();
    recordDecl->addDecl(sampleSliceTypeDecl);
    sampleSliceTypeDecl->startDefinition();
    const bool MutableFalse = false;
    FieldDecl *sliceHandleDecl = FieldDecl::Create(
        *m_context, sampleSliceTypeDecl, NoLoc, NoLoc,
        &m_context->Idents.get(StringRef("handle")), indexer0Type,
        m_context->CreateTypeSourceInfo(indexer0Type), nullptr, MutableFalse,
        ICIS_NoInit);
    sliceHandleDecl->setAccess(AS_private);
    sampleSliceTypeDecl->addDecl(sliceHandleDecl);

    CXXMethodDecl *sampleSliceSubscriptDecl =
        CreateObjectFunctionDeclarationWithParams(
            *m_context, sampleSliceTypeDecl, elementType,
            ArrayRef<QualType>(indexer1Type),
            ArrayRef<StringRef>(StringRef(indexer1Name)), subscriptName, true);
    hlsl::CreateFunctionTemplateDecl(
        *m_context, sampleSliceTypeDecl, sampleSliceSubscriptDecl,
        reinterpret_cast<NamedDecl **>(&templateTypeParmDecl), 1);
    sampleSliceTypeDecl->completeDefinition();

    CXXRecordDecl *sampleTypeDecl =
        CXXRecordDecl::Create(*m_context, TTK_Class, recordDecl, NoLoc, NoLoc,
                              &m_context->Idents.get(StringRef(type0Name)));
    sampleTypeDecl->setAccess(AS_public);
    recordDecl->addDecl(sampleTypeDecl);
    sampleTypeDecl->startDefinition();
    sampleTypeDecl->setImplicit();

    FieldDecl *sampleHandleDecl = FieldDecl::Create(
        *m_context, sampleTypeDecl, NoLoc, NoLoc,
        &m_context->Idents.get(StringRef("handle")), indexer0Type,
        m_context->CreateTypeSourceInfo(indexer0Type), nullptr, MutableFalse,
        ICIS_NoInit);
    sampleHandleDecl->setAccess(AS_private);
    sampleTypeDecl->addDecl(sampleHandleDecl);

    QualType sampleSliceType = m_context->getRecordType(sampleSliceTypeDecl);

    CXXMethodDecl *sampleSubscriptDecl =
        CreateObjectFunctionDeclarationWithParams(
            *m_context, sampleTypeDecl,
            m_context->getLValueReferenceType(sampleSliceType),
            ArrayRef<QualType>(indexer0Type),
            ArrayRef<StringRef>(StringRef(indexer0Name)), subscriptName, true);
    sampleTypeDecl->completeDefinition();

    // Add subscript attribute
    AddHLSLSubscriptAttr(sampleSubscriptDecl, *m_context,
                         HLSubscriptOpcode::DoubleSubscript);

    QualType sampleTypeQT = m_context->getRecordType(sampleTypeDecl);
    FieldDecl *sampleFieldDecl = FieldDecl::Create(
        *m_context, recordDecl, NoLoc, NoLoc,
        &m_context->Idents.get(StringRef(memberName)), sampleTypeQT,
        m_context->CreateTypeSourceInfo(sampleTypeQT), nullptr, MutableTrue,
        ICIS_NoInit);
    sampleFieldDecl->setAccess(AS_public);
    recordDecl->addDecl(sampleFieldDecl);
  }

  void AddObjectSubscripts(ArBasicKind kind, ClassTemplateDecl *typeDecl,
                           CXXRecordDecl *recordDecl,
                           SubscriptOperatorRecord op) {
    DXASSERT_NOMSG(typeDecl != nullptr);
    DXASSERT_NOMSG(recordDecl != nullptr);
    DXASSERT_NOMSG(0 <= op.SubscriptCardinality &&
                   op.SubscriptCardinality <= 3);
    DXASSERT(op.SubscriptCardinality > 0 ||
                 (op.HasMips == false && op.HasSample == false),
             "objects that have .mips or .sample member also have a plain "
             "subscript defined (otherwise static table is "
             "likely incorrect, and this function won't know the cardinality "
             "of the position parameter");

    bool isReadWrite = GetBasicKindProps(kind) & BPROP_RWBUFFER;
    DXASSERT(!isReadWrite || (op.HasMips == false),
             "read/write objects don't have .mips members");

    // Return early if there is no work to be done.
    if (op.SubscriptCardinality == 0) {
      return;
    }

    const unsigned int templateDepth = 1;

    // Add an operator[].
    TemplateTypeParmDecl *templateTypeParmDecl = cast<TemplateTypeParmDecl>(
        typeDecl->getTemplateParameters()->getParam(0));
    QualType resultType = m_context->getTemplateTypeParmType(
        templateDepth, 0, ParameterPackFalse, templateTypeParmDecl);
    if (!isReadWrite)
      resultType = m_context->getConstType(resultType);
    resultType = m_context->getLValueReferenceType(resultType);

    QualType indexType =
        op.SubscriptCardinality == 1
            ? m_context->UnsignedIntTy
            : NewSimpleAggregateType(AR_TOBJ_VECTOR, AR_BASIC_UINT32, 0, 1,
                                     op.SubscriptCardinality);

    CXXMethodDecl *functionDecl = CreateObjectFunctionDeclarationWithParams(
        *m_context, recordDecl, resultType, ArrayRef<QualType>(indexType),
        ArrayRef<StringRef>(StringRef("index")),
        m_context->DeclarationNames.getCXXOperatorName(OO_Subscript), true,
        StorageClass::SC_None, true);
    hlsl::CreateFunctionTemplateDecl(
        *m_context, recordDecl, functionDecl,
        reinterpret_cast<NamedDecl **>(&templateTypeParmDecl), 1);
    functionDecl->addAttr(HLSLCXXOverloadAttr::CreateImplicit(*m_context));

    // Add a .mips member if necessary.
    QualType uintType = m_context->UnsignedIntTy;
    if (op.HasMips) {
      AddDoubleSubscriptSupport(typeDecl, recordDecl, "mips", resultType,
                                templateTypeParmDecl, "mips_type",
                                "mips_slice_type", "mipSlice", uintType, "pos",
                                indexType);
    }

    // Add a .sample member if necessary.
    if (op.HasSample) {
      AddDoubleSubscriptSupport(typeDecl, recordDecl, "sample", resultType,
                                templateTypeParmDecl, "sample_type",
                                "sample_slice_type", "sampleSlice", uintType,
                                "pos", indexType);
      // TODO: support operator[][](indexType, uint).
    }
  }

  static bool
  ObjectTypeDeclMapTypeCmp(const std::pair<CXXRecordDecl *, unsigned> &a,
                           const std::pair<CXXRecordDecl *, unsigned> &b) {
    return a.first < b.first;
  };

  int FindObjectBasicKindIndex(const CXXRecordDecl *recordDecl) {
    auto begin = m_objectTypeDeclsMap.begin();
    auto end = m_objectTypeDeclsMap.end();
    auto val = std::make_pair(const_cast<CXXRecordDecl *>(recordDecl), 0);
    auto low = std::lower_bound(begin, end, val, ObjectTypeDeclMapTypeCmp);
    if (low == end)
      return -1;
    if (recordDecl == low->first)
      return low->second;
    else
      return -1;
  }

  SmallVector<NamedDecl *, 1> CreateTemplateTypeParmDeclsForIntrinsicFunction(
      const HLSL_INTRINSIC *intrinsic, NamespaceDecl *nsDecl) {
    SmallVector<NamedDecl *, 1> templateTypeParmDecls;
    auto &context = m_sema->getASTContext();
    const HLSL_INTRINSIC_ARGUMENT *pArgs = intrinsic->pArgs;
    UINT uNumArgs = intrinsic->uNumArgs;
    TypeSourceInfo *TInfo = nullptr;
    for (UINT i = 0; i < uNumArgs; ++i) {
      if (pArgs[i].uTemplateId == INTRIN_TEMPLATE_FROM_FUNCTION ||
          pArgs[i].uLegalTemplates == LITEMPLATE_ANY) {
        IdentifierInfo *id = &context.Idents.get("T");
        TemplateTypeParmDecl *templateTypeParmDecl =
            TemplateTypeParmDecl::Create(context, nsDecl, NoLoc, NoLoc, 0, 0,
                                         id, TypenameTrue, ParameterPackFalse);
        if (TInfo == nullptr) {
          TInfo = m_sema->getASTContext().CreateTypeSourceInfo(
              m_context->UnsignedIntTy, 0);
        }
        templateTypeParmDecl->setDefaultArgument(TInfo);
        templateTypeParmDecls.push_back(templateTypeParmDecl);
        continue;
      }
      if (pArgs[i].uTemplateId == INTRIN_TEMPLATE_FROM_FUNCTION_2) {
        if (TInfo == nullptr) {
          TInfo = m_sema->getASTContext().CreateTypeSourceInfo(
              m_context->UnsignedIntTy, 0);
        }
        IdentifierInfo *idT = &context.Idents.get("T");
        IdentifierInfo *idA = &context.Idents.get("A");
        TemplateTypeParmDecl *templateTypeParmDecl =
            TemplateTypeParmDecl::Create(context, m_vkNSDecl, NoLoc, NoLoc, 0,
                                         0, idT, TypenameTrue,
                                         ParameterPackFalse);
        NonTypeTemplateParmDecl *nonTypeTemplateParmDecl =
            NonTypeTemplateParmDecl::Create(context, m_vkNSDecl, NoLoc, NoLoc,
                                            0, 1, idA, context.UnsignedIntTy,
                                            ParameterPackFalse, TInfo);
        templateTypeParmDecl->setDefaultArgument(TInfo);
        templateTypeParmDecls.push_back(templateTypeParmDecl);
        templateTypeParmDecls.push_back(nonTypeTemplateParmDecl);
      }
    }
    return templateTypeParmDecls;
  }

  SmallVector<ParmVarDecl *, g_MaxIntrinsicParamCount>
  CreateParmDeclsForIntrinsicFunction(
      const HLSL_INTRINSIC *intrinsic,
      const SmallVectorImpl<QualType> &paramTypes,
      const SmallVectorImpl<ParameterModifier> &paramMods) {
    auto &context = m_sema->getASTContext();
    SmallVector<ParmVarDecl *, g_MaxIntrinsicParamCount> paramDecls;
    const HLSL_INTRINSIC_ARGUMENT *pArgs = intrinsic->pArgs;
    UINT uNumArgs = intrinsic->uNumArgs;
    for (UINT i = 1, numVariadicArgs = 0; i < uNumArgs; ++i) {
      if (IsVariadicArgument(pArgs[i])) {
        ++numVariadicArgs;
        continue;
      }
      IdentifierInfo *id = &context.Idents.get(StringRef(pArgs[i].pName));
      TypeSourceInfo *TInfo = m_sema->getASTContext().CreateTypeSourceInfo(
          paramTypes[i - numVariadicArgs], 0);
      ParmVarDecl *paramDecl = ParmVarDecl::Create(
          context, nullptr, NoLoc, NoLoc, id, paramTypes[i - numVariadicArgs],
          TInfo, StorageClass::SC_None, nullptr,
          paramMods[i - 1 - numVariadicArgs]);
      paramDecls.push_back(paramDecl);
    }
    return paramDecls;
  }

  SmallVector<QualType, 2> getIntrinsicFunctionParamTypes(
      const HLSL_INTRINSIC *intrinsic,
      const SmallVectorImpl<NamedDecl *> &templateTypeParmDecls) {
    auto &context = m_sema->getASTContext();
    const HLSL_INTRINSIC_ARGUMENT *pArgs = intrinsic->pArgs;
    UINT uNumArgs = intrinsic->uNumArgs;
    SmallVector<QualType, 2> paramTypes;
    auto templateParmItr = templateTypeParmDecls.begin();
    for (UINT i = 0; i < uNumArgs; ++i) {
      if (pArgs[i].uTemplateId == INTRIN_TEMPLATE_FROM_FUNCTION ||
          pArgs[i].uLegalTemplates == LITEMPLATE_ANY) {
        DXASSERT(templateParmItr != templateTypeParmDecls.end(),
                 "Missing TemplateTypeParmDecl for a template type parameter");
        TemplateTypeParmDecl *templateParmDecl =
            dyn_cast<TemplateTypeParmDecl>(*templateParmItr);
        DXASSERT(templateParmDecl != nullptr,
                 "TemplateTypeParmDecl is nullptr");
        paramTypes.push_back(context.getTemplateTypeParmType(
            0, i, ParameterPackFalse, templateParmDecl));
        ++templateParmItr;
        continue;
      }
      if (IsVariadicArgument(pArgs[i])) {
        continue;
      }
      switch (pArgs[i].uLegalComponentTypes) {
      case LICOMPTYPE_UINT64:
        paramTypes.push_back(context.UnsignedLongLongTy);
        break;
      case LICOMPTYPE_UINT:
        paramTypes.push_back(context.UnsignedIntTy);
        break;
      case LICOMPTYPE_VOID:
        paramTypes.push_back(context.VoidTy);
        break;
      case LICOMPTYPE_HIT_OBJECT:
        paramTypes.push_back(GetBasicKindType(AR_OBJECT_HIT_OBJECT));
        break;
#ifdef ENABLE_SPIRV_CODEGEN
      case LICOMPTYPE_VK_BUFFER_POINTER: {
        const ArBasicKind *match =
            std::find(g_ArBasicKindsAsTypes,
                      &g_ArBasicKindsAsTypes[_countof(g_ArBasicKindsAsTypes)],
                      AR_OBJECT_VK_BUFFER_POINTER);
        DXASSERT(match !=
                     &g_ArBasicKindsAsTypes[_countof(g_ArBasicKindsAsTypes)],
                 "otherwise can't find constant in basic kinds");
        size_t index = match - g_ArBasicKindsAsTypes;
        paramTypes.push_back(
            m_sema->getASTContext().getTypeDeclType(m_objectTypeDecls[index]));
        break;
      }
#endif
      default:
        DXASSERT(false, "Argument type of intrinsic function is not "
                        "supported");
        break;
      }
    }
    return paramTypes;
  }

  QualType getIntrinsicFunctionType(
      const SmallVectorImpl<QualType> &paramTypes,
      const SmallVectorImpl<ParameterModifier> &paramMods) {
    DXASSERT(!paramTypes.empty(), "Given param type vector is empty");

    ArrayRef<QualType> params({});
    if (paramTypes.size() > 1) {
      params = ArrayRef<QualType>(&paramTypes[1], paramTypes.size() - 1);
    }

    FunctionProtoType::ExtProtoInfo EmptyEPI;
    return m_sema->getASTContext().getFunctionType(paramTypes[0], params,
                                                   EmptyEPI, paramMods);
  }

  void SetParmDeclsForIntrinsicFunction(
      TypeSourceInfo *TInfo, FunctionDecl *functionDecl,
      const SmallVectorImpl<ParmVarDecl *> &paramDecls) {
    FunctionProtoTypeLoc Proto =
        TInfo->getTypeLoc().getAs<FunctionProtoTypeLoc>();

    // Attach the parameters
    for (unsigned P = 0; P < paramDecls.size(); ++P) {
      paramDecls[P]->setOwningFunction(functionDecl);
      paramDecls[P]->setScopeInfo(0, P);
      Proto.setParam(P, paramDecls[P]);
    }
    functionDecl->setParams(paramDecls);
  }

  void AddIntrinsicFunctionsToNamespace(const HLSL_INTRINSIC *table,
                                        uint32_t tableSize,
                                        NamespaceDecl *nsDecl) {
    auto &context = m_sema->getASTContext();
    for (uint32_t i = 0; i < tableSize; ++i) {
      const HLSL_INTRINSIC *intrinsic = &table[i];
      const IdentifierInfo &fnII = context.Idents.get(
          intrinsic->pArgs->pName, tok::TokenKind::identifier);
      DeclarationName functionName(&fnII);

      // Create TemplateTypeParmDecl.
      SmallVector<NamedDecl *, 1> templateTypeParmDecls =
          CreateTemplateTypeParmDeclsForIntrinsicFunction(intrinsic, nsDecl);

      // Get types for parameters.
      SmallVector<QualType, 2> paramTypes =
          getIntrinsicFunctionParamTypes(intrinsic, templateTypeParmDecls);
      SmallVector<hlsl::ParameterModifier, g_MaxIntrinsicParamCount> paramMods;
      InitParamMods(intrinsic, paramMods);

      // Create FunctionDecl.
      StorageClass SC = IsStaticMember(intrinsic) ? SC_Static : SC_Extern;
      QualType fnType = getIntrinsicFunctionType(paramTypes, paramMods);
      TypeSourceInfo *TInfo =
          m_sema->getASTContext().CreateTypeSourceInfo(fnType, 0);
      FunctionDecl *functionDecl = FunctionDecl::Create(
          context, nsDecl, NoLoc, DeclarationNameInfo(functionName, NoLoc),
          fnType, TInfo, SC, InlineSpecifiedFalse, HasWrittenPrototypeTrue);

      // Create and set ParmVarDecl.
      SmallVector<ParmVarDecl *, g_MaxIntrinsicParamCount> paramDecls =
          CreateParmDeclsForIntrinsicFunction(intrinsic, paramTypes, paramMods);
      SetParmDeclsForIntrinsicFunction(TInfo, functionDecl, paramDecls);

      if (!templateTypeParmDecls.empty()) {
        TemplateParameterList *templateParmList = TemplateParameterList::Create(
            context, NoLoc, NoLoc, templateTypeParmDecls.data(),
            templateTypeParmDecls.size(), NoLoc);
        functionDecl->setTemplateParameterListsInfo(context, 1,
                                                    &templateParmList);
        FunctionTemplateDecl *functionTemplate =
            FunctionTemplateDecl::Create(context, nsDecl, NoLoc, functionName,
                                         templateParmList, functionDecl);
        functionDecl->setDescribedFunctionTemplate(functionTemplate);
        nsDecl->addDecl(functionTemplate);
        functionTemplate->setDeclContext(nsDecl);
      } else {
        nsDecl->addDecl(functionDecl);
        functionDecl->setLexicalDeclContext(nsDecl);
        functionDecl->setDeclContext(nsDecl);
      }

      functionDecl->setImplicit(true);
    }
  }

  // Adds intrinsic function declarations to the "dx" namespace.
  // Assumes the implicit "vk" namespace has already been created.
  void AddDxIntrinsicFunctions() {
    DXASSERT(m_dxNSDecl, "caller has not created the dx namespace yet");

    AddIntrinsicFunctionsToNamespace(g_DxIntrinsics, _countof(g_DxIntrinsics),
                                     m_dxNSDecl);
    // Eagerly declare HitObject methods. This is required to make lookup of
    // 'static' HLSL member functions work without special-casing HLSL scope
    // lookup.
    CXXRecordDecl *HitObjectDecl =
        GetBasicKindType(AR_OBJECT_HIT_OBJECT)->getAsCXXRecordDecl();
    CompleteType(HitObjectDecl);
  }

#ifdef ENABLE_SPIRV_CODEGEN
  // Adds intrinsic function declarations to the "vk" namespace.
  // It does so only if SPIR-V code generation is being done.
  // Assumes the implicit "vk" namespace has already been created.
  void AddVkIntrinsicFunctions() {
    // If not doing SPIR-V CodeGen, return.
    if (!m_sema->getLangOpts().SPIRV)
      return;

    DXASSERT(m_vkNSDecl, "caller has not created the vk namespace yet");

    AddIntrinsicFunctionsToNamespace(g_VkIntrinsics, _countof(g_VkIntrinsics),
                                     m_vkNSDecl);
  }

  // Adds implicitly defined Vulkan-specific constants to the "vk" namespace.
  // It does so only if SPIR-V code generation is being done.
  // Assumes the implicit "vk" namespace has already been created.
  void AddVkIntrinsicConstants() {
    // If not doing SPIR-V CodeGen, return.
    if (!m_sema->getLangOpts().SPIRV)
      return;

    DXASSERT(m_vkNSDecl, "caller has not created the vk namespace yet");

    for (auto intConst : GetVkIntegerConstants()) {
      const llvm::StringRef name = intConst.first;
      const uint32_t value = intConst.second;
      auto &context = m_sema->getASTContext();
      QualType type = context.getConstType(context.UnsignedIntTy);
      IdentifierInfo &Id = context.Idents.get(name, tok::TokenKind::identifier);
      VarDecl *varDecl =
          VarDecl::Create(context, m_vkNSDecl, NoLoc, NoLoc, &Id, type,
                          context.getTrivialTypeSourceInfo(type),
                          clang::StorageClass::SC_Static);
      Expr *exprVal = IntegerLiteral::Create(
          context, llvm::APInt(context.getIntWidth(type), value), type, NoLoc);
      varDecl->setInit(exprVal);
      varDecl->setImplicit(true);
      m_vkNSDecl->addDecl(varDecl);
    }
  }
#endif // ENABLE_SPIRV_CODEGEN

  // Adds all built-in HLSL object types.
  void AddObjectTypes() {
    DXASSERT(m_context != nullptr,
             "otherwise caller hasn't initialized context yet");

    QualType float4Type = LookupVectorType(HLSLScalarType_float, 4);
    TypeSourceInfo *float4TypeSourceInfo =
        m_context->getTrivialTypeSourceInfo(float4Type, NoLoc);
    unsigned effectKindIndex = 0;
    const auto *SM =
        hlsl::ShaderModel::GetByName(m_sema->getLangOpts().HLSLProfile.c_str());
    CXXRecordDecl *nodeOutputDecl = nullptr, *emptyNodeOutputDecl = nullptr;

    for (unsigned i = 0; i < _countof(g_ArBasicKindsAsTypes); i++) {
      ArBasicKind kind = g_ArBasicKindsAsTypes[i];
      if (kind == AR_OBJECT_WAVE) { // wave objects are currently unused
        continue;
      }
      if (kind == AR_OBJECT_LEGACY_EFFECT)
        effectKindIndex = i;

      InheritableAttr *Attr = nullptr;
      if (IS_BASIC_STREAM(kind))
        Attr = HLSLStreamOutputAttr::CreateImplicit(
            *m_context, kind - AR_OBJECT_POINTSTREAM + 1);
      else if (IS_BASIC_PATCH(kind))
        Attr = HLSLTessPatchAttr::CreateImplicit(*m_context,
                                                 kind == AR_OBJECT_INPUTPATCH);
      else {
        DXIL::ResourceKind ResKind = DXIL::ResourceKind::NumEntries;
        DXIL::ResourceClass ResClass = DXIL::ResourceClass::Invalid;
        if (GetBasicKindResourceKindAndClass(kind, ResKind, ResClass))
          Attr = HLSLResourceAttr::CreateImplicit(*m_context, (unsigned)ResKind,
                                                  (unsigned)ResClass);
      }
      DXASSERT(kind < _countof(g_ArBasicTypeNames),
               "g_ArBasicTypeNames has the wrong number of entries");
      assert(kind < _countof(g_ArBasicTypeNames));
      const char *typeName = g_ArBasicTypeNames[kind];
      uint8_t templateArgCount = g_ArBasicKindsTemplateCount[i];
      CXXRecordDecl *recordDecl = nullptr;
      if (kind == AR_OBJECT_RAY_DESC) {
        QualType float3Ty =
            LookupVectorType(HLSLScalarType::HLSLScalarType_float, 3);
        recordDecl = CreateRayDescStruct(*m_context, float3Ty);
      } else if (kind == AR_OBJECT_TRIANGLE_INTERSECTION_ATTRIBUTES) {
        QualType float2Type =
            LookupVectorType(HLSLScalarType::HLSLScalarType_float, 2);
        recordDecl =
            AddBuiltInTriangleIntersectionAttributes(*m_context, float2Type);
      } else if (IsSubobjectBasicKind(kind)) {
        switch (kind) {
        case AR_OBJECT_STATE_OBJECT_CONFIG:
          recordDecl = CreateSubobjectStateObjectConfig(*m_context);
          break;
        case AR_OBJECT_GLOBAL_ROOT_SIGNATURE:
          recordDecl = CreateSubobjectRootSignature(*m_context, true);
          break;
        case AR_OBJECT_LOCAL_ROOT_SIGNATURE:
          recordDecl = CreateSubobjectRootSignature(*m_context, false);
          break;
        case AR_OBJECT_SUBOBJECT_TO_EXPORTS_ASSOC:
          recordDecl = CreateSubobjectSubobjectToExportsAssoc(*m_context);
          break;
        case AR_OBJECT_RAYTRACING_SHADER_CONFIG:
          recordDecl = CreateSubobjectRaytracingShaderConfig(*m_context);
          break;
        case AR_OBJECT_RAYTRACING_PIPELINE_CONFIG:
          recordDecl = CreateSubobjectRaytracingPipelineConfig(*m_context);
          break;
        case AR_OBJECT_TRIANGLE_HIT_GROUP:
          recordDecl = CreateSubobjectTriangleHitGroup(*m_context);
          break;
        case AR_OBJECT_PROCEDURAL_PRIMITIVE_HIT_GROUP:
          recordDecl = CreateSubobjectProceduralPrimitiveHitGroup(*m_context);
          break;
        case AR_OBJECT_RAYTRACING_PIPELINE_CONFIG1:
          recordDecl = CreateSubobjectRaytracingPipelineConfig1(*m_context);
          break;
        }
      } else if (kind == AR_OBJECT_CONSTANT_BUFFER) {
        recordDecl = DeclareConstantBufferViewType(*m_context, Attr);
      } else if (kind == AR_OBJECT_TEXTURE_BUFFER) {
        recordDecl = DeclareConstantBufferViewType(*m_context, Attr);
      } else if (kind == AR_OBJECT_RAY_QUERY) {
        recordDecl = DeclareRayQueryType(*m_context);
      } else if (kind == AR_OBJECT_HIT_OBJECT) {
        // Declare 'HitObject' in '::dx' extension namespace.
        DXASSERT(m_dxNSDecl, "namespace ::dx must be declared in SM6.9+");
        recordDecl = DeclareHitObjectType(*m_dxNSDecl);
      } else if (kind == AR_OBJECT_HEAP_RESOURCE) {
        recordDecl = DeclareResourceType(*m_context, /*bSampler*/ false);
        if (SM->IsSM66Plus()) {
          // create Resource ResourceDescriptorHeap;
          DeclareBuiltinGlobal("ResourceDescriptorHeap",
                               m_context->getRecordType(recordDecl),
                               *m_context);
        }
      } else if (kind == AR_OBJECT_HEAP_SAMPLER) {
        recordDecl = DeclareResourceType(*m_context, /*bSampler*/ true);
        if (SM->IsSM66Plus()) {
          // create Resource SamplerDescriptorHeap;
          DeclareBuiltinGlobal("SamplerDescriptorHeap",
                               m_context->getRecordType(recordDecl),
                               *m_context);
        }
      } else if (kind == AR_OBJECT_FEEDBACKTEXTURE2D) {
        recordDecl = DeclareUIntTemplatedTypeWithHandle(
            *m_context, "FeedbackTexture2D", "kind", Attr);
      } else if (kind == AR_OBJECT_FEEDBACKTEXTURE2D_ARRAY) {
        recordDecl = DeclareUIntTemplatedTypeWithHandle(
            *m_context, "FeedbackTexture2DArray", "kind", Attr);
      } else if (kind == AR_OBJECT_EMPTY_NODE_INPUT) {
        recordDecl = DeclareNodeOrRecordType(
            *m_context, DXIL::NodeIOKind::EmptyInput,
            /*IsRecordTypeTemplate*/ false, /*IsConst*/ true,
            /*HasGetMethods*/ false,
            /*IsArray*/ false, /*IsCompleteType*/ false);
      } else if (kind == AR_OBJECT_DISPATCH_NODE_INPUT_RECORD) {
        recordDecl = DeclareNodeOrRecordType(
            *m_context, DXIL::NodeIOKind::DispatchNodeInputRecord,
            /*IsRecordTypeTemplate*/ true,
            /*IsConst*/ true, /*HasGetMethods*/ true,
            /*IsArray*/ false, /*IsCompleteType*/ true);
      } else if (kind == AR_OBJECT_RWDISPATCH_NODE_INPUT_RECORD) {
        recordDecl = DeclareNodeOrRecordType(
            *m_context, DXIL::NodeIOKind::RWDispatchNodeInputRecord,
            /*IsRecordTypeTemplate*/ true, /*IsConst*/ false,
            /*HasGetMethods*/ true,
            /*IsArray*/ false, /*IsCompleteType*/ false);
      } else if (kind == AR_OBJECT_GROUP_NODE_INPUT_RECORDS) {
        recordDecl = DeclareNodeOrRecordType(
            *m_context, DXIL::NodeIOKind::GroupNodeInputRecords,
            /*IsRecordTypeTemplate*/ true,
            /*IsConst*/ true, /*HasGetMethods*/ true,
            /*IsArray*/ true, /*IsCompleteType*/ false);
      } else if (kind == AR_OBJECT_RWGROUP_NODE_INPUT_RECORDS) {
        recordDecl = DeclareNodeOrRecordType(
            *m_context, DXIL::NodeIOKind::RWGroupNodeInputRecords,
            /*IsRecordTypeTemplate*/ true,
            /*IsConst*/ false, /*HasGetMethods*/ true,
            /*IsArray*/ true, /*IsCompleteType*/ false);
      } else if (kind == AR_OBJECT_THREAD_NODE_INPUT_RECORD) {
        recordDecl = DeclareNodeOrRecordType(
            *m_context, DXIL::NodeIOKind::ThreadNodeInputRecord,
            /*IsRecordTypeTemplate*/ true,
            /*IsConst*/ true, /*HasGetMethods*/ true,
            /*IsArray*/ false, /*IsCompleteType*/ true);
      } else if (kind == AR_OBJECT_RWTHREAD_NODE_INPUT_RECORD) {
        recordDecl = DeclareNodeOrRecordType(
            *m_context, DXIL::NodeIOKind::RWThreadNodeInputRecord,
            /*IsRecordTypeTemplate*/ true,
            /*IsConst*/ false, /*HasGetMethods*/ true,
            /*IsArray*/ false, /*IsCompleteType*/ true);
      } else if (kind == AR_OBJECT_NODE_OUTPUT) {
        recordDecl = DeclareNodeOrRecordType(
            *m_context, DXIL::NodeIOKind::NodeOutput,
            /*IsRecordTypeTemplate*/ true, /*IsConst*/ true,
            /*HasGetMethods*/ false,
            /*IsArray*/ false, /*IsCompleteType*/ false);
        nodeOutputDecl = recordDecl;
      } else if (kind == AR_OBJECT_EMPTY_NODE_OUTPUT) {
        recordDecl = DeclareNodeOrRecordType(
            *m_context, DXIL::NodeIOKind::EmptyOutput,
            /*IsRecordTypeTemplate*/ false, /*IsConst*/ true,
            /*HasGetMethods*/ false,
            /*IsArray*/ false, /*IsCompleteType*/ false);
        emptyNodeOutputDecl = recordDecl;
      } else if (kind == AR_OBJECT_NODE_OUTPUT_ARRAY) {
        assert(nodeOutputDecl != nullptr);
        recordDecl = DeclareNodeOutputArray(*m_context,
                                            DXIL::NodeIOKind::NodeOutputArray,
                                            /* ItemType */ nodeOutputDecl,
                                            /*IsRecordTypeTemplate*/ true);
      } else if (kind == AR_OBJECT_EMPTY_NODE_OUTPUT_ARRAY) {
        assert(emptyNodeOutputDecl != nullptr);
        recordDecl = DeclareNodeOutputArray(*m_context,
                                            DXIL::NodeIOKind::EmptyOutputArray,
                                            /* ItemType */ emptyNodeOutputDecl,
                                            /*IsRecordTypeTemplate*/ false);
      } else if (kind == AR_OBJECT_GROUP_NODE_OUTPUT_RECORDS) {
        recordDecl = m_GroupNodeOutputRecordsTemplateDecl->getTemplatedDecl();
      } else if (kind == AR_OBJECT_THREAD_NODE_OUTPUT_RECORDS) {
        recordDecl = m_ThreadNodeOutputRecordsTemplateDecl->getTemplatedDecl();
      }
#ifdef ENABLE_SPIRV_CODEGEN
      else if (kind == AR_OBJECT_VK_SPIRV_TYPE) {
        if (!m_vkNSDecl)
          continue;
        recordDecl =
            DeclareInlineSpirvType(*m_context, m_vkNSDecl, typeName, false);
        recordDecl->setImplicit(true);
      } else if (kind == AR_OBJECT_VK_SPIRV_OPAQUE_TYPE) {
        if (!m_vkNSDecl)
          continue;
        recordDecl =
            DeclareInlineSpirvType(*m_context, m_vkNSDecl, typeName, true);
        recordDecl->setImplicit(true);
      } else if (kind == AR_OBJECT_VK_INTEGRAL_CONSTANT) {
        if (!m_vkNSDecl)
          continue;
        recordDecl =
            DeclareVkIntegralConstant(*m_context, m_vkNSDecl, typeName,
                                      &m_vkIntegralConstantTemplateDecl);
        recordDecl->setImplicit(true);
      } else if (kind == AR_OBJECT_VK_LITERAL) {
        if (!m_vkNSDecl)
          continue;
        recordDecl = DeclareTemplateTypeWithHandleInDeclContext(
            *m_context, m_vkNSDecl, typeName, 1, nullptr);
        recordDecl->setImplicit(true);
        m_vkLiteralTemplateDecl = recordDecl->getDescribedClassTemplate();
      } else if (kind == AR_OBJECT_VK_SPV_INTRINSIC_TYPE) {
        if (!m_vkNSDecl)
          continue;
        recordDecl = DeclareUIntTemplatedTypeWithHandleInDeclContext(
            *m_context, m_vkNSDecl, typeName, "id");
        recordDecl->setImplicit(true);
      } else if (kind == AR_OBJECT_VK_SPV_INTRINSIC_RESULT_ID) {
        if (!m_vkNSDecl)
          continue;
        recordDecl = DeclareTemplateTypeWithHandleInDeclContext(
            *m_context, m_vkNSDecl, typeName, 1, nullptr);
        recordDecl->setImplicit(true);
      } else if (kind == AR_OBJECT_VK_BUFFER_POINTER) {
        if (!m_vkNSDecl)
          continue;
        recordDecl = DeclareVkBufferPointerType(*m_context, m_vkNSDecl);
        recordDecl->setImplicit(true);
        m_vkBufferPointerTemplateDecl = recordDecl->getDescribedClassTemplate();
      }
#endif
      else if (templateArgCount == 0) {
        recordDecl =
            DeclareRecordTypeWithHandle(*m_context, typeName,
                                        /*isCompleteType*/ false, Attr);
      } else {
        DXASSERT(templateArgCount == 1 || templateArgCount == 2,
                 "otherwise a new case has been added");
        TypeSourceInfo *typeDefault =
            TemplateHasDefaultType(kind) ? float4TypeSourceInfo : nullptr;
        recordDecl = DeclareTemplateTypeWithHandle(
            *m_context, typeName, templateArgCount, typeDefault, Attr);
      }
      m_objectTypeDecls[i] = recordDecl;
      m_objectTypeDeclsMap[i] = std::make_pair(recordDecl, i);
    }

    // Create an alias for SamplerState. 'sampler' is very commonly used.
    {
      DeclContext *currentDeclContext = m_context->getTranslationUnitDecl();
      IdentifierInfo &samplerId = m_context->Idents.get(
          StringRef("sampler"), tok::TokenKind::identifier);
      TypeSourceInfo *samplerTypeSource = m_context->getTrivialTypeSourceInfo(
          GetBasicKindType(AR_OBJECT_SAMPLER));
      TypedefDecl *samplerDecl =
          TypedefDecl::Create(*m_context, currentDeclContext, NoLoc, NoLoc,
                              &samplerId, samplerTypeSource);
      currentDeclContext->addDecl(samplerDecl);
      samplerDecl->setImplicit(true);

      // Create decls for each deprecated effect object type:
      unsigned effectObjBase = _countof(g_ArBasicKindsAsTypes);
      // TypeSourceInfo* effectObjTypeSource =
      // m_context->getTrivialTypeSourceInfo(GetBasicKindType(AR_OBJECT_LEGACY_EFFECT));
      for (unsigned i = 0; i < _countof(g_DeprecatedEffectObjectNames); i++) {
        IdentifierInfo &idInfo =
            m_context->Idents.get(StringRef(g_DeprecatedEffectObjectNames[i]),
                                  tok::TokenKind::identifier);
        // TypedefDecl* effectObjDecl = TypedefDecl::Create(*m_context,
        // currentDeclContext, NoLoc, NoLoc, &idInfo, effectObjTypeSource);
        CXXRecordDecl *effectObjDecl =
            CXXRecordDecl::Create(*m_context, TagTypeKind::TTK_Struct,
                                  currentDeclContext, NoLoc, NoLoc, &idInfo);
        currentDeclContext->addDecl(effectObjDecl);
        effectObjDecl->setImplicit(true);
        m_objectTypeDeclsMap[i + effectObjBase] =
            std::make_pair(effectObjDecl, effectKindIndex);
      }
    }

    // Make sure it's in order.
    std::sort(m_objectTypeDeclsMap.begin(), m_objectTypeDeclsMap.end(),
              ObjectTypeDeclMapTypeCmp);
  }

  FunctionDecl *
  AddSubscriptSpecialization(FunctionTemplateDecl *functionTemplate,
                             QualType objectElement,
                             const FindStructBasicTypeResult &findResult);

  ImplicitCastExpr *CreateLValueToRValueCast(Expr *input) {
    return ImplicitCastExpr::Create(*m_context, input->getType(),
                                    CK_LValueToRValue, input, nullptr,
                                    VK_RValue);
  }
  ImplicitCastExpr *CreateFlatConversionCast(Expr *input) {
    return ImplicitCastExpr::Create(*m_context, input->getType(),
                                    CK_LValueToRValue, input, nullptr,
                                    VK_RValue);
  }

  static TYPE_CONVERSION_REMARKS RemarksUnused;
  static ImplicitConversionKind ImplicitConversionKindUnused;
  HRESULT CombineDimensions(
      QualType leftType, QualType rightType, QualType *resultType,
      ImplicitConversionKind &convKind = ImplicitConversionKindUnused,
      TYPE_CONVERSION_REMARKS &Remarks = RemarksUnused);

  clang::TypedefDecl *LookupMatrixShorthandType(HLSLScalarType scalarType,
                                                UINT rowCount, UINT colCount) {
    DXASSERT_NOMSG(scalarType != HLSLScalarType::HLSLScalarType_unknown &&
                   rowCount <= 4 && colCount <= 4);
    TypedefDecl *qts =
        m_matrixShorthandTypes[scalarType][rowCount - 1][colCount - 1];
    if (qts == nullptr) {
      QualType type = LookupMatrixType(scalarType, rowCount, colCount);
      qts = CreateMatrixSpecializationShorthand(*m_context, type, scalarType,
                                                rowCount, colCount);
      m_matrixShorthandTypes[scalarType][rowCount - 1][colCount - 1] = qts;
    }
    return qts;
  }

  clang::TypedefDecl *LookupVectorShorthandType(HLSLScalarType scalarType,
                                                UINT colCount) {
    DXASSERT_NOMSG(scalarType != HLSLScalarType::HLSLScalarType_unknown &&
                   colCount <= DXIL::kDefaultMaxVectorLength);
    TypedefDecl *qts = m_vectorTypedefs[scalarType][colCount - 1];
    if (qts == nullptr) {
      QualType type = LookupVectorType(scalarType, colCount);
      qts = CreateVectorSpecializationShorthand(*m_context, type, scalarType,
                                                colCount);
      m_vectorTypedefs[scalarType][colCount - 1] = qts;
    }
    return qts;
  }

public:
  HLSLExternalSource()
      : m_matrixTemplateDecl(nullptr), m_vectorTemplateDecl(nullptr),
        m_vkIntegralConstantTemplateDecl(nullptr),
        m_vkLiteralTemplateDecl(nullptr),
        m_vkBufferPointerTemplateDecl(nullptr), m_hlslNSDecl(nullptr),
        m_vkNSDecl(nullptr), m_dxNSDecl(nullptr), m_context(nullptr),
        m_sema(nullptr), m_hlslStringTypedef(nullptr) {
    memset(m_matrixTypes, 0, sizeof(m_matrixTypes));
    memset(m_matrixShorthandTypes, 0, sizeof(m_matrixShorthandTypes));
    memset(m_vectorTypes, 0, sizeof(m_vectorTypes));
    memset(m_vectorTypedefs, 0, sizeof(m_vectorTypedefs));
    memset(m_scalarTypes, 0, sizeof(m_scalarTypes));
    memset(m_scalarTypeDefs, 0, sizeof(m_scalarTypeDefs));
    memset(m_baseTypes, 0, sizeof(m_baseTypes));
  }

  ~HLSLExternalSource() {}

  static HLSLExternalSource *FromSema(Sema *self) {
    DXASSERT_NOMSG(self != nullptr);

    ExternalSemaSource *externalSource = self->getExternalSource();
    DXASSERT(externalSource != nullptr,
             "otherwise caller shouldn't call HLSL-specific function");

    HLSLExternalSource *hlsl =
        reinterpret_cast<HLSLExternalSource *>(externalSource);
    return hlsl;
  }

  void InitializeSema(Sema &S) override {
    auto &context = S.getASTContext();
    m_sema = &S;
    S.addExternalSource(this);

    m_dxNSDecl =
        NamespaceDecl::Create(context, context.getTranslationUnitDecl(),
                              /*Inline*/ false, SourceLocation(),
                              SourceLocation(), &context.Idents.get("dx"),
                              /*PrevDecl*/ nullptr);
    m_dxNSDecl->setImplicit();
    m_dxNSDecl->setHasExternalLexicalStorage(true);
    context.getTranslationUnitDecl()->addDecl(m_dxNSDecl);

#ifdef ENABLE_SPIRV_CODEGEN
    if (m_sema->getLangOpts().SPIRV) {
      // Create the "vk" namespace which contains Vulkan-specific intrinsics.
      m_vkNSDecl =
          NamespaceDecl::Create(context, context.getTranslationUnitDecl(),
                                /*Inline*/ false, SourceLocation(),
                                SourceLocation(), &context.Idents.get("vk"),
                                /*PrevDecl*/ nullptr);
      context.getTranslationUnitDecl()->addDecl(m_vkNSDecl);
    }
#endif // ENABLE_SPIRV_CODEGEN

    AddObjectTypes();
    AddStdIsEqualImplementation(context, S);
    for (auto &&intrinsic : m_intrinsicTables) {
      AddIntrinsicTableMethods(intrinsic);
    }

    AddDxIntrinsicFunctions();

#ifdef ENABLE_SPIRV_CODEGEN
    if (m_sema->getLangOpts().SPIRV) {
      // Add Vulkan-specific intrinsics.
      AddVkIntrinsicFunctions();
      AddVkIntrinsicConstants();
    }
#endif // ENABLE_SPIRV_CODEGEN
  }

  void ForgetSema() override { m_sema = nullptr; }

  Sema *getSema() { return m_sema; }

  TypedefDecl *LookupScalarTypeDef(HLSLScalarType scalarType) {
    // We shouldn't create Typedef for built in scalar types.
    // For built in scalar types, this funciton may be called for
    // TypoCorrection. In that case, we return a nullptr.
    if (m_scalarTypes[scalarType].isNull()) {
      m_scalarTypeDefs[scalarType] = CreateGlobalTypedef(
          m_context, HLSLScalarTypeNames[scalarType], m_baseTypes[scalarType]);
      m_scalarTypes[scalarType] =
          m_context->getTypeDeclType(m_scalarTypeDefs[scalarType]);
    }
    return m_scalarTypeDefs[scalarType];
  }

  QualType LookupMatrixType(HLSLScalarType scalarType, unsigned int rowCount,
                            unsigned int colCount) {
    QualType qt = m_matrixTypes[scalarType][rowCount - 1][colCount - 1];
    if (qt.isNull()) {
      // lazy initialization of scalar types
      if (m_scalarTypes[scalarType].isNull()) {
        LookupScalarTypeDef(scalarType);
      }
      qt = GetOrCreateMatrixSpecialization(
          *m_context, m_sema, m_matrixTemplateDecl, m_scalarTypes[scalarType],
          rowCount, colCount);
      m_matrixTypes[scalarType][rowCount - 1][colCount - 1] = qt;
    }
    return qt;
  }

  QualType LookupVectorType(HLSLScalarType scalarType, unsigned int colCount) {
    QualType qt;
    if (colCount < DXIL::kDefaultMaxVectorLength)
      qt = m_vectorTypes[scalarType][colCount - 1];
    if (qt.isNull()) {
      if (m_scalarTypes[scalarType].isNull()) {
        LookupScalarTypeDef(scalarType);
      }
      qt = GetOrCreateVectorSpecialization(*m_context, m_sema,
                                           m_vectorTemplateDecl,
                                           m_scalarTypes[scalarType], colCount);
      if (colCount < DXIL::kDefaultMaxVectorLength)
        m_vectorTypes[scalarType][colCount - 1] = qt;
    }
    return qt;
  }

  TypedefDecl *GetStringTypedef() {
    if (m_hlslStringTypedef == nullptr) {
      m_hlslStringTypedef =
          CreateGlobalTypedef(m_context, "string", m_hlslStringType);
      m_hlslStringType = m_context->getTypeDeclType(m_hlslStringTypedef);
    }
    DXASSERT_NOMSG(m_hlslStringTypedef != nullptr);
    return m_hlslStringTypedef;
  }

  static bool IsSubobjectBasicKind(ArBasicKind kind) {
    return kind >= AR_OBJECT_STATE_OBJECT_CONFIG &&
           kind <= AR_OBJECT_RAYTRACING_PIPELINE_CONFIG1;
  }

  bool IsSubobjectType(QualType type) {
    return IsSubobjectBasicKind(GetTypeElementKind(type));
  }

  void WarnMinPrecision(QualType Type, SourceLocation Loc) {
    Type = Type->getCanonicalTypeUnqualified();
    if (IsVectorType(m_sema, Type) || IsMatrixType(m_sema, Type)) {
      Type = GetOriginalMatrixOrVectorElementType(Type);
    }
    // TODO: enalbe this once we introduce precise master option
    bool UseMinPrecision = m_context->getLangOpts().UseMinPrecision;
    if (Type == m_context->Min12IntTy) {
      QualType PromotedType =
          UseMinPrecision ? m_context->Min16IntTy : m_context->ShortTy;
      m_sema->Diag(Loc, diag::warn_hlsl_sema_minprecision_promotion)
          << Type << PromotedType;
    } else if (Type == m_context->Min10FloatTy) {
      QualType PromotedType =
          UseMinPrecision ? m_context->Min16FloatTy : m_context->HalfTy;
      m_sema->Diag(Loc, diag::warn_hlsl_sema_minprecision_promotion)
          << Type << PromotedType;
    }
    if (!UseMinPrecision) {
      if (Type == m_context->Min16FloatTy) {
        m_sema->Diag(Loc, diag::warn_hlsl_sema_minprecision_promotion)
            << Type << m_context->HalfTy;
      } else if (Type == m_context->Min16IntTy) {
        m_sema->Diag(Loc, diag::warn_hlsl_sema_minprecision_promotion)
            << Type << m_context->ShortTy;
      } else if (Type == m_context->Min16UIntTy) {
        m_sema->Diag(Loc, diag::warn_hlsl_sema_minprecision_promotion)
            << Type << m_context->UnsignedShortTy;
      }
    }
  }

  bool DiagnoseHLSLScalarType(HLSLScalarType type, SourceLocation Loc) {
    if (getSema()->getLangOpts().HLSLVersion < hlsl::LangStd::v2018) {
      switch (type) {
      case HLSLScalarType_float16:
      case HLSLScalarType_float32:
      case HLSLScalarType_float64:
      case HLSLScalarType_int16:
      case HLSLScalarType_int32:
      case HLSLScalarType_uint16:
      case HLSLScalarType_uint32:
        m_sema->Diag(Loc, diag::err_hlsl_unsupported_keyword_for_version)
            << HLSLScalarTypeNames[type] << "2018";
        return false;
      default:
        break;
      }
    }
    if (getSema()->getLangOpts().UseMinPrecision) {
      switch (type) {
      case HLSLScalarType_float16:
      case HLSLScalarType_int16:
      case HLSLScalarType_uint16:
        m_sema->Diag(Loc, diag::err_hlsl_unsupported_keyword_for_min_precision)
            << HLSLScalarTypeNames[type];
        return false;
      default:
        break;
      }
    }
    return true;
  }

  bool LookupUnqualified(LookupResult &R, Scope *S) override {
    const DeclarationNameInfo declName = R.getLookupNameInfo();
    IdentifierInfo *idInfo = declName.getName().getAsIdentifierInfo();
    if (idInfo == nullptr) {
      return false;
    }

    // Currently template instantiation is blocked when a fatal error is
    // detected. So no faulting-in types at this point, instead we simply
    // back out.
    if (this->m_sema->Diags.hasFatalErrorOccurred()) {
      return false;
    }

    StringRef nameIdentifier = idInfo->getName();
    HLSLScalarType parsedType;
    int rowCount;
    int colCount;

    // Try parsing hlsl scalar types that is not initialized at AST time.
    if (TryParseAny(nameIdentifier.data(), nameIdentifier.size(), &parsedType,
                    &rowCount, &colCount, getSema()->getLangOpts())) {
      assert(parsedType != HLSLScalarType_unknown &&
             "otherwise, TryParseHLSLScalarType should not have succeeded.");
      if (rowCount == 0 && colCount == 0) { // scalar
        if (!DiagnoseHLSLScalarType(parsedType, R.getNameLoc()))
          return false;
        TypedefDecl *typeDecl = LookupScalarTypeDef(parsedType);
        if (!typeDecl)
          return false;
        R.addDecl(typeDecl);
      } else if (rowCount == 0) { // vector
        TypedefDecl *qts = LookupVectorShorthandType(parsedType, colCount);
        R.addDecl(qts);
      } else { // matrix
        TypedefDecl *qts =
            LookupMatrixShorthandType(parsedType, rowCount, colCount);
        R.addDecl(qts);
      }
      return true;
    }
    // string
    else if (TryParseString(nameIdentifier.data(), nameIdentifier.size(),
                            getSema()->getLangOpts())) {
      TypedefDecl *strDecl = GetStringTypedef();
      R.addDecl(strDecl);
    }
    return false;
  }

  /// <summary>
  /// Determines whether the specify record type is a matrix, another HLSL
  /// object, or a user-defined structure.
  /// </summary>
  ArTypeObjectKind ClassifyRecordType(const RecordType *type) {
    DXASSERT_NOMSG(type != nullptr);

    const CXXRecordDecl *typeRecordDecl = type->getAsCXXRecordDecl();
    const ClassTemplateSpecializationDecl *templateSpecializationDecl =
        dyn_cast<ClassTemplateSpecializationDecl>(typeRecordDecl);
    if (templateSpecializationDecl) {
      ClassTemplateDecl *decl =
          templateSpecializationDecl->getSpecializedTemplate();
      if (decl == m_matrixTemplateDecl)
        return AR_TOBJ_MATRIX;
      else if (decl == m_vectorTemplateDecl)
        return AR_TOBJ_VECTOR;
      else if (decl == m_vkIntegralConstantTemplateDecl ||
               decl == m_vkLiteralTemplateDecl)
        return AR_TOBJ_COMPOUND;
      else if (!decl->isImplicit())
        return AR_TOBJ_COMPOUND;
      return AR_TOBJ_OBJECT;
    }

    if (typeRecordDecl && typeRecordDecl->isImplicit()) {
      if (typeRecordDecl->getDeclContext()->isFileContext()) {
        int index = FindObjectBasicKindIndex(typeRecordDecl);
        if (index != -1) {
          ArBasicKind kind = g_ArBasicKindsAsTypes[index];
          if (AR_OBJECT_RAY_DESC == kind ||
              AR_OBJECT_TRIANGLE_INTERSECTION_ATTRIBUTES == kind)
            return AR_TOBJ_COMPOUND;
        }
        return AR_TOBJ_OBJECT;
      } else
        return AR_TOBJ_INNER_OBJ;
    }

    return AR_TOBJ_COMPOUND;
  }

  /// <summary>Given a Clang type, determines whether it is a built-in object
  /// type (sampler, texture, etc).</summary>
  bool IsBuiltInObjectType(QualType type) {
    type = GetStructuralForm(type);

    if (!type.isNull() && type->isStructureOrClassType()) {
      const RecordType *recordType = type->getAs<RecordType>();
      return ClassifyRecordType(recordType) == AR_TOBJ_OBJECT;
    }

    return false;
  }

  /// <summary>
  /// Given the specified type (typed a DeclContext for convenience), determines
  /// its RecordDecl, possibly refering to original template record if it's a
  /// specialization; this makes the result suitable for looking up in
  /// initialization tables.
  /// </summary>
  const CXXRecordDecl *
  GetRecordDeclForBuiltInOrStruct(const DeclContext *context) {
    const CXXRecordDecl *recordDecl;
    if (const ClassTemplateSpecializationDecl *decl =
            dyn_cast<ClassTemplateSpecializationDecl>(context)) {
      recordDecl = decl->getSpecializedTemplate()->getTemplatedDecl();
    } else {
      recordDecl = dyn_cast<CXXRecordDecl>(context);
    }

    return recordDecl;
  }

  /// <summary>Given a Clang type, return the ArTypeObjectKind classification,
  /// (eg AR_TOBJ_VECTOR).</summary>
  ArTypeObjectKind GetTypeObjectKind(QualType type) {
    DXASSERT_NOMSG(!type.isNull());

    type = GetStructuralForm(type);

    if (type->isVoidType())
      return AR_TOBJ_VOID;
    if (type->isArrayType()) {
      return hlsl::IsArrayConstantStringType(type) ? AR_TOBJ_STRING
                                                   : AR_TOBJ_ARRAY;
    }
    if (type->isPointerType()) {
      return hlsl::IsPointerStringType(type) ? AR_TOBJ_STRING : AR_TOBJ_POINTER;
    }
    if (type->isDependentType()) {
      return AR_TOBJ_DEPENDENT;
    }
    if (type->isStructureOrClassType()) {
      const RecordType *recordType = type->getAs<RecordType>();
      return ClassifyRecordType(recordType);
    } else if (const InjectedClassNameType *ClassNameTy =
                   type->getAs<InjectedClassNameType>()) {
      const CXXRecordDecl *typeRecordDecl = ClassNameTy->getDecl();
      const ClassTemplateSpecializationDecl *templateSpecializationDecl =
          dyn_cast<ClassTemplateSpecializationDecl>(typeRecordDecl);
      if (templateSpecializationDecl) {
        ClassTemplateDecl *decl =
            templateSpecializationDecl->getSpecializedTemplate();
        if (decl == m_matrixTemplateDecl)
          return AR_TOBJ_MATRIX;
        else if (decl == m_vectorTemplateDecl)
          return AR_TOBJ_VECTOR;
        DXASSERT(decl->isImplicit(),
                 "otherwise object template decl is not set to implicit");
        return AR_TOBJ_OBJECT;
      }

      if (typeRecordDecl && typeRecordDecl->isImplicit()) {
        if (typeRecordDecl->getDeclContext()->isFileContext())
          return AR_TOBJ_OBJECT;
        else
          return AR_TOBJ_INNER_OBJ;
      }

      return AR_TOBJ_COMPOUND;
    }

    if (type->isBuiltinType())
      return AR_TOBJ_BASIC;
    if (type->isEnumeralType())
      return AR_TOBJ_BASIC;

    return AR_TOBJ_INVALID;
  }

  /// <summary>Gets the element type of a matrix or vector type (eg, the 'float'
  /// in 'float4x4' or 'float4').</summary>
  QualType GetMatrixOrVectorElementType(QualType type) {
    type = GetStructuralForm(type);

    const CXXRecordDecl *typeRecordDecl = type->getAsCXXRecordDecl();
    DXASSERT_NOMSG(typeRecordDecl);
    const ClassTemplateSpecializationDecl *templateSpecializationDecl =
        dyn_cast<ClassTemplateSpecializationDecl>(typeRecordDecl);
    DXASSERT_NOMSG(templateSpecializationDecl);
    DXASSERT_NOMSG(templateSpecializationDecl->getSpecializedTemplate() ==
                       m_matrixTemplateDecl ||
                   templateSpecializationDecl->getSpecializedTemplate() ==
                       m_vectorTemplateDecl);
    return templateSpecializationDecl->getTemplateArgs().get(0).getAsType();
  }

  /// <summary>Gets the type with structural information (elements and shape)
  /// for the given type.</summary> <remarks>This function will strip
  /// lvalue/rvalue references, attributes and qualifiers.</remarks>
  QualType GetStructuralForm(QualType type) {
    if (type.isNull()) {
      return type;
    }

    const ReferenceType *RefType = nullptr;
    const AttributedType *AttrType = nullptr;
    while ((RefType = dyn_cast<ReferenceType>(type)) ||
           (AttrType = dyn_cast<AttributedType>(type))) {
      type =
          RefType ? RefType->getPointeeType() : AttrType->getEquivalentType();
    }

    // Despite its name, getCanonicalTypeUnqualified will preserve const for
    // array elements or something
    return QualType(type->getCanonicalTypeUnqualified()->getTypePtr(), 0);
  }

  /// <summary>Given a Clang type, return the QualType for its element, drilling
  /// through any array/vector/matrix.</summary>
  QualType GetTypeElementType(QualType type) {
    type = GetStructuralForm(type);
    ArTypeObjectKind kind = GetTypeObjectKind(type);
    if (kind == AR_TOBJ_MATRIX || kind == AR_TOBJ_VECTOR) {
      type = GetMatrixOrVectorElementType(type);
    } else if (kind == AR_TOBJ_STRING) {
      // return original type even if it's an array (string literal)
    } else if (type->isArrayType()) {
      const ArrayType *arrayType = type->getAsArrayTypeUnsafe();
      type = GetTypeElementType(arrayType->getElementType());
    }
    return type;
  }

  /// <summary>Given a Clang type, return the ArBasicKind classification for its
  /// contents.</summary>
  ArBasicKind GetTypeElementKind(QualType type) {
    type = GetStructuralForm(type);

    ArTypeObjectKind kind = GetTypeObjectKind(type);
    if (kind == AR_TOBJ_MATRIX || kind == AR_TOBJ_VECTOR) {
      QualType elementType = GetMatrixOrVectorElementType(type);
      return GetTypeElementKind(elementType);
    }

    if (kind == AR_TOBJ_STRING) {
      return type->isArrayType() ? AR_OBJECT_STRING_LITERAL : AR_OBJECT_STRING;
    }

    if (type->isArrayType()) {
      const ArrayType *arrayType = type->getAsArrayTypeUnsafe();
      return GetTypeElementKind(arrayType->getElementType());
    }

    if (kind == AR_TOBJ_INNER_OBJ) {
      return AR_OBJECT_INNER;
    } else if (kind == AR_TOBJ_OBJECT) {
      // Classify the object as the element type.
      const CXXRecordDecl *typeRecordDecl =
          GetRecordDeclForBuiltInOrStruct(type->getAsCXXRecordDecl());
      int index = FindObjectBasicKindIndex(typeRecordDecl);
      // NOTE: this will likely need to be updated for specialized records
      DXASSERT(index != -1,
               "otherwise can't find type we already determined was an object");
      return g_ArBasicKindsAsTypes[index];
    }

    CanQualType canType = type->getCanonicalTypeUnqualified();
    return BasicTypeForScalarType(canType);
  }

  ArBasicKind BasicTypeForScalarType(CanQualType type) {
    if (const BuiltinType *BT = dyn_cast<BuiltinType>(type)) {
      switch (BT->getKind()) {
      case BuiltinType::Bool:
        return AR_BASIC_BOOL;
      case BuiltinType::Double:
        return AR_BASIC_FLOAT64;
      case BuiltinType::Float:
        return AR_BASIC_FLOAT32;
      case BuiltinType::Half:
        return AR_BASIC_FLOAT16;
      case BuiltinType::HalfFloat:
        return AR_BASIC_FLOAT32_PARTIAL_PRECISION;
      case BuiltinType::Int:
        return AR_BASIC_INT32;
      case BuiltinType::UInt:
        return AR_BASIC_UINT32;
      case BuiltinType::Short:
        return AR_BASIC_INT16;
      case BuiltinType::UShort:
        return AR_BASIC_UINT16;
      case BuiltinType::Long:
        return AR_BASIC_INT32;
      case BuiltinType::ULong:
        return AR_BASIC_UINT32;
      case BuiltinType::LongLong:
        return AR_BASIC_INT64;
      case BuiltinType::ULongLong:
        return AR_BASIC_UINT64;
      case BuiltinType::Min12Int:
        return AR_BASIC_MIN12INT;
      case BuiltinType::Min16Float:
        return AR_BASIC_MIN16FLOAT;
      case BuiltinType::Min16Int:
        return AR_BASIC_MIN16INT;
      case BuiltinType::Min16UInt:
        return AR_BASIC_MIN16UINT;
      case BuiltinType::Min10Float:
        return AR_BASIC_MIN10FLOAT;
      case BuiltinType::LitFloat:
        return AR_BASIC_LITERAL_FLOAT;
      case BuiltinType::LitInt:
        return AR_BASIC_LITERAL_INT;
      case BuiltinType::Int8_4Packed:
        return AR_BASIC_INT8_4PACKED;
      case BuiltinType::UInt8_4Packed:
        return AR_BASIC_UINT8_4PACKED;
      case BuiltinType::Dependent:
        return AR_BASIC_DEPENDENT;
      default:
        // Only builtin types that have basickind equivalents.
        break;
      }
    }
    if (const EnumType *ET = dyn_cast<EnumType>(type)) {
      if (ET->getDecl()->isScopedUsingClassTag())
        return AR_BASIC_ENUM_CLASS;
      return AR_BASIC_ENUM;
    }
    return AR_BASIC_UNKNOWN;
  }

  void AddIntrinsicTableMethods(IDxcIntrinsicTable *table) {
    DXASSERT_NOMSG(table != nullptr);

    // Function intrinsics are added on-demand, objects get template methods.
    for (unsigned i = 0; i < _countof(g_ArBasicKindsAsTypes); i++) {
      // Grab information already processed by AddObjectTypes.
      ArBasicKind kind = g_ArBasicKindsAsTypes[i];
      const char *typeName = g_ArBasicTypeNames[kind];
      uint8_t templateArgCount = g_ArBasicKindsTemplateCount[i];
      DXASSERT(templateArgCount <= 3, "otherwise a new case has been added");
      int startDepth = (templateArgCount == 0) ? 0 : 1;
      CXXRecordDecl *recordDecl = m_objectTypeDecls[i];
      if (recordDecl == nullptr) {
        continue;
      }

      // This is a variation of AddObjectMethods using the new table.
      const HLSL_INTRINSIC *pIntrinsic = nullptr;
      const HLSL_INTRINSIC *pPrior = nullptr;
      UINT64 lookupCookie = 0;
      CA2W wideTypeName(typeName);
      HRESULT found = table->LookupIntrinsic(wideTypeName, L"*", &pIntrinsic,
                                             &lookupCookie);
      while (pIntrinsic != nullptr && SUCCEEDED(found)) {
        if (!AreIntrinsicTemplatesEquivalent(pIntrinsic, pPrior)) {
          AddObjectIntrinsicTemplate(recordDecl, startDepth, pIntrinsic);
          // NOTE: this only works with the current implementation because
          // intrinsics are alive as long as the table is alive.
          pPrior = pIntrinsic;
        }
        found = table->LookupIntrinsic(wideTypeName, L"*", &pIntrinsic,
                                       &lookupCookie);
      }
    }
  }

  void RegisterIntrinsicTable(IDxcIntrinsicTable *table) {
    DXASSERT_NOMSG(table != nullptr);
    m_intrinsicTables.push_back(table);
    // If already initialized, add methods immediately.
    if (m_sema != nullptr) {
      AddIntrinsicTableMethods(table);
    }
  }

  HLSLScalarType ScalarTypeForBasic(ArBasicKind kind) {
    DXASSERT(kind < AR_BASIC_COUNT,
             "otherwise caller didn't check that the value was in range");
    switch (kind) {
    case AR_BASIC_BOOL:
      return HLSLScalarType_bool;
    case AR_BASIC_LITERAL_FLOAT:
      return HLSLScalarType_float_lit;
    case AR_BASIC_FLOAT16:
      return HLSLScalarType_half;
    case AR_BASIC_FLOAT32_PARTIAL_PRECISION:
      return HLSLScalarType_float;
    case AR_BASIC_FLOAT32:
      return HLSLScalarType_float;
    case AR_BASIC_FLOAT64:
      return HLSLScalarType_double;
    case AR_BASIC_LITERAL_INT:
      return HLSLScalarType_int_lit;
    case AR_BASIC_INT8:
      return HLSLScalarType_int;
    case AR_BASIC_UINT8:
      return HLSLScalarType_uint;
    case AR_BASIC_INT16:
      return HLSLScalarType_int16;
    case AR_BASIC_UINT16:
      return HLSLScalarType_uint16;
    case AR_BASIC_INT32:
      return HLSLScalarType_int;
    case AR_BASIC_UINT32:
      return HLSLScalarType_uint;
    case AR_BASIC_MIN10FLOAT:
      return HLSLScalarType_float_min10;
    case AR_BASIC_MIN16FLOAT:
      return HLSLScalarType_float_min16;
    case AR_BASIC_MIN12INT:
      return HLSLScalarType_int_min12;
    case AR_BASIC_MIN16INT:
      return HLSLScalarType_int_min16;
    case AR_BASIC_MIN16UINT:
      return HLSLScalarType_uint_min16;
    case AR_BASIC_INT8_4PACKED:
      return HLSLScalarType_int8_4packed;
    case AR_BASIC_UINT8_4PACKED:
      return HLSLScalarType_uint8_4packed;

    case AR_BASIC_INT64:
      return HLSLScalarType_int64;
    case AR_BASIC_UINT64:
      return HLSLScalarType_uint64;
    case AR_BASIC_ENUM:
      return HLSLScalarType_int;
    default:
      return HLSLScalarType_unknown;
    }
  }

  QualType GetBasicKindType(ArBasicKind kind) {
    DXASSERT_VALIDBASICKIND(kind);

    switch (kind) {
    case AR_OBJECT_NULL:
      return m_context->VoidTy;
    case AR_BASIC_BOOL:
      return m_context->BoolTy;
    case AR_BASIC_LITERAL_FLOAT:
      return m_context->LitFloatTy;
    case AR_BASIC_FLOAT16:
      return m_context->HalfTy;
    case AR_BASIC_FLOAT32_PARTIAL_PRECISION:
      return m_context->HalfFloatTy;
    case AR_BASIC_FLOAT32:
      return m_context->FloatTy;
    case AR_BASIC_FLOAT64:
      return m_context->DoubleTy;
    case AR_BASIC_LITERAL_INT:
      return m_context->LitIntTy;
    case AR_BASIC_INT8:
      return m_context->IntTy;
    case AR_BASIC_UINT8:
      return m_context->UnsignedIntTy;
    case AR_BASIC_INT16:
      return m_context->ShortTy;
    case AR_BASIC_UINT16:
      return m_context->UnsignedShortTy;
    case AR_BASIC_INT32:
      return m_context->IntTy;
    case AR_BASIC_UINT32:
      return m_context->UnsignedIntTy;
    case AR_BASIC_INT64:
      return m_context->LongLongTy;
    case AR_BASIC_UINT64:
      return m_context->UnsignedLongLongTy;
    case AR_BASIC_MIN10FLOAT:
      return m_scalarTypes[HLSLScalarType_float_min10];
    case AR_BASIC_MIN16FLOAT:
      return m_scalarTypes[HLSLScalarType_float_min16];
    case AR_BASIC_MIN12INT:
      return m_scalarTypes[HLSLScalarType_int_min12];
    case AR_BASIC_MIN16INT:
      return m_scalarTypes[HLSLScalarType_int_min16];
    case AR_BASIC_MIN16UINT:
      return m_scalarTypes[HLSLScalarType_uint_min16];
    case AR_BASIC_INT8_4PACKED:
      return m_scalarTypes[HLSLScalarType_int8_4packed];
    case AR_BASIC_UINT8_4PACKED:
      return m_scalarTypes[HLSLScalarType_uint8_4packed];
    case AR_BASIC_ENUM:
      return m_context->IntTy;
    case AR_BASIC_ENUM_CLASS:
      return m_context->IntTy;

    case AR_OBJECT_STRING:
      return m_hlslStringType;
    case AR_OBJECT_STRING_LITERAL:
      // m_hlslStringType is defined as 'char *'.
      // for STRING_LITERAL we should use 'const char *'.
      return m_context->getPointerType(m_context->CharTy.withConst());

    case AR_OBJECT_LEGACY_EFFECT: // used for all legacy effect object types

    case AR_OBJECT_TEXTURE1D:
    case AR_OBJECT_TEXTURE1D_ARRAY:
    case AR_OBJECT_TEXTURE2D:
    case AR_OBJECT_TEXTURE2D_ARRAY:
    case AR_OBJECT_TEXTURE3D:
    case AR_OBJECT_TEXTURECUBE:
    case AR_OBJECT_TEXTURECUBE_ARRAY:
    case AR_OBJECT_TEXTURE2DMS:
    case AR_OBJECT_TEXTURE2DMS_ARRAY:

    case AR_OBJECT_SAMPLER:
    case AR_OBJECT_SAMPLERCOMPARISON:

    case AR_OBJECT_HEAP_RESOURCE:
    case AR_OBJECT_HEAP_SAMPLER:

    case AR_OBJECT_BUFFER:

    case AR_OBJECT_POINTSTREAM:
    case AR_OBJECT_LINESTREAM:
    case AR_OBJECT_TRIANGLESTREAM:

    case AR_OBJECT_INPUTPATCH:
    case AR_OBJECT_OUTPUTPATCH:

    case AR_OBJECT_RWTEXTURE1D:
    case AR_OBJECT_RWTEXTURE1D_ARRAY:
    case AR_OBJECT_RWTEXTURE2D:
    case AR_OBJECT_RWTEXTURE2D_ARRAY:
    case AR_OBJECT_RWTEXTURE3D:
    case AR_OBJECT_RWBUFFER:

    case AR_OBJECT_BYTEADDRESS_BUFFER:
    case AR_OBJECT_RWBYTEADDRESS_BUFFER:
    case AR_OBJECT_STRUCTURED_BUFFER:
    case AR_OBJECT_RWSTRUCTURED_BUFFER:
    case AR_OBJECT_APPEND_STRUCTURED_BUFFER:
    case AR_OBJECT_CONSUME_STRUCTURED_BUFFER:
    case AR_OBJECT_WAVE:
    case AR_OBJECT_ACCELERATION_STRUCT:
    case AR_OBJECT_RAY_DESC:
    case AR_OBJECT_HIT_OBJECT:
    case AR_OBJECT_TRIANGLE_INTERSECTION_ATTRIBUTES:
    case AR_OBJECT_RWTEXTURE2DMS:
    case AR_OBJECT_RWTEXTURE2DMS_ARRAY:

    case AR_OBJECT_EMPTY_NODE_INPUT:
    case AR_OBJECT_DISPATCH_NODE_INPUT_RECORD:
    case AR_OBJECT_RWDISPATCH_NODE_INPUT_RECORD:
    case AR_OBJECT_GROUP_NODE_INPUT_RECORDS:
    case AR_OBJECT_RWGROUP_NODE_INPUT_RECORDS:
    case AR_OBJECT_THREAD_NODE_INPUT_RECORD:
    case AR_OBJECT_RWTHREAD_NODE_INPUT_RECORD:
    case AR_OBJECT_NODE_OUTPUT:
    case AR_OBJECT_EMPTY_NODE_OUTPUT:
    case AR_OBJECT_NODE_OUTPUT_ARRAY:
    case AR_OBJECT_EMPTY_NODE_OUTPUT_ARRAY:
    case AR_OBJECT_THREAD_NODE_OUTPUT_RECORDS:
    case AR_OBJECT_GROUP_NODE_OUTPUT_RECORDS:
#ifdef ENABLE_SPIRV_CODEGEN
    case AR_OBJECT_VK_BUFFER_POINTER:
#endif
    {
      const ArBasicKind *match = std::find(
          g_ArBasicKindsAsTypes,
          &g_ArBasicKindsAsTypes[_countof(g_ArBasicKindsAsTypes)], kind);
      DXASSERT(match != &g_ArBasicKindsAsTypes[_countof(g_ArBasicKindsAsTypes)],
               "otherwise can't find constant in basic kinds");
      size_t index = match - g_ArBasicKindsAsTypes;
      return m_context->getTagDeclType(this->m_objectTypeDecls[index]);
    }

    case AR_OBJECT_SAMPLER1D:
    case AR_OBJECT_SAMPLER2D:
    case AR_OBJECT_SAMPLER3D:
    case AR_OBJECT_SAMPLERCUBE:
      // Turn dimension-typed samplers into sampler states.
      return GetBasicKindType(AR_OBJECT_SAMPLER);

    case AR_OBJECT_STATEBLOCK:

    case AR_OBJECT_RASTERIZER:
    case AR_OBJECT_DEPTHSTENCIL:
    case AR_OBJECT_BLEND:

    case AR_OBJECT_RWSTRUCTURED_BUFFER_ALLOC:
    case AR_OBJECT_RWSTRUCTURED_BUFFER_CONSUME:

    default:
      return QualType();
    }
  }

  // Retrieves the ResourceKind and ResourceClass in `ResKind` and `ResClass`
  // respectively that correspond to the given basic kind `BasicKind`.
  // Returns true if `BasicKind` is a resource and return params are assigned.
  bool GetBasicKindResourceKindAndClass(ArBasicKind BasicKind,
                                        DXIL::ResourceKind &ResKind,
                                        DXIL::ResourceClass &ResClass) {
    DXASSERT_VALIDBASICKIND(BasicKind);
    switch (BasicKind) {
    case AR_OBJECT_TEXTURE1D:
      ResKind = DXIL::ResourceKind::Texture1D;
      ResClass = DXIL::ResourceClass::SRV;
      return true;
    case AR_OBJECT_RWTEXTURE1D:
    case AR_OBJECT_ROVTEXTURE1D:
      ResKind = DXIL::ResourceKind::Texture1D;
      ResClass = DXIL::ResourceClass::UAV;
      return true;
    case AR_OBJECT_TEXTURE1D_ARRAY:
      ResKind = DXIL::ResourceKind::Texture1DArray;
      ResClass = DXIL::ResourceClass::SRV;
      return true;
    case AR_OBJECT_RWTEXTURE1D_ARRAY:
    case AR_OBJECT_ROVTEXTURE1D_ARRAY:
      ResKind = DXIL::ResourceKind::Texture1DArray;
      ResClass = DXIL::ResourceClass::UAV;
      return true;
    case AR_OBJECT_TEXTURE2D:
      ResKind = DXIL::ResourceKind::Texture2D;
      ResClass = DXIL::ResourceClass::SRV;
      return true;
    case AR_OBJECT_RWTEXTURE2D:
    case AR_OBJECT_ROVTEXTURE2D:
      ResKind = DXIL::ResourceKind::Texture2D;
      ResClass = DXIL::ResourceClass::UAV;
      return true;
    case AR_OBJECT_TEXTURE2D_ARRAY:
      ResKind = DXIL::ResourceKind::Texture2DArray;
      ResClass = DXIL::ResourceClass::SRV;
      return true;
    case AR_OBJECT_RWTEXTURE2D_ARRAY:
    case AR_OBJECT_ROVTEXTURE2D_ARRAY:
      ResKind = DXIL::ResourceKind::Texture2DArray;
      ResClass = DXIL::ResourceClass::UAV;
      return true;
    case AR_OBJECT_TEXTURE3D:
      ResKind = DXIL::ResourceKind::Texture3D;
      ResClass = DXIL::ResourceClass::SRV;
      return true;
    case AR_OBJECT_RWTEXTURE3D:
    case AR_OBJECT_ROVTEXTURE3D:
      ResKind = DXIL::ResourceKind::Texture3D;
      ResClass = DXIL::ResourceClass::UAV;
      return true;
    case AR_OBJECT_TEXTURECUBE:
      ResKind = DXIL::ResourceKind::TextureCube;
      ResClass = DXIL::ResourceClass::SRV;
      return true;
    case AR_OBJECT_TEXTURECUBE_ARRAY:
      ResKind = DXIL::ResourceKind::TextureCubeArray;
      ResClass = DXIL::ResourceClass::SRV;
      return true;
    case AR_OBJECT_TEXTURE2DMS:
      ResKind = DXIL::ResourceKind::Texture2DMS;
      ResClass = DXIL::ResourceClass::SRV;
      return true;
    case AR_OBJECT_RWTEXTURE2DMS:
      ResKind = DXIL::ResourceKind::Texture2DMS;
      ResClass = DXIL::ResourceClass::UAV;
      return true;
    case AR_OBJECT_TEXTURE2DMS_ARRAY:
      ResKind = DXIL::ResourceKind::Texture2DMSArray;
      ResClass = DXIL::ResourceClass::SRV;
      return true;
    case AR_OBJECT_RWTEXTURE2DMS_ARRAY:
      ResKind = DXIL::ResourceKind::Texture2DMSArray;
      ResClass = DXIL::ResourceClass::UAV;
      return true;
    case AR_OBJECT_BUFFER:
      ResKind = DXIL::ResourceKind::TypedBuffer;
      ResClass = DXIL::ResourceClass::SRV;
      return true;
    case AR_OBJECT_RWBUFFER:
    case AR_OBJECT_ROVBUFFER:
      ResKind = DXIL::ResourceKind::TypedBuffer;
      ResClass = DXIL::ResourceClass::UAV;
      return true;
    case AR_OBJECT_BYTEADDRESS_BUFFER:
      ResKind = DXIL::ResourceKind::RawBuffer;
      ResClass = DXIL::ResourceClass::SRV;
      return true;
    case AR_OBJECT_RWBYTEADDRESS_BUFFER:
    case AR_OBJECT_ROVBYTEADDRESS_BUFFER:
      ResKind = DXIL::ResourceKind::RawBuffer;
      ResClass = DXIL::ResourceClass::UAV;
      return true;
    case AR_OBJECT_STRUCTURED_BUFFER:
      ResKind = DXIL::ResourceKind::StructuredBuffer;
      ResClass = DXIL::ResourceClass::SRV;
      return true;
    case AR_OBJECT_RWSTRUCTURED_BUFFER:
    case AR_OBJECT_ROVSTRUCTURED_BUFFER:
    case AR_OBJECT_CONSUME_STRUCTURED_BUFFER:
    case AR_OBJECT_APPEND_STRUCTURED_BUFFER:
      ResKind = DXIL::ResourceKind::StructuredBuffer;
      ResClass = DXIL::ResourceClass::UAV;
      return true;
    case AR_OBJECT_CONSTANT_BUFFER:
      ResKind = DXIL::ResourceKind::CBuffer;
      ResClass = DXIL::ResourceClass::CBuffer;
      return true;
    case AR_OBJECT_TEXTURE_BUFFER:
      ResKind = DXIL::ResourceKind::TBuffer;
      ResClass = DXIL::ResourceClass::SRV;
      return true;
    case AR_OBJECT_FEEDBACKTEXTURE2D:
      ResKind = DXIL::ResourceKind::FeedbackTexture2D;
      ResClass = DXIL::ResourceClass::SRV;
      return true;
    case AR_OBJECT_FEEDBACKTEXTURE2D_ARRAY:
      ResKind = DXIL::ResourceKind::FeedbackTexture2DArray;
      ResClass = DXIL::ResourceClass::SRV;
      return true;
    case AR_OBJECT_SAMPLER:
    case AR_OBJECT_SAMPLERCOMPARISON:
      ResKind = DXIL::ResourceKind::Sampler;
      ResClass = DXIL::ResourceClass::Sampler;
      return true;
    case AR_OBJECT_ACCELERATION_STRUCT:
      ResKind = DXIL::ResourceKind::RTAccelerationStructure;
      ResClass = DXIL::ResourceClass::SRV;
      return true;
    default:
      return false;
    }
    return false;
  }

  /// <summary>Promotes the specified expression to an integer type if it's a
  /// boolean type.</summary <param name="E">Expression to typecast.</param>
  /// <returns>E typecast to a integer type if it's a valid boolean type; E
  /// otherwise.</returns>
  ExprResult PromoteToIntIfBool(ExprResult &E);

  QualType NewQualifiedType(UINT64 qwUsages, QualType type) {
    // NOTE: NewQualifiedType does quite a bit more in the prior compiler
    (void)(qwUsages);
    return type;
  }

  QualType NewSimpleAggregateType(ArTypeObjectKind ExplicitKind,
                                  ArBasicKind componentType, UINT64 qwQual,
                                  UINT uRows, UINT uCols) {
    DXASSERT_VALIDBASICKIND(componentType);

    QualType pType; // The type to return.
    if (componentType < AR_BASIC_COUNT) {
      // If basic numeric, call LookupScalarTypeDef to ensure on-demand
      // initialization
      LookupScalarTypeDef(ScalarTypeForBasic(componentType));
    }
    QualType pEltType = GetBasicKindType(componentType);
    DXASSERT(!pEltType.isNull(),
             "otherwise caller is specifying an incorrect basic kind type");

    // TODO: handle adding qualifications like const
    pType = NewQualifiedType(
        qwQual & ~(UINT64)(AR_QUAL_COLMAJOR | AR_QUAL_ROWMAJOR), pEltType);

    if (uRows > 1 || uCols > 1 || ExplicitKind == AR_TOBJ_VECTOR ||
        ExplicitKind == AR_TOBJ_MATRIX) {
      HLSLScalarType scalarType = ScalarTypeForBasic(componentType);
      DXASSERT(scalarType != HLSLScalarType_unknown,
               "otherwise caller is specifying an incorrect type");

      if ((uRows == 1 && ExplicitKind != AR_TOBJ_MATRIX) ||
          ExplicitKind == AR_TOBJ_VECTOR) {
        pType = LookupVectorType(scalarType, uCols);
      } else {
        pType = LookupMatrixType(scalarType, uRows, uCols);
      }

      // TODO: handle colmajor/rowmajor
      // if ((qwQual & (AR_QUAL_COLMAJOR | AR_QUAL_ROWMAJOR)) != 0)
      //{
      //  VN(pType = NewQualifiedType(pSrcLoc,
      //    qwQual & (AR_QUAL_COLMAJOR |
      //    AR_QUAL_ROWMAJOR),
      //    pMatrix));
      //}
      // else
      //{
      //  pType = pMatrix;
      //}
    }

    return pType;
  }

  /// <summary>Attempts to match Args to the signature specification in
  /// pIntrinsic.</summary> <param name="cursor">Intrinsic function
  /// iterator.</param> <param name="objectElement">Type element on the class
  /// intrinsic belongs to; possibly null (eg, 'float' in
  /// 'Texture2D<float>').</param> <param name="Args">Invocation arguments to
  /// match.</param> <param name="argTypes">After exectuion, type of
  /// arguments.</param> <param name="badArgIdx">The first argument to mismatch
  /// if any</param> <remarks>On success, argTypes includes the clang Types to
  /// use for the signature, with the first being the return type.</remarks>
  bool MatchArguments(const IntrinsicDefIter &cursor, QualType objectType,
                      QualType objectElement, QualType functionTemplateTypeArg,
                      ArrayRef<Expr *> Args, std::vector<QualType> *,
                      size_t &badArgIdx);

  /// <summary>Validate object element on intrinsic to catch case like integer
  /// on Sample.</summary> <param name="tableName">Intrinsic function to
  /// validate.</param> <param name="op">Intrinsic opcode to validate.</param>
  /// <param name="objectElement">Type element on the class intrinsic belongs
  /// to; possibly null (eg, 'float' in 'Texture2D<float>').</param>
  bool IsValidObjectElement(LPCSTR tableName, IntrinsicOp op,
                            QualType objectElement);

  // Returns the iterator with the first entry that matches the requirement
  IntrinsicDefIter FindIntrinsicByNameAndArgCount(const HLSL_INTRINSIC *table,
                                                  size_t tableSize,
                                                  StringRef typeName,
                                                  StringRef nameIdentifier,
                                                  size_t argumentCount) {
    // This is implemented by a linear scan for now.
    // We tested binary search on tables, and there was no performance gain on
    // samples probably for the following reasons.
    // 1. The tables are not big enough to make noticable difference
    // 2. The user of this function assumes that it returns the first entry in
    // the table that matches name and argument count. So even in the binary
    // search, we have to scan backwards until the entry does not match the name
    // or arg count. For linear search this is not a problem
    for (unsigned int i = 0; i < tableSize; i++) {
      const HLSL_INTRINSIC *pIntrinsic = &table[i];

      const bool isVariadicFn = IsVariadicIntrinsicFunction(pIntrinsic);

      // Do some quick checks to verify size and name.
      if (!isVariadicFn && pIntrinsic->uNumArgs != 1 + argumentCount) {
        continue;
      }
      if (!nameIdentifier.equals(StringRef(pIntrinsic->pArgs[0].pName))) {
        continue;
      }

      return IntrinsicDefIter::CreateStart(
          table, tableSize, pIntrinsic,
          IntrinsicTableDefIter::CreateStart(m_intrinsicTables, typeName,
                                             nameIdentifier, argumentCount));
    }

    return IntrinsicDefIter::CreateStart(
        table, tableSize, table + tableSize,
        IntrinsicTableDefIter::CreateStart(m_intrinsicTables, typeName,
                                           nameIdentifier, argumentCount));
  }

  bool AddOverloadedCallCandidates(UnresolvedLookupExpr *ULE,
                                   ArrayRef<Expr *> Args,
                                   OverloadCandidateSet &CandidateSet, Scope *S,
                                   bool PartialOverloading) override {
    DXASSERT_NOMSG(ULE != nullptr);

    const bool isQualified = ULE->getQualifier();

    const bool isGlobalNamespace =
        ULE->getQualifier() &&
        ULE->getQualifier()->getKind() == NestedNameSpecifier::Global;

    const bool isVkNamespace =
        ULE->getQualifier() &&
        ULE->getQualifier()->getKind() == NestedNameSpecifier::Namespace &&
        ULE->getQualifier()->getAsNamespace()->getName() == "vk";

    const bool isDxNamespace =
        ULE->getQualifier() &&
        ULE->getQualifier()->getKind() == NestedNameSpecifier::Namespace &&
        ULE->getQualifier()->getAsNamespace()->getName() == "dx";

    // Intrinsics live in the global namespace, so references to their names
    // should be either unqualified or '::'-prefixed.
    // Exceptions:
    // - Vulkan-specific intrinsics live in the 'vk::' namespace.
    // - DirectX-specific intrinsics live in the 'dx::' namespace.
    // - Global namespaces could just mean we have a `using` declaration... so
    // it can be anywhere!
    if (isQualified && !isGlobalNamespace && !isVkNamespace && !isDxNamespace)
      return false;

    const DeclarationNameInfo declName = ULE->getNameInfo();
    IdentifierInfo *idInfo = declName.getName().getAsIdentifierInfo();
    if (idInfo == nullptr) {
      return false;
    }

    StringRef nameIdentifier = idInfo->getName();
    using IntrinsicArray = llvm::ArrayRef<const HLSL_INTRINSIC>;
    struct IntrinsicTableEntry {
      IntrinsicArray Table;
      NamespaceDecl *NS;
    };

    llvm::SmallVector<IntrinsicTableEntry, 3> SearchTables;

    bool SearchDX = isDxNamespace;
    bool SearchVK = isVkNamespace;
    if (isGlobalNamespace || !isQualified)
      SearchTables.push_back(
          IntrinsicTableEntry{IntrinsicArray(g_Intrinsics), m_hlslNSDecl});

    if (S && !isQualified) {
      SmallVector<const DeclContext *, 4> NSContexts;
      m_sema->CollectNamespaceContexts(S, NSContexts);
      for (const auto &UD : NSContexts) {
        if (static_cast<DeclContext *>(m_dxNSDecl) == UD)
          SearchDX = true;
        else if (static_cast<DeclContext *>(m_vkNSDecl) == UD)
          SearchVK = true;
      }
    }

    if (SearchDX)
      SearchTables.push_back(
          IntrinsicTableEntry{IntrinsicArray(g_DxIntrinsics), m_dxNSDecl});
#ifdef ENABLE_SPIRV_CODEGEN
    if (SearchVK)
      SearchTables.push_back(
          IntrinsicTableEntry{IntrinsicArray(g_VkIntrinsics), m_vkNSDecl});
#endif

    assert(!SearchTables.empty() && "Must have at least one search table!");

    for (const auto &T : SearchTables) {

      IntrinsicDefIter cursor = FindIntrinsicByNameAndArgCount(
          T.Table.data(), T.Table.size(), StringRef(), nameIdentifier,
          Args.size());
      IntrinsicDefIter end = IntrinsicDefIter::CreateEnd(
          T.Table.data(), T.Table.size(),
          IntrinsicTableDefIter::CreateEnd(m_intrinsicTables));

      for (; cursor != end; ++cursor) {
        // If this is the intrinsic we're interested in, build up a
        // representation of the types we need.
        const HLSL_INTRINSIC *pIntrinsic = *cursor;
        LPCSTR tableName = cursor.GetTableName();
        LPCSTR lowering = cursor.GetLoweringStrategy();
        DXASSERT(pIntrinsic->uNumArgs <= g_MaxIntrinsicParamCount + 1,
                 "otherwise g_MaxIntrinsicParamCount needs to be updated for "
                 "wider signatures");

        std::vector<QualType> functionArgTypes;
        size_t badArgIdx;
        bool argsMatch =
            MatchArguments(cursor, QualType(), QualType(), QualType(), Args,
                           &functionArgTypes, badArgIdx);
        if (!functionArgTypes.size())
          return false;

        // Get or create the overload we're interested in.
        FunctionDecl *intrinsicFuncDecl = nullptr;
        std::pair<UsedIntrinsicStore::iterator, bool> insertResult =
            m_usedIntrinsics.insert(
                UsedIntrinsic(pIntrinsic, functionArgTypes));
        bool insertedNewValue = insertResult.second;
        if (insertedNewValue) {
          DXASSERT(tableName,
                   "otherwise IDxcIntrinsicTable::GetTableName() failed");
          intrinsicFuncDecl =
              AddHLSLIntrinsicFunction(*m_context, T.NS, tableName, lowering,
                                       pIntrinsic, &functionArgTypes);
          insertResult.first->setFunctionDecl(intrinsicFuncDecl);
        } else {
          intrinsicFuncDecl = (*insertResult.first).getFunctionDecl();
        }

        OverloadCandidate &candidate = CandidateSet.addCandidate(Args.size());
        candidate.Function = intrinsicFuncDecl;
        candidate.FoundDecl.setDecl(intrinsicFuncDecl);
        candidate.Viable = argsMatch;
        CandidateSet.isNewCandidate(
            intrinsicFuncDecl); // used to insert into set
        if (argsMatch)
          return true;
        if (badArgIdx) {
          candidate.FailureKind = ovl_fail_bad_conversion;
          QualType ParamType =
              intrinsicFuncDecl->getParamDecl(badArgIdx - 1)->getType();
          candidate.Conversions[badArgIdx - 1].setBad(
              BadConversionSequence::no_conversion, Args[badArgIdx - 1],
              ParamType);
        } else {
          // A less informative error. Needed when the failure relates to the
          // return type
          candidate.FailureKind = ovl_fail_bad_final_conversion;
        }
      }
    }

    return false;
  }

  bool Initialize(ASTContext &context) {
    m_context = &context;

    // The HLSL namespace is disabled here pending a decision on
    // https://github.com/microsoft/hlsl-specs/issues/484.
    if (false && context.getLangOpts().HLSLVersion >= hlsl::LangStd::v202x) {
      m_hlslNSDecl =
          NamespaceDecl::Create(context, context.getTranslationUnitDecl(),
                                /*Inline*/ false, SourceLocation(),
                                SourceLocation(), &context.Idents.get("hlsl"),
                                /*PrevDecl*/ nullptr);
      m_hlslNSDecl->setImplicit();
    }
    AddBaseTypes();
    AddHLSLScalarTypes();
    AddHLSLStringType();

    AddHLSLVectorTemplate(*m_context, &m_vectorTemplateDecl);
    DXASSERT(
        m_vectorTemplateDecl != nullptr,
        "AddHLSLVectorTypes failed to return the vector template declaration");
    AddHLSLMatrixTemplate(*m_context, m_vectorTemplateDecl,
                          &m_matrixTemplateDecl);
    DXASSERT(
        m_matrixTemplateDecl != nullptr,
        "AddHLSLMatrixTypes failed to return the matrix template declaration");

    // Initializing built in integers for ray tracing
    AddRaytracingConstants(*m_context);
    AddSamplerFeedbackConstants(*m_context);
    AddBarrierConstants(*m_context);

    AddHLSLNodeOutputRecordTemplate(*m_context,
                                    DXIL::NodeIOKind::GroupNodeOutputRecords,
                                    &m_GroupNodeOutputRecordsTemplateDecl,
                                    /* isCompleteType */ false);
    AddHLSLNodeOutputRecordTemplate(*m_context,
                                    DXIL::NodeIOKind::ThreadNodeOutputRecords,
                                    &m_ThreadNodeOutputRecordsTemplateDecl,
                                    /* isCompleteType */ false);

    return true;
  }

  /// <summary>Checks whether the specified type is numeric or composed of
  /// numeric elements exclusively.</summary>
  bool IsTypeNumeric(QualType type, UINT *count);

  /// <summary>Checks whether the specified type is a scalar type.</summary>
  bool IsScalarType(const QualType &type) {
    DXASSERT(!type.isNull(), "caller should validate its type is initialized");
    return BasicTypeForScalarType(type->getCanonicalTypeUnqualified()) !=
           AR_BASIC_UNKNOWN;
  }

  /// <summary>Checks whether the specified value is a valid matrix row or
  /// column size.</summary>
  bool IsValidMatrixColOrRowSize(size_t length) {
    return 1 <= length && length <= 4;
  }

  bool IsValidTemplateArgumentType(SourceLocation argLoc, const QualType &type,
                                   bool requireScalar) {
    if (type.isNull()) {
      return false;
    }

    if (type.hasQualifiers()) {
      return false;
    }

    QualType qt = GetStructuralForm(type);

    if (requireScalar) {
      if (!IsScalarType(qt)) {
        m_sema->Diag(argLoc,
                     diag::err_hlsl_typeintemplateargument_requires_scalar)
            << type;
        return false;
      }
      return true;
    } else {
      ArTypeObjectKind objectKind = GetTypeObjectKind(qt);

      if (qt->isArrayType()) {
        const ArrayType *arrayType = qt->getAsArrayTypeUnsafe();
        return IsValidTemplateArgumentType(argLoc, arrayType->getElementType(),
                                           false);
      } else if (objectKind == AR_TOBJ_VECTOR) {
        bool valid = true;
        if (!IsScalarType(GetMatrixOrVectorElementType(type))) {
          valid = false;
          m_sema->Diag(argLoc, diag::err_hlsl_unsupportedvectortype)
              << type << GetMatrixOrVectorElementType(type);
        }
        return valid;
      } else if (objectKind == AR_TOBJ_MATRIX) {
        bool valid = true;
        UINT rowCount, colCount;
        GetRowsAndCols(type, rowCount, colCount);
        if (!IsValidMatrixColOrRowSize(rowCount) ||
            !IsValidMatrixColOrRowSize(colCount)) {
          valid = false;
          m_sema->Diag(argLoc, diag::err_hlsl_unsupportedmatrixsize)
              << type << rowCount << colCount;
        }
        if (!IsScalarType(GetMatrixOrVectorElementType(type))) {
          valid = false;
          m_sema->Diag(argLoc, diag::err_hlsl_unsupportedvectortype)
              << type << GetMatrixOrVectorElementType(type);
        }
        return valid;
#ifdef ENABLE_SPIRV_CODEGEN
      } else if (hlsl::IsVKBufferPointerType(qt)) {
        return true;
#endif
      } else if (qt->isStructureOrClassType()) {
        const RecordType *recordType = qt->getAs<RecordType>();
        objectKind = ClassifyRecordType(recordType);
        switch (objectKind) {
        case AR_TOBJ_OBJECT:
#ifdef ENABLE_SPIRV_CODEGEN
          if (const auto *namespaceDecl = dyn_cast<NamespaceDecl>(
                  recordType->getDecl()->getDeclContext());
              namespaceDecl && namespaceDecl->getName().equals("vk") &&
              (recordType->getDecl()->getName().equals("SpirvType") ||
               recordType->getDecl()->getName().equals("SpirvOpaqueType"))) {
            return true;
          }
#endif
          m_sema->Diag(argLoc, diag::err_hlsl_unsupported_object_context)
              << type << static_cast<unsigned>(TypeDiagContext::TypeParameter);
          return false;
        case AR_TOBJ_COMPOUND: {
          const RecordDecl *recordDecl = recordType->getDecl();
          if (recordDecl->isInvalidDecl())
            return false;
          RecordDecl::field_iterator begin = recordDecl->field_begin();
          RecordDecl::field_iterator end = recordDecl->field_end();
          bool result = true;
          while (begin != end) {
            const FieldDecl *fieldDecl = *begin;
            if (!IsValidTemplateArgumentType(argLoc, fieldDecl->getType(),
                                             false)) {
              m_sema->Diag(argLoc, diag::note_field_type_usage)
                  << fieldDecl->getType() << fieldDecl->getIdentifier() << type;
              result = false;
            }
            begin++;
          }
          return result;
        }
        default:
          m_sema->Diag(argLoc, diag::err_hlsl_typeintemplateargument) << type;
          return false;
        }
      } else if (IsScalarType(qt)) {
        return true;
      } else {
        m_sema->Diag(argLoc, diag::err_hlsl_typeintemplateargument) << type;
        return false;
      }
    }
  }

  /// <summary>Checks whether the source type can be converted to the target
  /// type.</summary>
  bool CanConvert(SourceLocation loc, Expr *sourceExpr, QualType target,
                  bool explicitConversion, TYPE_CONVERSION_REMARKS *remarks,
                  StandardConversionSequence *sequence);
  void CollectInfo(QualType type, ArTypeInfo *pTypeInfo);
  void GetConversionForm(QualType type, bool explicitConversion,
                         ArTypeInfo *pTypeInfo);
  bool ValidateCast(SourceLocation Loc, Expr *source, QualType target,
                    bool explicitConversion, bool suppressWarnings,
                    bool suppressErrors, StandardConversionSequence *sequence);
  bool ValidatePrimitiveTypeForOperand(SourceLocation loc, QualType type,
                                       ArTypeObjectKind kind);
  bool ValidateTypeRequirements(SourceLocation loc, ArBasicKind elementKind,
                                ArTypeObjectKind objectKind,
                                bool requiresIntegrals, bool requiresNumerics);

  /// <summary>Validates and adjusts operands for the specified binary
  /// operator.</summary> <param name="OpLoc">Source location for
  /// operator.</param> <param name="Opc">Kind of binary operator.</param>
  /// <param name="LHS">Left-hand-side expression, possibly updated by this
  /// function.</param> <param name="RHS">Right-hand-side expression, possibly
  /// updated by this function.</param> <param name="ResultTy">Result type for
  /// operator expression.</param> <param name="CompLHSTy">Type of LHS after
  /// promotions for computation.</param> <param name="CompResultTy">Type of
  /// computation result.</param>
  void CheckBinOpForHLSL(SourceLocation OpLoc, BinaryOperatorKind Opc,
                         ExprResult &LHS, ExprResult &RHS, QualType &ResultTy,
                         QualType &CompLHSTy, QualType &CompResultTy);

  /// <summary>Validates and adjusts operands for the specified unary
  /// operator.</summary> <param name="OpLoc">Source location for
  /// operator.</param> <param name="Opc">Kind of operator.</param> <param
  /// name="InputExpr">Input expression to the operator.</param> <param
  /// name="VK">Value kind for resulting expression.</param> <param
  /// name="OK">Object kind for resulting expression.</param> <returns>The
  /// result type for the expression.</returns>
  QualType CheckUnaryOpForHLSL(SourceLocation OpLoc, UnaryOperatorKind Opc,
                               ExprResult &InputExpr, ExprValueKind &VK,
                               ExprObjectKind &OK);

  /// <summary>Checks vector conditional operator (Cond ? LHS : RHS).</summary>
  /// <param name="Cond">Vector condition expression.</param>
  /// <param name="LHS">Left hand side.</param>
  /// <param name="RHS">Right hand side.</param>
  /// <param name="QuestionLoc">Location of question mark in operator.</param>
  /// <returns>Result type of vector conditional expression.</returns>
  clang::QualType CheckVectorConditional(ExprResult &Cond, ExprResult &LHS,
                                         ExprResult &RHS,
                                         SourceLocation QuestionLoc);

  clang::QualType ApplyTypeSpecSignToParsedType(clang::QualType &type,
                                                TypeSpecifierSign TSS,
                                                SourceLocation Loc);

  bool CheckRangedTemplateArgument(SourceLocation diagLoc,
                                   llvm::APSInt &sintValue, bool IsVector) {
    unsigned MaxLength = DXIL::kDefaultMaxVectorLength;
    if (IsVector)
      MaxLength = m_sema->getLangOpts().MaxHLSLVectorLength;
    if (!sintValue.isStrictlyPositive() ||
        sintValue.getLimitedValue() > MaxLength) {
      m_sema->Diag(diagLoc, diag::err_hlsl_invalid_range_1_to_max) << MaxLength;
      return true;
    }

    return false;
  }

  /// <summary>Performs HLSL-specific processing of template
  /// declarations.</summary>
  bool
  CheckTemplateArgumentListForHLSL(TemplateDecl *Template,
                                   SourceLocation /* TemplateLoc */,
                                   TemplateArgumentListInfo &TemplateArgList) {
    DXASSERT_NOMSG(Template != nullptr);

    // Determine which object type the template refers to.
    StringRef templateName = Template->getName();

    // NOTE: this 'escape valve' allows unit tests to perform type checks.
    if (templateName.equals(StringRef("is_same"))) {
      return false;
    }
    // Allow object type for Constant/TextureBuffer.
    HLSLResourceAttr *ResAttr =
        Template->getTemplatedDecl()->getAttr<HLSLResourceAttr>();
    if (ResAttr && DXIL::IsCTBuffer(ResAttr->getResKind())) {
      if (TemplateArgList.size() == 1) {
        const TemplateArgumentLoc &argLoc = TemplateArgList[0];
        const TemplateArgument &arg = argLoc.getArgument();
        DXASSERT(arg.getKind() == TemplateArgument::ArgKind::Type,
                 "cbuffer with non-type template arg");
        QualType argType = arg.getAsType();
        SourceLocation argSrcLoc = argLoc.getLocation();
        if (IsScalarType(argType) || IsVectorType(m_sema, argType) ||
            IsMatrixType(m_sema, argType) || argType->isArrayType()) {
          m_sema->Diag(argSrcLoc,
                       diag::err_hlsl_typeintemplateargument_requires_struct)
              << argType;
          return true;
        }
        m_sema->RequireCompleteType(argSrcLoc, argType,
                                    diag::err_typecheck_decl_incomplete_type);

        TypeDiagContext DiagContext =
            TypeDiagContext::ConstantBuffersOrTextureBuffers;
        if (DiagnoseTypeElements(*m_sema, argSrcLoc, argType, DiagContext,
                                 DiagContext))
          return true;
      }
      return false;
    } else if (ResAttr && DXIL::IsStructuredBuffer(ResAttr->getResKind())) {
      if (TemplateArgList.size() == 1) {
        const TemplateArgumentLoc &ArgLoc = TemplateArgList[0];
        const TemplateArgument &Arg = ArgLoc.getArgument();
        if (Arg.getKind() == TemplateArgument::ArgKind::Type) {
          QualType ArgType = Arg.getAsType();
          SourceLocation ArgSrcLoc = ArgLoc.getLocation();
          if (DiagnoseTypeElements(
                  *m_sema, ArgSrcLoc, ArgType,
                  TypeDiagContext::StructuredBuffers /*ObjDiagContext*/,
                  TypeDiagContext::Valid /*LongVecDiagContext*/))
            return true;
        }
      }

    } else if (Template->getTemplatedDecl()->hasAttr<HLSLNodeObjectAttr>()) {

      DXASSERT(TemplateArgList.size() == 1,
               "otherwise the template has not been declared properly");
      // The first argument must be a user defined struct type that does not
      // contain any HLSL object
      const TemplateArgumentLoc &ArgLoc = TemplateArgList[0];
      const TemplateArgument &Arg = ArgLoc.getArgument();

      // To get here the arg must have been accepted as a type acceptable to
      // HLSL, but that includes HLSL templates without args which we want to
      // disallow here.
      if (Arg.getKind() == TemplateArgument::ArgKind::Template) {
        TemplateDecl *TD = Arg.getAsTemplate().getAsTemplateDecl();
        SourceLocation ArgSrcLoc = ArgLoc.getLocation();
        m_sema->Diag(ArgSrcLoc, diag::err_hlsl_node_record_type)
            << TD->getName();
        return true;
      }

      QualType ArgTy = Arg.getAsType();
      // Ignore dependent types. Dependent argument types get expanded during
      // template instantiation.
      if (ArgTy->isDependentType())
        return false;
      // Make sure specialization is done before IsTypeNumeric.
      // If not, ArgTy might be treat as empty struct.
      m_sema->RequireCompleteType(ArgLoc.getLocation(), ArgTy,
                                  diag::err_typecheck_decl_incomplete_type);
      CXXRecordDecl *Decl = ArgTy->getAsCXXRecordDecl();
      if (Decl && !Decl->isCompleteDefinition())
        return true;
      // The node record type must be compound - error if it is not.
      if (GetTypeObjectKind(ArgTy) != AR_TOBJ_COMPOUND) {
        m_sema->Diag(ArgLoc.getLocation(), diag::err_hlsl_node_record_type)
            << ArgTy << ArgLoc.getSourceRange();
        return true;
      }

      bool EmptyStruct = true;
      if (DiagnoseNodeStructArgument(m_sema, ArgLoc, ArgTy, EmptyStruct))
        return true;
      // a node input/output record can't be empty - EmptyStruct is false if
      // any fields were found by DiagnoseNodeStructArgument()
      if (EmptyStruct) {
        m_sema->Diag(ArgLoc.getLocation(), diag::err_hlsl_zero_sized_record)
            << templateName << ArgLoc.getSourceRange();
        const RecordDecl *RD = ArgTy->getAs<RecordType>()->getDecl();
        m_sema->Diag(RD->getLocation(), diag::note_defined_here)
            << "zero sized record";
        return true;
      }
      return false;
    } else if (Template->getTemplatedDecl()
                   ->hasAttr<HLSLRayQueryObjectAttr>()) {
      int numArgs = TemplateArgList.size();
      DXASSERT(numArgs == 1 || numArgs == 2,
               "otherwise the template has not been declared properly");

      // first, determine if the rayquery flag AllowOpacityMicromaps is set
      bool HasRayQueryFlagAllowOpacityMicromaps = false;
      if (numArgs > 1) {
        const TemplateArgument &Arg2 = TemplateArgList[1].getArgument();
        Expr *Expr2 = Arg2.getAsExpr();
        llvm::APSInt Arg2val;
        Expr2->isIntegerConstantExpr(Arg2val, m_sema->getASTContext());
        if (Arg2val.getZExtValue() &
            (unsigned)DXIL::RayQueryFlag::AllowOpacityMicromaps)
          HasRayQueryFlagAllowOpacityMicromaps = true;
      }

      // next, get the first template argument, to check if
      // the ForceOMM2State flag is set
      const TemplateArgument &Arg1 = TemplateArgList[0].getArgument();
      Expr *Expr1 = Arg1.getAsExpr();
      llvm::APSInt Arg1val;
      bool HasRayFlagForceOMM2State =
          Expr1->isIntegerConstantExpr(Arg1val, m_sema->getASTContext()) &&
          (Arg1val.getLimitedValue() &
           (uint64_t)DXIL::RayFlag::ForceOMM2State) != 0;

      // finally, if ForceOMM2State is set and AllowOpacityMicromaps
      // isn't, emit a warning
      if (HasRayFlagForceOMM2State && !HasRayQueryFlagAllowOpacityMicromaps)
        m_sema->Diag(TemplateArgList[0].getLocation(),
                     diag::warn_hlsl_rayquery_flags_conflict);
    } else if (Template->getTemplatedDecl()->hasAttr<HLSLTessPatchAttr>()) {
      DXASSERT(TemplateArgList.size() > 0,
               "Tessellation patch should have at least one template args");
      const TemplateArgumentLoc &argLoc = TemplateArgList[0];
      const TemplateArgument &arg = argLoc.getArgument();
      DXASSERT(arg.getKind() == TemplateArgument::ArgKind::Type,
               "Tessellation patch requires type template arg 0");

      m_sema->RequireCompleteType(argLoc.getLocation(), arg.getAsType(),
                                  diag::err_typecheck_decl_incomplete_type);
      CXXRecordDecl *Decl = arg.getAsType()->getAsCXXRecordDecl();
      if (Decl && !Decl->isCompleteDefinition())
        return true;
      const TypeDiagContext DiagContext = TypeDiagContext::TessellationPatches;
      if (DiagnoseTypeElements(*m_sema, argLoc.getLocation(), arg.getAsType(),
                               DiagContext, DiagContext))
        return true;
    } else if (Template->getTemplatedDecl()->hasAttr<HLSLStreamOutputAttr>()) {
      DXASSERT(TemplateArgList.size() > 0,
               "Geometry streams should have at least one template args");
      const TemplateArgumentLoc &argLoc = TemplateArgList[0];
      const TemplateArgument &arg = argLoc.getArgument();
      DXASSERT(arg.getKind() == TemplateArgument::ArgKind::Type,
               "Geometry stream requires type template arg 0");
      m_sema->RequireCompleteType(argLoc.getLocation(), arg.getAsType(),
                                  diag::err_typecheck_decl_incomplete_type);
      CXXRecordDecl *Decl = arg.getAsType()->getAsCXXRecordDecl();
      if (Decl && !Decl->isCompleteDefinition())
        return true;
      const TypeDiagContext DiagContext = TypeDiagContext::GeometryStreams;
      if (DiagnoseTypeElements(*m_sema, argLoc.getLocation(), arg.getAsType(),
                               DiagContext, DiagContext))
        return true;
    }

    bool isMatrix = Template->getCanonicalDecl() ==
                    m_matrixTemplateDecl->getCanonicalDecl();
    bool isVector = Template->getCanonicalDecl() ==
                    m_vectorTemplateDecl->getCanonicalDecl();
    bool requireScalar = isMatrix || isVector;

    // Check constraints on the type.
    for (unsigned int i = 0; i < TemplateArgList.size(); i++) {
      const TemplateArgumentLoc &argLoc = TemplateArgList[i];
      SourceLocation argSrcLoc = argLoc.getLocation();
      const TemplateArgument &arg = argLoc.getArgument();
      if (arg.getKind() == TemplateArgument::ArgKind::Type) {
        QualType argType = arg.getAsType();
        // Skip dependent types.  Types will be checked later, when concrete.
        if (!argType->isDependentType()) {
          if (!IsValidTemplateArgumentType(argSrcLoc, argType, requireScalar)) {
            // NOTE: IsValidTemplateArgumentType emits its own diagnostics
            return true;
          }
          if (ResAttr && IsTyped(ResAttr->getResKind())) {
            // Check vectors for being too large.
            if (IsVectorType(m_sema, argType)) {
              unsigned NumElt = hlsl::GetElementCount(argType);
              QualType VecEltTy = hlsl::GetHLSLVecElementType(argType);
              if (NumElt > 4 ||
                  NumElt * m_sema->getASTContext().getTypeSize(VecEltTy) >
                      4 * 32) {
                m_sema->Diag(
                    argLoc.getLocation(),
                    diag::
                        err_hlsl_unsupported_typedbuffer_template_parameter_size);
                return true;
              }
              // Disallow non-vectors and non-scalars entirely.
            } else if (!IsScalarType(argType)) {
              m_sema->Diag(
                  argLoc.getLocation(),
                  diag::err_hlsl_unsupported_typedbuffer_template_parameter);
              return true;
            }
          }
        }
      } else if (arg.getKind() == TemplateArgument::ArgKind::Expression) {
        if (isMatrix || isVector) {
          Expr *expr = arg.getAsExpr();
          llvm::APSInt constantResult;
          if (expr != nullptr &&
              expr->isIntegerConstantExpr(constantResult, *m_context)) {
            if (CheckRangedTemplateArgument(argSrcLoc, constantResult,
                                            isVector))
              return true;
          }
        }
      } else if (arg.getKind() == TemplateArgument::ArgKind::Integral) {
        if (isMatrix || isVector) {
          llvm::APSInt Val = arg.getAsIntegral();
          if (CheckRangedTemplateArgument(argSrcLoc, Val, isVector))
            return true;
        }
      }
    }

    return false;
  }

  FindStructBasicTypeResult
  FindStructBasicType(DeclContext *functionDeclContext);

  /// <summary>Finds the table of intrinsics for the declaration context of a
  /// member function.</summary> <param name="functionDeclContext">Declaration
  /// context of function.</param> <param name="name">After execution, the name
  /// of the object to which the table applies.</param> <param
  /// name="intrinsics">After execution, the intrinsic table.</param> <param
  /// name="intrinsicCount">After execution, the count of elements in the
  /// intrinsic table.</param>
  void FindIntrinsicTable(DeclContext *functionDeclContext, const char **name,
                          const HLSL_INTRINSIC **intrinsics,
                          size_t *intrinsicCount);

  /// <summary>Deduces the template arguments by comparing the argument types
  /// and the HLSL intrinsic tables.</summary> <param
  /// name="FunctionTemplate">The declaration for the function template being
  /// deduced.</param> <param name="ExplicitTemplateArgs">Explicitly-provided
  /// template arguments. Should be empty for an HLSL program.</param> <param
  /// name="Args">Array of expressions being used as arguments.</param> <param
  /// name="Specialization">The declaration for the resolved
  /// specialization.</param> <param name="Info">Provides information about an
  /// attempted template argument deduction.</param> <returns>The result of the
  /// template deduction, TDK_Invalid if no HLSL-specific processing
  /// done.</returns>
  Sema::TemplateDeductionResult DeduceTemplateArgumentsForHLSL(
      FunctionTemplateDecl *FunctionTemplate,
      TemplateArgumentListInfo *ExplicitTemplateArgs, ArrayRef<Expr *> Args,
      FunctionDecl *&Specialization, TemplateDeductionInfo &Info);

  clang::OverloadingResult
  GetBestViableFunction(clang::SourceLocation Loc,
                        clang::OverloadCandidateSet &set,
                        clang::OverloadCandidateSet::iterator &Best);

  /// <summary>
  /// Initializes the specified <paramref name="initSequence" /> describing how
  /// <paramref name="Entity" /> is initialized with <paramref name="Args" />.
  /// </summary>
  /// <param name="Entity">Entity being initialized; a variable, return result,
  /// etc.</param> <param name="Kind">Kind of initialization: copying,
  /// list-initializing, constructing, etc.</param> <param name="Args">Arguments
  /// to the initialization.</param> <param name="TopLevelOfInitList">Whether
  /// this is the top-level of an initialization list.</param> <param
  /// name="initSequence">Initialization sequence description to
  /// initialize.</param>
  void InitializeInitSequenceForHLSL(const InitializedEntity &Entity,
                                     const InitializationKind &Kind,
                                     MultiExprArg Args, bool TopLevelOfInitList,
                                     InitializationSequence *initSequence);

  /// <summary>
  /// Checks whether the specified conversion occurs to a type of idential
  /// element type but less elements.
  /// </summary>
  /// <remarks>This is an important case because a cast of this type does not
  /// turn an lvalue into an rvalue.</remarks>
  bool IsConversionToLessOrEqualElements(const ExprResult &sourceExpr,
                                         const QualType &targetType,
                                         bool explicitConversion);

  /// <summary>
  /// Checks whether the specified conversion occurs to a type of idential
  /// element type but less elements.
  /// </summary>
  /// <remarks>This is an important case because a cast of this type does not
  /// turn an lvalue into an rvalue.</remarks>
  bool IsConversionToLessOrEqualElements(const QualType &sourceType,
                                         const QualType &targetType,
                                         bool explicitConversion);

  /// <summary>Performs a member lookup on the specified BaseExpr if it's a
  /// matrix.</summary> <param name="BaseExpr">Base expression for member
  /// access.</param> <param name="MemberName">Name of member to look
  /// up.</param> <param name="IsArrow">Whether access is through arrow (a->b)
  /// rather than period (a.b).</param> <param name="OpLoc">Location of access
  /// operand.</param> <param name="MemberLoc">Location of member.</param>
  /// <returns>Result of lookup operation.</returns>
  ExprResult LookupMatrixMemberExprForHLSL(Expr &BaseExpr,
                                           DeclarationName MemberName,
                                           bool IsArrow, SourceLocation OpLoc,
                                           SourceLocation MemberLoc);

  /// <summary>Performs a member lookup on the specified BaseExpr if it's a
  /// vector.</summary> <param name="BaseExpr">Base expression for member
  /// access.</param> <param name="MemberName">Name of member to look
  /// up.</param> <param name="IsArrow">Whether access is through arrow (a->b)
  /// rather than period (a.b).</param> <param name="OpLoc">Location of access
  /// operand.</param> <param name="MemberLoc">Location of member.</param>
  /// <returns>Result of lookup operation.</returns>
  ExprResult LookupVectorMemberExprForHLSL(Expr &BaseExpr,
                                           DeclarationName MemberName,
                                           bool IsArrow, SourceLocation OpLoc,
                                           SourceLocation MemberLoc);

  /// <summary>Performs a member lookup on the specified BaseExpr if it's an
  /// array.</summary> <param name="BaseExpr">Base expression for member
  /// access.</param> <param name="MemberName">Name of member to look
  /// up.</param> <param name="IsArrow">Whether access is through arrow (a->b)
  /// rather than period (a.b).</param> <param name="OpLoc">Location of access
  /// operand.</param> <param name="MemberLoc">Location of member.</param>
  /// <returns>Result of lookup operation.</returns>
  ExprResult LookupArrayMemberExprForHLSL(Expr &BaseExpr,
                                          DeclarationName MemberName,
                                          bool IsArrow, SourceLocation OpLoc,
                                          SourceLocation MemberLoc);

  /// <summary>If E is a scalar, converts it to a 1-element vector. If E is a
  /// Constant/TextureBuffer<T>, converts it to const T.</summary>
  /// <param name="E">Expression to convert.</param>
  /// <returns>The result of the conversion; or E if the type is not a
  /// scalar.</returns>
  ExprResult MaybeConvertMemberAccess(clang::Expr *E);

  clang::Expr *HLSLImpCastToScalar(clang::Sema *self, clang::Expr *From,
                                   ArTypeObjectKind FromShape,
                                   ArBasicKind EltKind);
  clang::ExprResult
  PerformHLSLConversion(clang::Expr *From, clang::QualType targetType,
                        const clang::StandardConversionSequence &SCS,
                        clang::Sema::CheckedConversionKind CCK);

  /// <summary>Diagnoses an error when precessing the specified type if nesting
  /// is too deep.</summary>
  void ReportUnsupportedTypeNesting(SourceLocation loc, QualType type);

  /// <summary>
  /// Checks if a static cast can be performed, and performs it if possible.
  /// </summary>
  /// <param name="SrcExpr">Expression to cast.</param>
  /// <param name="DestType">Type to cast SrcExpr to.</param>
  /// <param name="CCK">Kind of conversion: implicit, C-style, functional,
  /// other.</param> <param name="OpRange">Source range for the cast
  /// operation.</param> <param name="msg">Error message from the diag::*
  /// enumeration to fail with; zero to suppress messages.</param> <param
  /// name="Kind">The kind of operation required for a conversion.</param>
  /// <param name="BasePath">A simple array of base specifiers.</param>
  /// <param name="ListInitialization">Whether the cast is in the context of a
  /// list initialization.</param> <param name="SuppressWarnings">Whether
  /// warnings should be omitted.</param> <param name="SuppressErrors">Whether
  /// errors should be omitted.</param>
  bool TryStaticCastForHLSL(ExprResult &SrcExpr, QualType DestType,
                            Sema::CheckedConversionKind CCK,
                            const SourceRange &OpRange, unsigned &msg,
                            CastKind &Kind, CXXCastPath &BasePath,
                            bool ListInitialization, bool SuppressWarnings,
                            bool SuppressErrors,
                            StandardConversionSequence *standard);

  /// <summary>
  /// Checks if a subscript index argument can be initialized from the given
  /// expression.
  /// </summary>
  /// <param name="SrcExpr">Source expression used as argument.</param>
  /// <param name="DestType">Parameter type to initialize.</param>
  /// <remarks>
  /// Rules for subscript index initialization follow regular implicit casting
  /// rules, with the exception that no changes in arity are allowed (i.e., int2
  /// can become uint2, but uint or uint3 cannot).
  /// </remarks>
  ImplicitConversionSequence
  TrySubscriptIndexInitialization(clang::Expr *SrcExpr,
                                  clang::QualType DestType);

  void CompleteType(TagDecl *Tag) override {
    if (Tag->isCompleteDefinition() || !isa<CXXRecordDecl>(Tag))
      return;

    CXXRecordDecl *recordDecl = cast<CXXRecordDecl>(Tag);
    if (auto TDecl = dyn_cast<ClassTemplateSpecializationDecl>(recordDecl)) {
      recordDecl = TDecl->getSpecializedTemplate()->getTemplatedDecl();

      if (recordDecl->isCompleteDefinition())
        return;
    }

    int idx = FindObjectBasicKindIndex(recordDecl);
    // Not object type.
    if (idx == -1)
      return;

    ArBasicKind kind = g_ArBasicKindsAsTypes[idx];
    uint8_t templateArgCount = g_ArBasicKindsTemplateCount[idx];

    int startDepth = 0;

    if (templateArgCount > 0) {
      DXASSERT(templateArgCount <= 3, "otherwise a new case has been added");
      ClassTemplateDecl *typeDecl = recordDecl->getDescribedClassTemplate();
      AddObjectSubscripts(kind, typeDecl, recordDecl,
                          g_ArBasicKindsSubscripts[idx]);
      startDepth = 1;
    }

    AddObjectMethods(kind, recordDecl, startDepth);
    recordDecl->completeDefinition();
  }

  FunctionDecl *AddHLSLIntrinsicMethod(LPCSTR tableName, LPCSTR lowering,
                                       const HLSL_INTRINSIC *intrinsic,
                                       FunctionTemplateDecl *FunctionTemplate,
                                       ArrayRef<Expr *> Args,
                                       QualType *parameterTypes,
                                       size_t parameterTypeCount) {
    DXASSERT_NOMSG(intrinsic != nullptr);
    DXASSERT_NOMSG(FunctionTemplate != nullptr);
    DXASSERT_NOMSG(parameterTypes != nullptr);
    DXASSERT(parameterTypeCount >= 1,
             "otherwise caller didn't initialize - there should be at least a "
             "void return type");

    const bool IsStatic = IsStaticMember(intrinsic);

    // Create the template arguments.
    SmallVector<TemplateArgument, g_MaxIntrinsicParamCount + 1> templateArgs;
    for (size_t i = 0; i < parameterTypeCount; i++) {
      templateArgs.push_back(TemplateArgument(parameterTypes[i]));
    }

    // Look for an existing specialization.
    void *InsertPos = nullptr;
    FunctionDecl *SpecFunc =
        FunctionTemplate->findSpecialization(templateArgs, InsertPos);
    if (SpecFunc != nullptr) {
      return SpecFunc;
    }

    // Change return type to lvalue reference type for aggregate types
    QualType retTy = parameterTypes[0];
    if (hlsl::IsHLSLAggregateType(retTy))
      parameterTypes[0] = m_context->getLValueReferenceType(retTy);

    // Create a new specialization.
    SmallVector<hlsl::ParameterModifier, g_MaxIntrinsicParamCount> paramMods;
    InitParamMods(intrinsic, paramMods);

    for (unsigned int i = 1; i < parameterTypeCount; i++) {
      // Change out/inout parameter type to rvalue reference type.
      if (paramMods[i - 1].isAnyOut()) {
        parameterTypes[i] =
            m_context->getLValueReferenceType(parameterTypes[i]);
      }
    }

    IntrinsicOp intrinOp = static_cast<IntrinsicOp>(intrinsic->Op);

    if (IsBuiltinTable(tableName) && intrinOp == IntrinsicOp::MOP_SampleBias) {
      // Remove this when update intrinsic table not affect other things.
      // Change vector<float,1> into float for bias.
      const unsigned biasOperandID = 3; // return type, sampler, coord, bias.
      DXASSERT(parameterTypeCount > biasOperandID,
               "else operation was misrecognized");
      if (const ExtVectorType *VecTy =
              hlsl::ConvertHLSLVecMatTypeToExtVectorType(
                  *m_context, parameterTypes[biasOperandID])) {
        if (VecTy->getNumElements() == 1)
          parameterTypes[biasOperandID] = VecTy->getElementType();
      }
    }

    DeclContext *owner = FunctionTemplate->getDeclContext();
    TemplateArgumentList templateArgumentList(
        TemplateArgumentList::OnStackType::OnStack, templateArgs.data(),
        templateArgs.size());
    MultiLevelTemplateArgumentList mlTemplateArgumentList(templateArgumentList);
    TemplateDeclInstantiator declInstantiator(*this->m_sema, owner,
                                              mlTemplateArgumentList);
    FunctionProtoType::ExtProtoInfo EmptyEPI;
    QualType functionType = m_context->getFunctionType(
        parameterTypes[0],
        ArrayRef<QualType>(parameterTypes + 1, parameterTypeCount - 1),
        EmptyEPI, paramMods);
    TypeSourceInfo *TInfo = m_context->CreateTypeSourceInfo(functionType, 0);
    FunctionProtoTypeLoc Proto =
        TInfo->getTypeLoc().getAs<FunctionProtoTypeLoc>();

    SmallVector<ParmVarDecl *, g_MaxIntrinsicParamCount> Params;
    for (unsigned int i = 1; i < parameterTypeCount; i++) {
      // The first parameter in the HLSL intrinsic record is just the intrinsic
      // name and aliases with the 'this' pointer for non-static members. Skip
      // this first parameter for static functions.
      unsigned ParamIdx = IsStatic ? i : i - 1;
      IdentifierInfo *id =
          &m_context->Idents.get(StringRef(intrinsic->pArgs[ParamIdx].pName));
      ParmVarDecl *paramDecl = ParmVarDecl::Create(
          *m_context, nullptr, NoLoc, NoLoc, id, parameterTypes[i], nullptr,
          StorageClass::SC_None, nullptr, paramMods[i - 1]);
      Params.push_back(paramDecl);
    }

    StorageClass SC = IsStatic ? SC_Static : SC_Extern;
    QualType T = TInfo->getType();
    DeclarationNameInfo NameInfo(FunctionTemplate->getDeclName(), NoLoc);
    CXXMethodDecl *method = CXXMethodDecl::Create(
        *m_context, dyn_cast<CXXRecordDecl>(owner), NoLoc, NameInfo, T, TInfo,
        SC, InlineSpecifiedFalse, IsConstexprFalse, NoLoc);

    // Add intrinsic attr
    AddHLSLIntrinsicAttr(method, *m_context, tableName, lowering, intrinsic);

    // Record this function template specialization.
    TemplateArgumentList *argListCopy = TemplateArgumentList::CreateCopy(
        *m_context, templateArgs.data(), templateArgs.size());
    method->setFunctionTemplateSpecialization(FunctionTemplate, argListCopy, 0);

    // Attach the parameters
    for (unsigned P = 0; P < Params.size(); ++P) {
      Params[P]->setOwningFunction(method);
      Proto.setParam(P, Params[P]);
    }
    method->setParams(Params);

    // Adjust access.
    method->setAccess(AccessSpecifier::AS_public);
    FunctionTemplate->setAccess(method->getAccess());

    return method;
  }

  // Overload support.
  UINT64 ScoreCast(QualType leftType, QualType rightType);
  UINT64 ScoreFunction(OverloadCandidateSet::iterator &Cand);
  UINT64 ScoreImplicitConversionSequence(const ImplicitConversionSequence *s);
  unsigned GetNumElements(QualType anyType);
  unsigned GetNumBasicElements(QualType anyType);
  unsigned GetNumConvertCheckElts(QualType leftType, unsigned leftSize,
                                  QualType rightType, unsigned rightSize);
  QualType GetNthElementType(QualType type, unsigned index);
  bool IsPromotion(ArBasicKind leftKind, ArBasicKind rightKind);
  bool IsCast(ArBasicKind leftKind, ArBasicKind rightKind);
  bool IsIntCast(ArBasicKind leftKind, ArBasicKind rightKind);
};

TYPE_CONVERSION_REMARKS HLSLExternalSource::RemarksUnused =
    TYPE_CONVERSION_REMARKS::TYPE_CONVERSION_NONE;
ImplicitConversionKind HLSLExternalSource::ImplicitConversionKindUnused =
    ImplicitConversionKind::ICK_Identity;

// Use this class to flatten a type into HLSL primitives and iterate through
// them.
class FlattenedTypeIterator {
private:
  enum FlattenedIterKind {
    FK_Simple,
    FK_Fields,
    FK_Expressions,
    FK_IncompleteArray,
    FK_Bases,
  };

  // Use this struct to represent a specific point in the tracked tree.
  struct FlattenedTypeTracker {
    QualType Type;      // Type at this position in the tree.
    unsigned int Count; // Count of consecutive types
    CXXRecordDecl::base_class_iterator
        CurrentBase; // Current base for a structure type.
    CXXRecordDecl::base_class_iterator EndBase; // STL-style end of bases.
    RecordDecl::field_iterator
        CurrentField; // Current field in for a structure type.
    RecordDecl::field_iterator EndField; // STL-style end of fields.
    MultiExprArg::iterator CurrentExpr; // Current expression (advanceable for a
                                        // list of expressions).
    MultiExprArg::iterator EndExpr;     // STL-style end of expressions.
    FlattenedIterKind IterKind;         // Kind of tracker.
    bool IsConsidered; // If a FlattenedTypeTracker already been considered.

    FlattenedTypeTracker(QualType type)
        : Type(type), Count(0), CurrentExpr(nullptr),
          IterKind(FK_IncompleteArray), IsConsidered(false) {}
    FlattenedTypeTracker(QualType type, unsigned int count,
                         MultiExprArg::iterator expression)
        : Type(type), Count(count), CurrentExpr(expression),
          IterKind(FK_Simple), IsConsidered(false) {}
    FlattenedTypeTracker(QualType type, RecordDecl::field_iterator current,
                         RecordDecl::field_iterator end)
        : Type(type), Count(0), CurrentField(current), EndField(end),
          CurrentExpr(nullptr), IterKind(FK_Fields), IsConsidered(false) {}
    FlattenedTypeTracker(MultiExprArg::iterator current,
                         MultiExprArg::iterator end)
        : Count(0), CurrentExpr(current), EndExpr(end),
          IterKind(FK_Expressions), IsConsidered(false) {}
    FlattenedTypeTracker(QualType type,
                         CXXRecordDecl::base_class_iterator current,
                         CXXRecordDecl::base_class_iterator end)
        : Count(0), CurrentBase(current), EndBase(end), CurrentExpr(nullptr),
          IterKind(FK_Bases), IsConsidered(false) {}

    /// <summary>Gets the current expression if one is available.</summary>
    Expr *getExprOrNull() const { return CurrentExpr ? *CurrentExpr : nullptr; }
    /// <summary>Replaces the current expression.</summary>
    void replaceExpr(Expr *e) { *CurrentExpr = e; }
  };

  HLSLExternalSource &m_source; // Source driving the iteration.
  SmallVector<FlattenedTypeTracker, 4>
      m_typeTrackers; // Active stack of trackers.
  bool m_draining; // Whether the iterator is meant to drain (will not generate
                   // new elements in incomplete arrays).
  bool m_springLoaded; // Whether the current element has been set up by an
                       // incomplete array but hasn't been used yet.
  unsigned int
      m_incompleteCount; // The number of elements in an incomplete array.
  size_t m_typeDepth;    // Depth of type analysis, to avoid stack overflows.
  QualType m_firstType;  // Name of first type found, used for diagnostics.
  SourceLocation m_loc;  // Location used for diagnostics.
  static const size_t MaxTypeDepth = 100;

  void advanceLeafTracker();
  /// <summary>Consumes leaves.</summary>
  void consumeLeaf();
  /// <summary>Considers whether the leaf has a usable expression without
  /// consuming anything.</summary>
  bool considerLeaf();
  /// <summary>Pushes a tracker for the specified expression; returns true if
  /// there is something to evaluate.</summary>
  bool pushTrackerForExpression(MultiExprArg::iterator expression);
  /// <summary>Pushes a tracker for the specified type; returns true if there is
  /// something to evaluate.</summary>
  bool pushTrackerForType(QualType type, MultiExprArg::iterator expression);

public:
  /// <summary>Constructs a FlattenedTypeIterator for the specified
  /// type.</summary>
  FlattenedTypeIterator(SourceLocation loc, QualType type,
                        HLSLExternalSource &source);
  /// <summary>Constructs a FlattenedTypeIterator for the specified
  /// arguments.</summary>
  FlattenedTypeIterator(SourceLocation loc, MultiExprArg args,
                        HLSLExternalSource &source);

  /// <summary>Gets the current element in the flattened type
  /// hierarchy.</summary>
  QualType getCurrentElement() const;
  /// <summary>Get the number of repeated current elements.</summary>
  unsigned int getCurrentElementSize() const;
  /// <summary>Gets the current element's Iterkind.</summary>
  FlattenedIterKind getCurrentElementKind() const {
    return m_typeTrackers.back().IterKind;
  }
  /// <summary>Checks whether the iterator has a current element type to
  /// report.</summary>
  bool hasCurrentElement() const;
  /// <summary>Consumes count elements on this iterator.</summary>
  void advanceCurrentElement(unsigned int count);
  /// <summary>Counts the remaining elements in this iterator (consuming all
  /// elements).</summary>
  unsigned int countRemaining();
  /// <summary>Gets the current expression if one is available.</summary>
  Expr *getExprOrNull() const { return m_typeTrackers.back().getExprOrNull(); }
  /// <summary>Replaces the current expression.</summary>
  void replaceExpr(Expr *e) { m_typeTrackers.back().replaceExpr(e); }

  struct ComparisonResult {
    unsigned int LeftCount;
    unsigned int RightCount;

    /// <summary>Whether elements from right sequence are identical into left
    /// sequence elements.</summary>
    bool AreElementsEqual;

    /// <summary>Whether elements from right sequence can be converted into left
    /// sequence elements.</summary>
    bool CanConvertElements;

    /// <summary>Whether the elements can be converted and the sequences have
    /// the same length.</summary>
    bool IsConvertibleAndEqualLength() const {
      return CanConvertElements && LeftCount == RightCount;
    }

    /// <summary>Whether the elements can be converted but the left-hand
    /// sequence is longer.</summary>
    bool IsConvertibleAndLeftLonger() const {
      return CanConvertElements && LeftCount > RightCount;
    }

    bool IsRightLonger() const { return RightCount > LeftCount; }

    bool IsEqualLength() const { return LeftCount == RightCount; }
  };

  static ComparisonResult CompareIterators(HLSLExternalSource &source,
                                           SourceLocation loc,
                                           FlattenedTypeIterator &leftIter,
                                           FlattenedTypeIterator &rightIter);
  static ComparisonResult CompareTypes(HLSLExternalSource &source,
                                       SourceLocation leftLoc,
                                       SourceLocation rightLoc, QualType left,
                                       QualType right);
  // Compares the arguments to initialize the left type, modifying them if
  // necessary.
  static ComparisonResult CompareTypesForInit(HLSLExternalSource &source,
                                              QualType left, MultiExprArg args,
                                              SourceLocation leftLoc,
                                              SourceLocation rightLoc);
};

static QualType GetFirstElementTypeFromDecl(const Decl *decl) {
  const ClassTemplateSpecializationDecl *specialization =
      dyn_cast<ClassTemplateSpecializationDecl>(decl);
  if (specialization) {
    const TemplateArgumentList &list = specialization->getTemplateArgs();
    if (list.size()) {
      if (list[0].getKind() == TemplateArgument::ArgKind::Type)
        return list[0].getAsType();
    }
  }

  return QualType();
}

void HLSLExternalSource::AddBaseTypes() {
  DXASSERT(m_baseTypes[HLSLScalarType_unknown].isNull(),
           "otherwise unknown was initialized to an actual type");
  m_baseTypes[HLSLScalarType_bool] = m_context->BoolTy;
  m_baseTypes[HLSLScalarType_int] = m_context->IntTy;
  m_baseTypes[HLSLScalarType_uint] = m_context->UnsignedIntTy;
  m_baseTypes[HLSLScalarType_dword] = m_context->UnsignedIntTy;
  m_baseTypes[HLSLScalarType_half] = m_context->getLangOpts().UseMinPrecision
                                         ? m_context->HalfFloatTy
                                         : m_context->HalfTy;
  m_baseTypes[HLSLScalarType_float] = m_context->FloatTy;
  m_baseTypes[HLSLScalarType_double] = m_context->DoubleTy;
  m_baseTypes[HLSLScalarType_float_min10] = m_context->Min10FloatTy;
  m_baseTypes[HLSLScalarType_float_min16] = m_context->Min16FloatTy;
  m_baseTypes[HLSLScalarType_int_min12] = m_context->Min12IntTy;
  m_baseTypes[HLSLScalarType_int_min16] = m_context->Min16IntTy;
  m_baseTypes[HLSLScalarType_uint_min16] = m_context->Min16UIntTy;
  m_baseTypes[HLSLScalarType_int8_4packed] = m_context->Int8_4PackedTy;
  m_baseTypes[HLSLScalarType_uint8_4packed] = m_context->UInt8_4PackedTy;
  m_baseTypes[HLSLScalarType_float_lit] = m_context->LitFloatTy;
  m_baseTypes[HLSLScalarType_int_lit] = m_context->LitIntTy;
  m_baseTypes[HLSLScalarType_int16] = m_context->ShortTy;
  m_baseTypes[HLSLScalarType_int32] = m_context->IntTy;
  m_baseTypes[HLSLScalarType_int64] = m_context->LongLongTy;
  m_baseTypes[HLSLScalarType_uint16] = m_context->UnsignedShortTy;
  m_baseTypes[HLSLScalarType_uint32] = m_context->UnsignedIntTy;
  m_baseTypes[HLSLScalarType_uint64] = m_context->UnsignedLongLongTy;
  m_baseTypes[HLSLScalarType_float16] = m_context->HalfTy;
  m_baseTypes[HLSLScalarType_float32] = m_context->FloatTy;
  m_baseTypes[HLSLScalarType_float64] = m_context->DoubleTy;
}

void HLSLExternalSource::AddHLSLScalarTypes() {
  DXASSERT(m_scalarTypes[HLSLScalarType_unknown].isNull(),
           "otherwise unknown was initialized to an actual type");
  m_scalarTypes[HLSLScalarType_bool] = m_baseTypes[HLSLScalarType_bool];
  m_scalarTypes[HLSLScalarType_int] = m_baseTypes[HLSLScalarType_int];
  m_scalarTypes[HLSLScalarType_float] = m_baseTypes[HLSLScalarType_float];
  m_scalarTypes[HLSLScalarType_double] = m_baseTypes[HLSLScalarType_double];
  m_scalarTypes[HLSLScalarType_float_lit] =
      m_baseTypes[HLSLScalarType_float_lit];
  m_scalarTypes[HLSLScalarType_int_lit] = m_baseTypes[HLSLScalarType_int_lit];
}

void HLSLExternalSource::AddHLSLStringType() {
  m_hlslStringType = m_context->HLSLStringTy;
}

FunctionDecl *HLSLExternalSource::AddSubscriptSpecialization(
    FunctionTemplateDecl *functionTemplate, QualType objectElement,
    const FindStructBasicTypeResult &findResult) {
  DXASSERT_NOMSG(functionTemplate != nullptr);
  DXASSERT_NOMSG(!objectElement.isNull());
  DXASSERT_NOMSG(findResult.Found());
  DXASSERT(g_ArBasicKindsSubscripts[findResult.BasicKindsAsTypeIndex]
                   .SubscriptCardinality > 0,
           "otherwise the template shouldn't have an operator[] that the "
           "caller is trying to specialize");

  // Subscript is templated only on its return type.

  // Create the template argument.
  bool isReadWrite = GetBasicKindProps(findResult.Kind) & BPROP_RWBUFFER;
  QualType resultType = objectElement;
  if (!isReadWrite)
    resultType = m_context->getConstType(resultType);
  resultType = m_context->getLValueReferenceType(resultType);

  TemplateArgument templateArgument(resultType);
  unsigned subscriptCardinality =
      g_ArBasicKindsSubscripts[findResult.BasicKindsAsTypeIndex]
          .SubscriptCardinality;
  QualType subscriptIndexType =
      subscriptCardinality == 1
          ? m_context->UnsignedIntTy
          : NewSimpleAggregateType(AR_TOBJ_VECTOR, AR_BASIC_UINT32, 0, 1,
                                   subscriptCardinality);

  // Look for an existing specialization.
  void *InsertPos = nullptr;
  FunctionDecl *SpecFunc = functionTemplate->findSpecialization(
      ArrayRef<TemplateArgument>(&templateArgument, 1), InsertPos);
  if (SpecFunc != nullptr) {
    return SpecFunc;
  }

  // Create a new specialization.
  DeclContext *owner = functionTemplate->getDeclContext();
  TemplateArgumentList templateArgumentList(
      TemplateArgumentList::OnStackType::OnStack, &templateArgument, 1);
  MultiLevelTemplateArgumentList mlTemplateArgumentList(templateArgumentList);
  TemplateDeclInstantiator declInstantiator(*this->m_sema, owner,
                                            mlTemplateArgumentList);
  const FunctionType *templateFnType =
      functionTemplate->getTemplatedDecl()->getType()->getAs<FunctionType>();
  const FunctionProtoType *protoType =
      dyn_cast<FunctionProtoType>(templateFnType);
  FunctionProtoType::ExtProtoInfo templateEPI = protoType->getExtProtoInfo();
  QualType functionType = m_context->getFunctionType(
      resultType, subscriptIndexType, templateEPI, None);
  TypeSourceInfo *TInfo = m_context->CreateTypeSourceInfo(functionType, 0);
  FunctionProtoTypeLoc Proto =
      TInfo->getTypeLoc().getAs<FunctionProtoTypeLoc>();

  IdentifierInfo *id = &m_context->Idents.get(StringRef("index"));
  ParmVarDecl *indexerParam = ParmVarDecl::Create(
      *m_context, nullptr, NoLoc, NoLoc, id, subscriptIndexType, nullptr,
      StorageClass::SC_None, nullptr);

  QualType T = TInfo->getType();
  DeclarationNameInfo NameInfo(functionTemplate->getDeclName(), NoLoc);
  CXXMethodDecl *method = CXXMethodDecl::Create(
      *m_context, dyn_cast<CXXRecordDecl>(owner), NoLoc, NameInfo, T, TInfo,
      SC_Extern, InlineSpecifiedFalse, IsConstexprFalse, NoLoc);

  // Add subscript attribute
  AddHLSLSubscriptAttr(method, *m_context, HLSubscriptOpcode::DefaultSubscript);

  // Record this function template specialization.
  method->setFunctionTemplateSpecialization(
      functionTemplate,
      TemplateArgumentList::CreateCopy(*m_context, &templateArgument, 1), 0);

  // Attach the parameters
  indexerParam->setOwningFunction(method);
  Proto.setParam(0, indexerParam);
  method->setParams(ArrayRef<ParmVarDecl *>(indexerParam));

  // Adjust access.
  method->setAccess(AccessSpecifier::AS_public);
  functionTemplate->setAccess(method->getAccess());

  return method;
}

/// <summary>
/// This routine combines Source into Target. If you have a symmetric operation
/// and want to treat either side equally you should call it twice, swapping the
/// parameter order.
/// </summary>
static bool CombineObjectTypes(ArBasicKind Target, ArBasicKind Source,
                               ArBasicKind *pCombined) {
  if (Target == Source) {
    AssignOpt(Target, pCombined);
    return true;
  }

  if (Source == AR_OBJECT_NULL) {
    // NULL is valid for any object type.
    AssignOpt(Target, pCombined);
    return true;
  }

  switch (Target) {
  AR_BASIC_ROBJECT_CASES:
    if (Source == AR_OBJECT_STATEBLOCK) {
      AssignOpt(Target, pCombined);
      return true;
    }
    break;

  AR_BASIC_TEXTURE_CASES:

  AR_BASIC_NON_CMP_SAMPLER_CASES:
    if (Source == AR_OBJECT_SAMPLER || Source == AR_OBJECT_STATEBLOCK) {
      AssignOpt(Target, pCombined);
      return true;
    }
    break;

  case AR_OBJECT_SAMPLERCOMPARISON:
    if (Source == AR_OBJECT_STATEBLOCK) {
      AssignOpt(Target, pCombined);
      return true;
    }
    break;
  default:
    // Not a combinable target.
    break;
  }

  AssignOpt(AR_BASIC_UNKNOWN, pCombined);
  return false;
}

static ArBasicKind LiteralToConcrete(Expr *litExpr,
                                     HLSLExternalSource *pHLSLExternalSource) {
  if (IntegerLiteral *intLit = dyn_cast<IntegerLiteral>(litExpr)) {
    llvm::APInt val = intLit->getValue();
    unsigned width = val.getActiveBits();
    bool isNeg = val.isNegative();
    if (isNeg) {
      // Signed.
      if (width <= 32)
        return ArBasicKind::AR_BASIC_INT32;
      else
        return ArBasicKind::AR_BASIC_INT64;
    } else {
      // Unsigned.
      if (width <= 32)
        return ArBasicKind::AR_BASIC_UINT32;
      else
        return ArBasicKind::AR_BASIC_UINT64;
    }
  } else if (FloatingLiteral *floatLit = dyn_cast<FloatingLiteral>(litExpr)) {
    llvm::APFloat val = floatLit->getValue();
    unsigned width = val.getSizeInBits(val.getSemantics());
    if (width <= 16)
      return ArBasicKind::AR_BASIC_FLOAT16;
    else if (width <= 32)
      return ArBasicKind::AR_BASIC_FLOAT32;
    else
      return AR_BASIC_FLOAT64;
  } else if (UnaryOperator *UO = dyn_cast<UnaryOperator>(litExpr)) {
    ArBasicKind kind = LiteralToConcrete(UO->getSubExpr(), pHLSLExternalSource);
    if (UO->getOpcode() == UnaryOperator::Opcode::UO_Minus) {
      if (kind == ArBasicKind::AR_BASIC_UINT32)
        kind = ArBasicKind::AR_BASIC_INT32;
      else if (kind == ArBasicKind::AR_BASIC_UINT64)
        kind = ArBasicKind::AR_BASIC_INT64;
    }
    return kind;
  } else if (HLSLVectorElementExpr *VEE =
                 dyn_cast<HLSLVectorElementExpr>(litExpr)) {
    return pHLSLExternalSource->GetTypeElementKind(VEE->getType());
  } else if (BinaryOperator *BO = dyn_cast<BinaryOperator>(litExpr)) {
    ArBasicKind kind = LiteralToConcrete(BO->getLHS(), pHLSLExternalSource);
    ArBasicKind kind1 = LiteralToConcrete(BO->getRHS(), pHLSLExternalSource);
    CombineBasicTypes(kind, kind1, &kind);
    return kind;
  } else if (ParenExpr *PE = dyn_cast<ParenExpr>(litExpr)) {
    ArBasicKind kind = LiteralToConcrete(PE->getSubExpr(), pHLSLExternalSource);
    return kind;
  } else if (ConditionalOperator *CO = dyn_cast<ConditionalOperator>(litExpr)) {
    ArBasicKind kind = LiteralToConcrete(CO->getLHS(), pHLSLExternalSource);
    ArBasicKind kind1 = LiteralToConcrete(CO->getRHS(), pHLSLExternalSource);
    CombineBasicTypes(kind, kind1, &kind);
    return kind;
  } else if (ImplicitCastExpr *IC = dyn_cast<ImplicitCastExpr>(litExpr)) {
    // Use target Type for cast.
    ArBasicKind kind = pHLSLExternalSource->GetTypeElementKind(IC->getType());
    return kind;
  } else {
    // Could only be function call.
    CallExpr *CE = cast<CallExpr>(litExpr);
    // TODO: calculate the function call result.
    if (CE->getNumArgs() == 1)
      return LiteralToConcrete(CE->getArg(0), pHLSLExternalSource);
    else {
      ArBasicKind kind = LiteralToConcrete(CE->getArg(0), pHLSLExternalSource);
      for (unsigned i = 1; i < CE->getNumArgs(); i++) {
        ArBasicKind kindI =
            LiteralToConcrete(CE->getArg(i), pHLSLExternalSource);
        CombineBasicTypes(kind, kindI, &kind);
      }
      return kind;
    }
  }
}

static bool SearchTypeInTable(ArBasicKind kind, const ArBasicKind *pCT) {
  while (AR_BASIC_UNKNOWN != *pCT && AR_BASIC_NOCAST != *pCT) {
    if (kind == *pCT)
      return true;
    pCT++;
  }
  return false;
}

static ArBasicKind
ConcreteLiteralType(Expr *litExpr, ArBasicKind kind,
                    unsigned uLegalComponentTypes,
                    HLSLExternalSource *pHLSLExternalSource) {
  const ArBasicKind *pCT = g_LegalIntrinsicCompTypes[uLegalComponentTypes];
  ArBasicKind defaultKind = *pCT;
  // Use first none literal kind as defaultKind.
  while (AR_BASIC_UNKNOWN != *pCT && AR_BASIC_NOCAST != *pCT) {
    ArBasicKind kind = *pCT;
    pCT++;
    // Skip literal type.
    if (kind == AR_BASIC_LITERAL_INT || kind == AR_BASIC_LITERAL_FLOAT)
      continue;
    defaultKind = kind;
    break;
  }

  ArBasicKind litKind = LiteralToConcrete(litExpr, pHLSLExternalSource);

  if (kind == AR_BASIC_LITERAL_INT) {
    // Search for match first.
    // For literal arg which don't affect return type, the search should always
    // success. Unless use literal int on a float parameter.
    if (SearchTypeInTable(litKind,
                          g_LegalIntrinsicCompTypes[uLegalComponentTypes]))
      return litKind;

    // Return the default.
    return defaultKind;
  } else {
    // Search for float32 first.
    if (SearchTypeInTable(AR_BASIC_FLOAT32,
                          g_LegalIntrinsicCompTypes[uLegalComponentTypes]))
      return AR_BASIC_FLOAT32;
    // Search for float64.
    if (SearchTypeInTable(AR_BASIC_FLOAT64,
                          g_LegalIntrinsicCompTypes[uLegalComponentTypes]))
      return AR_BASIC_FLOAT64;

    // return default.
    return defaultKind;
  }
}

bool HLSLExternalSource::IsValidObjectElement(LPCSTR tableName,
                                              const IntrinsicOp op,
                                              QualType objectElement) {
  // Only meant to exclude builtins, assume others are fine
  if (!IsBuiltinTable(tableName))
    return true;
  switch (op) {
  case IntrinsicOp::MOP_Sample:
  case IntrinsicOp::MOP_SampleBias:
  case IntrinsicOp::MOP_SampleCmp:
  case IntrinsicOp::MOP_SampleCmpLevel:
  case IntrinsicOp::MOP_SampleCmpLevelZero:
  case IntrinsicOp::MOP_SampleGrad:
  case IntrinsicOp::MOP_SampleLevel: {
    ArBasicKind kind = GetTypeElementKind(objectElement);
    UINT uBits = GET_BPROP_BITS(kind);
    if (IS_BASIC_FLOAT(kind) && uBits != BPROP_BITS64)
      return true;
    // 6.7 adds UINT sampler support
    if (IS_BASIC_UINT(kind) || IS_BASIC_SINT(kind)) {
      bool IsSampleC = (op == IntrinsicOp::MOP_SampleCmp ||
                        op == IntrinsicOp::MOP_SampleCmpLevel ||
                        op == IntrinsicOp::MOP_SampleCmpLevelZero);
      // SampleCmp* cannot support integer resource.
      if (IsSampleC)
        return false;
      const auto *SM = hlsl::ShaderModel::GetByName(
          m_sema->getLangOpts().HLSLProfile.c_str());
      return SM->IsSM67Plus();
    }
    return false;
  }
  case IntrinsicOp::MOP_GatherRaw: {
    ArBasicKind kind = GetTypeElementKind(objectElement);
    UINT numEles = GetNumElements(objectElement);
    return IS_BASIC_UINT(kind) && numEles == 1;
  } break;
  default:
    return true;
  }
}

bool HLSLExternalSource::MatchArguments(
    const IntrinsicDefIter &cursor, QualType objectType, QualType objectElement,
    QualType functionTemplateTypeArg, ArrayRef<Expr *> Args,
    std::vector<QualType> *argTypesVector, size_t &badArgIdx) {
  const HLSL_INTRINSIC *pIntrinsic = *cursor;
  LPCSTR tableName = cursor.GetTableName();
  IntrinsicOp builtinOp = IntrinsicOp::Num_Intrinsics;
  if (IsBuiltinTable(tableName))
    builtinOp = static_cast<IntrinsicOp>(pIntrinsic->Op);

  DXASSERT_NOMSG(pIntrinsic != nullptr);
  DXASSERT_NOMSG(argTypesVector != nullptr);
  std::vector<QualType> &argTypes = *argTypesVector;
  argTypes.clear();
  const bool isVariadic = IsVariadicIntrinsicFunction(pIntrinsic);

  static const uint32_t UnusedSize = std::numeric_limits<uint32_t>::max();
  static const uint32_t MaxIntrinsicArgs = g_MaxIntrinsicParamCount + 1;
  assert(MaxIntrinsicArgs < std::numeric_limits<uint8_t>::max() &&
         "This should be a pretty small number");
#define CAB(cond, arg)                                                         \
  {                                                                            \
    if (!(cond)) {                                                             \
      badArgIdx = (arg);                                                       \
      return false;                                                            \
    }                                                                          \
  }

  ArTypeObjectKind
      Template[MaxIntrinsicArgs]; // Template type for each argument,
                                  // AR_TOBJ_UNKNOWN if unspecified.
  ArBasicKind
      ComponentType[MaxIntrinsicArgs]; // Component type for each argument,
                                       // AR_BASIC_UNKNOWN if unspecified.
  UINT uSpecialSize[IA_SPECIAL_SLOTS]; // row/col matching types, UnusedSize
                                       // if unspecified.
  badArgIdx = MaxIntrinsicArgs;

  // Reset infos
  std::fill(Template, Template + _countof(Template), AR_TOBJ_UNKNOWN);
  std::fill(ComponentType, ComponentType + _countof(ComponentType),
            AR_BASIC_UNKNOWN);
  std::fill(uSpecialSize, uSpecialSize + _countof(uSpecialSize), UnusedSize);

  const unsigned retArgIdx = 0;
  unsigned retTypeIdx = pIntrinsic->pArgs[retArgIdx].uComponentTypeId;

  // Populate the template for each argument.
  ArrayRef<Expr *>::iterator iterArg = Args.begin();
  ArrayRef<Expr *>::iterator end = Args.end();
  size_t iArg = 1;
  for (; iterArg != end; ++iterArg) {
    Expr *pCallArg = *iterArg;

    // If vararg is reached, we can break out of this loop.
    if (pIntrinsic->pArgs[iArg].uTemplateId == INTRIN_TEMPLATE_VARARGS)
      break;

    // Check bounds for non-variadic functions.
    if (iArg >= MaxIntrinsicArgs || iArg > pIntrinsic->uNumArgs) {
      // Currently never reached
      badArgIdx = iArg;
      return false;
    }

    const HLSL_INTRINSIC_ARGUMENT *pIntrinsicArg;
    pIntrinsicArg = &pIntrinsic->pArgs[iArg];
    DXASSERT(isVariadic ||
                 pIntrinsicArg->uTemplateId != INTRIN_TEMPLATE_VARARGS,
             "found vararg for non-variadic function");

    QualType pType = pCallArg->getType();
    ArTypeObjectKind TypeInfoShapeKind = GetTypeObjectKind(pType);
    ArBasicKind TypeInfoEltKind = GetTypeElementKind(pType);

    if (pIntrinsicArg->uLegalComponentTypes == LICOMPTYPE_RAYDESC) {
      if (TypeInfoShapeKind == AR_TOBJ_COMPOUND) {
        if (CXXRecordDecl *pDecl = pType->getAsCXXRecordDecl()) {
          int index = FindObjectBasicKindIndex(pDecl);
          if (index != -1 &&
              AR_OBJECT_RAY_DESC == g_ArBasicKindsAsTypes[index]) {
            ++iArg;
            continue;
          }
        }
      }
      m_sema->Diag(pCallArg->getExprLoc(), diag::err_hlsl_ray_desc_required);
      badArgIdx = iArg;
      return false;
    }

    if (pIntrinsicArg->uLegalComponentTypes == LICOMPTYPE_USER_DEFINED_TYPE) {
      DXASSERT_NOMSG(objectElement.isNull());
      QualType Ty = pCallArg->getType();
      // Must be user define type for LICOMPTYPE_USER_DEFINED_TYPE arg.
      if (TypeInfoShapeKind != AR_TOBJ_COMPOUND) {
        m_sema->Diag(pCallArg->getExprLoc(),
                     diag::err_hlsl_no_struct_user_defined_type);
        badArgIdx = iArg;
        return false;
      }
      objectElement = Ty;
      ++iArg;
      continue;
    }

    // If we are a type and templateID requires one, this isn't a match.
    if (pIntrinsicArg->uTemplateId == INTRIN_TEMPLATE_FROM_TYPE ||
        pIntrinsicArg->uTemplateId == INTRIN_TEMPLATE_FROM_FUNCTION) {
      ++iArg;
      continue;
    }

    // Verify TypeInfoEltKind can be cast to something legal for this param
    if (AR_BASIC_UNKNOWN != TypeInfoEltKind) {
      for (const ArBasicKind *pCT =
               g_LegalIntrinsicCompTypes[pIntrinsicArg->uLegalComponentTypes];
           AR_BASIC_UNKNOWN != *pCT; pCT++) {
        if (TypeInfoEltKind == *pCT)
          break;
        else if ((TypeInfoEltKind == AR_BASIC_LITERAL_INT &&
                  *pCT == AR_BASIC_LITERAL_FLOAT) ||
                 (TypeInfoEltKind == AR_BASIC_LITERAL_FLOAT &&
                  *pCT == AR_BASIC_LITERAL_INT))
          break;
        else if (*pCT == AR_BASIC_NOCAST) {
          badArgIdx = std::min(badArgIdx, iArg);
        }
      }
    }

    if (TypeInfoEltKind == AR_BASIC_LITERAL_INT ||
        TypeInfoEltKind == AR_BASIC_LITERAL_FLOAT) {
      bool affectRetType =
          (iArg != retArgIdx && retTypeIdx == pIntrinsicArg->uComponentTypeId);
      // For literal arg which don't affect return type, find concrete type.
      // For literal arg affect return type,
      //   TryEvalIntrinsic in CGHLSLMSFinishCodeGen.cpp will take care of
      //     cases where all arguments are literal.
      //   CombineBasicTypes will cover the rest cases.
      if (!affectRetType) {
        TypeInfoEltKind =
            ConcreteLiteralType(pCallArg, TypeInfoEltKind,
                                pIntrinsicArg->uLegalComponentTypes, this);
      }
    }

    UINT TypeInfoCols = 1;
    UINT TypeInfoRows = 1;
    switch (TypeInfoShapeKind) {
    case AR_TOBJ_MATRIX:
      GetRowsAndCols(pType, TypeInfoRows, TypeInfoCols);
      break;
    case AR_TOBJ_VECTOR:
      TypeInfoCols = GetHLSLVecSize(pType);
      break;
    case AR_TOBJ_BASIC:
    case AR_TOBJ_OBJECT:
    case AR_TOBJ_STRING:
    case AR_TOBJ_ARRAY:
      break;
    default:
      badArgIdx = std::min(badArgIdx, iArg); // no struct, arrays or void
    }

    DXASSERT(
        pIntrinsicArg->uTemplateId < MaxIntrinsicArgs,
        "otherwise intrinsic table was modified and g_MaxIntrinsicParamCount "
        "was not updated (or uTemplateId is out of bounds)");

    // Compare template
    if ((AR_TOBJ_UNKNOWN == Template[pIntrinsicArg->uTemplateId]) ||
        ((AR_TOBJ_SCALAR == Template[pIntrinsicArg->uTemplateId]) &&
         (AR_TOBJ_VECTOR == TypeInfoShapeKind ||
          AR_TOBJ_MATRIX == TypeInfoShapeKind))) {
      // Unrestricted or truncation of tuples to scalars are allowed
      Template[pIntrinsicArg->uTemplateId] = TypeInfoShapeKind;
    } else if (AR_TOBJ_SCALAR == TypeInfoShapeKind) {
      if (AR_TOBJ_SCALAR != Template[pIntrinsicArg->uTemplateId] &&
          AR_TOBJ_VECTOR != Template[pIntrinsicArg->uTemplateId] &&
          AR_TOBJ_MATRIX != Template[pIntrinsicArg->uTemplateId]) {
        // Scalars to tuples can be splatted, scalar to anything else is not
        // allowed
        badArgIdx = std::min(badArgIdx, iArg);
      }
    } else {
      if (TypeInfoShapeKind != Template[pIntrinsicArg->uTemplateId]) {
        // Outside of simple splats and truncations, templates must match
        badArgIdx = std::min(badArgIdx, iArg);
      }
    }

    // Process component type from object element after loop
    if (pIntrinsicArg->uComponentTypeId == INTRIN_COMPTYPE_FROM_TYPE_ELT0) {
      ++iArg;
      continue;
    }

    DXASSERT(pIntrinsicArg->uComponentTypeId < MaxIntrinsicArgs,
             "otherwise intrinsic table was modified and MaxIntrinsicArgs was "
             "not updated (or uComponentTypeId is out of bounds)");

    // Merge ComponentTypes
    if (AR_BASIC_UNKNOWN == ComponentType[pIntrinsicArg->uComponentTypeId]) {
      ComponentType[pIntrinsicArg->uComponentTypeId] = TypeInfoEltKind;
    } else {
      if (!CombineBasicTypes(ComponentType[pIntrinsicArg->uComponentTypeId],
                             TypeInfoEltKind,
                             &ComponentType[pIntrinsicArg->uComponentTypeId])) {
        badArgIdx = std::min(badArgIdx, iArg);
      }
    }

    // Rows
    if (AR_TOBJ_SCALAR != TypeInfoShapeKind) {
      if (pIntrinsicArg->uRows >= IA_SPECIAL_BASE) {
        UINT uSpecialId = pIntrinsicArg->uRows - IA_SPECIAL_BASE;
        CAB(uSpecialId < IA_SPECIAL_SLOTS, iArg);
        if (uSpecialSize[uSpecialId] > TypeInfoRows) {
          uSpecialSize[uSpecialId] = TypeInfoRows;
        }
      } else {
        if (TypeInfoRows < pIntrinsicArg->uRows) {
          badArgIdx = std::min(badArgIdx, iArg);
        }
      }
    }

    // Columns
    if (AR_TOBJ_SCALAR != TypeInfoShapeKind) {
      if (pIntrinsicArg->uCols >= IA_SPECIAL_BASE) {
        UINT uSpecialId = pIntrinsicArg->uCols - IA_SPECIAL_BASE;
        CAB(uSpecialId < IA_SPECIAL_SLOTS, iArg);

        if (uSpecialSize[uSpecialId] > TypeInfoCols) {
          uSpecialSize[uSpecialId] = TypeInfoCols;
        }
      } else {
        if (TypeInfoCols < pIntrinsicArg->uCols) {
          badArgIdx = std::min(badArgIdx, iArg);
        }
      }
    }

    ASTContext &actx = m_sema->getASTContext();
    // Usage

    // Argument must be non-constant and non-bitfield for out, inout, and ref
    // parameters because they may be treated as pass-by-reference.
    // This is hacky. We should actually be handling this by failing reference
    // binding in sema init with SK_BindReference*. That code path is currently
    // hacked off for HLSL and less trivial to fix.
    if (pIntrinsicArg->qwUsage & AR_QUAL_OUT ||
        pIntrinsicArg->qwUsage & AR_QUAL_REF) {
      if (pType.isConstant(actx) || pCallArg->getObjectKind() == OK_BitField) {
        // Can't use a const type in an out or inout parameter.
        badArgIdx = std::min(badArgIdx, iArg);
      }
    }
    iArg++;
  }

  DXASSERT(isVariadic || iterArg == end,
           "otherwise the argument list wasn't fully processed");

  // Default template and component type for return value
  if (pIntrinsic->pArgs[0].qwUsage &&
      pIntrinsic->pArgs[0].uTemplateId != INTRIN_TEMPLATE_FROM_TYPE &&
      pIntrinsic->pArgs[0].uTemplateId != INTRIN_TEMPLATE_FROM_FUNCTION &&
      pIntrinsic->pArgs[0].uTemplateId != INTRIN_TEMPLATE_FROM_FUNCTION_2 &&
      pIntrinsic->pArgs[0].uComponentTypeId !=
          INTRIN_COMPTYPE_FROM_NODEOUTPUT) {
    CAB(pIntrinsic->pArgs[0].uTemplateId < MaxIntrinsicArgs, 0);
    if (AR_TOBJ_UNKNOWN == Template[pIntrinsic->pArgs[0].uTemplateId]) {
      Template[pIntrinsic->pArgs[0].uTemplateId] =
          g_LegalIntrinsicTemplates[pIntrinsic->pArgs[0].uLegalTemplates][0];

      if (pIntrinsic->pArgs[0].uComponentTypeId !=
          INTRIN_COMPTYPE_FROM_TYPE_ELT0) {
        DXASSERT_NOMSG(pIntrinsic->pArgs[0].uComponentTypeId <
                       MaxIntrinsicArgs);
        if (AR_BASIC_UNKNOWN ==
            ComponentType[pIntrinsic->pArgs[0].uComponentTypeId]) {
          // half return type should map to float for min precision
          if (pIntrinsic->pArgs[0].uLegalComponentTypes ==
                  LEGAL_INTRINSIC_COMPTYPES::LICOMPTYPE_FLOAT16 &&
              getSema()->getLangOpts().UseMinPrecision) {
            ComponentType[pIntrinsic->pArgs[0].uComponentTypeId] =
                ArBasicKind::AR_BASIC_FLOAT32;
          } else {
            ComponentType[pIntrinsic->pArgs[0].uComponentTypeId] =
                g_LegalIntrinsicCompTypes[pIntrinsic->pArgs[0]
                                              .uLegalComponentTypes][0];
          }
        }
      }
    }
  }

  // Make sure all template, component type, and texture type selections are
  // valid.
  for (size_t i = 0; i < Args.size() + 1; i++) {
    const HLSL_INTRINSIC_ARGUMENT *pArgument = &pIntrinsic->pArgs[i];

    // If vararg is reached, we can break out of this loop.
    if (pIntrinsic->pArgs[i].uTemplateId == INTRIN_TEMPLATE_VARARGS)
      break;

    // Check template.
    if (pArgument->uTemplateId == INTRIN_TEMPLATE_FROM_TYPE ||
        pArgument->uTemplateId == INTRIN_TEMPLATE_FROM_FUNCTION ||
        pArgument->uTemplateId == INTRIN_TEMPLATE_FROM_FUNCTION_2) {
      continue; // Already verified that this is available.
    }
    if (pArgument->uLegalComponentTypes == LICOMPTYPE_USER_DEFINED_TYPE) {
      continue;
    }

    const ArTypeObjectKind *pTT =
        g_LegalIntrinsicTemplates[pArgument->uLegalTemplates];
    if (AR_TOBJ_UNKNOWN != Template[i]) {
      if ((AR_TOBJ_SCALAR == Template[i]) &&
          (AR_TOBJ_VECTOR == *pTT || AR_TOBJ_MATRIX == *pTT)) {
        Template[i] = *pTT;
      } else {
        while (AR_TOBJ_UNKNOWN != *pTT) {
          if (Template[i] == *pTT)
            break;
          pTT++;
        }
      }

      if (AR_TOBJ_UNKNOWN == *pTT) {
        Template[i] = g_LegalIntrinsicTemplates[pArgument->uLegalTemplates][0];
        badArgIdx = std::min(badArgIdx, i);
      }
    } else if (pTT) {
      Template[i] = *pTT;
    }

    // Check component type.
    const ArBasicKind *pCT =
        g_LegalIntrinsicCompTypes[pArgument->uLegalComponentTypes];
    if (AR_BASIC_UNKNOWN != ComponentType[i]) {
      while (AR_BASIC_UNKNOWN != *pCT && AR_BASIC_NOCAST != *pCT) {
        if (ComponentType[i] == *pCT)
          break;
        pCT++;
      }

      // has to be a strict match
      if (*pCT == AR_BASIC_NOCAST) {
        badArgIdx = std::min(badArgIdx, i);
        // the match has failed, but the types are useful for errors. Present
        // the cannonical overload for error
        ComponentType[i] =
            g_LegalIntrinsicCompTypes[pArgument->uLegalComponentTypes][0];
      }

      // If it is an object, see if it can be cast to the first thing in the
      // list, otherwise move on to next intrinsic.
      if (AR_TOBJ_OBJECT == Template[i] && AR_BASIC_UNKNOWN == *pCT) {
        if (!CombineObjectTypes(
                g_LegalIntrinsicCompTypes[pArgument->uLegalComponentTypes][0],
                ComponentType[i], nullptr)) {
          badArgIdx = std::min(badArgIdx, i);
        }
      }

      if (AR_BASIC_UNKNOWN == *pCT) {
        ComponentType[i] =
            g_LegalIntrinsicCompTypes[pArgument->uLegalComponentTypes][0];
      }
    } else if (pCT) {
      ComponentType[i] = *pCT;
    }
  }

  argTypes.resize(1 + Args.size()); // +1 for return type

  // Default to a void return type.
  argTypes[0] = m_context->VoidTy;

  // Default specials sizes.
  for (UINT i = 0; i < IA_SPECIAL_SLOTS; i++) {
    if (UnusedSize == uSpecialSize[i]) {
      uSpecialSize[i] = 1;
    }
  }

  std::string profile = m_sema->getLangOpts().HLSLProfile;
  const ShaderModel *SM = hlsl::ShaderModel::GetByName(profile.c_str());

  // Populate argTypes.
  for (size_t i = 0; i <= Args.size(); i++) {
    const HLSL_INTRINSIC_ARGUMENT *pArgument = &pIntrinsic->pArgs[i];

    // If vararg is reached, we can break out of this loop.
    if (pArgument->uTemplateId == INTRIN_TEMPLATE_VARARGS)
      break;

    if (!pArgument->qwUsage)
      continue;

    QualType pNewType;
    unsigned int quals = 0; // qualifications for this argument

    // If we have no type, set it to our input type (templatized)
    if (pArgument->uTemplateId == INTRIN_TEMPLATE_FROM_TYPE) {
      // Use the templated input type, but resize it if the
      // intrinsic's rows/cols isn't 0
      if (pArgument->uRows && pArgument->uCols) {
        UINT uRows, uCols = 0;

        // if type is overriden, use new type size, for
        // now it only supports scalars
        if (pArgument->uRows >= IA_SPECIAL_BASE) {
          UINT uSpecialId = pArgument->uRows - IA_SPECIAL_BASE;
          CAB(uSpecialId < IA_SPECIAL_SLOTS, i);

          uRows = uSpecialSize[uSpecialId];
        } else if (pArgument->uRows > 0) {
          uRows = pArgument->uRows;
        }

        if (pArgument->uCols >= IA_SPECIAL_BASE) {
          UINT uSpecialId = pArgument->uCols - IA_SPECIAL_BASE;
          CAB(uSpecialId < IA_SPECIAL_SLOTS, i);

          uCols = uSpecialSize[uSpecialId];
        } else if (pArgument->uCols > 0) {
          uCols = pArgument->uCols;
        }

        // 1x1 numeric outputs are always scalar.. since these
        // are most flexible
        if ((1 == uCols) && (1 == uRows)) {
          pNewType = objectElement;
          if (pNewType.isNull()) {
            badArgIdx = std::min(badArgIdx, i);
          }
        } else {
          // non-scalars unsupported right now since nothing
          // uses it, would have to create either a type
          // list for sub-structures or just resize the
          // given type

          // VH(E_NOTIMPL);
          badArgIdx = std::min(badArgIdx, i);
        }
      } else {
        DXASSERT_NOMSG(!pArgument->uRows && !pArgument->uCols);
        if (objectElement.isNull()) {
          badArgIdx = std::min(badArgIdx, i);
        }
        pNewType = objectElement;
      }
    } else if (pArgument->uTemplateId == INTRIN_TEMPLATE_FROM_FUNCTION) {
      if (functionTemplateTypeArg.isNull()) {
        if (i == 0) {
          // [RW]ByteAddressBuffer.Load, default to uint
          pNewType = m_context->UnsignedIntTy;
          if (builtinOp != hlsl::IntrinsicOp::MOP_Load)
            badArgIdx = std::min(badArgIdx, i);
        } else {
          // [RW]ByteAddressBuffer.Store, default to argument type
          pNewType = Args[i - 1]->getType().getNonReferenceType();
          if (const BuiltinType *BuiltinTy = pNewType->getAs<BuiltinType>()) {
            // For backcompat, ensure that Store(0, 42 or 42.0) matches a
            // uint/float overload rather than a uint64_t/double one.
            if (BuiltinTy->getKind() == BuiltinType::LitInt) {
              pNewType = m_context->UnsignedIntTy;
            } else if (BuiltinTy->getKind() == BuiltinType::LitFloat) {
              pNewType = m_context->FloatTy;
            }
          }
        }
      } else {
        pNewType = functionTemplateTypeArg;
      }
    } else if (pArgument->uTemplateId == INTRIN_TEMPLATE_FROM_FUNCTION_2) {
      if (i == 0 &&
          (builtinOp == hlsl::IntrinsicOp::IOP_Vkreinterpret_pointer_cast ||
           builtinOp == hlsl::IntrinsicOp::IOP_Vkstatic_pointer_cast)) {
        pNewType = Args[0]->getType();
      } else {
        badArgIdx = std::min(badArgIdx, i);
      }
    } else if (pArgument->uLegalComponentTypes ==
               LICOMPTYPE_USER_DEFINED_TYPE) {
      if (objectElement.isNull()) {
        badArgIdx = std::min(badArgIdx, i);
      }
      pNewType = objectElement;
    } else if (i != 0 && Template[pArgument->uTemplateId] == AR_TOBJ_OBJECT) {
      // For object parameters, just use the argument type
      // Return type is assigned below
      pNewType = Args[i - 1]->getType().getNonReferenceType();
    } else if (pArgument->uLegalComponentTypes ==
               LICOMPTYPE_NODE_RECORD_OR_UAV) {
      pNewType = Args[i - 1]->getType().getNonReferenceType();
    } else if (pArgument->uLegalComponentTypes ==
               LICOMPTYPE_ANY_NODE_OUTPUT_RECORD) {
      pNewType = Args[i - 1]->getType().getNonReferenceType();
    } else {
      ArBasicKind pEltType;

      // ComponentType, if the Id is special then it gets the
      // component type from the first component of the type, if
      // we need more (for the second component, e.g.), then we
      // can use more specials, etc.
      if (pArgument->uComponentTypeId == INTRIN_COMPTYPE_FROM_TYPE_ELT0) {
        if (objectElement.isNull()) {
          badArgIdx = std::min(badArgIdx, i);
          return false;
        }
        pEltType = GetTypeElementKind(objectElement);
        if (!IsValidBasicKind(pEltType)) {
          // This can happen with Texture2D<Struct> or other invalid
          // declarations
          badArgIdx = std::min(badArgIdx, i);
          return false;
        }
      } else if (pArgument->uComponentTypeId ==
                 INTRIN_COMPTYPE_FROM_NODEOUTPUT) {
        ClassTemplateDecl *templateDecl = nullptr;
        if (pArgument->uLegalComponentTypes ==
            LICOMPTYPE_GROUP_NODE_OUTPUT_RECORDS)
          templateDecl = m_GroupNodeOutputRecordsTemplateDecl;
        else if (pArgument->uLegalComponentTypes ==
                 LICOMPTYPE_THREAD_NODE_OUTPUT_RECORDS)
          templateDecl = m_ThreadNodeOutputRecordsTemplateDecl;
        else {
          assert(false && "unexpected comp type");
        }

        CXXRecordDecl *recordDecl = templateDecl->getTemplatedDecl();
        if (!recordDecl->isCompleteDefinition()) {
          CompleteType(recordDecl);
        }

        pNewType = GetOrCreateNodeOutputRecordSpecialization(
            *m_context, m_sema, templateDecl, objectElement);
        argTypes[i] = QualType(pNewType.getTypePtr(), quals);
        continue;
      } else {
        pEltType = ComponentType[pArgument->uComponentTypeId];
        DXASSERT_VALIDBASICKIND(pEltType);
      }

      UINT uRows, uCols;

      // Rows
      if (pArgument->uRows >= IA_SPECIAL_BASE) {
        UINT uSpecialId = pArgument->uRows - IA_SPECIAL_BASE;
        CAB(uSpecialId < IA_SPECIAL_SLOTS, i);
        uRows = uSpecialSize[uSpecialId];
      } else {
        uRows = pArgument->uRows;
      }

      // Cols
      if (pArgument->uCols >= IA_SPECIAL_BASE) {
        UINT uSpecialId = pArgument->uCols - IA_SPECIAL_BASE;
        CAB(uSpecialId < IA_SPECIAL_SLOTS, i);
        uCols = uSpecialSize[uSpecialId];
      } else {
        uCols = pArgument->uCols;
      }

      // Verify that the final results are in bounds.
      CAB((uCols > 0 && uRows > 0 &&
           ((uCols <= MaxVectorSize && uRows <= MaxVectorSize) ||
            (SM->IsSM69Plus() && uRows == 1))),
          i);

      // Const
      UINT64 qwQual =
          pArgument->qwUsage &
          (AR_QUAL_ROWMAJOR | AR_QUAL_COLMAJOR | AR_QUAL_GROUPSHARED);

      if ((0 == i) || !(pArgument->qwUsage & AR_QUAL_OUT))
        qwQual |= AR_QUAL_CONST;

      DXASSERT_VALIDBASICKIND(pEltType);
      pNewType = NewSimpleAggregateType(Template[pArgument->uTemplateId],
                                        pEltType, qwQual, uRows, uCols);

      // If array type, wrap in the argument's array type.
      if (i > 0 && Template[pArgument->uTemplateId] == AR_TOBJ_ARRAY) {
        QualType arrayElt = Args[i - 1]->getType();
        SmallVector<UINT, 4> sizes;
        while (arrayElt->isArrayType()) {
          UINT size = 0;
          if (arrayElt->isConstantArrayType()) {
            const ConstantArrayType *arrayType =
                (const ConstantArrayType *)arrayElt->getAsArrayTypeUnsafe();
            size = arrayType->getSize().getLimitedValue();
          }
          arrayElt = QualType(
              arrayElt->getAsArrayTypeUnsafe()->getArrayElementTypeNoTypeQual(),
              0);
          sizes.push_back(size);
        }
        // Wrap element in matching array dimensions:
        while (sizes.size()) {
          uint64_t size = sizes.pop_back_val();
          if (size) {
            pNewType = m_context->getConstantArrayType(
                pNewType, llvm::APInt(32, size, false),
                ArrayType::ArraySizeModifier::Normal, 0);
          } else {
            pNewType = m_context->getIncompleteArrayType(
                pNewType, ArrayType::ArraySizeModifier::Normal, 0);
          }
        }
        if (qwQual & AR_QUAL_CONST)
          pNewType = QualType(pNewType.getTypePtr(), Qualifiers::Const);

        if (qwQual & AR_QUAL_GROUPSHARED)
          pNewType =
              m_context->getAddrSpaceQualType(pNewType, DXIL::kTGSMAddrSpace);

        pNewType = m_context->getLValueReferenceType(pNewType);
      }
    }

    DXASSERT(!pNewType.isNull(), "otherwise there's a branch in this function "
                                 "that fails to assign this");
    argTypes[i] = QualType(pNewType.getTypePtr(), quals);
  }

  // For variadic functions, we need to add the additional arguments here.
  if (isVariadic) {
    for (; iArg <= Args.size(); ++iArg) {
      argTypes[iArg] = Args[iArg - 1]->getType().getNonReferenceType();
    }
  } else {
    DXASSERT(iArg == pIntrinsic->uNumArgs,
             "In the absence of varargs, a successful match would indicate we "
             "have as many arguments and types as the intrinsic template");
  }

  // For object return types that need to match arguments, we need to slot in
  // the full type here Can't do it sooner because when return is encountered
  // above, the other arg types haven't been set
  if (pIntrinsic->pArgs[0].uTemplateId < MaxIntrinsicArgs) {
    if (Template[pIntrinsic->pArgs[0].uTemplateId] == AR_TOBJ_OBJECT)
      argTypes[0] = argTypes[pIntrinsic->pArgs[0].uComponentTypeId];
  }

  return badArgIdx == MaxIntrinsicArgs;
#undef CAB
}

HLSLExternalSource::FindStructBasicTypeResult
HLSLExternalSource::FindStructBasicType(DeclContext *functionDeclContext) {
  DXASSERT_NOMSG(functionDeclContext != nullptr);

  // functionDeclContext may be a specialization of a template, such as
  // AppendBuffer<MY_STRUCT>, or it may be a simple class, such as
  // RWByteAddressBuffer.
  const CXXRecordDecl *recordDecl =
      GetRecordDeclForBuiltInOrStruct(functionDeclContext);

  // We save the caller from filtering out other types of context (like the
  // translation unit itself).
  if (recordDecl != nullptr) {
    int index = FindObjectBasicKindIndex(recordDecl);
    if (index != -1) {
      ArBasicKind kind = g_ArBasicKindsAsTypes[index];
      return HLSLExternalSource::FindStructBasicTypeResult(kind, index);
    }
  }

  return HLSLExternalSource::FindStructBasicTypeResult(AR_BASIC_UNKNOWN, 0);
}

void HLSLExternalSource::FindIntrinsicTable(DeclContext *functionDeclContext,
                                            const char **name,
                                            const HLSL_INTRINSIC **intrinsics,
                                            size_t *intrinsicCount) {
  DXASSERT_NOMSG(functionDeclContext != nullptr);
  DXASSERT_NOMSG(name != nullptr);
  DXASSERT_NOMSG(intrinsics != nullptr);
  DXASSERT_NOMSG(intrinsicCount != nullptr);

  *intrinsics = nullptr;
  *intrinsicCount = 0;
  *name = nullptr;

  HLSLExternalSource::FindStructBasicTypeResult lookup =
      FindStructBasicType(functionDeclContext);
  if (lookup.Found()) {
    GetIntrinsicMethods(lookup.Kind, intrinsics, intrinsicCount);
    *name = g_ArBasicTypeNames[lookup.Kind];
  }
}

static bool BinaryOperatorKindIsArithmetic(BinaryOperatorKind Opc) {
  return
      // Arithmetic operators.
      Opc == BinaryOperatorKind::BO_Add ||
      Opc == BinaryOperatorKind::BO_AddAssign ||
      Opc == BinaryOperatorKind::BO_Sub ||
      Opc == BinaryOperatorKind::BO_SubAssign ||
      Opc == BinaryOperatorKind::BO_Rem ||
      Opc == BinaryOperatorKind::BO_RemAssign ||
      Opc == BinaryOperatorKind::BO_Div ||
      Opc == BinaryOperatorKind::BO_DivAssign ||
      Opc == BinaryOperatorKind::BO_Mul ||
      Opc == BinaryOperatorKind::BO_MulAssign;
}

static bool BinaryOperatorKindIsCompoundAssignment(BinaryOperatorKind Opc) {
  return
      // Arithmetic-and-assignment operators.
      Opc == BinaryOperatorKind::BO_AddAssign ||
      Opc == BinaryOperatorKind::BO_SubAssign ||
      Opc == BinaryOperatorKind::BO_RemAssign ||
      Opc == BinaryOperatorKind::BO_DivAssign ||
      Opc == BinaryOperatorKind::BO_MulAssign ||
      // Bitwise-and-assignment operators.
      Opc == BinaryOperatorKind::BO_ShlAssign ||
      Opc == BinaryOperatorKind::BO_ShrAssign ||
      Opc == BinaryOperatorKind::BO_AndAssign ||
      Opc == BinaryOperatorKind::BO_OrAssign ||
      Opc == BinaryOperatorKind::BO_XorAssign;
}

static bool
BinaryOperatorKindIsCompoundAssignmentForBool(BinaryOperatorKind Opc) {
  return Opc == BinaryOperatorKind::BO_AndAssign ||
         Opc == BinaryOperatorKind::BO_OrAssign ||
         Opc == BinaryOperatorKind::BO_XorAssign;
}

static bool BinaryOperatorKindIsBitwise(BinaryOperatorKind Opc) {
  return Opc == BinaryOperatorKind::BO_Shl ||
         Opc == BinaryOperatorKind::BO_ShlAssign ||
         Opc == BinaryOperatorKind::BO_Shr ||
         Opc == BinaryOperatorKind::BO_ShrAssign ||
         Opc == BinaryOperatorKind::BO_And ||
         Opc == BinaryOperatorKind::BO_AndAssign ||
         Opc == BinaryOperatorKind::BO_Or ||
         Opc == BinaryOperatorKind::BO_OrAssign ||
         Opc == BinaryOperatorKind::BO_Xor ||
         Opc == BinaryOperatorKind::BO_XorAssign;
}

static bool BinaryOperatorKindIsBitwiseShift(BinaryOperatorKind Opc) {
  return Opc == BinaryOperatorKind::BO_Shl ||
         Opc == BinaryOperatorKind::BO_ShlAssign ||
         Opc == BinaryOperatorKind::BO_Shr ||
         Opc == BinaryOperatorKind::BO_ShrAssign;
}

static bool BinaryOperatorKindIsEqualComparison(BinaryOperatorKind Opc) {
  return Opc == BinaryOperatorKind::BO_EQ || Opc == BinaryOperatorKind::BO_NE;
}

static bool BinaryOperatorKindIsOrderComparison(BinaryOperatorKind Opc) {
  return Opc == BinaryOperatorKind::BO_LT || Opc == BinaryOperatorKind::BO_GT ||
         Opc == BinaryOperatorKind::BO_LE || Opc == BinaryOperatorKind::BO_GE;
}

static bool BinaryOperatorKindIsComparison(BinaryOperatorKind Opc) {
  return BinaryOperatorKindIsEqualComparison(Opc) ||
         BinaryOperatorKindIsOrderComparison(Opc);
}

static bool BinaryOperatorKindIsLogical(BinaryOperatorKind Opc) {
  return Opc == BinaryOperatorKind::BO_LAnd ||
         Opc == BinaryOperatorKind::BO_LOr;
}

static bool BinaryOperatorKindRequiresNumeric(BinaryOperatorKind Opc) {
  return BinaryOperatorKindIsArithmetic(Opc) ||
         BinaryOperatorKindIsOrderComparison(Opc) ||
         BinaryOperatorKindIsLogical(Opc);
}

static bool BinaryOperatorKindRequiresIntegrals(BinaryOperatorKind Opc) {
  return BinaryOperatorKindIsBitwise(Opc);
}

static bool BinaryOperatorKindRequiresBoolAsNumeric(BinaryOperatorKind Opc) {
  return BinaryOperatorKindIsBitwise(Opc) ||
         BinaryOperatorKindIsArithmetic(Opc);
}

static bool UnaryOperatorKindRequiresIntegrals(UnaryOperatorKind Opc) {
  return Opc == UnaryOperatorKind::UO_Not;
}

static bool UnaryOperatorKindRequiresNumerics(UnaryOperatorKind Opc) {
  return Opc == UnaryOperatorKind::UO_LNot ||
         Opc == UnaryOperatorKind::UO_Plus ||
         Opc == UnaryOperatorKind::UO_Minus ||
         // The omission in fxc caused objects and structs to accept this.
         Opc == UnaryOperatorKind::UO_PreDec ||
         Opc == UnaryOperatorKind::UO_PreInc ||
         Opc == UnaryOperatorKind::UO_PostDec ||
         Opc == UnaryOperatorKind::UO_PostInc;
}

static bool UnaryOperatorKindRequiresModifiableValue(UnaryOperatorKind Opc) {
  return Opc == UnaryOperatorKind::UO_PreDec ||
         Opc == UnaryOperatorKind::UO_PreInc ||
         Opc == UnaryOperatorKind::UO_PostDec ||
         Opc == UnaryOperatorKind::UO_PostInc;
}

static bool UnaryOperatorKindRequiresBoolAsNumeric(UnaryOperatorKind Opc) {
  return Opc == UnaryOperatorKind::UO_Not ||
         Opc == UnaryOperatorKind::UO_Plus ||
         Opc == UnaryOperatorKind::UO_Minus;
}

static bool UnaryOperatorKindDisallowsBool(UnaryOperatorKind Opc) {
  return Opc == UnaryOperatorKind::UO_PreDec ||
         Opc == UnaryOperatorKind::UO_PreInc ||
         Opc == UnaryOperatorKind::UO_PostDec ||
         Opc == UnaryOperatorKind::UO_PostInc;
}

static bool IsIncrementOp(UnaryOperatorKind Opc) {
  return Opc == UnaryOperatorKind::UO_PreInc ||
         Opc == UnaryOperatorKind::UO_PostInc;
}

/// <summary>
/// Checks whether the specified AR_TOBJ* value is a primitive or aggregate of
/// primitive elements (as opposed to a built-in object like a sampler or
/// texture, or a void type).
/// </summary>
static bool IsObjectKindPrimitiveAggregate(ArTypeObjectKind value) {
  return value == AR_TOBJ_BASIC || value == AR_TOBJ_MATRIX ||
         value == AR_TOBJ_VECTOR;
}

static bool IsBasicKindIntegral(ArBasicKind value) {
  return IS_BASIC_AINT(value) || IS_BASIC_BOOL(value);
}

static bool IsBasicKindIntMinPrecision(ArBasicKind kind) {
  return IS_BASIC_SINT(kind) && IS_BASIC_MIN_PRECISION(kind);
}

static bool IsBasicKindNumeric(ArBasicKind value) {
  return GetBasicKindProps(value) & BPROP_NUMERIC;
}

ExprResult HLSLExternalSource::PromoteToIntIfBool(ExprResult &E) {
  // An invalid expression is pass-through at this point.
  if (E.isInvalid()) {
    return E;
  }

  QualType qt = E.get()->getType();
  ArBasicKind elementKind = this->GetTypeElementKind(qt);
  if (elementKind != AR_BASIC_BOOL) {
    return E;
  }

  // Construct a scalar/vector/matrix type with the same shape as E.
  ArTypeObjectKind objectKind = this->GetTypeObjectKind(qt);

  QualType targetType;
  UINT colCount, rowCount;
  GetRowsAndColsForAny(qt, rowCount, colCount);

  targetType =
      NewSimpleAggregateType(objectKind, AR_BASIC_INT32, 0, rowCount, colCount)
          ->getCanonicalTypeInternal();

  if (E.get()->isLValue()) {
    E = m_sema->DefaultLvalueConversion(E.get()).get();
  }

  switch (objectKind) {
  case AR_TOBJ_SCALAR:
    return ImplicitCastExpr::Create(*m_context, targetType,
                                    CastKind::CK_IntegralCast, E.get(), nullptr,
                                    ExprValueKind::VK_RValue);
  case AR_TOBJ_ARRAY:
  case AR_TOBJ_VECTOR:
  case AR_TOBJ_MATRIX:
    return ImplicitCastExpr::Create(*m_context, targetType,
                                    CastKind::CK_HLSLCC_IntegralCast, E.get(),
                                    nullptr, ExprValueKind::VK_RValue);
  default:
    DXASSERT(false, "unsupported objectKind for PromoteToIntIfBool");
  }
  return E;
}

void HLSLExternalSource::CollectInfo(QualType type, ArTypeInfo *pTypeInfo) {
  DXASSERT_NOMSG(pTypeInfo != nullptr);
  DXASSERT_NOMSG(!type.isNull());

  memset(pTypeInfo, 0, sizeof(*pTypeInfo));

  // TODO: Get* functions used here add up to a bunch of redundant code.
  //       Try to inline that here, making it cheaper to use this function
  //       when retrieving multiple properties.
  pTypeInfo->ObjKind = GetTypeElementKind(type);
  pTypeInfo->ShapeKind = GetTypeObjectKind(type);

  GetRowsAndColsForAny(type, pTypeInfo->uRows, pTypeInfo->uCols);
  pTypeInfo->EltKind = pTypeInfo->ObjKind;
  pTypeInfo->EltTy =
      GetTypeElementType(type)->getCanonicalTypeUnqualified()->getTypePtr();

  pTypeInfo->uTotalElts = pTypeInfo->uRows * pTypeInfo->uCols;
}

// Highest possible score (i.e., worst possible score).
static const UINT64 SCORE_MAX = 0xFFFFFFFFFFFFFFFF;

// Leave the first two score bits to handle higher-level
// variations like target type.
#define SCORE_MIN_SHIFT 2

// Space out scores to allow up to 128 parameters to
// vary between score sets spill into each other.
#define SCORE_PARAM_SHIFT 7

unsigned HLSLExternalSource::GetNumElements(QualType anyType) {
  if (anyType.isNull()) {
    return 0;
  }

  anyType = GetStructuralForm(anyType);

  ArTypeObjectKind kind = GetTypeObjectKind(anyType);
  switch (kind) {
  case AR_TOBJ_BASIC:
  case AR_TOBJ_OBJECT:
  case AR_TOBJ_STRING:
    return 1;
  case AR_TOBJ_COMPOUND: {
    // TODO: consider caching this value for perf
    unsigned total = 0;
    const RecordType *recordType = anyType->getAs<RecordType>();
    RecordDecl::field_iterator fi = recordType->getDecl()->field_begin();
    RecordDecl::field_iterator fend = recordType->getDecl()->field_end();
    while (fi != fend) {
      total += GetNumElements(fi->getType());
      ++fi;
    }
    return total;
  }
  case AR_TOBJ_ARRAY:
  case AR_TOBJ_MATRIX:
  case AR_TOBJ_VECTOR:
    return GetElementCount(anyType);
  default:
    DXASSERT(kind == AR_TOBJ_VOID,
             "otherwise the type cannot be classified or is not supported");
    return 0;
  }
}

unsigned HLSLExternalSource::GetNumBasicElements(QualType anyType) {
  if (anyType.isNull()) {
    return 0;
  }

  anyType = GetStructuralForm(anyType);

  ArTypeObjectKind kind = GetTypeObjectKind(anyType);
  switch (kind) {
  case AR_TOBJ_BASIC:
  case AR_TOBJ_OBJECT:
  case AR_TOBJ_STRING:
    return 1;
  case AR_TOBJ_COMPOUND: {
    // TODO: consider caching this value for perf
    unsigned total = 0;
    const RecordType *recordType = anyType->getAs<RecordType>();
    RecordDecl *RD = recordType->getDecl();
    // Take care base.
    if (const CXXRecordDecl *CXXRD = dyn_cast<CXXRecordDecl>(RD)) {
      if (CXXRD->getNumBases()) {
        for (const auto &I : CXXRD->bases()) {
          const CXXRecordDecl *BaseDecl =
              cast<CXXRecordDecl>(I.getType()->castAs<RecordType>()->getDecl());
          if (BaseDecl->field_empty())
            continue;
          QualType parentTy = QualType(BaseDecl->getTypeForDecl(), 0);
          total += GetNumBasicElements(parentTy);
        }
      }
    }
    RecordDecl::field_iterator fi = RD->field_begin();
    RecordDecl::field_iterator fend = RD->field_end();
    while (fi != fend) {
      total += GetNumBasicElements(fi->getType());
      ++fi;
    }
    return total;
  }
  case AR_TOBJ_ARRAY: {
    unsigned arraySize = GetElementCount(anyType);
    unsigned eltSize = GetNumBasicElements(
        QualType(anyType->getArrayElementTypeNoTypeQual(), 0));
    return arraySize * eltSize;
  }
  case AR_TOBJ_MATRIX:
  case AR_TOBJ_VECTOR:
    return GetElementCount(anyType);
  default:
    DXASSERT(kind == AR_TOBJ_VOID,
             "otherwise the type cannot be classified or is not supported");
    return 0;
  }
}

unsigned HLSLExternalSource::GetNumConvertCheckElts(QualType leftType,
                                                    unsigned leftSize,
                                                    QualType rightType,
                                                    unsigned rightSize) {
  // We can convert from a larger type to a smaller
  // but not a smaller type to a larger so default
  // to just comparing the destination size.
  unsigned uElts = leftSize;

  leftType = GetStructuralForm(leftType);
  rightType = GetStructuralForm(rightType);

  if (leftType->isArrayType() && rightType->isArrayType()) {
    //
    // If we're comparing arrays we don't
    // need to compare every element of
    // the arrays since all elements
    // will have the same type.
    // We only need to compare enough
    // elements that we've tried every
    // possible mix of dst and src elements.
    //

    // TODO: handle multidimensional arrays and arrays of arrays
    QualType pDstElt = leftType->getAsArrayTypeUnsafe()->getElementType();
    unsigned uDstEltSize = GetNumElements(pDstElt);

    QualType pSrcElt = rightType->getAsArrayTypeUnsafe()->getElementType();
    unsigned uSrcEltSize = GetNumElements(pSrcElt);

    if (uDstEltSize == uSrcEltSize) {
      uElts = uDstEltSize;
    } else if (uDstEltSize > uSrcEltSize) {
      // If one size is not an even multiple of the other we need to let the
      // full compare run in order to try all alignments.
      if (uSrcEltSize && (uDstEltSize % uSrcEltSize) == 0) {
        uElts = uDstEltSize;
      }
    } else if (uDstEltSize && (uSrcEltSize % uDstEltSize) == 0) {
      uElts = uSrcEltSize;
    }
  }

  return uElts;
}

QualType HLSLExternalSource::GetNthElementType(QualType type, unsigned index) {
  if (type.isNull()) {
    return type;
  }

  ArTypeObjectKind kind = GetTypeObjectKind(type);
  switch (kind) {
  case AR_TOBJ_BASIC:
  case AR_TOBJ_OBJECT:
  case AR_TOBJ_STRING:
    return (index == 0) ? type : QualType();
  case AR_TOBJ_COMPOUND: {
    // TODO: consider caching this value for perf
    const RecordType *recordType = type->getAs<RecordType>();
    RecordDecl::field_iterator fi = recordType->getDecl()->field_begin();
    RecordDecl::field_iterator fend = recordType->getDecl()->field_end();
    while (fi != fend) {
      if (!fi->getType().isNull()) {
        unsigned subElements = GetNumElements(fi->getType());
        if (index < subElements) {
          return GetNthElementType(fi->getType(), index);
        } else {
          index -= subElements;
        }
      }
      ++fi;
    }
    return QualType();
  }
  case AR_TOBJ_ARRAY: {
    unsigned arraySize;
    QualType elementType;
    unsigned elementCount;
    elementType =
        type.getNonReferenceType()->getAsArrayTypeUnsafe()->getElementType();
    elementCount = GetElementCount(elementType);
    if (index < elementCount) {
      return GetNthElementType(elementType, index);
    }
    arraySize = GetArraySize(type);
    if (index >= arraySize * elementCount) {
      return QualType();
    }
    return GetNthElementType(elementType, index % elementCount);
  }
  case AR_TOBJ_MATRIX:
  case AR_TOBJ_VECTOR:
    return (index < GetElementCount(type)) ? GetMatrixOrVectorElementType(type)
                                           : QualType();
  default:
    DXASSERT(kind == AR_TOBJ_VOID,
             "otherwise the type cannot be classified or is not supported");
    return QualType();
  }
}

bool HLSLExternalSource::IsPromotion(ArBasicKind leftKind,
                                     ArBasicKind rightKind) {
  // Eliminate exact matches first, then check for promotions.
  if (leftKind == rightKind) {
    return false;
  }

  switch (rightKind) {
  case AR_BASIC_FLOAT16:
    switch (leftKind) {
    case AR_BASIC_FLOAT32:
    case AR_BASIC_FLOAT32_PARTIAL_PRECISION:
    case AR_BASIC_FLOAT64:
      return true;
    default:
      return false; // No other type is a promotion.
    }
    break;
  case AR_BASIC_FLOAT32_PARTIAL_PRECISION:
    switch (leftKind) {
    case AR_BASIC_FLOAT32:
    case AR_BASIC_FLOAT64:
      return true;
    default:
      return false; // No other type is a promotion.
    }
    break;
  case AR_BASIC_FLOAT32:
    switch (leftKind) {
    case AR_BASIC_FLOAT64:
      return true;
    default:
      return false; // No other type is a promotion.
    }
    break;
  case AR_BASIC_MIN10FLOAT:
    switch (leftKind) {
    case AR_BASIC_MIN16FLOAT:
    case AR_BASIC_FLOAT16:
    case AR_BASIC_FLOAT32:
    case AR_BASIC_FLOAT32_PARTIAL_PRECISION:
    case AR_BASIC_FLOAT64:
      return true;
    default:
      return false; // No other type is a promotion.
    }
    break;
  case AR_BASIC_MIN16FLOAT:
    switch (leftKind) {
    case AR_BASIC_FLOAT16:
    case AR_BASIC_FLOAT32:
    case AR_BASIC_FLOAT32_PARTIAL_PRECISION:
    case AR_BASIC_FLOAT64:
      return true;
    default:
      return false; // No other type is a promotion.
    }
    break;

  case AR_BASIC_INT8:
  case AR_BASIC_UINT8:
    // For backwards compat we consider signed/unsigned the same.
    switch (leftKind) {
    case AR_BASIC_INT16:
    case AR_BASIC_INT32:
    case AR_BASIC_INT64:
    case AR_BASIC_UINT16:
    case AR_BASIC_UINT32:
    case AR_BASIC_UINT64:
      return true;
    default:
      return false; // No other type is a promotion.
    }
    break;
  case AR_BASIC_INT16:
  case AR_BASIC_UINT16:
    // For backwards compat we consider signed/unsigned the same.
    switch (leftKind) {
    case AR_BASIC_INT32:
    case AR_BASIC_INT64:
    case AR_BASIC_UINT32:
    case AR_BASIC_UINT64:
      return true;
    default:
      return false; // No other type is a promotion.
    }
    break;
  case AR_BASIC_INT32:
  case AR_BASIC_UINT32:
    // For backwards compat we consider signed/unsigned the same.
    switch (leftKind) {
    case AR_BASIC_INT64:
    case AR_BASIC_UINT64:
      return true;
    default:
      return false; // No other type is a promotion.
    }
    break;
  case AR_BASIC_MIN12INT:
    switch (leftKind) {
    case AR_BASIC_MIN16INT:
    case AR_BASIC_INT32:
    case AR_BASIC_INT64:
      return true;
    default:
      return false; // No other type is a promotion.
    }
    break;
  case AR_BASIC_MIN16INT:
    switch (leftKind) {
    case AR_BASIC_INT32:
    case AR_BASIC_INT64:
      return true;
    default:
      return false; // No other type is a promotion.
    }
    break;
  case AR_BASIC_MIN16UINT:
    switch (leftKind) {
    case AR_BASIC_UINT32:
    case AR_BASIC_UINT64:
      return true;
    default:
      return false; // No other type is a promotion.
    }
    break;
  }

  return false;
}

bool HLSLExternalSource::IsCast(ArBasicKind leftKind, ArBasicKind rightKind) {
  // Eliminate exact matches first, then check for casts.
  if (leftKind == rightKind) {
    return false;
  }

  //
  // All minimum-bits types are only considered matches of themselves
  // and thus are not in this table.
  //

  switch (leftKind) {
  case AR_BASIC_LITERAL_INT:
    switch (rightKind) {
    case AR_BASIC_INT8:
    case AR_BASIC_INT16:
    case AR_BASIC_INT32:
    case AR_BASIC_INT64:
    case AR_BASIC_UINT8:
    case AR_BASIC_UINT16:
    case AR_BASIC_UINT32:
    case AR_BASIC_UINT64:
      return false;
    default:
      break; // No other valid cast types
    }
    break;

  case AR_BASIC_INT8:
    switch (rightKind) {
    // For backwards compat we consider signed/unsigned the same.
    case AR_BASIC_LITERAL_INT:
    case AR_BASIC_UINT8:
      return false;
    default:
      break; // No other valid cast types
    }
    break;

  case AR_BASIC_INT16:
    switch (rightKind) {
    // For backwards compat we consider signed/unsigned the same.
    case AR_BASIC_LITERAL_INT:
    case AR_BASIC_UINT16:
      return false;
    default:
      break; // No other valid cast types
    }
    break;

  case AR_BASIC_INT32:
    switch (rightKind) {
    // For backwards compat we consider signed/unsigned the same.
    case AR_BASIC_LITERAL_INT:
    case AR_BASIC_UINT32:
      return false;
    default:
      break; // No other valid cast types.
    }
    break;

  case AR_BASIC_INT64:
    switch (rightKind) {
    // For backwards compat we consider signed/unsigned the same.
    case AR_BASIC_LITERAL_INT:
    case AR_BASIC_UINT64:
      return false;
    default:
      break; // No other valid cast types.
    }
    break;

  case AR_BASIC_UINT8:
    switch (rightKind) {
    // For backwards compat we consider signed/unsigned the same.
    case AR_BASIC_LITERAL_INT:
    case AR_BASIC_INT8:
      return false;
    default:
      break; // No other valid cast types.
    }
    break;

  case AR_BASIC_UINT16:
    switch (rightKind) {
    // For backwards compat we consider signed/unsigned the same.
    case AR_BASIC_LITERAL_INT:
    case AR_BASIC_INT16:
      return false;
    default:
      break; // No other valid cast types.
    }
    break;

  case AR_BASIC_UINT32:
    switch (rightKind) {
    // For backwards compat we consider signed/unsigned the same.
    case AR_BASIC_LITERAL_INT:
    case AR_BASIC_INT32:
      return false;
    default:
      break; // No other valid cast types.
    }
    break;

  case AR_BASIC_UINT64:
    switch (rightKind) {
    // For backwards compat we consider signed/unsigned the same.
    case AR_BASIC_LITERAL_INT:
    case AR_BASIC_INT64:
      return false;
    default:
      break; // No other valid cast types.
    }
    break;

  case AR_BASIC_LITERAL_FLOAT:
    switch (rightKind) {
    case AR_BASIC_LITERAL_FLOAT:
    case AR_BASIC_FLOAT16:
    case AR_BASIC_FLOAT32:
    case AR_BASIC_FLOAT32_PARTIAL_PRECISION:
    case AR_BASIC_FLOAT64:
      return false;
    default:
      break; // No other valid cast types.
    }
    break;

  case AR_BASIC_FLOAT16:
    switch (rightKind) {
    case AR_BASIC_LITERAL_FLOAT:
      return false;
    default:
      break; // No other valid cast types.
    }
    break;

  case AR_BASIC_FLOAT32_PARTIAL_PRECISION:
    switch (rightKind) {
    case AR_BASIC_LITERAL_FLOAT:
      return false;
    default:
      break; // No other valid cast types.
    }
    break;

  case AR_BASIC_FLOAT32:
    switch (rightKind) {
    case AR_BASIC_LITERAL_FLOAT:
      return false;
    default:
      break; // No other valid cast types.
    }
    break;

  case AR_BASIC_FLOAT64:
    switch (rightKind) {
    case AR_BASIC_LITERAL_FLOAT:
      return false;
    default:
      break; // No other valid cast types.
    }
    break;
  default:
    break; // No other relevant targets.
  }

  return true;
}

bool HLSLExternalSource::IsIntCast(ArBasicKind leftKind,
                                   ArBasicKind rightKind) {
  // Eliminate exact matches first, then check for casts.
  if (leftKind == rightKind) {
    return false;
  }

  //
  // All minimum-bits types are only considered matches of themselves
  // and thus are not in this table.
  //

  switch (leftKind) {
  case AR_BASIC_LITERAL_INT:
    switch (rightKind) {
    case AR_BASIC_INT8:
    case AR_BASIC_INT16:
    case AR_BASIC_INT32:
    case AR_BASIC_INT64:
    case AR_BASIC_UINT8:
    case AR_BASIC_UINT16:
    case AR_BASIC_UINT32:
    case AR_BASIC_UINT64:
      return false;
    default:
      break; // No other valid conversions
    }
    break;

  case AR_BASIC_INT8:
  case AR_BASIC_INT16:
  case AR_BASIC_INT32:
  case AR_BASIC_INT64:
  case AR_BASIC_UINT8:
  case AR_BASIC_UINT16:
  case AR_BASIC_UINT32:
  case AR_BASIC_UINT64:
    switch (rightKind) {
    case AR_BASIC_LITERAL_INT:
      return false;
    default:
      break; // No other valid conversions
    }
    break;

  case AR_BASIC_LITERAL_FLOAT:
    switch (rightKind) {
    case AR_BASIC_LITERAL_FLOAT:
    case AR_BASIC_FLOAT16:
    case AR_BASIC_FLOAT32:
    case AR_BASIC_FLOAT32_PARTIAL_PRECISION:
    case AR_BASIC_FLOAT64:
      return false;
    default:
      break; // No other valid conversions
    }
    break;

  case AR_BASIC_FLOAT16:
  case AR_BASIC_FLOAT32:
  case AR_BASIC_FLOAT32_PARTIAL_PRECISION:
  case AR_BASIC_FLOAT64:
    switch (rightKind) {
    case AR_BASIC_LITERAL_FLOAT:
      return false;
    default:
      break; // No other valid conversions
    }
    break;
  default:
    // No other relevant targets
    break;
  }

  return true;
}

UINT64 HLSLExternalSource::ScoreCast(QualType pLType, QualType pRType) {
  if (pLType.getCanonicalType() == pRType.getCanonicalType()) {
    return 0;
  }

  UINT64 uScore = 0;
  UINT uLSize = GetNumElements(pLType);
  UINT uRSize = GetNumElements(pRType);
  UINT uCompareSize;

  bool bLCast = false;
  bool bRCast = false;
  bool bLIntCast = false;
  bool bRIntCast = false;
  bool bLPromo = false;
  bool bRPromo = false;

  uCompareSize = GetNumConvertCheckElts(pLType, uLSize, pRType, uRSize);
  if (uCompareSize > uRSize) {
    uCompareSize = uRSize;
  }

  for (UINT i = 0; i < uCompareSize; i++) {
    ArBasicKind LeftElementKind, RightElementKind;
    ArBasicKind CombinedKind = AR_BASIC_BOOL;

    QualType leftSub = GetNthElementType(pLType, i);
    QualType rightSub = GetNthElementType(pRType, i);
    ArTypeObjectKind leftKind = GetTypeObjectKind(leftSub);
    ArTypeObjectKind rightKind = GetTypeObjectKind(rightSub);
    LeftElementKind = GetTypeElementKind(leftSub);
    RightElementKind = GetTypeElementKind(rightSub);

    // CollectInfo is called with AR_TINFO_ALLOW_OBJECTS, and the resulting
    // information needed is the ShapeKind, EltKind and ObjKind.

    if (!leftSub.isNull() && !rightSub.isNull() &&
        leftKind != AR_TOBJ_INVALID && rightKind != AR_TOBJ_INVALID) {
      bool bCombine;

      if (leftKind == AR_TOBJ_OBJECT || rightKind == AR_TOBJ_OBJECT) {
        DXASSERT(rightKind == AR_TOBJ_OBJECT,
                 "otherwise prior check is incorrect");
        ArBasicKind LeftObjKind =
            LeftElementKind; // actually LeftElementKind would have been the
                             // element
        ArBasicKind RightObjKind = RightElementKind;
        LeftElementKind = LeftObjKind;
        RightElementKind = RightObjKind;

        if (leftKind != rightKind) {
          bCombine = false;
        } else if (!(bCombine = CombineObjectTypes(LeftObjKind, RightObjKind,
                                                   &CombinedKind))) {
          bCombine =
              CombineObjectTypes(RightObjKind, LeftObjKind, &CombinedKind);
        }
      } else {
        bCombine =
            CombineBasicTypes(LeftElementKind, RightElementKind, &CombinedKind);
      }

      if (bCombine && IsPromotion(LeftElementKind, CombinedKind)) {
        bLPromo = true;
      } else if (!bCombine || IsCast(LeftElementKind, CombinedKind)) {
        bLCast = true;
      } else if (IsIntCast(LeftElementKind, CombinedKind)) {
        bLIntCast = true;
      }

      if (bCombine && IsPromotion(CombinedKind, RightElementKind)) {
        bRPromo = true;
      } else if (!bCombine || IsCast(CombinedKind, RightElementKind)) {
        bRCast = true;
      } else if (IsIntCast(CombinedKind, RightElementKind)) {
        bRIntCast = true;
      }
    } else {
      bLCast = true;
      bRCast = true;
    }
  }

#define SCORE_COND(shift, cond)                                                \
  {                                                                            \
    if (cond)                                                                  \
      uScore += 1ULL << (SCORE_MIN_SHIFT + SCORE_PARAM_SHIFT * shift);         \
  }
  SCORE_COND(0, uRSize < uLSize);
  SCORE_COND(1, bLPromo);
  SCORE_COND(2, bRPromo);
  SCORE_COND(3, bLIntCast);
  SCORE_COND(4, bRIntCast);
  SCORE_COND(5, bLCast);
  SCORE_COND(6, bRCast);
  SCORE_COND(7, uLSize < uRSize);
#undef SCORE_COND

  // Make sure our scores fit in a UINT64.
  static_assert(SCORE_MIN_SHIFT + SCORE_PARAM_SHIFT * 8 <= 64);

  return uScore;
}

UINT64 HLSLExternalSource::ScoreImplicitConversionSequence(
    const ImplicitConversionSequence *ics) {
  DXASSERT(ics, "otherwise conversion has not been initialized");
  if (!ics->isInitialized()) {
    return 0;
  }
  if (!ics->isStandard()) {
    return SCORE_MAX;
  }

  QualType fromType = ics->Standard.getFromType();
  QualType toType = ics->Standard.getToType(2); // final type
  return ScoreCast(toType, fromType);
}

UINT64 HLSLExternalSource::ScoreFunction(OverloadCandidateSet::iterator &Cand) {
  // Ignore target version mismatches.

  // in/out considerations have been taken care of by viability.

  // 'this' considerations don't matter without inheritance, other
  // than lookup and viability.

  UINT64 result = 0;
  for (unsigned convIdx = 0; convIdx < Cand->NumConversions; ++convIdx) {
    UINT64 score;

    score = ScoreImplicitConversionSequence(Cand->Conversions + convIdx);
    if (score == SCORE_MAX) {
      return SCORE_MAX;
    }
    result += score;

    score = ScoreImplicitConversionSequence(Cand->OutConversions + convIdx);
    if (score == SCORE_MAX) {
      return SCORE_MAX;
    }
    result += score;
  }
  return result;
}

OverloadingResult HLSLExternalSource::GetBestViableFunction(
    SourceLocation Loc, OverloadCandidateSet &set,
    OverloadCandidateSet::iterator &Best) {
  UINT64 bestScore = SCORE_MAX;
  unsigned scoreMatch = 0;
  Best = set.end();

  if (set.size() == 1 && set.begin()->Viable) {
    Best = set.begin();
    return OR_Success;
  }

  for (OverloadCandidateSet::iterator Cand = set.begin(); Cand != set.end();
       ++Cand) {
    if (Cand->Viable) {
      UINT64 score = ScoreFunction(Cand);
      if (score != SCORE_MAX) {
        if (score == bestScore) {
          ++scoreMatch;
        } else if (score < bestScore) {
          Best = Cand;
          scoreMatch = 1;
          bestScore = score;
        }
      }
    }
  }

  if (Best == set.end()) {
    return OR_No_Viable_Function;
  }

  if (scoreMatch > 1) {
    Best = set.end();
    return OR_Ambiguous;
  }

  // No need to check for deleted functions to yield OR_Deleted.

  return OR_Success;
}

/// <summary>
/// Initializes the specified <paramref name="initSequence" /> describing how
/// <paramref name="Entity" /> is initialized with <paramref name="Args" />.
/// </summary>
/// <param name="Entity">Entity being initialized; a variable, return result,
/// etc.</param> <param name="Kind">Kind of initialization: copying,
/// list-initializing, constructing, etc.</param> <param name="Args">Arguments
/// to the initialization.</param> <param name="TopLevelOfInitList">Whether this
/// is the top-level of an initialization list.</param> <param
/// name="initSequence">Initialization sequence description to
/// initialize.</param>
void HLSLExternalSource::InitializeInitSequenceForHLSL(
    const InitializedEntity &Entity, const InitializationKind &Kind,
    MultiExprArg Args, bool TopLevelOfInitList,
    InitializationSequence *initSequence) {
  DXASSERT_NOMSG(initSequence != nullptr);

  // In HLSL there are no default initializers, eg float4x4 m();
  // Except for RayQuery and HitObject constructors (also handle
  // InitializationKind::IK_Value)
  if (Kind.getKind() == InitializationKind::IK_Default ||
      Kind.getKind() == InitializationKind::IK_Value) {
    QualType destBaseType = m_context->getBaseElementType(Entity.getType());
    ArTypeObjectKind destBaseShape = GetTypeObjectKind(destBaseType);
    if (destBaseShape == AR_TOBJ_OBJECT) {
      const CXXRecordDecl *typeRecordDecl = destBaseType->getAsCXXRecordDecl();
      int index = FindObjectBasicKindIndex(
          GetRecordDeclForBuiltInOrStruct(typeRecordDecl));
      DXASSERT(index != -1,
               "otherwise can't find type we already determined was an object");

      if (g_ArBasicKindsAsTypes[index] == AR_OBJECT_RAY_QUERY ||
          g_ArBasicKindsAsTypes[index] == AR_OBJECT_HIT_OBJECT) {
        CXXConstructorDecl *Constructor = *typeRecordDecl->ctor_begin();
        initSequence->AddConstructorInitializationStep(
            Constructor, AccessSpecifier::AS_public, destBaseType, false, false,
            false);
        return;
      }
    }
    // Value initializers occur for temporaries with empty parens or braces.
    if (Kind.getKind() == InitializationKind::IK_Value) {
      m_sema->Diag(Kind.getLocation(), diag::err_hlsl_type_empty_init)
          << Entity.getType();
      SilenceSequenceDiagnostics(initSequence);
    }
    return;
  }

  // If we have a DirectList, we should have a single InitListExprClass
  // argument.
  DXASSERT(
      Kind.getKind() != InitializationKind::IK_DirectList ||
          (Args.size() == 1 &&
           Args.front()->getStmtClass() == Stmt::InitListExprClass),
      "otherwise caller is passing in incorrect initialization configuration");

  bool isCast = Kind.isCStyleCast();
  QualType destType = Entity.getType();
  ArTypeObjectKind destShape = GetTypeObjectKind(destType);

  // Direct initialization occurs for explicit constructor arguments.
  // E.g.: http://en.cppreference.com/w/cpp/language/direct_initialization
  if (Kind.getKind() == InitializationKind::IK_Direct &&
      destShape == AR_TOBJ_COMPOUND && !Kind.isCStyleOrFunctionalCast()) {
    m_sema->Diag(Kind.getLocation(),
                 diag::err_hlsl_require_numeric_base_for_ctor);
    SilenceSequenceDiagnostics(initSequence);
    return;
  }

  bool flatten = (Kind.getKind() == InitializationKind::IK_Direct && !isCast) ||
                 Kind.getKind() == InitializationKind::IK_DirectList ||
                 (Args.size() == 1 &&
                  Args.front()->getStmtClass() == Stmt::InitListExprClass);

  if (flatten) {
    // TODO: InitializationSequence::Perform in SemaInit should take the arity
    // of incomplete array types to adjust the value - we do calculate this as
    // part of type analysis. Until this is done, s_arr_i_f arr_struct_none[] =
    // { }; succeeds when it should instead fail.
    FlattenedTypeIterator::ComparisonResult comparisonResult =
        FlattenedTypeIterator::CompareTypesForInit(
            *this, destType, Args, Kind.getLocation(), Kind.getLocation());
    if (comparisonResult.IsConvertibleAndEqualLength() ||
        (isCast && comparisonResult.IsConvertibleAndLeftLonger())) {
      initSequence->AddListInitializationStep(destType);
    } else {
      SourceLocation diagLocation;
      if (Args.size() > 0) {
        diagLocation = Args.front()->getLocStart();
      } else {
        diagLocation = Entity.getDiagLoc();
      }

      if (comparisonResult.IsEqualLength()) {
        m_sema->Diag(diagLocation, diag::err_hlsl_type_mismatch);
      } else {
        m_sema->Diag(diagLocation, diag::err_incorrect_num_initializers)
            << (comparisonResult.RightCount < comparisonResult.LeftCount)
            << IsSubobjectType(destType) << comparisonResult.LeftCount
            << comparisonResult.RightCount;
      }
      SilenceSequenceDiagnostics(initSequence);
    }
  } else {
    DXASSERT(
        Args.size() == 1,
        "otherwise this was mis-parsed or should be a list initialization");
    Expr *firstArg = Args.front();
    if (IsExpressionBinaryComma(firstArg)) {
      m_sema->Diag(firstArg->getExprLoc(), diag::warn_hlsl_comma_in_init);
    }

    ExprResult expr = ExprResult(firstArg);
    Sema::CheckedConversionKind cck =
        Kind.isExplicitCast()
            ? Sema::CheckedConversionKind::CCK_CStyleCast
            : Sema::CheckedConversionKind::CCK_ImplicitConversion;
    unsigned int msg = 0;
    CastKind castKind;
    CXXCastPath basePath;
    SourceRange range = Kind.getRange();
    ImplicitConversionSequence ics;
    ics.setStandard();
    bool castWorked = TryStaticCastForHLSL(
        expr, destType, cck, range, msg, castKind, basePath,
        ListInitializationFalse, SuppressWarningsFalse, SuppressErrorsTrue,
        &ics.Standard);
    if (castWorked) {
      if (destType.getCanonicalType() ==
              firstArg->getType().getCanonicalType() &&
          (ics.Standard).First != ICK_Lvalue_To_Rvalue) {
        initSequence->AddCAssignmentStep(destType);
      } else {
        initSequence->AddConversionSequenceStep(
            ics, destType.getNonReferenceType(), TopLevelOfInitList);
      }
    } else {
      initSequence->SetFailed(InitializationSequence::FK_ConversionFailed);
    }
  }
}

bool HLSLExternalSource::IsConversionToLessOrEqualElements(
    const QualType &sourceType, const QualType &targetType,
    bool explicitConversion) {
  DXASSERT_NOMSG(!sourceType.isNull());
  DXASSERT_NOMSG(!targetType.isNull());

  ArTypeInfo sourceTypeInfo;
  ArTypeInfo targetTypeInfo;
  GetConversionForm(sourceType, explicitConversion, &sourceTypeInfo);
  GetConversionForm(targetType, explicitConversion, &targetTypeInfo);
  if (sourceTypeInfo.EltKind != targetTypeInfo.EltKind) {
    return false;
  }

  bool isVecMatTrunc = sourceTypeInfo.ShapeKind == AR_TOBJ_VECTOR &&
                       targetTypeInfo.ShapeKind == AR_TOBJ_BASIC;

  if (sourceTypeInfo.ShapeKind != targetTypeInfo.ShapeKind && !isVecMatTrunc) {
    return false;
  }

  if (sourceTypeInfo.ShapeKind == AR_TOBJ_OBJECT &&
      sourceTypeInfo.ObjKind == targetTypeInfo.ObjKind) {
    return true;
  }

  // Same struct is eqaul.
  if (sourceTypeInfo.ShapeKind == AR_TOBJ_COMPOUND &&
      sourceType.getCanonicalType().getUnqualifiedType() ==
          targetType.getCanonicalType().getUnqualifiedType()) {
    return true;
  }
  // DerivedFrom is less.
  if (sourceTypeInfo.ShapeKind == AR_TOBJ_COMPOUND ||
      GetTypeObjectKind(sourceType) == AR_TOBJ_COMPOUND) {
    const RecordType *targetRT = targetType->getAs<RecordType>();
    const RecordType *sourceRT = sourceType->getAs<RecordType>();

    if (targetRT && sourceRT) {
      RecordDecl *targetRD = targetRT->getDecl();
      RecordDecl *sourceRD = sourceRT->getDecl();
      const CXXRecordDecl *targetCXXRD = dyn_cast<CXXRecordDecl>(targetRD);
      const CXXRecordDecl *sourceCXXRD = dyn_cast<CXXRecordDecl>(sourceRD);
      if (targetCXXRD && sourceCXXRD) {
        if (sourceCXXRD->isDerivedFrom(targetCXXRD))
          return true;
      }
    }
  }

  if (sourceTypeInfo.ShapeKind != AR_TOBJ_SCALAR &&
      sourceTypeInfo.ShapeKind != AR_TOBJ_VECTOR &&
      sourceTypeInfo.ShapeKind != AR_TOBJ_MATRIX) {
    return false;
  }

  return targetTypeInfo.uTotalElts <= sourceTypeInfo.uTotalElts;
}

bool HLSLExternalSource::IsConversionToLessOrEqualElements(
    const ExprResult &sourceExpr, const QualType &targetType,
    bool explicitConversion) {
  if (sourceExpr.isInvalid() || targetType.isNull()) {
    return false;
  }

  return IsConversionToLessOrEqualElements(sourceExpr.get()->getType(),
                                           targetType, explicitConversion);
}

bool HLSLExternalSource::IsTypeNumeric(QualType type, UINT *count) {
  DXASSERT_NOMSG(!type.isNull());
  DXASSERT_NOMSG(count != nullptr);

  *count = 0;
  UINT subCount = 0;
  ArTypeObjectKind shapeKind = GetTypeObjectKind(type);
  switch (shapeKind) {
  case AR_TOBJ_ARRAY:
    if (IsTypeNumeric(m_context->getAsArrayType(type)->getElementType(),
                      &subCount)) {
      *count = subCount * GetArraySize(type);
      return true;
    }
    return false;
  case AR_TOBJ_COMPOUND: {
    UINT maxCount = 0;
    { // Determine maximum count to prevent infinite loop on incomplete array
      FlattenedTypeIterator itCount(SourceLocation(), type, *this);
      maxCount = itCount.countRemaining();
      if (!maxCount) {
        return false; // empty struct.
      }
    }
    FlattenedTypeIterator it(SourceLocation(), type, *this);
    while (it.hasCurrentElement()) {
      bool isFieldNumeric = IsTypeNumeric(it.getCurrentElement(), &subCount);
      if (!isFieldNumeric) {
        return false;
      }
      if (*count >= maxCount) {
        // this element is an incomplete array at the end; iterator will not
        // advance past this element. don't add to *count either, so *count will
        // represent minimum size of the structure.
        break;
      }
      *count += (subCount * it.getCurrentElementSize());
      it.advanceCurrentElement(it.getCurrentElementSize());
    }
    return true;
  }
  default:
    DXASSERT(false, "unreachable");
    return false;
  case AR_TOBJ_BASIC:
  case AR_TOBJ_MATRIX:
  case AR_TOBJ_VECTOR:
    *count = GetElementCount(type);
    return IsBasicKindNumeric(GetTypeElementKind(type));
  case AR_TOBJ_OBJECT:
  case AR_TOBJ_DEPENDENT:
  case AR_TOBJ_STRING:
    return false;
  }
}

enum MatrixMemberAccessError {
  MatrixMemberAccessError_None,             // No errors found.
  MatrixMemberAccessError_BadFormat,        // Formatting error (non-digit).
  MatrixMemberAccessError_MixingRefs,       // Mix of zero-based and one-based
                                            // references.
  MatrixMemberAccessError_Empty,            // No members specified.
  MatrixMemberAccessError_ZeroInOneBased,   // A zero was used in a one-based
                                            // reference.
  MatrixMemberAccessError_FourInZeroBased,  // A four was used in a zero-based
                                            // reference.
  MatrixMemberAccessError_TooManyPositions, // Too many positions (more than
                                            // four) were specified.
};

static MatrixMemberAccessError TryConsumeMatrixDigit(const char *&memberText,
                                                     uint32_t *value) {
  DXASSERT_NOMSG(memberText != nullptr);
  DXASSERT_NOMSG(value != nullptr);

  if ('0' <= *memberText && *memberText <= '9') {
    *value = (*memberText) - '0';
  } else {
    return MatrixMemberAccessError_BadFormat;
  }

  memberText++;
  return MatrixMemberAccessError_None;
}

static MatrixMemberAccessError
TryParseMatrixMemberAccess(const char *memberText,
                           MatrixMemberAccessPositions *value) {
  DXASSERT_NOMSG(memberText != nullptr);
  DXASSERT_NOMSG(value != nullptr);

  MatrixMemberAccessPositions result;
  bool zeroBasedDecided = false;
  bool zeroBased = false;

  // Set the output value to invalid to allow early exits when errors are found.
  value->IsValid = 0;

  // Assume this is true until proven otherwise.
  result.IsValid = 1;
  result.Count = 0;

  while (*memberText) {
    // Check for a leading underscore.
    if (*memberText != '_') {
      return MatrixMemberAccessError_BadFormat;
    }
    ++memberText;

    // Check whether we have an 'm' or a digit.
    if (*memberText == 'm') {
      if (zeroBasedDecided && !zeroBased) {
        return MatrixMemberAccessError_MixingRefs;
      }
      zeroBased = true;
      zeroBasedDecided = true;
      ++memberText;
    } else if (!('0' <= *memberText && *memberText <= '9')) {
      return MatrixMemberAccessError_BadFormat;
    } else {
      if (zeroBasedDecided && zeroBased) {
        return MatrixMemberAccessError_MixingRefs;
      }
      zeroBased = false;
      zeroBasedDecided = true;
    }

    // Consume two digits for the position.
    uint32_t rowPosition;
    uint32_t colPosition;
    MatrixMemberAccessError digitError;
    if (MatrixMemberAccessError_None !=
        (digitError = TryConsumeMatrixDigit(memberText, &rowPosition))) {
      return digitError;
    }
    if (MatrixMemberAccessError_None !=
        (digitError = TryConsumeMatrixDigit(memberText, &colPosition))) {
      return digitError;
    }

    // Look for specific common errors (developer likely mixed up reference
    // style).
    if (zeroBased) {
      if (rowPosition == 4 || colPosition == 4) {
        return MatrixMemberAccessError_FourInZeroBased;
      }
    } else {
      if (rowPosition == 0 || colPosition == 0) {
        return MatrixMemberAccessError_ZeroInOneBased;
      }

      // SetPosition will use zero-based indices.
      --rowPosition;
      --colPosition;
    }

    if (result.Count == 4) {
      return MatrixMemberAccessError_TooManyPositions;
    }

    result.SetPosition(result.Count, rowPosition, colPosition);
    result.Count++;
  }

  if (result.Count == 0) {
    return MatrixMemberAccessError_Empty;
  }

  *value = result;
  return MatrixMemberAccessError_None;
}

ExprResult HLSLExternalSource::LookupMatrixMemberExprForHLSL(
    Expr &BaseExpr, DeclarationName MemberName, bool IsArrow,
    SourceLocation OpLoc, SourceLocation MemberLoc) {
  QualType BaseType = BaseExpr.getType();
  DXASSERT(!BaseType.isNull(),
           "otherwise caller should have stopped analysis much earlier");
  DXASSERT(GetTypeObjectKind(BaseType) == AR_TOBJ_MATRIX,
           "Should only be called on known matrix types");

  QualType elementType;
  UINT rowCount, colCount;
  GetRowsAndCols(BaseType, rowCount, colCount);
  elementType = GetMatrixOrVectorElementType(BaseType);

  IdentifierInfo *member = MemberName.getAsIdentifierInfo();
  const char *memberText = member->getNameStart();
  MatrixMemberAccessPositions positions;
  MatrixMemberAccessError memberAccessError;
  unsigned msg = 0;

  memberAccessError = TryParseMatrixMemberAccess(memberText, &positions);
  switch (memberAccessError) {
  case MatrixMemberAccessError_BadFormat:
    msg = diag::err_hlsl_matrix_member_bad_format;
    break;
  case MatrixMemberAccessError_Empty:
    msg = diag::err_hlsl_matrix_member_empty;
    break;
  case MatrixMemberAccessError_FourInZeroBased:
    msg = diag::err_hlsl_matrix_member_four_in_zero_based;
    break;
  case MatrixMemberAccessError_MixingRefs:
    msg = diag::err_hlsl_matrix_member_mixing_refs;
    break;
  case MatrixMemberAccessError_None:
    msg = 0;
    DXASSERT(positions.IsValid, "otherwise an error should have been returned");
    // Check the position with the type now.
    for (unsigned int i = 0; i < positions.Count; i++) {
      uint32_t rowPos, colPos;
      positions.GetPosition(i, &rowPos, &colPos);
      if (rowPos >= rowCount || colPos >= colCount) {
        msg = diag::err_hlsl_matrix_member_out_of_bounds;
        break;
      }
    }
    break;
  case MatrixMemberAccessError_TooManyPositions:
    msg = diag::err_hlsl_matrix_member_too_many_positions;
    break;
  case MatrixMemberAccessError_ZeroInOneBased:
    msg = diag::err_hlsl_matrix_member_zero_in_one_based;
    break;
  default:
    llvm_unreachable("Unknown MatrixMemberAccessError value");
  }

  if (msg != 0) {
    m_sema->Diag(MemberLoc, msg) << memberText;

    // It's possible that it's a simple out-of-bounds condition. In this case,
    // generate the member access expression with the correct arity and continue
    // processing.
    if (!positions.IsValid) {
      return ExprError();
    }
  }

  DXASSERT(positions.IsValid, "otherwise an error should have been returned");

  // Consume elements
  QualType resultType;
  if (positions.Count == 1)
    resultType = elementType;
  else
    resultType =
        NewSimpleAggregateType(AR_TOBJ_UNKNOWN, GetTypeElementKind(elementType),
                               0, OneRow, positions.Count);

  // Add qualifiers from BaseType.
  resultType =
      m_context->getQualifiedType(resultType, BaseType.getQualifiers());

  ExprValueKind VK = positions.ContainsDuplicateElements()
                         ? VK_RValue
                         : (IsArrow ? VK_LValue : BaseExpr.getValueKind());
  ExtMatrixElementExpr *matrixExpr = new (m_context) ExtMatrixElementExpr(
      resultType, VK, &BaseExpr, *member, MemberLoc, positions);

  return matrixExpr;
}

enum VectorMemberAccessError {
  VectorMemberAccessError_None,         // No errors found.
  VectorMemberAccessError_BadFormat,    // Formatting error (not in 'rgba' or
                                        // 'xyzw').
  VectorMemberAccessError_MixingStyles, // Mix of rgba and xyzw swizzle styles.
  VectorMemberAccessError_Empty,        // No members specified.
  VectorMemberAccessError_TooManyPositions, // Too many positions (more than
                                            // four) were specified.
};

static VectorMemberAccessError TryConsumeVectorDigit(const char *&memberText,
                                                     uint32_t *value,
                                                     bool &rgbaStyle) {
  DXASSERT_NOMSG(memberText != nullptr);
  DXASSERT_NOMSG(value != nullptr);

  rgbaStyle = false;

  switch (*memberText) {
  case 'r':
    rgbaStyle = true;
    LLVM_FALLTHROUGH;
  case 'x':
    *value = 0;
    break;

  case 'g':
    rgbaStyle = true;
    LLVM_FALLTHROUGH;
  case 'y':
    *value = 1;
    break;

  case 'b':
    rgbaStyle = true;
    LLVM_FALLTHROUGH;
  case 'z':
    *value = 2;
    break;

  case 'a':
    rgbaStyle = true;
    LLVM_FALLTHROUGH;
  case 'w':
    *value = 3;
    break;

  default:
    return VectorMemberAccessError_BadFormat;
  }

  memberText++;
  return VectorMemberAccessError_None;
}

static VectorMemberAccessError
TryParseVectorMemberAccess(const char *memberText,
                           VectorMemberAccessPositions *value) {
  DXASSERT_NOMSG(memberText != nullptr);
  DXASSERT_NOMSG(value != nullptr);

  VectorMemberAccessPositions result;
  bool rgbaStyleDecided = false;
  bool rgbaStyle = false;

  // Set the output value to invalid to allow early exits when errors are found.
  value->IsValid = 0;

  // Assume this is true until proven otherwise.
  result.IsValid = 1;
  result.Count = 0;

  while (*memberText) {
    // Consume one character for the swizzle.
    uint32_t colPosition;
    VectorMemberAccessError digitError;
    bool rgbaStyleTmp = false;
    if (VectorMemberAccessError_None !=
        (digitError =
             TryConsumeVectorDigit(memberText, &colPosition, rgbaStyleTmp))) {
      return digitError;
    }

    if (rgbaStyleDecided && rgbaStyleTmp != rgbaStyle) {
      return VectorMemberAccessError_MixingStyles;
    } else {
      rgbaStyleDecided = true;
      rgbaStyle = rgbaStyleTmp;
    }

    if (result.Count == 4) {
      return VectorMemberAccessError_TooManyPositions;
    }

    result.SetPosition(result.Count, colPosition);
    result.Count++;
  }

  if (result.Count == 0) {
    return VectorMemberAccessError_Empty;
  }

  *value = result;
  return VectorMemberAccessError_None;
}

bool IsExprAccessingOutIndicesArray(Expr *BaseExpr) {
  switch (BaseExpr->getStmtClass()) {
  case Stmt::ArraySubscriptExprClass: {
    ArraySubscriptExpr *ase = cast<ArraySubscriptExpr>(BaseExpr);
    return IsExprAccessingOutIndicesArray(ase->getBase());
  }
  case Stmt::ImplicitCastExprClass: {
    ImplicitCastExpr *ice = cast<ImplicitCastExpr>(BaseExpr);
    return IsExprAccessingOutIndicesArray(ice->getSubExpr());
  }
  case Stmt::DeclRefExprClass: {
    DeclRefExpr *dre = cast<DeclRefExpr>(BaseExpr);
    ValueDecl *vd = dre->getDecl();
    if (vd->getAttr<HLSLIndicesAttr>() && vd->getAttr<HLSLOutAttr>()) {
      return true;
    }
    return false;
  }
  default:
    return false;
  }
}

ExprResult HLSLExternalSource::LookupVectorMemberExprForHLSL(
    Expr &BaseExpr, DeclarationName MemberName, bool IsArrow,
    SourceLocation OpLoc, SourceLocation MemberLoc) {
  QualType BaseType = BaseExpr.getType();
  DXASSERT(!BaseType.isNull(),
           "otherwise caller should have stopped analysis much earlier");
  DXASSERT(GetTypeObjectKind(BaseType) == AR_TOBJ_VECTOR,
           "Should only be called on known vector types");

  QualType elementType;
  UINT colCount = GetHLSLVecSize(BaseType);
  elementType = GetMatrixOrVectorElementType(BaseType);

  IdentifierInfo *member = MemberName.getAsIdentifierInfo();
  const char *memberText = member->getNameStart();
  VectorMemberAccessPositions positions;
  VectorMemberAccessError memberAccessError;
  unsigned msg = 0;

  memberAccessError = TryParseVectorMemberAccess(memberText, &positions);
  switch (memberAccessError) {
  case VectorMemberAccessError_BadFormat:
    msg = diag::err_hlsl_vector_member_bad_format;
    break;
  case VectorMemberAccessError_Empty:
    msg = diag::err_hlsl_vector_member_empty;
    break;
  case VectorMemberAccessError_MixingStyles:
    msg = diag::err_ext_vector_component_name_mixedsets;
    break;
  case VectorMemberAccessError_None:
    msg = 0;
    DXASSERT(positions.IsValid, "otherwise an error should have been returned");
    // Check the position with the type now.
    for (unsigned int i = 0; i < positions.Count; i++) {
      uint32_t colPos;
      positions.GetPosition(i, &colPos);
      if (colPos >= colCount) {
        msg = diag::err_hlsl_vector_member_out_of_bounds;
        break;
      }
    }
    break;
  case VectorMemberAccessError_TooManyPositions:
    msg = diag::err_hlsl_vector_member_too_many_positions;
    break;
  default:
    llvm_unreachable("Unknown VectorMemberAccessError value");
  }

  if (colCount > 4)
    msg = diag::err_hlsl_vector_member_on_long_vector;

  if (msg != 0) {
    m_sema->Diag(MemberLoc, msg) << memberText;

    // It's possible that it's a simple out-of-bounds condition. In this case,
    // generate the member access expression with the correct arity and continue
    // processing.
    if (!positions.IsValid) {
      return ExprError();
    }
  }

  DXASSERT(positions.IsValid, "otherwise an error should have been returned");

  // Disallow component access for out indices for DXIL path. We still allow
  // this in SPIR-V path.
  if (!m_sema->getLangOpts().SPIRV &&
      IsExprAccessingOutIndicesArray(&BaseExpr) && positions.Count < colCount) {
    m_sema->Diag(MemberLoc, diag::err_hlsl_out_indices_array_incorrect_access);
    return ExprError();
  }

  // Consume elements
  QualType resultType;
  if (positions.Count == 1)
    resultType = elementType;
  else
    resultType =
        NewSimpleAggregateType(AR_TOBJ_UNKNOWN, GetTypeElementKind(elementType),
                               0, OneRow, positions.Count);

  // Add qualifiers from BaseType.
  resultType =
      m_context->getQualifiedType(resultType, BaseType.getQualifiers());

  ExprValueKind VK = positions.ContainsDuplicateElements()
                         ? VK_RValue
                         : (IsArrow ? VK_LValue : BaseExpr.getValueKind());

  Expr *E = &BaseExpr;
  // Insert an lvalue-to-rvalue cast if necessary
  if (BaseExpr.getValueKind() == VK_LValue && VK == VK_RValue) {
    // Remove qualifiers from result type and cast target type
    resultType = resultType.getUnqualifiedType();
    auto targetType = E->getType().getUnqualifiedType();
    E = ImplicitCastExpr::Create(*m_context, targetType,
                                 CastKind::CK_LValueToRValue, E, nullptr,
                                 VK_RValue);
  }
  HLSLVectorElementExpr *vectorExpr = new (m_context)
      HLSLVectorElementExpr(resultType, VK, E, *member, MemberLoc, positions);

  return vectorExpr;
}

ExprResult HLSLExternalSource::LookupArrayMemberExprForHLSL(
    Expr &BaseExpr, DeclarationName MemberName, bool IsArrow,
    SourceLocation OpLoc, SourceLocation MemberLoc) {

  QualType BaseType = BaseExpr.getType();
  DXASSERT(!BaseType.isNull(),
           "otherwise caller should have stopped analysis much earlier");
  DXASSERT(GetTypeObjectKind(BaseType) == AR_TOBJ_ARRAY,
           "Should only be called on known array types");

  IdentifierInfo *member = MemberName.getAsIdentifierInfo();
  const char *memberText = member->getNameStart();

  // The only property available on arrays is Length; it is deprecated and
  // available only on HLSL version <=2018
  if (member->getLength() == 6 && 0 == strcmp(memberText, "Length")) {
    if (const ConstantArrayType *CAT = dyn_cast<ConstantArrayType>(BaseType)) {
      // check version support
      hlsl::LangStd hlslVer = getSema()->getLangOpts().HLSLVersion;
      if (hlslVer > hlsl::LangStd::v2016) {
        m_sema->Diag(MemberLoc, diag::err_hlsl_unsupported_for_version_lower)
            << "Length"
            << "2016";
        return ExprError();
      }
      if (hlslVer == hlsl::LangStd::v2016) {
        m_sema->Diag(MemberLoc, diag::warn_deprecated) << "Length";
      }

      UnaryExprOrTypeTraitExpr *arrayLenExpr = new (m_context)
          UnaryExprOrTypeTraitExpr(UETT_ArrayLength, &BaseExpr,
                                   m_context->getSizeType(), MemberLoc,
                                   BaseExpr.getSourceRange().getEnd());

      return arrayLenExpr;
    }
  }
  m_sema->Diag(MemberLoc, diag::err_typecheck_member_reference_struct_union)
      << BaseType << BaseExpr.getSourceRange() << MemberLoc;

  return ExprError();
}

ExprResult HLSLExternalSource::MaybeConvertMemberAccess(clang::Expr *E) {
  DXASSERT_NOMSG(E != nullptr);

  if (IsHLSLObjectWithImplicitMemberAccess(E->getType())) {
    QualType targetType = hlsl::GetHLSLResourceResultType(E->getType());
    if (IsHLSLObjectWithImplicitROMemberAccess(E->getType()))
      targetType = m_context->getConstType(targetType);
    return ImplicitCastExpr::Create(*m_context, targetType,
                                    CastKind::CK_FlatConversion, E, nullptr,
                                    E->getValueKind());
  }
  ArBasicKind basic = GetTypeElementKind(E->getType());
  if (!IS_BASIC_PRIMITIVE(basic)) {
    return E;
  }

  ArTypeObjectKind kind = GetTypeObjectKind(E->getType());
  if (kind != AR_TOBJ_SCALAR) {
    return E;
  }

  QualType targetType = NewSimpleAggregateType(AR_TOBJ_VECTOR, basic, 0, 1, 1);
  if (E->getObjectKind() ==
      OK_BitField) // if E is a bitfield, then generate an R value.
    E = ImplicitCastExpr::Create(*m_context, E->getType(),
                                 CastKind::CK_LValueToRValue, E, nullptr,
                                 VK_RValue);
  return ImplicitCastExpr::Create(*m_context, targetType,
                                  CastKind::CK_HLSLVectorSplat, E, nullptr,
                                  E->getValueKind());
}

static clang::CastKind
ImplicitConversionKindToCastKind(clang::ImplicitConversionKind ICK,
                                 ArBasicKind FromKind, ArBasicKind ToKind) {
  // TODO: Shouldn't we have more specific ICK enums so we don't have to
  // re-evaluate
  //        based on from/to kinds in order to determine CastKind?
  //  There's a FIXME note in PerformImplicitConversion that calls out exactly
  //  this problem.
  switch (ICK) {
  case ICK_Integral_Promotion:
  case ICK_Integral_Conversion:
    return CK_IntegralCast;
  case ICK_Floating_Promotion:
  case ICK_Floating_Conversion:
    return CK_FloatingCast;
  case ICK_Floating_Integral:
    if (IS_BASIC_FLOAT(FromKind) && IS_BASIC_AINT(ToKind))
      return CK_FloatingToIntegral;
    else if ((IS_BASIC_AINT(FromKind) || IS_BASIC_BOOL(FromKind)) &&
             IS_BASIC_FLOAT(ToKind))
      return CK_IntegralToFloating;
    break;
  case ICK_Boolean_Conversion:
    if (IS_BASIC_FLOAT(FromKind) && IS_BASIC_BOOL(ToKind))
      return CK_FloatingToBoolean;
    else if (IS_BASIC_AINT(FromKind) && IS_BASIC_BOOL(ToKind))
      return CK_IntegralToBoolean;
    break;
  default:
    // Only covers implicit conversions with cast kind equivalents.
    return CK_Invalid;
  }
  return CK_Invalid;
}
static clang::CastKind ConvertToComponentCastKind(clang::CastKind CK) {
  switch (CK) {
  case CK_IntegralCast:
    return CK_HLSLCC_IntegralCast;
  case CK_FloatingCast:
    return CK_HLSLCC_FloatingCast;
  case CK_FloatingToIntegral:
    return CK_HLSLCC_FloatingToIntegral;
  case CK_IntegralToFloating:
    return CK_HLSLCC_IntegralToFloating;
  case CK_FloatingToBoolean:
    return CK_HLSLCC_FloatingToBoolean;
  case CK_IntegralToBoolean:
    return CK_HLSLCC_IntegralToBoolean;
  default:
    // Only HLSLCC castkinds are relevant. Ignore the rest.
    return CK_Invalid;
  }
  return CK_Invalid;
}

clang::Expr *HLSLExternalSource::HLSLImpCastToScalar(clang::Sema *self,
                                                     clang::Expr *From,
                                                     ArTypeObjectKind FromShape,
                                                     ArBasicKind EltKind) {
  clang::CastKind CK = CK_Invalid;
  if (AR_TOBJ_MATRIX == FromShape)
    CK = CK_HLSLMatrixToScalarCast;
  if (AR_TOBJ_VECTOR == FromShape)
    CK = CK_HLSLVectorToScalarCast;
  if (CK_Invalid != CK) {
    return self
        ->ImpCastExprToType(
            From, NewSimpleAggregateType(AR_TOBJ_BASIC, EltKind, 0, 1, 1), CK,
            From->getValueKind())
        .get();
  }
  return From;
}

clang::ExprResult HLSLExternalSource::PerformHLSLConversion(
    clang::Expr *From, clang::QualType targetType,
    const clang::StandardConversionSequence &SCS,
    clang::Sema::CheckedConversionKind CCK) {
  QualType sourceType = From->getType();
  sourceType = GetStructuralForm(sourceType);
  targetType = GetStructuralForm(targetType);
  ArTypeInfo SourceInfo, TargetInfo;
  CollectInfo(sourceType, &SourceInfo);
  CollectInfo(targetType, &TargetInfo);

  clang::CastKind CK = CK_Invalid;
  QualType intermediateTarget;

  // TODO: construct vector/matrix and component cast expressions
  switch (SCS.Second) {
  case ICK_Flat_Conversion: {
    // TODO: determine how to handle individual component conversions:
    // - have an array of conversions for ComponentConversion in SCS?
    //    convert that to an array of casts under a special kind of flat
    //    flat conversion node?  What do component conversion casts cast
    //    from?  We don't have a From expression for individiual components.
    From = m_sema
               ->ImpCastExprToType(From, targetType.getUnqualifiedType(),
                                   CK_FlatConversion, From->getValueKind(),
                                   /*BasePath=*/0, CCK)
               .get();
    break;
  }
  case ICK_HLSL_Derived_To_Base: {
    CXXCastPath BasePath;
    if (m_sema->CheckDerivedToBaseConversion(
            sourceType, targetType.getNonReferenceType(), From->getLocStart(),
            From->getSourceRange(), &BasePath, /*IgnoreAccess=*/true))
      return ExprError();

    From = m_sema
               ->ImpCastExprToType(From, targetType.getUnqualifiedType(),
                                   CK_HLSLDerivedToBase, From->getValueKind(),
                                   &BasePath, CCK)
               .get();
    break;
  }
  case ICK_HLSLVector_Splat: {
    // 1. optionally convert from vec1 or mat1x1 to scalar
    From = HLSLImpCastToScalar(m_sema, From, SourceInfo.ShapeKind,
                               SourceInfo.EltKind);
    // 2. optionally convert component type
    if (ICK_Identity != SCS.ComponentConversion) {
      CK = ImplicitConversionKindToCastKind(
          SCS.ComponentConversion, SourceInfo.EltKind, TargetInfo.EltKind);
      if (CK_Invalid != CK) {
        From = m_sema
                   ->ImpCastExprToType(
                       From,
                       NewSimpleAggregateType(AR_TOBJ_BASIC, TargetInfo.EltKind,
                                              0, 1, 1),
                       CK, From->getValueKind(), /*BasePath=*/0, CCK)
                   .get();
      }
    }
    // 3. splat scalar to final vector or matrix
    CK = CK_Invalid;
    if (AR_TOBJ_VECTOR == TargetInfo.ShapeKind)
      CK = CK_HLSLVectorSplat;
    else if (AR_TOBJ_MATRIX == TargetInfo.ShapeKind)
      CK = CK_HLSLMatrixSplat;
    if (CK_Invalid != CK) {
      From =
          m_sema
              ->ImpCastExprToType(From,
                                  NewSimpleAggregateType(
                                      TargetInfo.ShapeKind, TargetInfo.EltKind,
                                      0, TargetInfo.uRows, TargetInfo.uCols),
                                  CK, From->getValueKind(), /*BasePath=*/0, CCK)
              .get();
    }
    break;
  }
  case ICK_HLSLVector_Scalar: {
    // 1. select vector or matrix component
    From = HLSLImpCastToScalar(m_sema, From, SourceInfo.ShapeKind,
                               SourceInfo.EltKind);
    // 2. optionally convert component type
    if (ICK_Identity != SCS.ComponentConversion) {
      CK = ImplicitConversionKindToCastKind(
          SCS.ComponentConversion, SourceInfo.EltKind, TargetInfo.EltKind);
      if (CK_Invalid != CK) {
        From = m_sema
                   ->ImpCastExprToType(
                       From,
                       NewSimpleAggregateType(AR_TOBJ_BASIC, TargetInfo.EltKind,
                                              0, 1, 1),
                       CK, From->getValueKind(), /*BasePath=*/0, CCK)
                   .get();
      }
    }
    break;
  }

  // The following two (three if we re-introduce ICK_HLSLComponent_Conversion)
  // steps can be done with case fall-through, since this is the order in which
  // we want to do the conversion operations.
  case ICK_HLSLVector_Truncation: {
    // 1. dimension truncation
    // vector truncation or matrix truncation?
    if (SourceInfo.ShapeKind == AR_TOBJ_VECTOR) {
      From = m_sema
                 ->ImpCastExprToType(
                     From,
                     NewSimpleAggregateType(AR_TOBJ_VECTOR, SourceInfo.EltKind,
                                            0, 1, TargetInfo.uTotalElts),
                     CK_HLSLVectorTruncationCast, From->getValueKind(),
                     /*BasePath=*/0, CCK)
                 .get();
    } else if (SourceInfo.ShapeKind == AR_TOBJ_MATRIX) {
      if (TargetInfo.ShapeKind == AR_TOBJ_VECTOR && 1 == SourceInfo.uCols) {
        // Handle the column to vector case
        From =
            m_sema
                ->ImpCastExprToType(
                    From,
                    NewSimpleAggregateType(AR_TOBJ_MATRIX, SourceInfo.EltKind,
                                           0, TargetInfo.uCols, 1),
                    CK_HLSLMatrixTruncationCast, From->getValueKind(),
                    /*BasePath=*/0, CCK)
                .get();
      } else {
        From =
            m_sema
                ->ImpCastExprToType(From,
                                    NewSimpleAggregateType(
                                        AR_TOBJ_MATRIX, SourceInfo.EltKind, 0,
                                        TargetInfo.uRows, TargetInfo.uCols),
                                    CK_HLSLMatrixTruncationCast,
                                    From->getValueKind(), /*BasePath=*/0, CCK)
                .get();
      }
    } else {
      DXASSERT(
          false,
          "PerformHLSLConversion: Invalid source type for truncation cast");
    }
  }
    LLVM_FALLTHROUGH;

  case ICK_HLSLVector_Conversion: {
    // 2. Do ShapeKind conversion if necessary
    if (SourceInfo.ShapeKind != TargetInfo.ShapeKind) {
      switch (TargetInfo.ShapeKind) {
      case AR_TOBJ_VECTOR:
        DXASSERT(AR_TOBJ_MATRIX == SourceInfo.ShapeKind,
                 "otherwise, invalid casting sequence");
        From =
            m_sema
                ->ImpCastExprToType(From,
                                    NewSimpleAggregateType(
                                        AR_TOBJ_VECTOR, SourceInfo.EltKind, 0,
                                        TargetInfo.uRows, TargetInfo.uCols),
                                    CK_HLSLMatrixToVectorCast,
                                    From->getValueKind(), /*BasePath=*/0, CCK)
                .get();
        break;
      case AR_TOBJ_MATRIX:
        DXASSERT(AR_TOBJ_VECTOR == SourceInfo.ShapeKind,
                 "otherwise, invalid casting sequence");
        From =
            m_sema
                ->ImpCastExprToType(From,
                                    NewSimpleAggregateType(
                                        AR_TOBJ_MATRIX, SourceInfo.EltKind, 0,
                                        TargetInfo.uRows, TargetInfo.uCols),
                                    CK_HLSLVectorToMatrixCast,
                                    From->getValueKind(), /*BasePath=*/0, CCK)
                .get();
        break;
      case AR_TOBJ_BASIC:
        // Truncation may be followed by cast to scalar
        From = HLSLImpCastToScalar(m_sema, From, SourceInfo.ShapeKind,
                                   SourceInfo.EltKind);
        break;
      default:
        DXASSERT(false, "otherwise, invalid casting sequence");
        break;
      }
    }

    // 3. Do component type conversion
    if (ICK_Identity != SCS.ComponentConversion) {
      CK = ImplicitConversionKindToCastKind(
          SCS.ComponentConversion, SourceInfo.EltKind, TargetInfo.EltKind);
      if (TargetInfo.ShapeKind != AR_TOBJ_BASIC)
        CK = ConvertToComponentCastKind(CK);
      if (CK_Invalid != CK) {
        From =
            m_sema
                ->ImpCastExprToType(From, targetType, CK, From->getValueKind(),
                                    /*BasePath=*/0, CCK)
                .get();
      }
    }
    break;
  }
  case ICK_Identity:
    // Nothing to do.
    break;
  default:
    DXASSERT(false,
             "PerformHLSLConversion: Invalid SCS.Second conversion kind");
  }
  return From;
}

void HLSLExternalSource::GetConversionForm(QualType type,
                                           bool explicitConversion,
                                           ArTypeInfo *pTypeInfo) {
  // if (!CollectInfo(AR_TINFO_ALLOW_ALL, pTypeInfo))
  CollectInfo(type, pTypeInfo);

  // The fxc implementation reported pTypeInfo->ShapeKind separately in an
  // output argument, but that value is only used for pointer conversions.

  // When explicitly converting types complex aggregates can be treated
  // as vectors if they are entirely numeric.
  switch (pTypeInfo->ShapeKind) {
  case AR_TOBJ_COMPOUND:
  case AR_TOBJ_ARRAY:
    if (explicitConversion && IsTypeNumeric(type, &pTypeInfo->uTotalElts)) {
      pTypeInfo->ShapeKind = AR_TOBJ_VECTOR;
    } else {
      pTypeInfo->ShapeKind = AR_TOBJ_COMPOUND;
    }

    DXASSERT_NOMSG(pTypeInfo->uRows == 1);
    pTypeInfo->uCols = pTypeInfo->uTotalElts;
    break;

  case AR_TOBJ_VECTOR:
  case AR_TOBJ_MATRIX:
    // Convert 1x1 types to scalars.
    if (pTypeInfo->uCols == 1 && pTypeInfo->uRows == 1) {
      pTypeInfo->ShapeKind = AR_TOBJ_BASIC;
    }
    break;
  default:
    // Only convertable shapekinds are relevant.
    break;
  }
}

static bool HandleVoidConversion(QualType source, QualType target,
                                 bool explicitConversion, bool *allowed) {
  DXASSERT_NOMSG(allowed != nullptr);
  bool applicable = true;
  *allowed = true;
  if (explicitConversion) {
    // (void) non-void
    if (target->isVoidType()) {
      DXASSERT_NOMSG(*allowed);
    }
    // (non-void) void
    else if (source->isVoidType()) {
      *allowed = false;
    } else {
      applicable = false;
    }
  } else {
    // (void) void
    if (source->isVoidType() && target->isVoidType()) {
      DXASSERT_NOMSG(*allowed);
    }
    // (void) non-void, (non-void) void
    else if (source->isVoidType() || target->isVoidType()) {
      *allowed = false;
    } else {
      applicable = false;
    }
  }
  return applicable;
}

static bool ConvertDimensions(ArTypeInfo TargetInfo, ArTypeInfo SourceInfo,
                              ImplicitConversionKind &Second,
                              TYPE_CONVERSION_REMARKS &Remarks) {
  // The rules for aggregate conversions are:
  // 1. A scalar can be replicated to any layout.
  // 2. Any type may be truncated to anything else with one component.
  // 3. A vector may be truncated to a smaller vector.
  // 4. A matrix may be truncated to a smaller matrix.
  // 5. The result of a vector and a matrix is:
  //    a. If the matrix has one row it's a vector-sized
  //       piece of the row.
  //    b. If the matrix has one column it's a vector-sized
  //       piece of the column.
  //    c. Otherwise the number of elements in the vector
  //       and matrix must match and the result is the vector.
  // 6. The result of a matrix and a vector is similar to #5.

  switch (TargetInfo.ShapeKind) {
  case AR_TOBJ_BASIC:
    switch (SourceInfo.ShapeKind) {
    case AR_TOBJ_BASIC:
      Second = ICK_Identity;
      break;

    case AR_TOBJ_VECTOR:
      if (1 < SourceInfo.uCols)
        Second = ICK_HLSLVector_Truncation;
      else
        Second = ICK_HLSLVector_Scalar;
      break;

    case AR_TOBJ_MATRIX:
      if (1 < SourceInfo.uRows * SourceInfo.uCols)
        Second = ICK_HLSLVector_Truncation;
      else
        Second = ICK_HLSLVector_Scalar;
      break;

    default:
      return false;
    }

    break;

  case AR_TOBJ_VECTOR:
    switch (SourceInfo.ShapeKind) {
    case AR_TOBJ_BASIC:
      // Conversions between scalars and aggregates are always supported.
      Second = ICK_HLSLVector_Splat;
      break;

    case AR_TOBJ_VECTOR:
      if (TargetInfo.uCols > SourceInfo.uCols) {
        if (SourceInfo.uCols == 1) {
          Second = ICK_HLSLVector_Splat;
        } else {
          return false;
        }
      } else if (TargetInfo.uCols < SourceInfo.uCols) {
        Second = ICK_HLSLVector_Truncation;
      } else {
        Second = ICK_Identity;
      }
      break;

    case AR_TOBJ_MATRIX: {
      UINT SourceComponents = SourceInfo.uRows * SourceInfo.uCols;
      if (1 == SourceComponents && TargetInfo.uCols != 1) {
        // splat: matrix<[..], 1, 1> -> vector<[..], O>
        Second = ICK_HLSLVector_Splat;
      } else if (1 == SourceInfo.uRows || 1 == SourceInfo.uCols) {
        // cases for: matrix<[..], M, N> -> vector<[..], O>, where N == 1 or M
        // == 1
        if (TargetInfo.uCols > SourceComponents) // illegal: O > N*M
          return false;
        else if (TargetInfo.uCols < SourceComponents) // truncation: O < N*M
          Second = ICK_HLSLVector_Truncation;
        else // equalivalent: O == N*M
          Second = ICK_HLSLVector_Conversion;
      } else if (TargetInfo.uCols == 1 && SourceComponents > 1) {
        Second = ICK_HLSLVector_Truncation;
      } else if (TargetInfo.uCols != SourceComponents) {
        // illegal: matrix<[..], M, N> -> vector<[..], O> where N != 1 and M !=
        // 1 and O != N*M
        return false;
      } else {
        // legal: matrix<[..], M, N> -> vector<[..], O> where N != 1 and M != 1
        // and O == N*M
        Second = ICK_HLSLVector_Conversion;
      }
      break;
    }

    default:
      return false;
    }

    break;

  case AR_TOBJ_MATRIX: {
    UINT TargetComponents = TargetInfo.uRows * TargetInfo.uCols;
    switch (SourceInfo.ShapeKind) {
    case AR_TOBJ_BASIC:
      // Conversions between scalars and aggregates are always supported.
      Second = ICK_HLSLVector_Splat;
      break;

    case AR_TOBJ_VECTOR: {
      // We can only convert vector to matrix in following cases:
      //  - splat from vector<...,1>
      //  - same number of components
      //  - one target component (truncate to scalar)
      //  - matrix has one row or one column, and fewer components (truncation)
      // Other cases disallowed even if implicitly convertable in two steps
      // (truncation+conversion).
      if (1 == SourceInfo.uCols && TargetComponents != 1) {
        // splat: vector<[..], 1> -> matrix<[..], M, N>
        Second = ICK_HLSLVector_Splat;
      } else if (TargetComponents == SourceInfo.uCols) {
        // legal: vector<[..], O> -> matrix<[..], M, N> where N != 1 and M != 1
        // and O == N*M
        Second = ICK_HLSLVector_Conversion;
      } else if (1 == TargetComponents) {
        // truncate to scalar: matrix<[..], 1, 1>
        Second = ICK_HLSLVector_Truncation;
      } else if ((1 == TargetInfo.uRows || 1 == TargetInfo.uCols) &&
                 TargetComponents < SourceInfo.uCols) {
        Second = ICK_HLSLVector_Truncation;
      } else {
        // illegal: change in components without going to or from scalar
        // equivalent
        return false;
      }
      break;
    }

    case AR_TOBJ_MATRIX: {
      UINT SourceComponents = SourceInfo.uRows * SourceInfo.uCols;
      if (1 == SourceComponents && TargetComponents != 1) {
        // splat: matrix<[..], 1, 1> -> matrix<[..], M, N>
        Second = ICK_HLSLVector_Splat;
      } else if (TargetComponents == 1) {
        Second = ICK_HLSLVector_Truncation;
      } else if (TargetInfo.uRows > SourceInfo.uRows ||
                 TargetInfo.uCols > SourceInfo.uCols) {
        return false;
      } else if (TargetInfo.uRows < SourceInfo.uRows ||
                 TargetInfo.uCols < SourceInfo.uCols) {
        Second = ICK_HLSLVector_Truncation;
      } else {
        Second = ICK_Identity;
      }
      break;
    }

    default:
      return false;
    }

    break;
  }
  case AR_TOBJ_STRING:
    if (SourceInfo.ShapeKind == AR_TOBJ_STRING) {
      Second = ICK_Identity;
      break;
    } else {
      return false;
    }

  default:
    return false;
  }

  if (TargetInfo.uTotalElts < SourceInfo.uTotalElts) {
    Remarks |= TYPE_CONVERSION_ELT_TRUNCATION;
  }

  return true;
}

static bool ConvertComponent(ArTypeInfo TargetInfo, ArTypeInfo SourceInfo,
                             ImplicitConversionKind &ComponentConversion,
                             TYPE_CONVERSION_REMARKS &Remarks) {
  // Conversion to/from unknown types not supported.
  if (TargetInfo.EltKind == AR_BASIC_UNKNOWN ||
      SourceInfo.EltKind == AR_BASIC_UNKNOWN) {
    return false;
  }

  bool precisionLoss = false;
  if (GET_BASIC_BITS(TargetInfo.EltKind) != 0 &&
      GET_BASIC_BITS(TargetInfo.EltKind) < GET_BASIC_BITS(SourceInfo.EltKind)) {
    precisionLoss = true;
    Remarks |= TYPE_CONVERSION_PRECISION_LOSS;
  }

  // enum -> enum not allowed
  if ((SourceInfo.EltKind == AR_BASIC_ENUM &&
       TargetInfo.EltKind == AR_BASIC_ENUM) ||
      SourceInfo.EltKind == AR_BASIC_ENUM_CLASS ||
      TargetInfo.EltKind == AR_BASIC_ENUM_CLASS) {
    return false;
  }
  if (SourceInfo.EltKind != TargetInfo.EltKind) {
    if (IS_BASIC_BOOL(TargetInfo.EltKind)) {
      ComponentConversion = ICK_Boolean_Conversion;
    } else if (IS_BASIC_ENUM(TargetInfo.EltKind)) {
      // conversion to enum type not allowed
      return false;
    } else if (IS_BASIC_ENUM(SourceInfo.EltKind)) {
      // enum -> int/float
      if (IS_BASIC_FLOAT(TargetInfo.EltKind))
        ComponentConversion = ICK_Floating_Integral;
      else
        ComponentConversion = ICK_Integral_Conversion;
    } else if (TargetInfo.EltKind == AR_OBJECT_STRING) {
      if (SourceInfo.EltKind == AR_OBJECT_STRING_LITERAL) {
        ComponentConversion = ICK_Array_To_Pointer;
      } else {
        return false;
      }
    } else {
      bool targetIsInt = IS_BASIC_AINT(TargetInfo.EltKind);
      if (IS_BASIC_AINT(SourceInfo.EltKind)) {
        if (targetIsInt) {
          ComponentConversion =
              precisionLoss ? ICK_Integral_Conversion : ICK_Integral_Promotion;
        } else {
          ComponentConversion = ICK_Floating_Integral;
        }
      } else if (IS_BASIC_FLOAT(SourceInfo.EltKind)) {
        if (targetIsInt) {
          ComponentConversion = ICK_Floating_Integral;
        } else {
          ComponentConversion =
              precisionLoss ? ICK_Floating_Conversion : ICK_Floating_Promotion;
        }
      } else if (IS_BASIC_BOOL(SourceInfo.EltKind)) {
        if (targetIsInt)
          ComponentConversion = ICK_Integral_Conversion;
        else
          ComponentConversion = ICK_Floating_Integral;
      }
    }
  } else if (TargetInfo.EltTy != SourceInfo.EltTy) {
    // Types are identical in HLSL, but not identical in clang,
    // such as unsigned long vs. unsigned int.
    // Add conversion based on the type.
    if (IS_BASIC_AINT(TargetInfo.EltKind))
      ComponentConversion = ICK_Integral_Conversion;
    else if (IS_BASIC_FLOAT(TargetInfo.EltKind))
      ComponentConversion = ICK_Floating_Conversion;
    else {
      DXASSERT(false, "unhandled case for conversion that's identical in HLSL, "
                      "but not in clang");
      return false;
    }
  }

  return true;
}

bool HLSLExternalSource::CanConvert(SourceLocation loc, Expr *sourceExpr,
                                    QualType target, bool explicitConversion,
                                    TYPE_CONVERSION_REMARKS *remarks,
                                    StandardConversionSequence *standard) {
  UINT uTSize, uSSize;
  bool SourceIsAggregate,
      TargetIsAggregate; // Early declarations due to gotos below

  DXASSERT_NOMSG(sourceExpr != nullptr);
  DXASSERT_NOMSG(!target.isNull());

  // Implements the semantics of ArType::CanConvertTo.
  TYPE_CONVERSION_FLAGS Flags =
      explicitConversion ? TYPE_CONVERSION_EXPLICIT : TYPE_CONVERSION_DEFAULT;
  TYPE_CONVERSION_REMARKS Remarks = TYPE_CONVERSION_NONE;
  QualType source = sourceExpr->getType();
  // Cannot cast function type.
  if (source->isFunctionType())
    return false;

  // Convert to an r-value to begin with, with an exception for strings
  // since they are not first-class values and we want to preserve them as
  // literals.
  bool needsLValueToRValue =
      sourceExpr->isLValue() && !target->isLValueReferenceType() &&
      sourceExpr->getStmtClass() != Expr::StringLiteralClass;

  bool targetRef = target->isReferenceType();
  bool TargetIsAnonymous = false;

  // Initialize the output standard sequence if available.
  if (standard != nullptr) {
    // Set up a no-op conversion, other than lvalue to rvalue - HLSL does not
    // support references.
    standard->setAsIdentityConversion();
    if (needsLValueToRValue) {
      standard->First = ICK_Lvalue_To_Rvalue;
    }

    standard->setFromType(source);
    standard->setAllToTypes(target);
  }

  source = GetStructuralForm(source);
  target = GetStructuralForm(target);

  // Temporary conversion kind tracking which will be used/fixed up at the end
  ImplicitConversionKind Second = ICK_Identity;
  ImplicitConversionKind ComponentConversion = ICK_Identity;

  // Identical types require no conversion.
  if (source == target) {
    Remarks = TYPE_CONVERSION_IDENTICAL;
    goto lSuccess;
  }

  // Trivial cases for void.
  bool allowed;
  if (HandleVoidConversion(source, target, explicitConversion, &allowed)) {
    if (allowed) {
      Remarks = target->isVoidType() ? TYPE_CONVERSION_TO_VOID : Remarks;
      goto lSuccess;
    } else {
      return false;
    }
  }

  ArTypeInfo TargetInfo, SourceInfo;
  CollectInfo(target, &TargetInfo);
  CollectInfo(source, &SourceInfo);

  uTSize = TargetInfo.uTotalElts;
  uSSize = SourceInfo.uTotalElts;

  // TODO: TYPE_CONVERSION_BY_REFERENCE does not seem possible here
  // are we missing cases?
  if ((Flags & TYPE_CONVERSION_BY_REFERENCE) != 0 && uTSize != uSSize) {
    return false;
  }

#ifdef ENABLE_SPIRV_CODEGEN
  // Cast vk::BufferPointer to pointer address.
  if (SourceInfo.EltKind == AR_OBJECT_VK_BUFFER_POINTER) {
    return TargetInfo.EltKind == AR_BASIC_UINT64;
  }
#endif

  // Cast cbuffer to its result value.
  if ((SourceInfo.EltKind == AR_OBJECT_CONSTANT_BUFFER ||
       SourceInfo.EltKind == AR_OBJECT_TEXTURE_BUFFER) &&
      TargetInfo.ShapeKind == AR_TOBJ_COMPOUND) {
    if (standard)
      standard->Second = ICK_Flat_Conversion;
    return hlsl::GetHLSLResourceResultType(source) == target;
  }

  // Structure cast.
  SourceIsAggregate = SourceInfo.ShapeKind == AR_TOBJ_COMPOUND ||
                      SourceInfo.ShapeKind == AR_TOBJ_ARRAY;
  TargetIsAggregate = TargetInfo.ShapeKind == AR_TOBJ_COMPOUND ||
                      TargetInfo.ShapeKind == AR_TOBJ_ARRAY;
  if (SourceIsAggregate || TargetIsAggregate) {
    // For implicit conversions, FXC treats arrays the same as structures
    // and rejects conversions between them and numeric types
    if (!explicitConversion && SourceIsAggregate != TargetIsAggregate) {
      return false;
    }

    // Structure to structure cases
    const RecordType *targetRT = dyn_cast<RecordType>(target);
    const RecordType *sourceRT = dyn_cast<RecordType>(source);
    if (targetRT && sourceRT) {
      RecordDecl *targetRD = targetRT->getDecl();
      RecordDecl *sourceRD = sourceRT->getDecl();
      if (sourceRT && targetRT) {
        if (targetRD == sourceRD) {
          Second = ICK_Flat_Conversion;
          goto lSuccess;
        }

        const CXXRecordDecl *targetCXXRD = dyn_cast<CXXRecordDecl>(targetRD);
        const CXXRecordDecl *sourceCXXRD = dyn_cast<CXXRecordDecl>(sourceRD);
        if (targetCXXRD && sourceCXXRD &&
            sourceCXXRD->isDerivedFrom(targetCXXRD)) {
          Second = ICK_HLSL_Derived_To_Base;
          goto lSuccess;
        }
        // There is no way to cast to anonymous structures.  So we allow legacy
        // HLSL implicit casts to matching anonymous structure types.
        TargetIsAnonymous = !targetRD->hasNameForLinkage();
      }
    }

    // Handle explicit splats from single element numerical types (scalars,
    // vector1s and matrix1x1s) to aggregate types.
    if (explicitConversion) {
      const BuiltinType *sourceSingleElementBuiltinType =
          source->getAs<BuiltinType>();
      if (sourceSingleElementBuiltinType == nullptr &&
          hlsl::IsHLSLVecMatType(source) &&
          hlsl::GetElementCount(source) == 1) {
        sourceSingleElementBuiltinType =
            hlsl::GetElementTypeOrType(source)->getAs<BuiltinType>();
      }

      // We can only splat to target types that do not contain object/resource
      // types
      if (sourceSingleElementBuiltinType != nullptr &&
          hlsl::IsHLSLNumericOrAggregateOfNumericType(target)) {
        BuiltinType::Kind kind = sourceSingleElementBuiltinType->getKind();
        switch (kind) {
        case BuiltinType::Kind::UInt:
        case BuiltinType::Kind::Int:
        case BuiltinType::Kind::Float:
        case BuiltinType::Kind::LitFloat:
        case BuiltinType::Kind::LitInt:
          Second = ICK_Flat_Conversion;
          goto lSuccess;
        default:
          // Only flat conversion kinds are relevant.
          break;
        }
      }
    } else if (m_sema->getLangOpts().HLSLVersion >= hlsl::LangStd::v2021 &&
               (SourceInfo.ShapeKind == AR_TOBJ_COMPOUND ||
                TargetInfo.ShapeKind == AR_TOBJ_COMPOUND) &&
               !TargetIsAnonymous) {
      // Not explicit, either are struct/class, not derived-to-base,
      // target is named (so explicit cast is possible),
      // and using strict UDT rules: disallow this implicit cast.
      return false;
    }

    FlattenedTypeIterator::ComparisonResult result =
        FlattenedTypeIterator::CompareTypes(*this, loc, loc, target, source);
    if (!result.CanConvertElements) {
      return false;
    }

    // Only allow scalar to compound or array with explicit cast
    if (result.IsConvertibleAndLeftLonger()) {
      if (!explicitConversion || SourceInfo.ShapeKind != AR_TOBJ_SCALAR) {
        return false;
      }
    }

    // Assignment is valid if elements are exactly the same in type and size; if
    // an explicit conversion is being done, we accept converted elements and a
    // longer right-hand sequence.
    if (!explicitConversion &&
        (!result.AreElementsEqual || result.IsRightLonger())) {
      return false;
    }
    Second = ICK_Flat_Conversion;
    goto lSuccess;
  }

  // Cast from Resource to Object types.
  if (SourceInfo.EltKind == AR_OBJECT_HEAP_RESOURCE ||
      SourceInfo.EltKind == AR_OBJECT_HEAP_SAMPLER) {
    // TODO: skip things like PointStream.
    if (TargetInfo.ShapeKind == AR_TOBJ_OBJECT) {
      Second = ICK_Flat_Conversion;
      goto lSuccess;
    }
  }

  // Convert scalar/vector/matrix dimensions
  if (!ConvertDimensions(TargetInfo, SourceInfo, Second, Remarks))
    return false;

  // Convert component type
  if (!ConvertComponent(TargetInfo, SourceInfo, ComponentConversion, Remarks))
    return false;

lSuccess:
  if (standard) {
    if (sourceExpr->isLValue()) {
      if (needsLValueToRValue) {
        // We don't need LValueToRValue cast before casting a derived object
        // to its base.
        if (Second == ICK_HLSL_Derived_To_Base) {
          standard->First = ICK_Identity;
        } else {
          standard->First = ICK_Lvalue_To_Rvalue;
        }
      } else {
        switch (Second) {
        case ICK_NoReturn_Adjustment:
        case ICK_Vector_Conversion:
        case ICK_Vector_Splat:
          DXASSERT(false,
                   "We shouldn't be producing these implicit conversion kinds");
          break;
        case ICK_Flat_Conversion:
        case ICK_HLSLVector_Splat:
          standard->First = ICK_Lvalue_To_Rvalue;
          break;
        default:
          // Only flat and splat conversions handled.
          break;
        }
        switch (ComponentConversion) {
        case ICK_Integral_Promotion:
        case ICK_Integral_Conversion:
        case ICK_Floating_Promotion:
        case ICK_Floating_Conversion:
        case ICK_Floating_Integral:
        case ICK_Boolean_Conversion:
          standard->First = ICK_Lvalue_To_Rvalue;
          break;
        case ICK_Array_To_Pointer:
          standard->First = ICK_Array_To_Pointer;
          break;
        default:
          // Only potential assignments above covered.
          break;
        }
      }
    }

    // Finally fix up the cases for scalar->scalar component conversion, and
    // identity vector/matrix component conversion
    if (ICK_Identity != ComponentConversion) {
      if (Second == ICK_Identity) {
        if (TargetInfo.ShapeKind == AR_TOBJ_BASIC) {
          // Scalar to scalar type conversion, use normal mechanism (Second)
          Second = ComponentConversion;
          ComponentConversion = ICK_Identity;
        } else if (TargetInfo.ShapeKind != AR_TOBJ_STRING) {
          // vector or matrix dimensions are not being changed, but component
          // type is being converted, so change Second to signal the conversion
          Second = ICK_HLSLVector_Conversion;
        }
      }
    }

    standard->Second = Second;
    standard->ComponentConversion = ComponentConversion;

    // For conversion which change to RValue but targeting reference type
    // Hold the conversion to codeGen
    if (targetRef && standard->First == ICK_Lvalue_To_Rvalue) {
      standard->First = ICK_Identity;
      standard->Second = ICK_Identity;
    }
  }

  AssignOpt(Remarks, remarks);
  return true;
}

bool HLSLExternalSource::ValidateTypeRequirements(SourceLocation loc,
                                                  ArBasicKind elementKind,
                                                  ArTypeObjectKind objectKind,
                                                  bool requiresIntegrals,
                                                  bool requiresNumerics) {
  if (objectKind == AR_TOBJ_DEPENDENT)
    return true;
  if (elementKind == AR_BASIC_DEPENDENT)
    return true;

  if (requiresIntegrals || requiresNumerics) {
    if (!IsObjectKindPrimitiveAggregate(objectKind)) {
      m_sema->Diag(loc, diag::err_hlsl_requires_non_aggregate);
      return false;
    }
  }

  if (requiresIntegrals) {
    if (!IsBasicKindIntegral(elementKind)) {
      m_sema->Diag(loc, diag::err_hlsl_requires_int_or_uint);
      return false;
    }
  } else if (requiresNumerics) {
    if (!IsBasicKindNumeric(elementKind)) {
      m_sema->Diag(loc, diag::err_hlsl_requires_numeric);
      return false;
    }
  }

  return true;
}

bool HLSLExternalSource::ValidatePrimitiveTypeForOperand(
    SourceLocation loc, QualType type, ArTypeObjectKind kind) {
  bool isValid = true;
  if (IsBuiltInObjectType(type)) {
    m_sema->Diag(loc, diag::err_hlsl_unsupported_builtin_op) << type;
    isValid = false;
  }
  if (kind == AR_TOBJ_COMPOUND) {
    m_sema->Diag(loc, diag::err_hlsl_unsupported_struct_op) << type;
    isValid = false;
  }
  return isValid;
}

HRESULT HLSLExternalSource::CombineDimensions(
    QualType leftType, QualType rightType, QualType *resultType,
    ImplicitConversionKind &convKind, TYPE_CONVERSION_REMARKS &Remarks) {
  ArTypeInfo leftInfo, rightInfo;
  CollectInfo(leftType, &leftInfo);
  CollectInfo(rightType, &rightInfo);

  // Prefer larger, or left if same.
  if (leftInfo.uTotalElts >= rightInfo.uTotalElts) {
    if (ConvertDimensions(leftInfo, rightInfo, convKind, Remarks))
      *resultType = leftType;
    else if (ConvertDimensions(rightInfo, leftInfo, convKind, Remarks))
      *resultType = rightType;
    else
      return E_FAIL;
  } else {
    if (ConvertDimensions(rightInfo, leftInfo, convKind, Remarks))
      *resultType = rightType;
    else if (ConvertDimensions(leftInfo, rightInfo, convKind, Remarks))
      *resultType = leftType;
    else
      return E_FAIL;
  }

  return S_OK;
}

/// <summary>Validates and adjusts operands for the specified binary
/// operator.</summary> <param name="OpLoc">Source location for
/// operator.</param> <param name="Opc">Kind of binary operator.</param> <param
/// name="LHS">Left-hand-side expression, possibly updated by this
/// function.</param> <param name="RHS">Right-hand-side expression, possibly
/// updated by this function.</param> <param name="ResultTy">Result type for
/// operator expression.</param> <param name="CompLHSTy">Type of LHS after
/// promotions for computation.</param> <param name="CompResultTy">Type of
/// computation result.</param>
void HLSLExternalSource::CheckBinOpForHLSL(SourceLocation OpLoc,
                                           BinaryOperatorKind Opc,
                                           ExprResult &LHS, ExprResult &RHS,
                                           QualType &ResultTy,
                                           QualType &CompLHSTy,
                                           QualType &CompResultTy) {
  // At the start, none of the output types should be valid.
  DXASSERT_NOMSG(ResultTy.isNull());
  DXASSERT_NOMSG(CompLHSTy.isNull());
  DXASSERT_NOMSG(CompResultTy.isNull());

  LHS = m_sema->CorrectDelayedTyposInExpr(LHS);
  RHS = m_sema->CorrectDelayedTyposInExpr(RHS);

  // If either expression is invalid to begin with, propagate that.
  if (LHS.isInvalid() || RHS.isInvalid()) {
    return;
  }

  // If there is a dependent type we will use that as the result type
  if (LHS.get()->getType()->isDependentType() ||
      RHS.get()->getType()->isDependentType()) {
    if (LHS.get()->getType()->isDependentType())
      ResultTy = LHS.get()->getType();
    else
      ResultTy = RHS.get()->getType();
    if (BinaryOperatorKindIsCompoundAssignment(Opc))
      CompResultTy = ResultTy;
    return;
  }

  // TODO: re-review the Check** in Clang and add equivalent diagnostics if/as
  // needed, possibly after conversions

  // Handle Assign and Comma operators and return
  switch (Opc) {
  case BO_AddAssign:
  case BO_AndAssign:
  case BO_DivAssign:
  case BO_MulAssign:
  case BO_RemAssign:
  case BO_ShlAssign:
  case BO_ShrAssign:
  case BO_SubAssign:
  case BO_OrAssign:
  case BO_XorAssign: {
    extern bool CheckForModifiableLvalue(Expr * E, SourceLocation Loc,
                                         Sema & S);
    if (CheckForModifiableLvalue(LHS.get(), OpLoc, *m_sema)) {
      return;
    }
  } break;
  case BO_Assign: {
    extern bool CheckForModifiableLvalue(Expr * E, SourceLocation Loc,
                                         Sema & S);
    if (CheckForModifiableLvalue(LHS.get(), OpLoc, *m_sema)) {
      return;
    }
    bool complained = false;
    ResultTy = LHS.get()->getType();
    if (m_sema->DiagnoseAssignmentResult(
            Sema::AssignConvertType::Compatible, OpLoc, ResultTy,
            RHS.get()->getType(), RHS.get(),
            Sema::AssignmentAction::AA_Assigning, &complained)) {
      return;
    }
    StandardConversionSequence standard;
    if (!ValidateCast(OpLoc, RHS.get(), ResultTy, ExplicitConversionFalse,
                      SuppressWarningsFalse, SuppressErrorsFalse, &standard)) {
      return;
    }
    if (RHS.get()->isLValue()) {
      standard.First = ICK_Lvalue_To_Rvalue;
    }
    RHS = m_sema->PerformImplicitConversion(RHS.get(), ResultTy, standard,
                                            Sema::AA_Converting,
                                            Sema::CCK_ImplicitConversion);
    return;
  } break;
  case BO_Comma:
    // C performs conversions, C++ doesn't but still checks for type
    // completeness. There are also diagnostics for improper comma use. In the
    // HLSL case these cases don't apply or simply aren't surfaced.
    ResultTy = RHS.get()->getType();
    return;
  default:
    // Only assign and comma operations handled.
    break;
  }

  // Leave this diagnostic for last to emulate fxc behavior.
  bool isCompoundAssignment = BinaryOperatorKindIsCompoundAssignment(Opc);
  bool unsupportedBoolLvalue =
      isCompoundAssignment &&
      !BinaryOperatorKindIsCompoundAssignmentForBool(Opc) &&
      GetTypeElementKind(LHS.get()->getType()) == AR_BASIC_BOOL;

  // Turn operand inputs into r-values.
  QualType LHSTypeAsPossibleLValue = LHS.get()->getType();
  if (!isCompoundAssignment) {
    LHS = m_sema->DefaultLvalueConversion(LHS.get());
  }
  RHS = m_sema->DefaultLvalueConversion(RHS.get());
  if (LHS.isInvalid() || RHS.isInvalid()) {
    return;
  }

  // Gather type info
  QualType leftType = GetStructuralForm(LHS.get()->getType());
  QualType rightType = GetStructuralForm(RHS.get()->getType());
  ArBasicKind leftElementKind = GetTypeElementKind(leftType);
  ArBasicKind rightElementKind = GetTypeElementKind(rightType);
  ArTypeObjectKind leftObjectKind = GetTypeObjectKind(leftType);
  ArTypeObjectKind rightObjectKind = GetTypeObjectKind(rightType);

  // Validate type requirements
  {
    bool requiresNumerics = BinaryOperatorKindRequiresNumeric(Opc);
    bool requiresIntegrals = BinaryOperatorKindRequiresIntegrals(Opc);

    if (!ValidateTypeRequirements(OpLoc, leftElementKind, leftObjectKind,
                                  requiresIntegrals, requiresNumerics)) {
      return;
    }
    if (!ValidateTypeRequirements(OpLoc, rightElementKind, rightObjectKind,
                                  requiresIntegrals, requiresNumerics)) {
      return;
    }
  }

  if (unsupportedBoolLvalue) {
    m_sema->Diag(OpLoc, diag::err_hlsl_unsupported_bool_lvalue_op);
    return;
  }

  // We don't support binary operators on built-in object types other than
  // assignment or commas.
  {
    DXASSERT(Opc != BO_Assign,
             "otherwise this wasn't handled as an early exit");
    DXASSERT(Opc != BO_Comma, "otherwise this wasn't handled as an early exit");
    bool isValid;
    isValid = ValidatePrimitiveTypeForOperand(OpLoc, leftType, leftObjectKind);
    if (leftType != rightType &&
        !ValidatePrimitiveTypeForOperand(OpLoc, rightType, rightObjectKind)) {
      isValid = false;
    }
    if (!isValid) {
      return;
    }
  }

  // We don't support equality comparisons on arrays.
  if ((Opc == BO_EQ || Opc == BO_NE) &&
      (leftObjectKind == AR_TOBJ_ARRAY || rightObjectKind == AR_TOBJ_ARRAY)) {
    m_sema->Diag(OpLoc, diag::err_hlsl_unsupported_array_equality_op);
    return;
  }

  // Combine element types for computation.
  ArBasicKind resultElementKind = leftElementKind;
  {
    if (BinaryOperatorKindIsLogical(Opc)) {
      if (m_sema->getLangOpts().HLSLVersion >= hlsl::LangStd::v2021) {
        // Only allow scalar types for logical operators &&, ||
        if (leftObjectKind != ArTypeObjectKind::AR_TOBJ_BASIC ||
            rightObjectKind != ArTypeObjectKind::AR_TOBJ_BASIC) {
          SmallVector<char, 256> Buff;
          llvm::raw_svector_ostream OS(Buff);
          PrintingPolicy PP(m_sema->getLangOpts());
          if (Opc == BinaryOperatorKind::BO_LAnd) {
            OS << "and(";
          } else if (Opc == BinaryOperatorKind::BO_LOr) {
            OS << "or(";
          }
          LHS.get()->printPretty(OS, nullptr, PP);
          OS << ", ";
          RHS.get()->printPretty(OS, nullptr, PP);
          OS << ")";
          SourceRange FullRange =
              SourceRange(LHS.get()->getLocStart(), RHS.get()->getLocEnd());
          m_sema->Diag(OpLoc, diag::err_hlsl_logical_binop_scalar)
              << (Opc == BinaryOperatorKind::BO_LOr)
              << FixItHint::CreateReplacement(FullRange, OS.str());
          return;
        }
      }
      resultElementKind = AR_BASIC_BOOL;
    } else if (!BinaryOperatorKindIsBitwiseShift(Opc) &&
               leftElementKind != rightElementKind) {
      if (!CombineBasicTypes(leftElementKind, rightElementKind,
                             &resultElementKind)) {
        m_sema->Diag(OpLoc, diag::err_hlsl_type_mismatch);
        return;
      }
    } else if (BinaryOperatorKindIsBitwiseShift(Opc) &&
               (resultElementKind == AR_BASIC_LITERAL_INT ||
                resultElementKind == AR_BASIC_LITERAL_FLOAT) &&
               rightElementKind != AR_BASIC_LITERAL_INT &&
               rightElementKind != AR_BASIC_LITERAL_FLOAT) {
      // For case like 1<<x.
      m_sema->Diag(OpLoc, diag::warn_hlsl_ambiguous_literal_shift);
      if (rightElementKind == AR_BASIC_UINT32)
        resultElementKind = AR_BASIC_UINT32;
      else
        resultElementKind = AR_BASIC_INT32;
    } else if (resultElementKind == AR_BASIC_BOOL &&
               BinaryOperatorKindRequiresBoolAsNumeric(Opc)) {
      resultElementKind = AR_BASIC_INT32;
    }

    // The following combines the selected/combined element kind above with
    // the dimensions that are legal to implicitly cast.  This means that
    // element kind may be taken from one side and the dimensions from the
    // other.

    if (!isCompoundAssignment) {
      // Legal dimension combinations are identical, splat, and truncation.
      // ResultTy will be set to whichever type can be converted to, if legal,
      // with preference for leftType if both are possible.
      if (FAILED(CombineDimensions(LHS.get()->getType(), RHS.get()->getType(),
                                   &ResultTy))) {
        // Just choose leftType, and allow ValidateCast to catch this later
        ResultTy = LHS.get()->getType();
      }
    } else {
      ResultTy = LHS.get()->getType();
    }

    // Here, element kind is combined with dimensions for computation type, if
    // different.
    if (resultElementKind != GetTypeElementKind(ResultTy)) {
      UINT rowCount, colCount;
      GetRowsAndColsForAny(ResultTy, rowCount, colCount);
      ResultTy =
          NewSimpleAggregateType(GetTypeObjectKind(ResultTy), resultElementKind,
                                 0, rowCount, colCount);
    }
  }

  bool bFailedFirstRHSCast = false;

  // Perform necessary conversion sequences for LHS and RHS
  if (RHS.get()->getType() != ResultTy) {
    StandardConversionSequence standard;
    // Suppress type narrowing or truncation warnings for RHS on bitwise shift,
    // since we only care about the LHS type.
    bool bSuppressWarnings = BinaryOperatorKindIsBitwiseShift(Opc);
    // Suppress errors on compound assignment, since we will validate the cast
    // to the final type later.
    bool bSuppressErrors = isCompoundAssignment;
    // Suppress errors if either operand has a dependent type.
    if (RHS.get()->getType()->isDependentType() || ResultTy->isDependentType())
      bSuppressErrors = true;
    // If compound assignment, suppress errors until later, but report warning
    // (vector truncation/type narrowing) here.
    if (ValidateCast(SourceLocation(), RHS.get(), ResultTy,
                     ExplicitConversionFalse, bSuppressWarnings,
                     bSuppressErrors, &standard)) {
      if (standard.First != ICK_Identity || !standard.isIdentityConversion())
        RHS = m_sema->PerformImplicitConversion(RHS.get(), ResultTy, standard,
                                                Sema::AA_Casting,
                                                Sema::CCK_ImplicitConversion);
    } else if (!isCompoundAssignment) {
      // If compound assignment, validate cast from RHS directly to LHS later,
      // otherwise, fail here.
      ResultTy = QualType();
      return;
    } else {
      bFailedFirstRHSCast = true;
    }
  }

  if (isCompoundAssignment) {
    CompResultTy = ResultTy;
    CompLHSTy = CompResultTy;

    // For a compound operation, C/C++ promotes both types, performs the
    // arithmetic, then converts to the result type and then assigns.
    //
    // So int + float promotes the int to float, does a floating-point addition,
    // then the result becomes and int and is assigned.
    ResultTy = LHSTypeAsPossibleLValue;

    // Validate remainder of cast from computation type to final result type
    StandardConversionSequence standard;
    if (!ValidateCast(SourceLocation(), RHS.get(), ResultTy,
                      ExplicitConversionFalse, SuppressWarningsFalse,
                      SuppressErrorsFalse, &standard)) {
      ResultTy = QualType();
      return;
    }
    DXASSERT_LOCALVAR(bFailedFirstRHSCast, !bFailedFirstRHSCast,
                      "otherwise, hit compound assign case that failed RHS -> "
                      "CompResultTy cast, but succeeded RHS -> LHS cast.");

  } else if (LHS.get()->getType() != ResultTy) {
    StandardConversionSequence standard;
    if (ValidateCast(SourceLocation(), LHS.get(), ResultTy,
                     ExplicitConversionFalse, SuppressWarningsFalse,
                     SuppressErrorsFalse, &standard)) {
      if (standard.First != ICK_Identity || !standard.isIdentityConversion())
        LHS = m_sema->PerformImplicitConversion(LHS.get(), ResultTy, standard,
                                                Sema::AA_Casting,
                                                Sema::CCK_ImplicitConversion);
    } else {
      ResultTy = QualType();
      return;
    }
  }

  if (BinaryOperatorKindIsComparison(Opc) || BinaryOperatorKindIsLogical(Opc)) {
    DXASSERT(!isCompoundAssignment,
             "otherwise binary lookup tables are inconsistent");
    // Return bool vector for vector types.
    if (IsVectorType(m_sema, ResultTy)) {
      UINT rowCount, colCount;
      GetRowsAndColsForAny(ResultTy, rowCount, colCount);
      ResultTy =
          LookupVectorType(HLSLScalarType::HLSLScalarType_bool, colCount);
    } else if (IsMatrixType(m_sema, ResultTy)) {
      UINT rowCount, colCount;
      GetRowsAndColsForAny(ResultTy, rowCount, colCount);
      ResultTy = LookupMatrixType(HLSLScalarType::HLSLScalarType_bool, rowCount,
                                  colCount);
    } else
      ResultTy = m_context->BoolTy.withConst();
  }

  // Run diagnostics. Some are emulating checks that occur in IR emission in
  // fxc.
  if (Opc == BO_Div || Opc == BO_DivAssign || Opc == BO_Rem ||
      Opc == BO_RemAssign) {
    if (IsBasicKindIntMinPrecision(resultElementKind)) {
      m_sema->Diag(OpLoc, diag::err_hlsl_unsupported_div_minint);
      return;
    }
  }

  if (Opc == BO_Rem || Opc == BO_RemAssign) {
    if (resultElementKind == AR_BASIC_FLOAT64) {
      m_sema->Diag(OpLoc, diag::err_hlsl_unsupported_mod_double);
      return;
    }
  }
}

/// <summary>Validates and adjusts operands for the specified unary
/// operator.</summary> <param name="OpLoc">Source location for
/// operator.</param> <param name="Opc">Kind of operator.</param> <param
/// name="InputExpr">Input expression to the operator.</param> <param
/// name="VK">Value kind for resulting expression.</param> <param
/// name="OK">Object kind for resulting expression.</param> <returns>The result
/// type for the expression.</returns>
QualType HLSLExternalSource::CheckUnaryOpForHLSL(SourceLocation OpLoc,
                                                 UnaryOperatorKind Opc,
                                                 ExprResult &InputExpr,
                                                 ExprValueKind &VK,
                                                 ExprObjectKind &OK) {
  InputExpr = m_sema->CorrectDelayedTyposInExpr(InputExpr);

  if (InputExpr.isInvalid())
    return QualType();

  // Reject unsupported operators * and &
  switch (Opc) {
  case UO_AddrOf:
  case UO_Deref:
    m_sema->Diag(OpLoc, diag::err_hlsl_unsupported_operator);
    return QualType();
  default:
    // Only * and & covered.
    break;
  }

  Expr *expr = InputExpr.get();
  if (expr->isTypeDependent())
    return m_context->DependentTy;

  ArBasicKind elementKind = GetTypeElementKind(expr->getType());

  if (UnaryOperatorKindRequiresModifiableValue(Opc)) {
    if (elementKind == AR_BASIC_ENUM) {
      bool isInc = IsIncrementOp(Opc);
      m_sema->Diag(OpLoc, diag::err_increment_decrement_enum)
          << isInc << expr->getType();
      return QualType();
    }

    extern bool CheckForModifiableLvalue(Expr * E, SourceLocation Loc,
                                         Sema & S);
    if (CheckForModifiableLvalue(expr, OpLoc, *m_sema))
      return QualType();
  } else {
    InputExpr = m_sema->DefaultLvalueConversion(InputExpr.get()).get();
    if (InputExpr.isInvalid())
      return QualType();
  }

  if (UnaryOperatorKindDisallowsBool(Opc) && IS_BASIC_BOOL(elementKind)) {
    m_sema->Diag(OpLoc, diag::err_hlsl_unsupported_bool_lvalue_op);
    return QualType();
  }

  if (UnaryOperatorKindRequiresBoolAsNumeric(Opc)) {
    InputExpr = PromoteToIntIfBool(InputExpr);
    expr = InputExpr.get();
    elementKind = GetTypeElementKind(expr->getType());
  }

  ArTypeObjectKind objectKind = GetTypeObjectKind(expr->getType());
  bool requiresIntegrals = UnaryOperatorKindRequiresIntegrals(Opc);
  bool requiresNumerics = UnaryOperatorKindRequiresNumerics(Opc);
  if (!ValidateTypeRequirements(OpLoc, elementKind, objectKind,
                                requiresIntegrals, requiresNumerics)) {
    return QualType();
  }

  if (Opc == UnaryOperatorKind::UO_Minus) {
    if (IS_BASIC_UINT(Opc)) {
      m_sema->Diag(OpLoc, diag::warn_hlsl_unary_negate_unsigned);
    }
  }

  // By default, the result type is the operand type.
  // Logical not however should cast to a bool.
  QualType resultType = expr->getType();
  if (Opc == UnaryOperatorKind::UO_LNot) {
    UINT rowCount, colCount;
    GetRowsAndColsForAny(expr->getType(), rowCount, colCount);
    resultType = NewSimpleAggregateType(objectKind, AR_BASIC_BOOL,
                                        AR_QUAL_CONST, rowCount, colCount);
    StandardConversionSequence standard;
    if (!CanConvert(OpLoc, expr, resultType, false, nullptr, &standard)) {
      m_sema->Diag(OpLoc, diag::err_hlsl_requires_bool_for_not);
      return QualType();
    }

    // Cast argument.
    ExprResult result = m_sema->PerformImplicitConversion(
        InputExpr.get(), resultType, standard, Sema::AA_Casting,
        Sema::CCK_ImplicitConversion);
    if (result.isUsable()) {
      InputExpr = result.get();
    }
  }

  bool isPrefix = Opc == UO_PreInc || Opc == UO_PreDec;
  if (isPrefix) {
    VK = VK_LValue;
    return resultType;
  } else {
    VK = VK_RValue;
    return resultType.getUnqualifiedType();
  }
}

clang::QualType
HLSLExternalSource::CheckVectorConditional(ExprResult &Cond, ExprResult &LHS,
                                           ExprResult &RHS,
                                           SourceLocation QuestionLoc) {

  Cond = m_sema->CorrectDelayedTyposInExpr(Cond);
  LHS = m_sema->CorrectDelayedTyposInExpr(LHS);
  RHS = m_sema->CorrectDelayedTyposInExpr(RHS);

  // If either expression is invalid to begin with, propagate that.
  if (Cond.isInvalid() || LHS.isInvalid() || RHS.isInvalid()) {
    return QualType();
  }

  // Gather type info
  QualType condType = GetStructuralForm(Cond.get()->getType());
  QualType leftType = GetStructuralForm(LHS.get()->getType());
  QualType rightType = GetStructuralForm(RHS.get()->getType());

  // If any type is dependent, we will use that as the type to return.
  if (leftType->isDependentType())
    return leftType;
  if (rightType->isDependentType())
    return rightType;
  if (condType->isDependentType())
    return condType;

  ArBasicKind condElementKind = GetTypeElementKind(condType);
  ArBasicKind leftElementKind = GetTypeElementKind(leftType);
  ArBasicKind rightElementKind = GetTypeElementKind(rightType);
  ArTypeObjectKind condObjectKind = GetTypeObjectKind(condType);
  ArTypeObjectKind leftObjectKind = GetTypeObjectKind(leftType);
  ArTypeObjectKind rightObjectKind = GetTypeObjectKind(rightType);

  QualType ResultTy = leftType;

  if (m_sema->getLangOpts().HLSLVersion >= hlsl::LangStd::v2021) {
    // Only allow scalar.
    if (condObjectKind == AR_TOBJ_VECTOR || condObjectKind == AR_TOBJ_MATRIX) {
      SmallVector<char, 256> Buff;
      llvm::raw_svector_ostream OS(Buff);
      PrintingPolicy PP(m_sema->getLangOpts());
      OS << "select(";
      Cond.get()->printPretty(OS, nullptr, PP);
      OS << ", ";
      LHS.get()->printPretty(OS, nullptr, PP);
      OS << ", ";
      RHS.get()->printPretty(OS, nullptr, PP);
      OS << ")";
      SourceRange FullRange =
          SourceRange(Cond.get()->getLocStart(), RHS.get()->getLocEnd());
      m_sema->Diag(QuestionLoc, diag::err_hlsl_ternary_scalar)
          << FixItHint::CreateReplacement(FullRange, OS.str());
      return QualType();
    }
  }

  bool condIsSimple = condObjectKind == AR_TOBJ_BASIC ||
                      condObjectKind == AR_TOBJ_VECTOR ||
                      condObjectKind == AR_TOBJ_MATRIX;
  if (!condIsSimple) {
    m_sema->Diag(QuestionLoc, diag::err_hlsl_conditional_cond_typecheck);
    return QualType();
  }

  UINT rowCountCond, colCountCond;
  GetRowsAndColsForAny(condType, rowCountCond, colCountCond);

  bool leftIsSimple = leftObjectKind == AR_TOBJ_BASIC ||
                      leftObjectKind == AR_TOBJ_VECTOR ||
                      leftObjectKind == AR_TOBJ_MATRIX;
  bool rightIsSimple = rightObjectKind == AR_TOBJ_BASIC ||
                       rightObjectKind == AR_TOBJ_VECTOR ||
                       rightObjectKind == AR_TOBJ_MATRIX;

  if (!leftIsSimple || !rightIsSimple) {
    if (leftObjectKind == AR_TOBJ_OBJECT && leftObjectKind == AR_TOBJ_OBJECT) {
      if (leftType == rightType) {
        return leftType;
      }
    }
    // NOTE: Limiting this operator to working only on basic numeric types.
    // This is due to extremely limited (and even broken) support for any other
    // case. In the future we may decide to support more cases.
    m_sema->Diag(QuestionLoc, diag::err_hlsl_conditional_result_typecheck);
    return QualType();
  }

  // Types should be only scalar, vector, or matrix after this point.

  ArBasicKind resultElementKind = leftElementKind;
  // Combine LHS and RHS element types for computation.
  if (leftElementKind != rightElementKind) {
    if (!CombineBasicTypes(leftElementKind, rightElementKind,
                           &resultElementKind)) {
      m_sema->Diag(QuestionLoc,
                   diag::err_hlsl_conditional_result_comptype_mismatch);
      return QualType();
    }
  }

  // Restore left/right type to original to avoid stripping attributed type or
  // typedef type
  leftType = LHS.get()->getType();
  rightType = RHS.get()->getType();

  // Combine LHS and RHS dimensions
  if (FAILED(CombineDimensions(leftType, rightType, &ResultTy))) {
    m_sema->Diag(QuestionLoc, diag::err_hlsl_conditional_result_dimensions);
    return QualType();
  }

  UINT rowCount, colCount;
  GetRowsAndColsForAny(ResultTy, rowCount, colCount);

  // If result is scalar, use condition dimensions.
  // Otherwise, condition must either match or is scalar, then use result
  // dimensions
  if (rowCount * colCount == 1) {
    rowCount = rowCountCond;
    colCount = colCountCond;
  } else if (rowCountCond * colCountCond != 1 &&
             (rowCountCond != rowCount || colCountCond != colCount)) {
    m_sema->Diag(QuestionLoc, diag::err_hlsl_conditional_dimensions);
    return QualType();
  }

  // Here, element kind is combined with dimensions for primitive types.
  if (IS_BASIC_PRIMITIVE(resultElementKind)) {
    ResultTy = NewSimpleAggregateType(AR_TOBJ_INVALID, resultElementKind, 0,
                                      rowCount, colCount)
                   ->getCanonicalTypeInternal();
  } else {
    DXASSERT(rowCount == 1 && colCount == 1,
             "otherwise, attempting to construct vector or matrix with "
             "non-primitive component type");
    ResultTy = ResultTy.getUnqualifiedType();
  }

  // Cast condition to RValue
  if (Cond.get()->isLValue())
    Cond.set(CreateLValueToRValueCast(Cond.get()));

  // Convert condition component type to bool, using result component dimensions
  QualType boolType;
  // If short-circuiting, condition must be scalar.
  if (m_sema->getLangOpts().HLSLVersion >= hlsl::LangStd::v2021)
    boolType = NewSimpleAggregateType(AR_TOBJ_INVALID, AR_BASIC_BOOL, 0, 1, 1)
                   ->getCanonicalTypeInternal();
  else
    boolType = NewSimpleAggregateType(AR_TOBJ_INVALID, AR_BASIC_BOOL, 0,
                                      rowCount, colCount)
                   ->getCanonicalTypeInternal();

  if (condElementKind != AR_BASIC_BOOL || condType != boolType) {
    StandardConversionSequence standard;
    if (ValidateCast(SourceLocation(), Cond.get(), boolType,
                     ExplicitConversionFalse, SuppressWarningsFalse,
                     SuppressErrorsFalse, &standard)) {
      if (standard.First != ICK_Identity || !standard.isIdentityConversion())
        Cond = m_sema->PerformImplicitConversion(Cond.get(), boolType, standard,
                                                 Sema::AA_Casting,
                                                 Sema::CCK_ImplicitConversion);
    } else {
      return QualType();
    }
  }

  // TODO: Is this correct?  Does fxc support lvalue return here?
  // Cast LHS/RHS to RValue
  if (LHS.get()->isLValue())
    LHS.set(CreateLValueToRValueCast(LHS.get()));
  if (RHS.get()->isLValue())
    RHS.set(CreateLValueToRValueCast(RHS.get()));

  if (leftType != ResultTy) {
    StandardConversionSequence standard;
    if (ValidateCast(SourceLocation(), LHS.get(), ResultTy,
                     ExplicitConversionFalse, SuppressWarningsFalse,
                     SuppressErrorsFalse, &standard)) {
      if (standard.First != ICK_Identity || !standard.isIdentityConversion())
        LHS = m_sema->PerformImplicitConversion(LHS.get(), ResultTy, standard,
                                                Sema::AA_Casting,
                                                Sema::CCK_ImplicitConversion);
    } else {
      return QualType();
    }
  }
  if (rightType != ResultTy) {
    StandardConversionSequence standard;
    if (ValidateCast(SourceLocation(), RHS.get(), ResultTy,
                     ExplicitConversionFalse, SuppressWarningsFalse,
                     SuppressErrorsFalse, &standard)) {
      if (standard.First != ICK_Identity || !standard.isIdentityConversion())
        RHS = m_sema->PerformImplicitConversion(RHS.get(), ResultTy, standard,
                                                Sema::AA_Casting,
                                                Sema::CCK_ImplicitConversion);
    } else {
      return QualType();
    }
  }

  return ResultTy;
}

// Apply type specifier sign to the given QualType.
// Other than privmitive int type, only allow shorthand vectors and matrices to
// be unsigned.
clang::QualType
HLSLExternalSource::ApplyTypeSpecSignToParsedType(clang::QualType &type,
                                                  clang::TypeSpecifierSign TSS,
                                                  clang::SourceLocation Loc) {
  if (TSS == TypeSpecifierSign::TSS_unspecified) {
    return type;
  }
  DXASSERT(TSS != TypeSpecifierSign::TSS_signed,
           "else signed keyword is supported in HLSL");
  ArTypeObjectKind objKind = GetTypeObjectKind(type);
  if (objKind != AR_TOBJ_VECTOR && objKind != AR_TOBJ_MATRIX &&
      objKind != AR_TOBJ_BASIC && objKind != AR_TOBJ_ARRAY) {
    return type;
  }
  // check if element type is unsigned and check if such vector exists
  // If not create a new one, Make a QualType of the new kind
  ArBasicKind elementKind = GetTypeElementKind(type);
  // Only ints can have signed/unsigend ty
  if (!IS_BASIC_UNSIGNABLE(elementKind)) {
    return type;
  } else {
    // Check given TypeSpecifierSign. If unsigned, change int to uint.
    HLSLScalarType scalarType = ScalarTypeForBasic(elementKind);
    HLSLScalarType newScalarType = MakeUnsigned(scalarType);

    // Get new vector types for a given TypeSpecifierSign.
    if (objKind == AR_TOBJ_VECTOR) {
      UINT colCount = GetHLSLVecSize(type);
      TypedefDecl *qts = LookupVectorShorthandType(newScalarType, colCount);
      return m_context->getTypeDeclType(qts);
    } else if (objKind == AR_TOBJ_MATRIX) {
      UINT rowCount, colCount;
      GetRowsAndCols(type, rowCount, colCount);
      TypedefDecl *qts =
          LookupMatrixShorthandType(newScalarType, rowCount, colCount);
      return m_context->getTypeDeclType(qts);
    } else {
      DXASSERT_NOMSG(objKind == AR_TOBJ_BASIC || objKind == AR_TOBJ_ARRAY);
      return m_scalarTypes[newScalarType];
    }
  }
}

bool CheckIntersectionAttributeArg(Sema &S, Expr *E) {
  SourceLocation Loc = E->getExprLoc();
  QualType Ty = E->getType();

  // Identify problematic fields first (high diagnostic accuracy, may miss some
  // invalid cases)
  const TypeDiagContext DiagContext = TypeDiagContext::Attributes;
  if (DiagnoseTypeElements(S, Loc, Ty, DiagContext, DiagContext))
    return true;

  // Must be a UDT (low diagnostic accuracy, catches remaining invalid cases)
  if (Ty.isNull() || !hlsl::IsHLSLCopyableAnnotatableRecord(Ty)) {
    S.Diag(Loc, diag::err_payload_attrs_must_be_udt)
        << /*payload|attributes|callable*/ 1 << /*parameter %2|type*/ 1;
    return true;
  }

  return false;
}

Sema::TemplateDeductionResult
HLSLExternalSource::DeduceTemplateArgumentsForHLSL(
    FunctionTemplateDecl *FunctionTemplate,
    TemplateArgumentListInfo *ExplicitTemplateArgs, ArrayRef<Expr *> Args,
    FunctionDecl *&Specialization, TemplateDeductionInfo &Info) {
  DXASSERT_NOMSG(FunctionTemplate != nullptr);

  // Get information about the function we have.
  CXXMethodDecl *functionMethod =
      dyn_cast<CXXMethodDecl>(FunctionTemplate->getTemplatedDecl());
  if (!functionMethod) {
    // standalone function.
    return Sema::TemplateDeductionResult::TDK_Invalid;
  }
  CXXRecordDecl *functionParentRecord = functionMethod->getParent();
  DXASSERT(functionParentRecord != nullptr, "otherwise function is orphaned");
  QualType objectElement = GetFirstElementTypeFromDecl(functionParentRecord);

  // Preserve full object type for special cases in method matching
  QualType objectType = m_context->getTagDeclType(functionParentRecord);

  QualType functionTemplateTypeArg{};
  if (ExplicitTemplateArgs != nullptr && ExplicitTemplateArgs->size() == 1) {
    const TemplateArgument &firstTemplateArg =
        (*ExplicitTemplateArgs)[0].getArgument();
    if (firstTemplateArg.getKind() == TemplateArgument::ArgKind::Type)
      functionTemplateTypeArg = firstTemplateArg.getAsType();
  }

  // Handle subscript overloads.
  if (FunctionTemplate->getDeclName() ==
      m_context->DeclarationNames.getCXXOperatorName(OO_Subscript)) {
    DeclContext *functionTemplateContext = FunctionTemplate->getDeclContext();
    FindStructBasicTypeResult findResult =
        FindStructBasicType(functionTemplateContext);
    if (!findResult.Found()) {
      // This might be a nested type. Do a lookup on the parent.
      CXXRecordDecl *parentRecordType =
          dyn_cast_or_null<CXXRecordDecl>(functionTemplateContext);
      if (parentRecordType == nullptr ||
          parentRecordType->getDeclContext() == nullptr) {
        return Sema::TemplateDeductionResult::TDK_Invalid;
      }

      findResult = FindStructBasicType(parentRecordType->getDeclContext());
      if (!findResult.Found()) {
        return Sema::TemplateDeductionResult::TDK_Invalid;
      }

      DXASSERT(parentRecordType->getDeclContext()->getDeclKind() ==
                       Decl::Kind::CXXRecord ||
                   parentRecordType->getDeclContext()->getDeclKind() ==
                       Decl::Kind::ClassTemplateSpecialization,
               "otherwise FindStructBasicType should have failed - no other "
               "types are allowed");
      objectElement = GetFirstElementTypeFromDecl(
          cast<CXXRecordDecl>(parentRecordType->getDeclContext()));
    }

    Specialization =
        AddSubscriptSpecialization(FunctionTemplate, objectElement, findResult);
    DXASSERT_NOMSG(Specialization->getPrimaryTemplate()->getCanonicalDecl() ==
                   FunctionTemplate->getCanonicalDecl());

    return Sema::TemplateDeductionResult::TDK_Success;
  }

  // Reject overload lookups that aren't identifier-based.
  if (!FunctionTemplate->getDeclName().isIdentifier()) {
    return Sema::TemplateDeductionResult::TDK_NonDeducedMismatch;
  }

  // Find the table of intrinsics based on the object type.
  const HLSL_INTRINSIC *intrinsics = nullptr;
  size_t intrinsicCount = 0;
  const char *objectName = nullptr;
  FindIntrinsicTable(FunctionTemplate->getDeclContext(), &objectName,
                     &intrinsics, &intrinsicCount);
  // user-defined template object.
  if (objectName == nullptr && intrinsics == nullptr) {
    return Sema::TemplateDeductionResult::TDK_Invalid;
  }

  DXASSERT(objectName != nullptr &&
               (intrinsics != nullptr || m_intrinsicTables.size() > 0),
           "otherwise FindIntrinsicTable failed to lookup a valid object, "
           "or the parser let a user-defined template object through");

  // Look for an intrinsic for which we can match arguments.
  std::vector<QualType> argTypes;
  StringRef nameIdentifier = FunctionTemplate->getName();
  IntrinsicDefIter cursor = FindIntrinsicByNameAndArgCount(
      intrinsics, intrinsicCount, objectName, nameIdentifier, Args.size());
  IntrinsicDefIter end = IntrinsicDefIter::CreateEnd(
      intrinsics, intrinsicCount,
      IntrinsicTableDefIter::CreateEnd(m_intrinsicTables));

  while (cursor != end) {
    size_t badArgIdx;
    if (!MatchArguments(cursor, objectType, objectElement,
                        functionTemplateTypeArg, Args, &argTypes, badArgIdx)) {
      ++cursor;
      continue;
    }

    LPCSTR tableName = cursor.GetTableName();
    // Currently only intrinsic we allow for explicit template arguments are
    // for Load/Store for ByteAddressBuffer/RWByteAddressBuffer

    // Check Explicit template arguments
    UINT intrinsicOp = (*cursor)->Op;
    LPCSTR intrinsicName = (*cursor)->pArgs[0].pName;
    bool Is2018 = getSema()->getLangOpts().HLSLVersion >= hlsl::LangStd::v2018;
    bool IsBAB =
        objectName == g_ArBasicTypeNames[AR_OBJECT_BYTEADDRESS_BUFFER] ||
        objectName == g_ArBasicTypeNames[AR_OBJECT_RWBYTEADDRESS_BUFFER];
    bool IsBABLoad = false;
    bool IsBABStore = false;
    if (IsBuiltinTable(tableName) && IsBAB) {
      IsBABLoad = intrinsicOp == (UINT)IntrinsicOp::MOP_Load;
      IsBABStore = intrinsicOp == (UINT)IntrinsicOp::MOP_Store;
    }
    if (ExplicitTemplateArgs && ExplicitTemplateArgs->size() >= 1) {
      SourceLocation Loc = ExplicitTemplateArgs->getLAngleLoc();
      if (!IsBABLoad && !IsBABStore) {
        getSema()->Diag(Loc, diag::err_hlsl_intrinsic_template_arg_unsupported)
            << intrinsicName;
        return Sema::TemplateDeductionResult::TDK_Invalid;
      }
      Loc = (*ExplicitTemplateArgs)[0].getLocation();
      if (!Is2018) {
        getSema()->Diag(Loc,
                        diag::err_hlsl_intrinsic_template_arg_requires_2018)
            << intrinsicName;
        return Sema::TemplateDeductionResult::TDK_Invalid;
      }

      if (IsBABLoad || IsBABStore) {
        const bool IsNull = functionTemplateTypeArg.isNull();
        // Incomplete type is diagnosed elsewhere, so just fail if incomplete.
        if (!IsNull &&
            getSema()->RequireCompleteType(Loc, functionTemplateTypeArg, 0))
          return Sema::TemplateDeductionResult::TDK_Invalid;
        if (IsNull || !hlsl::IsHLSLNumericOrAggregateOfNumericType(
                          functionTemplateTypeArg)) {
          getSema()->Diag(Loc, diag::err_hlsl_intrinsic_template_arg_numeric)
              << intrinsicName;
          DiagnoseTypeElements(
              *getSema(), Loc, functionTemplateTypeArg,
              TypeDiagContext::TypeParameter /*ObjDiagContext*/,
              TypeDiagContext::Valid /*LongVecDiagContext*/);
          return Sema::TemplateDeductionResult::TDK_Invalid;
        }
      }
    } else if (IsBABStore) {
      // Prior to HLSL 2018, Store operation only stored scalar uint.
      if (!Is2018) {
        if (GetNumElements(argTypes[2]) != 1) {
          getSema()->Diag(Args[1]->getLocStart(),
                          diag::err_ovl_no_viable_member_function_in_call)
              << intrinsicName;
          return Sema::TemplateDeductionResult::TDK_Invalid;
        }
        argTypes[2] = getSema()->getASTContext().getIntTypeForBitwidth(
            32, /*signed*/ false);
      }
    }
    Specialization = AddHLSLIntrinsicMethod(
        tableName, cursor.GetLoweringStrategy(), *cursor, FunctionTemplate,
        Args, argTypes.data(), argTypes.size());
    DXASSERT_NOMSG(Specialization->getPrimaryTemplate()->getCanonicalDecl() ==
                   FunctionTemplate->getCanonicalDecl());

    const HLSL_INTRINSIC *pIntrinsic = *cursor;
    if (!IsValidObjectElement(tableName,
                              static_cast<IntrinsicOp>(pIntrinsic->Op),
                              objectElement)) {
      UINT numEles = GetNumElements(objectElement);
      std::string typeName(
          g_ArBasicTypeNames[GetTypeElementKind(objectElement)]);
      if (numEles > 1)
        typeName += std::to_string(numEles);
      m_sema->Diag(Args[0]->getExprLoc(),
                   diag::err_hlsl_invalid_resource_type_on_intrinsic)
          << nameIdentifier << typeName;
    }
    return Sema::TemplateDeductionResult::TDK_Success;
  }

  return Sema::TemplateDeductionResult::TDK_NonDeducedMismatch;
}

void HLSLExternalSource::ReportUnsupportedTypeNesting(SourceLocation loc,
                                                      QualType type) {
  m_sema->Diag(loc, diag::err_hlsl_unsupported_type_nesting) << type;
}

bool HLSLExternalSource::TryStaticCastForHLSL(
    ExprResult &SrcExpr, QualType DestType, Sema::CheckedConversionKind CCK,
    const SourceRange &OpRange, unsigned &msg, CastKind &Kind,
    CXXCastPath &BasePath, bool ListInitialization, bool SuppressWarnings,
    bool SuppressErrors, StandardConversionSequence *standard) {
  DXASSERT(!SrcExpr.isInvalid(),
           "caller should check for invalid expressions and placeholder types");
  bool explicitConversion =
      (CCK == Sema::CCK_CStyleCast || CCK == Sema::CCK_FunctionalCast);
  bool suppressWarnings = explicitConversion || SuppressWarnings;
  SourceLocation loc = OpRange.getBegin();
  if (ValidateCast(loc, SrcExpr.get(), DestType, explicitConversion,
                   suppressWarnings, SuppressErrors, standard)) {
    // TODO: LValue to RValue cast was all that CanConvert (ValidateCast) did
    // anyway, so do this here until we figure out why this is needed.
    if (standard && standard->First == ICK_Lvalue_To_Rvalue) {
      SrcExpr.set(CreateLValueToRValueCast(SrcExpr.get()));
    }
    return true;
  }

  // ValidateCast includes its own error messages.
  msg = 0;
  return false;
}

/// <summary>
/// Checks if a subscript index argument can be initialized from the given
/// expression.
/// </summary>
/// <param name="SrcExpr">Source expression used as argument.</param>
/// <param name="DestType">Parameter type to initialize.</param>
/// <remarks>
/// Rules for subscript index initialization follow regular implicit casting
/// rules, with the exception that no changes in arity are allowed (i.e., int2
/// can become uint2, but uint or uint3 cannot).
/// </remarks>
ImplicitConversionSequence
HLSLExternalSource::TrySubscriptIndexInitialization(clang::Expr *SrcExpr,
                                                    clang::QualType DestType) {
  DXASSERT_NOMSG(SrcExpr != nullptr);
  DXASSERT_NOMSG(!DestType.isNull());

  unsigned int msg = 0;
  CastKind kind;
  CXXCastPath path;
  ImplicitConversionSequence sequence;
  sequence.setStandard();
  ExprResult sourceExpr(SrcExpr);
  if (GetElementCount(SrcExpr->getType()) != GetElementCount(DestType)) {
    sequence.setBad(BadConversionSequence::FailureKind::no_conversion,
                    SrcExpr->getType(), DestType);
  } else if (!TryStaticCastForHLSL(sourceExpr, DestType,
                                   Sema::CCK_ImplicitConversion, NoRange, msg,
                                   kind, path, ListInitializationFalse,
                                   SuppressWarningsFalse, SuppressErrorsTrue,
                                   &sequence.Standard)) {
    sequence.setBad(BadConversionSequence::FailureKind::no_conversion,
                    SrcExpr->getType(), DestType);
  }

  return sequence;
}

template <typename T>
static bool IsValueInRange(T value, T minValue, T maxValue) {
  return minValue <= value && value <= maxValue;
}

#define D3DX_16F_MAX 6.550400e+004 // max value
#define D3DX_16F_MIN 6.1035156e-5f // min positive value

static void GetFloatLimits(ArBasicKind basicKind, double *minValue,
                           double *maxValue) {
  DXASSERT_NOMSG(minValue != nullptr);
  DXASSERT_NOMSG(maxValue != nullptr);

  switch (basicKind) {
  case AR_BASIC_MIN10FLOAT:
  case AR_BASIC_MIN16FLOAT:
  case AR_BASIC_FLOAT16:
    *minValue = -(D3DX_16F_MIN);
    *maxValue = D3DX_16F_MAX;
    return;
  case AR_BASIC_FLOAT32_PARTIAL_PRECISION:
  case AR_BASIC_FLOAT32:
    *minValue = -(FLT_MIN);
    *maxValue = FLT_MAX;
    return;
  case AR_BASIC_FLOAT64:
    *minValue = -(DBL_MIN);
    *maxValue = DBL_MAX;
    return;
  default:
    // No other float types.
    break;
  }

  DXASSERT(false, "unreachable");
  *minValue = 0;
  *maxValue = 0;
  return;
}

static void GetUnsignedLimit(ArBasicKind basicKind, uint64_t *maxValue) {
  DXASSERT_NOMSG(maxValue != nullptr);

  switch (basicKind) {
  case AR_BASIC_BOOL:
    *maxValue = 1;
    return;
  case AR_BASIC_UINT8:
    *maxValue = UINT8_MAX;
    return;
  case AR_BASIC_MIN16UINT:
  case AR_BASIC_UINT16:
    *maxValue = UINT16_MAX;
    return;
  case AR_BASIC_UINT32:
    *maxValue = UINT32_MAX;
    return;
  case AR_BASIC_UINT64:
    *maxValue = UINT64_MAX;
    return;
  case AR_BASIC_UINT8_4PACKED:
  case AR_BASIC_INT8_4PACKED:
    *maxValue = UINT32_MAX;
    return;
  default:
    // No other unsigned int types.
    break;
  }

  DXASSERT(false, "unreachable");
  *maxValue = 0;
  return;
}

static void GetSignedLimits(ArBasicKind basicKind, int64_t *minValue,
                            int64_t *maxValue) {
  DXASSERT_NOMSG(minValue != nullptr);
  DXASSERT_NOMSG(maxValue != nullptr);

  switch (basicKind) {
  case AR_BASIC_INT8:
    *minValue = INT8_MIN;
    *maxValue = INT8_MAX;
    return;
  case AR_BASIC_MIN12INT:
  case AR_BASIC_MIN16INT:
  case AR_BASIC_INT16:
    *minValue = INT16_MIN;
    *maxValue = INT16_MAX;
    return;
  case AR_BASIC_INT32:
    *minValue = INT32_MIN;
    *maxValue = INT32_MAX;
    return;
  case AR_BASIC_INT64:
    *minValue = INT64_MIN;
    *maxValue = INT64_MAX;
    return;
  default:
    // No other signed int types.
    break;
  }

  DXASSERT(false, "unreachable");
  *minValue = 0;
  *maxValue = 0;
  return;
}

static bool IsValueInBasicRange(ArBasicKind basicKind, const APValue &value) {
  if (IS_BASIC_FLOAT(basicKind)) {

    double val;
    if (value.isInt()) {
      val = value.getInt().getLimitedValue();
    } else if (value.isFloat()) {
      llvm::APFloat floatValue = value.getFloat();
      if (!floatValue.isFinite()) {
        return false;
      }
      llvm::APFloat valueFloat = value.getFloat();
      if (&valueFloat.getSemantics() == &llvm::APFloat::IEEEsingle) {
        val = value.getFloat().convertToFloat();
      } else {
        val = value.getFloat().convertToDouble();
      }
    } else {
      return false;
    }
    double minValue, maxValue;
    GetFloatLimits(basicKind, &minValue, &maxValue);
    return IsValueInRange(val, minValue, maxValue);
  } else if (IS_BASIC_SINT(basicKind)) {
    if (!value.isInt()) {
      return false;
    }
    int64_t val = value.getInt().getSExtValue();
    int64_t minValue, maxValue;
    GetSignedLimits(basicKind, &minValue, &maxValue);
    return IsValueInRange(val, minValue, maxValue);
  } else if (IS_BASIC_UINT(basicKind) || IS_BASIC_BOOL(basicKind)) {
    if (!value.isInt()) {
      return false;
    }
    uint64_t val = value.getInt().getLimitedValue();
    uint64_t maxValue;
    GetUnsignedLimit(basicKind, &maxValue);
    return IsValueInRange(val, (uint64_t)0, maxValue);
  } else {
    return false;
  }
}

static bool IsPrecisionLossIrrelevant(ASTContext &Ctx, const Expr *sourceExpr,
                                      QualType targetType,
                                      ArBasicKind targetKind) {
  DXASSERT_NOMSG(!targetType.isNull());
  DXASSERT_NOMSG(sourceExpr != nullptr);

  Expr::EvalResult evalResult;
  if (sourceExpr->EvaluateAsRValue(evalResult, Ctx)) {
    if (evalResult.Diag == nullptr || evalResult.Diag->empty()) {
      return IsValueInBasicRange(targetKind, evalResult.Val);
    }
  }

  return false;
}

bool HLSLExternalSource::ValidateCast(SourceLocation OpLoc, Expr *sourceExpr,
                                      QualType target, bool explicitConversion,
                                      bool suppressWarnings,
                                      bool suppressErrors,
                                      StandardConversionSequence *standard) {
  DXASSERT_NOMSG(sourceExpr != nullptr);

  if (OpLoc.isInvalid())
    OpLoc = sourceExpr->getExprLoc();

  QualType source = sourceExpr->getType();
  TYPE_CONVERSION_REMARKS remarks = TYPE_CONVERSION_NONE;

  if (!CanConvert(OpLoc, sourceExpr, target, explicitConversion, &remarks,
                  standard)) {

    //
    // Check whether the lack of explicit-ness matters.
    //
    // Setting explicitForDiagnostics to true in that case will avoid the
    // message saying anything about the implicit nature of the cast, when
    // adding the explicit cast won't make a difference.
    //
    bool explicitForDiagnostics = explicitConversion;
    if (explicitConversion == false) {
      if (!CanConvert(OpLoc, sourceExpr, target, true, &remarks, nullptr)) {
        // Can't convert either way - implicit/explicit doesn't matter.
        explicitForDiagnostics = true;
      }
    }

    if (!suppressErrors) {
      bool IsOutputParameter = false;
      if (clang::DeclRefExpr *OutFrom =
              dyn_cast<clang::DeclRefExpr>(sourceExpr)) {
        if (ParmVarDecl *Param = dyn_cast<ParmVarDecl>(OutFrom->getDecl())) {
          IsOutputParameter = Param->isModifierOut();
        }
      }

      m_sema->Diag(OpLoc, diag::err_hlsl_cannot_convert)
          << explicitForDiagnostics << IsOutputParameter << source << target;
    }
    return false;
  }

  if (!suppressWarnings) {
    if (!explicitConversion) {
      if ((remarks & TYPE_CONVERSION_PRECISION_LOSS) != 0) {
        // This is a much more restricted version of the analysis does
        // StandardConversionSequence::getNarrowingKind
        if (!IsPrecisionLossIrrelevant(*m_context, sourceExpr, target,
                                       GetTypeElementKind(target))) {
          m_sema->Diag(OpLoc, diag::warn_hlsl_narrowing) << source << target;
        }
      }

      if ((remarks & TYPE_CONVERSION_ELT_TRUNCATION) != 0) {
        m_sema->Diag(OpLoc, diag::warn_hlsl_implicit_vector_truncation);
      }
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////
// Functions exported from this translation unit.                             //

/// <summary>Performs HLSL-specific processing for unary operators.</summary>
QualType hlsl::CheckUnaryOpForHLSL(Sema &self, SourceLocation OpLoc,
                                   UnaryOperatorKind Opc, ExprResult &InputExpr,
                                   ExprValueKind &VK, ExprObjectKind &OK) {
  ExternalSemaSource *externalSource = self.getExternalSource();
  if (externalSource == nullptr) {
    return QualType();
  }

  HLSLExternalSource *hlsl =
      reinterpret_cast<HLSLExternalSource *>(externalSource);
  return hlsl->CheckUnaryOpForHLSL(OpLoc, Opc, InputExpr, VK, OK);
}

/// <summary>Performs HLSL-specific processing for binary operators.</summary>
void hlsl::CheckBinOpForHLSL(Sema &self, SourceLocation OpLoc,
                             BinaryOperatorKind Opc, ExprResult &LHS,
                             ExprResult &RHS, QualType &ResultTy,
                             QualType &CompLHSTy, QualType &CompResultTy) {
  ExternalSemaSource *externalSource = self.getExternalSource();
  if (externalSource == nullptr) {
    return;
  }

  HLSLExternalSource *hlsl =
      reinterpret_cast<HLSLExternalSource *>(externalSource);
  return hlsl->CheckBinOpForHLSL(OpLoc, Opc, LHS, RHS, ResultTy, CompLHSTy,
                                 CompResultTy);
}

/// <summary>Performs HLSL-specific processing of template
/// declarations.</summary>
bool hlsl::CheckTemplateArgumentListForHLSL(
    Sema &self, TemplateDecl *Template, SourceLocation TemplateLoc,
    TemplateArgumentListInfo &TemplateArgList) {
  DXASSERT_NOMSG(Template != nullptr);

  ExternalSemaSource *externalSource = self.getExternalSource();
  if (externalSource == nullptr) {
    return false;
  }

  HLSLExternalSource *hlsl =
      reinterpret_cast<HLSLExternalSource *>(externalSource);
  return hlsl->CheckTemplateArgumentListForHLSL(Template, TemplateLoc,
                                                TemplateArgList);
}

/// <summary>Deduces template arguments on a function call in an HLSL
/// program.</summary>
Sema::TemplateDeductionResult hlsl::DeduceTemplateArgumentsForHLSL(
    Sema *self, FunctionTemplateDecl *FunctionTemplate,
    TemplateArgumentListInfo *ExplicitTemplateArgs, ArrayRef<Expr *> Args,
    FunctionDecl *&Specialization, TemplateDeductionInfo &Info) {
  return HLSLExternalSource::FromSema(self)->DeduceTemplateArgumentsForHLSL(
      FunctionTemplate, ExplicitTemplateArgs, Args, Specialization, Info);
}

void hlsl::DiagnoseControlFlowConditionForHLSL(Sema *self, Expr *condExpr,
                                               StringRef StmtName) {
  while (ImplicitCastExpr *IC = dyn_cast<ImplicitCastExpr>(condExpr)) {
    if (IC->getCastKind() == CastKind::CK_HLSLMatrixTruncationCast ||
        IC->getCastKind() == CastKind::CK_HLSLVectorTruncationCast) {
      self->Diag(condExpr->getLocStart(),
                 diag::err_hlsl_control_flow_cond_not_scalar)
          << StmtName;
      return;
    }
    condExpr = IC->getSubExpr();
  }
}

static bool ShaderModelsMatch(const StringRef &left, const StringRef &right) {
  // TODO: handle shorthand cases.
  return left.size() == 0 || right.size() == 0 || left.equals(right);
}

void hlsl::DiagnosePackingOffset(clang::Sema *self, SourceLocation loc,
                                 clang::QualType type, int componentOffset) {
  DXASSERT_NOMSG(0 <= componentOffset && componentOffset <= 3);

  if (componentOffset > 0) {
    HLSLExternalSource *source = HLSLExternalSource::FromSema(self);
    ArBasicKind element = source->GetTypeElementKind(type);
    ArTypeObjectKind shape = source->GetTypeObjectKind(type);

    // Only perform some simple validation for now.
    if (IsObjectKindPrimitiveAggregate(shape) && IsBasicKindNumeric(element)) {
      int count = GetElementCount(type);
      if (count > (4 - componentOffset)) {
        self->Diag(loc, diag::err_hlsl_register_or_offset_bind_not_valid);
      }
    }
    if (hlsl::IsMatrixType(self, type) || type->isArrayType() ||
        type->isStructureType()) {
      self->Diag(loc, diag::err_hlsl_register_or_offset_bind_not_valid);
    }
  }
}

void hlsl::DiagnoseRegisterType(clang::Sema *self, clang::SourceLocation loc,
                                clang::QualType type, char registerType) {
  // Register type can be zero if only a register space was provided.
  if (registerType == 0)
    return;

  if (registerType >= 'A' && registerType <= 'Z')
    registerType = registerType + ('a' - 'A');

  HLSLExternalSource *source = HLSLExternalSource::FromSema(self);
  ArBasicKind element = source->GetTypeElementKind(type);
  StringRef expected("none");
  bool isValid = true;
  bool isWarning = false;
  switch (element) {
  case AR_BASIC_BOOL:
  case AR_BASIC_LITERAL_FLOAT:
  case AR_BASIC_FLOAT16:
  case AR_BASIC_FLOAT32_PARTIAL_PRECISION:
  case AR_BASIC_FLOAT32:
  case AR_BASIC_FLOAT64:
  case AR_BASIC_LITERAL_INT:
  case AR_BASIC_INT8:
  case AR_BASIC_UINT8:
  case AR_BASIC_INT16:
  case AR_BASIC_UINT16:
  case AR_BASIC_INT32:
  case AR_BASIC_UINT32:
  case AR_BASIC_INT64:
  case AR_BASIC_UINT64:

  case AR_BASIC_MIN10FLOAT:
  case AR_BASIC_MIN16FLOAT:
  case AR_BASIC_MIN12INT:
  case AR_BASIC_MIN16INT:
  case AR_BASIC_MIN16UINT:
    expected = "'b', 'c', or 'i'";
    isValid = registerType == 'b' || registerType == 'c' || registerType == 'i';
    break;

  case AR_OBJECT_TEXTURE1D:
  case AR_OBJECT_TEXTURE1D_ARRAY:
  case AR_OBJECT_TEXTURE2D:
  case AR_OBJECT_TEXTURE2D_ARRAY:
  case AR_OBJECT_TEXTURE3D:
  case AR_OBJECT_TEXTURECUBE:
  case AR_OBJECT_TEXTURECUBE_ARRAY:
  case AR_OBJECT_TEXTURE2DMS:
  case AR_OBJECT_TEXTURE2DMS_ARRAY:
    expected = "'t' or 's'";
    isValid = registerType == 't' || registerType == 's';
    break;

  case AR_OBJECT_SAMPLER:
  case AR_OBJECT_SAMPLER1D:
  case AR_OBJECT_SAMPLER2D:
  case AR_OBJECT_SAMPLER3D:
  case AR_OBJECT_SAMPLERCUBE:
  case AR_OBJECT_SAMPLERCOMPARISON:
    expected = "'s' or 't'";
    isValid = registerType == 's' || registerType == 't';
    break;

  case AR_OBJECT_BUFFER:
    expected = "'t'";
    isValid = registerType == 't';
    break;

  case AR_OBJECT_POINTSTREAM:
  case AR_OBJECT_LINESTREAM:
  case AR_OBJECT_TRIANGLESTREAM:
    isValid = false;
    isWarning = true;
    break;

  case AR_OBJECT_INPUTPATCH:
  case AR_OBJECT_OUTPUTPATCH:
    isValid = false;
    isWarning = true;
    break;

  case AR_OBJECT_RWTEXTURE1D:
  case AR_OBJECT_RWTEXTURE1D_ARRAY:
  case AR_OBJECT_RWTEXTURE2D:
  case AR_OBJECT_RWTEXTURE2D_ARRAY:
  case AR_OBJECT_RWTEXTURE3D:
  case AR_OBJECT_RWBUFFER:
    expected = "'u'";
    isValid = registerType == 'u';
    break;

  case AR_OBJECT_BYTEADDRESS_BUFFER:
  case AR_OBJECT_STRUCTURED_BUFFER:
    expected = "'t'";
    isValid = registerType == 't';
    break;

  case AR_OBJECT_CONSUME_STRUCTURED_BUFFER:
  case AR_OBJECT_RWBYTEADDRESS_BUFFER:
  case AR_OBJECT_RWSTRUCTURED_BUFFER:
  case AR_OBJECT_RWSTRUCTURED_BUFFER_ALLOC:
  case AR_OBJECT_RWSTRUCTURED_BUFFER_CONSUME:
  case AR_OBJECT_APPEND_STRUCTURED_BUFFER:
    expected = "'u'";
    isValid = registerType == 'u';
    break;

  case AR_OBJECT_CONSTANT_BUFFER:
    expected = "'b'";
    isValid = registerType == 'b';
    break;
  case AR_OBJECT_TEXTURE_BUFFER:
    expected = "'t'";
    isValid = registerType == 't';
    break;

  case AR_OBJECT_ROVBUFFER:
  case AR_OBJECT_ROVBYTEADDRESS_BUFFER:
  case AR_OBJECT_ROVSTRUCTURED_BUFFER:
  case AR_OBJECT_ROVTEXTURE1D:
  case AR_OBJECT_ROVTEXTURE1D_ARRAY:
  case AR_OBJECT_ROVTEXTURE2D:
  case AR_OBJECT_ROVTEXTURE2D_ARRAY:
  case AR_OBJECT_ROVTEXTURE3D:
  case AR_OBJECT_FEEDBACKTEXTURE2D:
  case AR_OBJECT_FEEDBACKTEXTURE2D_ARRAY:
    expected = "'u'";
    isValid = registerType == 'u';
    break;

  case AR_OBJECT_LEGACY_EFFECT: // Used for all unsupported but ignored legacy
                                // effect types
    isWarning = true;
    break; // So we don't care what you tried to bind it to
  default: // Other types have no associated registers.
    break;
  }

  // fxc is inconsistent as to when it reports an error and when it ignores
  // invalid bind semantics, so emit a warning instead.
  if (!isValid) {
    unsigned DiagID = isWarning ? diag::warn_hlsl_incorrect_bind_semantic
                                : diag::err_hlsl_incorrect_bind_semantic;
    self->Diag(loc, DiagID) << expected;
  }
}

// FIXME: DiagnoseSVForLaunchType is wrong in multiple ways:
// - It doesn't handle system values inside structs
// - It doesn't account for the fact that semantics are case-insensitive
// - It doesn't account for optional index at the end of semantic name
// - It permits any `SV_*` for Broadcasting launch, not just the legal ones
// - It doesn't prevent multiple system values with the same semantic
// - It doesn't check that the type is valid for the system value
// Produce diagnostics for any system values attached to `FD` function
// that are invalid for the `LaunchTy` launch type
static void DiagnoseSVForLaunchType(const FunctionDecl *FD,
                                    DXIL::NodeLaunchType LaunchTy,
                                    DiagnosticsEngine &Diags) {
  // Validate Compute Shader system value inputs per launch mode
  for (ParmVarDecl *param : FD->parameters()) {
    for (const hlsl::UnusualAnnotation *it : param->getUnusualAnnotations()) {
      if (it->getKind() == hlsl::UnusualAnnotation::UA_SemanticDecl) {
        const hlsl::SemanticDecl *sd = cast<hlsl::SemanticDecl>(it);
        // if the node launch type is Thread, then there are no system values
        // allowed
        if (LaunchTy == DXIL::NodeLaunchType::Thread) {
          if (sd->SemanticName.startswith("SV_")) {
            // emit diagnostic
            unsigned DiagID = Diags.getCustomDiagID(
                DiagnosticsEngine::Error,
                "Invalid system value semantic '%0' for launchtype '%1'");
            Diags.Report(param->getLocation(), DiagID)
                << sd->SemanticName << "Thread";
          }
        }

        // if the node launch type is Coalescing, then only
        // SV_GroupIndex and SV_GroupThreadID are allowed
        else if (LaunchTy == DXIL::NodeLaunchType::Coalescing) {
          if (!(sd->SemanticName.equals("SV_GroupIndex") ||
                sd->SemanticName.equals("SV_GroupThreadID"))) {
            // emit diagnostic
            unsigned DiagID = Diags.getCustomDiagID(
                DiagnosticsEngine::Error,
                "Invalid system value semantic '%0' for launchtype '%1'");
            Diags.Report(param->getLocation(), DiagID)
                << sd->SemanticName << "Coalescing";
          }
        }
        // Broadcasting nodes allow all node shader system value semantics
        else if (LaunchTy == DXIL::NodeLaunchType::Broadcasting) {
          continue;
        }
      }
    }
  }
}

/////////////////////////////////////////////////////////////////////////////
// Check HLSL intrinsic calls without call-graph context.

static bool CheckFinishedCrossGroupSharingCall(Sema &S, CXXMethodDecl *MD,
                                               SourceLocation Loc) {
  const CXXRecordDecl *NodeRecDecl = MD->getParent();
  // Node I/O records are templateTypes
  const ClassTemplateSpecializationDecl *templateDecl =
      cast<ClassTemplateSpecializationDecl>(NodeRecDecl);
  auto &TemplateArgs = templateDecl->getTemplateArgs();
  DXASSERT(TemplateArgs.size() == 1,
           "Input record types need to have one template argument");
  auto &Rec = TemplateArgs.get(0);
  clang::QualType RecType = Rec.getAsType();
  RecordDecl *RD = RecType->getAs<RecordType>()->getDecl();
  if (!RD->hasAttr<HLSLNodeTrackRWInputSharingAttr>()) {
    S.Diags.Report(Loc, diag::err_hlsl_wg_nodetrackrwinputsharing_missing);
    return true;
  }
  return false;
}

static bool CheckBarrierCall(Sema &S, FunctionDecl *FD, CallExpr *CE,
                             const hlsl::ShaderModel *SM) {
  DXASSERT(FD->getNumParams() == 2, "otherwise, unknown Barrier overload");

  // Emit error when MemoryTypeFlags are known to be invalid.
  QualType Param0Ty = FD->getParamDecl(0)->getType();
  if (Param0Ty ==
      HLSLExternalSource::FromSema(&S)->GetBasicKindType(AR_BASIC_UINT32)) {
    uint32_t MemoryTypeFlags = 0;
    Expr *MemoryTypeFlagsExpr = CE->getArg(0);
    llvm::APSInt MemoryTypeFlagsVal;
    if (MemoryTypeFlagsExpr->isIntegerConstantExpr(MemoryTypeFlagsVal,
                                                   S.Context)) {
      MemoryTypeFlags = MemoryTypeFlagsVal.getLimitedValue();
      if ((uint32_t)MemoryTypeFlags &
          ~(uint32_t)DXIL::MemoryTypeFlag::ValidMask) {
        S.Diags.Report(MemoryTypeFlagsExpr->getExprLoc(),
                       diag::err_hlsl_barrier_invalid_memory_flags)
            << (uint32_t)MemoryTypeFlags
            << (uint32_t)DXIL::MemoryTypeFlag::ValidMask;
        return true;
      }
    }
  }

  // Emit error when SemanticFlags are known to be invalid.
  uint32_t SemanticFlags = 0;
  Expr *SemanticFlagsExpr = CE->getArg(1);
  llvm::APSInt SemanticFlagsVal;
  if (SemanticFlagsExpr->isIntegerConstantExpr(SemanticFlagsVal, S.Context)) {
    SemanticFlags = SemanticFlagsVal.getLimitedValue();
    uint32_t ValidMask = 0U;
    if (SM->IsSM69Plus()) {
      ValidMask =
          static_cast<unsigned>(hlsl::DXIL::BarrierSemanticFlag::ValidMask);
    } else {
      ValidMask =
          static_cast<unsigned>(hlsl::DXIL::BarrierSemanticFlag::LegacyFlags);
    }
    if ((uint32_t)SemanticFlags & ~ValidMask) {
      S.Diags.Report(SemanticFlagsExpr->getExprLoc(),
                     diag::err_hlsl_barrier_invalid_semantic_flags)
          << SM->IsSM69Plus();
      return true;
    }
  }

  return false;
}

// MatVec Ops
static const unsigned kMatVecMulOutputVectorIdx = 0;
static const unsigned kMatVecMulOutputIsUnsignedIdx = 1;
static const unsigned kMatVecMulInputVectorIdx = 2;
static const unsigned kMatVecMulIsInputUnsignedIdx = 3;
static const unsigned kMatVecMulInputInterpretationIdx = 4;
// static const unsigned kMatVecMulMatrixBufferIdx = 5;
// static const unsigned kMatVecMulMatrixOffsetIdx = 6;
static const unsigned kMatVecMulMatrixInterpretationIdx = 7;
static const unsigned kMatVecMulMatrixMIdx = 8;
static const unsigned kMatVecMulMatrixKIdx = 9;
static const unsigned kMatVecMulMatrixLayoutIdx = 10;
static const unsigned kMatVecMulMatrixTransposeIdx = 11;
static const unsigned kMatVecMulMatrixStrideIdx = 12;

// MatVecAdd
const unsigned kMatVecMulAddBiasInterpretation = 15;

static bool IsValidMatrixLayoutForMulAndMulAddOps(unsigned Layout) {
  return Layout <=
         static_cast<unsigned>(DXIL::LinalgMatrixLayout::OuterProductOptimal);
}

static bool IsOptimalTypeMatrixLayout(unsigned Layout) {
  return (
      Layout == (static_cast<unsigned>(DXIL::LinalgMatrixLayout::MulOptimal)) ||
      (Layout ==
       (static_cast<unsigned>(DXIL::LinalgMatrixLayout::OuterProductOptimal))));
}

static bool IsValidTransposeForMatrixLayout(unsigned Layout, bool Transposed) {
  switch (static_cast<DXIL::LinalgMatrixLayout>(Layout)) {
  case DXIL::LinalgMatrixLayout::RowMajor:
  case DXIL::LinalgMatrixLayout::ColumnMajor:
    return !Transposed;

  default:
    return true;
  }
}

static bool IsPackedType(unsigned type) {
  return (type == static_cast<unsigned>(DXIL::ComponentType::PackedS8x32) ||
          type == static_cast<unsigned>(DXIL::ComponentType::PackedU8x32));
}

static bool IsValidLinalgTypeInterpretation(uint32_t Input, bool InRegister) {

  switch (static_cast<DXIL::ComponentType>(Input)) {
  case DXIL::ComponentType::I16:
  case DXIL::ComponentType::U16:
  case DXIL::ComponentType::I32:
  case DXIL::ComponentType::U32:
  case DXIL::ComponentType::F16:
  case DXIL::ComponentType::F32:
  case DXIL::ComponentType::U8:
  case DXIL::ComponentType::I8:
  case DXIL::ComponentType::F8_E4M3:
  case DXIL::ComponentType::F8_E5M2:
    return true;
  case DXIL::ComponentType::PackedS8x32:
  case DXIL::ComponentType::PackedU8x32:
    return InRegister;
  default:
    return false;
  }
}

static bool IsValidVectorAndMatrixDimensions(Sema &S, CallExpr *CE,
                                             unsigned InputVectorSize,
                                             unsigned OutputVectorSize,
                                             unsigned MatrixK, unsigned MatrixM,
                                             bool isInputPacked) {
  // Check if output vector size equals to matrix dimension M
  if (OutputVectorSize != MatrixM) {
    Expr *OutputVector = CE->getArg(kMatVecMulOutputVectorIdx);
    S.Diags.Report(
        OutputVector->getExprLoc(),
        diag::
            err_hlsl_linalg_mul_muladd_output_vector_size_not_equal_to_matrix_M);
    return false;
  }

  // Check if input vector size equals to matrix dimension K in the unpacked
  // case.
  // Check if input vector size equals the smallest number that can hold
  // matrix dimension K values
  const unsigned PackingFactor = isInputPacked ? 4 : 1;
  unsigned MinInputVectorSize = (MatrixK + PackingFactor - 1) / PackingFactor;
  if (InputVectorSize != MinInputVectorSize) {
    Expr *InputVector = CE->getArg(kMatVecMulInputVectorIdx);
    if (isInputPacked) {
      S.Diags.Report(
          InputVector->getExprLoc(),
          diag::err_hlsl_linalg_mul_muladd_packed_input_vector_size_incorrect);
      return false;
    } else {
      S.Diags.Report(
          InputVector->getExprLoc(),
          diag::
              err_hlsl_linalg_mul_muladd_unpacked_input_vector_size_not_equal_to_matrix_K);
      return false;
    }
  }

  return true;
}

static void CheckCommonMulAndMulAddParameters(Sema &S, CallExpr *CE,
                                              const hlsl::ShaderModel *SM) {
  // Check if IsOutputUnsigned is a const parameter
  bool IsOutputUnsignedFlagValue = false;
  Expr *IsOutputUnsignedExpr = CE->getArg(kMatVecMulOutputIsUnsignedIdx);
  llvm::APSInt IsOutputUnsignedExprVal;
  if (IsOutputUnsignedExpr->isIntegerConstantExpr(IsOutputUnsignedExprVal,
                                                  S.Context)) {
    IsOutputUnsignedFlagValue = IsOutputUnsignedExprVal.getBoolValue();
  } else {
    S.Diags.Report(IsOutputUnsignedExpr->getExprLoc(), diag::err_expr_not_ice)
        << 0;
    return;
  }

  Expr *OutputVectorExpr = CE->getArg(kMatVecMulOutputVectorIdx);
  unsigned OutputVectorSizeValue = 0;
  if (IsHLSLVecType(OutputVectorExpr->getType())) {
    OutputVectorSizeValue = GetHLSLVecSize(OutputVectorExpr->getType());
    QualType OutputVectorType =
        GetHLSLVecElementType(OutputVectorExpr->getType());
    const Type *OutputVectorTypePtr = OutputVectorType.getTypePtr();

    // Check if IsOutputUnsigned flag matches output vector type.
    // Must be true for unsigned int outputs, false for signed int/float
    // outputs.
    if (IsOutputUnsignedFlagValue &&
        !OutputVectorTypePtr->isUnsignedIntegerType()) {
      DXASSERT_NOMSG(OutputVectorTypePtr->isSignedIntegerType() ||
                     OutputVectorTypePtr->isFloatingType());
      S.Diags.Report(IsOutputUnsignedExpr->getExprLoc(),
                     diag::err_hlsl_linalg_isunsigned_incorrect_for_given_type)
          << "IsOuputUnsigned" << false
          << (OutputVectorTypePtr->isSignedIntegerType() ? 1 : 0);
      return;
    } else if (!IsOutputUnsignedFlagValue &&
               OutputVectorTypePtr->isUnsignedIntegerType()) {
      S.Diags.Report(IsOutputUnsignedExpr->getExprLoc(),
                     diag::err_hlsl_linalg_isunsigned_incorrect_for_given_type)
          << "IsOuputUnsigned" << true << 2;
      return;
    }
  }

  // Check if isInputUnsigned parameter is a constant
  bool IsInputUnsignedFlagValue = false;
  Expr *IsInputUnsignedExpr = CE->getArg(kMatVecMulIsInputUnsignedIdx);
  llvm::APSInt IsInputUnsignedExprVal;
  if (IsInputUnsignedExpr->isIntegerConstantExpr(IsInputUnsignedExprVal,
                                                 S.Context)) {
    IsInputUnsignedFlagValue = IsInputUnsignedExprVal.getBoolValue();
  } else {
    S.Diags.Report(IsInputUnsignedExpr->getExprLoc(), diag::err_expr_not_ice)
        << 0;
    return;
  }

  // Get InputInterpretation, check if it is constant
  Expr *InputInterpretationExpr = CE->getArg(kMatVecMulInputInterpretationIdx);
  llvm::APSInt InputInterpretationExprVal;
  unsigned InputInterpretationValue = 0;
  if (InputInterpretationExpr->isIntegerConstantExpr(InputInterpretationExprVal,
                                                     S.Context)) {
    InputInterpretationValue = InputInterpretationExprVal.getLimitedValue();
    const bool InRegisterInterpretation = true;
    if (!IsValidLinalgTypeInterpretation(InputInterpretationValue,
                                         InRegisterInterpretation)) {
      S.Diags.Report(InputInterpretationExpr->getExprLoc(),
                     diag::err_hlsl_linalg_interpretation_value_incorrect)
          << std::to_string(InputInterpretationValue)
          << InRegisterInterpretation;
      return;
    }
  } else {
    S.Diags.Report(InputInterpretationExpr->getExprLoc(),
                   diag::err_expr_not_ice)
        << 0;
    return;
  }

  bool IsInputVectorPacked = IsPackedType(InputInterpretationValue);

  // For packed types input vector type must be uint and isUnsigned must be
  // true. The signedness is determined from the InputInterpretation
  Expr *InputVectorExpr = CE->getArg(kMatVecMulInputVectorIdx);
  unsigned InputVectorSizeValue = 0;
  if (IsHLSLVecType(InputVectorExpr->getType())) {
    InputVectorSizeValue = GetHLSLVecSize(InputVectorExpr->getType());
    QualType InputVectorType =
        GetHLSLVecElementType(InputVectorExpr->getType());
    unsigned BitWidth = S.Context.getTypeSize(InputVectorType);
    bool Is32Bit = (BitWidth == 32);
    const Type *InputVectorTypePtr = InputVectorType.getTypePtr();

    // Check if the isUnsigned flag setting
    if (IsInputVectorPacked) {
      // Check that the input vector element type is "32bit"
      if (!Is32Bit) {
        S.Diags.Report(
            InputVectorExpr->getExprLoc(),
            diag::err_hlsl_linalg_mul_muladd_packed_input_vector_must_be_uint);
        return;
      }

      // Check that the input vector element type is an unsigned int
      if (!InputVectorTypePtr->isUnsignedIntegerType()) {
        S.Diags.Report(
            InputVectorExpr->getExprLoc(),
            diag::err_hlsl_linalg_mul_muladd_packed_input_vector_must_be_uint);
        return;
      }

      // Check that isInputUnsigned is always true
      // Actual signedness is inferred from the InputInterpretation
      if (!IsInputUnsignedFlagValue) {
        S.Diags.Report(
            IsInputUnsignedExpr->getExprLoc(),
            diag::
                err_hlsl_linalg_mul_muladd_isUnsigned_for_packed_input_must_be_true);
        return;
      }
    } else {
      if (IsInputUnsignedFlagValue &&
          !InputVectorTypePtr->isUnsignedIntegerType()) {
        DXASSERT_NOMSG(InputVectorTypePtr->isSignedIntegerType() ||
                       InputVectorTypePtr->isFloatingType());
        S.Diags.Report(
            IsInputUnsignedExpr->getExprLoc(),
            diag::err_hlsl_linalg_isunsigned_incorrect_for_given_type)
            << "IsInputUnsigned" << false
            << (InputVectorTypePtr->isSignedIntegerType() ? 1 : 0);
        return;
      } else if (!IsInputUnsignedFlagValue &&
                 InputVectorTypePtr->isUnsignedIntegerType()) {
        S.Diags.Report(
            IsInputUnsignedExpr->getExprLoc(),
            diag::err_hlsl_linalg_isunsigned_incorrect_for_given_type)
            << "IsInputUnsigned" << true << 2;
        return;
      }
    }
  }

  // Get Matrix Dimensions M and K, check if they are constants
  Expr *MatrixKExpr = CE->getArg(kMatVecMulMatrixKIdx);
  llvm::APSInt MatrixKExprVal;
  unsigned MatrixKValue = 0;
  if (MatrixKExpr->isIntegerConstantExpr(MatrixKExprVal, S.Context)) {
    MatrixKValue = MatrixKExprVal.getLimitedValue();
  } else {
    S.Diags.Report(MatrixKExpr->getExprLoc(), diag::err_expr_not_ice) << 0;
    return;
  }

  Expr *MatrixMExpr = CE->getArg(kMatVecMulMatrixMIdx);
  llvm::APSInt MatrixMExprVal;
  unsigned MatrixMValue = 0;
  if (MatrixMExpr->isIntegerConstantExpr(MatrixMExprVal, S.Context)) {
    MatrixMValue = MatrixMExprVal.getLimitedValue();
  } else {
    S.Diags.Report(MatrixMExpr->getExprLoc(), diag::err_expr_not_ice) << 0;
    return;
  }

  // Check MatrixM and MatrixK values are non-zero
  if (MatrixMValue == 0) {
    S.Diags.Report(MatrixMExpr->getExprLoc(),
                   diag::err_hlsl_linalg_matrix_dim_must_be_greater_than_zero)
        << std::to_string(DXIL::kSM69MaxVectorLength);
    return;
  }

  if (MatrixKValue == 0) {
    S.Diags.Report(MatrixKExpr->getExprLoc(),
                   diag::err_hlsl_linalg_matrix_dim_must_be_greater_than_zero)
        << std::to_string(DXIL::kSM69MaxVectorLength);
    return;
  }

  // Check MatrixM and MatrixK values are less than max
  // Matrix dimension cannot exceed largest vector length in a Mul/MulAdd
  // operation.
  if (MatrixMValue > DXIL::kSM69MaxVectorLength) {
    S.Diags.Report(MatrixMExpr->getExprLoc(),
                   diag::err_hlsl_linalg_mul_muladd_invalid_dim)
        << 0 << std::to_string(DXIL::kSM69MaxVectorLength);
    return;
  }

  // For packed input vectors 4 values are packed in a uint, so max Matrix K
  // can be 4096
  if (IsInputVectorPacked) {
    const unsigned PackingFactor =
        4; // Only supported packed formats: DATA_TYPE_(U)SINT8_T4_PACKED
    if (MatrixKValue > DXIL::kSM69MaxVectorLength * PackingFactor) {
      S.Diags.Report(MatrixKExpr->getExprLoc(),
                     diag::err_hlsl_linalg_mul_muladd_invalid_dim)
          << 2 << std::to_string(DXIL::kSM69MaxVectorLength * PackingFactor);
      return;
    }
  } else {
    if (MatrixKValue > DXIL::kSM69MaxVectorLength) {
      S.Diags.Report(MatrixKExpr->getExprLoc(),
                     diag::err_hlsl_linalg_mul_muladd_invalid_dim)
          << 1 << std::to_string(DXIL::kSM69MaxVectorLength);
      return;
    }
  }

  if (!IsValidVectorAndMatrixDimensions(S, CE, InputVectorSizeValue,
                                        OutputVectorSizeValue, MatrixKValue,
                                        MatrixMValue, IsInputVectorPacked)) {
    return;
  }

  // Get MatrixInterpretation, check if it is constant
  // Make sure it is a valid value
  Expr *MatrixInterpretationExpr =
      CE->getArg(kMatVecMulMatrixInterpretationIdx);
  llvm::APSInt MatrixInterpretationExprVal;
  unsigned MatrixInterpretationValue = 0;
  if (MatrixInterpretationExpr->isIntegerConstantExpr(
          MatrixInterpretationExprVal, S.Context)) {
    MatrixInterpretationValue = MatrixInterpretationExprVal.getLimitedValue();
    const bool InRegisterInterpretation = false;
    if (!IsValidLinalgTypeInterpretation(MatrixInterpretationValue,
                                         InRegisterInterpretation)) {
      S.Diags.Report(MatrixInterpretationExpr->getExprLoc(),
                     diag::err_hlsl_linalg_interpretation_value_incorrect)
          << std::to_string(MatrixInterpretationValue)
          << InRegisterInterpretation;
      return;
    }
  } else {
    S.Diags.Report(MatrixInterpretationExpr->getExprLoc(),
                   diag::err_expr_not_ice)
        << 0;
    return;
  }

  // Get MatrixLayout, check if it is constant and valid value
  Expr *MatrixLayoutExpr = CE->getArg(kMatVecMulMatrixLayoutIdx);
  llvm::APSInt MatrixLayoutExprVal;
  unsigned MatrixLayoutValue = 0;
  if (MatrixLayoutExpr->isIntegerConstantExpr(MatrixLayoutExprVal, S.Context)) {
    MatrixLayoutValue = MatrixLayoutExprVal.getLimitedValue();
    if (!IsValidMatrixLayoutForMulAndMulAddOps(MatrixLayoutValue)) {
      S.Diags.Report(MatrixLayoutExpr->getExprLoc(),
                     diag::err_hlsl_linalg_matrix_layout_invalid)
          << std::to_string(MatrixLayoutValue)
          << std::to_string(
                 static_cast<unsigned>(DXIL::LinalgMatrixLayout::RowMajor))
          << std::to_string(static_cast<unsigned>(
                 DXIL::LinalgMatrixLayout::OuterProductOptimal));
      return;
    }
  } else {
    S.Diags.Report(MatrixLayoutExpr->getExprLoc(), diag::err_expr_not_ice) << 0;
    return;
  }

  // Get MatrixTranspose, check if it is constant
  Expr *MatrixTransposeExpr = CE->getArg(kMatVecMulMatrixTransposeIdx);
  llvm::APSInt MatrixTransposeExprVal;
  unsigned MatrixTransposeValue = 0;
  if (MatrixTransposeExpr->isIntegerConstantExpr(MatrixTransposeExprVal,
                                                 S.Context)) {
    MatrixTransposeValue = MatrixTransposeExprVal.getBoolValue();
    if (!IsValidTransposeForMatrixLayout(MatrixLayoutValue,
                                         MatrixTransposeValue)) {

      S.Diags.Report(MatrixTransposeExpr->getExprLoc(),
                     diag::err_hlsl_linalg_matrix_layout_is_not_transposable);
      return;
    }
  } else {
    S.Diags.Report(MatrixTransposeExpr->getExprLoc(), diag::err_expr_not_ice)
        << 0;
    return;
  }

  // Get MatrixStride, check if it is constant, if yes it should be zero
  // for optimal layouts
  Expr *MatrixStrideExpr = CE->getArg(kMatVecMulMatrixStrideIdx);
  llvm::APSInt MatrixStrideExprVal;
  unsigned MatrixStrideValue = 0;
  if (MatrixStrideExpr->isIntegerConstantExpr(MatrixStrideExprVal, S.Context)) {
    MatrixStrideValue = MatrixStrideExprVal.getLimitedValue();
    if (IsOptimalTypeMatrixLayout(MatrixLayoutValue) &&
        MatrixStrideValue != 0) {
      S.Diags.Report(
          MatrixStrideExpr->getExprLoc(),
          diag::
              err_hlsl_linalg_optimal_matrix_layout_matrix_stride_must_be_zero);
      return;
    }
  }
}

static void CheckMulCall(Sema &S, FunctionDecl *FD, CallExpr *CE,
                         const hlsl::ShaderModel *SM) {
  CheckCommonMulAndMulAddParameters(S, CE, SM);
}

static void CheckMulAddCall(Sema &S, FunctionDecl *FD, CallExpr *CE,
                            const hlsl::ShaderModel *SM) {
  CheckCommonMulAndMulAddParameters(S, CE, SM);

  // Check if BiasInterpretation is constant and a valid value
  Expr *BiasInterpretationExpr = CE->getArg(kMatVecMulAddBiasInterpretation);
  llvm::APSInt BiasInterpretationExprVal;
  unsigned BiasInterpretationValue = 0;
  if (BiasInterpretationExpr->isIntegerConstantExpr(BiasInterpretationExprVal,
                                                    S.Context)) {
    BiasInterpretationValue = BiasInterpretationExprVal.getLimitedValue();
    const bool InRegisterInterpretation = false;
    if (!IsValidLinalgTypeInterpretation(BiasInterpretationValue,
                                         InRegisterInterpretation)) {
      S.Diags.Report(BiasInterpretationExpr->getExprLoc(),
                     diag::err_hlsl_linalg_interpretation_value_incorrect)
          << std::to_string(BiasInterpretationValue)
          << InRegisterInterpretation;
      return;
    }
  } else {
    S.Diags.Report(BiasInterpretationExpr->getExprLoc(), diag::err_expr_not_ice)
        << 0;
    return;
  }
}

// Linalg Outer Product Accumulate
// OuterProductAccumulate builtin function parameters
static const unsigned kOuterProdAccInputVector1Idx = 0;
static const unsigned kOuterProdAccInputVector2Idx = 1;
// static const unsigned kOuterProdAccMatrixBufferIdx = 2;
// static const unsigned kOuterProdAccMatrixOffsetIdx = 3;
static const unsigned kOuterProdAccMatrixInterpretationIdx = 4;
static const unsigned kOuterProdAccMatrixLayoutIdx = 5;
static const unsigned kOuterProdAccMatrixStrideIdx = 6;

static void CheckOuterProductAccumulateCall(Sema &S, FunctionDecl *FD,
                                            CallExpr *CE) {
  // Check InputVector1 and InputVector2 are the same type
  const Expr *InputVector1Expr = CE->getArg(kOuterProdAccInputVector1Idx);
  const Expr *InputVector2Expr = CE->getArg(kOuterProdAccInputVector2Idx);
  QualType InputVector1Type = InputVector1Expr->getType();
  QualType InputVector2Type = InputVector2Expr->getType();

  // Get the element types of the vectors
  const QualType InputVector1ElementType =
      GetHLSLVecElementType(InputVector1Type);
  const QualType InputVector2ElementType =
      GetHLSLVecElementType(InputVector2Type);

  if (!S.Context.hasSameType(InputVector1ElementType,
                             InputVector2ElementType)) {
    S.Diags.Report(InputVector2Expr->getExprLoc(),
                   diag::err_hlsl_linalg_outer_prod_acc_vector_type_mismatch);
    return;
  }

  // Check Matrix Interpretation is a constant and a valid value
  Expr *MatrixInterpretationExpr =
      CE->getArg(kOuterProdAccMatrixInterpretationIdx);
  llvm::APSInt MatrixInterpretationExprVal;
  unsigned MatrixInterpretationValue = 0;
  if (MatrixInterpretationExpr->isIntegerConstantExpr(
          MatrixInterpretationExprVal, S.Context)) {
    MatrixInterpretationValue = MatrixInterpretationExprVal.getLimitedValue();
    const bool InRegisterInterpretation = false;
    if (!IsValidLinalgTypeInterpretation(MatrixInterpretationValue,
                                         InRegisterInterpretation)) {
      S.Diags.Report(MatrixInterpretationExpr->getExprLoc(),
                     diag::err_hlsl_linalg_interpretation_value_incorrect)
          << std::to_string(MatrixInterpretationValue)
          << InRegisterInterpretation;
      return;
    }
  } else {
    S.Diags.Report(MatrixInterpretationExpr->getExprLoc(),
                   diag::err_expr_not_ice)
        << 0;
    return;
  }

  // Check Matrix Layout must be a constant and Training Optimal
  Expr *MatrixLayoutExpr = CE->getArg(kOuterProdAccMatrixLayoutIdx);
  llvm::APSInt MatrixLayoutExprVal;
  unsigned MatrixLayoutValue = 0;
  if (MatrixLayoutExpr->isIntegerConstantExpr(MatrixLayoutExprVal, S.Context)) {
    MatrixLayoutValue = MatrixLayoutExprVal.getLimitedValue();
    if (MatrixLayoutValue !=
        static_cast<unsigned>(DXIL::LinalgMatrixLayout::OuterProductOptimal)) {
      S.Diags.Report(
          MatrixLayoutExpr->getExprLoc(),
          diag::
              err_hlsl_linalg_outer_prod_acc_matrix_layout_must_be_outer_prod_acc_optimal)
          << std::to_string(static_cast<unsigned>(
                 DXIL::LinalgMatrixLayout::OuterProductOptimal));
      return;
    }
  } else {
    S.Diags.Report(MatrixLayoutExpr->getExprLoc(), diag::err_expr_not_ice) << 0;
    return;
  }

  // Matrix Stride must be zero (Training Optimal matrix layout)
  Expr *MatrixStrideExpr = CE->getArg(kOuterProdAccMatrixStrideIdx);
  llvm::APSInt MatrixStrideExprVal;
  unsigned MatrixStrideValue = 0;
  if (MatrixStrideExpr->isIntegerConstantExpr(MatrixStrideExprVal, S.Context)) {
    MatrixStrideValue = MatrixStrideExprVal.getLimitedValue();
    if (MatrixStrideValue != 0) {
      S.Diags.Report(
          MatrixStrideExpr->getExprLoc(),
          diag::
              err_hlsl_linalg_optimal_matrix_layout_matrix_stride_must_be_zero);
      return;
    }
  }
}

#ifdef ENABLE_SPIRV_CODEGEN
static bool CheckVKBufferPointerCast(Sema &S, FunctionDecl *FD, CallExpr *CE,
                                     bool isStatic) {
  const Expr *argExpr = CE->getArg(0);
  QualType srcType = argExpr->getType();
  QualType destType = CE->getType();
  QualType srcTypeArg = hlsl::GetVKBufferPointerBufferType(srcType);
  QualType destTypeArg = hlsl::GetVKBufferPointerBufferType(destType);

  if (isStatic && srcTypeArg != destTypeArg &&
      !S.IsDerivedFrom(srcTypeArg, destTypeArg)) {
    S.Diags.Report(CE->getExprLoc(),
                   diag::err_hlsl_vk_static_pointer_cast_type);
    return true;
  }

  if (hlsl::GetVKBufferPointerAlignment(destType) >
      hlsl::GetVKBufferPointerAlignment(srcType)) {
    S.Diags.Report(CE->getExprLoc(), diag::err_hlsl_vk_pointer_cast_alignment);
    return true;
  }

  return false;
}
#endif

static bool isRelatedDeclMarkedNointerpolation(Expr *E) {
  if (!E)
    return false;
  E = E->IgnoreCasts();
  if (auto *DRE = dyn_cast<DeclRefExpr>(E))
    return DRE->getDecl()->hasAttr<HLSLNoInterpolationAttr>();

  if (auto *ME = dyn_cast<MemberExpr>(E))
    return ME->getMemberDecl()->hasAttr<HLSLNoInterpolationAttr>() ||
           isRelatedDeclMarkedNointerpolation(ME->getBase());

  if (auto *HVE = dyn_cast<HLSLVectorElementExpr>(E))
    return isRelatedDeclMarkedNointerpolation(HVE->getBase());

  if (auto *ASE = dyn_cast<ArraySubscriptExpr>(E))
    return isRelatedDeclMarkedNointerpolation(ASE->getBase());

  return false;
}

static bool CheckIntrinsicGetAttributeAtVertex(Sema &S, FunctionDecl *FDecl,
                                               CallExpr *TheCall) {
  assert(TheCall->getNumArgs() > 0);
  auto argument = TheCall->getArg(0)->IgnoreCasts();

  if (!isRelatedDeclMarkedNointerpolation(argument)) {
    S.Diag(argument->getExprLoc(), diag::err_hlsl_parameter_requires_attribute)
        << 0 << FDecl->getName() << "nointerpolation";
    return true;
  }

  return false;
}

static bool CheckNoInterpolationParams(Sema &S, FunctionDecl *FDecl,
                                       CallExpr *TheCall) {
  // See #hlsl-specs/issues/181. Feature is broken. For SPIR-V we want
  // to limit the scope, and fail gracefully in some cases.
  if (!S.getLangOpts().SPIRV)
    return false;

  bool error = false;
  for (unsigned i = 0; i < FDecl->getNumParams(); i++) {
    assert(i < TheCall->getNumArgs());

    if (!FDecl->getParamDecl(i)->hasAttr<HLSLNoInterpolationAttr>())
      continue;

    if (!isRelatedDeclMarkedNointerpolation(TheCall->getArg(i))) {
      S.Diag(TheCall->getArg(i)->getExprLoc(),
             diag::err_hlsl_parameter_requires_attribute)
          << i << FDecl->getName() << "nointerpolation";
      error = true;
    }
  }

  return error;
}

// Verify that user-defined intrinsic struct args contain no long vectors
static bool CheckUDTIntrinsicArg(Sema &S, Expr *Arg) {
  const TypeDiagContext DiagContext =
      TypeDiagContext::UserDefinedStructParameter;
  return DiagnoseTypeElements(S, Arg->getExprLoc(), Arg->getType(), DiagContext,
                              DiagContext);
}

// Check HLSL call constraints, not fatal to creating the AST.
void Sema::CheckHLSLFunctionCall(FunctionDecl *FDecl, CallExpr *TheCall) {
  if (CheckNoInterpolationParams(*this, FDecl, TheCall))
    return;

  HLSLIntrinsicAttr *IntrinsicAttr = FDecl->getAttr<HLSLIntrinsicAttr>();
  if (!IntrinsicAttr)
    return;
  if (!IsBuiltinTable(IntrinsicAttr->getGroup()))
    return;

  const auto *SM =
      hlsl::ShaderModel::GetByName(getLangOpts().HLSLProfile.c_str());

  hlsl::IntrinsicOp opCode = (hlsl::IntrinsicOp)IntrinsicAttr->getOpcode();
  switch (opCode) {
  case hlsl::IntrinsicOp::MOP_FinishedCrossGroupSharing:
    CheckFinishedCrossGroupSharingCall(*this, cast<CXXMethodDecl>(FDecl),
                                       TheCall->getLocStart());
    break;
  case hlsl::IntrinsicOp::IOP_Barrier:
    CheckBarrierCall(*this, FDecl, TheCall, SM);
    break;
  case hlsl::IntrinsicOp::IOP___builtin_MatVecMul:
    CheckMulCall(*this, FDecl, TheCall, SM);
    break;
  case hlsl::IntrinsicOp::IOP___builtin_MatVecMulAdd:
    CheckMulAddCall(*this, FDecl, TheCall, SM);
    break;
  case hlsl::IntrinsicOp::IOP___builtin_OuterProductAccumulate:
    CheckOuterProductAccumulateCall(*this, FDecl, TheCall);
    break;
  case hlsl::IntrinsicOp::IOP_GetAttributeAtVertex:
    // See #hlsl-specs/issues/181. Feature is broken. For SPIR-V we want
    // to limit the scope, and fail gracefully in some cases.
    if (!getLangOpts().SPIRV)
      return;
    CheckIntrinsicGetAttributeAtVertex(*this, FDecl, TheCall);
    break;
  case hlsl::IntrinsicOp::IOP_DispatchMesh:
    CheckUDTIntrinsicArg(*this, TheCall->getArg(3)->IgnoreCasts());
    break;
  case hlsl::IntrinsicOp::IOP_CallShader:
    CheckUDTIntrinsicArg(*this, TheCall->getArg(1)->IgnoreCasts());
    break;
  case hlsl::IntrinsicOp::IOP_TraceRay:
    CheckUDTIntrinsicArg(*this, TheCall->getArg(7)->IgnoreCasts());
    break;
  case hlsl::IntrinsicOp::IOP_ReportHit:
    CheckIntersectionAttributeArg(*this, TheCall->getArg(2)->IgnoreCasts());
    break;
  case hlsl::IntrinsicOp::MOP_DxHitObject_GetAttributes:
    CheckIntersectionAttributeArg(*this, TheCall->getArg(0)->IgnoreCasts());
    break;
#ifdef ENABLE_SPIRV_CODEGEN
  case hlsl::IntrinsicOp::IOP_Vkreinterpret_pointer_cast:
    CheckVKBufferPointerCast(*this, FDecl, TheCall, false);
    break;
  case hlsl::IntrinsicOp::IOP_Vkstatic_pointer_cast:
    CheckVKBufferPointerCast(*this, FDecl, TheCall, true);
    break;
#endif
  default:
    break;
  }
}

/////////////////////////////////////////////////////////////////////////////
// Check HLSL intrinsic calls reachable from entry/export functions.

static void DiagnoseNumThreadsForDerivativeOp(const HLSLNumThreadsAttr *Attr,
                                              SourceLocation LocDeriv,
                                              FunctionDecl *FD,
                                              const FunctionDecl *EntryDecl,
                                              DiagnosticsEngine &Diags) {
  bool invalidNumThreads = false;
  if (Attr->getY() != 1) {
    // 2D mode requires x and y to be multiple of 2.
    invalidNumThreads = !((Attr->getX() % 2) == 0 && (Attr->getY() % 2) == 0);
  } else {
    // 1D mode requires x to be multiple of 4 and y and z to be 1.
    invalidNumThreads = (Attr->getX() % 4) != 0 || (Attr->getZ() != 1);
  }
  if (invalidNumThreads) {
    Diags.Report(LocDeriv, diag::warn_hlsl_derivatives_wrong_numthreads)
        << FD->getNameAsString() << EntryDecl->getNameAsString();
    Diags.Report(EntryDecl->getLocation(), diag::note_hlsl_entry_defined_here);
  }
}

static void DiagnoseDerivativeOp(Sema &S, FunctionDecl *FD, SourceLocation Loc,
                                 const hlsl::ShaderModel *SM,
                                 DXIL::ShaderKind EntrySK,
                                 DXIL::NodeLaunchType NodeLaunchTy,
                                 const FunctionDecl *EntryDecl,
                                 DiagnosticsEngine &Diags) {
  switch (EntrySK) {
  default: {
    if (!SM->AllowDerivatives(EntrySK)) {
      Diags.Report(Loc, diag::warn_hlsl_derivatives_in_wrong_shader_kind)
          << FD->getNameAsString() << EntryDecl->getNameAsString();
      Diags.Report(EntryDecl->getLocation(),
                   diag::note_hlsl_entry_defined_here);
    }
  } break;
  case DXIL::ShaderKind::Compute:
  case DXIL::ShaderKind::Amplification:
  case DXIL::ShaderKind::Mesh: {
    if (!SM->IsSM66Plus()) {
      Diags.Report(Loc, diag::warn_hlsl_derivatives_in_wrong_shader_model)
          << FD->getNameAsString() << EntryDecl->getNameAsString();
      Diags.Report(EntryDecl->getLocation(),
                   diag::note_hlsl_entry_defined_here);
    }
  } break;
  case DXIL::ShaderKind::Node: {
    if (NodeLaunchTy != DXIL::NodeLaunchType::Broadcasting) {
      Diags.Report(Loc, diag::warn_hlsl_derivatives_in_wrong_shader_kind)
          << FD->getNameAsString() << EntryDecl->getNameAsString();
      Diags.Report(EntryDecl->getLocation(),
                   diag::note_hlsl_entry_defined_here);
    }
  } break;
  }

  if (const HLSLNumThreadsAttr *Attr =
          EntryDecl->getAttr<HLSLNumThreadsAttr>()) {
    DiagnoseNumThreadsForDerivativeOp(Attr, Loc, FD, EntryDecl, Diags);
  }
}

static void DiagnoseCalculateLOD(Sema &S, FunctionDecl *FD, SourceLocation Loc,
                                 const hlsl::ShaderModel *SM,
                                 DXIL::ShaderKind EntrySK,
                                 DXIL::NodeLaunchType NodeLaunchTy,
                                 const FunctionDecl *EntryDecl,
                                 DiagnosticsEngine &Diags,
                                 bool locallyVisited) {
  if (FD->getParamDecl(0)->getType() !=
      HLSLExternalSource::FromSema(&S)->GetBasicKindType(
          AR_OBJECT_SAMPLERCOMPARISON))
    return;

  if (!locallyVisited && !SM->IsSM68Plus()) {
    Diags.Report(Loc, diag::warn_hlsl_intrinsic_overload_in_wrong_shader_model)
        << FD->getNameAsString() + " with SamplerComparisonState"
        << "6.8";
    return;
  }

  DiagnoseDerivativeOp(S, FD, Loc, SM, EntrySK, NodeLaunchTy, EntryDecl, Diags);
}

static uint32_t
DiagnoseMemoryFlags(SourceLocation ArgLoc, uint32_t MemoryTypeFlags,
                    bool hasVisibleGroup, DXIL::ShaderKind EntrySK,
                    const FunctionDecl *EntryDecl, DiagnosticsEngine &Diags) {
  // Check flags against context.
  // If DXIL::MemoryTypeFlag::AllMemory, filter flags for context, otherwise,
  // emit errors for invalid flags.
  uint32_t MemoryTypeFiltered = MemoryTypeFlags;

  // If group memory specified, must have a visible group.
  if (!hasVisibleGroup) {
    if ((uint32_t)MemoryTypeFlags &
        (uint32_t)DXIL::MemoryTypeFlag::GroupFlags) {
      if (MemoryTypeFlags == (uint32_t)DXIL::MemoryTypeFlag::AllMemory) {
        // If AllMemory, filter out group flags.
        MemoryTypeFiltered &= ~(uint32_t)DXIL::MemoryTypeFlag::GroupFlags;
      } else {
        Diags.Report(ArgLoc,
                     diag::warn_hlsl_barrier_group_memory_requires_group);
        Diags.Report(EntryDecl->getLocation(),
                     diag::note_hlsl_entry_defined_here);
      }
    }
  }

  // If node memory specified, must be a node shader.
  if (EntrySK != DXIL::ShaderKind::Node &&
      EntrySK != DXIL::ShaderKind::Library &&
      ((uint32_t)MemoryTypeFlags & (uint32_t)DXIL::MemoryTypeFlag::NodeFlags)) {
    if (MemoryTypeFlags == (uint32_t)DXIL::MemoryTypeFlag::AllMemory) {
      // If AllMemory, filter out node flags.
      MemoryTypeFiltered &= ~(uint32_t)DXIL::MemoryTypeFlag::NodeFlags;
    } else {
      Diags.Report(ArgLoc, diag::warn_hlsl_barrier_node_memory_requires_node);
      Diags.Report(EntryDecl->getLocation(),
                   diag::note_hlsl_entry_defined_here);
    }
  }

  // Return filtered flags.
  return MemoryTypeFiltered;
}

static void DiagnoseSemanticFlags(SourceLocation ArgLoc, uint32_t SemanticFlags,
                                  bool hasVisibleGroup,
                                  bool memAtLeastGroupScope,
                                  bool memAtLeastDeviceScope,
                                  const FunctionDecl *EntryDecl,
                                  DiagnosticsEngine &Diags) {
  // If hasVisibleGroup is false, emit error for group flags.
  if (!hasVisibleGroup) {
    if ((uint32_t)SemanticFlags &
        (uint32_t)DXIL::BarrierSemanticFlag::GroupFlags) {
      Diags.Report(ArgLoc,
                   diag::warn_hlsl_barrier_group_semantic_requires_group);
      Diags.Report(EntryDecl->getLocation(),
                   diag::note_hlsl_entry_defined_here);
    }
  }

  // Error on DeviceScope or GroupScope when memory lacks this scope.
  if (!memAtLeastDeviceScope &&
      ((uint32_t)SemanticFlags &
       (uint32_t)DXIL::BarrierSemanticFlag::DeviceScope)) {
    Diags.Report(ArgLoc,
                 diag::warn_hlsl_barrier_no_mem_with_required_device_scope);
    Diags.Report(EntryDecl->getLocation(), diag::note_hlsl_entry_defined_here);
  }
  if (!memAtLeastGroupScope &&
      ((uint32_t)SemanticFlags &
       (uint32_t)DXIL::BarrierSemanticFlag::GroupScope)) {
    Diags.Report(ArgLoc,
                 diag::warn_hlsl_barrier_no_mem_with_required_group_scope);
    Diags.Report(EntryDecl->getLocation(), diag::note_hlsl_entry_defined_here);
  }
}

static void DiagnoseReachableBarrier(Sema &S, CallExpr *CE,
                                     const hlsl::ShaderModel *SM,
                                     DXIL::ShaderKind EntrySK,
                                     DXIL::NodeLaunchType NodeLaunchTy,
                                     const FunctionDecl *EntryDecl,
                                     DiagnosticsEngine &Diags) {
  FunctionDecl *FD = CE->getDirectCallee();
  DXASSERT(FD->getNumParams() == 2, "otherwise, unknown Barrier overload");

  // First, check shader model constraint.
  if (!SM->IsSM68Plus()) {
    Diags.Report(CE->getExprLoc(),
                 diag::warn_hlsl_intrinsic_in_wrong_shader_model)
        << FD->getNameAsString() << EntryDecl->getNameAsString() << "6.8";
    Diags.Report(EntryDecl->getLocation(), diag::note_hlsl_entry_defined_here);
    return;
  }

  // Does shader have visible group?
  // Allow exported library functions as well.
  bool hasVisibleGroup = ShaderModel::HasVisibleGroup(EntrySK, NodeLaunchTy);
  QualType Param0Ty = FD->getParamDecl(0)->getType();

  // Used when checking scope flags
  // Default to true to avoid over-strict diagnostics
  bool memAtLeastGroupScope = true;
  bool memAtLeastDeviceScope = true;

  if (Param0Ty ==
      HLSLExternalSource::FromSema(&S)->GetBasicKindType(AR_BASIC_UINT32)) {
    // overload: Barrier(uint MemoryTypeFlags, uint SemanticFlags)
    uint32_t MemoryTypeFlags = 0;
    Expr *MemoryTypeFlagsExpr = CE->getArg(0);
    llvm::APSInt MemoryTypeFlagsVal;
    if (MemoryTypeFlagsExpr->isIntegerConstantExpr(MemoryTypeFlagsVal,
                                                   S.Context)) {
      MemoryTypeFlags = MemoryTypeFlagsVal.getLimitedValue();
      MemoryTypeFlags = DiagnoseMemoryFlags(MemoryTypeFlagsExpr->getExprLoc(),
                                            MemoryTypeFlags, hasVisibleGroup,
                                            EntrySK, EntryDecl, Diags);
      // Consider group scope if any group flags remain.
      memAtLeastGroupScope = 0 != MemoryTypeFlags;
      // Consider it device scope if UavMemory or any NodeFlags remain.
      memAtLeastDeviceScope =
          0 != (MemoryTypeFlags & ((uint32_t)DXIL::MemoryTypeFlag::UavMemory |
                                   (uint32_t)DXIL::MemoryTypeFlag::NodeFlags));
    }
  } else {
    DXIL::NodeIOKind IOKind = GetNodeIOType(Param0Ty);
    if (IOKind == DXIL::NodeIOKind::Invalid) {
      // overload: Barrier(<UAV Resource>, uint SemanticFlags)
      // UAV objects have at least device scope.
      DXASSERT(IsHLSLResourceType(Param0Ty),
               "otherwise, missed a case for Barrier");
      // mem scope flags already set to true.
    } else {
      // Must be a record object
      // overload: Barrier(<node record object>, uint SemanticFlags)
      // Only record objects specify a record granularity
      DXASSERT((uint32_t)IOKind &
                   (uint32_t)DXIL::NodeIOFlags::RecordGranularityMask,
               "otherwise, missed a Node object case for Barrier");

      DXIL::NodeIOFlags RecordGranularity = (DXIL::NodeIOFlags)(
          (uint32_t)IOKind &
          (uint32_t)DXIL::NodeIOFlags::RecordGranularityMask);
      switch (RecordGranularity) {
      case DXIL::NodeIOFlags::ThreadRecord:
        memAtLeastGroupScope = false;
        LLVM_FALLTHROUGH;
      case DXIL::NodeIOFlags::GroupRecord:
        memAtLeastDeviceScope = false;
        break;
      default:
        break;
      }
    }
  }

  // All barrier overloads have SemanticFlags as second paramter
  uint32_t SemanticFlags = 0;
  Expr *SemanticFlagsExpr = CE->getArg(1);
  llvm::APSInt SemanticFlagsVal;
  if (SemanticFlagsExpr->isIntegerConstantExpr(SemanticFlagsVal, S.Context)) {
    SemanticFlags = SemanticFlagsVal.getLimitedValue();
    DiagnoseSemanticFlags(SemanticFlagsExpr->getExprLoc(), SemanticFlags,
                          hasVisibleGroup, memAtLeastGroupScope,
                          memAtLeastDeviceScope, EntryDecl, Diags);
  }
}

bool IsRayFlagForceOMM2StateSet(Sema &sema, const CallExpr *CE) {
  const Expr *Expr1 = CE->getArg(1);
  llvm::APSInt constantResult;
  return Expr1->isIntegerConstantExpr(constantResult, sema.getASTContext()) &&
         (constantResult.getLimitedValue() &
          (uint64_t)DXIL::RayFlag::ForceOMM2State) != 0;
}

void DiagnoseTraceRayInline(Sema &sema, CallExpr *callExpr) {
  // Validate if the RayFlag parameter has RAY_FLAG_FORCE_OMM_2_STATE set,
  // the RayQuery decl must have RAYQUERY_FLAG_ALLOW_OPACITY_MICROMAPS set,
  // otherwise emit a diagnostic.
  if (IsRayFlagForceOMM2StateSet(sema, callExpr)) {
    CXXMemberCallExpr *CXXCallExpr = dyn_cast<CXXMemberCallExpr>(callExpr);
    if (!CXXCallExpr) {
      return;
    }
    const DeclRefExpr *DRE =
        dyn_cast<DeclRefExpr>(CXXCallExpr->getImplicitObjectArgument());
    assert(DRE);
    QualType QT = DRE->getType();
    auto *typeRecordDecl = QT->getAsCXXRecordDecl();
    ClassTemplateSpecializationDecl *SpecDecl =
        llvm::dyn_cast<ClassTemplateSpecializationDecl>(typeRecordDecl);

    if (!SpecDecl)
      return;

    // Guaranteed 2 arguments since the rayquery constructor
    // automatically creates 2 template args
    DXASSERT(SpecDecl->getTemplateArgs().size() == 2,
             "else rayquery constructor template args are not 2");
    llvm::APSInt Arg2val = SpecDecl->getTemplateArgs()[1].getAsIntegral();
    bool IsRayQueryAllowOMMSet =
        Arg2val.getZExtValue() &
        (unsigned)DXIL::RayQueryFlag::AllowOpacityMicromaps;
    if (!IsRayQueryAllowOMMSet) {
      // Diagnose the call
      sema.Diag(CXXCallExpr->getExprLoc(),
                diag::warn_hlsl_rayquery_flags_conflict);
      sema.Diag(DRE->getDecl()->getLocation(), diag::note_previous_decl)
          << "RayQueryFlags";
    }
  }
}

static bool isStringLiteral(QualType type) {
  if (!type->isConstantArrayType())
    return false;
  const Type *eType = type->getArrayElementTypeNoTypeQual();
  return eType->isSpecificBuiltinType(BuiltinType::Char_S);
}

static void DiagnoseReachableSERCall(Sema &S, CallExpr *CE,
                                     DXIL::ShaderKind EntrySK,
                                     const FunctionDecl *EntryDecl,
                                     bool IsReorderOperation) {
  bool ValidEntry = false;
  switch (EntrySK) {
  default:
    break;
  case DXIL::ShaderKind::ClosestHit:
  case DXIL::ShaderKind::Miss:
    ValidEntry = !IsReorderOperation;
    break;
  case DXIL::ShaderKind::RayGeneration:
    ValidEntry = true;
    break;
  }

  if (ValidEntry)
    return;

  int DiagID = IsReorderOperation ? diag::err_hlsl_reorder_unsupported_stage
                                  : diag::err_hlsl_hitobject_unsupported_stage;

  SourceLocation EntryLoc = EntryDecl->getLocation();
  SourceLocation Loc = CE->getExprLoc();
  S.Diag(Loc, DiagID) << ShaderModel::FullNameFromKind(EntrySK);
  S.Diag(EntryLoc, diag::note_hlsl_entry_defined_here);
}

// Check HLSL member call constraints for used functions.
// locallyVisited is true if this call has been visited already from any other
// entry function.  Used to avoid duplicate diagnostics when not dependent on
// entry function (or export function) properties.
void Sema::DiagnoseReachableHLSLCall(CallExpr *CE, const hlsl::ShaderModel *SM,
                                     DXIL::ShaderKind EntrySK,
                                     DXIL::NodeLaunchType NodeLaunchTy,
                                     const FunctionDecl *EntryDecl,
                                     bool locallyVisited) {
  FunctionDecl *FD = CE->getDirectCallee();
  if (!FD)
    return;

  for (ParmVarDecl *P : FD->parameters()) {
    if (isStringLiteral(P->getType())) {
      Diags.Report(CE->getExprLoc(), diag::err_hlsl_unsupported_string_literal);
    }
  }

  HLSLIntrinsicAttr *IntrinsicAttr = FD->getAttr<HLSLIntrinsicAttr>();
  if (!IntrinsicAttr)
    return;
  if (!IsBuiltinTable(IntrinsicAttr->getGroup()))
    return;

  SourceLocation Loc = CE->getExprLoc();
  hlsl::IntrinsicOp opCode = (IntrinsicOp)IntrinsicAttr->getOpcode();
  switch (opCode) {
  case hlsl::IntrinsicOp::MOP_CalculateLevelOfDetail:
  case hlsl::IntrinsicOp::MOP_CalculateLevelOfDetailUnclamped:
    DiagnoseCalculateLOD(*this, FD, Loc, SM, EntrySK, NodeLaunchTy, EntryDecl,
                         Diags, locallyVisited);
    break;
  case hlsl::IntrinsicOp::IOP_Barrier:
    DiagnoseReachableBarrier(*this, CE, SM, EntrySK, NodeLaunchTy, EntryDecl,
                             Diags);
    break;
  case hlsl::IntrinsicOp::MOP_TraceRayInline:
    DiagnoseTraceRayInline(*this, CE);
    break;
  case hlsl::IntrinsicOp::MOP_DxHitObject_FromRayQuery:
  case hlsl::IntrinsicOp::MOP_DxHitObject_Invoke:
  case hlsl::IntrinsicOp::MOP_DxHitObject_MakeMiss:
  case hlsl::IntrinsicOp::MOP_DxHitObject_MakeNop:
  case hlsl::IntrinsicOp::MOP_DxHitObject_TraceRay:
    DiagnoseReachableSERCall(*this, CE, EntrySK, EntryDecl, false);
    break;
  case hlsl::IntrinsicOp::IOP_DxMaybeReorderThread:
    DiagnoseReachableSERCall(*this, CE, EntrySK, EntryDecl, true);
    break;
  default:
    break;
  }
}

/////////////////////////////////////////////////////////////////////////////

static bool AllowObjectInContext(QualType Ty, TypeDiagContext DiagContext) {
  // Disallow all object in template type parameters (former
  // err_hlsl_objectintemplateargument)
  if (DiagContext == TypeDiagContext::TypeParameter)
    return false;
  // Disallow all objects in node records (former
  // err_hlsl_node_record_object)
  if (DiagContext == TypeDiagContext::NodeRecords)
    return false;
  // TODO: Extend this list for other object types.
  if (IsHLSLHitObjectType(Ty))
    return false;
  return true;
}

// Determine if `Ty` is valid in this `DiagContext` and/or an empty type.  If
// invalid returns false and Sema `S`, location `Loc`, error index
// `DiagContext`, and FieldDecl `FD` are used to emit diagnostics. If
// `CheckLongVec` is set, errors are produced if `Ty` is a long vector. If the
// type is not empty, `Empty` is set to false. `CheckedDecls` is used to prevent
// redundant recursive type checks.
static bool
DiagnoseElementTypes(Sema &S, SourceLocation Loc, QualType Ty, bool &Empty,
                     TypeDiagContext ObjDiagContext,
                     TypeDiagContext LongVecDiagContext,
                     llvm::SmallPtrSet<const RecordDecl *, 8> &CheckedDecls,
                     const clang::FieldDecl *FD) {
  if (Ty.isNull() || Ty->isDependentType())
    return false;

  const bool CheckLongVec = LongVecDiagContext != TypeDiagContext::Valid;
  const bool CheckObjects = ObjDiagContext != TypeDiagContext::Valid;

  while (const ArrayType *Arr = Ty->getAsArrayTypeUnsafe())
    Ty = Arr->getElementType();

  const int ObjDiagContextIdx = static_cast<int>(ObjDiagContext);
  const int LongVecDiagContextIdx = static_cast<int>(LongVecDiagContext);
  DXASSERT_NOMSG(
      LongVecDiagContext == TypeDiagContext::Valid ||
      (0 <= LongVecDiagContextIdx &&
       LongVecDiagContextIdx <=
           static_cast<int>(TypeDiagContext::LongVecDiagMaxSelectIndex)));

  HLSLExternalSource *Source = HLSLExternalSource::FromSema(&S);
  ArTypeObjectKind ShapeKind = Source->GetTypeObjectKind(Ty);
  switch (ShapeKind) {
  case AR_TOBJ_VECTOR:
    if (CheckLongVec && GetHLSLVecSize(Ty) > DXIL::kDefaultMaxVectorLength) {
      S.Diag(Loc, diag::err_hlsl_unsupported_long_vector)
          << LongVecDiagContextIdx;
      Empty = false;
      return false;
    }
    LLVM_FALLTHROUGH;
  case AR_TOBJ_BASIC:
  case AR_TOBJ_MATRIX:
    Empty = false;
    return false;
  case AR_TOBJ_OBJECT:
    Empty = false;
    if (!CheckObjects || AllowObjectInContext(Ty, ObjDiagContext))
      return false;
    S.Diag(Loc, diag::err_hlsl_unsupported_object_context)
        << Ty << ObjDiagContextIdx;
    if (FD)
      S.Diag(FD->getLocation(), diag::note_field_declared_here)
          << FD->getType() << FD->getSourceRange();
    return true;
  case AR_TOBJ_DEPENDENT:
    llvm_unreachable("obj dependent should go dependent type path, not reach "
                     "here");
    return true;
  case AR_TOBJ_COMPOUND: {
    bool ErrorFound = false;
    const RecordDecl *RD = Ty->getAs<RecordType>()->getDecl();
    // Never recurse redundantly into related subtypes that have already been
    // checked.
    if (!CheckedDecls.insert(RD).second)
      return false;

    // Check the fields of the RecordDecl
    for (auto *ElemFD : RD->fields()) {
      ErrorFound |=
          DiagnoseElementTypes(S, Loc, ElemFD->getType(), Empty, ObjDiagContext,
                               LongVecDiagContext, CheckedDecls, ElemFD);
    }
    if (!RD->isCompleteDefinition())
      return ErrorFound;

    if (auto *Child = dyn_cast<CXXRecordDecl>(RD))
      // Walk up the inheritance chain and check base class fields
      for (auto &B : Child->bases())
        ErrorFound |=
            DiagnoseElementTypes(S, Loc, B.getType(), Empty, ObjDiagContext,
                                 LongVecDiagContext, CheckedDecls, nullptr);
    return ErrorFound;
  }
  default:
    // Not a recursive type, no element types to check here
    Empty = false;
    return false;
  }
}

bool hlsl::DiagnoseTypeElements(Sema &S, SourceLocation Loc, QualType Ty,
                                TypeDiagContext ObjDiagContext,
                                TypeDiagContext LongVecDiagContext,
                                const clang::FieldDecl *FD) {
  bool Empty = false;
  llvm::SmallPtrSet<const RecordDecl *, 8> CheckedDecls;
  return DiagnoseElementTypes(S, Loc, Ty, Empty, ObjDiagContext,
                              LongVecDiagContext, CheckedDecls, FD);
}

bool hlsl::DiagnoseNodeStructArgument(Sema *self, TemplateArgumentLoc ArgLoc,
                                      QualType ArgTy, bool &Empty,
                                      const FieldDecl *FD) {
  llvm::SmallPtrSet<const RecordDecl *, 8> CheckedDecls;
  return DiagnoseElementTypes(*self, ArgLoc.getLocation(), ArgTy, Empty,
                              TypeDiagContext::NodeRecords,
                              TypeDiagContext::NodeRecords, CheckedDecls, FD);
}

// This function diagnoses whether or not all entry-point attributes
// should exist on this shader stage
void DiagnoseEntryAttrAllowedOnStage(clang::Sema *self,
                                     FunctionDecl *entryPointDecl,
                                     DXIL::ShaderKind shaderKind) {

  if (entryPointDecl->hasAttrs()) {
    for (Attr *pAttr : entryPointDecl->getAttrs()) {
      switch (pAttr->getKind()) {
      case clang::attr::HLSLWaveSize: {
        switch (shaderKind) {
        case DXIL::ShaderKind::Compute:
        case DXIL::ShaderKind::Node:
          break;
        default:
          self->Diag(pAttr->getRange().getBegin(),
                     diag::err_hlsl_attribute_unsupported_stage)
              << "WaveSize"
              << "compute or node";
          break;
        }
        break;
      }
      case clang::attr::HLSLNodeLaunch:
      case clang::attr::HLSLNodeIsProgramEntry:
      case clang::attr::HLSLNodeId:
      case clang::attr::HLSLNodeLocalRootArgumentsTableIndex:
      case clang::attr::HLSLNodeShareInputOf:
      case clang::attr::HLSLNodeDispatchGrid:
      case clang::attr::HLSLNodeMaxDispatchGrid:
      case clang::attr::HLSLNodeMaxRecursionDepth: {
        if (shaderKind != DXIL::ShaderKind::Node) {
          self->Diag(pAttr->getRange().getBegin(),
                     diag::err_hlsl_attribute_unsupported_stage)
              << pAttr->getSpelling() << "node";
        }
        break;
      }
      }
    }
  }
}

std::string getFQFunctionName(FunctionDecl *FD) {
  std::string name = "";
  if (!FD) {
    return name;
  }
  if (FD->getName().empty()) {
    // Anonymous functions are not supported.
    return name;
  }
  name = FD->getName();
  while (!FD->isGlobal()) {
    DeclContext *parent = FD->getParent();
    if (NamespaceDecl *ns = dyn_cast<NamespaceDecl>(parent)) {
      // function declaration is in a namespace
      name = ns->getName().str() + "::" + name;
    } else if (RecordDecl *record = dyn_cast<RecordDecl>(parent)) {
      // function declaration is in a record or class
      name = record->getName().str() + "::" + name;
    } else if (FunctionDecl *parentFunc = dyn_cast<FunctionDecl>(parent)) {
      // function declaration is in a nested function
      name = parentFunc->getName().str() + "::" + name;
      FD = parentFunc;
    } else {
      // function declaration is in an unknown scope
      name = "unknown::" + name;
    }
  }

  return name;
}

void hlsl::DiagnosePayloadAccessQualifierAnnotations(
    Sema &S, Declarator &D, const QualType &T,
    const std::vector<hlsl::UnusualAnnotation *> &annotations) {

  auto &&iter = annotations.begin();
  auto &&end = annotations.end();

  hlsl::PayloadAccessAnnotation *readAnnotation = nullptr;
  hlsl::PayloadAccessAnnotation *writeAnnotation = nullptr;

  for (; iter != end; ++iter) {
    switch ((*iter)->getKind()) {
    case hlsl::UnusualAnnotation::UA_PayloadAccessQualifier: {
      hlsl::PayloadAccessAnnotation *annotation =
          cast<hlsl::PayloadAccessAnnotation>(*iter);
      if (annotation->qualifier == DXIL::PayloadAccessQualifier::Read) {
        if (!readAnnotation)
          readAnnotation = annotation;
        else {
          S.Diag(annotation->Loc,
                 diag::err_hlsl_payload_access_qualifier_multiple_defined)
              << "read";
          return;
        }
      } else if (annotation->qualifier == DXIL::PayloadAccessQualifier::Write) {
        if (!writeAnnotation)
          writeAnnotation = annotation;
        else {
          S.Diag(annotation->Loc,
                 diag::err_hlsl_payload_access_qualifier_multiple_defined)
              << "write";
          return;
        }
      }

      break;
    }
    default:
      // Ignore all other annotations here.
      break;
    }
  }

  struct PayloadAccessQualifierInformation {
    bool anyhit = false;
    bool closesthit = false;
    bool miss = false;
    bool caller = false;
  } readQualContains, writeQualContains;

  auto collectInformationAboutShaderStages =
      [&](hlsl::PayloadAccessAnnotation *annotation,
          PayloadAccessQualifierInformation &info) {
        for (auto shaderType : annotation->ShaderStages) {
          if (shaderType == DXIL::PayloadAccessShaderStage::Anyhit)
            info.anyhit = true;
          else if (shaderType == DXIL::PayloadAccessShaderStage::Closesthit)
            info.closesthit = true;
          else if (shaderType == DXIL::PayloadAccessShaderStage::Miss)
            info.miss = true;
          else if (shaderType == DXIL::PayloadAccessShaderStage::Caller)
            info.caller = true;
        }
        return true;
      };

  if (readAnnotation) {
    if (!collectInformationAboutShaderStages(readAnnotation, readQualContains))
      return;
  }
  if (writeAnnotation) {
    if (!collectInformationAboutShaderStages(writeAnnotation,
                                             writeQualContains))
      return;
  }

  if (writeAnnotation) {
    // Note: keep the following two checks separated to diagnose both
    //       stages (closesthit and miss)
    // If closesthit/miss writes a value the caller must consume it.
    if (writeQualContains.miss) {
      if (!readAnnotation || !readQualContains.caller) {
        S.Diag(writeAnnotation->Loc,
               diag::err_hlsl_payload_access_qualifier_invalid_combination)
            << D.getIdentifier() << "write"
            << "miss"
            << "consumer";
      }
    }
    if (writeQualContains.closesthit) {
      if (!readAnnotation || !readQualContains.caller) {
        S.Diag(writeAnnotation->Loc,
               diag::err_hlsl_payload_access_qualifier_invalid_combination)
            << D.getIdentifier() << "write"
            << "closesthit"
            << "consumer";
      }
    }

    // If anyhit writes, we need at least one consumer
    if (writeQualContains.anyhit && !readAnnotation) {
      S.Diag(writeAnnotation->Loc,
             diag::err_hlsl_payload_access_qualifier_invalid_combination)
          << D.getIdentifier() << "write"
          << "anyhit"
          << "consumer";
    }

    // If the caller writes, we need at least one consumer
    if (writeQualContains.caller && !readAnnotation) {
      S.Diag(writeAnnotation->Loc,
             diag::err_hlsl_payload_access_qualifier_invalid_combination)
          << D.getIdentifier() << "write"
          << "caller"
          << "consumer";
    }
  }

  // Validate the read qualifer if present.
  if (readAnnotation) {
    // Note: keep the following two checks separated to diagnose both
    //       stages (closesthit and miss)
    // If closeshit/miss consume a value we need a producer.
    // Valid producers are the caller and anyhit.
    if (readQualContains.miss) {
      if (!writeAnnotation ||
          !(writeQualContains.anyhit || writeQualContains.caller)) {
        S.Diag(readAnnotation->Loc,
               diag::err_hlsl_payload_access_qualifier_invalid_combination)
            << D.getIdentifier() << "read"
            << "miss"
            << "producer";
      }
    }

    // If closeshit/miss consume a value we need a producer.
    // Valid producers are the caller and anyhit.
    if (readQualContains.closesthit) {
      if (!writeAnnotation ||
          !(writeQualContains.anyhit || writeQualContains.caller)) {
        S.Diag(readAnnotation->Loc,
               diag::err_hlsl_payload_access_qualifier_invalid_combination)
            << D.getIdentifier() << "read"
            << "closesthit"
            << "producer";
      }
    }

    // If anyhit consumes the value we need a producer.
    // Valid producers are the caller and antoher anyhit.
    if (readQualContains.anyhit) {
      if (!writeAnnotation ||
          !(writeQualContains.anyhit || writeQualContains.caller)) {
        S.Diag(readAnnotation->Loc,
               diag::err_hlsl_payload_access_qualifier_invalid_combination)
            << D.getIdentifier() << "read"
            << "anyhit"
            << "producer";
      }
    }

    // If the caller consumes the value we need a valid producer.
    if (readQualContains.caller && !writeAnnotation) {
      S.Diag(readAnnotation->Loc,
             diag::err_hlsl_payload_access_qualifier_invalid_combination)
          << D.getIdentifier() << "read"
          << "caller"
          << "producer";
    }
  }
}

void hlsl::DiagnoseUnusualAnnotationsForHLSL(
    Sema &S, std::vector<hlsl::UnusualAnnotation *> &annotations) {
  bool packoffsetOverriddenReported = false;
  auto &&iter = annotations.begin();
  auto &&end = annotations.end();
  for (; iter != end; ++iter) {
    switch ((*iter)->getKind()) {
    case hlsl::UnusualAnnotation::UA_ConstantPacking: {
      hlsl::ConstantPacking *constantPacking =
          cast<hlsl::ConstantPacking>(*iter);

      // Check whether this will conflict with other packoffsets. If so, only
      // issue a warning; last one wins.
      if (!packoffsetOverriddenReported) {
        auto newIter = iter;
        ++newIter;
        while (newIter != end) {
          hlsl::ConstantPacking *other =
              dyn_cast_or_null<hlsl::ConstantPacking>(*newIter);
          if (other != nullptr &&
              (other->Subcomponent != constantPacking->Subcomponent ||
               other->ComponentOffset != constantPacking->ComponentOffset)) {
            S.Diag(constantPacking->Loc, diag::warn_hlsl_packoffset_overridden);
            packoffsetOverriddenReported = true;
            break;
          }
          ++newIter;
        }
      }

      break;
    }
    case hlsl::UnusualAnnotation::UA_RegisterAssignment: {
      hlsl::RegisterAssignment *registerAssignment =
          cast<hlsl::RegisterAssignment>(*iter);

      // Check whether this will conflict with other register assignments of the
      // same type.
      auto newIter = iter;
      ++newIter;
      while (newIter != end) {
        hlsl::RegisterAssignment *other =
            dyn_cast_or_null<hlsl::RegisterAssignment>(*newIter);

        // Same register bank and profile, but different number.
        if (other != nullptr &&
            ShaderModelsMatch(other->ShaderProfile,
                              registerAssignment->ShaderProfile) &&
            other->RegisterType == registerAssignment->RegisterType &&
            (other->RegisterNumber != registerAssignment->RegisterNumber ||
             other->RegisterOffset != registerAssignment->RegisterOffset)) {
          // Obvious conflict - report it up front.
          S.Diag(registerAssignment->Loc,
                 diag::err_hlsl_register_semantics_conflicting);
        }

        ++newIter;
      }
      break;
    }
    case hlsl::UnusualAnnotation::UA_SemanticDecl: {
      // hlsl::SemanticDecl* semanticDecl = cast<hlsl::SemanticDecl>(*iter);
      // No common validation to be performed.
      break;
    }
    case hlsl::UnusualAnnotation::UA_PayloadAccessQualifier: {
      // Validation happens sperately
      break;
    }
    }
  }
}

clang::OverloadingResult
hlsl::GetBestViableFunction(clang::Sema &S, clang::SourceLocation Loc,
                            clang::OverloadCandidateSet &set,
                            clang::OverloadCandidateSet::iterator &Best) {
  return HLSLExternalSource::FromSema(&S)->GetBestViableFunction(Loc, set,
                                                                 Best);
}

void hlsl::InitializeInitSequenceForHLSL(Sema *self,
                                         const InitializedEntity &Entity,
                                         const InitializationKind &Kind,
                                         MultiExprArg Args,
                                         bool TopLevelOfInitList,
                                         InitializationSequence *initSequence) {
  return HLSLExternalSource::FromSema(self)->InitializeInitSequenceForHLSL(
      Entity, Kind, Args, TopLevelOfInitList, initSequence);
}

static unsigned CaculateInitListSize(HLSLExternalSource *hlslSource,
                                     const clang::InitListExpr *InitList) {
  unsigned totalSize = 0;
  for (unsigned i = 0; i < InitList->getNumInits(); i++) {
    const clang::Expr *EltInit = InitList->getInit(i);
    QualType EltInitTy = EltInit->getType();
    if (const InitListExpr *EltInitList = dyn_cast<InitListExpr>(EltInit)) {
      totalSize += CaculateInitListSize(hlslSource, EltInitList);
    } else {
      totalSize += hlslSource->GetNumBasicElements(EltInitTy);
    }
  }
  return totalSize;
}

unsigned
hlsl::CaculateInitListArraySizeForHLSL(clang::Sema *sema,
                                       const clang::InitListExpr *InitList,
                                       const clang::QualType EltTy) {
  HLSLExternalSource *hlslSource = HLSLExternalSource::FromSema(sema);
  unsigned totalSize = CaculateInitListSize(hlslSource, InitList);
  unsigned eltSize = hlslSource->GetNumBasicElements(EltTy);

  if (totalSize > 0 && (totalSize % eltSize) == 0) {
    return totalSize / eltSize;
  } else {
    return 0;
  }
}

// NRVO unsafe for a variety of cases in HLSL
// - vectors/matrix with bool component types
// - attributes not captured to QualType, such as precise and globallycoherent
bool hlsl::ShouldSkipNRVO(clang::Sema &sema, clang::QualType returnType,
                          clang::VarDecl *VD, clang::FunctionDecl *FD) {
  // exclude vectors/matrix (not treated as record type)
  // NRVO breaks on bool component type due to diff between
  // i32 memory and i1 register representation
  if (hlsl::IsHLSLVecMatType(returnType))
    return true;
  QualType ArrayEltTy = returnType;
  while (const clang::ArrayType *AT =
             sema.getASTContext().getAsArrayType(ArrayEltTy)) {
    ArrayEltTy = AT->getElementType();
  }
  // exclude resource for globallycoherent.
  if (hlsl::IsHLSLResourceType(ArrayEltTy) || hlsl::IsHLSLNodeType(ArrayEltTy))
    return true;
  // exclude precise.
  if (VD->hasAttr<HLSLPreciseAttr>()) {
    return true;
  }
  if (FD) {
    // propagate precise the the VD.
    if (FD->hasAttr<HLSLPreciseAttr>()) {
      VD->addAttr(FD->getAttr<HLSLPreciseAttr>());
      return true;
    }

    // Don't do NRVO if this is an entry function or a patch contsant function.
    // With NVRO, writing to the return variable directly writes to the output
    // argument instead of to an alloca which gets copied to the output arg in
    // one spot. This causes many extra dx.storeOutput's to be emitted.
    //
    // Check if this is an entry function the easy way if we're a library
    if (const HLSLShaderAttr *Attr = FD->getAttr<HLSLShaderAttr>()) {
      return true;
    }
    // Check if it's an entry function the hard way
    if (!FD->getDeclContext()->isNamespace() && FD->isGlobal()) {
      // Check if this is an entry function by comparing name
      // TODO: Remove this once we put HLSLShaderAttr on all entry functions.
      if (FD->getIdentifier() &&
          FD->getName() == sema.getLangOpts().HLSLEntryFunction) {
        return true;
      }

      // See if it's the patch constant function
      if (sema.getLangOpts().HLSLProfile.size() &&
          (sema.getLangOpts().HLSLProfile[0] == 'h' /*For 'hs'*/ ||
           sema.getLangOpts().HLSLProfile[0] == 'l' /*For 'lib'*/)) {
        if (hlsl::IsPatchConstantFunctionDecl(FD))
          return true;
      }
    }
  }

  return false;
}

bool hlsl::IsConversionToLessOrEqualElements(
    clang::Sema *self, const clang::ExprResult &sourceExpr,
    const clang::QualType &targetType, bool explicitConversion) {
  return HLSLExternalSource::FromSema(self)->IsConversionToLessOrEqualElements(
      sourceExpr, targetType, explicitConversion);
}

ExprResult hlsl::LookupMatrixMemberExprForHLSL(Sema *self, Expr &BaseExpr,
                                               DeclarationName MemberName,
                                               bool IsArrow,
                                               SourceLocation OpLoc,
                                               SourceLocation MemberLoc) {
  return HLSLExternalSource::FromSema(self)->LookupMatrixMemberExprForHLSL(
      BaseExpr, MemberName, IsArrow, OpLoc, MemberLoc);
}

ExprResult hlsl::LookupVectorMemberExprForHLSL(Sema *self, Expr &BaseExpr,
                                               DeclarationName MemberName,
                                               bool IsArrow,
                                               SourceLocation OpLoc,
                                               SourceLocation MemberLoc) {
  return HLSLExternalSource::FromSema(self)->LookupVectorMemberExprForHLSL(
      BaseExpr, MemberName, IsArrow, OpLoc, MemberLoc);
}

ExprResult hlsl::LookupArrayMemberExprForHLSL(Sema *self, Expr &BaseExpr,
                                              DeclarationName MemberName,
                                              bool IsArrow,
                                              SourceLocation OpLoc,
                                              SourceLocation MemberLoc) {
  return HLSLExternalSource::FromSema(self)->LookupArrayMemberExprForHLSL(
      BaseExpr, MemberName, IsArrow, OpLoc, MemberLoc);
}

bool hlsl::LookupRecordMemberExprForHLSL(Sema *self, Expr &BaseExpr,
                                         DeclarationName MemberName,
                                         bool IsArrow, SourceLocation OpLoc,
                                         SourceLocation MemberLoc,
                                         ExprResult &result) {
  HLSLExternalSource *source = HLSLExternalSource::FromSema(self);
  switch (source->GetTypeObjectKind(BaseExpr.getType())) {
  case AR_TOBJ_MATRIX:
    result = source->LookupMatrixMemberExprForHLSL(BaseExpr, MemberName,
                                                   IsArrow, OpLoc, MemberLoc);
    return true;
  case AR_TOBJ_VECTOR:
    result = source->LookupVectorMemberExprForHLSL(BaseExpr, MemberName,
                                                   IsArrow, OpLoc, MemberLoc);
    return true;
  case AR_TOBJ_ARRAY:
    result = source->LookupArrayMemberExprForHLSL(BaseExpr, MemberName, IsArrow,
                                                  OpLoc, MemberLoc);
    return true;
  default:
    return false;
  }
  return false;
}

clang::ExprResult hlsl::MaybeConvertMemberAccess(clang::Sema *self,
                                                 clang::Expr *E) {
  return HLSLExternalSource::FromSema(self)->MaybeConvertMemberAccess(E);
}

bool hlsl::TryStaticCastForHLSL(
    Sema *self, ExprResult &SrcExpr, QualType DestType,
    Sema::CheckedConversionKind CCK, const SourceRange &OpRange, unsigned &msg,
    CastKind &Kind, CXXCastPath &BasePath, bool ListInitialization,
    bool SuppressDiagnostics, StandardConversionSequence *standard) {
  return HLSLExternalSource::FromSema(self)->TryStaticCastForHLSL(
      SrcExpr, DestType, CCK, OpRange, msg, Kind, BasePath, ListInitialization,
      SuppressDiagnostics, SuppressDiagnostics, standard);
}

clang::ExprResult
hlsl::PerformHLSLConversion(clang::Sema *self, clang::Expr *From,
                            clang::QualType targetType,
                            const clang::StandardConversionSequence &SCS,
                            clang::Sema::CheckedConversionKind CCK) {
  return HLSLExternalSource::FromSema(self)->PerformHLSLConversion(
      From, targetType, SCS, CCK);
}

clang::ImplicitConversionSequence
hlsl::TrySubscriptIndexInitialization(clang::Sema *self, clang::Expr *SrcExpr,
                                      clang::QualType DestType) {
  return HLSLExternalSource::FromSema(self)->TrySubscriptIndexInitialization(
      SrcExpr, DestType);
}

/// <summary>Performs HLSL-specific initialization on the specified
/// context.</summary>
void hlsl::InitializeASTContextForHLSL(ASTContext &context) {
  HLSLExternalSource *hlslSource = new HLSLExternalSource();
  IntrusiveRefCntPtr<ExternalASTSource> externalSource(hlslSource);
  if (hlslSource->Initialize(context)) {
    context.setExternalSource(externalSource);
  }
}

////////////////////////////////////////////////////////////////////////////////
// FlattenedTypeIterator implementation                                       //

/// <summary>Constructs a FlattenedTypeIterator for the specified
/// type.</summary>
FlattenedTypeIterator::FlattenedTypeIterator(SourceLocation loc, QualType type,
                                             HLSLExternalSource &source)
    : m_source(source), m_draining(false), m_springLoaded(false),
      m_incompleteCount(0), m_typeDepth(0), m_loc(loc) {
  if (pushTrackerForType(type, nullptr)) {
    while (!m_typeTrackers.empty() && !considerLeaf())
      consumeLeaf();
  }
}

/// <summary>Constructs a FlattenedTypeIterator for the specified
/// expressions.</summary>
FlattenedTypeIterator::FlattenedTypeIterator(SourceLocation loc,
                                             MultiExprArg args,
                                             HLSLExternalSource &source)
    : m_source(source), m_draining(false), m_springLoaded(false),
      m_incompleteCount(0), m_typeDepth(0), m_loc(loc) {
  if (!args.empty()) {
    MultiExprArg::iterator ii = args.begin();
    MultiExprArg::iterator ie = args.end();
    DXASSERT(ii != ie, "otherwise empty() returned an incorrect value");
    m_typeTrackers.push_back(
        FlattenedTypeIterator::FlattenedTypeTracker(ii, ie));

    if (!considerLeaf()) {
      m_typeTrackers.clear();
    }
  }
}

/// <summary>Gets the current element in the flattened type hierarchy.</summary>
QualType FlattenedTypeIterator::getCurrentElement() const {
  return m_typeTrackers.back().Type;
}

/// <summary>Get the number of repeated current elements.</summary>
unsigned int FlattenedTypeIterator::getCurrentElementSize() const {
  const FlattenedTypeTracker &back = m_typeTrackers.back();
  return (back.IterKind == FK_IncompleteArray) ? 1 : back.Count;
}

/// <summary>Checks whether the iterator has a current element type to
/// report.</summary>
bool FlattenedTypeIterator::hasCurrentElement() const {
  return m_typeTrackers.size() > 0;
}

/// <summary>Consumes count elements on this iterator.</summary>
void FlattenedTypeIterator::advanceCurrentElement(unsigned int count) {
  DXASSERT(
      !m_typeTrackers.empty(),
      "otherwise caller should not be trying to advance to another element");
  DXASSERT(m_typeTrackers.back().IterKind == FK_IncompleteArray ||
               count <= m_typeTrackers.back().Count,
           "caller should never exceed currently pending element count");

  FlattenedTypeTracker &tracker = m_typeTrackers.back();
  if (tracker.IterKind == FK_IncompleteArray) {
    tracker.Count += count;
    m_springLoaded = true;
  } else {
    tracker.Count -= count;
    m_springLoaded = false;
    if (m_typeTrackers.back().Count == 0) {
      advanceLeafTracker();
    }
  }
}

unsigned int FlattenedTypeIterator::countRemaining() {
  m_draining = true; // when draining the iterator, incomplete arrays stop
                     // functioning as an infinite array
  size_t result = 0;
  while (hasCurrentElement() && !m_springLoaded) {
    size_t pending = getCurrentElementSize();
    result += pending;
    advanceCurrentElement(pending);
  }
  return result;
}

void FlattenedTypeIterator::advanceLeafTracker() {
  DXASSERT(
      !m_typeTrackers.empty(),
      "otherwise caller should not be trying to advance to another element");
  for (;;) {
    consumeLeaf();
    if (m_typeTrackers.empty()) {
      return;
    }

    if (considerLeaf()) {
      return;
    }
  }
}

bool FlattenedTypeIterator::considerLeaf() {
  if (m_typeTrackers.empty()) {
    return false;
  }

  m_typeDepth++;
  if (m_typeDepth > MaxTypeDepth) {
    m_source.ReportUnsupportedTypeNesting(m_loc, m_firstType);
    m_typeTrackers.clear();
    m_typeDepth--;
    return false;
  }

  bool result = false;
  FlattenedTypeTracker &tracker = m_typeTrackers.back();
  tracker.IsConsidered = true;

  switch (tracker.IterKind) {
  case FlattenedIterKind::FK_Expressions:
    if (pushTrackerForExpression(tracker.CurrentExpr)) {
      result = considerLeaf();
    }
    break;
  case FlattenedIterKind::FK_Fields:
    if (pushTrackerForType(tracker.CurrentField->getType(), nullptr)) {
      result = considerLeaf();
    }
    break;
  case FlattenedIterKind::FK_Bases:
    if (pushTrackerForType(tracker.CurrentBase->getType(), nullptr)) {
      result = considerLeaf();
    }
    break;
  case FlattenedIterKind::FK_IncompleteArray:
    m_springLoaded = true;
    LLVM_FALLTHROUGH;
  default:
  case FlattenedIterKind::FK_Simple: {
    ArTypeObjectKind objectKind = m_source.GetTypeObjectKind(tracker.Type);
    if (objectKind != ArTypeObjectKind::AR_TOBJ_BASIC &&
        objectKind != ArTypeObjectKind::AR_TOBJ_OBJECT &&
        objectKind != ArTypeObjectKind::AR_TOBJ_STRING) {
      if (pushTrackerForType(tracker.Type, tracker.CurrentExpr)) {
        result = considerLeaf();
      }
    } else {
      result = true;
    }
  }
  }

  m_typeDepth--;
  return result;
}

void FlattenedTypeIterator::consumeLeaf() {
  bool topConsumed = true; // Tracks whether we're processing the topmost item
                           // which we should consume.
  for (;;) {
    if (m_typeTrackers.empty()) {
      return;
    }

    FlattenedTypeTracker &tracker = m_typeTrackers.back();
    // Reach a leaf which is not considered before.
    // Stop here.
    if (!tracker.IsConsidered) {
      break;
    }
    switch (tracker.IterKind) {
    case FlattenedIterKind::FK_Expressions:
      ++tracker.CurrentExpr;
      if (tracker.CurrentExpr == tracker.EndExpr) {
        m_typeTrackers.pop_back();
        topConsumed = false;
      } else {
        return;
      }
      break;
    case FlattenedIterKind::FK_Fields:

      ++tracker.CurrentField;
      if (tracker.CurrentField == tracker.EndField) {
        m_typeTrackers.pop_back();
        topConsumed = false;
      } else {
        return;
      }
      break;
    case FlattenedIterKind::FK_Bases:
      ++tracker.CurrentBase;
      if (tracker.CurrentBase == tracker.EndBase) {
        m_typeTrackers.pop_back();
        topConsumed = false;
      } else {
        return;
      }
      break;
    case FlattenedIterKind::FK_IncompleteArray:
      if (m_draining) {
        DXASSERT(m_typeTrackers.size() == 1,
                 "m_typeTrackers.size() == 1, otherwise incomplete array isn't "
                 "topmost");
        m_incompleteCount = tracker.Count;
        m_typeTrackers.pop_back();
      }
      return;
    default:
    case FlattenedIterKind::FK_Simple: {
      m_springLoaded = false;
      if (!topConsumed) {
        DXASSERT(tracker.Count > 0,
                 "tracker.Count > 0 - otherwise we shouldn't be on stack");
        --tracker.Count;
      } else {
        topConsumed = false;
      }
      if (tracker.Count == 0) {
        m_typeTrackers.pop_back();
      } else {
        return;
      }
    }
    }
  }
}

bool FlattenedTypeIterator::pushTrackerForExpression(
    MultiExprArg::iterator expression) {
  Expr *e = *expression;
  Stmt::StmtClass expressionClass = e->getStmtClass();
  if (expressionClass == Stmt::StmtClass::InitListExprClass) {
    InitListExpr *initExpr = dyn_cast<InitListExpr>(e);
    if (initExpr->getNumInits() == 0) {
      return false;
    }

    MultiExprArg inits(initExpr->getInits(), initExpr->getNumInits());
    MultiExprArg::iterator ii = inits.begin();
    MultiExprArg::iterator ie = inits.end();
    DXASSERT(ii != ie, "otherwise getNumInits() returned an incorrect value");
    m_typeTrackers.push_back(
        FlattenedTypeIterator::FlattenedTypeTracker(ii, ie));
    return true;
  }

  return pushTrackerForType(e->getType(), expression);
}

// TODO: improve this to provide a 'peek' at intermediate types,
// which should help compare struct foo[1000] to avoid 1000 steps + per-field
// steps
bool FlattenedTypeIterator::pushTrackerForType(
    QualType type, MultiExprArg::iterator expression) {
  if (type->isVoidType()) {
    return false;
  }

  if (type->isFunctionType()) {
    return false;
  }

  if (m_firstType.isNull()) {
    m_firstType = type;
  }

  ArTypeObjectKind objectKind = m_source.GetTypeObjectKind(type);
  QualType elementType;
  unsigned int elementCount;
  const RecordType *recordType;
  RecordDecl::field_iterator fi, fe;
  switch (objectKind) {
  case ArTypeObjectKind::AR_TOBJ_ARRAY:
    // TODO: handle multi-dimensional arrays
    elementType = type->getAsArrayTypeUnsafe()
                      ->getElementType(); // handle arrays of arrays
    elementCount = GetArraySize(type);
    if (elementCount == 0) {
      if (type->isIncompleteArrayType()) {
        m_typeTrackers.push_back(
            FlattenedTypeIterator::FlattenedTypeTracker(elementType));
        return true;
      }
      return false;
    }

    m_typeTrackers.push_back(FlattenedTypeIterator::FlattenedTypeTracker(
        elementType, elementCount, nullptr));

    return true;
  case ArTypeObjectKind::AR_TOBJ_BASIC:
    m_typeTrackers.push_back(
        FlattenedTypeIterator::FlattenedTypeTracker(type, 1, expression));
    return true;
  case ArTypeObjectKind::AR_TOBJ_COMPOUND: {
    recordType = type->getAs<RecordType>();
    DXASSERT(recordType, "compound type is expected to be a RecordType");

    fi = recordType->getDecl()->field_begin();
    fe = recordType->getDecl()->field_end();

    bool bAddTracker = false;

    // Skip empty struct.
    if (fi != fe) {
      m_typeTrackers.push_back(
          FlattenedTypeIterator::FlattenedTypeTracker(type, fi, fe));
      type = (*fi)->getType();
      bAddTracker = true;
    }

    if (CXXRecordDecl *cxxRecordDecl =
            dyn_cast<CXXRecordDecl>(recordType->getDecl())) {
      // We'll error elsewhere if the record has no definition,
      // just don't attempt to use it.
      if (cxxRecordDecl->hasDefinition()) {
        CXXRecordDecl::base_class_iterator bi, be;
        bi = cxxRecordDecl->bases_begin();
        be = cxxRecordDecl->bases_end();
        if (bi != be) {
          // Add type tracker for base.
          // Add base after child to make sure base considered first.
          m_typeTrackers.push_back(
              FlattenedTypeIterator::FlattenedTypeTracker(type, bi, be));
          bAddTracker = true;
        }
      }
    }
    return bAddTracker;
  }
  case ArTypeObjectKind::AR_TOBJ_MATRIX:
    m_typeTrackers.push_back(FlattenedTypeIterator::FlattenedTypeTracker(
        m_source.GetMatrixOrVectorElementType(type), GetElementCount(type),
        nullptr));
    return true;
  case ArTypeObjectKind::AR_TOBJ_VECTOR:
    m_typeTrackers.push_back(FlattenedTypeIterator::FlattenedTypeTracker(
        m_source.GetMatrixOrVectorElementType(type), GetHLSLVecSize(type),
        nullptr));
    return true;
  case ArTypeObjectKind::AR_TOBJ_OBJECT: {
    if (m_source.IsSubobjectType(type)) {
      // subobjects are initialized with initialization lists
      recordType = type->getAs<RecordType>();
      fi = recordType->getDecl()->field_begin();
      fe = recordType->getDecl()->field_end();

      m_typeTrackers.push_back(
          FlattenedTypeIterator::FlattenedTypeTracker(type, fi, fe));
      return true;
    } else {
      // Object have no sub-types.
      m_typeTrackers.push_back(FlattenedTypeIterator::FlattenedTypeTracker(
          type.getCanonicalType(), 1, expression));
      return true;
    }
  }
  case ArTypeObjectKind::AR_TOBJ_STRING: {
    // Strings have no sub-types.
    m_typeTrackers.push_back(FlattenedTypeIterator::FlattenedTypeTracker(
        type.getCanonicalType(), 1, expression));
    return true;
  }
  default:
    DXASSERT(false, "unreachable");
    return false;
  }
}

FlattenedTypeIterator::ComparisonResult FlattenedTypeIterator::CompareIterators(
    HLSLExternalSource &source, SourceLocation loc,
    FlattenedTypeIterator &leftIter, FlattenedTypeIterator &rightIter) {
  FlattenedTypeIterator::ComparisonResult result;
  result.LeftCount = 0;
  result.RightCount = 0;
  result.AreElementsEqual = true;   // Until proven otherwise.
  result.CanConvertElements = true; // Until proven otherwise.

  while (leftIter.hasCurrentElement() && rightIter.hasCurrentElement()) {
    Expr *actualExpr = rightIter.getExprOrNull();
    bool hasExpr = actualExpr != nullptr;
    StmtExpr scratchExpr(nullptr, rightIter.getCurrentElement(), NoLoc, NoLoc);
    StandardConversionSequence standard;
    ExprResult convertedExpr;
    if (!source.CanConvert(loc, hasExpr ? actualExpr : &scratchExpr,
                           leftIter.getCurrentElement(),
                           ExplicitConversionFalse, nullptr, &standard)) {
      result.AreElementsEqual = false;
      result.CanConvertElements = false;
      break;
    } else if (hasExpr && (standard.First != ICK_Identity ||
                           !standard.isIdentityConversion())) {
      convertedExpr = source.getSema()->PerformImplicitConversion(
          actualExpr, leftIter.getCurrentElement(), standard, Sema::AA_Casting,
          Sema::CCK_ImplicitConversion);
    }

    if (rightIter.getCurrentElement()->getCanonicalTypeUnqualified() !=
        leftIter.getCurrentElement()->getCanonicalTypeUnqualified()) {
      result.AreElementsEqual = false;
    }

    unsigned int advance = std::min(leftIter.getCurrentElementSize(),
                                    rightIter.getCurrentElementSize());
    DXASSERT(advance > 0, "otherwise one iterator should report empty");

    // If we need to apply conversions to the expressions, then advance a single
    // element.
    if (hasExpr && convertedExpr.isUsable()) {
      rightIter.replaceExpr(convertedExpr.get());
      advance = 1;
    }

    // If both elements are unbound arrays, break out or we'll never finish
    if (leftIter.getCurrentElementKind() == FK_IncompleteArray &&
        rightIter.getCurrentElementKind() == FK_IncompleteArray)
      break;

    leftIter.advanceCurrentElement(advance);
    rightIter.advanceCurrentElement(advance);
    result.LeftCount += advance;
    result.RightCount += advance;
  }

  result.LeftCount += leftIter.countRemaining();
  result.RightCount += rightIter.countRemaining();

  return result;
}

FlattenedTypeIterator::ComparisonResult FlattenedTypeIterator::CompareTypes(
    HLSLExternalSource &source, SourceLocation leftLoc, SourceLocation rightLoc,
    QualType left, QualType right) {
  FlattenedTypeIterator leftIter(leftLoc, left, source);
  FlattenedTypeIterator rightIter(rightLoc, right, source);

  return CompareIterators(source, leftLoc, leftIter, rightIter);
}

FlattenedTypeIterator::ComparisonResult
FlattenedTypeIterator::CompareTypesForInit(HLSLExternalSource &source,
                                           QualType left, MultiExprArg args,
                                           SourceLocation leftLoc,
                                           SourceLocation rightLoc) {
  FlattenedTypeIterator leftIter(leftLoc, left, source);
  FlattenedTypeIterator rightIter(rightLoc, args, source);

  return CompareIterators(source, leftLoc, leftIter, rightIter);
}

////////////////////////////////////////////////////////////////////////////////
// Attribute processing support.                                              //

static int ValidateAttributeIntArg(Sema &S, const AttributeList &Attr,
                                   unsigned index = 0) {
  int64_t value = 0;

  if (Attr.getNumArgs() > index) {
    Expr *E = nullptr;
    if (!Attr.isArgExpr(index)) {
      // For case arg is constant variable.
      IdentifierLoc *loc = Attr.getArgAsIdent(index);

      VarDecl *decl = dyn_cast_or_null<VarDecl>(
          S.LookupSingleName(S.getCurScope(), loc->Ident, loc->Loc,
                             Sema::LookupNameKind::LookupOrdinaryName));
      if (!decl) {
        S.Diag(Attr.getLoc(), diag::warn_hlsl_attribute_expects_uint_literal)
            << Attr.getName();
        return value;
      }
      Expr *init = decl->getInit();
      if (!init) {
        S.Diag(Attr.getLoc(), diag::warn_hlsl_attribute_expects_uint_literal)
            << Attr.getName();
        return value;
      }
      E = init;
    } else
      E = Attr.getArgAsExpr(index);

    clang::APValue ArgNum;
    bool displayError = false;
    if (E->isTypeDependent() || E->isValueDependent() ||
        !E->isCXX11ConstantExpr(S.Context, &ArgNum)) {
      displayError = true;
    } else {
      if (ArgNum.isInt()) {
        value = ArgNum.getInt().getSExtValue();
        if (!(E->getType()->isIntegralOrEnumerationType()) || value < 0) {
          S.Diag(Attr.getLoc(), diag::warn_hlsl_attribute_expects_uint_literal)
              << Attr.getName();
        }
      } else if (ArgNum.isFloat()) {
        llvm::APSInt floatInt;
        bool isPrecise;
        if (ArgNum.getFloat().convertToInteger(
                floatInt, llvm::APFloat::rmTowardZero, &isPrecise) ==
            llvm::APFloat::opStatus::opOK) {
          value = floatInt.getSExtValue();
          if (value < 0) {
            S.Diag(Attr.getLoc(),
                   diag::warn_hlsl_attribute_expects_uint_literal)
                << Attr.getName();
          }
        } else {
          S.Diag(Attr.getLoc(), diag::warn_hlsl_attribute_expects_uint_literal)
              << Attr.getName();
        }
      } else {
        displayError = true;
      }
    }

    if (displayError) {
      S.Diag(Attr.getLoc(), diag::err_attribute_argument_type)
          << Attr.getName() << AANT_ArgumentIntegerConstant
          << E->getSourceRange();
    }
  }

  return (int)value;
}

// TODO: support float arg directly.
static int ValidateAttributeFloatArg(Sema &S, const AttributeList &Attr,
                                     unsigned index = 0) {
  int value = 0;
  if (Attr.getNumArgs() > index) {
    Expr *E = Attr.getArgAsExpr(index);

    if (FloatingLiteral *FL = dyn_cast<FloatingLiteral>(E)) {
      llvm::APFloat flV = FL->getValue();
      if (flV.getSizeInBits(flV.getSemantics()) == 64) {
        llvm::APInt intV = llvm::APInt::floatToBits(flV.convertToDouble());
        value = intV.getLimitedValue();
      } else {
        llvm::APInt intV = llvm::APInt::floatToBits(flV.convertToFloat());
        value = intV.getLimitedValue();
      }
    } else if (IntegerLiteral *IL = dyn_cast<IntegerLiteral>(E)) {
      llvm::APInt intV =
          llvm::APInt::floatToBits((float)IL->getValue().getLimitedValue());
      value = intV.getLimitedValue();
    } else {
      S.Diag(E->getLocStart(), diag::err_hlsl_attribute_expects_float_literal)
          << Attr.getName();
    }
  }
  return value;
}

template <typename AttrType, typename EnumType,
          bool (*ConvertStrToEnumType)(StringRef, EnumType &)>
static EnumType ValidateAttributeEnumArg(Sema &S, const AttributeList &Attr,
                                         EnumType defaultValue,
                                         unsigned index = 0,
                                         bool isCaseSensitive = true) {
  EnumType value(defaultValue);
  StringRef Str = "";
  SourceLocation ArgLoc;

  if (Attr.getNumArgs() > index) {
    if (!S.checkStringLiteralArgumentAttr(Attr, 0, Str, &ArgLoc))
      return value;

    std::string str = isCaseSensitive ? Str.str() : Str.lower();

    if (!ConvertStrToEnumType(str, value)) {
      S.Diag(Attr.getLoc(), diag::warn_attribute_type_not_supported)
          << Attr.getName() << Str << ArgLoc;
    }
    return value;
  }
  return value;
}

static Stmt *IgnoreParensAndDecay(Stmt *S) {
  for (;;) {
    switch (S->getStmtClass()) {
    case Stmt::ParenExprClass:
      S = cast<ParenExpr>(S)->getSubExpr();
      break;
    case Stmt::ImplicitCastExprClass: {
      ImplicitCastExpr *castExpr = cast<ImplicitCastExpr>(S);
      if (castExpr->getCastKind() != CK_ArrayToPointerDecay &&
          castExpr->getCastKind() != CK_NoOp &&
          castExpr->getCastKind() != CK_LValueToRValue) {
        return S;
      }
      S = castExpr->getSubExpr();
    } break;
    default:
      return S;
    }
  }
}

static Expr *ValidateClipPlaneArraySubscriptExpr(Sema &S,
                                                 ArraySubscriptExpr *E) {
  DXASSERT_NOMSG(E != nullptr);

  Expr *subscriptExpr = E->getIdx();
  subscriptExpr = dyn_cast<Expr>(subscriptExpr->IgnoreParens());
  if (subscriptExpr == nullptr || subscriptExpr->isTypeDependent() ||
      subscriptExpr->isValueDependent() ||
      !subscriptExpr->isCXX11ConstantExpr(S.Context)) {
    S.Diag((subscriptExpr == nullptr) ? E->getLocStart()
                                      : subscriptExpr->getLocStart(),
           diag::err_hlsl_unsupported_clipplane_argument_subscript_expression);
    return nullptr;
  }

  return E->getBase();
}

static bool IsValidClipPlaneDecl(Decl *D) {
  Decl::Kind kind = D->getKind();
  if (kind == Decl::Var) {
    VarDecl *varDecl = cast<VarDecl>(D);
    if (varDecl->getStorageClass() == StorageClass::SC_Static &&
        varDecl->getType().isConstQualified()) {
      return false;
    }

    return true;
  } else if (kind == Decl::Field) {
    return true;
  }
  return false;
}

static Expr *ValidateClipPlaneExpr(Sema &S, Expr *E) {
  Stmt *cursor = E;

  // clip plane expressions are a linear path, so no need to traverse the tree
  // here.
  while (cursor != nullptr) {
    bool supported = true;
    cursor = IgnoreParensAndDecay(cursor);
    switch (cursor->getStmtClass()) {
    case Stmt::ArraySubscriptExprClass:
      cursor = ValidateClipPlaneArraySubscriptExpr(
          S, cast<ArraySubscriptExpr>(cursor));
      if (cursor == nullptr) {
        // nullptr indicates failure, and the error message has already been
        // printed out
        return nullptr;
      }
      break;
    case Stmt::DeclRefExprClass: {
      DeclRefExpr *declRef = cast<DeclRefExpr>(cursor);
      Decl *decl = declRef->getDecl();
      supported = IsValidClipPlaneDecl(decl);
      cursor = supported ? nullptr : cursor;
    } break;
    case Stmt::MemberExprClass: {
      MemberExpr *member = cast<MemberExpr>(cursor);
      supported = IsValidClipPlaneDecl(member->getMemberDecl());
      cursor = supported ? member->getBase() : cursor;
    } break;
    default:
      supported = false;
      break;
    }

    if (!supported) {
      DXASSERT(
          cursor != nullptr,
          "otherwise it was cleared when the supported flag was set to false");
      S.Diag(cursor->getLocStart(),
             diag::err_hlsl_unsupported_clipplane_argument_expression);
      return nullptr;
    }
  }

  // Validate that the type is a float4.
  QualType expressionType = E->getType();
  HLSLExternalSource *hlslSource = HLSLExternalSource::FromSema(&S);
  if (hlslSource->GetTypeElementKind(expressionType) !=
          ArBasicKind::AR_BASIC_FLOAT32 ||
      hlslSource->GetTypeObjectKind(expressionType) !=
          ArTypeObjectKind::AR_TOBJ_VECTOR) {
    S.Diag(E->getLocStart(), diag::err_hlsl_unsupported_clipplane_argument_type)
        << expressionType;
    return nullptr;
  }

  return E;
}

static Attr *HandleClipPlanes(Sema &S, const AttributeList &A) {
  Expr *clipExprs[6];
  for (unsigned int index = 0; index < _countof(clipExprs); index++) {
    if (A.getNumArgs() <= index) {
      clipExprs[index] = nullptr;
      continue;
    }

    Expr *E = A.getArgAsExpr(index);
    clipExprs[index] = ValidateClipPlaneExpr(S, E);
  }

  return ::new (S.Context)
      HLSLClipPlanesAttr(A.getRange(), S.Context, clipExprs[0], clipExprs[1],
                         clipExprs[2], clipExprs[3], clipExprs[4], clipExprs[5],
                         A.getAttributeSpellingListIndex());
}

static Attr *HandleUnrollAttribute(Sema &S, const AttributeList &Attr) {
  int argValue = ValidateAttributeIntArg(S, Attr);
  // Default value is 0 (full unroll).
  if (Attr.getNumArgs() == 0)
    argValue = 0;
  return ::new (S.Context) HLSLUnrollAttr(Attr.getRange(), S.Context, argValue,
                                          Attr.getAttributeSpellingListIndex());
}

static void ValidateAttributeOnLoop(Sema &S, Stmt *St,
                                    const AttributeList &Attr) {
  Stmt::StmtClass stClass = St->getStmtClass();
  if (stClass != Stmt::ForStmtClass && stClass != Stmt::WhileStmtClass &&
      stClass != Stmt::DoStmtClass) {
    S.Diag(Attr.getLoc(),
           diag::warn_hlsl_unsupported_statement_for_loop_attribute)
        << Attr.getName();
  }
}

static void ValidateAttributeOnSwitch(Sema &S, Stmt *St,
                                      const AttributeList &Attr) {
  Stmt::StmtClass stClass = St->getStmtClass();
  if (stClass != Stmt::SwitchStmtClass) {
    S.Diag(Attr.getLoc(),
           diag::warn_hlsl_unsupported_statement_for_switch_attribute)
        << Attr.getName();
  }
}

static void ValidateAttributeOnSwitchOrIf(Sema &S, Stmt *St,
                                          const AttributeList &Attr) {
  Stmt::StmtClass stClass = St->getStmtClass();
  if (stClass != Stmt::SwitchStmtClass && stClass != Stmt::IfStmtClass) {
    S.Diag(Attr.getLoc(),
           diag::warn_hlsl_unsupported_statement_for_if_switch_attribute)
        << Attr.getName();
  }
}

static StringRef ValidateAttributeStringArg(Sema &S, const AttributeList &A,
                                            const char *values,
                                            unsigned index = 0) {

  // values is an optional comma-separated list of potential values.
  if (A.getNumArgs() <= index)
    return StringRef();

  Expr *E = A.getArgAsExpr(index);
  if (E->isTypeDependent() || E->isValueDependent() ||
      E->getStmtClass() != Stmt::StringLiteralClass) {
    S.Diag(E->getLocStart(), diag::err_hlsl_attribute_expects_string_literal)
        << A.getName();
    return StringRef();
  }

  StringLiteral *sl = cast<StringLiteral>(E);
  StringRef result = sl->getString();

  // Return result with no additional validation.
  if (values == nullptr) {
    return result;
  }

  const char *value = values;
  while (*value != '\0') {
    DXASSERT_NOMSG(*value != ','); // no leading commas in values

    // Look for a match.
    const char *argData = result.data();
    size_t argDataLen = result.size();

    while (argDataLen != 0 && *argData == *value && *value) {
      ++argData;
      ++value;
      --argDataLen;
    }

    // Match found if every input character matched.
    if (argDataLen == 0 && (*value == '\0' || *value == ',')) {
      return result;
    }

    // Move to next separator.
    while (*value != '\0' && *value != ',') {
      ++value;
    }

    // Move to the start of the next item if any.
    if (*value == ',')
      value++;
  }

  DXASSERT_NOMSG(*value == '\0'); // no other terminating conditions

  // No match found.
  S.Diag(E->getLocStart(),
         diag::err_hlsl_attribute_expects_string_literal_from_list)
      << A.getName() << values;
  return StringRef();
}

static bool ValidateAttributeTargetIsFunction(Sema &S, Decl *D,
                                              const AttributeList &A) {
  if (D->isFunctionOrFunctionTemplate()) {
    return true;
  }

  S.Diag(A.getLoc(), diag::err_hlsl_attribute_valid_on_function_only);
  return false;
}

HLSLShaderAttr *ValidateShaderAttributes(Sema &S, Decl *D,
                                         const AttributeList &A) {
  Expr *ArgExpr = A.getArgAsExpr(0);
  StringLiteral *Literal = dyn_cast<StringLiteral>(ArgExpr->IgnoreParenCasts());
  DXIL::ShaderKind Stage = ShaderModel::KindFromFullName(Literal->getString());
  if (Stage == DXIL::ShaderKind::Invalid) {
    S.Diag(A.getLoc(),
           diag::err_hlsl_attribute_expects_string_literal_from_list)
        << "'shader'"
        << "compute,vertex,pixel,hull,domain,geometry,raygeneration,"
           "intersection,anyhit,closesthit,miss,callable,mesh,"
           "amplification,node";
    return nullptr; // don't create the attribute
  }

  HLSLShaderAttr *Existing = D->getAttr<HLSLShaderAttr>();
  if (Existing) {
    DXIL::ShaderKind NewStage =
        ShaderModel::KindFromFullName(Existing->getStage());
    if (Stage == NewStage)
      return nullptr; // don't create, but no error.
    else {
      S.Diag(A.getLoc(), diag::err_hlsl_conflicting_shader_attribute)
          << ShaderModel::FullNameFromKind(Stage)
          << ShaderModel::FullNameFromKind(NewStage);
      S.Diag(Existing->getLocation(), diag::note_conflicting_attribute);
      return nullptr;
    }
  }
  return ::new (S.Context)
      HLSLShaderAttr(A.getRange(), S.Context, Literal->getString(),
                     A.getAttributeSpellingListIndex());
}

HLSLMaxRecordsAttr *ValidateMaxRecordsAttributes(Sema &S, Decl *D,
                                                 const AttributeList &A) {

  HLSLMaxRecordsAttr *ExistingMRA = D->getAttr<HLSLMaxRecordsAttr>();
  HLSLMaxRecordsSharedWithAttr *ExistingMRSWA =
      D->getAttr<HLSLMaxRecordsSharedWithAttr>();

  if (ExistingMRA || ExistingMRSWA) {
    Expr *ArgExpr = A.getArgAsExpr(0);
    IntegerLiteral *LiteralInt =
        dyn_cast<IntegerLiteral>(ArgExpr->IgnoreParenCasts());

    if (ExistingMRSWA || ExistingMRA->getMaxCount() != LiteralInt->getValue()) {
      clang::SourceLocation Loc = ExistingMRA ? ExistingMRA->getLocation()
                                              : ExistingMRSWA->getLocation();
      S.Diag(A.getLoc(), diag::err_hlsl_maxrecord_attrs_on_same_arg);
      S.Diag(Loc, diag::note_conflicting_attribute);
      return nullptr;
    }
  }

  return ::new (S.Context)
      HLSLMaxRecordsAttr(A.getRange(), S.Context, ValidateAttributeIntArg(S, A),
                         A.getAttributeSpellingListIndex());
}

// This function validates the wave size attribute in a stand-alone way,
// by directly determining whether the attribute is well formed or
// allowed. It performs validation outside of the context
// of other attributes that could exist on this decl, and immediately
// upon detecting the attribute on the decl.
HLSLWaveSizeAttr *ValidateWaveSizeAttributes(Sema &S, Decl *D,
                                             const AttributeList &A) {
  // validate that the wavesize argument is a power of 2 between 4 and 128
  // inclusive
  HLSLWaveSizeAttr *pAttr = ::new (S.Context) HLSLWaveSizeAttr(
      A.getRange(), S.Context, ValidateAttributeIntArg(S, A, 0),
      ValidateAttributeIntArg(S, A, 1), ValidateAttributeIntArg(S, A, 2),
      A.getAttributeSpellingListIndex());

  pAttr->setSpelledArgsCount(A.getNumArgs());

  hlsl::DxilWaveSize waveSize(pAttr->getMin(), pAttr->getMax(),
                              pAttr->getPreferred());

  DxilWaveSize::ValidationResult validationResult = waveSize.Validate();

  // WaveSize validation succeeds when not defined, but since we have an
  // attribute, this means min was zero, which is invalid for min.
  if (validationResult == DxilWaveSize::ValidationResult::Success &&
      !waveSize.IsDefined())
    validationResult = DxilWaveSize::ValidationResult::InvalidMin;

  // It is invalid to explicitly specify degenerate cases.
  if (A.getNumArgs() > 1 && waveSize.Max == 0)
    validationResult = DxilWaveSize::ValidationResult::InvalidMax;
  else if (A.getNumArgs() > 2 && waveSize.Preferred == 0)
    validationResult = DxilWaveSize::ValidationResult::InvalidPreferred;

  switch (validationResult) {
  case DxilWaveSize::ValidationResult::Success:
    break;
  case DxilWaveSize::ValidationResult::InvalidMin:
  case DxilWaveSize::ValidationResult::InvalidMax:
  case DxilWaveSize::ValidationResult::InvalidPreferred:
  case DxilWaveSize::ValidationResult::NoRangeOrMin:
    S.Diag(A.getLoc(), diag::err_hlsl_wavesize_size)
        << DXIL::kMinWaveSize << DXIL::kMaxWaveSize;
    break;
  case DxilWaveSize::ValidationResult::MaxEqualsMin:
    S.Diag(A.getLoc(), diag::warn_hlsl_wavesize_min_eq_max)
        << (unsigned)waveSize.Min << (unsigned)waveSize.Max;
    break;
  case DxilWaveSize::ValidationResult::MaxLessThanMin:
    S.Diag(A.getLoc(), diag::err_hlsl_wavesize_min_geq_max)
        << (unsigned)waveSize.Min << (unsigned)waveSize.Max;
    break;
  case DxilWaveSize::ValidationResult::PreferredOutOfRange:
    S.Diag(A.getLoc(), diag::err_hlsl_wavesize_pref_size_out_of_range)
        << (unsigned)waveSize.Preferred << (unsigned)waveSize.Min
        << (unsigned)waveSize.Max;
    break;
  case DxilWaveSize::ValidationResult::MaxOrPreferredWhenUndefined:
  case DxilWaveSize::ValidationResult::PreferredWhenNoRange:
    llvm_unreachable("Should have hit InvalidMax or InvalidPreferred instead.");
    break;
  default:
    llvm_unreachable("Unknown ValidationResult");
  }

  // make sure there is not already an existing conflicting
  // wavesize attribute on the decl
  HLSLWaveSizeAttr *waveSizeAttr = D->getAttr<HLSLWaveSizeAttr>();
  if (waveSizeAttr) {
    if (waveSizeAttr->getMin() != pAttr->getMin() ||
        waveSizeAttr->getMax() != pAttr->getMax() ||
        waveSizeAttr->getPreferred() != pAttr->getPreferred()) {
      S.Diag(A.getLoc(), diag::err_hlsl_conflicting_shader_attribute)
          << pAttr->getSpelling() << waveSizeAttr->getSpelling();
      S.Diag(waveSizeAttr->getLocation(), diag::note_conflicting_attribute);
    }
  }
  return pAttr;
}

HLSLMaxRecordsSharedWithAttr *
ValidateMaxRecordsSharedWithAttributes(Sema &S, Decl *D,
                                       const AttributeList &A) {

  if (!A.isArgIdent(0)) {
    S.Diag(A.getLoc(), diag::err_attribute_argument_n_type)
        << A.getName() << 1 << AANT_ArgumentIdentifier;
    return nullptr;
  }

  IdentifierInfo *II = A.getArgAsIdent(0)->Ident;
  StringRef sharedName = II->getName();

  HLSLMaxRecordsAttr *ExistingMRA = D->getAttr<HLSLMaxRecordsAttr>();
  HLSLMaxRecordsSharedWithAttr *ExistingMRSWA =
      D->getAttr<HLSLMaxRecordsSharedWithAttr>();

  ParmVarDecl *pPVD = cast<ParmVarDecl>(D);
  StringRef ArgName = pPVD->getName();

  // check that this is the only MaxRecords* attribute for this parameter
  if (ExistingMRA || ExistingMRSWA) {
    // only emit a diagnostic if the argument to the attribute differs from the
    // current attribute when an extra MRSWA attribute is attached to this
    // parameter
    if (ExistingMRA ||
        sharedName !=
            ExistingMRSWA->getName()
                ->getName()) { // won't null deref, because short-circuit
      clang::SourceLocation Loc = ExistingMRA ? ExistingMRA->getLocation()
                                              : ExistingMRSWA->getLocation();
      S.Diag(A.getLoc(), diag::err_hlsl_maxrecord_attrs_on_same_arg);
      S.Diag(Loc, diag::note_conflicting_attribute);
      return nullptr;
    }
  }

  // check that the parameter that MaxRecordsSharedWith is targeting isn't
  // applied to that exact parameter
  if (sharedName == ArgName) {
    S.Diag(A.getLoc(), diag::err_hlsl_maxrecordssharedwith_references_itself);
    return nullptr;
  }

  return ::new (S.Context) HLSLMaxRecordsSharedWithAttr(
      A.getRange(), S.Context, II, A.getAttributeSpellingListIndex());
}

void Sema::DiagnoseHLSLDeclAttr(const Decl *D, const Attr *A) {
  HLSLExternalSource *ExtSource = HLSLExternalSource::FromSema(this);
  const bool IsGCAttr = isa<HLSLGloballyCoherentAttr>(A);
  const bool IsRCAttr = isa<HLSLReorderCoherentAttr>(A);
  if (IsGCAttr || IsRCAttr) {
    const ValueDecl *TD = cast<ValueDecl>(D);
    if (TD->getType()->isDependentType())
      return;
    QualType DeclType = TD->getType();
    if (const FunctionDecl *FD = dyn_cast<FunctionDecl>(TD))
      DeclType = FD->getReturnType();
    while (DeclType->isArrayType())
      DeclType = QualType(DeclType->getArrayElementTypeNoTypeQual(), 0);
    const bool IsAllowedNodeIO =
        IsGCAttr &&
        GetNodeIOType(DeclType) == DXIL::NodeIOKind::RWDispatchNodeInputRecord;
    const bool IsUAV =
        hlsl::GetResourceClassForType(getASTContext(), DeclType) ==
        hlsl::DXIL::ResourceClass::UAV;
    if (ExtSource->GetTypeObjectKind(DeclType) != AR_TOBJ_OBJECT ||
        (!IsUAV && !IsAllowedNodeIO)) {
      Diag(A->getLocation(), diag::err_hlsl_varmodifierna_decltype)
          << A << DeclType->getCanonicalTypeUnqualified() << A->getRange();
      Diag(A->getLocation(), diag::note_hlsl_coherence_applies_to)
          << (int)IsGCAttr << A << A->getRange();
    }
    return;
  }
}

void Sema::DiagnoseCoherenceMismatch(const Expr *SrcExpr, QualType TargetType,
                                     SourceLocation Loc) {
  QualType SrcTy = SrcExpr->getType();
  QualType DstTy = TargetType;
  if (SrcTy->isArrayType() && DstTy->isArrayType()) {
    SrcTy = QualType(SrcTy->getBaseElementTypeUnsafe(), 0);
    DstTy = QualType(DstTy->getBaseElementTypeUnsafe(), 0);
  }
  if ((hlsl::IsHLSLResourceType(DstTy) &&
       !hlsl::IsHLSLDynamicResourceType(SrcTy)) ||
      GetNodeIOType(DstTy) == DXIL::NodeIOKind::RWDispatchNodeInputRecord) {
    bool SrcGL = hlsl::HasHLSLGloballyCoherent(SrcTy);
    bool DstGL = hlsl::HasHLSLGloballyCoherent(DstTy);
    // 'reordercoherent' attribute dropped earlier in presence of
    // 'globallycoherent'
    bool SrcRD = hlsl::HasHLSLReorderCoherent(SrcTy);
    bool DstRD = hlsl::HasHLSLReorderCoherent(DstTy);

    enum {
      NoMismatch = -1,
      DemoteToRD = 0,
      PromoteToGL = 1,
      LosesRD = 2,
      LosesGL = 3,
      AddsRD = 4,
      AddsGL = 5
    } MismatchType = NoMismatch;

    if (SrcGL && DstRD)
      MismatchType = DemoteToRD;
    else if (SrcRD && DstGL)
      MismatchType = PromoteToGL;
    else if (SrcRD && !DstRD)
      MismatchType = LosesRD;
    else if (SrcGL && !DstGL)
      MismatchType = LosesGL;
    else if (!SrcRD && DstRD)
      MismatchType = AddsRD;
    else if (!SrcGL && DstGL)
      MismatchType = AddsGL;

    if (MismatchType == NoMismatch)
      return;

    Diag(Loc, diag::warn_hlsl_impcast_coherence_mismatch)
        << SrcExpr->getType() << TargetType << MismatchType;
  }
}

void ValidateDispatchGridValues(DiagnosticsEngine &Diags,
                                const AttributeList &A, Attr *declAttr) {
  unsigned x = 1, y = 1, z = 1;
  if (HLSLNodeDispatchGridAttr *pA =
          dyn_cast<HLSLNodeDispatchGridAttr>(declAttr)) {
    x = pA->getX();
    y = pA->getY();
    z = pA->getZ();
  } else if (HLSLNodeMaxDispatchGridAttr *pA =
                 dyn_cast<HLSLNodeMaxDispatchGridAttr>(declAttr)) {
    x = pA->getX();
    y = pA->getY();
    z = pA->getZ();
  } else {
    llvm_unreachable("ValidateDispatchGridValues() called for wrong attribute");
  }
  static const unsigned MaxComponentValue = 65535;  // 2^16 - 1
  static const unsigned MaxProductValue = 16777215; // 2^24 - 1
  // If a component is out of range, we reset it to 0 to avoid also generating
  // a secondary error if the product would be out of range
  if (x < 1 || x > MaxComponentValue) {
    Diags.Report(A.getArgAsExpr(0)->getExprLoc(),
                 diag::err_hlsl_dispatchgrid_component)
        << A.getName() << "X" << A.getRange();
    x = 0;
  }
  if (y < 1 || y > MaxComponentValue) {
    Diags.Report(A.getArgAsExpr(1)->getExprLoc(),
                 diag::err_hlsl_dispatchgrid_component)
        << A.getName() << "Y" << A.getRange();
    y = 0;
  }
  if (z < 1 || z > MaxComponentValue) {
    Diags.Report(A.getArgAsExpr(2)->getExprLoc(),
                 diag::err_hlsl_dispatchgrid_component)
        << A.getName() << "Z" << A.getRange();
    z = 0;
  }
  uint64_t product = (uint64_t)x * (uint64_t)y * (uint64_t)z;
  if (product > MaxProductValue)
    Diags.Report(A.getLoc(), diag::err_hlsl_dispatchgrid_product)
        << A.getName() << A.getRange();
}

void hlsl::HandleDeclAttributeForHLSL(Sema &S, Decl *D, const AttributeList &A,
                                      bool &Handled) {
  DXASSERT_NOMSG(D != nullptr);
  DXASSERT_NOMSG(!A.isInvalid());

  Attr *declAttr = nullptr;
  Handled = true;
  switch (A.getKind()) {
  case AttributeList::AT_HLSLIn:
    declAttr = ::new (S.Context)
        HLSLInAttr(A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_HLSLOut:
    declAttr = ::new (S.Context)
        HLSLOutAttr(A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_HLSLInOut:
    declAttr = ::new (S.Context) HLSLInOutAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_HLSLMaybeUnused:
    declAttr = ::new (S.Context) HLSLMaybeUnusedAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;

  case AttributeList::AT_HLSLNoInterpolation:
    declAttr = ::new (S.Context) HLSLNoInterpolationAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_HLSLLinear:
  case AttributeList::AT_HLSLCenter:
    declAttr = ::new (S.Context) HLSLLinearAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_HLSLNoPerspective:
    declAttr = ::new (S.Context) HLSLNoPerspectiveAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_HLSLSample:
    declAttr = ::new (S.Context) HLSLSampleAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_HLSLCentroid:
    declAttr = ::new (S.Context) HLSLCentroidAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;

  case AttributeList::AT_HLSLPrecise:
    declAttr = ::new (S.Context) HLSLPreciseAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_HLSLShared:
    declAttr = ::new (S.Context) HLSLSharedAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_HLSLGroupShared:
    declAttr = ::new (S.Context) HLSLGroupSharedAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    if (VarDecl *VD = dyn_cast<VarDecl>(D)) {
      VD->setType(
          S.Context.getAddrSpaceQualType(VD->getType(), DXIL::kTGSMAddrSpace));
    }
    break;
  case AttributeList::AT_HLSLUniform:
    declAttr = ::new (S.Context) HLSLUniformAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;

  case AttributeList::AT_HLSLUnorm:
    declAttr = ::new (S.Context) HLSLUnormAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_HLSLSnorm:
    declAttr = ::new (S.Context) HLSLSnormAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;

  case AttributeList::AT_HLSLPoint:
    declAttr = ::new (S.Context) HLSLPointAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_HLSLLine:
    declAttr = ::new (S.Context) HLSLLineAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_HLSLLineAdj:
    declAttr = ::new (S.Context) HLSLLineAdjAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_HLSLTriangle:
    declAttr = ::new (S.Context) HLSLTriangleAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_HLSLTriangleAdj:
    declAttr = ::new (S.Context) HLSLTriangleAdjAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_HLSLGloballyCoherent:
    declAttr = ::new (S.Context) HLSLGloballyCoherentAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_HLSLReorderCoherent:
    declAttr = ::new (S.Context) HLSLReorderCoherentAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_HLSLIndices:
    declAttr = ::new (S.Context) HLSLIndicesAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_HLSLVertices:
    declAttr = ::new (S.Context) HLSLVerticesAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_HLSLPrimitives:
    declAttr = ::new (S.Context) HLSLPrimitivesAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_HLSLPayload:
    declAttr = ::new (S.Context) HLSLPayloadAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_HLSLRayPayload:
    declAttr = ::new (S.Context) HLSLRayPayloadAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_HLSLMaxRecords:
    declAttr = ValidateMaxRecordsAttributes(S, D, A);
    if (!declAttr) {
      return;
    }

    break;
  case AttributeList::AT_HLSLMaxRecordsSharedWith: {
    declAttr = ValidateMaxRecordsSharedWithAttributes(S, D, A);
    if (!declAttr) {
      return;
    }
    break;
  }
  case AttributeList::AT_HLSLNodeArraySize: {
    declAttr = ::new (S.Context) HLSLNodeArraySizeAttr(
        A.getRange(), S.Context, ValidateAttributeIntArg(S, A),
        A.getAttributeSpellingListIndex());
    break;
  }
  case AttributeList::AT_HLSLAllowSparseNodes:
    declAttr = ::new (S.Context) HLSLAllowSparseNodesAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_HLSLUnboundedSparseNodes:
    declAttr = ::new (S.Context) HLSLUnboundedSparseNodesAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_HLSLNodeId:
    declAttr = ::new (S.Context) HLSLNodeIdAttr(
        A.getRange(), S.Context, ValidateAttributeStringArg(S, A, nullptr, 0),
        ValidateAttributeIntArg(S, A, 1), A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_HLSLNodeTrackRWInputSharing:
    declAttr = ::new (S.Context) HLSLNodeTrackRWInputSharingAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  // SPIRV Change Starts
  case AttributeList::AT_VKAliasedPointer: {
    declAttr = ::new (S.Context) VKAliasedPointerAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
  } break;
  case AttributeList::AT_VKDecorateIdExt: {
    if (A.getNumArgs() == 0 || !A.getArg(0).is<clang::Expr *>()) {
      Handled = false;
      break;
    }

    unsigned decoration = 0;
    if (IntegerLiteral *decorationAsLiteral =
            dyn_cast<IntegerLiteral>(A.getArg(0).get<clang::Expr *>())) {
      decoration = decorationAsLiteral->getValue().getZExtValue();
    } else {
      Handled = false;
      break;
    }

    llvm::SmallVector<Expr *, 2> args;
    for (unsigned i = 1; i < A.getNumArgs(); ++i) {
      if (!A.getArg(i).is<clang::Expr *>()) {
        Handled = false;
        break;
      }
      args.push_back(A.getArg(i).get<clang::Expr *>());
    }
    if (!Handled)
      break;
    declAttr = ::new (S.Context)
        VKDecorateIdExtAttr(A.getRange(), S.Context, decoration, args.data(),
                            args.size(), A.getAttributeSpellingListIndex());
  } break;
    // SPIRV Change Ends

  default:
    Handled = false;
    break;
  }

  if (declAttr != nullptr) {
    S.DiagnoseHLSLDeclAttr(D, declAttr);
    DXASSERT_NOMSG(Handled);
    D->addAttr(declAttr);
    return;
  }

  Handled = true;
  switch (A.getKind()) {
  // These apply to statements, not declarations. The warning messages clarify
  // this properly.
  case AttributeList::AT_HLSLUnroll:
  case AttributeList::AT_HLSLAllowUAVCondition:
  case AttributeList::AT_HLSLLoop:
  case AttributeList::AT_HLSLFastOpt:
    S.Diag(A.getLoc(), diag::warn_hlsl_unsupported_statement_for_loop_attribute)
        << A.getName();
    return;
  case AttributeList::AT_HLSLBranch:
  case AttributeList::AT_HLSLFlatten:
    S.Diag(A.getLoc(),
           diag::warn_hlsl_unsupported_statement_for_if_switch_attribute)
        << A.getName();
    return;
  case AttributeList::AT_HLSLForceCase:
  case AttributeList::AT_HLSLCall:
    S.Diag(A.getLoc(),
           diag::warn_hlsl_unsupported_statement_for_switch_attribute)
        << A.getName();
    return;

  // These are the cases that actually apply to declarations.
  case AttributeList::AT_HLSLClipPlanes:
    declAttr = HandleClipPlanes(S, A);
    break;
  case AttributeList::AT_HLSLDomain:
    declAttr = ::new (S.Context)
        HLSLDomainAttr(A.getRange(), S.Context,
                       ValidateAttributeStringArg(S, A, "tri,quad,isoline"),
                       A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_HLSLEarlyDepthStencil:
    declAttr = ::new (S.Context) HLSLEarlyDepthStencilAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_HLSLInstance:
    declAttr = ::new (S.Context)
        HLSLInstanceAttr(A.getRange(), S.Context, ValidateAttributeIntArg(S, A),
                         A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_HLSLMaxTessFactor:
    declAttr = ::new (S.Context) HLSLMaxTessFactorAttr(
        A.getRange(), S.Context, ValidateAttributeFloatArg(S, A),
        A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_HLSLNumThreads: {
    int X = ValidateAttributeIntArg(S, A, 0);
    int Y = ValidateAttributeIntArg(S, A, 1);
    int Z = ValidateAttributeIntArg(S, A, 2);
    int N = X * Y * Z;
    if (N > 0 && N <= 1024) {
      auto numThreads = ::new (S.Context) HLSLNumThreadsAttr(
          A.getRange(), S.Context, X, Y, Z, A.getAttributeSpellingListIndex());
      declAttr = numThreads;
    } else {
      // If the number of threads is invalid, diagnose and drop the attribute.
      S.Diags.Report(A.getLoc(), diag::warn_hlsl_numthreads_group_size)
          << N << X << Y << Z << A.getRange();
      return;
    }
    break;
  }
  case AttributeList::AT_HLSLRootSignature:
    declAttr = ::new (S.Context) HLSLRootSignatureAttr(
        A.getRange(), S.Context,
        ValidateAttributeStringArg(S, A, /*validate strings*/ nullptr),
        A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_HLSLOutputControlPoints: {
    // Hull shader output must be between 1 and 32 control points.
    int outputControlPoints = ValidateAttributeIntArg(S, A);
    if (outputControlPoints < 1 || outputControlPoints > 32) {
      S.Diags.Report(A.getLoc(), diag::err_hlsl_controlpoints_size)
          << outputControlPoints << A.getRange();
      return;
    }
    declAttr = ::new (S.Context) HLSLOutputControlPointsAttr(
        A.getRange(), S.Context, outputControlPoints,
        A.getAttributeSpellingListIndex());
    break;
  }
  case AttributeList::AT_HLSLOutputTopology:
    declAttr = ::new (S.Context) HLSLOutputTopologyAttr(
        A.getRange(), S.Context,
        ValidateAttributeStringArg(
            S, A, "point,line,triangle,triangle_cw,triangle_ccw"),
        A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_HLSLPartitioning:
    declAttr = ::new (S.Context) HLSLPartitioningAttr(
        A.getRange(), S.Context,
        ValidateAttributeStringArg(
            S, A, "integer,fractional_even,fractional_odd,pow2"),
        A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_HLSLPatchConstantFunc:
    declAttr = ::new (S.Context) HLSLPatchConstantFuncAttr(
        A.getRange(), S.Context, ValidateAttributeStringArg(S, A, nullptr),
        A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_HLSLShader:
    declAttr = ValidateShaderAttributes(S, D, A);
    if (!declAttr) {
      Handled = true;
      return;
    }
    break;
  case AttributeList::AT_HLSLMaxVertexCount:
    declAttr = ::new (S.Context) HLSLMaxVertexCountAttr(
        A.getRange(), S.Context, ValidateAttributeIntArg(S, A),
        A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_HLSLExperimental:
    declAttr = ::new (S.Context) HLSLExperimentalAttr(
        A.getRange(), S.Context, ValidateAttributeStringArg(S, A, nullptr, 0),
        ValidateAttributeStringArg(S, A, nullptr, 1),
        A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_NoInline:
    declAttr = ::new (S.Context) NoInlineAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_HLSLExport:
    declAttr = ::new (S.Context) HLSLExportAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_HLSLWaveSensitive:
    declAttr = ::new (S.Context) HLSLWaveSensitiveAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_HLSLWaveSize:
    declAttr = ValidateWaveSizeAttributes(S, D, A);
    break;
  case AttributeList::AT_HLSLWaveOpsIncludeHelperLanes:
    declAttr = ::new (S.Context) HLSLWaveOpsIncludeHelperLanesAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_HLSLNodeLaunch:
    declAttr = ::new (S.Context) HLSLNodeLaunchAttr(
        A.getRange(), S.Context,
        ValidateAttributeStringArg(S, A, "broadcasting,coalescing,thread"),
        A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_HLSLNodeIsProgramEntry:
    declAttr = ::new (S.Context) HLSLNodeIsProgramEntryAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_HLSLNodeTrackRWInputSharing:
    declAttr = ::new (S.Context) HLSLNodeTrackRWInputSharingAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_HLSLNodeLocalRootArgumentsTableIndex:
    declAttr = ::new (S.Context) HLSLNodeLocalRootArgumentsTableIndexAttr(
        A.getRange(), S.Context, ValidateAttributeIntArg(S, A),
        A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_HLSLNodeShareInputOf:
    declAttr = ::new (S.Context) HLSLNodeShareInputOfAttr(
        A.getRange(), S.Context, ValidateAttributeStringArg(S, A, nullptr, 0),
        ValidateAttributeIntArg(S, A, 1), A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_HLSLNodeDispatchGrid:
    declAttr = ::new (S.Context) HLSLNodeDispatchGridAttr(
        A.getRange(), S.Context, ValidateAttributeIntArg(S, A),
        ValidateAttributeIntArg(S, A, 1), ValidateAttributeIntArg(S, A, 2),
        A.getAttributeSpellingListIndex());
    ValidateDispatchGridValues(S.Diags, A, declAttr);
    break;
  case AttributeList::AT_HLSLNodeMaxDispatchGrid:
    declAttr = ::new (S.Context) HLSLNodeMaxDispatchGridAttr(
        A.getRange(), S.Context, ValidateAttributeIntArg(S, A),
        ValidateAttributeIntArg(S, A, 1), ValidateAttributeIntArg(S, A, 2),
        A.getAttributeSpellingListIndex());
    ValidateDispatchGridValues(S.Diags, A, declAttr);
    break;
  case AttributeList::AT_HLSLNodeMaxRecursionDepth:
    declAttr = ::new (S.Context) HLSLNodeMaxRecursionDepthAttr(
        A.getRange(), S.Context, ValidateAttributeIntArg(S, A),
        A.getAttributeSpellingListIndex());
    if (cast<HLSLNodeMaxRecursionDepthAttr>(declAttr)->getCount() > 32)
      S.Diags.Report(declAttr->getLocation(),
                     diag::err_hlsl_maxrecursiondepth_exceeded)
          << declAttr->getRange();
    break;
  default:
    Handled = false;
    break; // SPIRV Change: was return;
  }

  if (declAttr != nullptr) {
    DXASSERT_NOMSG(Handled);
    D->addAttr(declAttr);

    // The attribute has been set but will have no effect. Validation will emit
    // a diagnostic and prevent code generation.
    ValidateAttributeTargetIsFunction(S, D, A);

    return; // SPIRV Change
  }

  // SPIRV Change Starts
  Handled = true;
  switch (A.getKind()) {
  case AttributeList::AT_VKBuiltIn:
    declAttr = ::new (S.Context)
        VKBuiltInAttr(A.getRange(), S.Context,
                      ValidateAttributeStringArg(
                          S, A,
                          "PointSize,HelperInvocation,BaseVertex,BaseInstance,"
                          "DrawIndex,DeviceIndex,ViewportMaskNV"),
                      A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_VKExtBuiltinInput:
    declAttr = ::new (S.Context) VKExtBuiltinInputAttr(
        A.getRange(), S.Context, ValidateAttributeIntArg(S, A),
        A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_VKExtBuiltinOutput:
    declAttr = ::new (S.Context) VKExtBuiltinOutputAttr(
        A.getRange(), S.Context, ValidateAttributeIntArg(S, A),
        A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_VKLocation:
    declAttr = ::new (S.Context)
        VKLocationAttr(A.getRange(), S.Context, ValidateAttributeIntArg(S, A),
                       A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_VKIndex:
    declAttr = ::new (S.Context)
        VKIndexAttr(A.getRange(), S.Context, ValidateAttributeIntArg(S, A),
                    A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_VKBinding:
    declAttr = ::new (S.Context) VKBindingAttr(
        A.getRange(), S.Context, ValidateAttributeIntArg(S, A),
        A.getNumArgs() < 2 ? INT_MIN : ValidateAttributeIntArg(S, A, 1),
        A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_VKCounterBinding:
    declAttr = ::new (S.Context) VKCounterBindingAttr(
        A.getRange(), S.Context, ValidateAttributeIntArg(S, A),
        A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_VKPushConstant:
    declAttr = ::new (S.Context) VKPushConstantAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_VKOffset:
    declAttr = ::new (S.Context)
        VKOffsetAttr(A.getRange(), S.Context, ValidateAttributeIntArg(S, A),
                     A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_VKCombinedImageSampler:
    declAttr = ::new (S.Context) VKCombinedImageSamplerAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_VKImageFormat: {
    VKImageFormatAttr::ImageFormatType Kind = ValidateAttributeEnumArg<
        VKImageFormatAttr, VKImageFormatAttr::ImageFormatType,
        VKImageFormatAttr::ConvertStrToImageFormatType>(
        S, A, VKImageFormatAttr::ImageFormatType::unknown);
    declAttr = ::new (S.Context) VKImageFormatAttr(
        A.getRange(), S.Context, Kind, A.getAttributeSpellingListIndex());
    break;
  }
  case AttributeList::AT_VKInputAttachmentIndex:
    declAttr = ::new (S.Context) VKInputAttachmentIndexAttr(
        A.getRange(), S.Context, ValidateAttributeIntArg(S, A),
        A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_VKConstantId:
    declAttr = ::new (S.Context)
        VKConstantIdAttr(A.getRange(), S.Context, ValidateAttributeIntArg(S, A),
                         A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_VKPostDepthCoverage:
    declAttr = ::new (S.Context) VKPostDepthCoverageAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_VKEarlyAndLateTests:
    declAttr = ::new (S.Context) VKEarlyAndLateTestsAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_VKDepthUnchanged:
    declAttr = ::new (S.Context) VKDepthUnchangedAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_VKStencilRefUnchangedFront:
    declAttr = ::new (S.Context) VKStencilRefUnchangedFrontAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_VKStencilRefGreaterEqualFront:
    declAttr = ::new (S.Context) VKStencilRefGreaterEqualFrontAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_VKStencilRefLessEqualFront:
    declAttr = ::new (S.Context) VKStencilRefLessEqualFrontAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_VKStencilRefUnchangedBack:
    declAttr = ::new (S.Context) VKStencilRefUnchangedBackAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_VKStencilRefGreaterEqualBack:
    declAttr = ::new (S.Context) VKStencilRefGreaterEqualBackAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_VKStencilRefLessEqualBack:
    declAttr = ::new (S.Context) VKStencilRefLessEqualBackAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_VKShaderRecordNV:
    declAttr = ::new (S.Context) VKShaderRecordNVAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_VKShaderRecordEXT:
    declAttr = ::new (S.Context) VKShaderRecordEXTAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_VKCapabilityExt:
    declAttr = ::new (S.Context) VKCapabilityExtAttr(
        A.getRange(), S.Context, ValidateAttributeIntArg(S, A),
        A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_VKExtensionExt:
    declAttr = ::new (S.Context) VKExtensionExtAttr(
        A.getRange(), S.Context, ValidateAttributeStringArg(S, A, nullptr),
        A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_VKSpvExecutionMode:
    declAttr = ::new (S.Context) VKSpvExecutionModeAttr(
        A.getRange(), S.Context, ValidateAttributeIntArg(S, A),
        A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_VKInstructionExt:
    declAttr = ::new (S.Context) VKInstructionExtAttr(
        A.getRange(), S.Context, ValidateAttributeIntArg(S, A),
        ValidateAttributeStringArg(S, A, nullptr, 1),
        A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_VKLiteralExt:
    declAttr = ::new (S.Context) VKLiteralExtAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_VKReferenceExt:
    declAttr = ::new (S.Context) VKReferenceExtAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_VKDecorateExt: {
    unsigned decoration = unsigned(ValidateAttributeIntArg(S, A));
    llvm::SmallVector<unsigned, 2> args;
    for (unsigned i = 1; i < A.getNumArgs(); ++i) {
      args.push_back(unsigned(ValidateAttributeIntArg(S, A, i)));
    }
    // Note that `llvm::SmallVector<unsigned, 2> args` will be destroyed at
    // the end of this function. However, VKDecorateExtAttr() constructor
    // allocate a new integer array internally for args. It does not create
    // a dangling pointer.
    declAttr = ::new (S.Context)
        VKDecorateExtAttr(A.getRange(), S.Context, decoration, args.data(),
                          args.size(), A.getAttributeSpellingListIndex());
  } break;
  case AttributeList::AT_VKDecorateStringExt: {
    unsigned decoration = unsigned(ValidateAttributeIntArg(S, A));
    llvm::SmallVector<std::string, 2> args;
    for (unsigned i = 1; i < A.getNumArgs(); ++i) {
      args.push_back(ValidateAttributeStringArg(S, A, nullptr, i));
    }
    // Note that `llvm::SmallVector<std::string, 2> args` will be destroyed
    // at the end of this function. However, VKDecorateExtAttr() constructor
    // allocate a new integer array internally for args. It does not create
    // a dangling pointer.
    declAttr = ::new (S.Context) VKDecorateStringExtAttr(
        A.getRange(), S.Context, decoration, args.data(), args.size(),
        A.getAttributeSpellingListIndex());
  } break;
  case AttributeList::AT_VKStorageClassExt:
    declAttr = ::new (S.Context) VKStorageClassExtAttr(
        A.getRange(), S.Context, unsigned(ValidateAttributeIntArg(S, A)),
        A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_VKTypeDefExt:
    declAttr = ::new (S.Context) VKTypeDefExtAttr(
        A.getRange(), S.Context, unsigned(ValidateAttributeIntArg(S, A)),
        unsigned(ValidateAttributeIntArg(S, A, 1)),
        A.getAttributeSpellingListIndex());
    break;
  default:
    Handled = false;
    return;
  }

  if (declAttr != nullptr) {
    DXASSERT_NOMSG(Handled);
    D->addAttr(declAttr);
  }
  // SPIRV Change Ends
}

/// <summary>Processes an attribute for a statement.</summary>
/// <param name="S">Sema with context.</param>
/// <param name="St">Statement annotated.</param>
/// <param name="A">Single parsed attribute to process.</param>
/// <param name="Range">Range of all attribute lists (useful for FixIts to
/// suggest inclusions).</param> <param name="Handled">After execution, whether
/// this was recognized and handled.</param> <returns>An attribute instance if
/// processed, nullptr if not recognized or an error was found.</returns>
Attr *hlsl::ProcessStmtAttributeForHLSL(Sema &S, Stmt *St,
                                        const AttributeList &A,
                                        SourceRange Range, bool &Handled) {
  // | Construct        | Allowed Attributes                         |
  // +------------------+--------------------------------------------+
  // | for, while, do   | loop, fastopt, unroll, allow_uav_condition |
  // | if               | branch, flatten                            |
  // | switch           | branch, flatten, forcecase, call           |

  Attr *result = nullptr;
  Handled = true;

  // SPIRV Change Starts
  if (A.hasScope() && A.getScopeName()->getName().equals("vk")) {
    switch (A.getKind()) {
    case AttributeList::AT_VKCapabilityExt:
      return ::new (S.Context) VKCapabilityExtAttr(
          A.getRange(), S.Context, ValidateAttributeIntArg(S, A),
          A.getAttributeSpellingListIndex());
    case AttributeList::AT_VKExtensionExt:
      return ::new (S.Context) VKExtensionExtAttr(
          A.getRange(), S.Context, ValidateAttributeStringArg(S, A, nullptr),
          A.getAttributeSpellingListIndex());
    default:
      Handled = false;
      return nullptr;
    }
  }
  // SPIRV Change Ends

  switch (A.getKind()) {
  case AttributeList::AT_HLSLUnroll:
    ValidateAttributeOnLoop(S, St, A);
    result = HandleUnrollAttribute(S, A);
    break;
  case AttributeList::AT_HLSLAllowUAVCondition:
    ValidateAttributeOnLoop(S, St, A);
    result = ::new (S.Context) HLSLAllowUAVConditionAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_HLSLLoop:
    ValidateAttributeOnLoop(S, St, A);
    result = ::new (S.Context) HLSLLoopAttr(A.getRange(), S.Context,
                                            A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_HLSLFastOpt:
    ValidateAttributeOnLoop(S, St, A);
    result = ::new (S.Context) HLSLFastOptAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_HLSLBranch:
    ValidateAttributeOnSwitchOrIf(S, St, A);
    result = ::new (S.Context) HLSLBranchAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_HLSLFlatten:
    ValidateAttributeOnSwitchOrIf(S, St, A);
    result = ::new (S.Context) HLSLFlattenAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_HLSLForceCase:
    ValidateAttributeOnSwitch(S, St, A);
    result = ::new (S.Context) HLSLForceCaseAttr(
        A.getRange(), S.Context, A.getAttributeSpellingListIndex());
    break;
  case AttributeList::AT_HLSLCall:
    ValidateAttributeOnSwitch(S, St, A);
    result = ::new (S.Context) HLSLCallAttr(A.getRange(), S.Context,
                                            A.getAttributeSpellingListIndex());
    break;
  default:
    Handled = false;
    break;
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////
// Implementation of Sema members.                                            //

Decl *Sema::ActOnStartHLSLBuffer(
    Scope *bufferScope, bool cbuffer, SourceLocation KwLoc,
    IdentifierInfo *Ident, SourceLocation IdentLoc,
    std::vector<hlsl::UnusualAnnotation *> &BufferAttributes,
    SourceLocation LBrace) {
  // For anonymous namespace, take the location of the left brace.
  DeclContext *lexicalParent = getCurLexicalContext();
  clang::HLSLBufferDecl *result = HLSLBufferDecl::Create(
      Context, lexicalParent, cbuffer, /*isConstantBufferView*/ false, KwLoc,
      Ident, IdentLoc, BufferAttributes, LBrace);

  // Keep track of the currently active buffer.
  HLSLBuffers.push_back(result);

  // Validate unusual annotations and emit diagnostics.
  DiagnoseUnusualAnnotationsForHLSL(*this, BufferAttributes);
  auto &&unusualIter = BufferAttributes.begin();
  auto &&unusualEnd = BufferAttributes.end();
  char expectedRegisterType = cbuffer ? 'b' : 't';
  for (; unusualIter != unusualEnd; ++unusualIter) {
    switch ((*unusualIter)->getKind()) {
    case hlsl::UnusualAnnotation::UA_ConstantPacking: {
      hlsl::ConstantPacking *constantPacking =
          cast<hlsl::ConstantPacking>(*unusualIter);
      Diag(constantPacking->Loc, diag::err_hlsl_unsupported_buffer_packoffset);
      break;
    }
    case hlsl::UnusualAnnotation::UA_RegisterAssignment: {
      hlsl::RegisterAssignment *registerAssignment =
          cast<hlsl::RegisterAssignment>(*unusualIter);

      if (registerAssignment->isSpaceOnly())
        continue;

      if (registerAssignment->RegisterType != expectedRegisterType &&
          registerAssignment->RegisterType != toupper(expectedRegisterType)) {
        Diag(registerAssignment->Loc, diag::err_hlsl_incorrect_bind_semantic)
            << (cbuffer ? "'b'" : "'t'");
      } else if (registerAssignment->ShaderProfile.size() > 0) {
        Diag(registerAssignment->Loc,
             diag::err_hlsl_unsupported_buffer_slot_target_specific);
      }
      break;
    }
    case hlsl::UnusualAnnotation::UA_SemanticDecl: {
      // Ignore semantic declarations.
      break;
    }
    case hlsl::UnusualAnnotation::UA_PayloadAccessQualifier: {
      hlsl::PayloadAccessAnnotation *annotation =
          cast<hlsl::PayloadAccessAnnotation>(*unusualIter);
      Diag(annotation->Loc,
           diag::err_hlsl_unsupported_payload_access_qualifier);
      break;
    }
    }
  }

  PushOnScopeChains(result, bufferScope);
  PushDeclContext(bufferScope, result);

  ActOnDocumentableDecl(result);

  return result;
}

void Sema::ActOnFinishHLSLBuffer(Decl *Dcl, SourceLocation RBrace) {
  DXASSERT_NOMSG(Dcl != nullptr);
  DXASSERT(Dcl == HLSLBuffers.back(), "otherwise push/pop is incorrect");
  auto *BufDecl = cast<HLSLBufferDecl>(Dcl);
  BufDecl->setRBraceLoc(RBrace);
  HLSLBuffers.pop_back();

  // Validate packoffset.
  llvm::SmallVector<std::pair<VarDecl *, unsigned>, 4> PackOffsetVec;
  bool HasPackOffset = false;
  bool HasNonPackOffset = false;
  for (auto *Field : BufDecl->decls()) {
    VarDecl *Var = dyn_cast<VarDecl>(Field);
    if (!Var)
      continue;

    unsigned Offset = UINT_MAX;

    for (const hlsl::UnusualAnnotation *it : Var->getUnusualAnnotations()) {
      if (it->getKind() == hlsl::UnusualAnnotation::UA_ConstantPacking) {
        const hlsl::ConstantPacking *packOffset =
            cast<hlsl::ConstantPacking>(it);
        unsigned CBufferOffset = packOffset->Subcomponent << 2;
        CBufferOffset += packOffset->ComponentOffset;
        // Change to bits.
        Offset = CBufferOffset << 5;
        HasPackOffset = true;
      }
    }
    PackOffsetVec.emplace_back(Var, Offset);
    if (Offset == UINT_MAX) {
      HasNonPackOffset = true;
    }
  }

  if (HasPackOffset && HasNonPackOffset) {
    Diag(BufDecl->getLocation(), diag::warn_hlsl_packoffset_mix);
  } else if (HasPackOffset) {
    // Make sure no overlap in packoffset.
    llvm::SmallDenseMap<VarDecl *, std::pair<unsigned, unsigned>>
        PackOffsetRanges;
    for (auto &Pair : PackOffsetVec) {
      VarDecl *Var = Pair.first;
      unsigned Size = Context.getTypeSize(Var->getType());
      unsigned Begin = Pair.second;
      unsigned End = Begin + Size;
      for (auto &Range : PackOffsetRanges) {
        VarDecl *OtherVar = Range.first;
        unsigned OtherBegin = Range.second.first;
        unsigned OtherEnd = Range.second.second;
        if (Begin < OtherEnd && OtherBegin < Begin) {
          Diag(Var->getLocation(), diag::err_hlsl_packoffset_overlap)
              << Var << OtherVar;
          break;
        } else if (OtherBegin < End && Begin < OtherBegin) {
          Diag(Var->getLocation(), diag::err_hlsl_packoffset_overlap)
              << Var << OtherVar;
          break;
        }
      }
      PackOffsetRanges[Var] = std::make_pair(Begin, End);
    }
  }
  PopDeclContext();
}

Decl *Sema::getActiveHLSLBuffer() const {
  return HLSLBuffers.empty() ? nullptr : HLSLBuffers.back();
}

bool Sema::IsOnHLSLBufferView() {
  // nullptr will not pushed for cbuffer.
  return !HLSLBuffers.empty() && getActiveHLSLBuffer() == nullptr;
}
HLSLBufferDecl::HLSLBufferDecl(
    DeclContext *DC, bool cbuffer, bool cbufferView, SourceLocation KwLoc,
    IdentifierInfo *Id, SourceLocation IdLoc,
    std::vector<hlsl::UnusualAnnotation *> &BufferAttributes,
    SourceLocation LBrace)
    : NamedDecl(Decl::HLSLBuffer, DC, IdLoc, DeclarationName(Id)),
      DeclContext(Decl::HLSLBuffer), LBraceLoc(LBrace), KwLoc(KwLoc),
      IsCBuffer(cbuffer), IsConstantBufferView(cbufferView) {
  if (!BufferAttributes.empty()) {
    setUnusualAnnotations(UnusualAnnotation::CopyToASTContextArray(
        getASTContext(), BufferAttributes.data(), BufferAttributes.size()));
  }
}

HLSLBufferDecl *
HLSLBufferDecl::Create(ASTContext &C, DeclContext *lexicalParent, bool cbuffer,
                       bool constantbuffer, SourceLocation KwLoc,
                       IdentifierInfo *Id, SourceLocation IdLoc,
                       std::vector<hlsl::UnusualAnnotation *> &BufferAttributes,
                       SourceLocation LBrace) {
  DeclContext *DC = C.getTranslationUnitDecl();
  HLSLBufferDecl *result = ::new (C) HLSLBufferDecl(
      DC, cbuffer, constantbuffer, KwLoc, Id, IdLoc, BufferAttributes, LBrace);
  if (DC != lexicalParent) {
    result->setLexicalDeclContext(lexicalParent);
  }

  return result;
}

const char *HLSLBufferDecl::getDeclKindName() const {
  static const char *HLSLBufferNames[] = {"tbuffer", "cbuffer", "TextureBuffer",
                                          "ConstantBuffer"};
  unsigned index = (unsigned)isCBuffer() | (isConstantBufferView()) << 1;
  return HLSLBufferNames[index];
}

void Sema::TransferUnusualAttributes(Declarator &D, NamedDecl *NewDecl) {
  assert(NewDecl != nullptr);

  if (!getLangOpts().HLSL) {
    return;
  }

  if (!D.UnusualAnnotations.empty()) {
    NewDecl->setUnusualAnnotations(UnusualAnnotation::CopyToASTContextArray(
        getASTContext(), D.UnusualAnnotations.data(),
        D.UnusualAnnotations.size()));
    D.UnusualAnnotations.clear();
  }
}

/// Checks whether a usage attribute is compatible with those seen so far and
/// maintains history.
static bool IsUsageAttributeCompatible(AttributeList::Kind kind, bool &usageIn,
                                       bool &usageOut) {
  switch (kind) {
  case AttributeList::AT_HLSLIn:
    if (usageIn)
      return false;
    usageIn = true;
    break;
  case AttributeList::AT_HLSLOut:
    if (usageOut)
      return false;
    usageOut = true;
    break;
  default:
    assert(kind == AttributeList::AT_HLSLInOut);
    if (usageOut || usageIn)
      return false;
    usageIn = usageOut = true;
    break;
  }
  return true;
}

// Diagnose valid/invalid modifiers for HLSL.
bool Sema::DiagnoseHLSLDecl(Declarator &D, DeclContext *DC, Expr *BitWidth,
                            TypeSourceInfo *TInfo, bool isParameter) {
  assert(getLangOpts().HLSL &&
         "otherwise this is called without checking language first");

  // If we have a template declaration but haven't enabled templates, error.
  if (DC->isDependentContext() &&
      getLangOpts().HLSLVersion < hlsl::LangStd::v2021)
    return false;

  DeclSpec::SCS storage = D.getDeclSpec().getStorageClassSpec();
  assert(!DC->isClosure() && "otherwise parser accepted closure syntax instead "
                             "of failing with a syntax error");

  bool result = true;
  bool isTypedef = storage == DeclSpec::SCS_typedef;
  bool isFunction = D.isFunctionDeclarator() && !DC->isRecord();
  bool isLocalVar = DC->isFunctionOrMethod() && !isFunction && !isTypedef;
  bool isGlobal = !isParameter && !isTypedef && !isFunction &&
                  (DC->isTranslationUnit() || DC->isNamespace() ||
                   DC->getDeclKind() == Decl::HLSLBuffer);
  bool isMethod = DC->isRecord() && D.isFunctionDeclarator() && !isTypedef;
  bool isField = DC->isRecord() && !D.isFunctionDeclarator() && !isTypedef;

  bool isConst = D.getDeclSpec().getTypeQualifiers() & DeclSpec::TQ::TQ_const;
  bool isVolatile =
      D.getDeclSpec().getTypeQualifiers() & DeclSpec::TQ::TQ_volatile;
  bool isStatic = storage == DeclSpec::SCS::SCS_static;
  bool isExtern = storage == DeclSpec::SCS::SCS_extern;

  bool hasSignSpec =
      D.getDeclSpec().getTypeSpecSign() != DeclSpec::TSS::TSS_unspecified;

  // Function declarations are not allowed in parameter declaration
  // TODO : Remove this check once we support function declarations/pointers in
  // HLSL
  if (isParameter && isFunction) {
    Diag(D.getLocStart(), diag::err_hlsl_func_in_func_decl);
    D.setInvalidType();
    return false;
  }

  assert((1 == (isLocalVar ? 1 : 0) + (isGlobal ? 1 : 0) + (isField ? 1 : 0) +
                   (isTypedef ? 1 : 0) + (isFunction ? 1 : 0) +
                   (isMethod ? 1 : 0) + (isParameter ? 1 : 0)) &&
         "exactly one type of declarator is being processed");

  // qt/pType captures either the type being modified, or the return type in the
  // case of a function (or method).
  QualType qt = TInfo->getType();
  const Type *pType = qt.getTypePtrOrNull();
  HLSLExternalSource *hlslSource = HLSLExternalSource::FromSema(this);

  if (!isFunction)
    hlslSource->WarnMinPrecision(qt, D.getLocStart());

  // Early checks - these are not simple attribution errors, but constructs that
  // are fundamentally unsupported,
  // and so we avoid errors that might indicate they can be repaired.
  if (DC->isRecord()) {
    unsigned int nestedDiagId = 0;
    if (isTypedef) {
      nestedDiagId = diag::err_hlsl_unsupported_nested_typedef;
    }

    if (isField && pType && pType->isIncompleteArrayType()) {
      nestedDiagId = diag::err_hlsl_unsupported_incomplete_array;
    }

    if (nestedDiagId) {
      Diag(D.getLocStart(), nestedDiagId);
      D.setInvalidType();
      return false;
    }
  }

  // String and subobject declarations are supported only as top level global
  // variables. Const and static modifiers are implied - add them if missing.
  if ((hlsl::IsStringType(qt) || hlslSource->IsSubobjectType(qt)) &&
      !D.isInvalidType()) {
    // string are supported only as top level global variables
    if (!DC->isTranslationUnit()) {
      Diag(D.getLocStart(), diag::err_hlsl_object_not_global)
          << (int)hlsl::IsStringType(qt);
      result = false;
    }
    if (isExtern) {
      Diag(D.getLocStart(), diag::err_hlsl_object_extern_not_supported)
          << (int)hlsl::IsStringType(qt);
      result = false;
    }
    const char *PrevSpec = nullptr;
    unsigned DiagID = 0;
    if (!isStatic) {
      D.getMutableDeclSpec().SetStorageClassSpec(
          *this, DeclSpec::SCS_static, D.getLocStart(), PrevSpec, DiagID,
          Context.getPrintingPolicy());
      isStatic = true;
    }
    if (!isConst) {
      D.getMutableDeclSpec().SetTypeQual(DeclSpec::TQ_const, D.getLocStart(),
                                         PrevSpec, DiagID, getLangOpts());
      isConst = true;
    }
  }

  const char *declarationType = (isLocalVar)    ? "local variable"
                                : (isTypedef)   ? "typedef"
                                : (isFunction)  ? "function"
                                : (isMethod)    ? "method"
                                : (isGlobal)    ? "global variable"
                                : (isParameter) ? "parameter"
                                : (isField)     ? "field"
                                                : "<unknown>";

  if (pType && D.isFunctionDeclarator()) {
    const FunctionProtoType *pFP = pType->getAs<FunctionProtoType>();
    if (pFP) {
      qt = pFP->getReturnType();
      hlslSource->WarnMinPrecision(qt, D.getLocStart());
      pType = qt.getTypePtrOrNull();

      // prohibit string as a return type
      if (hlsl::IsStringType(qt)) {
        static const unsigned selectReturnValueIdx = 2;
        Diag(D.getLocStart(), diag::err_hlsl_unsupported_string_decl)
            << selectReturnValueIdx;
        D.setInvalidType();
      }
    }
  }

  // Check for deprecated effect object type here, warn, and invalidate decl
  bool bDeprecatedEffectObject = false;
  bool bIsObject = false;
  if (hlsl::IsObjectType(this, qt, &bDeprecatedEffectObject)) {
    bIsObject = true;
    if (bDeprecatedEffectObject) {
      Diag(D.getLocStart(), diag::warn_hlsl_effect_object);
      D.setInvalidType();
      return false;
    }
  } else if (qt->isArrayType()) {
    QualType eltQt(qt->getArrayElementTypeNoTypeQual(), 0);
    while (eltQt->isArrayType())
      eltQt = QualType(eltQt->getArrayElementTypeNoTypeQual(), 0);
    if (hlsl::IsObjectType(this, eltQt, &bDeprecatedEffectObject)) {
      bIsObject = true;
    }
  }

  if (isExtern) {
    if (!(isFunction || isGlobal)) {
      Diag(D.getLocStart(), diag::err_hlsl_varmodifierna)
          << "'extern'" << declarationType;
      result = false;
    }
  }

  if (isStatic) {
    if (!(isLocalVar || isGlobal || isFunction || isMethod || isField)) {
      Diag(D.getLocStart(), diag::err_hlsl_varmodifierna)
          << "'static'" << declarationType;
      result = false;
    }
  }

  if (isVolatile) {
    if (!(isLocalVar || isTypedef)) {
      Diag(D.getLocStart(), diag::err_hlsl_varmodifierna)
          << "'volatile'" << declarationType;
      result = false;
    }
  }

  if (isConst) {
    if (isField && !isStatic) {
      Diag(D.getLocStart(), diag::err_hlsl_varmodifierna)
          << "'const'" << declarationType;
      result = false;
    }
  }

  ArBasicKind basicKind = hlslSource->GetTypeElementKind(qt);

  if (hasSignSpec) {
    ArTypeObjectKind objKind = hlslSource->GetTypeObjectKind(qt);
    // vectors or matrices can only have unsigned integer types.
    if (objKind == AR_TOBJ_MATRIX || objKind == AR_TOBJ_VECTOR ||
        objKind == AR_TOBJ_BASIC || objKind == AR_TOBJ_ARRAY) {
      if (!IS_BASIC_UNSIGNABLE(basicKind)) {
        Diag(D.getLocStart(), diag::err_sema_invalid_sign_spec)
            << g_ArBasicTypeNames[basicKind];
        result = false;
      }
    } else {
      Diag(D.getLocStart(), diag::err_sema_invalid_sign_spec)
          << g_ArBasicTypeNames[basicKind];
      result = false;
    }
  }

  // Validate attributes
  clang::AttributeList *pUniform = nullptr, *pUsage = nullptr,
                       *pNoInterpolation = nullptr, *pLinear = nullptr,
                       *pNoPerspective = nullptr, *pSample = nullptr,
                       *pCentroid = nullptr, *pCenter = nullptr,
                       *pAnyLinear = nullptr, // first linear attribute found
                           *pTopology = nullptr, *pMeshModifier = nullptr,
                       *pDispatchGrid = nullptr, *pMaxDispatchGrid = nullptr;
  bool usageIn = false;
  bool usageOut = false;
  bool isGroupShared = false;

  for (clang::AttributeList *pAttr = D.getDeclSpec().getAttributes().getList();
       pAttr != NULL; pAttr = pAttr->getNext()) {
    if (pAttr->isInvalid() || pAttr->isUsedAsTypeAttr())
      continue;

    switch (pAttr->getKind()) {
    case AttributeList::AT_HLSLPrecise: // precise is applicable everywhere.
      break;
    case AttributeList::AT_HLSLShared:
      if (!isGlobal) {
        Diag(pAttr->getLoc(), diag::err_hlsl_varmodifierna)
            << pAttr->getName() << declarationType << pAttr->getRange();
        result = false;
      }
      if (isStatic) {
        Diag(pAttr->getLoc(), diag::err_hlsl_varmodifiersna)
            << "'static'" << pAttr->getName() << declarationType
            << pAttr->getRange();
        result = false;
      }
      break;
    case AttributeList::AT_HLSLGroupShared:
      isGroupShared = true;
      if (!isGlobal) {
        Diag(pAttr->getLoc(), diag::err_hlsl_varmodifierna)
            << pAttr->getName() << declarationType << pAttr->getRange();
        result = false;
      }
      if (isExtern) {
        Diag(pAttr->getLoc(), diag::err_hlsl_varmodifiersna)
            << "'extern'" << pAttr->getName() << declarationType
            << pAttr->getRange();
        result = false;
      }
      break;
    case AttributeList::AT_HLSLGloballyCoherent: // Handled elsewhere
    case AttributeList::AT_HLSLReorderCoherent:  // Handled elsewhere
      break;
    case AttributeList::AT_HLSLUniform:
      if (!(isGlobal || isParameter)) {
        Diag(pAttr->getLoc(), diag::err_hlsl_varmodifierna)
            << pAttr->getName() << declarationType << pAttr->getRange();
        result = false;
      }
      if (isStatic) {
        Diag(pAttr->getLoc(), diag::err_hlsl_varmodifiersna)
            << "'static'" << pAttr->getName() << declarationType
            << pAttr->getRange();
        result = false;
      }
      pUniform = pAttr;
      break;

    case AttributeList::AT_HLSLIn:
    case AttributeList::AT_HLSLOut:
    case AttributeList::AT_HLSLInOut:
      if (!isParameter) {
        Diag(pAttr->getLoc(), diag::err_hlsl_usage_not_on_parameter)
            << pAttr->getName() << pAttr->getRange();
        result = false;
      }
      if (!IsUsageAttributeCompatible(pAttr->getKind(), usageIn, usageOut)) {
        Diag(pAttr->getLoc(), diag::err_hlsl_duplicate_parameter_usages)
            << pAttr->getName() << pAttr->getRange();
        result = false;
      }
      pUsage = pAttr;
      break;

    case AttributeList::AT_HLSLNoInterpolation:
      if (!(isParameter || isField || isFunction)) {
        Diag(pAttr->getLoc(), diag::err_hlsl_varmodifierna)
            << pAttr->getName() << declarationType << pAttr->getRange();
        result = false;
      }
      if (pNoInterpolation) {
        Diag(pAttr->getLoc(), diag::warn_hlsl_duplicate_specifier)
            << pAttr->getName() << pAttr->getRange();
      }
      pNoInterpolation = pAttr;
      break;

    case AttributeList::AT_HLSLLinear:
    case AttributeList::AT_HLSLCenter:
    case AttributeList::AT_HLSLNoPerspective:
    case AttributeList::AT_HLSLSample:
    case AttributeList::AT_HLSLCentroid:
      if (!(isParameter || isField || isFunction)) {
        Diag(pAttr->getLoc(), diag::err_hlsl_varmodifierna)
            << pAttr->getName() << declarationType << pAttr->getRange();
        result = false;
      }

      if (nullptr == pAnyLinear)
        pAnyLinear = pAttr;

      switch (pAttr->getKind()) {
      case AttributeList::AT_HLSLLinear:
        if (pLinear) {
          Diag(pAttr->getLoc(), diag::warn_hlsl_duplicate_specifier)
              << pAttr->getName() << pAttr->getRange();
        }
        pLinear = pAttr;
        break;
      case AttributeList::AT_HLSLCenter:
        if (pCenter) {
          Diag(pAttr->getLoc(), diag::warn_hlsl_duplicate_specifier)
              << pAttr->getName() << pAttr->getRange();
        }
        pCenter = pAttr;
        break;
      case AttributeList::AT_HLSLNoPerspective:
        if (pNoPerspective) {
          Diag(pAttr->getLoc(), diag::warn_hlsl_duplicate_specifier)
              << pAttr->getName() << pAttr->getRange();
        }
        pNoPerspective = pAttr;
        break;
      case AttributeList::AT_HLSLSample:
        if (pSample) {
          Diag(pAttr->getLoc(), diag::warn_hlsl_duplicate_specifier)
              << pAttr->getName() << pAttr->getRange();
        }
        pSample = pAttr;
        break;
      case AttributeList::AT_HLSLCentroid:
        if (pCentroid) {
          Diag(pAttr->getLoc(), diag::warn_hlsl_duplicate_specifier)
              << pAttr->getName() << pAttr->getRange();
        }
        pCentroid = pAttr;
        break;
      default:
        // Only relevant to the four attribs included in this block.
        break;
      }
      break;

    case AttributeList::AT_HLSLPoint:
    case AttributeList::AT_HLSLLine:
    case AttributeList::AT_HLSLLineAdj:
    case AttributeList::AT_HLSLTriangle:
    case AttributeList::AT_HLSLTriangleAdj:
      if (!(isParameter)) {
        Diag(pAttr->getLoc(), diag::err_hlsl_varmodifierna)
            << pAttr->getName() << declarationType << pAttr->getRange();
        result = false;
      }

      if (pTopology) {
        if (pTopology->getKind() == pAttr->getKind()) {
          Diag(pAttr->getLoc(), diag::warn_hlsl_duplicate_specifier)
              << pAttr->getName() << pAttr->getRange();
        } else {
          Diag(pAttr->getLoc(), diag::err_hlsl_varmodifiersna)
              << pAttr->getName() << pTopology->getName() << declarationType
              << pAttr->getRange();
          result = false;
        }
      }
      pTopology = pAttr;
      break;

    case AttributeList::AT_HLSLExport:
      if (!isFunction) {
        Diag(pAttr->getLoc(), diag::err_hlsl_varmodifierna)
            << pAttr->getName() << declarationType << pAttr->getRange();

        result = false;
      }
      if (isStatic) {
        Diag(pAttr->getLoc(), diag::err_hlsl_varmodifiersna)
            << "'static'" << pAttr->getName() << declarationType
            << pAttr->getRange();
        result = false;
      }
      break;

    case AttributeList::AT_HLSLIndices:
    case AttributeList::AT_HLSLVertices:
    case AttributeList::AT_HLSLPrimitives:
    case AttributeList::AT_HLSLPayload:
      if (!(isParameter)) {
        Diag(pAttr->getLoc(), diag::err_hlsl_varmodifierna)
            << pAttr->getName() << declarationType << pAttr->getRange();
        result = false;
      }
      if (pMeshModifier) {
        if (pMeshModifier->getKind() == pAttr->getKind()) {
          Diag(pAttr->getLoc(), diag::warn_hlsl_duplicate_specifier)
              << pAttr->getName() << pAttr->getRange();
        } else {
          Diag(pAttr->getLoc(), diag::err_hlsl_varmodifiersna)
              << pAttr->getName() << pMeshModifier->getName() << declarationType
              << pAttr->getRange();
          result = false;
        }
      }
      pMeshModifier = pAttr;
      break;
    case AttributeList::AT_HLSLNodeDispatchGrid:
      if (pDispatchGrid) {
        // TODO: it would be nice to diffentiate between an exact duplicate and
        // conflicting values
        Diag(pAttr->getLoc(), diag::warn_duplicate_attribute_exact)
            << pAttr->getName() << pAttr->getRange();
        result = false;
      } else {
        // Note: the NodeDispatchGrid values are validated later in
        // HandleDeclAttributeForHLSL()
        pDispatchGrid = pAttr;
      }
      break;
    case AttributeList::AT_HLSLNodeMaxDispatchGrid:
      if (pMaxDispatchGrid) {
        // TODO: it would be nice to diffentiate between an exact duplicate and
        // conflicting values
        Diag(pAttr->getLoc(), diag::warn_duplicate_attribute_exact)
            << pAttr->getName() << pAttr->getRange();
        result = false;
      } else {
        // Note: the NodeMaxDispatchGrid values are validated later in
        // HandleDeclAttributeForHLSL()
        pMaxDispatchGrid = pAttr;
      }
      break;

    default:
      break;
    }
  }

  if (pNoInterpolation && pAnyLinear) {
    Diag(pNoInterpolation->getLoc(), diag::err_hlsl_varmodifiersna)
        << pNoInterpolation->getName() << pAnyLinear->getName()
        << declarationType << pNoInterpolation->getRange();
    result = false;
  }
  if (pSample && pCentroid) {
    Diag(pCentroid->getLoc(), diag::warn_hlsl_specifier_overridden)
        << pCentroid->getName() << pSample->getName() << pCentroid->getRange();
  }
  if (pCenter && pCentroid) {
    Diag(pCenter->getLoc(), diag::warn_hlsl_specifier_overridden)
        << pCenter->getName() << pCentroid->getName() << pCenter->getRange();
  }
  if (pSample && pCenter) {
    Diag(pCenter->getLoc(), diag::warn_hlsl_specifier_overridden)
        << pCenter->getName() << pSample->getName() << pCenter->getRange();
  }
  clang::AttributeList *pNonUniformAttr =
      pAnyLinear ? pAnyLinear
                 : (pNoInterpolation ? pNoInterpolation : pTopology);
  if (pUniform && pNonUniformAttr) {
    Diag(pUniform->getLoc(), diag::err_hlsl_varmodifiersna)
        << pNonUniformAttr->getName() << pUniform->getName() << declarationType
        << pUniform->getRange();
    result = false;
  }
  if (pAnyLinear && pTopology) {
    Diag(pAnyLinear->getLoc(), diag::err_hlsl_varmodifiersna)
        << pTopology->getName() << pAnyLinear->getName() << declarationType
        << pAnyLinear->getRange();
    result = false;
  }
  if (pNoInterpolation && pTopology) {
    Diag(pNoInterpolation->getLoc(), diag::err_hlsl_varmodifiersna)
        << pTopology->getName() << pNoInterpolation->getName()
        << declarationType << pNoInterpolation->getRange();
    result = false;
  }
  if (pUniform && pUsage) {
    if (pUsage->getKind() != AttributeList::Kind::AT_HLSLIn) {
      Diag(pUniform->getLoc(), diag::err_hlsl_varmodifiersna)
          << pUsage->getName() << pUniform->getName() << declarationType
          << pUniform->getRange();
      result = false;
    }
  }
  if (pMeshModifier) {
    if (pMeshModifier->getKind() == AttributeList::Kind::AT_HLSLPayload) {
      if (!usageIn) {
        Diag(D.getLocStart(), diag::err_hlsl_missing_in_attr)
            << pMeshModifier->getName();
        result = false;
      }
    } else {
      if (!usageOut) {
        Diag(D.getLocStart(), diag::err_hlsl_missing_out_attr)
            << pMeshModifier->getName();
        result = false;
      }
    }
  }

  // Validate that stream-ouput objects are marked as inout
  if (isParameter && !(usageIn && usageOut) &&
      (basicKind == ArBasicKind::AR_OBJECT_LINESTREAM ||
       basicKind == ArBasicKind::AR_OBJECT_POINTSTREAM ||
       basicKind == ArBasicKind::AR_OBJECT_TRIANGLESTREAM)) {
    Diag(D.getLocStart(), diag::err_hlsl_missing_inout_attr);
    result = false;
  }

  // Disallow intangible HLSL objects in the global scope.
  if (isGlobal) {
    // Suppress actual emitting of errors for incompletable types here
    // They are redundant to those produced in ActOnUninitializedDecl.
    struct SilentDiagnoser : public TypeDiagnoser {
      SilentDiagnoser() : TypeDiagnoser(true) {}
      virtual void diagnose(Sema &S, SourceLocation Loc, QualType T) {}
    } SD;
    RequireCompleteType(D.getLocStart(), qt, SD);

    // Disallow objects in the global context
    TypeDiagContext ObjDiagContext = TypeDiagContext::CBuffersOrTBuffers;
    if (isGroupShared)
      ObjDiagContext = TypeDiagContext::GroupShared;
    else if (isStatic)
      ObjDiagContext = TypeDiagContext::GlobalVariables;

    TypeDiagContext LongVecDiagContext = TypeDiagContext::Valid;

    // Disallow long vecs from $Global cbuffers.
    if (!isStatic && !isGroupShared && !IS_BASIC_OBJECT(basicKind))
      LongVecDiagContext = TypeDiagContext::CBuffersOrTBuffers;
    if (DiagnoseTypeElements(*this, D.getLocStart(), qt, ObjDiagContext,
                             LongVecDiagContext))
      result = false;
  }

  // SPIRV change starts
#ifdef ENABLE_SPIRV_CODEGEN
  // Validate that Vulkan specific feature is only used when targeting SPIR-V
  if (!getLangOpts().SPIRV) {
    if (basicKind == ArBasicKind::AR_OBJECT_VK_SUBPASS_INPUT ||
        basicKind == ArBasicKind::AR_OBJECT_VK_SUBPASS_INPUT_MS ||
        basicKind == ArBasicKind::AR_OBJECT_VK_SPIRV_TYPE ||
        basicKind == ArBasicKind::AR_OBJECT_VK_SPIRV_OPAQUE_TYPE ||
        basicKind == ArBasicKind::AR_OBJECT_VK_SPV_INTRINSIC_TYPE ||
        basicKind == ArBasicKind::AR_OBJECT_VK_SPV_INTRINSIC_RESULT_ID) {
      Diag(D.getLocStart(), diag::err_hlsl_vulkan_specific_feature)
          << g_ArBasicTypeNames[basicKind];
      result = false;
    }
  }
#endif // ENABLE_SPIRV_CODEGEN
  // SPIRV change ends

  // Disallow bitfields where not enabled explicitly or by HV
  if (BitWidth) {
    if (getLangOpts().HLSLVersion < hlsl::LangStd::v2021) {
      Diag(BitWidth->getExprLoc(), diag::err_hlsl_bitfields);
      result = false;
    } else if (!D.UnusualAnnotations.empty()) {
      Diag(BitWidth->getExprLoc(), diag::err_hlsl_bitfields_with_annotation);
      result = false;
    }
  }

  // Validate unusual annotations.
  hlsl::DiagnoseUnusualAnnotationsForHLSL(*this, D.UnusualAnnotations);
  if (isField)
    hlsl::DiagnosePayloadAccessQualifierAnnotations(*this, D, qt,
                                                    D.UnusualAnnotations);
  auto &&unusualIter = D.UnusualAnnotations.begin();
  auto &&unusualEnd = D.UnusualAnnotations.end();
  for (; unusualIter != unusualEnd; ++unusualIter) {
    switch ((*unusualIter)->getKind()) {
    case hlsl::UnusualAnnotation::UA_ConstantPacking: {
      hlsl::ConstantPacking *constantPacking =
          cast<hlsl::ConstantPacking>(*unusualIter);
      if (!isGlobal || HLSLBuffers.size() == 0) {
        Diag(constantPacking->Loc, diag::err_hlsl_packoffset_requires_cbuffer);
        continue;
      }
      if (constantPacking->ComponentOffset > 0) {
        // Validate that this will fit.
        if (!qt.isNull()) {
          hlsl::DiagnosePackingOffset(this, constantPacking->Loc, qt,
                                      constantPacking->ComponentOffset);
        }
      }
      break;
    }
    case hlsl::UnusualAnnotation::UA_RegisterAssignment: {
      hlsl::RegisterAssignment *registerAssignment =
          cast<hlsl::RegisterAssignment>(*unusualIter);
      if (registerAssignment->IsValid) {
        if (!qt.isNull()) {
          hlsl::DiagnoseRegisterType(this, registerAssignment->Loc, qt,
                                     registerAssignment->RegisterType);
        }
      }
      break;
    }
    case hlsl::UnusualAnnotation::UA_SemanticDecl: {
      hlsl::SemanticDecl *semanticDecl = cast<hlsl::SemanticDecl>(*unusualIter);
      if (isTypedef || isLocalVar) {
        Diag(semanticDecl->Loc, diag::err_hlsl_varmodifierna)
            << "semantic" << declarationType;
      }
      break;
    }
    case hlsl::UnusualAnnotation::UA_PayloadAccessQualifier: {
      hlsl::PayloadAccessAnnotation *annotation =
          cast<hlsl::PayloadAccessAnnotation>(*unusualIter);
      if (!isField) {
        Diag(annotation->Loc,
             diag::err_hlsl_unsupported_payload_access_qualifier);
      }
      break;
    }
    }
  }

  if (!result) {
    D.setInvalidType();
  }

  return result;
}

static QualType getUnderlyingType(QualType Type) {
  while (const TypedefType *TD = dyn_cast<TypedefType>(Type)) {
    if (const TypedefNameDecl *pDecl = TD->getDecl())
      Type = pDecl->getUnderlyingType();
    else
      break;
  }
  return Type;
}

/// <summary>Return HLSL AttributedType objects if they exist on type.</summary>
/// <param name="self">Sema with context.</param>
/// <param name="type">QualType to inspect.</param>
/// <param name="ppMatrixOrientation">Set pointer to column_major/row_major
/// AttributedType if supplied.</param> <param name="ppNorm">Set pointer to
/// snorm/unorm AttributedType if supplied.</param>
void hlsl::GetHLSLAttributedTypes(
    clang::Sema *self, clang::QualType type,
    const clang::AttributedType **ppMatrixOrientation,
    const clang::AttributedType **ppNorm, const clang::AttributedType **ppGLC,
    const clang::AttributedType **ppRDC) {
  AssignOpt<const clang::AttributedType *>(nullptr, ppMatrixOrientation);
  AssignOpt<const clang::AttributedType *>(nullptr, ppNorm);
  AssignOpt<const clang::AttributedType *>(nullptr, ppGLC);
  AssignOpt<const clang::AttributedType *>(nullptr, ppRDC);

  // Note: we clear output pointers once set so we can stop searching
  QualType Desugared = getUnderlyingType(type);
  const AttributedType *AT = dyn_cast<AttributedType>(Desugared);
  while (AT && (ppMatrixOrientation || ppNorm || ppGLC || ppRDC)) {
    AttributedType::Kind Kind = AT->getAttrKind();

    if (Kind == AttributedType::attr_hlsl_row_major ||
        Kind == AttributedType::attr_hlsl_column_major) {
      if (ppMatrixOrientation) {
        *ppMatrixOrientation = AT;
        ppMatrixOrientation = nullptr;
      }
    } else if (Kind == AttributedType::attr_hlsl_unorm ||
               Kind == AttributedType::attr_hlsl_snorm) {
      if (ppNorm) {
        *ppNorm = AT;
        ppNorm = nullptr;
      }
    } else if (Kind == AttributedType::attr_hlsl_globallycoherent) {
      if (ppGLC) {
        *ppGLC = AT;
        ppGLC = nullptr;
      }
    } else if (Kind == AttributedType::attr_hlsl_reordercoherent) {
      if (ppRDC) {
        *ppRDC = AT;
        ppRDC = nullptr;
      }
    }

    Desugared = getUnderlyingType(AT->getEquivalentType());
    AT = dyn_cast<AttributedType>(Desugared);
  }

  // Unwrap component type on vector or matrix and check snorm/unorm
  Desugared = getUnderlyingType(hlsl::GetOriginalElementType(self, Desugared));
  AT = dyn_cast<AttributedType>(Desugared);
  while (AT && ppNorm) {
    AttributedType::Kind Kind = AT->getAttrKind();

    if (Kind == AttributedType::attr_hlsl_unorm ||
        Kind == AttributedType::attr_hlsl_snorm) {
      *ppNorm = AT;
      ppNorm = nullptr;
    }

    Desugared = getUnderlyingType(AT->getEquivalentType());
    AT = dyn_cast<AttributedType>(Desugared);
  }
}

/// <summary>Returns true if QualType is an HLSL Matrix type.</summary>
/// <param name="self">Sema with context.</param>
/// <param name="type">QualType to check.</param>
bool hlsl::IsMatrixType(clang::Sema *self, clang::QualType type) {
  return HLSLExternalSource::FromSema(self)->GetTypeObjectKind(type) ==
         AR_TOBJ_MATRIX;
}

/// <summary>Returns true if QualType is an HLSL Vector type.</summary>
/// <param name="self">Sema with context.</param>
/// <param name="type">QualType to check.</param>
bool hlsl::IsVectorType(clang::Sema *self, clang::QualType type) {
  return HLSLExternalSource::FromSema(self)->GetTypeObjectKind(type) ==
         AR_TOBJ_VECTOR;
}

/// <summary>Get element type for an HLSL Matrix or Vector, preserving
/// AttributedType.</summary> <param name="self">Sema with context.</param>
/// <param name="type">Matrix or Vector type.</param>
clang::QualType
hlsl::GetOriginalMatrixOrVectorElementType(clang::QualType type) {
  // TODO: Determine if this is really the best way to get the matrix/vector
  // specialization without losing the AttributedType on the template parameter
  if (const Type *Ty = type.getTypePtrOrNull()) {
    // A non-dependent template specialization type is always "sugar",
    // typically for a RecordType.  For example, a class template
    // specialization type of @c vector<int> will refer to a tag type for
    // the instantiation @c std::vector<int, std::allocator<int>>.
    if (const TemplateSpecializationType *pTemplate =
            Ty->getAs<TemplateSpecializationType>()) {
      // If we have enough arguments, pull them from the template directly,
      // rather than doing the extra lookups.
      if (pTemplate->getNumArgs() > 0)
        return pTemplate->getArg(0).getAsType();

      QualType templateRecord = pTemplate->desugar();
      Ty = templateRecord.getTypePtr();
    }
    if (!Ty)
      return QualType();

    if (const auto *TagTy = Ty->getAs<TagType>()) {
      if (const auto *SpecDecl =
              dyn_cast_or_null<ClassTemplateSpecializationDecl>(
                  TagTy->getDecl()))
        return SpecDecl->getTemplateArgs()[0].getAsType();
    }
  }
  return QualType();
}

/// <summary>Get element type, preserving AttributedType, if vector or matrix,
/// otherwise return the type unmodified.</summary> <param name="self">Sema with
/// context.</param> <param name="type">Input type.</param>
clang::QualType hlsl::GetOriginalElementType(clang::Sema *self,
                                             clang::QualType type) {
  ArTypeObjectKind Kind =
      HLSLExternalSource::FromSema(self)->GetTypeObjectKind(type);
  if (Kind == AR_TOBJ_MATRIX || Kind == AR_TOBJ_VECTOR) {
    return GetOriginalMatrixOrVectorElementType(type);
  }
  return type;
}

void hlsl::CustomPrintHLSLAttr(const clang::Attr *A, llvm::raw_ostream &Out,
                               const clang::PrintingPolicy &Policy,
                               unsigned int Indentation) {
  switch (A->getKind()) {

  // Parameter modifiers
  case clang::attr::HLSLIn:
    Out << "in ";
    break;

  case clang::attr::HLSLInOut:
    Out << "inout ";
    break;

  case clang::attr::HLSLOut:
    Out << "out ";
    break;

  // Interpolation modifiers
  case clang::attr::HLSLLinear:
    Out << "linear ";
    break;

  case clang::attr::HLSLCenter:
    Out << "center ";
    break;

  case clang::attr::HLSLCentroid:
    Out << "centroid ";
    break;

  case clang::attr::HLSLNoInterpolation:
    Out << "nointerpolation ";
    break;

  case clang::attr::HLSLNoPerspective:
    Out << "noperspective ";
    break;

  case clang::attr::HLSLSample:
    Out << "sample ";
    break;

  // Function attributes
  case clang::attr::HLSLClipPlanes: {
    Attr *noconst = const_cast<Attr *>(A);
    HLSLClipPlanesAttr *ACast = static_cast<HLSLClipPlanesAttr *>(noconst);

    if (!ACast->getClipPlane1())
      break;

    Indent(Indentation, Out);
    Out << "[clipplanes(";
    ACast->getClipPlane1()->printPretty(Out, 0, Policy);
    PrintClipPlaneIfPresent(ACast->getClipPlane2(), Out, Policy);
    PrintClipPlaneIfPresent(ACast->getClipPlane3(), Out, Policy);
    PrintClipPlaneIfPresent(ACast->getClipPlane4(), Out, Policy);
    PrintClipPlaneIfPresent(ACast->getClipPlane5(), Out, Policy);
    PrintClipPlaneIfPresent(ACast->getClipPlane6(), Out, Policy);
    Out << ")]\n";

    break;
  }

  case clang::attr::HLSLDomain: {
    Attr *noconst = const_cast<Attr *>(A);
    HLSLDomainAttr *ACast = static_cast<HLSLDomainAttr *>(noconst);
    Indent(Indentation, Out);
    Out << "[domain(\"" << ACast->getDomainType() << "\")]\n";
    break;
  }

  case clang::attr::HLSLEarlyDepthStencil:
    Indent(Indentation, Out);
    Out << "[earlydepthstencil]\n";
    break;

  case clang::attr::HLSLInstance: // TODO - test
  {
    Attr *noconst = const_cast<Attr *>(A);
    HLSLInstanceAttr *ACast = static_cast<HLSLInstanceAttr *>(noconst);
    Indent(Indentation, Out);
    Out << "[instance(" << ACast->getCount() << ")]\n";
    break;
  }

  case clang::attr::HLSLMaxTessFactor: // TODO - test
  {
    Attr *noconst = const_cast<Attr *>(A);
    HLSLMaxTessFactorAttr *ACast =
        static_cast<HLSLMaxTessFactorAttr *>(noconst);
    Indent(Indentation, Out);
    Out << "[maxtessfactor(" << ACast->getFactor() << ")]\n";
    break;
  }

  case clang::attr::HLSLNumThreads: // TODO - test
  {
    Attr *noconst = const_cast<Attr *>(A);
    HLSLNumThreadsAttr *ACast = static_cast<HLSLNumThreadsAttr *>(noconst);
    Indent(Indentation, Out);
    Out << "[numthreads(" << ACast->getX() << ", " << ACast->getY() << ", "
        << ACast->getZ() << ")]\n";
    break;
  }

  case clang::attr::HLSLRootSignature: {
    Attr *noconst = const_cast<Attr *>(A);
    HLSLRootSignatureAttr *ACast =
        static_cast<HLSLRootSignatureAttr *>(noconst);
    Indent(Indentation, Out);
    Out << "[RootSignature(\"" << ACast->getSignatureName() << "\")]\n";
    break;
  }

  case clang::attr::HLSLOutputControlPoints: {
    Attr *noconst = const_cast<Attr *>(A);
    HLSLOutputControlPointsAttr *ACast =
        static_cast<HLSLOutputControlPointsAttr *>(noconst);
    Indent(Indentation, Out);
    Out << "[outputcontrolpoints(" << ACast->getCount() << ")]\n";
    break;
  }

  case clang::attr::HLSLOutputTopology: {
    Attr *noconst = const_cast<Attr *>(A);
    HLSLOutputTopologyAttr *ACast =
        static_cast<HLSLOutputTopologyAttr *>(noconst);
    Indent(Indentation, Out);
    Out << "[outputtopology(\"" << ACast->getTopology() << "\")]\n";
    break;
  }

  case clang::attr::HLSLPartitioning: {
    Attr *noconst = const_cast<Attr *>(A);
    HLSLPartitioningAttr *ACast = static_cast<HLSLPartitioningAttr *>(noconst);
    Indent(Indentation, Out);
    Out << "[partitioning(\"" << ACast->getScheme() << "\")]\n";
    break;
  }

  case clang::attr::HLSLPatchConstantFunc: {
    Attr *noconst = const_cast<Attr *>(A);
    HLSLPatchConstantFuncAttr *ACast =
        static_cast<HLSLPatchConstantFuncAttr *>(noconst);
    Indent(Indentation, Out);
    Out << "[patchconstantfunc(\"" << ACast->getFunctionName() << "\")]\n";
    break;
  }

  case clang::attr::HLSLShader: {
    Attr *noconst = const_cast<Attr *>(A);
    HLSLShaderAttr *ACast = static_cast<HLSLShaderAttr *>(noconst);
    Indent(Indentation, Out);
    Out << "[shader(\"" << ACast->getStage() << "\")]\n";
    break;
  }

  case clang::attr::HLSLExperimental: {
    Attr *noconst = const_cast<Attr *>(A);
    HLSLExperimentalAttr *ACast = static_cast<HLSLExperimentalAttr *>(noconst);
    Indent(Indentation, Out);
    Out << "[experimental(\"" << ACast->getName() << "\", \""
        << ACast->getValue() << "\")]\n";
    break;
  }

  case clang::attr::HLSLMaxVertexCount: {
    Attr *noconst = const_cast<Attr *>(A);
    HLSLMaxVertexCountAttr *ACast =
        static_cast<HLSLMaxVertexCountAttr *>(noconst);
    Indent(Indentation, Out);
    Out << "[maxvertexcount(" << ACast->getCount() << ")]\n";
    break;
  }

  case clang::attr::NoInline:
    Indent(Indentation, Out);
    Out << "[noinline]\n";
    break;

  case clang::attr::HLSLExport:
    Indent(Indentation, Out);
    Out << "export\n";
    break;

    // Statement attributes
  case clang::attr::HLSLAllowUAVCondition:
    Indent(Indentation, Out);
    Out << "[allow_uav_condition]\n";
    break;

  case clang::attr::HLSLBranch:
    Indent(Indentation, Out);
    Out << "[branch]\n";
    break;

  case clang::attr::HLSLCall:
    Indent(Indentation, Out);
    Out << "[call]\n";
    break;

  case clang::attr::HLSLFastOpt:
    Indent(Indentation, Out);
    Out << "[fastopt]\n";
    break;

  case clang::attr::HLSLFlatten:
    Indent(Indentation, Out);
    Out << "[flatten]\n";
    break;

  case clang::attr::HLSLForceCase:
    Indent(Indentation, Out);
    Out << "[forcecase]\n";
    break;

  case clang::attr::HLSLLoop:
    Indent(Indentation, Out);
    Out << "[loop]\n";
    break;

  case clang::attr::HLSLUnroll: {
    Attr *noconst = const_cast<Attr *>(A);
    HLSLUnrollAttr *ACast = static_cast<HLSLUnrollAttr *>(noconst);
    Indent(Indentation, Out);
    if (ACast->getCount() == 0)
      Out << "[unroll]\n";
    else
      Out << "[unroll(" << ACast->getCount() << ")]\n";
    break;
  }

  case clang::attr::HLSLWaveSize: {
    Attr *noconst = const_cast<Attr *>(A);
    HLSLWaveSizeAttr *ACast = static_cast<HLSLWaveSizeAttr *>(noconst);
    Indent(Indentation, Out);
    Out << "[wavesize(" << ACast->getMin();
    if (ACast->getMax() > 0) {
      Out << ", " << ACast->getMax();
      if (ACast->getPreferred() > 0)
        Out << ", " << ACast->getPreferred();
    }
    Out << ")]\n";
    break;
  }

  // Variable modifiers
  case clang::attr::HLSLGroupShared:
    Out << "groupshared ";
    break;

  case clang::attr::HLSLPrecise:
    Out << "precise ";
    break;

  case clang::attr::HLSLSemantic: // TODO: Consider removing HLSLSemantic
                                  // attribute
    break;

  case clang::attr::HLSLShared:
    Out << "shared ";
    break;

  case clang::attr::HLSLUniform:
    Out << "uniform ";
    break;

  // These four cases are printed in TypePrinter::printAttributedBefore
  case clang::attr::HLSLSnorm:
  case clang::attr::HLSLUnorm:
    break;

  case clang::attr::HLSLPoint:
    Out << "point ";
    break;

  case clang::attr::HLSLLine:
    Out << "line ";
    break;

  case clang::attr::HLSLLineAdj:
    Out << "lineadj ";
    break;

  case clang::attr::HLSLTriangle:
    Out << "triangle ";
    break;

  case clang::attr::HLSLTriangleAdj:
    Out << "triangleadj ";
    break;

  case clang::attr::HLSLGloballyCoherent:
    Out << "globallycoherent ";
    break;

  case clang::attr::HLSLReorderCoherent:
    Out << "reordercoherent ";
    break;

  case clang::attr::HLSLIndices:
    Out << "indices ";
    break;

  case clang::attr::HLSLVertices:
    Out << "vertices ";
    break;

  case clang::attr::HLSLPrimitives:
    Out << "primitives ";
    break;

  case clang::attr::HLSLPayload:
    Out << "payload ";
    break;

  case clang::attr::HLSLNodeLaunch: {
    Attr *noconst = const_cast<Attr *>(A);
    HLSLNodeLaunchAttr *ACast = static_cast<HLSLNodeLaunchAttr *>(noconst);
    Indent(Indentation, Out);
    Out << "[NodeLaunch(\"" << ACast->getLaunchType() << "\")]\n";
    break;
  }

  case clang::attr::HLSLNodeIsProgramEntry:
    Indent(Indentation, Out);
    Out << "[NodeIsProgramEntry]\n";
    break;

  case clang::attr::HLSLNodeId: {
    Attr *noconst = const_cast<Attr *>(A);
    HLSLNodeIdAttr *ACast = static_cast<HLSLNodeIdAttr *>(noconst);
    Indent(Indentation, Out);
    if (ACast->getArrayIndex() > 0)
      Out << "[NodeId(\"" << ACast->getName() << "\"," << ACast->getArrayIndex()
          << ")]\n";
    else
      Out << "[NodeId(\"" << ACast->getName() << "\")]\n";
    break;
  }

  case clang::attr::HLSLNodeLocalRootArgumentsTableIndex: {
    Attr *noconst = const_cast<Attr *>(A);
    HLSLNodeLocalRootArgumentsTableIndexAttr *ACast =
        static_cast<HLSLNodeLocalRootArgumentsTableIndexAttr *>(noconst);
    Indent(Indentation, Out);
    Out << "[NodeLocalRootTableIndex(" << ACast->getIndex() << ")]\n";
    break;
  }

  case clang::attr::HLSLNodeShareInputOf: {
    Attr *noconst = const_cast<Attr *>(A);
    HLSLNodeShareInputOfAttr *ACast =
        static_cast<HLSLNodeShareInputOfAttr *>(noconst);
    Indent(Indentation, Out);
    if (ACast->getArrayIndex() > 0)
      Out << "[NodeShareInputOf(\"" << ACast->getName() << "\","
          << ACast->getArrayIndex() << ")]\n";
    else
      Out << "[NodeShareInputOf(\"" << ACast->getName() << "\")]\n";
    break;
  }

  case clang::attr::HLSLNodeTrackRWInputSharing: {
    Indent(Indentation, Out);
    Out << "[NodeTrackRWInputSharing]\n";
    break;
  }

  case clang::attr::HLSLNodeDispatchGrid: {
    Attr *noconst = const_cast<Attr *>(A);
    HLSLNodeDispatchGridAttr *ACast =
        static_cast<HLSLNodeDispatchGridAttr *>(noconst);
    Indent(Indentation, Out);
    Out << "[NodeDispatchGrid(" << ACast->getX() << ", " << ACast->getY()
        << ", " << ACast->getZ() << ")]\n";
    break;
  }

  case clang::attr::HLSLNodeMaxDispatchGrid: {
    Attr *noconst = const_cast<Attr *>(A);
    HLSLNodeMaxDispatchGridAttr *ACast =
        static_cast<HLSLNodeMaxDispatchGridAttr *>(noconst);
    Indent(Indentation, Out);
    Out << "[NodeMaxDispatchGrid(" << ACast->getX() << ", " << ACast->getY()
        << ", " << ACast->getZ() << ")]\n";
    break;
  }

  case clang::attr::HLSLNodeMaxRecursionDepth: {
    Attr *noconst = const_cast<Attr *>(A);
    HLSLNodeMaxRecursionDepthAttr *ACast =
        static_cast<HLSLNodeMaxRecursionDepthAttr *>(noconst);
    Indent(Indentation, Out);
    Out << "[NodeMaxRecursionDepth(" << ACast->getCount() << ")]\n";
    break;
  }

  case clang::attr::HLSLMaxRecords: {
    Attr *noconst = const_cast<Attr *>(A);
    auto *ACast = static_cast<HLSLMaxRecordsAttr *>(noconst);
    Indent(Indentation, Out);
    Out << "[MaxRecords(" << ACast->getMaxCount() << ")]\n";
    break;
  }
  case clang::attr::HLSLNodeArraySize: {
    Attr *noconst = const_cast<Attr *>(A);
    auto *ACast = static_cast<HLSLNodeArraySizeAttr *>(noconst);
    Indent(Indentation, Out);
    Out << "[NodeArraySize(" << ACast->getCount() << ")]\n";
    break;
  }

  case clang::attr::HLSLMaxRecordsSharedWith: {
    Attr *noconst = const_cast<Attr *>(A);
    HLSLMaxRecordsSharedWithAttr *ACast =
        static_cast<HLSLMaxRecordsSharedWithAttr *>(noconst);
    Indent(Indentation, Out);
    Out << "[MaxRecordsSharedWith(\"" << ACast->getName() << "\")]\n";
    break;
  }

  case clang::attr::HLSLAllowSparseNodes: {
    Indent(Indentation, Out);
    Out << "[AllowSparseNodes]\n";
    break;
  }

  case clang::attr::HLSLUnboundedSparseNodes: {
    Indent(Indentation, Out);
    Out << "[UnboundedSparseNodes]\n";
    break;
  }

  default:
    A->printPretty(Out, Policy);
    break;
  }
}

bool hlsl::IsHLSLAttr(clang::attr::Kind AttrKind) {
  switch (AttrKind) {
  case clang::attr::HLSLAllowUAVCondition:
  case clang::attr::HLSLBranch:
  case clang::attr::HLSLCall:
  case clang::attr::HLSLCentroid:
  case clang::attr::HLSLClipPlanes:
  case clang::attr::HLSLDomain:
  case clang::attr::HLSLEarlyDepthStencil:
  case clang::attr::HLSLFastOpt:
  case clang::attr::HLSLFlatten:
  case clang::attr::HLSLForceCase:
  case clang::attr::HLSLGroupShared:
  case clang::attr::HLSLIn:
  case clang::attr::HLSLInOut:
  case clang::attr::HLSLInstance:
  case clang::attr::HLSLLinear:
  case clang::attr::HLSLCenter:
  case clang::attr::HLSLLoop:
  case clang::attr::HLSLMaxTessFactor:
  case clang::attr::HLSLNoInterpolation:
  case clang::attr::HLSLNoPerspective:
  case clang::attr::HLSLNumThreads:
  case clang::attr::HLSLRootSignature:
  case clang::attr::HLSLOut:
  case clang::attr::HLSLOutputControlPoints:
  case clang::attr::HLSLOutputTopology:
  case clang::attr::HLSLPartitioning:
  case clang::attr::HLSLPatchConstantFunc:
  case clang::attr::HLSLMaxVertexCount:
  case clang::attr::HLSLPrecise:
  case clang::attr::HLSLSample:
  case clang::attr::HLSLSemantic:
  case clang::attr::HLSLShader:
  case clang::attr::HLSLShared:
  case clang::attr::HLSLSnorm:
  case clang::attr::HLSLUniform:
  case clang::attr::HLSLUnorm:
  case clang::attr::HLSLUnroll:
  case clang::attr::HLSLPoint:
  case clang::attr::HLSLLine:
  case clang::attr::HLSLLineAdj:
  case clang::attr::HLSLTriangle:
  case clang::attr::HLSLTriangleAdj:
  case clang::attr::HLSLGloballyCoherent:
  case clang::attr::HLSLIndices:
  case clang::attr::HLSLVertices:
  case clang::attr::HLSLPrimitives:
  case clang::attr::HLSLPayload:
  case clang::attr::NoInline:
  case clang::attr::HLSLExport:
  case clang::attr::HLSLWaveSensitive:
  case clang::attr::HLSLWaveSize:
  case clang::attr::HLSLMaxRecordsSharedWith:
  case clang::attr::HLSLMaxRecords:
  case clang::attr::HLSLNodeArraySize:
  case clang::attr::HLSLAllowSparseNodes:
  case clang::attr::HLSLUnboundedSparseNodes:
  case clang::attr::HLSLNodeDispatchGrid:
  case clang::attr::HLSLNodeMaxDispatchGrid:
  case clang::attr::HLSLNodeMaxRecursionDepth:
  case clang::attr::HLSLNodeId:
  case clang::attr::HLSLNodeIsProgramEntry:
  case clang::attr::HLSLNodeLaunch:
  case clang::attr::HLSLNodeLocalRootArgumentsTableIndex:
  case clang::attr::HLSLNodeShareInputOf:
  case clang::attr::HLSLNodeTrackRWInputSharing:
  case clang::attr::HLSLReorderCoherent:
  case clang::attr::VKBinding:
  case clang::attr::VKBuiltIn:
  case clang::attr::VKConstantId:
  case clang::attr::VKCounterBinding:
  case clang::attr::VKIndex:
  case clang::attr::VKInputAttachmentIndex:
  case clang::attr::VKLocation:
  case clang::attr::VKOffset:
  case clang::attr::VKPushConstant:
  case clang::attr::VKShaderRecordNV:
  case clang::attr::VKShaderRecordEXT:
    return true;
  default:
    // Only HLSL/VK Attributes return true. Only used for printPretty(), which
    // doesn't support them.
    break;
  }

  return false;
}

void hlsl::PrintClipPlaneIfPresent(clang::Expr *ClipPlane,
                                   llvm::raw_ostream &Out,
                                   const clang::PrintingPolicy &Policy) {
  if (ClipPlane) {
    Out << ", ";
    ClipPlane->printPretty(Out, 0, Policy);
  }
}

bool hlsl::IsObjectType(clang::Sema *self, clang::QualType type,
                        bool *isDeprecatedEffectObject) {
  HLSLExternalSource *pExternalSource = HLSLExternalSource::FromSema(self);
  if (pExternalSource &&
      pExternalSource->GetTypeObjectKind(type) == AR_TOBJ_OBJECT) {
    if (isDeprecatedEffectObject)
      *isDeprecatedEffectObject =
          pExternalSource->GetTypeElementKind(type) == AR_OBJECT_LEGACY_EFFECT;
    return true;
  }
  if (isDeprecatedEffectObject)
    *isDeprecatedEffectObject = false;
  return false;
}

bool hlsl::CanConvert(clang::Sema *self, clang::SourceLocation loc,
                      clang::Expr *sourceExpr, clang::QualType target,
                      bool explicitConversion,
                      clang::StandardConversionSequence *standard) {
  return HLSLExternalSource::FromSema(self)->CanConvert(
      loc, sourceExpr, target, explicitConversion, nullptr, standard);
}

void hlsl::Indent(unsigned int Indentation, llvm::raw_ostream &Out) {
  for (unsigned i = 0; i != Indentation; ++i)
    Out << "  ";
}

void hlsl::RegisterIntrinsicTable(clang::ExternalSemaSource *self,
                                  IDxcIntrinsicTable *table) {
  DXASSERT_NOMSG(self != nullptr);
  DXASSERT_NOMSG(table != nullptr);

  HLSLExternalSource *source = (HLSLExternalSource *)self;
  source->RegisterIntrinsicTable(table);
}

clang::QualType
hlsl::CheckVectorConditional(clang::Sema *self, clang::ExprResult &Cond,
                             clang::ExprResult &LHS, clang::ExprResult &RHS,
                             clang::SourceLocation QuestionLoc) {
  return HLSLExternalSource::FromSema(self)->CheckVectorConditional(
      Cond, LHS, RHS, QuestionLoc);
}

bool IsTypeNumeric(clang::Sema *self, clang::QualType &type) {
  UINT count;
  return HLSLExternalSource::FromSema(self)->IsTypeNumeric(type, &count);
}

void Sema::CheckHLSLArrayAccess(const Expr *expr) {
  DXASSERT_NOMSG(isa<CXXOperatorCallExpr>(expr));
  const CXXOperatorCallExpr *OperatorCallExpr = cast<CXXOperatorCallExpr>(expr);
  DXASSERT_NOMSG(OperatorCallExpr->getOperator() ==
                 OverloadedOperatorKind::OO_Subscript);

  const Expr *RHS = OperatorCallExpr->getArg(1); // first subscript expression
  llvm::APSInt index;
  if (RHS->EvaluateAsInt(index, Context)) {
    int64_t intIndex = index.getLimitedValue();
    const QualType LHSQualType = OperatorCallExpr->getArg(0)->getType();
    if (IsVectorType(this, LHSQualType)) {
      uint32_t vectorSize = GetHLSLVecSize(LHSQualType);
      // If expression is a double two subscript operator for matrix (e.g
      // x[0][1]) we also have to check the first subscript oprator by
      // recursively calling this funciton for the first CXXOperatorCallExpr
      if (isa<CXXOperatorCallExpr>(OperatorCallExpr->getArg(0))) {
        const CXXOperatorCallExpr *object =
            cast<CXXOperatorCallExpr>(OperatorCallExpr->getArg(0));
        if (object->getOperator() == OverloadedOperatorKind::OO_Subscript) {
          CheckHLSLArrayAccess(object);
        }
      }
      if (intIndex < 0 || (uint32_t)intIndex >= vectorSize) {
        Diag(RHS->getExprLoc(),
             diag::err_hlsl_vector_element_index_out_of_bounds)
            << (int)intIndex;
      }
    } else if (IsMatrixType(this, LHSQualType)) {
      uint32_t rowCount, colCount;
      GetHLSLMatRowColCount(LHSQualType, rowCount, colCount);
      if (intIndex < 0 || (uint32_t)intIndex >= rowCount) {
        Diag(RHS->getExprLoc(), diag::err_hlsl_matrix_row_index_out_of_bounds)
            << (int)intIndex;
      }
    }
  }
}

clang::QualType ApplyTypeSpecSignToParsedType(clang::Sema *self,
                                              clang::QualType &type,
                                              clang::TypeSpecifierSign TSS,
                                              clang::SourceLocation Loc) {
  return HLSLExternalSource::FromSema(self)->ApplyTypeSpecSignToParsedType(
      type, TSS, Loc);
}

QualType Sema::getHLSLDefaultSpecialization(TemplateDecl *Decl) {
  if (Decl->getTemplateParameters()->getMinRequiredArguments() == 0) {
    TemplateArgumentListInfo EmptyArgs;
    EmptyArgs.setLAngleLoc(Decl->getSourceRange().getEnd());
    EmptyArgs.setRAngleLoc(Decl->getSourceRange().getEnd());
    return CheckTemplateIdType(TemplateName(Decl),
                               Decl->getSourceRange().getEnd(), EmptyArgs);
  }
  return QualType();
}

namespace hlsl {

static bool nodeInputIsCompatible(DXIL::NodeIOKind IOType,
                                  DXIL::NodeLaunchType launchType) {
  switch (IOType) {
  case DXIL::NodeIOKind::DispatchNodeInputRecord:
  case DXIL::NodeIOKind::RWDispatchNodeInputRecord:
    return launchType == DXIL::NodeLaunchType::Broadcasting;

  case DXIL::NodeIOKind::GroupNodeInputRecords:
  case DXIL::NodeIOKind::RWGroupNodeInputRecords:
  case DXIL::NodeIOKind::EmptyInput:
    return launchType == DXIL::NodeLaunchType::Coalescing;

  case DXIL::NodeIOKind::ThreadNodeInputRecord:
  case DXIL::NodeIOKind::RWThreadNodeInputRecord:
    return launchType == DXIL::NodeLaunchType::Thread;

  default:
    return false;
  }
}

// Diagnose input node record to make sure it has exactly one SV_DispatchGrid
// semantics. Recursivelly walk all fields on the record and all of its base
// classes/structs
void DiagnoseDispatchGridSemantics(Sema &S, RecordDecl *InputRecordDecl,
                                   SourceLocation NodeRecordLoc,
                                   SourceLocation &DispatchGridLoc,
                                   bool &Found) {
  if (auto *CXXInputRecordDecl = dyn_cast<CXXRecordDecl>(InputRecordDecl)) {
    // Walk up the inheritance chain and check all fields on base classes
    for (auto &B : CXXInputRecordDecl->bases()) {
      const RecordType *BaseStructType = B.getType()->getAsStructureType();
      if (nullptr != BaseStructType) {
        CXXRecordDecl *BaseTypeDecl =
            dyn_cast<CXXRecordDecl>(BaseStructType->getDecl());
        if (nullptr != BaseTypeDecl) {
          DiagnoseDispatchGridSemantics(S, BaseTypeDecl, NodeRecordLoc,
                                        DispatchGridLoc, Found);
        }
      }
    }
  }

  // Iterate over fields of the current struct
  for (FieldDecl *FD : InputRecordDecl->fields()) {
    // Check if any of the fields have SV_DispatchGrid annotation
    for (const hlsl::UnusualAnnotation *it : FD->getUnusualAnnotations()) {
      if (it->getKind() == hlsl::UnusualAnnotation::UA_SemanticDecl) {
        const hlsl::SemanticDecl *sd = cast<hlsl::SemanticDecl>(it);
        if (sd->SemanticName.equals("SV_DispatchGrid")) {
          if (!Found) {
            Found = true;
            QualType Ty = FD->getType();
            QualType ElTy = Ty;
            unsigned NumElt = 1;
            if (hlsl::IsVectorType(&S, Ty)) {
              NumElt = hlsl::GetElementCount(Ty);
              ElTy = hlsl::GetHLSLVecElementType(Ty);
            } else if (const ArrayType *AT = Ty->getAsArrayTypeUnsafe()) {
              if (auto *CAT = dyn_cast<ConstantArrayType>(AT)) {
                NumElt = CAT->getSize().getZExtValue();
                ElTy = AT->getElementType();
              }
            }
            ElTy = ElTy.getDesugaredType(S.getASTContext());
            if (NumElt > 3 || (ElTy != S.getASTContext().UnsignedIntTy &&
                               ElTy != S.getASTContext().UnsignedShortTy)) {
              S.Diags.Report(
                  it->Loc,
                  diag::err_hlsl_incompatible_dispatchgrid_semantic_type)
                  << Ty;
              S.Diags.Report(NodeRecordLoc, diag::note_defined_here)
                  << "NodeInput/Output record";
            }
            DispatchGridLoc = it->Loc;
          } else {
            // There should be just one SV_DispatchGrid in per record struct
            S.Diags.Report(
                it->Loc,
                diag::err_hlsl_dispatchgrid_semantic_already_specified);
            S.Diags.Report(DispatchGridLoc, diag::note_defined_here)
                << "other SV_DispatchGrid";
          }
          break;
        }
      }
    }
    // Check nested structs
    const RecordType *FieldTypeAsStruct = FD->getType()->getAsStructureType();
    if (nullptr != FieldTypeAsStruct) {
      CXXRecordDecl *FieldTypeDecl =
          dyn_cast<CXXRecordDecl>(FieldTypeAsStruct->getDecl());
      if (nullptr != FieldTypeDecl) {
        DiagnoseDispatchGridSemantics(S, FieldTypeDecl, NodeRecordLoc,
                                      DispatchGridLoc, Found);
      }
    }
  }
}

void DiagnoseDispatchGridSemantics(Sema &S, RecordDecl *NodeRecordStruct,
                                   SourceLocation NodeRecordLoc, bool &Found) {
  SourceLocation DispatchGridLoc;
  DiagnoseDispatchGridSemantics(S, NodeRecordStruct, NodeRecordLoc,
                                DispatchGridLoc, Found);
}

void DiagnoseAmplificationEntry(Sema &S, FunctionDecl *FD,
                                llvm::StringRef StageName) {

  if (!(FD->getAttr<HLSLNumThreadsAttr>()))
    S.Diags.Report(FD->getLocation(), diag::err_hlsl_missing_attr)
        << StageName << "numthreads";

  return;
}

void DiagnoseVertexEntry(Sema &S, FunctionDecl *FD, llvm::StringRef StageName) {
  for (auto *annotation : FD->getUnusualAnnotations()) {
    if (auto *sema = dyn_cast<hlsl::SemanticDecl>(annotation)) {
      if (sema->SemanticName.equals_lower("POSITION") ||
          sema->SemanticName.equals_lower("POSITION0")) {
        S.Diags.Report(FD->getLocation(),
                       diag::warn_hlsl_semantic_attribute_position_misuse_hint)
            << sema->SemanticName;
      }
    }
  }

  return;
}

void DiagnoseMeshEntry(Sema &S, FunctionDecl *FD, llvm::StringRef StageName) {

  if (!(FD->getAttr<HLSLNumThreadsAttr>()))
    S.Diags.Report(FD->getLocation(), diag::err_hlsl_missing_attr)
        << StageName << "numthreads";
  if (!(FD->getAttr<HLSLOutputTopologyAttr>()))
    S.Diags.Report(FD->getLocation(), diag::err_hlsl_missing_attr)
        << StageName << "outputtopology";
  return;
}

void DiagnoseDomainEntry(Sema &S, FunctionDecl *FD, llvm::StringRef StageName) {
  for (const auto *param : FD->params()) {
    if (!hlsl::IsHLSLOutputPatchType(param->getType()))
      continue;
    if (hlsl::GetHLSLInputPatchCount(param->getType()) > 0)
      continue;
    S.Diags.Report(param->getLocation(), diag::err_hlsl_outputpatch_size);
  }
  return;
}

void DiagnoseHullEntry(Sema &S, FunctionDecl *FD, llvm::StringRef StageName) {
  HLSLPatchConstantFuncAttr *Attr = FD->getAttr<HLSLPatchConstantFuncAttr>();
  if (!Attr)
    S.Diags.Report(FD->getLocation(), diag::err_hlsl_missing_attr)
        << StageName << "patchconstantfunc";
  if (!(FD->getAttr<HLSLOutputTopologyAttr>()))
    S.Diags.Report(FD->getLocation(), diag::err_hlsl_missing_attr)
        << StageName << "outputtopology";
  if (!(FD->getAttr<HLSLOutputControlPointsAttr>()))
    S.Diags.Report(FD->getLocation(), diag::err_hlsl_missing_attr)
        << StageName << "outputcontrolpoints";

  for (const auto *param : FD->params()) {
    if (!hlsl::IsHLSLInputPatchType(param->getType()))
      continue;
    if (hlsl::GetHLSLInputPatchCount(param->getType()) > 0)
      continue;
    S.Diags.Report(param->getLocation(), diag::err_hlsl_inputpatch_size);
  }
  return;
}

void DiagnoseGeometryEntry(Sema &S, FunctionDecl *FD,
                           llvm::StringRef StageName) {

  if (!(FD->getAttr<HLSLMaxVertexCountAttr>()))
    S.Diags.Report(FD->getLocation(), diag::err_hlsl_missing_attr)
        << StageName << "maxvertexcount";

  return;
}

void DiagnoseComputeEntry(Sema &S, FunctionDecl *FD, llvm::StringRef StageName,
                          bool isActiveEntry) {
  if (isActiveEntry) {
    if (!(FD->getAttr<HLSLNumThreadsAttr>()))
      S.Diags.Report(FD->getLocation(), diag::err_hlsl_missing_attr)
          << StageName << "numthreads";
    if (auto WaveSizeAttr = FD->getAttr<HLSLWaveSizeAttr>()) {
      std::string profile = S.getLangOpts().HLSLProfile;
      const ShaderModel *SM = hlsl::ShaderModel::GetByName(profile.c_str());
      if (!SM->IsSM66Plus()) {
        S.Diags.Report(WaveSizeAttr->getRange().getBegin(),
                       diag::err_hlsl_attribute_in_wrong_shader_model)
            << "wavesize"
            << "6.6";
      }
      if (!SM->IsSM68Plus() && WaveSizeAttr->getSpelledArgsCount() > 1)
        S.Diags.Report(WaveSizeAttr->getRange().getBegin(),
                       diag::err_hlsl_wavesize_insufficient_shader_model)
            << "wavesize" << 1;
    }
  }
}

void DiagnoseNodeEntry(Sema &S, FunctionDecl *FD, llvm::StringRef StageName,
                       bool isActiveEntry) {

  SourceLocation NodeLoc = SourceLocation();
  SourceLocation NodeLaunchLoc = SourceLocation();
  DXIL::NodeLaunchType NodeLaunchTy = DXIL::NodeLaunchType::Invalid;
  unsigned InputCount = 0;

  auto pAttr = FD->getAttr<HLSLShaderAttr>();
  DXIL::ShaderKind shaderKind = ShaderModel::KindFromFullName(StageName);
  if (shaderKind == DXIL::ShaderKind::Node) {
    NodeLoc = pAttr->getLocation();
    // SPIR-V node shader support is experimental
    if (S.getLangOpts().SPIRV) {
      S.Diag(NodeLoc, diag::warn_spirv_node_shaders_experimental);
    }
  }
  if (NodeLoc.isInvalid()) {
    return;
  }

  // save NodeLaunch type for use later
  if (auto NodeLaunchAttr = FD->getAttr<HLSLNodeLaunchAttr>()) {
    NodeLaunchTy =
        ShaderModel::NodeLaunchTypeFromName(NodeLaunchAttr->getLaunchType());
    NodeLaunchLoc = NodeLaunchAttr->getLocation();
  } else {
    NodeLaunchTy = DXIL::NodeLaunchType::Broadcasting;
    NodeLaunchLoc = SourceLocation();
  }

  // Check that if a Thread launch node has the NumThreads attribute the
  // thread group size is (1,1,1)
  if (NodeLaunchTy == DXIL::NodeLaunchType::Thread) {
    if (auto NumThreads = FD->getAttr<HLSLNumThreadsAttr>()) {
      if (NumThreads->getX() != 1 || NumThreads->getY() != 1 ||
          NumThreads->getZ() != 1) {
        S.Diags.Report(NumThreads->getLocation(),
                       diag::err_hlsl_wg_thread_launch_group_size)
            << NumThreads->getRange();
        // Only output the note if the source location is valid
        if (NodeLaunchLoc.isValid())
          S.Diags.Report(NodeLaunchLoc, diag::note_defined_here)
              << "Launch type";
      }
    }
  } else if (!FD->hasAttr<HLSLNumThreadsAttr>()) {
    // All other launch types require the NumThreads attribute.
    S.Diags.Report(FD->getLocation(), diag::err_hlsl_missing_node_attr)
        << FD->getName() << ShaderModel::GetNodeLaunchTypeName(NodeLaunchTy)
        << "numthreads";
  }

  if (isActiveEntry) {
    if (auto WaveSizeAttr = FD->getAttr<HLSLWaveSizeAttr>()) {
      std::string profile = S.getLangOpts().HLSLProfile;
      const ShaderModel *SM = hlsl::ShaderModel::GetByName(profile.c_str());
      if (!SM->IsSM66Plus()) {
        S.Diags.Report(WaveSizeAttr->getRange().getBegin(),
                       diag::err_hlsl_attribute_in_wrong_shader_model)
            << "wavesize"
            << "6.6";
      }
    }
  }

  auto *NodeDG = FD->getAttr<HLSLNodeDispatchGridAttr>();
  auto *NodeMDG = FD->getAttr<HLSLNodeMaxDispatchGridAttr>();
  if (NodeLaunchTy != DXIL::NodeLaunchType::Broadcasting) {
    // NodeDispatchGrid is only valid for Broadcasting nodes
    if (NodeDG) {
      S.Diags.Report(NodeDG->getLocation(), diag::err_hlsl_launch_type_attr)
          << NodeDG->getSpelling()
          << ShaderModel::GetNodeLaunchTypeName(
                 DXIL::NodeLaunchType::Broadcasting)
          << NodeDG->getRange();
      // Only output the note if the source location is valid
      if (NodeLaunchLoc.isValid())
        S.Diags.Report(NodeLaunchLoc, diag::note_defined_here) << "Launch type";
    }
    // NodeMaxDispatchGrid is only valid for Broadcasting nodes
    if (NodeMDG) {
      S.Diags.Report(NodeMDG->getLocation(), diag::err_hlsl_launch_type_attr)
          << NodeMDG->getSpelling()
          << ShaderModel::GetNodeLaunchTypeName(
                 DXIL::NodeLaunchType::Broadcasting)
          << NodeMDG->getRange();
      // Only output the note if the source location is valid
      if (NodeLaunchLoc.isValid())
        S.Diags.Report(NodeLaunchLoc, diag::note_defined_here) << "Launch type";
    }
  } else {
    // A Broadcasting node must have one of NodeDispatchGrid or
    // NodeMaxDispatchGrid
    if (!NodeMDG && !NodeDG)
      S.Diags.Report(FD->getLocation(),
                     diag::err_hlsl_missing_dispatchgrid_attr)
          << FD->getName();
    // NodeDispatchGrid and NodeMaxDispatchGrid may not be used together
    if (NodeMDG && NodeDG) {
      S.Diags.Report(NodeMDG->getLocation(),
                     diag::err_hlsl_incompatible_node_attr)
          << FD->getName() << NodeMDG->getSpelling() << NodeDG->getSpelling()
          << NodeMDG->getRange();
      S.Diags.Report(NodeDG->getLocation(), diag::note_defined_here)
          << NodeDG->getSpelling();
    }
    // Diagnose dispatch grid semantics.
    bool Found = false;
    for (ParmVarDecl *PD : FD->params()) {
      QualType ParamType = PD->getType().getCanonicalType();

      // Find parameter that is the node input record
      if (hlsl::IsHLSLNodeInputType(ParamType)) {
        // Node records are template types
        if (RecordDecl *NodeStructDecl =
                hlsl::GetRecordDeclFromNodeObjectType(ParamType)) {
          // Diagnose any SV_DispatchGrid semantics used in record.
          DiagnoseDispatchGridSemantics(S, NodeStructDecl, PD->getLocation(),
                                        Found);
        }
      }
    }
    // Node with NodeMaxDispatchGrid must have SV_DispatchGrid semantic.
    if (NodeMDG && !Found) {
      S.Diags.Report(FD->getLocation(),
                     diag::err_hlsl_missing_dispatchgrid_semantic)
          << FD->getName();
    }
  }

  // Dignose node output.
  for (ParmVarDecl *PD : FD->params()) {
    QualType ParamType = PD->getType().getCanonicalType();

    // Find parameter that is the node input record
    if (hlsl::IsHLSLNodeOutputType(ParamType)) {
      // Node records are template types
      if (RecordDecl *NodeStructDecl =
              hlsl::GetRecordDeclFromNodeObjectType(ParamType)) {
        // Diagnose any SV_DispatchGrid semantics used in record.
        bool OutputFound = false;
        DiagnoseDispatchGridSemantics(S, NodeStructDecl, PD->getLocation(),
                                      OutputFound);
      }
    }
  }

  if (!FD->getReturnType()->isVoidType())
    S.Diag(FD->getLocation(), diag::err_shader_must_return_void) << StageName;

  // Check parameter constraints
  for (unsigned Idx = 0; Idx < FD->getNumParams(); ++Idx) {
    ParmVarDecl *Param = FD->getParamDecl(Idx);
    clang::QualType ParamTy = Param->getType();

    auto *MaxRecordsAttr = Param->getAttr<HLSLMaxRecordsAttr>();
    auto *MaxRecordsSharedWithAttr =
        Param->getAttr<HLSLMaxRecordsSharedWithAttr>();
    auto *AllowSparseNodesAttr = Param->getAttr<HLSLAllowSparseNodesAttr>();
    auto *NodeArraySizeAttr = Param->getAttr<HLSLNodeArraySizeAttr>();
    auto *UnboundedSparseNodesAttr =
        Param->getAttr<HLSLUnboundedSparseNodesAttr>();
    // Check any node input is compatible with the node launch type
    if (hlsl::IsHLSLNodeInputType(ParamTy)) {
      InputCount++;
      if (NodeLaunchTy != DXIL::NodeLaunchType::Invalid &&
          !nodeInputIsCompatible(GetNodeIOType(Param->getType()),
                                 NodeLaunchTy)) {
        const RecordType *RT = Param->getType()->getAs<RecordType>();
        S.Diags.Report(Param->getLocation(), diag::err_hlsl_wg_input_kind)
            << RT->getDecl()->getName()
            << ShaderModel::GetNodeLaunchTypeName(NodeLaunchTy)
            << (static_cast<unsigned>(NodeLaunchTy) - 1)
            << Param->getSourceRange();
        if (NodeLaunchLoc.isValid())
          S.Diags.Report(NodeLaunchLoc, diag::note_defined_here)
              << "Launch type";
      }
      if (InputCount > 1)
        S.Diags.Report(Param->getLocation(),
                       diag::err_hlsl_too_many_node_inputs)
            << FD->getName() << Param->getSourceRange();

      if (MaxRecordsAttr && NodeLaunchTy != DXIL::NodeLaunchType::Coalescing) {
        S.Diags.Report(MaxRecordsAttr->getLocation(),
                       diag::err_hlsl_maxrecord_on_wrong_launch)
            << MaxRecordsAttr->getRange();
      }
    } else if (hlsl::IsHLSLNodeOutputType(ParamTy)) {
      // If node output is not an array, diagnose array only attributes
      if (((uint32_t)GetNodeIOType(ParamTy) &
           (uint32_t)DXIL::NodeIOFlags::NodeArray) == 0) {
        Attr *ArrayAttrs[] = {NodeArraySizeAttr, UnboundedSparseNodesAttr};
        for (auto *A : ArrayAttrs) {
          if (A) {
            S.Diags.Report(A->getLocation(),
                           diag::err_hlsl_wg_attr_only_on_output_array)
                << A << A->getRange();
          }
        }
      }
    } else {
      if (MaxRecordsAttr) {
        S.Diags.Report(MaxRecordsAttr->getLocation(),
                       diag::err_hlsl_wg_attr_only_on_output_or_input_record)
            << MaxRecordsAttr << MaxRecordsAttr->getRange();
      }
    }

    if (!hlsl::IsHLSLNodeOutputType(ParamTy)) {
      Attr *OutputOnly[] = {MaxRecordsSharedWithAttr, AllowSparseNodesAttr,
                            NodeArraySizeAttr, UnboundedSparseNodesAttr};
      for (auto *A : OutputOnly) {
        if (A) {
          S.Diags.Report(A->getLocation(),
                         diag::err_hlsl_wg_attr_only_on_output)
              << A << A->getRange();
        }
      }
    }

    if (UnboundedSparseNodesAttr && NodeArraySizeAttr &&
        NodeArraySizeAttr->getCount() != -1) {
      S.Diags.Report(NodeArraySizeAttr->getLocation(),
                     diag::err_hlsl_wg_nodearraysize_conflict_unbounded)
          << NodeArraySizeAttr->getCount() << NodeArraySizeAttr->getRange();
      S.Diags.Report(UnboundedSparseNodesAttr->getLocation(),
                     diag::note_conflicting_attribute)
          << UnboundedSparseNodesAttr->getRange();
    }

    // arrays of NodeOutput or EmptyNodeOutput are not supported as node
    // parameters
    if (ParamTy->isArrayType()) {
      const ArrayType *AT = dyn_cast<ArrayType>(ParamTy);
      DXIL::NodeIOKind Kind = GetNodeIOType(AT->getElementType());
      if (Kind != DXIL::NodeIOKind::Invalid) {
        Param->setInvalidDecl();
        S.Diags.Report(Param->getLocation(), diag::err_hlsl_array_disallowed)
            << ParamTy << /*entry parameter*/ 0;
        if (Kind == DXIL::NodeIOKind::NodeOutput ||
            Kind == DXIL::NodeIOKind::EmptyOutput)
          S.Diags.Report(Param->getLocation(), diag::note_hlsl_node_array)
              << HLSLNodeObjectAttr::ConvertRecordTypeToStr(Kind);
      }
    }
    HLSLMaxRecordsSharedWithAttr *ExistingMRSWA =
        Param->getAttr<HLSLMaxRecordsSharedWithAttr>();
    if (ExistingMRSWA) {
      StringRef sharedName = ExistingMRSWA->getName()->getName();
      bool Found = false;
      for (const ParmVarDecl *ParamDecl : FD->params()) {
        // validation that MRSW doesn't reference its own parameter is
        // already done at
        // SemaHLSL.cpp:ValidateMaxRecordsSharedWithAttributes so we don't
        // need to check that we are on the same argument.
        if (ParamDecl->getName() == sharedName) {
          // now we need to check that this parameter has an output record type.
          hlsl::NodeFlags nodeFlags;
          if (GetHLSLNodeIORecordType(ParamDecl, nodeFlags)) {
            hlsl::NodeIOProperties node(nodeFlags);
            if (node.Flags.IsOutputNode()) {
              Found = true;
              break;
            }
          }
        }
      }

      if (!Found) {
        S.Diag(ExistingMRSWA->getLocation(),
               diag::err_hlsl_maxrecordssharedwith_references_invalid_arg);
      }
    }

    // Make sure NodeTrackRWInputSharing attribute cannot be applied to
    // Input Records that are not RWDispatchNodeInputRecord
    if (hlsl::IsHLSLNodeInputType(ParamTy)) {
      hlsl::NodeFlags nodeFlags;
      if (GetHLSLNodeIORecordType(Param, nodeFlags)) {
        hlsl::NodeIOProperties node(nodeFlags);

        // determine if the NodeTrackRWInputSharing is an attribute on the
        // template type
        clang::RecordDecl *RD = hlsl::GetRecordDeclFromNodeObjectType(ParamTy);
        if (RD) {
          // Emit a diagnostic if the record is not RWDispatchNode and
          // if it has the NodeTrackRWInputSharing attribute
          if (RD->hasAttr<HLSLNodeTrackRWInputSharingAttr>() &&
              node.Flags.GetNodeIOKind() !=
                  DXIL::NodeIOKind::RWDispatchNodeInputRecord) {
            S.Diags.Report(Param->getLocation(),
                           diag::err_hlsl_wg_nodetrackrwinputsharing_invalid);
          }
        }
      }
    }
  }

  DiagnoseSVForLaunchType(FD, NodeLaunchTy, S.Diags);

  return;
}

// if this is the Entry FD, then try adding the target profile
// shader attribute to the FD and carry on with validation
void TryAddShaderAttrFromTargetProfile(Sema &S, FunctionDecl *FD,
                                       bool &isActiveEntry) {
  // When isActiveEntry is true and this function is an entry point, this entry
  // point is used in compilation. This is an important distinction when
  // diagnosing certain types of errors based on the compilation parameters. For
  // example, if isActiveEntry is false, diagnostics dependent on the shader
  // model should not be performed. That way we won't raise an error about a
  // feature used by the inactive entry that's not available in the current
  // shader model. Since that entry point is not used, it may still be valid in
  // another compilation where a different shader model is specified.
  isActiveEntry = false;
  const std::string &EntryPointName = S.getLangOpts().HLSLEntryFunction;

  // if there's no defined entry point, just return
  if (EntryPointName.empty()) {
    return;
  }

  // if this FD isn't the entry point, then we shouldn't add
  // a shader attribute to this decl, so just return
  if (!FD->getIdentifier() ||
      EntryPointName != FD->getIdentifier()->getName()) {
    return;
  }

  isActiveEntry = true;

  std::string profile = S.getLangOpts().HLSLProfile;
  const ShaderModel *SM = hlsl::ShaderModel::GetByName(profile.c_str());
  const llvm::StringRef fullName = ShaderModel::FullNameFromKind(SM->GetKind());

  // don't add the attribute for an invalid profile, like library
  if (fullName.empty()) {
    llvm_unreachable("invalid shader kind");
  }

  // At this point, we've found the active entry, so we'll take a note of that
  // and try to add the shader attr.
  isActiveEntry = true;

  HLSLShaderAttr *currentShaderAttr = FD->getAttr<HLSLShaderAttr>();
  // Don't add the attribute if it already exists as an attribute on the decl,
  // and emit an error.
  if (currentShaderAttr) {
    llvm::StringRef currentFullName = currentShaderAttr->getStage();
    if (currentFullName != fullName) {

      S.Diag(currentShaderAttr->getLocation(),
             diag::err_hlsl_profile_conflicts_with_shader_attribute)
          << fullName << profile << currentFullName << EntryPointName;
    }
    // Don't add another attr if one exists, to prevent
    // more unrelated errors down the line.
    return;
  }

  HLSLShaderAttr *pShaderAttr =
      HLSLShaderAttr::CreateImplicit(S.Context, fullName);

  FD->addAttr(pShaderAttr);
  return;
}

// The compiler should emit a warning when an entry-point-only attribute
// is detected without the presence of a shader attribute,
// to prevent reliance on deprecated behavior
// (where the compiler would infer a specific shader kind based on
// a present entry-point-only attribute).
void WarnOnEntryAttrWithoutShaderAttr(Sema &S, FunctionDecl *FD) {
  if (!FD->hasAttrs())
    return;
  for (Attr *A : FD->getAttrs()) {
    switch (A->getKind()) {
      // Entry-Function-only attributes
    case clang::attr::HLSLClipPlanes:
    case clang::attr::HLSLDomain:
    case clang::attr::HLSLEarlyDepthStencil:
    case clang::attr::HLSLInstance:
    case clang::attr::HLSLMaxTessFactor:
    case clang::attr::HLSLNumThreads:
    case clang::attr::HLSLRootSignature:
    case clang::attr::HLSLOutputControlPoints:
    case clang::attr::HLSLOutputTopology:
    case clang::attr::HLSLPartitioning:
    case clang::attr::HLSLPatchConstantFunc:
    case clang::attr::HLSLMaxVertexCount:
    case clang::attr::HLSLWaveSize:
    case clang::attr::HLSLNodeLaunch:
    case clang::attr::HLSLNodeIsProgramEntry:
    case clang::attr::HLSLNodeId:
    case clang::attr::HLSLNodeLocalRootArgumentsTableIndex:
    case clang::attr::HLSLNodeShareInputOf:
    case clang::attr::HLSLNodeDispatchGrid:
    case clang::attr::HLSLNodeMaxDispatchGrid:
    case clang::attr::HLSLNodeMaxRecursionDepth:
      S.Diag(A->getLocation(),
             diag::warn_hlsl_entry_attribute_without_shader_attribute)
          << A->getSpelling();
      break;
    }
  }
  return;
}

// The DiagnoseEntry function does 2 things:
// 1. Determine whether this function is the current entry point for a
// non-library compilation, add an implicit shader attribute if so.
// 2. For an entry point function, now identified by the shader attribute,
// diagnose entry point constraints:
//   a. Diagnose whether or not all entry point attributes on the decl are
//   allowed on the entry point type (ShaderKind) at all.
//   b. Diagnose the full entry point decl for required attributes, constraints
//   on or between attributes and parameters, and more.
void DiagnoseEntry(Sema &S, FunctionDecl *FD) {
  bool isActiveEntry = false;
  if (S.getLangOpts().IsHLSLLibrary) {
    // TODO: Analyze -exports option to determine which entries
    // are active for lib target.
    // For now, assume all entries are active.
    isActiveEntry = true;
  } else {
    TryAddShaderAttrFromTargetProfile(S, FD, isActiveEntry);
  }

  HLSLShaderAttr *shaderAttr = FD->getAttr<HLSLShaderAttr>();
  if (!shaderAttr) {
    if (S.getLangOpts().IsHLSLLibrary)
      WarnOnEntryAttrWithoutShaderAttr(S, FD);

    return;
  }

  // Check general parameter characteristics
  // Would be nice to check for resources here as they crash the compiler now.
  // See issue #7186.
  for (const auto *param : FD->params()) {
    const TypeDiagContext DiagContext =
        TypeDiagContext::EntryFunctionParameters;
    hlsl::DiagnoseTypeElements(S, param->getLocation(), param->getType(),
                               DiagContext, DiagContext);
  }

  const TypeDiagContext DiagContext = TypeDiagContext::EntryFunctionReturnType;
  DiagnoseTypeElements(S, FD->getLocation(), FD->getReturnType(), DiagContext,
                       DiagContext);

  DXIL::ShaderKind Stage =
      ShaderModel::KindFromFullName(shaderAttr->getStage());
  llvm::StringRef StageName = shaderAttr->getStage();
  DiagnoseEntryAttrAllowedOnStage(&S, FD, Stage);

  switch (Stage) {
  case DXIL::ShaderKind::Vertex:
    return DiagnoseVertexEntry(S, FD, StageName);
  case DXIL::ShaderKind::Pixel:
  case DXIL::ShaderKind::Library:
  case DXIL::ShaderKind::Invalid:
    return;
  case DXIL::ShaderKind::Amplification: {
    return DiagnoseAmplificationEntry(S, FD, StageName);
  }
  case DXIL::ShaderKind::Mesh: {
    return DiagnoseMeshEntry(S, FD, StageName);
  }
  case DXIL::ShaderKind::Domain:
    return DiagnoseDomainEntry(S, FD, StageName);
  case DXIL::ShaderKind::Hull: {
    return DiagnoseHullEntry(S, FD, StageName);
  }
  case DXIL::ShaderKind::Geometry: {
    return DiagnoseGeometryEntry(S, FD, StageName);
  }
  case DXIL::ShaderKind::Callable: {
    return DiagnoseCallableEntry(S, FD, StageName);
  }
  case DXIL::ShaderKind::Miss:
  case DXIL::ShaderKind::AnyHit: {
    return DiagnoseMissOrAnyHitEntry(S, FD, StageName, Stage);
  }
  case DXIL::ShaderKind::RayGeneration:
  case DXIL::ShaderKind::Intersection: {
    return DiagnoseRayGenerationOrIntersectionEntry(S, FD, StageName);
  }
  case DXIL::ShaderKind::ClosestHit: {
    return DiagnoseClosestHitEntry(S, FD, StageName);
  }
  case DXIL::ShaderKind::Compute: {
    return DiagnoseComputeEntry(S, FD, StageName, isActiveEntry);
  }

  case DXIL::ShaderKind::Node: {
    // A compute shader may also be a node, so we check it here
    return DiagnoseNodeEntry(S, FD, StageName, isActiveEntry);
  }
  }
}
} // namespace hlsl
