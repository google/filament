// RUN: %dxilver 1.5 | %dxc -T lib_6_3 -validator-version 1.5 %s | FileCheck %s -check-prefixes=CHECK,CHECK15
// RUN: %dxilver 1.8 | %dxc -T lib_6_3 -validator-version 1.8 %s | FileCheck %s -check-prefixes=CHECK,CHECK18

// RaytracingTier1_1 flag should not be set on the module based on subobjects.
// CHECK18-NOT: Raytracing tier 1.1 features
// CHECK15: Raytracing tier 1.1 features

// CHECK: ; GlobalRootSignature grs = { <48 bytes> };
// CHECK: ; StateObjectConfig soc = { STATE_OBJECT_FLAG_ALLOW_LOCAL_DEPENDENCIES_ON_EXTERNAL_DEFINITIONS | STATE_OBJECT_FLAG_ALLOW_STATE_OBJECT_ADDITIONS };
// CHECK: ; LocalRootSignature lrs = { <48 bytes> };
// CHECK: ; SubobjectToExportsAssociation sea = { "grs", { "a", "b", "foo", "c" }  };
// CHECK: ; SubobjectToExportsAssociation sea2 = { "grs", { }  };
// CHECK: ; SubobjectToExportsAssociation sea3 = { "grs", { }  };
// CHECK: ; RaytracingShaderConfig rsc = { MaxPayloadSizeInBytes = 128, MaxAttributeSizeInBytes = 64 };
// CHECK: ; RaytracingPipelineConfig1 rpc = { MaxTraceRecursionDepth = 32, Flags = RAYTRACING_PIPELINE_FLAG_SKIP_TRIANGLES };
// CHECK: ; SubobjectToExportsAssociation sea4 = { "rpc", { }  };
// CHECK: ; RaytracingPipelineConfig1 rpc2 = { MaxTraceRecursionDepth = 32, Flags = 0 };
// CHECK: ; HitGroup trHitGt = { HitGroupType = Triangle, Anyhit = "a", Closesthit = "b", Intersection = "" };
// CHECK: ; HitGroup ppHitGt = { HitGroupType = ProceduralPrimitive, Anyhit = "a", Closesthit = "b", Intersection = "c" };

GlobalRootSignature grs = {"CBV(b0)"};
StateObjectConfig soc = { STATE_OBJECT_FLAGS_ALLOW_LOCAL_DEPENDENCIES_ON_EXTERNAL_DEFINITONS | STATE_OBJECT_FLAG_ALLOW_STATE_OBJECT_ADDITIONS };
LocalRootSignature lrs = {"UAV(u0, visibility = SHADER_VISIBILITY_GEOMETRY), RootFlags(LOCAL_ROOT_SIGNATURE)"};
SubobjectToExportsAssociation sea = { "grs", "a;b;foo;c" };
// Empty association is well-defined: it creates a default association
SubobjectToExportsAssociation sea2 = { "grs", ";" };
SubobjectToExportsAssociation sea3 = { "grs", "" };
RaytracingShaderConfig rsc = { 128, 64 };
RaytracingPipelineConfig1 rpc = { 32, RAYTRACING_PIPELINE_FLAG_SKIP_TRIANGLES };
SubobjectToExportsAssociation sea4 = {"rpc", ";"};
RaytracingPipelineConfig1 rpc2 = {32, RAYTRACING_PIPELINE_FLAG_NONE };
TriangleHitGroup trHitGt = {"a", "b"};
ProceduralPrimitiveHitGroup ppHitGt = { "a", "b", "c"};

// DXR entry point to ensure RDAT flags match during validation.
[shader("raygeneration")]
void main(void) {
}
