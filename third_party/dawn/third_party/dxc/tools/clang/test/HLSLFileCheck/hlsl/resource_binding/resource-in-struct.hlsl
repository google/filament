// RUN: %dxc -E main -T ps_6_0 %s  | FileCheck %s
// RUN: %dxc -T lib_6_x %s  | FileCheck %s -check-prefixes=CHECKTYPE,CHECKRP

// For now: No scenario here should produce the type annotation, since that
// should be unused in final DXIL, and thus eliminated.  Uncomment when a
// scenario is crafted out that should produce the annotation.
// xUN: %dxc -T lib_6_7 -validator-version 1.7 %s  | FileCheck %s -check-prefixes=CHECKTYPE,CHECKNORP
// xUN: %dxc -T lib_6_8 %s  | FileCheck %s -check-prefixes=CHECKTYPE,CHECKRP

// CHECK: res.Tex1

// Make sure ResourceProperties are emitted.
// CHECKTYPE: !dx.typeAnnotations = !{![[TyAnn:[0-9]+]]
// CHECKTYPE: ![[TyAnn]] =
// CHECKTYPE-SAME: %struct.Resources undef, ![[Resources_Ann:[0-9]+]],
// CHECKTYPE: ![[Resources_Ann]] = !{i32 16, ![[Tex1_Ann:[0-9]+]],
// CHECKRP: ![[Tex1_Ann]] = !{i32 6, !"Tex1",
// CHECKRP-SAME: i32 10, %dx.types.ResourceProperties { i32 2, i32 1033 }
// CHECKNORP: ![[Tex1_Ann]] = !{i32 6, !"Tex1",
// CHECKNORP-NOT: %dx.types.ResourceProperties

SamplerState Samp;
struct Resources
{
  Texture2D Tex1;
  // Texture3D Tex2;
  // RWTexture2D<float4> RWTex1;
  // RWTexture3D<float4> RWTex2;
  // SamplerState Samp;
  float4 foo;
};

Resources g_res;

float4 foo(Resources res, float4 coord) {
  return res.Tex1.Sample(Samp, coord.xy) * res.foo;
}

[shader("pixel")]
float4 main(int4 a : A, float4 coord : TEXCOORD) : SV_TARGET
{
  return foo(g_res, coord);
}
