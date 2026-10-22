// RUN: %dxc -E main -T vs_6_0 -Vd -validator-version 0.0 %s | %D3DReflect %s | FileCheck %s
// RUN: %dxc -E main -T vs_6_6 -Vd -validator-version 0.0 %s | %D3DReflect %s | FileCheck %s

// Verify CB variable sizes align with expectations.
// This also tests some matrix, struct, and array cases that may
// have not been covered sufficiently elsewhere.

#if 0
// CHECK:  Constant Buffers:
// CHECK-NEXT:    ID3D12ShaderReflectionConstantBuffer:
// CHECK-NEXT:      D3D12_SHADER_BUFFER_DESC: Name: CB
// CHECK-NEXT:        Type: D3D_CT_CBUFFER
// CHECK-NEXT:        Size: 2176
// CHECK-NEXT:        uFlags: 0
// CHECK-NEXT:        Num Variables: 8

// CHECK:        ID3D12ShaderReflectionVariable:
// CHECK-NEXT:          D3D12_SHADER_VARIABLE_DESC: Name: A
// CHECK-NEXT:            Size: 8
// CHECK-NEXT:            StartOffset: 0

// CHECK:        ID3D12ShaderReflectionVariable:
// CHECK-NEXT:          D3D12_SHADER_VARIABLE_DESC: Name: B
// CHECK-NEXT:            Size: 16
// CHECK-NEXT:            StartOffset: 16
// CHECK-NEXT:            uFlags: (D3D_SVF_USED)

// CHECK:        ID3D12ShaderReflectionVariable:
// CHECK-NEXT:          D3D12_SHADER_VARIABLE_DESC: Name: D
// CHECK-NEXT:            Size: 8
// CHECK-NEXT:            StartOffset: 32
// CHECK-NEXT:            uFlags: 0
// CHECK-NEXT:            DefaultValue: <nullptr>
// CHECK-NEXT:          ID3D12ShaderReflectionType:
// CHECK-NEXT:            D3D12_SHADER_TYPE_DESC: Name: double
// CHECK-NEXT:              Class: D3D_SVC_SCALAR
// CHECK-NEXT:              Type: D3D_SVT_DOUBLE
// CHECK-NEXT:              Elements: 0
// CHECK-NEXT:              Rows: 1
// CHECK-NEXT:              Columns: 1
// CHECK-NEXT:              Members: 0
// CHECK-NEXT:              Offset: 0

// CHECK:        ID3D12ShaderReflectionVariable:
// CHECK-NEXT:          D3D12_SHADER_VARIABLE_DESC: Name: f3x2
// CHECK-NEXT:            Size: 28
// CHECK-NEXT:            StartOffset: 48
// CHECK-NEXT:            uFlags: 0
// CHECK-NEXT:            DefaultValue: <nullptr>
// CHECK-NEXT:          ID3D12ShaderReflectionType:
// CHECK-NEXT:            D3D12_SHADER_TYPE_DESC: Name: float3x2
// CHECK-NEXT:              Class: D3D_SVC_MATRIX_COLUMNS
// CHECK-NEXT:              Type: D3D_SVT_FLOAT
// CHECK-NEXT:              Elements: 0
// CHECK-NEXT:              Rows: 3
// CHECK-NEXT:              Columns: 2
// CHECK-NEXT:              Members: 0
// CHECK-NEXT:              Offset: 0

// CHECK:        ID3D12ShaderReflectionVariable:
// CHECK-NEXT:          D3D12_SHADER_VARIABLE_DESC: Name: f3x2_row
// CHECK-NEXT:            Size: 40
// CHECK-NEXT:            StartOffset: 80
// CHECK-NEXT:            uFlags: 0
// CHECK-NEXT:            DefaultValue: <nullptr>
// CHECK-NEXT:          ID3D12ShaderReflectionType:
// CHECK-NEXT:            D3D12_SHADER_TYPE_DESC: Name: float3x2
// CHECK-NEXT:              Class: D3D_SVC_MATRIX_ROWS
// CHECK-NEXT:              Type: D3D_SVT_FLOAT
// CHECK-NEXT:              Elements: 0
// CHECK-NEXT:              Rows: 3
// CHECK-NEXT:              Columns: 2
// CHECK-NEXT:              Members: 0
// CHECK-NEXT:              Offset: 0

// CHECK:        ID3D12ShaderReflectionVariable:
// CHECK-NEXT:          D3D12_SHADER_VARIABLE_DESC: Name: d3x4
// CHECK-NEXT:            Size: 120
// CHECK-NEXT:            StartOffset: 128
// CHECK-NEXT:            uFlags: 0
// CHECK-NEXT:            DefaultValue: <nullptr>
// CHECK-NEXT:          ID3D12ShaderReflectionType:
// CHECK-NEXT:            D3D12_SHADER_TYPE_DESC: Name: double3x4
// CHECK-NEXT:              Class: D3D_SVC_MATRIX_COLUMNS
// CHECK-NEXT:              Type: D3D_SVT_DOUBLE
// CHECK-NEXT:              Elements: 0
// CHECK-NEXT:              Rows: 3
// CHECK-NEXT:              Columns: 4
// CHECK-NEXT:              Members: 0
// CHECK-NEXT:              Offset: 0

// CHECK:        ID3D12ShaderReflectionVariable:
// CHECK-NEXT:          D3D12_SHADER_VARIABLE_DESC: Name: s1
// CHECK-NEXT:            Size: 312
// CHECK-NEXT:            StartOffset: 256
// CHECK-NEXT:            uFlags: 0
// CHECK-NEXT:            DefaultValue: <nullptr>
// CHECK-NEXT:          ID3D12ShaderReflectionType:
// CHECK-NEXT:            D3D12_SHADER_TYPE_DESC: Name: S1
// CHECK-NEXT:              Class: D3D_SVC_STRUCT
// CHECK-NEXT:              Type: D3D_SVT_VOID
// CHECK-NEXT:              Elements: 0
// CHECK-NEXT:              Rows: 1
// CHECK-NEXT:              Columns: 39
// CHECK-NEXT:              Members: 5
// CHECK-NEXT:              Offset: 0
// CHECK-NEXT:            {
// CHECK-NEXT:              ID3D12ShaderReflectionType:
// CHECK-NEXT:                D3D12_SHADER_TYPE_DESC: Name: int
// CHECK-NEXT:                  Class: D3D_SVC_SCALAR
// CHECK-NEXT:                  Type: D3D_SVT_INT
// CHECK-NEXT:                  Elements: 0
// CHECK-NEXT:                  Rows: 1
// CHECK-NEXT:                  Columns: 1
// CHECK-NEXT:                  Members: 0
// CHECK-NEXT:                  Offset: 0
// CHECK-NEXT:              ID3D12ShaderReflectionType:
// CHECK-NEXT:                D3D12_SHADER_TYPE_DESC: Name: float3x2
// CHECK-NEXT:                  Class: D3D_SVC_MATRIX_COLUMNS
// CHECK-NEXT:                  Type: D3D_SVT_FLOAT
// CHECK-NEXT:                  Elements: 0
// CHECK-NEXT:                  Rows: 3
// CHECK-NEXT:                  Columns: 2
// CHECK-NEXT:                  Members: 0
// CHECK-NEXT:                  Offset: 16
// CHECK-NEXT:              ID3D12ShaderReflectionType:
// CHECK-NEXT:                D3D12_SHADER_TYPE_DESC: Name: double3x4
// CHECK-NEXT:                  Class: D3D_SVC_MATRIX_COLUMNS
// CHECK-NEXT:                  Type: D3D_SVT_DOUBLE
// CHECK-NEXT:                  Elements: 0
// CHECK-NEXT:                  Rows: 3
// CHECK-NEXT:                  Columns: 4
// CHECK-NEXT:                  Members: 0
// CHECK-NEXT:                  Offset: 48
// CHECK-NEXT:              ID3D12ShaderReflectionType:
// CHECK-NEXT:                D3D12_SHADER_TYPE_DESC: Name: int2x1
// CHECK-NEXT:                  Class: D3D_SVC_MATRIX_COLUMNS
// CHECK-NEXT:                  Type: D3D_SVT_INT
// CHECK-NEXT:                  Elements: 0
// CHECK-NEXT:                  Rows: 2
// CHECK-NEXT:                  Columns: 1
// CHECK-NEXT:                  Members: 0
// CHECK-NEXT:                  Offset: 168
// CHECK-NEXT:              ID3D12ShaderReflectionType:
// CHECK-NEXT:                D3D12_SHADER_TYPE_DESC: Name: float3x2
// CHECK-NEXT:                  Class: D3D_SVC_MATRIX_ROWS
// CHECK-NEXT:                  Type: D3D_SVT_FLOAT
// CHECK-NEXT:                  Elements: 3
// CHECK-NEXT:                  Rows: 3
// CHECK-NEXT:                  Columns: 2
// CHECK-NEXT:                  Members: 0
// CHECK-NEXT:                  Offset: 176

// CHECK:        ID3D12ShaderReflectionVariable:
// CHECK-NEXT:          D3D12_SHADER_VARIABLE_DESC: Name: s1_arr
// CHECK-NEXT:            Size: 1592
// CHECK-NEXT:            StartOffset: 576
// CHECK-NEXT:            uFlags: 0
// CHECK-NEXT:            DefaultValue: <nullptr>
// CHECK-NEXT:          ID3D12ShaderReflectionType:
// CHECK-NEXT:            D3D12_SHADER_TYPE_DESC: Name: S1
// CHECK-NEXT:              Class: D3D_SVC_STRUCT
// CHECK-NEXT:              Type: D3D_SVT_VOID
// CHECK-NEXT:              Elements: 5
// CHECK-NEXT:              Rows: 1
// CHECK-NEXT:              Columns: 39
// CHECK-NEXT:              Members: 5
// CHECK-NEXT:              Offset: 0
// CHECK-NEXT:            {
// CHECK-NEXT:              ID3D12ShaderReflectionType:
// CHECK-NEXT:                D3D12_SHADER_TYPE_DESC: Name: int
// CHECK-NEXT:                  Class: D3D_SVC_SCALAR
// CHECK-NEXT:                  Type: D3D_SVT_INT
// CHECK-NEXT:                  Elements: 0
// CHECK-NEXT:                  Rows: 1
// CHECK-NEXT:                  Columns: 1
// CHECK-NEXT:                  Members: 0
// CHECK-NEXT:                  Offset: 0
// CHECK-NEXT:              ID3D12ShaderReflectionType:
// CHECK-NEXT:                D3D12_SHADER_TYPE_DESC: Name: float3x2
// CHECK-NEXT:                  Class: D3D_SVC_MATRIX_COLUMNS
// CHECK-NEXT:                  Type: D3D_SVT_FLOAT
// CHECK-NEXT:                  Elements: 0
// CHECK-NEXT:                  Rows: 3
// CHECK-NEXT:                  Columns: 2
// CHECK-NEXT:                  Members: 0
// CHECK-NEXT:                  Offset: 16
// CHECK-NEXT:              ID3D12ShaderReflectionType:
// CHECK-NEXT:                D3D12_SHADER_TYPE_DESC: Name: double3x4
// CHECK-NEXT:                  Class: D3D_SVC_MATRIX_COLUMNS
// CHECK-NEXT:                  Type: D3D_SVT_DOUBLE
// CHECK-NEXT:                  Elements: 0
// CHECK-NEXT:                  Rows: 3
// CHECK-NEXT:                  Columns: 4
// CHECK-NEXT:                  Members: 0
// CHECK-NEXT:                  Offset: 48
// CHECK-NEXT:              ID3D12ShaderReflectionType:
// CHECK-NEXT:                D3D12_SHADER_TYPE_DESC: Name: int2x1
// CHECK-NEXT:                  Class: D3D_SVC_MATRIX_COLUMNS
// CHECK-NEXT:                  Type: D3D_SVT_INT
// CHECK-NEXT:                  Elements: 0
// CHECK-NEXT:                  Rows: 2
// CHECK-NEXT:                  Columns: 1
// CHECK-NEXT:                  Members: 0
// CHECK-NEXT:                  Offset: 168
// CHECK-NEXT:              ID3D12ShaderReflectionType:
// CHECK-NEXT:                D3D12_SHADER_TYPE_DESC: Name: float3x2
// CHECK-NEXT:                  Class: D3D_SVC_MATRIX_ROWS
// CHECK-NEXT:                  Type: D3D_SVT_FLOAT
// CHECK-NEXT:                  Elements: 3
// CHECK-NEXT:                  Rows: 3
// CHECK-NEXT:                  Columns: 2
// CHECK-NEXT:                  Members: 0
// CHECK-NEXT:                  Offset: 176

#endif

struct S1 {
  int i;
  float3x2 f3x2;
  double3x4 d3x4;
  int2x1 i2x1;
  row_major float3x2 f3x2_row[3];
};

cbuffer CB {
  float2 A;
  float4 B;
  double D;
  float3x2 f3x2;
  row_major float3x2 f3x2_row;
  double3x4 d3x4;
  S1 s1;
  S1 s1_arr[5];
}

float4 main() : OUT {
  return B;
}
