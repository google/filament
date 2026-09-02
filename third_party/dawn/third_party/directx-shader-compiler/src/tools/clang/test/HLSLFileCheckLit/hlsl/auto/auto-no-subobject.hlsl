// RUN: %dxc -T lib_6_3 -HV 202x -verify %s

GlobalRootSignature grs = {"CBV(b0)"};
LocalRootSignature lrs = {"UAV(u0)"};
StateObjectConfig soc = { STATE_OBJECT_FLAGS_ALLOW_LOCAL_DEPENDENCIES_ON_EXTERNAL_DEFINITONS };
SubobjectToExportsAssociation sea = { "grs", "a;b" };
RaytracingShaderConfig rsc = { 128, 64 };
RaytracingPipelineConfig rpc = { 512 };
RaytracingPipelineConfig1 rpc1 = { 32, RAYTRACING_PIPELINE_FLAG_ALLOW_OPACITY_MICROMAPS };
TriangleHitGroup trHitGt = { "a", "b" };
ProceduralPrimitiveHitGroup ppHitGt = { "a", "b", "c" };

void useSubobjects() {
    // expected-error@+1 {{'auto' cannot deduce type 'GlobalRootSignature'}}
    auto a = grs;
    // expected-error@+1 {{'auto' cannot deduce type 'LocalRootSignature'}}
    auto b = lrs;
    // expected-error@+1 {{'auto' cannot deduce type 'StateObjectConfig'}}
    auto c = soc;
    // expected-error@+1 {{'auto' cannot deduce type 'SubobjectToExportsAssociation'}}
    auto d = sea;
    // expected-error@+1 {{'auto' cannot deduce type 'RaytracingShaderConfig'}}
    auto e = rsc;
    // expected-error@+1 {{'auto' cannot deduce type 'RaytracingPipelineConfig'}}
    auto f = rpc;
    // expected-error@+1 {{'auto' cannot deduce type 'RaytracingPipelineConfig1'}}
    auto g = rpc1;
    // expected-error@+1 {{'auto' cannot deduce type 'TriangleHitGroup'}}
    auto h = trHitGt;
    // expected-error@+1 {{'auto' cannot deduce type 'ProceduralPrimitiveHitGroup'}}
    auto i = ppHitGt;
}
