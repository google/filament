// RUN: %dxc -T lib_6_3 -Qstrip_reflect %s | FileCheck %s

// CHECK: ; GlobalRootSignature grs = { <48 bytes> };
// CHECK: ; StateObjectConfig soc = { STATE_OBJECT_FLAG_ALLOW_LOCAL_DEPENDENCIES_ON_EXTERNAL_DEFINITIONS };
// CHECK: ; LocalRootSignature lrs = { <48 bytes> };
// CHECK: ; SubobjectToExportsAssociation sea = { "grs", { "a", "b", "foo", "c" }  };
// CHECK: ; SubobjectToExportsAssociation sea2 = { "grs", { }  };
// CHECK: ; SubobjectToExportsAssociation sea3 = { "grs", { }  };
// CHECK: ; RaytracingShaderConfig rsc = { MaxPayloadSizeInBytes = 128, MaxAttributeSizeInBytes = 64 };
// CHECK: ; RaytracingPipelineConfig rpc = { MaxTraceRecursionDepth = 512 };
// CHECK: ; HitGroup trHitGt = { HitGroupType = Triangle, Anyhit = "a", Closesthit = "b", Intersection = "" };
// CHECK: ; HitGroup ppHitGt = { HitGroupType = ProceduralPrimitive, Anyhit = "a", Closesthit = "b", Intersection = "c" };

GlobalRootSignature grs = {"CBV(b0)"};
StateObjectConfig soc = { STATE_OBJECT_FLAGS_ALLOW_LOCAL_DEPENDENCIES_ON_EXTERNAL_DEFINITONS };
LocalRootSignature lrs = {"UAV(u0, visibility = SHADER_VISIBILITY_GEOMETRY), RootFlags(LOCAL_ROOT_SIGNATURE)"};
SubobjectToExportsAssociation sea = { "grs", "a;b;foo;c" };
// Empty association is well-defined: it creates a default association
SubobjectToExportsAssociation sea2 = { "grs", ";" };
SubobjectToExportsAssociation sea3 = { "grs", "" };
RaytracingShaderConfig rsc = { 128, 64 };
RaytracingPipelineConfig rpc = { 512 };
TriangleHitGroup trHitGt = { "a", "b" };
ProceduralPrimitiveHitGroup ppHitGt = { "a", "b", "c"};

int main(int i : INDEX) : SV_Target {
  return 1;
}
