#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wmissing-braces"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

template<typename T, size_t Num>
struct spvUnsafeArray
{
    T elements[Num ? Num : 1];
    
    thread T& operator [] (size_t pos) thread
    {
        return elements[pos];
    }
    constexpr const thread T& operator [] (size_t pos) const thread
    {
        return elements[pos];
    }
    
    device T& operator [] (size_t pos) device
    {
        return elements[pos];
    }
    constexpr const device T& operator [] (size_t pos) const device
    {
        return elements[pos];
    }
    
    constexpr const constant T& operator [] (size_t pos) const constant
    {
        return elements[pos];
    }
    
    threadgroup T& operator [] (size_t pos) threadgroup
    {
        return elements[pos];
    }
    constexpr const threadgroup T& operator [] (size_t pos) const threadgroup
    {
        return elements[pos];
    }
};

struct FVertexFactoryInterpolantsVSToPS
{
    float4 TangentToWorld0;
    float4 TangentToWorld2;
};

struct FVertexFactoryInterpolantsVSToDS
{
    FVertexFactoryInterpolantsVSToPS InterpolantsVSToPS;
};

struct FSharedBasePassInterpolants
{
};
struct FBasePassInterpolantsVSToDS
{
    FSharedBasePassInterpolants _m0;
};

struct FBasePassVSToDS
{
    FVertexFactoryInterpolantsVSToDS FactoryInterpolants;
    FBasePassInterpolantsVSToDS BasePassInterpolants;
    float4 Position;
};

struct FFlatTessellationHSToDS
{
    FBasePassVSToDS PassSpecificData;
    float3 DisplacementScale;
    float TessellationMultiplier;
    float WorldDisplacementMultiplier;
};

struct type_Primitive
{
    float4x4 Primitive_LocalToWorld;
    float4 Primitive_InvNonUniformScaleAndDeterminantSign;
    float4 Primitive_ObjectWorldPositionAndRadius;
    float4x4 Primitive_WorldToLocal;
    float4x4 Primitive_PreviousLocalToWorld;
    float4x4 Primitive_PreviousWorldToLocal;
    packed_float3 Primitive_ActorWorldPosition;
    float Primitive_UseSingleSampleShadowFromStationaryLights;
    packed_float3 Primitive_ObjectBounds;
    float Primitive_LpvBiasMultiplier;
    float Primitive_DecalReceiverMask;
    float Primitive_PerObjectGBufferData;
    float Primitive_UseVolumetricLightmapShadowFromStationaryLights;
    float Primitive_DrawsVelocity;
    float4 Primitive_ObjectOrientation;
    float4 Primitive_NonUniformScale;
    packed_float3 Primitive_LocalObjectBoundsMin;
    uint Primitive_LightingChannelMask;
    packed_float3 Primitive_LocalObjectBoundsMax;
    uint Primitive_LightmapDataIndex;
    packed_float3 Primitive_PreSkinnedLocalBounds;
    int Primitive_SingleCaptureIndex;
    uint Primitive_OutputVelocity;
    uint PrePadding_Primitive_420;
    uint PrePadding_Primitive_424;
    uint PrePadding_Primitive_428;
    float4 Primitive_CustomPrimitiveData[4];
};

struct type_Material
{
    float4 Material_VectorExpressions[3];
    float4 Material_ScalarExpressions[1];
};

constant float4 _88 = {};

struct main0_out
{
    float3 out_var_Flat_DisplacementScales;
    float out_var_Flat_TessellationMultiplier;
    float out_var_Flat_WorldDisplacementMultiplier;
    float4 out_var_TEXCOORD10_centroid;
    float4 out_var_TEXCOORD11_centroid;
    float4 out_var_VS_To_DS_Position;
};

struct main0_in
{
    float4 in_var_TEXCOORD10_centroid [[attribute(0)]];
    float4 in_var_TEXCOORD11_centroid [[attribute(1)]];
    float4 in_var_VS_To_DS_Position [[attribute(2)]];
};

kernel void main0(main0_in in [[stage_in]], constant type_Primitive& Primitive [[buffer(0)]], constant type_Material& Material [[buffer(1)]], uint gl_InvocationID [[thread_index_in_threadgroup]], uint gl_PrimitiveID [[threadgroup_position_in_grid]], device main0_out* spvOut [[buffer(28)]], constant uint* spvIndirectParams [[buffer(29)]], device MTLTriangleTessellationFactorsHalf* spvTessLevel [[buffer(26)]], threadgroup main0_in* gl_in [[threadgroup(0)]])
{
    threadgroup FFlatTessellationHSToDS temp_var_hullMainRetVal[3];
    device main0_out* gl_out = &spvOut[gl_PrimitiveID * 3];
    if (gl_InvocationID < spvIndirectParams[0])
        gl_in[gl_InvocationID] = in;
    threadgroup_barrier(mem_flags::mem_threadgroup);
    if (gl_InvocationID >= 3)
        return;
    spvUnsafeArray<float4, 3> _90 = spvUnsafeArray<float4, 3>({ gl_in[0].in_var_TEXCOORD10_centroid, gl_in[1].in_var_TEXCOORD10_centroid, gl_in[2].in_var_TEXCOORD10_centroid });
    spvUnsafeArray<float4, 3> _91 = spvUnsafeArray<float4, 3>({ gl_in[0].in_var_TEXCOORD11_centroid, gl_in[1].in_var_TEXCOORD11_centroid, gl_in[2].in_var_TEXCOORD11_centroid });
    spvUnsafeArray<float4, 3> _104 = spvUnsafeArray<float4, 3>({ gl_in[0].in_var_VS_To_DS_Position, gl_in[1].in_var_VS_To_DS_Position, gl_in[2].in_var_VS_To_DS_Position });
    spvUnsafeArray<FBasePassVSToDS, 3> _111 = spvUnsafeArray<FBasePassVSToDS, 3>({ FBasePassVSToDS{ FVertexFactoryInterpolantsVSToDS{ FVertexFactoryInterpolantsVSToPS{ _90[0], _91[0] } }, FBasePassInterpolantsVSToDS{ { } }, _104[0] }, FBasePassVSToDS{ FVertexFactoryInterpolantsVSToDS{ FVertexFactoryInterpolantsVSToPS{ _90[1], _91[1] } }, FBasePassInterpolantsVSToDS{ { } }, _104[1] }, FBasePassVSToDS{ FVertexFactoryInterpolantsVSToDS{ FVertexFactoryInterpolantsVSToPS{ _90[2], _91[2] } }, FBasePassInterpolantsVSToDS{ { } }, _104[2] } });
    spvUnsafeArray<FBasePassVSToDS, 3> param_var_I;
    param_var_I = _111;
    float3 _128 = Primitive.Primitive_NonUniformScale.xyz * float3x3(param_var_I[gl_InvocationID].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld0.xyz, cross(param_var_I[gl_InvocationID].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld2.xyz, param_var_I[gl_InvocationID].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld0.xyz) * float3(param_var_I[gl_InvocationID].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld2.w), param_var_I[gl_InvocationID].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld2.xyz);
    gl_out[gl_InvocationID].out_var_TEXCOORD10_centroid = param_var_I[gl_InvocationID].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld0;
    gl_out[gl_InvocationID].out_var_TEXCOORD11_centroid = param_var_I[gl_InvocationID].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld2;
    gl_out[gl_InvocationID].out_var_VS_To_DS_Position = param_var_I[gl_InvocationID].Position;
    gl_out[gl_InvocationID].out_var_Flat_DisplacementScales = _128;
    gl_out[gl_InvocationID].out_var_Flat_TessellationMultiplier = Material.Material_ScalarExpressions[0].x;
    gl_out[gl_InvocationID].out_var_Flat_WorldDisplacementMultiplier = 1.0;
    temp_var_hullMainRetVal[gl_InvocationID] = FFlatTessellationHSToDS{ param_var_I[gl_InvocationID], _128, Material.Material_ScalarExpressions[0].x, 1.0 };
    threadgroup_barrier(mem_flags::mem_device | mem_flags::mem_threadgroup);
    if (gl_InvocationID == 0u)
    {
        float4 _154;
        _154.x = 0.5 * (temp_var_hullMainRetVal[1u].TessellationMultiplier + temp_var_hullMainRetVal[2u].TessellationMultiplier);
        _154.y = 0.5 * (temp_var_hullMainRetVal[2u].TessellationMultiplier + temp_var_hullMainRetVal[0u].TessellationMultiplier);
        _154.z = 0.5 * (temp_var_hullMainRetVal[0u].TessellationMultiplier + temp_var_hullMainRetVal[1u].TessellationMultiplier);
        _154.w = 0.333000004291534423828125 * ((temp_var_hullMainRetVal[0u].TessellationMultiplier + temp_var_hullMainRetVal[1u].TessellationMultiplier) + temp_var_hullMainRetVal[2u].TessellationMultiplier);
        float4 _173 = fast::clamp(_154, float4(1.0), float4(15.0));
        spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[0u] = half(_173.x);
        spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[1u] = half(_173.y);
        spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[2u] = half(_173.z);
        spvTessLevel[gl_PrimitiveID].insideTessellationFactor = half(_173.w);
    }
}

