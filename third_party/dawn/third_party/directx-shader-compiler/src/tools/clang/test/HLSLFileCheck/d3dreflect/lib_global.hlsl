// RUN: %dxc -T lib_6_3 -enable-16bit-types -Vd -validator-version 0.0 %s | %D3DReflect %s | FileCheck %s
// RUN: %dxc -T lib_6_6 -enable-16bit-types -Vd -validator-version 0.0 %s | %D3DReflect %s | FileCheck %s

// Note: validator version 1.5 is required because these tests use
// module disassembly -> reassembly between steps, and type annotations
// compatible with the 1.4 validator does not have usage metadata, so it's lost.

// Make sure CB usage is correct.
#if 0
// CHECK: ID3D12LibraryReflection:
// CHECK-NEXT:   D3D12_LIBRARY_DESC:
// CHECK:     Flags: 0
// CHECK-NEXT:     FunctionCount: 2
// CHECK-NEXT:   ID3D12FunctionReflection:
// CHECK-NEXT:     D3D12_FUNCTION_DESC: Name: _GLOBAL__sub_I_lib_global.hlsl
// CHECK-NEXT:       Shader Version: Library
// CHECK:       Flags: 0
// CHECK-NEXT:       RequiredFeatureFlags: 0x40000
// CHECK-NEXT:       ConstantBuffers: 2
// CHECK-NEXT:       BoundResources: 2
// CHECK-NEXT:       FunctionParameterCount: 0
// CHECK-NEXT:       HasReturn: FALSE
// CHECK-NEXT:     Constant Buffers:
// CHECK-NEXT:       ID3D12ShaderReflectionConstantBuffer:
// CHECK-NEXT:         D3D12_SHADER_BUFFER_DESC: Name: X
// CHECK-NEXT:           Type: D3D_CT_CBUFFER
// CHECK-NEXT:           Size: 16
// CHECK-NEXT:           uFlags: 0
// CHECK-NEXT:           Num Variables: 2
// CHECK-NEXT:         {
// CHECK-NEXT:           ID3D12ShaderReflectionVariable:
// CHECK-NEXT:             D3D12_SHADER_VARIABLE_DESC: Name: e
// CHECK-NEXT:               Size: 2
// CHECK-NEXT:               StartOffset: 0
// CHECK-NEXT:               uFlags: 0
// CHECK-NEXT:               DefaultValue: <nullptr>
// CHECK-NEXT:             ID3D12ShaderReflectionType:
// CHECK-NEXT:               D3D12_SHADER_TYPE_DESC: Name: float16_t
// CHECK-NEXT:                 Class: D3D_SVC_SCALAR
// CHECK-NEXT:                 Type: D3D_SVT_FLOAT16
// CHECK-NEXT:                 Elements: 0
// CHECK-NEXT:                 Rows: 1
// CHECK-NEXT:                 Columns: 1
// CHECK-NEXT:                 Members: 0
// CHECK-NEXT:                 Offset: 0
// CHECK-NEXT:             CBuffer: X
// CHECK-NEXT:           ID3D12ShaderReflectionVariable:
// CHECK-NEXT:             D3D12_SHADER_VARIABLE_DESC: Name: f
// CHECK-NEXT:               Size: 2
// CHECK-NEXT:               StartOffset: 2
// CHECK-NEXT:               uFlags: (D3D_SVF_USED)
// CHECK-NEXT:               DefaultValue: <nullptr>
// CHECK-NEXT:             ID3D12ShaderReflectionType:
// CHECK-NEXT:               D3D12_SHADER_TYPE_DESC: Name: float16_t
// CHECK-NEXT:                 Class: D3D_SVC_SCALAR
// CHECK-NEXT:                 Type: D3D_SVT_FLOAT16
// CHECK-NEXT:                 Elements: 0
// CHECK-NEXT:                 Rows: 1
// CHECK-NEXT:                 Columns: 1
// CHECK-NEXT:                 Members: 0
// CHECK-NEXT:                 Offset: 0
// CHECK-NEXT:             CBuffer: X
// CHECK-NEXT:         }
// CHECK-NEXT:       ID3D12ShaderReflectionConstantBuffer:
// CHECK-NEXT:         D3D12_SHADER_BUFFER_DESC: Name: X
// CHECK-NEXT:           Type: D3D_CT_RESOURCE_BIND_INFO
// CHECK-NEXT:           Size: 72
// CHECK-NEXT:           uFlags: 0
// CHECK-NEXT:           Num Variables: 1
// CHECK-NEXT:         {
// CHECK-NEXT:           ID3D12ShaderReflectionVariable:
// CHECK-NEXT:             D3D12_SHADER_VARIABLE_DESC: Name: $Element
// CHECK-NEXT:               Size: 72
// CHECK-NEXT:               StartOffset: 0
// CHECK-NEXT:               uFlags: (D3D_SVF_USED)
// CHECK-NEXT:               DefaultValue: <nullptr>
// CHECK-NEXT:             ID3D12ShaderReflectionType:
// CHECK-NEXT:               D3D12_SHADER_TYPE_DESC: Name: SBStruct
// CHECK-NEXT:                 Class: D3D_SVC_STRUCT
// CHECK-NEXT:                 Type: D3D_SVT_VOID
// CHECK-NEXT:                 Elements: 0
// CHECK-NEXT:                 Rows: 1
// CHECK-NEXT:                 Columns: 11
// CHECK-NEXT:                 Members: 8
// CHECK-NEXT:                 Offset: 0
// CHECK-NEXT:               {
// CHECK-NEXT:                 ID3D12ShaderReflectionType:
// CHECK-NEXT:                   D3D12_SHADER_TYPE_DESC: Name: int16_t
// CHECK-NEXT:                     Class: D3D_SVC_SCALAR
// CHECK-NEXT:                     Type: D3D_SVT_INT16
// CHECK-NEXT:                     Elements: 0
// CHECK-NEXT:                     Rows: 1
// CHECK-NEXT:                     Columns: 1
// CHECK-NEXT:                     Members: 0
// CHECK-NEXT:                     Offset: 0
// CHECK-NEXT:                 ID3D12ShaderReflectionType:
// CHECK-NEXT:                   D3D12_SHADER_TYPE_DESC: Name: uint64_t
// CHECK-NEXT:                     Class: D3D_SVC_SCALAR
// CHECK-NEXT:                     Type: D3D_SVT_UINT64
// CHECK-NEXT:                     Elements: 0
// CHECK-NEXT:                     Rows: 1
// CHECK-NEXT:                     Columns: 1
// CHECK-NEXT:                     Members: 0
// CHECK-NEXT:                     Offset: 8
// CHECK-NEXT:                 ID3D12ShaderReflectionType:
// CHECK-NEXT:                   D3D12_SHADER_TYPE_DESC: Name: int16_t
// CHECK-NEXT:                     Class: D3D_SVC_SCALAR
// CHECK-NEXT:                     Type: D3D_SVT_INT16
// CHECK-NEXT:                     Elements: 0
// CHECK-NEXT:                     Rows: 1
// CHECK-NEXT:                     Columns: 1
// CHECK-NEXT:                     Members: 0
// CHECK-NEXT:                     Offset: 16
// CHECK-NEXT:                 ID3D12ShaderReflectionType:
// CHECK-NEXT:                   D3D12_SHADER_TYPE_DESC: Name: int64_t
// CHECK-NEXT:                     Class: D3D_SVC_SCALAR
// CHECK-NEXT:                     Type: D3D_SVT_INT64
// CHECK-NEXT:                     Elements: 0
// CHECK-NEXT:                     Rows: 1
// CHECK-NEXT:                     Columns: 1
// CHECK-NEXT:                     Members: 0
// CHECK-NEXT:                     Offset: 24
// CHECK-NEXT:                 ID3D12ShaderReflectionType:
// CHECK-NEXT:                   D3D12_SHADER_TYPE_DESC: Name: float4
// CHECK-NEXT:                     Class: D3D_SVC_VECTOR
// CHECK-NEXT:                     Type: D3D_SVT_FLOAT
// CHECK-NEXT:                     Elements: 0
// CHECK-NEXT:                     Rows: 1
// CHECK-NEXT:                     Columns: 4
// CHECK-NEXT:                     Members: 0
// CHECK-NEXT:                     Offset: 32
// CHECK-NEXT:                 ID3D12ShaderReflectionType:
// CHECK-NEXT:                   D3D12_SHADER_TYPE_DESC: Name: int16_t
// CHECK-NEXT:                     Class: D3D_SVC_SCALAR
// CHECK-NEXT:                     Type: D3D_SVT_INT16
// CHECK-NEXT:                     Elements: 0
// CHECK-NEXT:                     Rows: 1
// CHECK-NEXT:                     Columns: 1
// CHECK-NEXT:                     Members: 0
// CHECK-NEXT:                     Offset: 48
// CHECK-NEXT:                 ID3D12ShaderReflectionType:
// CHECK-NEXT:                   D3D12_SHADER_TYPE_DESC: Name: double
// CHECK-NEXT:                     Class: D3D_SVC_SCALAR
// CHECK-NEXT:                     Type: D3D_SVT_DOUBLE
// CHECK-NEXT:                     Elements: 0
// CHECK-NEXT:                     Rows: 1
// CHECK-NEXT:                     Columns: 1
// CHECK-NEXT:                     Members: 0
// CHECK-NEXT:                     Offset: 56
// CHECK-NEXT:                 ID3D12ShaderReflectionType:
// CHECK-NEXT:                   D3D12_SHADER_TYPE_DESC: Name: uint16_t
// CHECK-NEXT:                     Class: D3D_SVC_SCALAR
// CHECK-NEXT:                     Type: D3D_SVT_UINT16
// CHECK-NEXT:                     Elements: 0
// CHECK-NEXT:                     Rows: 1
// CHECK-NEXT:                     Columns: 1
// CHECK-NEXT:                     Members: 0
// CHECK-NEXT:                     Offset: 64
// CHECK-NEXT:               }
// CHECK-NEXT:             CBuffer: X
// CHECK-NEXT:         }
// CHECK-NEXT:     Bound Resources:
// CHECK-NEXT:       D3D12_SHADER_INPUT_BIND_DESC: Name: X
// CHECK-NEXT:         Type: D3D_SIT_CBUFFER
// CHECK-NEXT:         uID: 0
// CHECK-NEXT:         BindCount: 1
// CHECK-NEXT:         BindPoint: 4294967295
// CHECK-NEXT:         Space: 4294967295
// CHECK-NEXT:         ReturnType: <unknown: 0>
// CHECK-NEXT:         Dimension: D3D_SRV_DIMENSION_UNKNOWN
// CHECK-NEXT:         NumSamples (or stride): 0
// CHECK-NEXT:         uFlags: (D3D_SIF_USERPACKED)
// CHECK-NEXT:       D3D12_SHADER_INPUT_BIND_DESC: Name: X
// CHECK-NEXT:         Type: D3D_SIT_STRUCTURED
// CHECK-NEXT:         uID: 1
// CHECK-NEXT:         BindCount: 1
// CHECK-NEXT:         BindPoint: 4294967295
// CHECK-NEXT:         Space: 4294967295
// CHECK-NEXT:         ReturnType: D3D_RETURN_TYPE_MIXED
// CHECK-NEXT:         Dimension: D3D_SRV_DIMENSION_BUFFER
// CHECK-NEXT:         NumSamples (or stride): 72
// CHECK-NEXT:         uFlags: 0
// CHECK-NEXT:   ID3D12FunctionReflection:
// CHECK-NEXT:     D3D12_FUNCTION_DESC: Name: test
// CHECK-NEXT:       Shader Version: Pixel
// CHECK:       Flags: 0
// CHECK-NEXT:       RequiredFeatureFlags: 0x40000
// CHECK-NEXT:       ConstantBuffers: 2
// CHECK-NEXT:       BoundResources: 4
// CHECK-NEXT:       FunctionParameterCount: 0
// CHECK-NEXT:       HasReturn: FALSE
// CHECK-NEXT:     Constant Buffers:
// CHECK-NEXT:       ID3D12ShaderReflectionConstantBuffer:
// CHECK-NEXT:         D3D12_SHADER_BUFFER_DESC: Name: X
// CHECK-NEXT:           Type: D3D_CT_CBUFFER
// CHECK-NEXT:           Size: 16
// CHECK-NEXT:           uFlags: 0
// CHECK-NEXT:           Num Variables: 2
// CHECK-NEXT:         {
// CHECK-NEXT:           ID3D12ShaderReflectionVariable:
// CHECK-NEXT:             D3D12_SHADER_VARIABLE_DESC: Name: e
// CHECK-NEXT:               Size: 2
// CHECK-NEXT:               StartOffset: 0
// CHECK-NEXT:               uFlags: 0
// CHECK-NEXT:               DefaultValue: <nullptr>
// CHECK-NEXT:             ID3D12ShaderReflectionType:
// CHECK-NEXT:               D3D12_SHADER_TYPE_DESC: Name: float16_t
// CHECK-NEXT:                 Class: D3D_SVC_SCALAR
// CHECK-NEXT:                 Type: D3D_SVT_FLOAT16
// CHECK-NEXT:                 Elements: 0
// CHECK-NEXT:                 Rows: 1
// CHECK-NEXT:                 Columns: 1
// CHECK-NEXT:                 Members: 0
// CHECK-NEXT:                 Offset: 0
// CHECK-NEXT:             CBuffer: X
// CHECK-NEXT:           ID3D12ShaderReflectionVariable:
// CHECK-NEXT:             D3D12_SHADER_VARIABLE_DESC: Name: f
// CHECK-NEXT:               Size: 2
// CHECK-NEXT:               StartOffset: 2
// CHECK-NEXT:               uFlags: (D3D_SVF_USED)
// CHECK-NEXT:               DefaultValue: <nullptr>
// CHECK-NEXT:             ID3D12ShaderReflectionType:
// CHECK-NEXT:               D3D12_SHADER_TYPE_DESC: Name: float16_t
// CHECK-NEXT:                 Class: D3D_SVC_SCALAR
// CHECK-NEXT:                 Type: D3D_SVT_FLOAT16
// CHECK-NEXT:                 Elements: 0
// CHECK-NEXT:                 Rows: 1
// CHECK-NEXT:                 Columns: 1
// CHECK-NEXT:                 Members: 0
// CHECK-NEXT:                 Offset: 0
// CHECK-NEXT:             CBuffer: X
// CHECK-NEXT:         }
// CHECK-NEXT:       ID3D12ShaderReflectionConstantBuffer:
// CHECK-NEXT:         D3D12_SHADER_BUFFER_DESC: Name: X
// CHECK-NEXT:           Type: D3D_CT_RESOURCE_BIND_INFO
// CHECK-NEXT:           Size: 72
// CHECK-NEXT:           uFlags: 0
// CHECK-NEXT:           Num Variables: 1
// CHECK-NEXT:         {
// CHECK-NEXT:           ID3D12ShaderReflectionVariable:
// CHECK-NEXT:             D3D12_SHADER_VARIABLE_DESC: Name: $Element
// CHECK-NEXT:               Size: 72
// CHECK-NEXT:               StartOffset: 0
// CHECK-NEXT:               uFlags: (D3D_SVF_USED)
// CHECK-NEXT:               DefaultValue: <nullptr>
// CHECK-NEXT:             ID3D12ShaderReflectionType:
// CHECK-NEXT:               D3D12_SHADER_TYPE_DESC: Name: SBStruct
// CHECK-NEXT:                 Class: D3D_SVC_STRUCT
// CHECK-NEXT:                 Type: D3D_SVT_VOID
// CHECK-NEXT:                 Elements: 0
// CHECK-NEXT:                 Rows: 1
// CHECK-NEXT:                 Columns: 11
// CHECK-NEXT:                 Members: 8
// CHECK-NEXT:                 Offset: 0
// CHECK-NEXT:               {
// CHECK-NEXT:                 ID3D12ShaderReflectionType:
// CHECK-NEXT:                   D3D12_SHADER_TYPE_DESC: Name: int16_t
// CHECK-NEXT:                     Class: D3D_SVC_SCALAR
// CHECK-NEXT:                     Type: D3D_SVT_INT16
// CHECK-NEXT:                     Elements: 0
// CHECK-NEXT:                     Rows: 1
// CHECK-NEXT:                     Columns: 1
// CHECK-NEXT:                     Members: 0
// CHECK-NEXT:                     Offset: 0
// CHECK-NEXT:                 ID3D12ShaderReflectionType:
// CHECK-NEXT:                   D3D12_SHADER_TYPE_DESC: Name: uint64_t
// CHECK-NEXT:                     Class: D3D_SVC_SCALAR
// CHECK-NEXT:                     Type: D3D_SVT_UINT64
// CHECK-NEXT:                     Elements: 0
// CHECK-NEXT:                     Rows: 1
// CHECK-NEXT:                     Columns: 1
// CHECK-NEXT:                     Members: 0
// CHECK-NEXT:                     Offset: 8
// CHECK-NEXT:                 ID3D12ShaderReflectionType:
// CHECK-NEXT:                   D3D12_SHADER_TYPE_DESC: Name: int16_t
// CHECK-NEXT:                     Class: D3D_SVC_SCALAR
// CHECK-NEXT:                     Type: D3D_SVT_INT16
// CHECK-NEXT:                     Elements: 0
// CHECK-NEXT:                     Rows: 1
// CHECK-NEXT:                     Columns: 1
// CHECK-NEXT:                     Members: 0
// CHECK-NEXT:                     Offset: 16
// CHECK-NEXT:                 ID3D12ShaderReflectionType:
// CHECK-NEXT:                   D3D12_SHADER_TYPE_DESC: Name: int64_t
// CHECK-NEXT:                     Class: D3D_SVC_SCALAR
// CHECK-NEXT:                     Type: D3D_SVT_INT64
// CHECK-NEXT:                     Elements: 0
// CHECK-NEXT:                     Rows: 1
// CHECK-NEXT:                     Columns: 1
// CHECK-NEXT:                     Members: 0
// CHECK-NEXT:                     Offset: 24
// CHECK-NEXT:                 ID3D12ShaderReflectionType:
// CHECK-NEXT:                   D3D12_SHADER_TYPE_DESC: Name: float4
// CHECK-NEXT:                     Class: D3D_SVC_VECTOR
// CHECK-NEXT:                     Type: D3D_SVT_FLOAT
// CHECK-NEXT:                     Elements: 0
// CHECK-NEXT:                     Rows: 1
// CHECK-NEXT:                     Columns: 4
// CHECK-NEXT:                     Members: 0
// CHECK-NEXT:                     Offset: 32
// CHECK-NEXT:                 ID3D12ShaderReflectionType:
// CHECK-NEXT:                   D3D12_SHADER_TYPE_DESC: Name: int16_t
// CHECK-NEXT:                     Class: D3D_SVC_SCALAR
// CHECK-NEXT:                     Type: D3D_SVT_INT16
// CHECK-NEXT:                     Elements: 0
// CHECK-NEXT:                     Rows: 1
// CHECK-NEXT:                     Columns: 1
// CHECK-NEXT:                     Members: 0
// CHECK-NEXT:                     Offset: 48
// CHECK-NEXT:                 ID3D12ShaderReflectionType:
// CHECK-NEXT:                   D3D12_SHADER_TYPE_DESC: Name: double
// CHECK-NEXT:                     Class: D3D_SVC_SCALAR
// CHECK-NEXT:                     Type: D3D_SVT_DOUBLE
// CHECK-NEXT:                     Elements: 0
// CHECK-NEXT:                     Rows: 1
// CHECK-NEXT:                     Columns: 1
// CHECK-NEXT:                     Members: 0
// CHECK-NEXT:                     Offset: 56
// CHECK-NEXT:                 ID3D12ShaderReflectionType:
// CHECK-NEXT:                   D3D12_SHADER_TYPE_DESC: Name: uint16_t
// CHECK-NEXT:                     Class: D3D_SVC_SCALAR
// CHECK-NEXT:                     Type: D3D_SVT_UINT16
// CHECK-NEXT:                     Elements: 0
// CHECK-NEXT:                     Rows: 1
// CHECK-NEXT:                     Columns: 1
// CHECK-NEXT:                     Members: 0
// CHECK-NEXT:                     Offset: 64
// CHECK-NEXT:               }
// CHECK-NEXT:             CBuffer: X
// CHECK-NEXT:         }
// CHECK-NEXT:     Bound Resources:
// CHECK-NEXT:       D3D12_SHADER_INPUT_BIND_DESC: Name: X
// CHECK-NEXT:         Type: D3D_SIT_CBUFFER
// CHECK-NEXT:         uID: 0
// CHECK-NEXT:         BindCount: 1
// CHECK-NEXT:         BindPoint: 4294967295
// CHECK-NEXT:         Space: 4294967295
// CHECK-NEXT:         ReturnType: <unknown: 0>
// CHECK-NEXT:         Dimension: D3D_SRV_DIMENSION_UNKNOWN
// CHECK-NEXT:         NumSamples (or stride): 0
// CHECK-NEXT:         uFlags: (D3D_SIF_USERPACKED)
// CHECK-NEXT:       D3D12_SHADER_INPUT_BIND_DESC: Name: g_samLinear
// CHECK-NEXT:         Type: D3D_SIT_SAMPLER
// CHECK-NEXT:         uID: 0
// CHECK-NEXT:         BindCount: 1
// CHECK-NEXT:         BindPoint: 4294967295
// CHECK-NEXT:         Space: 4294967295
// CHECK-NEXT:         ReturnType: <unknown: 0>
// CHECK-NEXT:         Dimension: D3D_SRV_DIMENSION_UNKNOWN
// CHECK-NEXT:         NumSamples (or stride): 0
// CHECK-NEXT:         uFlags: 0
// CHECK-NEXT:       D3D12_SHADER_INPUT_BIND_DESC: Name: g_txDiffuse
// CHECK-NEXT:         Type: D3D_SIT_TEXTURE
// CHECK-NEXT:         uID: 0
// CHECK-NEXT:         BindCount: 1
// CHECK-NEXT:         BindPoint: 4294967295
// CHECK-NEXT:         Space: 4294967295
// CHECK-NEXT:         ReturnType: D3D_RETURN_TYPE_FLOAT
// CHECK-NEXT:         Dimension: D3D_SRV_DIMENSION_TEXTURE2D
// CHECK-NEXT:         NumSamples (or stride): 4294967295
// CHECK-NEXT:         uFlags: (D3D_SIF_TEXTURE_COMPONENT_0 | D3D_SIF_TEXTURE_COMPONENT_1)
// CHECK-NEXT:       D3D12_SHADER_INPUT_BIND_DESC: Name: X
// CHECK-NEXT:         Type: D3D_SIT_STRUCTURED
// CHECK-NEXT:         uID: 1
// CHECK-NEXT:         BindCount: 1
// CHECK-NEXT:         BindPoint: 4294967295
// CHECK-NEXT:         Space: 4294967295
// CHECK-NEXT:         ReturnType: D3D_RETURN_TYPE_MIXED
// CHECK-NEXT:         Dimension: D3D_SRV_DIMENSION_BUFFER
// CHECK-NEXT:         NumSamples (or stride): 72
// CHECK-NEXT:         uFlags: 0
#endif

Texture2D    g_txDiffuse;
SamplerState    g_samLinear;

struct SBStruct {
  int16_t pad0; // ensure alignment of next to 8-bytes
  uint64_t u64;
  int16_t pad1; // ensure alignment of next to 8-bytes
  int64_t i64;
  float4 f4;
  int16_t i16;
  double d;     // Here to test alignment
  uint16_t u16;
};

// Note legal name collision with cbuffer X
StructuredBuffer<SBStruct> X;

cbuffer X {
  half e, f;
}

static float g[2] = { 1, f + X[0].f4.z };

[shader("pixel")]
float4 test(float2 c : C) : SV_TARGET
{
  float4 x = g_txDiffuse.Sample( g_samLinear, c );
  return x + g[1] + X[0].f4;
}

void update() {
  g[1]++;
}