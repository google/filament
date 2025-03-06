// RUN: %dxc -enable-16bit-types -E main -T ps_6_2 -Vd -validator-version 0.0 %s | %D3DReflect %s | FileCheck %s
// RUN: %dxc -enable-16bit-types -E main -T ps_6_6 -Vd -validator-version 0.0 %s | %D3DReflect %s | FileCheck %s

// -Vd -validator-version 0.0 is used to keep the reflection information
// in the actual module containing the cbuffer/tbuffer usage info.
// This is only necessary for test path at the moment because of the way
// the IR gets into D3DReflect by way of module disassembly and reassembly.
// Otherwise the separate reflection blob would correctly contain the usage information,
// since the metadata there is not gated on the shader target.

struct CBStruct
{
  float b;
  double d;         // aligned at 8 (4-bytes padding)
  float f;
  half4 h4;         // 8-bytes at 20
  // spill vector to new row:
  float16_t3 f16_3; // 6-bytes at 32 (4-bytes padding)
  min16float mf1;   // 2-bytes at 38 (must look at shift/trunc to detect use)
  uint16_t u16;     // 2-bytes at 40
  int16_t2 i16;     // 4-bytes at 42 (may need to look at multiple extractelement users to detect use of i16.x, if u16 is used)
  int i;            // 4-bytes at 48 (2-bytes padding)
  uint16_t4 u16_4;  // 8-bytes at 52
  half2 h2;         // 4-bytes at 60 (maps to float16_t2, fits in remaining row)
  float16_t f16;    // 2-bytes at 64
  int64_t i64;      // 8-bytes at 72 (6-bytes padding)
  float16_t f16b;   // 2-bytes at 80
  uint64_t u64;     // 8-bytes at 88 (6-bytes padding)
  float16_t f16c;   // 2-bytes at 96
  // overall size should be aligned to 16-byte boundary.
  // unaligned size = 98
  // aligned size = 112
};

ConstantBuffer<CBStruct> CBS : register(b1);

float main(int i : A) : SV_TARGET
{
  return CBS.b + CBS.mf1 + CBS.h2.y + CBS.f16_3.z + (float)(CBS.d * CBS.i64 + CBS.u64) + CBS.u16 * CBS.i16.x;
}

// CHECK: ID3D12ShaderReflection:
// CHECK-NEXT:   D3D12_SHADER_DESC:
// CHECK-NEXT:     Shader Version: Pixel
// CHECK:     Flags: 0
// CHECK-NEXT:     ConstantBuffers: 1
// CHECK-NEXT:     BoundResources: 1
// CHECK-NEXT:     InputParameters: 1
// CHECK-NEXT:     OutputParameters: 1
// CHECK-NEXT:     InstructionCount: {{34|35}}
// CHECK-NEXT:     TempArrayCount: 0
// CHECK-NEXT:     DynamicFlowControlCount: 0
// CHECK-NEXT:     ArrayInstructionCount: 0
// CHECK-NEXT:     TextureNormalInstructions: 0
// CHECK-NEXT:     TextureLoadInstructions: 0
// CHECK-NEXT:     TextureCompInstructions: 0
// CHECK-NEXT:     TextureBiasInstructions: 0
// CHECK-NEXT:     TextureGradientInstructions: 0
// CHECK-NEXT:     FloatInstructionCount: 14
// CHECK-NEXT:     IntInstructionCount: 1
// CHECK-NEXT:     UintInstructionCount: 0
// CHECK-NEXT:     CutInstructionCount: 0
// CHECK-NEXT:     EmitInstructionCount: 0
// CHECK-NEXT:     cBarrierInstructions: 0
// CHECK-NEXT:     cInterlockedInstructions: 0
// CHECK-NEXT:     cTextureStoreInstructions: 0
// CHECK:   Constant Buffers:
// CHECK-NEXT:     ID3D12ShaderReflectionConstantBuffer:
// CHECK-NEXT:       D3D12_SHADER_BUFFER_DESC: Name: CBS
// CHECK-NEXT:         Type: D3D_CT_CBUFFER
// CHECK-NEXT:         Size: 112
// CHECK-NEXT:         uFlags: 0
// CHECK-NEXT:         Num Variables: 1
// CHECK-NEXT:       {
// CHECK-NEXT:         ID3D12ShaderReflectionVariable:
// CHECK-NEXT:           D3D12_SHADER_VARIABLE_DESC: Name: CBS
// CHECK-NEXT:             Size: 98
// CHECK-NEXT:             StartOffset: 0
// CHECK-NEXT:             uFlags: (D3D_SVF_USED)
// CHECK-NEXT:             DefaultValue: <nullptr>
// CHECK-NEXT:           ID3D12ShaderReflectionType:
// CHECK-NEXT:             D3D12_SHADER_TYPE_DESC: Name: CBStruct
// CHECK-NEXT:               Class: D3D_SVC_STRUCT
// CHECK-NEXT:               Type: D3D_SVT_VOID
// CHECK-NEXT:               Elements: 0
// CHECK-NEXT:               Rows: 1
// CHECK-NEXT:               Columns: 26
// CHECK-NEXT:               Members: 16
// CHECK-NEXT:               Offset: 0
// CHECK-NEXT:             {
// CHECK-NEXT:               ID3D12ShaderReflectionType:
// CHECK-NEXT:                 D3D12_SHADER_TYPE_DESC: Name: float
// CHECK-NEXT:                   Class: D3D_SVC_SCALAR
// CHECK-NEXT:                   Type: D3D_SVT_FLOAT
// CHECK-NEXT:                   Elements: 0
// CHECK-NEXT:                   Rows: 1
// CHECK-NEXT:                   Columns: 1
// CHECK-NEXT:                   Members: 0
// CHECK-NEXT:                   Offset: 0
// CHECK-NEXT:               ID3D12ShaderReflectionType:
// CHECK-NEXT:                 D3D12_SHADER_TYPE_DESC: Name: double
// CHECK-NEXT:                   Class: D3D_SVC_SCALAR
// CHECK-NEXT:                   Type: D3D_SVT_DOUBLE
// CHECK-NEXT:                   Elements: 0
// CHECK-NEXT:                   Rows: 1
// CHECK-NEXT:                   Columns: 1
// CHECK-NEXT:                   Members: 0
// CHECK-NEXT:                   Offset: 8
// CHECK-NEXT:               ID3D12ShaderReflectionType:
// CHECK-NEXT:                 D3D12_SHADER_TYPE_DESC: Name: float
// CHECK-NEXT:                   Class: D3D_SVC_SCALAR
// CHECK-NEXT:                   Type: D3D_SVT_FLOAT
// CHECK-NEXT:                   Elements: 0
// CHECK-NEXT:                   Rows: 1
// CHECK-NEXT:                   Columns: 1
// CHECK-NEXT:                   Members: 0
// CHECK-NEXT:                   Offset: 16
// CHECK-NEXT:               ID3D12ShaderReflectionType:
// CHECK-NEXT:                 D3D12_SHADER_TYPE_DESC: Name: float16_t4
// CHECK-NEXT:                   Class: D3D_SVC_VECTOR
// CHECK-NEXT:                   Type: D3D_SVT_FLOAT16
// CHECK-NEXT:                   Elements: 0
// CHECK-NEXT:                   Rows: 1
// CHECK-NEXT:                   Columns: 4
// CHECK-NEXT:                   Members: 0
// CHECK-NEXT:                   Offset: 20
// CHECK-NEXT:               ID3D12ShaderReflectionType:
// CHECK-NEXT:                 D3D12_SHADER_TYPE_DESC: Name: float16_t3
// CHECK-NEXT:                   Class: D3D_SVC_VECTOR
// CHECK-NEXT:                   Type: D3D_SVT_FLOAT16
// CHECK-NEXT:                   Elements: 0
// CHECK-NEXT:                   Rows: 1
// CHECK-NEXT:                   Columns: 3
// CHECK-NEXT:                   Members: 0
// CHECK-NEXT:                   Offset: 32
// CHECK-NEXT:               ID3D12ShaderReflectionType:
// CHECK-NEXT:                 D3D12_SHADER_TYPE_DESC: Name: float16_t
// CHECK-NEXT:                   Class: D3D_SVC_SCALAR
// CHECK-NEXT:                   Type: D3D_SVT_FLOAT16
// CHECK-NEXT:                   Elements: 0
// CHECK-NEXT:                   Rows: 1
// CHECK-NEXT:                   Columns: 1
// CHECK-NEXT:                   Members: 0
// CHECK-NEXT:                   Offset: 40
// CHECK-NEXT:               ID3D12ShaderReflectionType:
// CHECK-NEXT:                 D3D12_SHADER_TYPE_DESC: Name: uint16_t
// CHECK-NEXT:                   Class: D3D_SVC_SCALAR
// CHECK-NEXT:                   Type: D3D_SVT_UINT16
// CHECK-NEXT:                   Elements: 0
// CHECK-NEXT:                   Rows: 1
// CHECK-NEXT:                   Columns: 1
// CHECK-NEXT:                   Members: 0
// CHECK-NEXT:                   Offset: 42
// CHECK-NEXT:               ID3D12ShaderReflectionType:
// CHECK-NEXT:                 D3D12_SHADER_TYPE_DESC: Name: int16_t2
// CHECK-NEXT:                   Class: D3D_SVC_VECTOR
// CHECK-NEXT:                   Type: D3D_SVT_INT16
// CHECK-NEXT:                   Elements: 0
// CHECK-NEXT:                   Rows: 1
// CHECK-NEXT:                   Columns: 2
// CHECK-NEXT:                   Members: 0
// CHECK-NEXT:                   Offset: 44
// CHECK-NEXT:               ID3D12ShaderReflectionType:
// CHECK-NEXT:                 D3D12_SHADER_TYPE_DESC: Name: int
// CHECK-NEXT:                   Class: D3D_SVC_SCALAR
// CHECK-NEXT:                   Type: D3D_SVT_INT
// CHECK-NEXT:                   Elements: 0
// CHECK-NEXT:                   Rows: 1
// CHECK-NEXT:                   Columns: 1
// CHECK-NEXT:                   Members: 0
// CHECK-NEXT:                   Offset: 48
// CHECK-NEXT:               ID3D12ShaderReflectionType:
// CHECK-NEXT:                 D3D12_SHADER_TYPE_DESC: Name: uint16_t4
// CHECK-NEXT:                   Class: D3D_SVC_VECTOR
// CHECK-NEXT:                   Type: D3D_SVT_UINT16
// CHECK-NEXT:                   Elements: 0
// CHECK-NEXT:                   Rows: 1
// CHECK-NEXT:                   Columns: 4
// CHECK-NEXT:                   Members: 0
// CHECK-NEXT:                   Offset: 52
// CHECK-NEXT:               ID3D12ShaderReflectionType:
// CHECK-NEXT:                 D3D12_SHADER_TYPE_DESC: Name: float16_t2
// CHECK-NEXT:                   Class: D3D_SVC_VECTOR
// CHECK-NEXT:                   Type: D3D_SVT_FLOAT16
// CHECK-NEXT:                   Elements: 0
// CHECK-NEXT:                   Rows: 1
// CHECK-NEXT:                   Columns: 2
// CHECK-NEXT:                   Members: 0
// CHECK-NEXT:                   Offset: 60
// CHECK-NEXT:               ID3D12ShaderReflectionType:
// CHECK-NEXT:                 D3D12_SHADER_TYPE_DESC: Name: float16_t
// CHECK-NEXT:                   Class: D3D_SVC_SCALAR
// CHECK-NEXT:                   Type: D3D_SVT_FLOAT16
// CHECK-NEXT:                   Elements: 0
// CHECK-NEXT:                   Rows: 1
// CHECK-NEXT:                   Columns: 1
// CHECK-NEXT:                   Members: 0
// CHECK-NEXT:                   Offset: 64
// CHECK-NEXT:               ID3D12ShaderReflectionType:
// CHECK-NEXT:                 D3D12_SHADER_TYPE_DESC: Name: int64_t
// CHECK-NEXT:                   Class: D3D_SVC_SCALAR
// CHECK-NEXT:                   Type: D3D_SVT_INT64
// CHECK-NEXT:                   Elements: 0
// CHECK-NEXT:                   Rows: 1
// CHECK-NEXT:                   Columns: 1
// CHECK-NEXT:                   Members: 0
// CHECK-NEXT:                   Offset: 72
// CHECK-NEXT:               ID3D12ShaderReflectionType:
// CHECK-NEXT:                 D3D12_SHADER_TYPE_DESC: Name: float16_t
// CHECK-NEXT:                   Class: D3D_SVC_SCALAR
// CHECK-NEXT:                   Type: D3D_SVT_FLOAT16
// CHECK-NEXT:                   Elements: 0
// CHECK-NEXT:                   Rows: 1
// CHECK-NEXT:                   Columns: 1
// CHECK-NEXT:                   Members: 0
// CHECK-NEXT:                   Offset: 80
// CHECK-NEXT:               ID3D12ShaderReflectionType:
// CHECK-NEXT:                 D3D12_SHADER_TYPE_DESC: Name: uint64_t
// CHECK-NEXT:                   Class: D3D_SVC_SCALAR
// CHECK-NEXT:                   Type: D3D_SVT_UINT64
// CHECK-NEXT:                   Elements: 0
// CHECK-NEXT:                   Rows: 1
// CHECK-NEXT:                   Columns: 1
// CHECK-NEXT:                   Members: 0
// CHECK-NEXT:                   Offset: 88
// CHECK-NEXT:               ID3D12ShaderReflectionType:
// CHECK-NEXT:                 D3D12_SHADER_TYPE_DESC: Name: float16_t
// CHECK-NEXT:                   Class: D3D_SVC_SCALAR
// CHECK-NEXT:                   Type: D3D_SVT_FLOAT16
// CHECK-NEXT:                   Elements: 0
// CHECK-NEXT:                   Rows: 1
// CHECK-NEXT:                   Columns: 1
// CHECK-NEXT:                   Members: 0
// CHECK-NEXT:                   Offset: 96
// CHECK-NEXT:             }
// CHECK-NEXT:           CBuffer: CBS
// CHECK-NEXT:       }
// CHECK-NEXT:   Bound Resources:
// CHECK-NEXT:     D3D12_SHADER_INPUT_BIND_DESC: Name: CBS
// CHECK-NEXT:       Type: D3D_SIT_CBUFFER
// CHECK-NEXT:       uID: 0
// CHECK-NEXT:       BindCount: 1
// CHECK-NEXT:       BindPoint: 1
// CHECK-NEXT:       Space: 0
// CHECK-NEXT:       ReturnType: <unknown: 0>
// CHECK-NEXT:       Dimension: D3D_SRV_DIMENSION_UNKNOWN
// CHECK-NEXT:       NumSamples (or stride): 0
// CHECK-NEXT:       uFlags: (D3D_SIF_USERPACKED)
