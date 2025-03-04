// RUN: %dxc -spirv -Zi -O0 -T lib_6_5 -D HLSL6 -fspv-target-env=vulkan1.2 -fspv-debug=vulkan-with-source -fvk-use-gl-layout %s | FileCheck %s

// CHECK: [[set:%[0-9]+]] = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
// CHECK: [[AS:%[0-9]+]] = OpString "@accelerationStructureNV"
// CHECK: [[US:%[0-9]+]] = OpString "accelerationStructureNV"
// CHECK: [[DI:%[0-9]+]] = OpExtInst %void [[set]] DebugInfoNone
// CHECK: [[DS:%[0-9]+]] = OpExtInst %void [[set]] DebugSource
// CHECK: [[CU:%[0-9]+]] = OpExtInst %void [[set]] DebugCompilationUnit

// CHECK: {{%[0-9]+}} = OpExtInst %void [[set]] DebugTypeComposite [[AS]] %uint_1 [[DS]] %uint_0 %uint_0 [[CU]] [[US]] [[DI]] %uint_3
struct hitPayload
{
  float3 hitValue;
  uint seed;
};

RaytracingAccelerationStructure topLevelAS; 
RWTexture2D<float4> image;

[shader("raygeneration")]
void main()
{
  const float2 pixelCenter = float2(DispatchRaysIndex().xy) + float2(0.5, 0.5);
  const float2 inUV        = pixelCenter / float2(DispatchRaysDimensions().xy);
  float2       d           = inUV * 2.0 - 1.0;

  float4 origin    = float4(0,0,0,1);
  float4 target    = float4(d.x, d.y, 1, 1);
  float4 direction = float4(normalize(target.xyz), 0)   ;

  uint  rayFlags = RAY_FLAG_FORCE_OPAQUE;
  float tMin     = 0.001;
  float tMax     = 10000.0;

  RayDesc desc;
  desc.Origin = origin.xyz;
  desc.Direction = direction.xyz;
  desc.TMin = tMin;
  desc.TMax = tMax;

  hitPayload prd;

  TraceRay(topLevelAS,    
          rayFlags,       
          0xFF,           
          0,              
          0,             
          0,              
	  desc,
          prd
  );

  image[DispatchRaysIndex().xy] = float4(prd.hitValue, 1.0);
}