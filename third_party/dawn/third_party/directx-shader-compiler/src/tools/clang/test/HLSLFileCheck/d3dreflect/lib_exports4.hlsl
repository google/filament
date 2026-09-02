// RUN: %dxc -auto-binding-space 13 -exports VSMain=PSMain;PSMain=VSMain -T lib_6_3 %s | %D3DReflect %s | FileCheck %s

Buffer<int> T0;

Texture2D<float4> T1;

struct Foo { uint u; float f; };
StructuredBuffer<Foo> T2;

[shader("vertex")]
float4 VSMain(int3 coord : COORD) : SV_Position {
  return T1.Load(coord);
}

[shader("pixel")]
float4 PSMain(int idx : INDEX) : SV_Target {
  return T2[T0.Load(idx)].f;
}


// CHECK: ID3D12LibraryReflection:
// CHECK:   D3D12_LIBRARY_DESC:
// CHECK:     FunctionCount: 4
// CHECK:   ID3D12FunctionReflection:
// CHECK:     D3D12_FUNCTION_DESC: Name: \01?PSMain{{[@$?.A-Za-z0-9_]+}}
// CHECK:       Shader Version: Library 6.3
// CHECK:       BoundResources: 1
// CHECK:     Bound Resources:
// CHECK:       D3D12_SHADER_INPUT_BIND_DESC: Name: T1
// CHECK:         Type: D3D_SIT_TEXTURE
// CHECK:         uID: 1
// CHECK:         BindCount: 1
// CHECK:         BindPoint: 1
// CHECK:         Space: 13
// CHECK:         ReturnType: D3D_RETURN_TYPE_FLOAT
// CHECK:         Dimension: D3D_SRV_DIMENSION_TEXTURE2D
// CHECK:         NumSamples (or stride): 4294967295
// CHECK:         uFlags: (D3D_SIF_TEXTURE_COMPONENT_0 | D3D_SIF_TEXTURE_COMPONENT_1)
// CHECK:   ID3D12FunctionReflection:
// CHECK:     D3D12_FUNCTION_DESC: Name: \01?VSMain{{[@$?.A-Za-z0-9_]+}}
// CHECK:       Shader Version: Library 6.3
// CHECK:       BoundResources: 2
// CHECK:     Bound Resources:
// CHECK:       D3D12_SHADER_INPUT_BIND_DESC: Name: T0
// CHECK:         Type: D3D_SIT_TEXTURE
// CHECK:         uID: 0
// CHECK:         BindCount: 1
// CHECK:         BindPoint: 0
// CHECK:         Space: 13
// CHECK:         ReturnType: D3D_RETURN_TYPE_SINT
// CHECK:         Dimension: D3D_SRV_DIMENSION_BUFFER
// CHECK:         NumSamples (or stride): 4294967295
// CHECK:         uFlags: 0
// CHECK:       D3D12_SHADER_INPUT_BIND_DESC: Name: T2
// CHECK:         Type: D3D_SIT_STRUCTURED
// CHECK:         uID: 2
// CHECK:         BindCount: 1
// CHECK:         BindPoint: 2
// CHECK:         Space: 13
// CHECK:         ReturnType: D3D_RETURN_TYPE_MIXED
// CHECK:         Dimension: D3D_SRV_DIMENSION_BUFFER
// CHECK:         NumSamples (or stride): 8
// CHECK:         uFlags: 0
// CHECK:   ID3D12FunctionReflection:
// CHECK:     D3D12_FUNCTION_DESC: Name: PSMain
// CHECK:       Shader Version: Vertex 6.3
// CHECK:       BoundResources: 1
// CHECK:     Bound Resources:
// CHECK:       D3D12_SHADER_INPUT_BIND_DESC: Name: T1
// CHECK:         Type: D3D_SIT_TEXTURE
// CHECK:         uID: 1
// CHECK:         BindCount: 1
// CHECK:         BindPoint: 1
// CHECK:         Space: 13
// CHECK:         ReturnType: D3D_RETURN_TYPE_FLOAT
// CHECK:         Dimension: D3D_SRV_DIMENSION_TEXTURE2D
// CHECK:         NumSamples (or stride): 4294967295
// CHECK:         uFlags: (D3D_SIF_TEXTURE_COMPONENT_0 | D3D_SIF_TEXTURE_COMPONENT_1)
// CHECK:   ID3D12FunctionReflection:
// CHECK:     D3D12_FUNCTION_DESC: Name: VSMain
// CHECK:       Shader Version: Pixel 6.3
// CHECK:       BoundResources: 2
// CHECK:     Bound Resources:
// CHECK:       D3D12_SHADER_INPUT_BIND_DESC: Name: T0
// CHECK:         Type: D3D_SIT_TEXTURE
// CHECK:         uID: 0
// CHECK:         BindCount: 1
// CHECK:         BindPoint: 0
// CHECK:         Space: 13
// CHECK:         ReturnType: D3D_RETURN_TYPE_SINT
// CHECK:         Dimension: D3D_SRV_DIMENSION_BUFFER
// CHECK:         NumSamples (or stride): 4294967295
// CHECK:         uFlags: 0
// CHECK:       D3D12_SHADER_INPUT_BIND_DESC: Name: T2
// CHECK:         Type: D3D_SIT_STRUCTURED
// CHECK:         uID: 2
// CHECK:         BindCount: 1
// CHECK:         BindPoint: 2
// CHECK:         Space: 13
// CHECK:         ReturnType: D3D_RETURN_TYPE_MIXED
// CHECK:         Dimension: D3D_SRV_DIMENSION_BUFFER
// CHECK:         NumSamples (or stride): 8
// CHECK:         uFlags: 0
