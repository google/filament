// RUN: %dxc -T lib_6_3 -auto-binding-space 11 -default-linkage external %s | %D3DReflect %s | FileCheck %s

struct Payload {
   float2 t;
   int3 t2;
};

RaytracingAccelerationStructure Acc;

uint RayFlags;
uint InstanceInclusionMask;
uint RayContributionToHitGroupIndex;
uint MultiplierForGeometryContributionToHitGroupIndex;
uint MissShaderIndex;


float4 emit(inout float2 f2, RayDesc Ray:R, inout Payload p )  {
  TraceRay(Acc,RayFlags,InstanceInclusionMask,
           RayContributionToHitGroupIndex,
           MultiplierForGeometryContributionToHitGroupIndex,MissShaderIndex, Ray, p);

   return 2.6;
}



// CHECK: ID3D12LibraryReflection:
// CHECK:   D3D12_LIBRARY_DESC:
// CHECK:     Flags: 0
// CHECK:     FunctionCount: 1
// CHECK:   ID3D12FunctionReflection:
// CHECK:     D3D12_FUNCTION_DESC: Name: \01?emit{{[@$?.A-Za-z0-9_]+}}
// CHECK:       Shader Version: Library 6.3
// CHECK:       Flags: 0
// CHECK:       ConstantBuffers: 1
// CHECK:       BoundResources: 2
// CHECK:       FunctionParameterCount: 0
// CHECK:       HasReturn: FALSE
// CHECK:     Bound Resources:
// CHECK:       D3D12_SHADER_INPUT_BIND_DESC: Name: Acc
// CHECK:         Type: D3D_SIT_RTACCELERATIONSTRUCTURE
// CHECK:         uID: 0
// CHECK:         BindCount: 1
// CHECK:         BindPoint: 0
// CHECK:         Space: 11
// CHECK:         ReturnType: D3D_RETURN_TYPE_SINT
// CHECK:         Dimension: D3D_SRV_DIMENSION_UNKNOWN
// CHECK:         NumSamples (or stride): 4294967295
// CHECK:         uFlags: 0
