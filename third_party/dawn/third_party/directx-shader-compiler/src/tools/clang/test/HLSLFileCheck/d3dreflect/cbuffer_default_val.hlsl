// RUN: %dxc -E main -T ps_6_0 %s | %D3DReflect %s | FileCheck %s
// RUN: %dxc -E main -T ps_6_6 %s | %D3DReflect %s | FileCheck %s

float t = 0.5;

float main() : SV_TARGET
{
  return sqrt(t);
}

// Default value unsupported for now:
// CHECK: ID3D12ShaderReflection:
// CHECK:   D3D12_SHADER_DESC:
// CHECK:     Shader Version: Pixel
// CHECK:     ConstantBuffers: 1
// CHECK:     BoundResources: 1
// CHECK:     OutputParameters: 1
// CHECK:   Constant Buffers:
// CHECK:     ID3D12ShaderReflectionConstantBuffer:
// CHECK:       D3D12_SHADER_BUFFER_DESC: Name: $Globals
// CHECK:         Type: D3D_CT_CBUFFER
// CHECK:         Size: 16
// CHECK:         Num Variables: 1
// CHECK:       {
// CHECK:         ID3D12ShaderReflectionVariable:
// CHECK:           D3D12_SHADER_VARIABLE_DESC: Name: t
// CHECK:             Size: 4
// CHECK:             uFlags: (D3D_SVF_USED)
// CHECK:             DefaultValue: <nullptr>
// CHECK:           ID3D12ShaderReflectionType:
// CHECK:             D3D12_SHADER_TYPE_DESC: Name: float
// CHECK:               Class: D3D_SVC_SCALAR
// CHECK:               Type: D3D_SVT_FLOAT
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
