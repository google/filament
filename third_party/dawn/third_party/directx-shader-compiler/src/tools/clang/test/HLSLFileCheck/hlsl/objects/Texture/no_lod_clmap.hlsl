// RUN: %dxilver 1.8 | %dxc -E test_sample -T ps_6_8 %s | FileCheck %s -check-prefixes=NRM,CHECK
// RUN: %dxilver 1.8 | %dxc -E test_sampleb -T ps_6_8 %s | FileCheck %s -check-prefixes=NRM,CHECK
// RUN: %dxilver 1.8 | %dxc -E test_sampleg -T ps_6_8 %s | FileCheck %s -check-prefixes=NRM,CHECK
// RUN: %dxilver 1.8 | %dxc -E test_samplec -T ps_6_8 %s | FileCheck %s -check-prefixes=NRM,CHECK
// RUN: %dxilver 1.8 | %dxc -E test_samplecb -T ps_6_8 %s | FileCheck %s -check-prefixes=CMPBG,CHECK
// RUN: %dxilver 1.8 | %dxc -E test_samplecg -T ps_6_8 %s | FileCheck %s -check-prefixes=CMPBG,CHECK
// RUN: %dxilver 1.8 | %dxc -E test_sample_zero -T ps_6_8 %s | FileCheck %s -check-prefixes=NRM,CHECK
// RUN: %dxilver 1.8 | %dxc -E test_sampleb_zero -T ps_6_8 %s | FileCheck %s -check-prefixes=NRM,CHECK
// RUN: %dxilver 1.8 | %dxc -E test_sampleg_zero -T ps_6_8 %s | FileCheck %s -check-prefixes=NRM,CHECK
// RUN: %dxilver 1.8 | %dxc -E test_samplec_zero -T ps_6_8 %s | FileCheck %s -check-prefixes=NRM,CHECK
// RUN: %dxilver 1.8 | %dxc -E test_samplecb_zero -T ps_6_8 %s | FileCheck %s -check-prefixes=CMPBG,CHECK
// RUN: %dxilver 1.8 | %dxc -E test_samplecg_zero -T ps_6_8 %s | FileCheck %s -check-prefixes=CMPBG,CHECK

// Make sure no tile resources when no lod clamp or clamp is 0.

// CHECK-NOT: Tiled resources
// CMPBG: SampleCmp with gradient or bias

// CHECK:define void @[[name:[a-z_]+]]()

// CHECK: !dx.entryPoints = !{![[entryPoints:[0-9]+]]}
// NRM: ![[entryPoints]] = !{void ()* @[[name]], !"[[name]]", !{{[0-9]+}}, !{{[0-9]+}}, null}
// CMPBG: ![[entryPoints]] = !{void ()* @[[name]], !"[[name]]", !{{[0-9]+}}, !{{[0-9]+}}, ![[extAttr:[0-9]+]]}

// tag 0: ShaderFlags, 4096 = Tiled resources
// NRM-NOT:!{i32 0, i64 4096}

// tag 0: ShaderFlags, 137438953472 = SampleCmpGradientOrBias
// CMPBG: ![[extAttr]] = !{i32 0, i64 137438953472}

Texture2D T2D;
SamplerState S;

float4 test_sample(float2 coord : TEXCOORD) : SV_Target {
  return T2D.Sample(S, coord);
}

float bias;

float4 test_sampleb(float2 coord : TEXCOORD) : SV_Target {
  return T2D.SampleBias(S, coord, bias);
}

SamplerComparisonState CS;
float cmp;

float4 test_samplec(float2 coord : TEXCOORD) : SV_Target {
  return T2D.SampleCmp(CS, coord, cmp);
}

float4 dd;

float4 test_sampleg(float2 coord : TEXCOORD) : SV_Target {
  // LOD clamp requires TiledResources feature
  return T2D.SampleGrad(S, coord, dd.xy, dd.zw);
}

float4 test_samplecb(float2 coord : TEXCOORD) : SV_Target {
  return T2D.SampleCmpBias(CS, coord, cmp, bias);
}

float4 test_samplecg(float2 coord : TEXCOORD) : SV_Target {
  return T2D.SampleCmpGrad(CS, coord, cmp, dd.xy, dd.zw);
}

float4 test_sample_zero(float2 coord : TEXCOORD) : SV_Target {
  // LOD clamp requires TiledResources feature
  return T2D.Sample(S, coord, int2(0,0), 0);
}

float4 test_sampleb_zero(float2 coord : TEXCOORD) : SV_Target {
  return T2D.SampleBias(S, coord, bias, int2(0,0), 0);
}

float4 test_samplec_zero(float2 coord : TEXCOORD) : SV_Target {
  return T2D.SampleCmp(CS, coord, cmp, int2(0,0), 0);
}


float4 test_sampleg_zero(float2 coord : TEXCOORD) : SV_Target {
  return T2D.SampleGrad(S, coord, dd.xy, dd.zw, int2(0,0), 0);
}

float4 test_samplecb_zero(float2 coord : TEXCOORD) : SV_Target {
  return T2D.SampleCmpBias(CS, coord, cmp, bias, int2(0,0), 0);
}

float4 test_samplecg_zero(float2 coord : TEXCOORD) : SV_Target {
  return T2D.SampleCmpGrad(CS, coord, cmp, dd.xy, dd.zw, int2(0,0), 0);
}
