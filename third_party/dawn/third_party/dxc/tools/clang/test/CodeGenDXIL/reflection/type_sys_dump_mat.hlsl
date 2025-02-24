// RUN: %dxc %s -Tlib_6_x -Fo %t
// RUN: %dxa %t -dumpreflection | FileCheck %s

// CHECK: ID3D12ShaderReflectionConstantBuffer:
// CHECK-NEXT:        D3D12_SHADER_BUFFER_DESC: Name: s
// CHECK-NEXT:          Type: D3D_CT_CBUFFER
// CHECK-NEXT:          Size: 288
// CHECK-NEXT:          uFlags: 0
// CHECK-NEXT:          Num Variables: 1
// CHECK-NEXT:        {
// CHECK-NEXT:          ID3D12ShaderReflectionVariable:
// CHECK-NEXT:            D3D12_SHADER_VARIABLE_DESC: Name: s
// CHECK-NEXT:              Size: 276
// CHECK-NEXT:              StartOffset: 0
// CHECK-NEXT:              uFlags: (D3D_SVF_USED)
// CHECK-NEXT:              DefaultValue: <nullptr>
// CHECK-NEXT:            ID3D12ShaderReflectionType:
// CHECK-NEXT:              D3D12_SHADER_TYPE_DESC: Name: S
// CHECK-NEXT:                Class: D3D_SVC_STRUCT
// CHECK-NEXT:                Type: D3D_SVT_VOID
// CHECK-NEXT:                Elements: 0
// CHECK-NEXT:                Rows: 1
// CHECK-NEXT:                Columns: 42
// CHECK-NEXT:                Members: 6
// CHECK-NEXT:                Offset: 0
// CHECK-NEXT:              {
// CHECK-NEXT:                ID3D12ShaderReflectionType:
// CHECK-NEXT:                  D3D12_SHADER_TYPE_DESC: Name: int4x3
// CHECK-NEXT:                    Class: D3D_SVC_MATRIX_ROWS
// CHECK-NEXT:                    Type: D3D_SVT_INT
// CHECK-NEXT:                    Elements: 0
// CHECK-NEXT:                    Rows: 4
// CHECK-NEXT:                    Columns: 3
// CHECK-NEXT:                    Members: 0
// CHECK-NEXT:                    Offset: 0
// CHECK-NEXT:                ID3D12ShaderReflectionType:
// CHECK-NEXT:                  D3D12_SHADER_TYPE_DESC: Name: int1x3
// CHECK-NEXT:                    Class: D3D_SVC_MATRIX_COLUMNS
// CHECK-NEXT:                    Type: D3D_SVT_INT
// CHECK-NEXT:                    Elements: 2
// CHECK-NEXT:                    Rows: 1
// CHECK-NEXT:                    Columns: 3
// CHECK-NEXT:                    Members: 0
// CHECK-NEXT:                    Offset: 64
// CHECK-NEXT:                ID3D12ShaderReflectionType:
// CHECK-NEXT:                  D3D12_SHADER_TYPE_DESC: Name: int3x1
// CHECK-NEXT:                    Class: D3D_SVC_MATRIX_COLUMNS
// CHECK-NEXT:                    Type: D3D_SVT_INT
// CHECK-NEXT:                    Elements: 0
// CHECK-NEXT:                    Rows: 3
// CHECK-NEXT:                    Columns: 1
// CHECK-NEXT:                    Members: 0
// CHECK-NEXT:                    Offset: 148
// CHECK-NEXT:                ID3D12ShaderReflectionType:
// CHECK-NEXT:                  D3D12_SHADER_TYPE_DESC: Name: int4x3
// CHECK-NEXT:                    Class: D3D_SVC_MATRIX_COLUMNS
// CHECK-NEXT:                    Type: D3D_SVT_INT
// CHECK-NEXT:                    Elements: 0
// CHECK-NEXT:                    Rows: 4
// CHECK-NEXT:                    Columns: 3
// CHECK-NEXT:                    Members: 0
// CHECK-NEXT:                    Offset: 160
// CHECK-NEXT:                ID3D12ShaderReflectionType:
// CHECK-NEXT:                  D3D12_SHADER_TYPE_DESC: Name: int1x3
// CHECK-NEXT:                    Class: D3D_SVC_MATRIX_ROWS
// CHECK-NEXT:                    Type: D3D_SVT_INT
// CHECK-NEXT:                    Elements: 2
// CHECK-NEXT:                    Rows: 1
// CHECK-NEXT:                    Columns: 3
// CHECK-NEXT:                    Members: 0
// CHECK-NEXT:                    Offset: 208
// CHECK-NEXT:                ID3D12ShaderReflectionType:
// CHECK-NEXT:                  D3D12_SHADER_TYPE_DESC: Name: int3x1
// CHECK-NEXT:                    Class: D3D_SVC_MATRIX_ROWS
// CHECK-NEXT:                    Type: D3D_SVT_INT
// CHECK-NEXT:                    Elements: 0
// CHECK-NEXT:                    Rows: 3
// CHECK-NEXT:                    Columns: 1
// CHECK-NEXT:                    Members: 0
// CHECK-NEXT:                    Offset: 240
// CHECK-NEXT:              }
// CHECK-NEXT:            CBuffer: s

struct S {
    row_major int4x3 m;
	int1x3 m1[2];
	int3x1 m2;
    int4x3 m3;
  row_major int1x3 m4[2];
  row_major int3x1 m5;
};

ConstantBuffer<S> s;


export
float foo() {
  return s.m5[0];
}
