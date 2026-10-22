// RUN: %dxc -E MainPS -T ps_6_6 %s | FileCheck %s
// RUN: %dxc -E MainCS -T cs_6_6 %s | FileCheck %s
// RUN: %dxc -E MainAS -T as_6_6 %s | FileCheck %s
// RUN: %dxc -E MainMS -T ms_6_6 %s | FileCheck %s
// RUN: %dxc -T lib_6_6 %s | FileCheck %s

// Make sure add is not sunk into if.
// Compute shader variant of convergent.hlsl

// CHECK: fadd
// CHECK: fadd
// CHECK: fcmp
// CHECK-NEXT: br


Texture2D<float4> tex;
RWBuffer<float4> output;
SamplerState s;

float4 doit(float2 a, float b){

  float2 coord = a + b;
  float4 c = b;
  if (b > 2) {
    c += tex.Sample(s, coord);
  }
  return c;
}

[shader("compute")]
[numthreads(4,4,4)]
void MainCS(uint ix : SV_GroupIndex, uint3 id : SV_GroupThreadID) {
  output[ix] = doit(id.xy, id.z);
}

struct Payload { int nothing; };

[shader("amplification")]
[numthreads(4,4,4)]
void MainAS(uint ix : SV_GroupIndex, uint3 id : SV_GroupThreadID) {
  output[ix] = doit(id.xy, id.z);
  Payload pld = (Payload)0;
  DispatchMesh(1,1,1,pld);
}


[shader("mesh")]
[numthreads(4,1,1)]
[outputtopology("triangle")]
void MainMS(uint ix : SV_GroupIndex, uint3 id : SV_GroupThreadID) {
  output[ix] = doit(id.xy, id.z);
}

[shader("pixel")]
float4 MainPS(float2 a:A, float b:B) : SV_Target {
  return doit(a, b);
}