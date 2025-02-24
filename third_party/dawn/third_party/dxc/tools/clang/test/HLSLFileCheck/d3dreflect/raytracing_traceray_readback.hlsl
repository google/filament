// RUN: %dxc -T lib_6_3 -auto-binding-space 11 %s | %D3DReflect %s | FileCheck %s

struct Payload {
   float4 abc;
   float4 color;
};

RWTexture2D<float4> RTOutput : register(u0);
RaytracingAccelerationStructure scene : register(t0);

int2 viewportDims;
float3 invView[4];
float tanHalfFovY;

[shader("raygeneration")]
void RayGenTestMain()
{
    uint2 LaunchIndex = DispatchRaysIndex();
    float2 d = ((LaunchIndex.xy / (float2)viewportDims) * 2.f - 1.f);
    float aspectRatio = (float)viewportDims.x / (float)viewportDims.y;

    RayDesc ray;
    ray.Origin = invView[3].xyz;
    ray.Direction = normalize((d.x * invView[0].xyz * tanHalfFovY * aspectRatio) + (-d.y * invView[1].xyz * tanHalfFovY) - invView[2].xyz);
    ray.TMin = 0;
    ray.TMax = 100000;

    Payload payload;
    payload.color = float4(0.0f, 1.0f, 0.0f, 1.0f);
    TraceRay(scene, 0 /*rayFlags*/, 0xFF /*rayMask*/, 0 /*sbtRecordOffset*/, 1 /*sbtRecordStride*/, 0 /*missIndex*/, ray, payload);
    RTOutput[LaunchIndex.xy] = payload.color;
}


// CHECK: ID3D12LibraryReflection:
// CHECK:   D3D12_LIBRARY_DESC:
// CHECK:     Flags: 0
// CHECK:     FunctionCount: 1
// CHECK:   ID3D12FunctionReflection:
// CHECK:     D3D12_FUNCTION_DESC: Name: \01?RayGenTestMain{{[@$?.A-Za-z0-9_]+}}
// CHECK:       Shader Version: RayGeneration 6.3
// CHECK:       Flags: 0
// CHECK:       ConstantBuffers: 1
// CHECK:       BoundResources: 3
// CHECK:       FunctionParameterCount: 0
// CHECK:       HasReturn: FALSE
// CHECK:     Constant Buffers:
// CHECK:       ID3D12ShaderReflectionConstantBuffer:
// CHECK:         D3D12_SHADER_BUFFER_DESC: Name: $Globals
// CHECK:           Type: D3D_CT_CBUFFER
// CHECK:           Size: 80
// CHECK:           uFlags: 0
// CHECK:           Num Variables: 3
// CHECK:     Bound Resources:
// CHECK:       D3D12_SHADER_INPUT_BIND_DESC: Name: scene
// CHECK:         Type: D3D_SIT_RTACCELERATIONSTRUCTURE
// CHECK:         uID: 0
// CHECK:         BindCount: 1
// CHECK:         BindPoint: 0
// CHECK:         Space: 0
// CHECK:         ReturnType: D3D_RETURN_TYPE_SINT
// CHECK:         Dimension: D3D_SRV_DIMENSION_UNKNOWN
// CHECK:         NumSamples (or stride): 4294967295
// CHECK:         uFlags: 0
