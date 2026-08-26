// RUN: %dxc -T ps_6_0 -E main %s | %D3DReflect %s | FileCheck %s
// RUN: %dxc -T ps_6_6 -E main %s | %D3DReflect %s | FileCheck %s

struct {
    int X;
} CB;

float main(int N : A, int C : B) : SV_TARGET {
    return CB.X;
}

// CHECK: ID3D12ShaderReflection:
// CHECK:   D3D12_SHADER_DESC:
// CHECK:     Shader Version: Pixel
// CHECK:     ConstantBuffers: 1
// CHECK:     BoundResources: 1
// CHECK:     InputParameters: 2
// CHECK:     OutputParameters: 1
// CHECK:   Constant Buffers:
// CHECK:     ID3D12ShaderReflectionConstantBuffer:
// CHECK:       D3D12_SHADER_BUFFER_DESC: Name: $Globals
// CHECK:         Type: D3D_CT_CBUFFER
// CHECK:         Size: 16
// CHECK:         Num Variables: 1
// CHECK:       {
// CHECK:         ID3D12ShaderReflectionVariable:
// CHECK:           D3D12_SHADER_VARIABLE_DESC: Name: CB
// CHECK:             Size: 4
// CHECK:             uFlags: (D3D_SVF_USED)
// CHECK:           ID3D12ShaderReflectionType:
// CHECK:             D3D12_SHADER_TYPE_DESC: Name: anon
// CHECK:               Class: D3D_SVC_STRUCT
// CHECK:               Type: D3D_SVT_VOID
// CHECK:               Rows: 1
// CHECK:               Columns: 1
// CHECK:               Members: 1
// CHECK:             {
// CHECK:               ID3D12ShaderReflectionType:
// CHECK:                 D3D12_SHADER_TYPE_DESC: Name: int
// CHECK:                   Class: D3D_SVC_SCALAR
// CHECK:                   Type: D3D_SVT_INT
// CHECK:                   Rows: 1
// CHECK:                   Columns: 1
// CHECK:             }
// CHECK:           CBuffer: $Globals
// CHECK:       }
// CHECK:   Bound Resources:
// CHECK:     D3D12_SHADER_INPUT_BIND_DESC: Name: $Globals
// CHECK:       Type: D3D_SIT_CBUFFER
// CHECK:       uID: 0
// CHECK:       BindPoint: 0
// CHECK:       Dimension: D3D_SRV_DIMENSION_UNKNOWN
