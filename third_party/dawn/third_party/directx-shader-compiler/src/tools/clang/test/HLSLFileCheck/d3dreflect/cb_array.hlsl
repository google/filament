// RUN: %dxc -E main -T ps_6_0 %s | %D3DReflect %s | FileCheck %s
// RUN: %dxc -E main -T ps_6_6 %s | %D3DReflect %s | FileCheck %s

float A[6] : register(b0);
float main(int i : A) : SV_TARGET
{
  return A[i] + A[i+1] + A[i+2] ;
}

// CHECK: ID3D12ShaderReflection:
// CHECK:   D3D12_SHADER_DESC:
// CHECK:     Shader Version: Pixel
// CHECK:     ConstantBuffers: 1
// CHECK:     BoundResources: 1
// CHECK:     InputParameters: 1
// CHECK:     OutputParameters: 1
// CHECK:   Constant Buffers:
// CHECK:     ID3D12ShaderReflectionConstantBuffer:
// CHECK:       D3D12_SHADER_BUFFER_DESC: Name: $Globals
// CHECK:         Type: D3D_CT_CBUFFER
// CHECK:         Size: 96
// CHECK:         Num Variables: 1
// CHECK:       {
// CHECK:         ID3D12ShaderReflectionVariable:
// CHECK:           D3D12_SHADER_VARIABLE_DESC: Name: A
// CHECK:             Size: 84
// CHECK:             uFlags: (D3D_SVF_USED)
// CHECK:           ID3D12ShaderReflectionType:
// CHECK:             D3D12_SHADER_TYPE_DESC: Name: float
// CHECK:               Class: D3D_SVC_SCALAR
// CHECK:               Type: D3D_SVT_FLOAT
// CHECK:               Elements: 6
// CHECK:               Rows: 1
// CHECK:               Columns: 1
// CHECK:           CBuffer: $Globals
// CHECK:       }
// CHECK:   Bound Resources:
// CHECK:     D3D12_SHADER_INPUT_BIND_DESC: Name: $Globals
// CHECK:       Type: D3D_SIT_CBUFFER
// CHECK:       uID: 0
// CHECK:       BindPoint: 0
// CHECK:       Dimension: D3D_SRV_DIMENSION_UNKNOWN
