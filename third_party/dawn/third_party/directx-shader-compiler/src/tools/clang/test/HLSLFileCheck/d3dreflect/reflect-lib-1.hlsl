// RUN: %dxc -T lib_6_3 -auto-binding-space 11 -default-linkage external -Vd -validator-version 0.0 %s | %D3DReflect %s | FileCheck %s
// RUN: %dxc -T lib_6_6 -auto-binding-space 11 -default-linkage external -Vd -validator-version 0.0 %s | %D3DReflect %s | FileCheck %s

float cbval1;
cbuffer MyCB : register(b11, space2) { int4 cbval2, cbval3; }
RWTexture1D<int4> tex : register(u5);
Texture1D<float4> tex2 : register(t0);
SamplerState samp : register(s7);
RWByteAddressBuffer b_buf;
float function0(min16float x) { 
  return x + cbval2.x + tex[0].x; }
float function1(float x, min12int i) {
  return x + cbval1 + b_buf.Load(x) + tex2.Sample(samp, x).x; }
[shader("vertex")]
float4 function2(float4 x : POSITION) : SV_Position { return x + cbval1 + cbval3.x; }


// CHECK: ID3D12LibraryReflection:
// CHECK:   D3D12_LIBRARY_DESC:
// CHECK:     FunctionCount: 3
// CHECK:   ID3D12FunctionReflection:
// CHECK:     D3D12_FUNCTION_DESC: Name: \01?function0{{[@$?.A-Za-z0-9_]+}}
// CHECK:       Shader Version: Library
// CHECK:       ConstantBuffers: 1
// CHECK:       BoundResources: 2
// CHECK:     Constant Buffers:
// CHECK:       ID3D12ShaderReflectionConstantBuffer:
// CHECK:         D3D12_SHADER_BUFFER_DESC: Name: MyCB
// CHECK:           Type: D3D_CT_CBUFFER
// CHECK:           Size: 32
// CHECK:           Num Variables: 2
// CHECK:         {
// CHECK:           ID3D12ShaderReflectionVariable:
// CHECK:             D3D12_SHADER_VARIABLE_DESC: Name: cbval2
// CHECK:               Size: 16
// CHECK:               uFlags: (D3D_SVF_USED)
// CHECK:             ID3D12ShaderReflectionType:
// CHECK:               D3D12_SHADER_TYPE_DESC: Name: int4
// CHECK:                 Class: D3D_SVC_VECTOR
// CHECK:                 Type: D3D_SVT_INT
// CHECK:                 Rows: 1
// CHECK:                 Columns: 4
// CHECK:             CBuffer: MyCB
// CHECK:           ID3D12ShaderReflectionVariable:
// CHECK:             D3D12_SHADER_VARIABLE_DESC: Name: cbval3
// CHECK:               Size: 16
// CHECK:               StartOffset: 16
// CHECK:               uFlags: (D3D_SVF_USED)
// CHECK:             ID3D12ShaderReflectionType:
// CHECK:               D3D12_SHADER_TYPE_DESC: Name: int4
// CHECK:                 Class: D3D_SVC_VECTOR
// CHECK:                 Type: D3D_SVT_INT
// CHECK:                 Rows: 1
// CHECK:                 Columns: 4
// CHECK:             CBuffer: MyCB
// CHECK:         }
// CHECK:     Bound Resources:
// CHECK:       D3D12_SHADER_INPUT_BIND_DESC: Name: MyCB
// CHECK:         Type: D3D_SIT_CBUFFER
// CHECK:         uID: 1
// CHECK:         BindPoint: 11
// CHECK:         Space: 2
// CHECK:         Dimension: D3D_SRV_DIMENSION_UNKNOWN
// CHECK:       D3D12_SHADER_INPUT_BIND_DESC: Name: tex
// CHECK:         Type: D3D_SIT_UAV_RWTYPED
// CHECK:         uID: 0
// CHECK:         BindPoint: 5
// CHECK:         ReturnType: D3D_RETURN_TYPE_SINT
// CHECK:         Dimension: D3D_SRV_DIMENSION_TEXTURE1D
// CHECK:         uFlags: (D3D_SIF_TEXTURE_COMPONENT_0 | D3D_SIF_TEXTURE_COMPONENT_1)
// CHECK:   ID3D12FunctionReflection:
// CHECK:     D3D12_FUNCTION_DESC: Name: \01?function1{{[@$?.A-Za-z0-9_]+}}
// CHECK:       Shader Version: Library
// CHECK:       ConstantBuffers: 1
// CHECK:       BoundResources: 4
// CHECK:     Constant Buffers:
// CHECK:       ID3D12ShaderReflectionConstantBuffer:
// CHECK:         D3D12_SHADER_BUFFER_DESC: Name: $Globals
// CHECK:           Type: D3D_CT_CBUFFER
// CHECK:           Size: 16
// CHECK:           Num Variables: 1
// CHECK:         {
// CHECK:           ID3D12ShaderReflectionVariable:
// CHECK:             D3D12_SHADER_VARIABLE_DESC: Name: cbval1
// CHECK:               Size: 4
// CHECK:               uFlags: (D3D_SVF_USED)
// CHECK:             ID3D12ShaderReflectionType:
// CHECK:               D3D12_SHADER_TYPE_DESC: Name: float
// CHECK:                 Class: D3D_SVC_SCALAR
// CHECK:                 Type: D3D_SVT_FLOAT
// CHECK:                 Rows: 1
// CHECK:                 Columns: 1
// CHECK:             CBuffer: $Globals
// CHECK:         }
// CHECK:     Bound Resources:
// CHECK:       D3D12_SHADER_INPUT_BIND_DESC: Name: $Globals
// CHECK:         Type: D3D_SIT_CBUFFER
// CHECK:         uID: 0
// CHECK:         Dimension: D3D_SRV_DIMENSION_UNKNOWN
// CHECK:       D3D12_SHADER_INPUT_BIND_DESC: Name: samp
// CHECK:         Type: D3D_SIT_SAMPLER
// CHECK:         uID: 0
// CHECK:         BindPoint: 7
// CHECK:         Dimension: D3D_SRV_DIMENSION_UNKNOWN
// CHECK:       D3D12_SHADER_INPUT_BIND_DESC: Name: tex2
// CHECK:         Type: D3D_SIT_TEXTURE
// CHECK:         uID: 0
// CHECK:         BindPoint: 0
// CHECK:         ReturnType: D3D_RETURN_TYPE_FLOAT
// CHECK:         Dimension: D3D_SRV_DIMENSION_TEXTURE1D
// CHECK:         uFlags: (D3D_SIF_TEXTURE_COMPONENT_0 | D3D_SIF_TEXTURE_COMPONENT_1)
// CHECK:       D3D12_SHADER_INPUT_BIND_DESC: Name: b_buf
// CHECK:         Type: D3D_SIT_UAV_RWBYTEADDRESS
// CHECK:         uID: 1
// CHECK:         ReturnType: D3D_RETURN_TYPE_MIXED
// CHECK:         Dimension: D3D_SRV_DIMENSION_BUFFER
// CHECK:   ID3D12FunctionReflection:
// CHECK:     D3D12_FUNCTION_DESC: Name: function2
// CHECK:       Shader Version: Vertex
// CHECK:       ConstantBuffers: 2
// CHECK:       BoundResources: 2
// CHECK:     Constant Buffers:
// CHECK:       ID3D12ShaderReflectionConstantBuffer:
// CHECK:         D3D12_SHADER_BUFFER_DESC: Name: $Globals
// CHECK:           Type: D3D_CT_CBUFFER
// CHECK:           Size: 16
// CHECK:           Num Variables: 1
// CHECK:         {
// CHECK:           ID3D12ShaderReflectionVariable:
// CHECK:             D3D12_SHADER_VARIABLE_DESC: Name: cbval1
// CHECK:               Size: 4
// CHECK:               uFlags: (D3D_SVF_USED)
// CHECK:             ID3D12ShaderReflectionType:
// CHECK:               D3D12_SHADER_TYPE_DESC: Name: float
// CHECK:                 Class: D3D_SVC_SCALAR
// CHECK:                 Type: D3D_SVT_FLOAT
// CHECK:                 Rows: 1
// CHECK:                 Columns: 1
// CHECK:             CBuffer: $Globals
// CHECK:         }
// CHECK:       ID3D12ShaderReflectionConstantBuffer:
// CHECK:         D3D12_SHADER_BUFFER_DESC: Name: MyCB
// CHECK:           Type: D3D_CT_CBUFFER
// CHECK:           Size: 32
// CHECK:           Num Variables: 2
// CHECK:         {
// CHECK:           ID3D12ShaderReflectionVariable:
// CHECK:             D3D12_SHADER_VARIABLE_DESC: Name: cbval2
// CHECK:               Size: 16
// CHECK:               uFlags: (D3D_SVF_USED)
// CHECK:             ID3D12ShaderReflectionType:
// CHECK:               D3D12_SHADER_TYPE_DESC: Name: int4
// CHECK:                 Class: D3D_SVC_VECTOR
// CHECK:                 Type: D3D_SVT_INT
// CHECK:                 Rows: 1
// CHECK:                 Columns: 4
// CHECK:             CBuffer: MyCB
// CHECK:           ID3D12ShaderReflectionVariable:
// CHECK:             D3D12_SHADER_VARIABLE_DESC: Name: cbval3
// CHECK:               Size: 16
// CHECK:               StartOffset: 16
// CHECK:               uFlags: (D3D_SVF_USED)
// CHECK:             ID3D12ShaderReflectionType:
// CHECK:               D3D12_SHADER_TYPE_DESC: Name: int4
// CHECK:                 Class: D3D_SVC_VECTOR
// CHECK:                 Type: D3D_SVT_INT
// CHECK:                 Rows: 1
// CHECK:                 Columns: 4
// CHECK:             CBuffer: MyCB
// CHECK:         }
// CHECK:     Bound Resources:
// CHECK:       D3D12_SHADER_INPUT_BIND_DESC: Name: $Globals
// CHECK:         Type: D3D_SIT_CBUFFER
// CHECK:         uID: 0
// CHECK:         Dimension: D3D_SRV_DIMENSION_UNKNOWN
// CHECK:       D3D12_SHADER_INPUT_BIND_DESC: Name: MyCB
// CHECK:         Type: D3D_SIT_CBUFFER
// CHECK:         uID: 1
// CHECK:         BindPoint: 11
// CHECK:         Space: 2
// CHECK:         Dimension: D3D_SRV_DIMENSION_UNKNOWN
