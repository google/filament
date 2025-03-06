// RUN: %dxc -T ps_6_0 -E main %s | %D3DReflect %s | FileCheck %s

Texture2DMS<float4> msTexture;
Texture2DMSArray<float4> msTextureArray : register(t2, space2);

// Not sure what -1 means, but it's legal for fxc, and it's preserved, so we match it here.
Texture2DMS<float4, -1> msTexture1 : register(t1);
Texture2DMSArray<float4, -1> msTextureArray1 : register(t2);

Texture2DMS<float4, 8> msTexture2 : register(t3);
Texture2DMSArray<float4, 4> msTextureArray2 : register(t4);

float4 main(uint4 color : COLOR) : SV_TARGET
{
  return float4(0,0,0,0)
   + msTexture.sample[2][color.xy]
   + msTextureArray.sample[3][color.xyz]
   + msTexture1.sample[2][color.xy]
   + msTextureArray1.sample[3][color.xyz]
   + msTexture2.sample[2][color.xy]
   + msTextureArray2.sample[3][color.xyz]
   ;
}

// CHECK: ID3D12ShaderReflection:
// CHECK:   Bound Resources:
// CHECK:    D3D12_SHADER_INPUT_BIND_DESC: Name: msTexture
// CHECK-NEXT:      Type: D3D_SIT_TEXTURE
// CHECK-NEXT:      uID: 0
// CHECK-NEXT:      BindCount: 1
// CHECK-NEXT:      BindPoint: 0
// CHECK-NEXT:      Space: 0
// CHECK-NEXT:      ReturnType: D3D_RETURN_TYPE_FLOAT
// CHECK-NEXT:      Dimension: D3D_SRV_DIMENSION_TEXTURE2DMS
// CHECK-NEXT:      NumSamples (or stride): 0
// CHECK-NEXT:      uFlags: (D3D_SIF_TEXTURE_COMPONENT_0 | D3D_SIF_TEXTURE_COMPONENT_1)
// CHECK:    D3D12_SHADER_INPUT_BIND_DESC: Name: msTextureArray
// CHECK-NEXT:      Type: D3D_SIT_TEXTURE
// CHECK-NEXT:      uID: 1
// CHECK-NEXT:      BindCount: 1
// CHECK-NEXT:      BindPoint: 2
// CHECK-NEXT:      Space: 2
// CHECK-NEXT:      ReturnType: D3D_RETURN_TYPE_FLOAT
// CHECK-NEXT:      Dimension: D3D_SRV_DIMENSION_TEXTURE2DMSARRAY
// CHECK-NEXT:      NumSamples (or stride): 0
// CHECK-NEXT:      uFlags: (D3D_SIF_TEXTURE_COMPONENT_0 | D3D_SIF_TEXTURE_COMPONENT_1)
// CHECK:    D3D12_SHADER_INPUT_BIND_DESC: Name: msTexture1
// CHECK-NEXT:      Type: D3D_SIT_TEXTURE
// CHECK-NEXT:      uID: 2
// CHECK-NEXT:      BindCount: 1
// CHECK-NEXT:      BindPoint: 1
// CHECK-NEXT:      Space: 0
// CHECK-NEXT:      ReturnType: D3D_RETURN_TYPE_FLOAT
// CHECK-NEXT:      Dimension: D3D_SRV_DIMENSION_TEXTURE2DMS
// CHECK-NEXT:      NumSamples (or stride): 4294967295
// CHECK-NEXT:      uFlags: (D3D_SIF_TEXTURE_COMPONENT_0 | D3D_SIF_TEXTURE_COMPONENT_1)
// CHECK:    D3D12_SHADER_INPUT_BIND_DESC: Name: msTextureArray1
// CHECK-NEXT:      Type: D3D_SIT_TEXTURE
// CHECK-NEXT:      uID: 3
// CHECK-NEXT:      BindCount: 1
// CHECK-NEXT:      BindPoint: 2
// CHECK-NEXT:      Space: 0
// CHECK-NEXT:      ReturnType: D3D_RETURN_TYPE_FLOAT
// CHECK-NEXT:      Dimension: D3D_SRV_DIMENSION_TEXTURE2DMSARRAY
// CHECK-NEXT:      NumSamples (or stride): 4294967295
// CHECK-NEXT:      uFlags: (D3D_SIF_TEXTURE_COMPONENT_0 | D3D_SIF_TEXTURE_COMPONENT_1)
// CHECK:    D3D12_SHADER_INPUT_BIND_DESC: Name: msTexture2
// CHECK-NEXT:      Type: D3D_SIT_TEXTURE
// CHECK-NEXT:      uID: 4
// CHECK-NEXT:      BindCount: 1
// CHECK-NEXT:      BindPoint: 3
// CHECK-NEXT:      Space: 0
// CHECK-NEXT:      ReturnType: D3D_RETURN_TYPE_FLOAT
// CHECK-NEXT:      Dimension: D3D_SRV_DIMENSION_TEXTURE2DMS
// CHECK-NEXT:      NumSamples (or stride): 8
// CHECK-NEXT:      uFlags: (D3D_SIF_TEXTURE_COMPONENT_0 | D3D_SIF_TEXTURE_COMPONENT_1)
// CHECK:    D3D12_SHADER_INPUT_BIND_DESC: Name: msTextureArray2
// CHECK-NEXT:      Type: D3D_SIT_TEXTURE
// CHECK-NEXT:      uID: 5
// CHECK-NEXT:      BindCount: 1
// CHECK-NEXT:      BindPoint: 4
// CHECK-NEXT:      Space: 0
// CHECK-NEXT:      ReturnType: D3D_RETURN_TYPE_FLOAT
// CHECK-NEXT:      Dimension: D3D_SRV_DIMENSION_TEXTURE2DMSARRAY
// CHECK-NEXT:      NumSamples (or stride): 4
// CHECK-NEXT:      uFlags: (D3D_SIF_TEXTURE_COMPONENT_0 | D3D_SIF_TEXTURE_COMPONENT_1)
