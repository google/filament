// RUN: %dxc -E main -T ps_6_0 -Vd -validator-version 0.0 %s | %D3DReflect %s | FileCheck %s
// RUN: %dxc -E main -T ps_6_6 -Vd -validator-version 0.0 %s | %D3DReflect %s | FileCheck %s

// -Vd -validator-version 0.0 is used to keep the reflection information
// in the actual module containing the cbuffer/tbuffer usage info.
// This is only necessary for test path at the moment because of the way
// the IR gets into D3DReflect by way of module disassembly and reassembly.
// Otherwise the separate reflection blob would correctly contain the usage information,
// since the metadata there is not gated on the shader target.

tbuffer tb : register(t1)
{
  float b;
  double d;         // aligned at 8 (4-bytes padding)

  // row
  float f;

  // row
  half4 h4;         // aligned at 32 (12-bytes padding, map to float)

  // row
  // min16float and min16uint on same row
  min16float mf;    // 4-bytes at 48
  min16uint mu;     // 4-bytes at 52

  // row (to prevent mixing of min-precision in same register)
  uint u;           // 4-bytes at 64

  // row (to prevent mixing of min-precision in same register)
  // spill vector to new row though 16-bit values would fit:
  min16int2 mi;     // 8-bytes at 80

  int2 i2;          // 8-bytes at 96 (to complete row)

  // row
  float a;          // 4-bytes on new row

  // overall size should be aligned to 16-byte boundary.

  // unaligned size = 108
  // aligned size = 112
};

float main(int i : A) : SV_TARGET
{
  return b + mi.y + a + mf * mu;
}

// CHECK: ID3D12ShaderReflection:
// CHECK-NEXT:   D3D12_SHADER_DESC:
// CHECK-NEXT:     Shader Version: Pixel
// CHECK:     Flags: 0
// CHECK-NEXT:     ConstantBuffers: 1
// CHECK-NEXT:     BoundResources: 1
// CHECK-NEXT:     InputParameters: 1
// CHECK-NEXT:     OutputParameters: 1
// CHECK-NEXT:     InstructionCount: 2{{[67]}}
// CHECK-NEXT:     TempArrayCount: 0
// CHECK-NEXT:     DynamicFlowControlCount: 0
// CHECK-NEXT:     ArrayInstructionCount: 0
// CHECK-NEXT:     TextureNormalInstructions: 0
// CHECK-NEXT:     TextureLoadInstructions: 5
// CHECK-NEXT:     TextureCompInstructions: 0
// CHECK-NEXT:     TextureBiasInstructions: 0
// CHECK-NEXT:     TextureGradientInstructions: 0
// CHECK-NEXT:     FloatInstructionCount: 8
// CHECK-NEXT:     IntInstructionCount: 2
// CHECK-NEXT:     UintInstructionCount: 0
// CHECK-NEXT:     CutInstructionCount: 0
// CHECK-NEXT:     EmitInstructionCount: 0
// CHECK-NEXT:     cBarrierInstructions: 0
// CHECK-NEXT:     cInterlockedInstructions: 0
// CHECK-NEXT:     cTextureStoreInstructions: 0
// CHECK:   Constant Buffers:
// CHECK-NEXT:     ID3D12ShaderReflectionConstantBuffer:
// CHECK-NEXT:       D3D12_SHADER_BUFFER_DESC: Name: tb
// CHECK-NEXT:         Type: D3D_CT_TBUFFER
// CHECK-NEXT:         Size: 112
// CHECK-NEXT:         uFlags: 0
// CHECK-NEXT:         Num Variables: 10
// CHECK-NEXT:       {
// CHECK-NEXT:         ID3D12ShaderReflectionVariable:
// CHECK-NEXT:           D3D12_SHADER_VARIABLE_DESC: Name: b
// CHECK-NEXT:             Size: 4
// CHECK-NEXT:             StartOffset: 0
// CHECK-NEXT:             uFlags: (D3D_SVF_USED)
// CHECK-NEXT:             DefaultValue: <nullptr>
// CHECK-NEXT:           ID3D12ShaderReflectionType:
// CHECK-NEXT:             D3D12_SHADER_TYPE_DESC: Name: float
// CHECK-NEXT:               Class: D3D_SVC_SCALAR
// CHECK-NEXT:               Type: D3D_SVT_FLOAT
// CHECK-NEXT:               Elements: 0
// CHECK-NEXT:               Rows: 1
// CHECK-NEXT:               Columns: 1
// CHECK-NEXT:               Members: 0
// CHECK-NEXT:               Offset: 0
// CHECK-NEXT:           CBuffer: tb
// CHECK-NEXT:         ID3D12ShaderReflectionVariable:
// CHECK-NEXT:           D3D12_SHADER_VARIABLE_DESC: Name: d
// CHECK-NEXT:             Size: 8
// CHECK-NEXT:             StartOffset: 8
// CHECK-NEXT:             uFlags: 0
// CHECK-NEXT:             DefaultValue: <nullptr>
// CHECK-NEXT:           ID3D12ShaderReflectionType:
// CHECK-NEXT:             D3D12_SHADER_TYPE_DESC: Name: double
// CHECK-NEXT:               Class: D3D_SVC_SCALAR
// CHECK-NEXT:               Type: D3D_SVT_DOUBLE
// CHECK-NEXT:               Elements: 0
// CHECK-NEXT:               Rows: 1
// CHECK-NEXT:               Columns: 1
// CHECK-NEXT:               Members: 0
// CHECK-NEXT:               Offset: 0
// CHECK-NEXT:           CBuffer: tb
// CHECK-NEXT:         ID3D12ShaderReflectionVariable:
// CHECK-NEXT:           D3D12_SHADER_VARIABLE_DESC: Name: f
// CHECK-NEXT:             Size: 4
// CHECK-NEXT:             StartOffset: 16
// CHECK-NEXT:             uFlags: 0
// CHECK-NEXT:             DefaultValue: <nullptr>
// CHECK-NEXT:           ID3D12ShaderReflectionType:
// CHECK-NEXT:             D3D12_SHADER_TYPE_DESC: Name: float
// CHECK-NEXT:               Class: D3D_SVC_SCALAR
// CHECK-NEXT:               Type: D3D_SVT_FLOAT
// CHECK-NEXT:               Elements: 0
// CHECK-NEXT:               Rows: 1
// CHECK-NEXT:               Columns: 1
// CHECK-NEXT:               Members: 0
// CHECK-NEXT:               Offset: 0
// CHECK-NEXT:           CBuffer: tb
// CHECK-NEXT:         ID3D12ShaderReflectionVariable:
// CHECK-NEXT:           D3D12_SHADER_VARIABLE_DESC: Name: h4
// CHECK-NEXT:             Size: 16
// CHECK-NEXT:             StartOffset: 32
// CHECK-NEXT:             uFlags: 0
// CHECK-NEXT:             DefaultValue: <nullptr>
// CHECK-NEXT:           ID3D12ShaderReflectionType:
// CHECK-NEXT:             D3D12_SHADER_TYPE_DESC: Name: float4
// CHECK-NEXT:               Class: D3D_SVC_VECTOR
// CHECK-NEXT:               Type: D3D_SVT_FLOAT
// CHECK-NEXT:               Elements: 0
// CHECK-NEXT:               Rows: 1
// CHECK-NEXT:               Columns: 4
// CHECK-NEXT:               Members: 0
// CHECK-NEXT:               Offset: 0
// CHECK-NEXT:           CBuffer: tb
// CHECK-NEXT:         ID3D12ShaderReflectionVariable:
// CHECK-NEXT:           D3D12_SHADER_VARIABLE_DESC: Name: mf
// CHECK-NEXT:             Size: 4
// CHECK-NEXT:             StartOffset: 48
// CHECK-NEXT:             uFlags: (D3D_SVF_USED)
// CHECK-NEXT:             DefaultValue: <nullptr>
// CHECK-NEXT:           ID3D12ShaderReflectionType:
// CHECK-NEXT:             D3D12_SHADER_TYPE_DESC: Name: min16float
// CHECK-NEXT:               Class: D3D_SVC_SCALAR
// CHECK-NEXT:               Type: D3D_SVT_MIN16FLOAT
// CHECK-NEXT:               Elements: 0
// CHECK-NEXT:               Rows: 1
// CHECK-NEXT:               Columns: 1
// CHECK-NEXT:               Members: 0
// CHECK-NEXT:               Offset: 0
// CHECK-NEXT:           CBuffer: tb
// CHECK-NEXT:         ID3D12ShaderReflectionVariable:
// CHECK-NEXT:           D3D12_SHADER_VARIABLE_DESC: Name: mu
// CHECK-NEXT:             Size: 4
// CHECK-NEXT:             StartOffset: 52
// CHECK-NEXT:             uFlags: (D3D_SVF_USED)
// CHECK-NEXT:             DefaultValue: <nullptr>
// CHECK-NEXT:           ID3D12ShaderReflectionType:
// CHECK-NEXT:             D3D12_SHADER_TYPE_DESC: Name: min16uint
// CHECK-NEXT:               Class: D3D_SVC_SCALAR
// CHECK-NEXT:               Type: D3D_SVT_MIN16UINT
// CHECK-NEXT:               Elements: 0
// CHECK-NEXT:               Rows: 1
// CHECK-NEXT:               Columns: 1
// CHECK-NEXT:               Members: 0
// CHECK-NEXT:               Offset: 0
// CHECK-NEXT:           CBuffer: tb
// CHECK-NEXT:         ID3D12ShaderReflectionVariable:
// CHECK-NEXT:           D3D12_SHADER_VARIABLE_DESC: Name: u
// CHECK-NEXT:             Size: 4
// CHECK-NEXT:             StartOffset: 64
// CHECK-NEXT:             uFlags: 0
// CHECK-NEXT:             DefaultValue: <nullptr>
// CHECK-NEXT:           ID3D12ShaderReflectionType:
// CHECK-NEXT:             D3D12_SHADER_TYPE_DESC: Name: dword
// CHECK-NEXT:               Class: D3D_SVC_SCALAR
// CHECK-NEXT:               Type: D3D_SVT_UINT
// CHECK-NEXT:               Elements: 0
// CHECK-NEXT:               Rows: 1
// CHECK-NEXT:               Columns: 1
// CHECK-NEXT:               Members: 0
// CHECK-NEXT:               Offset: 0
// CHECK-NEXT:           CBuffer: tb
// CHECK-NEXT:         ID3D12ShaderReflectionVariable:
// CHECK-NEXT:           D3D12_SHADER_VARIABLE_DESC: Name: mi
// CHECK-NEXT:             Size: 8
// CHECK-NEXT:             StartOffset: 80
// CHECK-NEXT:             uFlags: (D3D_SVF_USED)
// CHECK-NEXT:             DefaultValue: <nullptr>
// CHECK-NEXT:           ID3D12ShaderReflectionType:
// CHECK-NEXT:             D3D12_SHADER_TYPE_DESC: Name: min16int2
// CHECK-NEXT:               Class: D3D_SVC_VECTOR
// CHECK-NEXT:               Type: D3D_SVT_MIN16INT
// CHECK-NEXT:               Elements: 0
// CHECK-NEXT:               Rows: 1
// CHECK-NEXT:               Columns: 2
// CHECK-NEXT:               Members: 0
// CHECK-NEXT:               Offset: 0
// CHECK-NEXT:           CBuffer: tb
// CHECK-NEXT:         ID3D12ShaderReflectionVariable:
// CHECK-NEXT:           D3D12_SHADER_VARIABLE_DESC: Name: i2
// CHECK-NEXT:             Size: 8
// CHECK-NEXT:             StartOffset: 96
// CHECK-NEXT:             uFlags: 0
// CHECK-NEXT:             DefaultValue: <nullptr>
// CHECK-NEXT:           ID3D12ShaderReflectionType:
// CHECK-NEXT:             D3D12_SHADER_TYPE_DESC: Name: int2
// CHECK-NEXT:               Class: D3D_SVC_VECTOR
// CHECK-NEXT:               Type: D3D_SVT_INT
// CHECK-NEXT:               Elements: 0
// CHECK-NEXT:               Rows: 1
// CHECK-NEXT:               Columns: 2
// CHECK-NEXT:               Members: 0
// CHECK-NEXT:               Offset: 0
// CHECK-NEXT:           CBuffer: tb
// CHECK-NEXT:         ID3D12ShaderReflectionVariable:
// CHECK-NEXT:           D3D12_SHADER_VARIABLE_DESC: Name: a
// CHECK-NEXT:             Size: 4
// CHECK-NEXT:             StartOffset: 104
// CHECK-NEXT:             uFlags: (D3D_SVF_USED)
// CHECK-NEXT:             DefaultValue: <nullptr>
// CHECK-NEXT:           ID3D12ShaderReflectionType:
// CHECK-NEXT:             D3D12_SHADER_TYPE_DESC: Name: float
// CHECK-NEXT:               Class: D3D_SVC_SCALAR
// CHECK-NEXT:               Type: D3D_SVT_FLOAT
// CHECK-NEXT:               Elements: 0
// CHECK-NEXT:               Rows: 1
// CHECK-NEXT:               Columns: 1
// CHECK-NEXT:               Members: 0
// CHECK-NEXT:               Offset: 0
// CHECK-NEXT:           CBuffer: tb
// CHECK-NEXT:       }
// CHECK-NEXT:   Bound Resources:
// CHECK-NEXT:     D3D12_SHADER_INPUT_BIND_DESC: Name: tb
// CHECK-NEXT:       Type: D3D_SIT_TBUFFER
// CHECK-NEXT:       uID: 0
// CHECK-NEXT:       BindCount: 1
// CHECK-NEXT:       BindPoint: 1
// CHECK-NEXT:       Space: 0
// CHECK-NEXT:       ReturnType: <unknown: 0>
// CHECK-NEXT:       Dimension: D3D_SRV_DIMENSION_UNKNOWN
// CHECK-NEXT:       NumSamples (or stride): 0
// CHECK-NEXT:       uFlags: (D3D_SIF_USERPACKED)
