// RUN: %dxc -T lib_6_3 -auto-binding-space 11 -default-linkage external %s | %D3DReflect %s | FileCheck %s

struct Foo
{
    float4 a;
    uint b;
};

RWStructuredBuffer<Foo> g_buffer[2] : register(u0);

uint UseBuf(int2 idx) {
  return g_buffer[idx.x][idx.y].b;
}

// CHECK: ID3D12LibraryReflection:
// CHECK:   D3D12_LIBRARY_DESC:
// CHECK:     FunctionCount: 1
// CHECK:   ID3D12FunctionReflection:
// CHECK:     D3D12_FUNCTION_DESC: Name: \01?UseBuf{{[@$?.A-Za-z0-9_]+}}
// CHECK:       Shader Version: Library 6.3
// CHECK:       BoundResources: 1
// CHECK:     Bound Resources:
// CHECK:       D3D12_SHADER_INPUT_BIND_DESC: Name: g_buffer
// CHECK:         Type: D3D_SIT_UAV_RWSTRUCTURED
// CHECK:         uID: 0
// CHECK:         BindCount: 2
// CHECK:         BindPoint: 0
// CHECK:         ReturnType: D3D_RETURN_TYPE_MIXED
// CHECK:         Dimension: D3D_SRV_DIMENSION_BUFFER
// CHECK:         NumSamples (or stride): 20
