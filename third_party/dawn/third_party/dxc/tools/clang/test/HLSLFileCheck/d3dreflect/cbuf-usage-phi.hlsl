// RUN: %dxc -T ps_6_5 -E main -Vd -validator-version 1.5 %s | %D3DReflect %s | FileCheck %s
// RUN: %dxc -T ps_6_6 -E main -Vd -validator-version 1.6 %s | %D3DReflect %s | FileCheck %s
// Flags needed to ensure usage flags are preserved in reflection with -Qkeep_reflect_in_dxil (implied):
//   -T ps_6_5 -Vd -validator-version 1.5

float4 UnusedGlobal[2];

float4 LookupTable[2];
bool TestBool;
bool TestBool2;

cbuffer CB {
  int TestInt;
}

float4 main() : SV_Target {
  uint Index = 0;

  // This introduces a phi for Index, due to use of new cbuffer.
  // If unhandled, this results in a zero offset into $Globals for LookupTable usage.
  if (TestBool) {
    Index = TestInt;
  }

  return LookupTable[Index]
  // This will produce a select with the same problem.
       + LookupTable[TestBool2 ? Index : 1];
}

// {{$}} is used to prevent match to 0x<anything>
// CHECK: ID3D12ShaderReflection:
// CHECK:     Flags: 0{{$}}
// CHECK:     ConstantBuffers: 2
// CHECK:     BoundResources: 2
// CHECK:   Constant Buffers:
// CHECK:       D3D12_SHADER_BUFFER_DESC: Name: $Globals
// CHECK:         Type: D3D_CT_CBUFFER
// CHECK:         Size: 80
// CHECK:         uFlags: 0{{$}}
// CHECK:         Num Variables: 4
// CHECK:       {
// CHECK:           D3D12_SHADER_VARIABLE_DESC: Name: UnusedGlobal
// CHECK:             uFlags: 0{{$}}
// CHECK:           D3D12_SHADER_VARIABLE_DESC: Name: LookupTable
// CHECK:             uFlags: (D3D_SVF_USED)
// CHECK:           D3D12_SHADER_VARIABLE_DESC: Name: TestBool
// CHECK:             uFlags: (D3D_SVF_USED)
// CHECK:           D3D12_SHADER_VARIABLE_DESC: Name: TestBool2
// CHECK:             uFlags: (D3D_SVF_USED)
// CHECK:       }
// CHECK:     ID3D12ShaderReflectionConstantBuffer:
// CHECK:       D3D12_SHADER_BUFFER_DESC: Name: CB
// CHECK:       {
// CHECK:           D3D12_SHADER_VARIABLE_DESC: Name: TestInt
// CHECK:             uFlags: (D3D_SVF_USED)
// CHECK:       }
// CHECK:   Bound Resources:
// CHECK:     D3D12_SHADER_INPUT_BIND_DESC: Name: $Globals
// CHECK:       BindCount: 1
// CHECK:       BindPoint: 0
// CHECK:       Space: 0
// CHECK:       uFlags: (D3D_SIF_USERPACKED)
// CHECK:     D3D12_SHADER_INPUT_BIND_DESC: Name: CB
// CHECK:       BindCount: 1
// CHECK:       BindPoint: 1
// CHECK:       Space: 0
// CHECK:       uFlags: (D3D_SIF_USERPACKED)
