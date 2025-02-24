// RUN: %dxc -E main -T ps_6_5 %s | %D3DReflect %s | FileCheck %s

// Test reflection for FeedbackTexture2D[Array]

FeedbackTexture2D<SAMPLER_FEEDBACK_MIN_MIP> feedbackMinMip;
FeedbackTexture2D<SAMPLER_FEEDBACK_MIP_REGION_USED> feedbackMipRegionUsed;
FeedbackTexture2DArray<SAMPLER_FEEDBACK_MIN_MIP> feedbackMinMipArray;
FeedbackTexture2DArray<SAMPLER_FEEDBACK_MIP_REGION_USED> feebackMipRegionUsedArray;
Texture2D<float> texture2D;
Texture2DArray<float> texture2DArray;
SamplerState samp;

float main() : SV_Target
{
    float2 coords2D = float2(1, 2);
    float3 coords2DArray = float3(1, 2, 3);
    float clamp = 4;
    float bias = 0.5F;
    float lod = 6;
    float2 ddx = float2(1.0F / 32, 2.0F / 32);
    float2 ddy = float2(3.0F / 32, 4.0F / 32);

    float idx = 0;  // Make each coord set unique
    
    feedbackMinMip.WriteSamplerFeedback(texture2D, samp, coords2D + (10 * idx++), clamp);
    feedbackMipRegionUsed.WriteSamplerFeedback(texture2D, samp, coords2D + (10 * idx++));
    feedbackMinMipArray.WriteSamplerFeedback(texture2DArray, samp, coords2DArray + (10 * idx++));
    feebackMipRegionUsedArray.WriteSamplerFeedback(texture2DArray, samp, coords2DArray + (10 * idx++));

    return 0;
}

// CHECK: ID3D12ShaderReflection:
// CHECK-NEXT:   D3D12_SHADER_DESC:
// CHECK-NEXT:     Shader Version: Pixel 6.5
// CHECK:     Flags: 0
// CHECK-NEXT:     ConstantBuffers: 0
// CHECK-NEXT:     BoundResources: 7
// CHECK-NEXT:     InputParameters: 0
// CHECK-NEXT:     OutputParameters: 1
// CHECK:   Bound Resources:
// CHECK-NEXT:     D3D12_SHADER_INPUT_BIND_DESC: Name: samp
// CHECK-NEXT:       Type: D3D_SIT_SAMPLER
// CHECK-NEXT:       uID: 0
// CHECK-NEXT:       BindCount: 1
// CHECK-NEXT:       BindPoint: 0
// CHECK-NEXT:       Space: 0
// CHECK-NEXT:       ReturnType: <unknown: 0>
// CHECK-NEXT:       Dimension: D3D_SRV_DIMENSION_UNKNOWN
// CHECK-NEXT:       NumSamples (or stride): 0
// CHECK-NEXT:       uFlags: 0
// CHECK-NEXT:     D3D12_SHADER_INPUT_BIND_DESC: Name: texture2D
// CHECK-NEXT:       Type: D3D_SIT_TEXTURE
// CHECK-NEXT:       uID: 0
// CHECK-NEXT:       BindCount: 1
// CHECK-NEXT:       BindPoint: 0
// CHECK-NEXT:       Space: 0
// CHECK-NEXT:       ReturnType: D3D_RETURN_TYPE_FLOAT
// CHECK-NEXT:       Dimension: D3D_SRV_DIMENSION_TEXTURE2D
// CHECK-NEXT:       NumSamples (or stride): 4294967295
// CHECK-NEXT:       uFlags: 0
// CHECK-NEXT:     D3D12_SHADER_INPUT_BIND_DESC: Name: texture2DArray
// CHECK-NEXT:       Type: D3D_SIT_TEXTURE
// CHECK-NEXT:       uID: 1
// CHECK-NEXT:       BindCount: 1
// CHECK-NEXT:       BindPoint: 1
// CHECK-NEXT:       Space: 0
// CHECK-NEXT:       ReturnType: D3D_RETURN_TYPE_FLOAT
// CHECK-NEXT:       Dimension: D3D_SRV_DIMENSION_TEXTURE2DARRAY
// CHECK-NEXT:       NumSamples (or stride): 4294967295
// CHECK-NEXT:       uFlags: 0
// CHECK-NEXT:     D3D12_SHADER_INPUT_BIND_DESC: Name: feedbackMinMip
// CHECK-NEXT:       Type: D3D_SIT_UAV_FEEDBACKTEXTURE
// CHECK-NEXT:       uID: 0
// CHECK-NEXT:       BindCount: 1
// CHECK-NEXT:       BindPoint: 0
// CHECK-NEXT:       Space: 0
// CHECK-NEXT:       ReturnType: D3D_RETURN_TYPE_MIXED
// CHECK-NEXT:       Dimension: D3D_SRV_DIMENSION_TEXTURE2D
// CHECK-NEXT:       NumSamples (or stride): 4294967295
// CHECK-NEXT:       uFlags: 0
// CHECK-NEXT:     D3D12_SHADER_INPUT_BIND_DESC: Name: feedbackMipRegionUsed
// CHECK-NEXT:       Type: D3D_SIT_UAV_FEEDBACKTEXTURE
// CHECK-NEXT:       uID: 1
// CHECK-NEXT:       BindCount: 1
// CHECK-NEXT:       BindPoint: 1
// CHECK-NEXT:       Space: 0
// CHECK-NEXT:       ReturnType: D3D_RETURN_TYPE_MIXED
// CHECK-NEXT:       Dimension: D3D_SRV_DIMENSION_TEXTURE2D
// CHECK-NEXT:       NumSamples (or stride): 4294967295
// CHECK-NEXT:       uFlags: 0
// CHECK-NEXT:     D3D12_SHADER_INPUT_BIND_DESC: Name: feedbackMinMipArray
// CHECK-NEXT:       Type: D3D_SIT_UAV_FEEDBACKTEXTURE
// CHECK-NEXT:       uID: 2
// CHECK-NEXT:       BindCount: 1
// CHECK-NEXT:       BindPoint: 2
// CHECK-NEXT:       Space: 0
// CHECK-NEXT:       ReturnType: D3D_RETURN_TYPE_MIXED
// CHECK-NEXT:       Dimension: D3D_SRV_DIMENSION_TEXTURE2DARRAY
// CHECK-NEXT:       NumSamples (or stride): 4294967295
// CHECK-NEXT:       uFlags: 0
// CHECK-NEXT:     D3D12_SHADER_INPUT_BIND_DESC: Name: feebackMipRegionUsedArray
// CHECK-NEXT:       Type: D3D_SIT_UAV_FEEDBACKTEXTURE
// CHECK-NEXT:       uID: 3
// CHECK-NEXT:       BindCount: 1
// CHECK-NEXT:       BindPoint: 3
// CHECK-NEXT:       Space: 0
// CHECK-NEXT:       ReturnType: D3D_RETURN_TYPE_MIXED
// CHECK-NEXT:       Dimension: D3D_SRV_DIMENSION_TEXTURE2DARRAY
// CHECK-NEXT:       NumSamples (or stride): 4294967295
// CHECK-NEXT:       uFlags: 0
