// RUN: %dxc -E main -T vs_6_0 -Vd -validator-version 0.0 %s | %D3DReflect %s | FileCheck %s
// RUN: %dxc -E main -T vs_6_6 -Vd -validator-version 0.0 %s | %D3DReflect %s | FileCheck %s

float1x1 i1x1_arr0_1[1];
float1x1 i1x1_arr0[3];
float3x1 i3x1_arr0[4];
float1x3 i1x3_arr0[5];
row_major float3x1 rm_i3x1_arr0[6];
row_major float1x3 rm_i1x3_arr0[7];

cbuffer CB1 {
  float1x1 i1x1_arr1[8];
}

struct CBStruct {
  float1x1 i1x1_arr2[9];
};

ConstantBuffer<CBStruct> CB2;

float main() : OUT {
  return i1x1_arr0[0]
    + i1x1_arr1[1]
    + CB2.i1x1_arr2[2];
}

#if 0

// CHECK:     Constant Buffers:
// CHECK-NEXT:     ID3D12ShaderReflectionConstantBuffer:
// CHECK-NEXT:       D3D12_SHADER_BUFFER_DESC: Name: $Globals
// CHECK-NEXT:         Type: D3D_CT_CBUFFER
// CHECK-NEXT:         Size: 768
// CHECK-NEXT:         uFlags: 0
// CHECK-NEXT:         Num Variables: 6
// CHECK-NEXT:       {
// CHECK-NEXT:         ID3D12ShaderReflectionVariable:
// CHECK-NEXT:           D3D12_SHADER_VARIABLE_DESC: Name: i1x1_arr0_1
// CHECK-NEXT:             Size: 4
// CHECK-NEXT:             StartOffset: 0
// CHECK-NEXT:             uFlags: 0
// CHECK-NEXT:             DefaultValue: <nullptr>
// CHECK-NEXT:           ID3D12ShaderReflectionType:
// CHECK-NEXT:             D3D12_SHADER_TYPE_DESC: Name: float1x1
// CHECK-NEXT:               Class: D3D_SVC_MATRIX_COLUMNS
// CHECK-NEXT:               Type: D3D_SVT_FLOAT
// CHECK-NEXT:               Elements: 1
// CHECK-NEXT:               Rows: 1
// CHECK-NEXT:               Columns: 1
// CHECK-NEXT:               Members: 0
// CHECK-NEXT:               Offset: 0
// CHECK-NEXT:           CBuffer: $Globals
// CHECK-NEXT:         ID3D12ShaderReflectionVariable:
// CHECK-NEXT:           D3D12_SHADER_VARIABLE_DESC: Name: i1x1_arr0
// CHECK-NEXT:             Size: 36
// CHECK-NEXT:             StartOffset: 16
// CHECK-NEXT:             uFlags: (D3D_SVF_USED)
// CHECK-NEXT:             DefaultValue: <nullptr>
// CHECK-NEXT:           ID3D12ShaderReflectionType:
// CHECK-NEXT:             D3D12_SHADER_TYPE_DESC: Name: float1x1
// CHECK-NEXT:               Class: D3D_SVC_MATRIX_COLUMNS
// CHECK-NEXT:               Type: D3D_SVT_FLOAT
// CHECK-NEXT:               Elements: 3
// CHECK-NEXT:               Rows: 1
// CHECK-NEXT:               Columns: 1
// CHECK-NEXT:               Members: 0
// CHECK-NEXT:               Offset: 0
// CHECK-NEXT:           CBuffer: $Globals
// CHECK-NEXT:         ID3D12ShaderReflectionVariable:
// CHECK-NEXT:           D3D12_SHADER_VARIABLE_DESC: Name: i3x1_arr0
// CHECK-NEXT:             Size: 60
// CHECK-NEXT:             StartOffset: 64
// CHECK-NEXT:             uFlags: 0
// CHECK-NEXT:             DefaultValue: <nullptr>
// CHECK-NEXT:           ID3D12ShaderReflectionType:
// CHECK-NEXT:             D3D12_SHADER_TYPE_DESC: Name: float3x1
// CHECK-NEXT:               Class: D3D_SVC_MATRIX_COLUMNS
// CHECK-NEXT:               Type: D3D_SVT_FLOAT
// CHECK-NEXT:               Elements: 4
// CHECK-NEXT:               Rows: 3
// CHECK-NEXT:               Columns: 1
// CHECK-NEXT:               Members: 0
// CHECK-NEXT:               Offset: 0
// CHECK-NEXT:           CBuffer: $Globals
// CHECK-NEXT:         ID3D12ShaderReflectionVariable:
// CHECK-NEXT:           D3D12_SHADER_VARIABLE_DESC: Name: i1x3_arr0
// CHECK-NEXT:             Size: 228
// CHECK-NEXT:             StartOffset: 128
// CHECK-NEXT:             uFlags: 0
// CHECK-NEXT:             DefaultValue: <nullptr>
// CHECK-NEXT:           ID3D12ShaderReflectionType:
// CHECK-NEXT:             D3D12_SHADER_TYPE_DESC: Name: float1x3
// CHECK-NEXT:               Class: D3D_SVC_MATRIX_COLUMNS
// CHECK-NEXT:               Type: D3D_SVT_FLOAT
// CHECK-NEXT:               Elements: 5
// CHECK-NEXT:               Rows: 1
// CHECK-NEXT:               Columns: 3
// CHECK-NEXT:               Members: 0
// CHECK-NEXT:               Offset: 0
// CHECK-NEXT:           CBuffer: $Globals
// CHECK-NEXT:         ID3D12ShaderReflectionVariable:
// CHECK-NEXT:           D3D12_SHADER_VARIABLE_DESC: Name: rm_i3x1_arr0
// CHECK-NEXT:             Size: 276
// CHECK-NEXT:             StartOffset: 368
// CHECK-NEXT:             uFlags: 0
// CHECK-NEXT:             DefaultValue: <nullptr>
// CHECK-NEXT:           ID3D12ShaderReflectionType:
// CHECK-NEXT:             D3D12_SHADER_TYPE_DESC: Name: float3x1
// CHECK-NEXT:               Class: D3D_SVC_MATRIX_ROWS
// CHECK-NEXT:               Type: D3D_SVT_FLOAT
// CHECK-NEXT:               Elements: 6
// CHECK-NEXT:               Rows: 3
// CHECK-NEXT:               Columns: 1
// CHECK-NEXT:               Members: 0
// CHECK-NEXT:               Offset: 0
// CHECK-NEXT:           CBuffer: $Globals
// CHECK-NEXT:         ID3D12ShaderReflectionVariable:
// CHECK-NEXT:           D3D12_SHADER_VARIABLE_DESC: Name: rm_i1x3_arr0
// CHECK-NEXT:             Size: 108
// CHECK-NEXT:             StartOffset: 656
// CHECK-NEXT:             uFlags: 0
// CHECK-NEXT:             DefaultValue: <nullptr>
// CHECK-NEXT:           ID3D12ShaderReflectionType:
// CHECK-NEXT:             D3D12_SHADER_TYPE_DESC: Name: float1x3
// CHECK-NEXT:               Class: D3D_SVC_MATRIX_ROWS
// CHECK-NEXT:               Type: D3D_SVT_FLOAT
// CHECK-NEXT:               Elements: 7
// CHECK-NEXT:               Rows: 1
// CHECK-NEXT:               Columns: 3
// CHECK-NEXT:               Members: 0
// CHECK-NEXT:               Offset: 0
// CHECK-NEXT:           CBuffer: $Globals
// CHECK-NEXT:       }
// CHECK-NEXT:     ID3D12ShaderReflectionConstantBuffer:
// CHECK-NEXT:       D3D12_SHADER_BUFFER_DESC: Name: CB1
// CHECK-NEXT:         Type: D3D_CT_CBUFFER
// CHECK-NEXT:         Size: 128
// CHECK-NEXT:         uFlags: 0
// CHECK-NEXT:         Num Variables: 1
// CHECK-NEXT:       {
// CHECK-NEXT:         ID3D12ShaderReflectionVariable:
// CHECK-NEXT:           D3D12_SHADER_VARIABLE_DESC: Name: i1x1_arr1
// CHECK-NEXT:             Size: 116
// CHECK-NEXT:             StartOffset: 0
// CHECK-NEXT:             uFlags: (D3D_SVF_USED)
// CHECK-NEXT:             DefaultValue: <nullptr>
// CHECK-NEXT:           ID3D12ShaderReflectionType:
// CHECK-NEXT:             D3D12_SHADER_TYPE_DESC: Name: float1x1
// CHECK-NEXT:               Class: D3D_SVC_MATRIX_COLUMNS
// CHECK-NEXT:               Type: D3D_SVT_FLOAT
// CHECK-NEXT:               Elements: 8
// CHECK-NEXT:               Rows: 1
// CHECK-NEXT:               Columns: 1
// CHECK-NEXT:               Members: 0
// CHECK-NEXT:               Offset: 0
// CHECK-NEXT:           CBuffer: CB1
// CHECK-NEXT:       }
// CHECK-NEXT:     ID3D12ShaderReflectionConstantBuffer:
// CHECK-NEXT:       D3D12_SHADER_BUFFER_DESC: Name: CB2
// CHECK-NEXT:         Type: D3D_CT_CBUFFER
// CHECK-NEXT:         Size: 144
// CHECK-NEXT:         uFlags: 0
// CHECK-NEXT:         Num Variables: 1
// CHECK-NEXT:       {
// CHECK-NEXT:         ID3D12ShaderReflectionVariable:
// CHECK-NEXT:           D3D12_SHADER_VARIABLE_DESC: Name: CB2
// CHECK-NEXT:             Size: 132
// CHECK-NEXT:             StartOffset: 0
// CHECK-NEXT:             uFlags: (D3D_SVF_USED)
// CHECK-NEXT:             DefaultValue: <nullptr>
// CHECK-NEXT:           ID3D12ShaderReflectionType:
// CHECK-NEXT:             D3D12_SHADER_TYPE_DESC: Name: CBStruct
// CHECK-NEXT:               Class: D3D_SVC_STRUCT
// CHECK-NEXT:               Type: D3D_SVT_VOID
// CHECK-NEXT:               Elements: 0
// CHECK-NEXT:               Rows: 1
// CHECK-NEXT:               Columns: 9
// CHECK-NEXT:               Members: 1
// CHECK-NEXT:               Offset: 0
// CHECK-NEXT:             {
// CHECK-NEXT:               ID3D12ShaderReflectionType:
// CHECK-NEXT:                 D3D12_SHADER_TYPE_DESC: Name: float1x1
// CHECK-NEXT:                   Class: D3D_SVC_MATRIX_COLUMNS
// CHECK-NEXT:                   Type: D3D_SVT_FLOAT
// CHECK-NEXT:                   Elements: 9
// CHECK-NEXT:                   Rows: 1
// CHECK-NEXT:                   Columns: 1
// CHECK-NEXT:                   Members: 0
// CHECK-NEXT:                   Offset: 0
// CHECK-NEXT:             }
// CHECK-NEXT:           CBuffer: CB2
// CHECK-NEXT:       }

#endif