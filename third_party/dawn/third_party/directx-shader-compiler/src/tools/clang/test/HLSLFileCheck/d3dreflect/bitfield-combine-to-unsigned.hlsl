// RUN: %dxilver 1.7 | %dxc -E main -T ps_6_0 -HV 2021 -Vd -validator-version 0.0  %s | %D3DReflect %s | FileCheck %s

// Make sure bitfiled info is saved.

// CHECK:            D3D12_SHADER_TYPE_DESC: Name: BF
// CHECK-NEXT:              Class: D3D_SVC_STRUCT
// CHECK-NEXT:              Type: D3D_SVT_VOID
// CHECK-NEXT:              Elements: 0
// CHECK-NEXT:              Rows: 1
// CHECK-NEXT:              Columns: 2
// CHECK-NEXT:              Members: 2
// CHECK-NEXT:              Offset: 0
// CHECK-NEXT:            {
// CHECK-NEXT:              ID3D12ShaderReflectionType:
// CHECK-NEXT:                D3D12_SHADER_TYPE_DESC: Name: int
// CHECK-NEXT:                  Class: D3D_SVC_SCALAR
// CHECK-NEXT:                  Type: D3D_SVT_INT
// CHECK-NEXT:                  Elements: 0
// CHECK-NEXT:                  Rows: 1
// CHECK-NEXT:                  Columns: 1
// CHECK-NEXT:                  Members: 2
// CHECK-NEXT:                  Offset: 0
// CHECK-NEXT:                {
// CHECK-NEXT:                  ID3D12ShaderReflectionType:
// CHECK-NEXT:                    D3D12_SHADER_TYPE_DESC: Name: int
// CHECK-NEXT:                      Class: D3D_SVC_BIT_FIELD
// CHECK-NEXT:                      Type: D3D_SVT_INT
// CHECK-NEXT:                      Elements: 0
// CHECK-NEXT:                      Rows: 1
// CHECK-NEXT:                      Columns: 8
// CHECK-NEXT:                      Members: 0
// CHECK-NEXT:                      Offset: 0
// CHECK-NEXT:                  ID3D12ShaderReflectionType:
// CHECK-NEXT:                    D3D12_SHADER_TYPE_DESC: Name: int
// CHECK-NEXT:                      Class: D3D_SVC_BIT_FIELD
// CHECK-NEXT:                      Type: D3D_SVT_INT
// CHECK-NEXT:                      Elements: 0
// CHECK-NEXT:                      Rows: 1
// CHECK-NEXT:                      Columns: 8
// CHECK-NEXT:                      Members: 0
// CHECK-NEXT:                      Offset: 8
// CHECK-NEXT:                }
// CHECK-NEXT:              ID3D12ShaderReflectionType:
// CHECK-NEXT:                D3D12_SHADER_TYPE_DESC: Name: uint64_t
// CHECK-NEXT:                  Class: D3D_SVC_SCALAR
// CHECK-NEXT:                  Type: D3D_SVT_UINT64
// CHECK-NEXT:                  Elements: 0
// CHECK-NEXT:                  Rows: 1
// CHECK-NEXT:                  Columns: 1
// CHECK-NEXT:                  Members: 2
// CHECK-NEXT:                  Offset: 8
// CHECK-NEXT:                {
// CHECK-NEXT:                  ID3D12ShaderReflectionType:
// CHECK-NEXT:                    D3D12_SHADER_TYPE_DESC: Name: uint64_t
// CHECK-NEXT:                      Class: D3D_SVC_BIT_FIELD
// CHECK-NEXT:                      Type: D3D_SVT_UINT64
// CHECK-NEXT:                      Elements: 0
// CHECK-NEXT:                      Rows: 1
// CHECK-NEXT:                      Columns: 8
// CHECK-NEXT:                      Members: 0
// CHECK-NEXT:                      Offset: 0
// CHECK-NEXT:                  ID3D12ShaderReflectionType:
// CHECK-NEXT:                    D3D12_SHADER_TYPE_DESC: Name: uint64_t
// CHECK-NEXT:                      Class: D3D_SVC_BIT_FIELD
// CHECK-NEXT:                      Type: D3D_SVT_UINT64
// CHECK-NEXT:                      Elements: 0
// CHECK-NEXT:                      Rows: 1
// CHECK-NEXT:                      Columns: 8
// CHECK-NEXT:                      Members: 0
// CHECK-NEXT:                      Offset: 8
// CHECK-NEXT:                }
// CHECK-NEXT:            }
// CHECK-NEXT: CBuffer: B

struct BF {
   int i0 : 8;
   uint i1 : 8;
   uint64_t i2 : 8;
   int64_t i3 : 8;
};

StructuredBuffer<BF> B;


float4 main() : SV_Target {
  return float4(B[0].i0, B[0].i1, B[0].i2, B[0].i3);
}
