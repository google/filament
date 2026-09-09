// RUN: %dxilver 1.7 | %dxc -T lib_6_6 -HV 2021 -E main -Vd -validator-version 0.0   %s | %D3DReflect %s | FileCheck %s

// Make sure bit field on enum works.

// CHECK:          ID3D12ShaderReflectionVariable:
// CHECK-NEXT:            D3D12_SHADER_VARIABLE_DESC: Name: ss
// CHECK-NEXT:              Size: 4
// CHECK-NEXT:              StartOffset: 0
// CHECK-NEXT:              uFlags: (D3D_SVF_USED)
// CHECK-NEXT:              DefaultValue: <nullptr>
// CHECK-NEXT:            ID3D12ShaderReflectionType:
// CHECK-NEXT:              D3D12_SHADER_TYPE_DESC: Name: SomeStruct
// CHECK-NEXT:                Class: D3D_SVC_STRUCT
// CHECK-NEXT:                Type: D3D_SVT_VOID
// CHECK-NEXT:                Elements: 0
// CHECK-NEXT:                Rows: 1
// CHECK-NEXT:                Columns: 1
// CHECK-NEXT:                Members: 1
// CHECK-NEXT:                Offset: 0
// CHECK-NEXT:              {
// CHECK-NEXT:                ID3D12ShaderReflectionType:
// CHECK-NEXT:                  D3D12_SHADER_TYPE_DESC: Name: dword
// CHECK-NEXT:                    Class: D3D_SVC_SCALAR
// CHECK-NEXT:                    Type: D3D_SVT_UINT
// CHECK-NEXT:                    Elements: 0
// CHECK-NEXT:                    Rows: 1
// CHECK-NEXT:                    Columns: 1
// CHECK-NEXT:                    Members: 3
// CHECK-NEXT:                    Offset: 0
// CHECK-NEXT:                  {
// CHECK-NEXT:                    ID3D12ShaderReflectionType:
// CHECK-NEXT:                      D3D12_SHADER_TYPE_DESC: Name: dword
// CHECK-NEXT:                        Class: D3D_SVC_BIT_FIELD
// CHECK-NEXT:                        Type: D3D_SVT_UINT
// CHECK-NEXT:                        Elements: 0
// CHECK-NEXT:                        Rows: 1
// CHECK-NEXT:                        Columns: 16
// CHECK-NEXT:                        Members: 0
// CHECK-NEXT:                        Offset: 0
// CHECK-NEXT:                    ID3D12ShaderReflectionType:
// CHECK-NEXT:                      D3D12_SHADER_TYPE_DESC: Name: dword
// CHECK-NEXT:                        Class: D3D_SVC_BIT_FIELD
// CHECK-NEXT:                        Type: D3D_SVT_UINT
// CHECK-NEXT:                        Elements: 0
// CHECK-NEXT:                        Rows: 1
// CHECK-NEXT:                        Columns: 3
// CHECK-NEXT:                        Members: 0
// CHECK-NEXT:                        Offset: 16
// CHECK-NEXT:                    ID3D12ShaderReflectionType:
// CHECK-NEXT:                      D3D12_SHADER_TYPE_DESC: Name: dword
// CHECK-NEXT:                        Class: D3D_SVC_BIT_FIELD
// CHECK-NEXT:                        Type: D3D_SVT_UINT
// CHECK-NEXT:                        Elements: 0
// CHECK-NEXT:                        Rows: 1
// CHECK-NEXT:                        Columns: 2
// CHECK-NEXT:                        Members: 0
// CHECK-NEXT:                        Offset: 19
// CHECK-NEXT:                  }
// CHECK-NEXT:              }
// CHECK-NEXT:            CBuffer: $Globals
enum SomeEnum { Val1 };

using u32 = uint32_t;
using SE = SomeEnum;
typedef SomeEnum SE2;

struct SomeStruct
{
    u32 m1 : 16;
    SE m3 : 3;
    SE2 m4 : 2;
};

SomeStruct ss;

export int SomeFunc()
{
    return (int)ss.m3;
}
