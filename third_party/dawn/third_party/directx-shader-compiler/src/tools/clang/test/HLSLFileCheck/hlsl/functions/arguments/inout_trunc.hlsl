// RUN: %dxc -E NocrashMain -T ps_6_0 %s | FileCheck %s -check-prefix=CHK_NOCRASH
// RUN: %dxc -E WarnMain -T ps_6_0 %s | FileCheck %s -check-prefix=CHK_WARN

// Test that no crashes result when a scalar is provided to an outvar
// and that the new warning is produced.

// CHK_WARN: warning: implicit truncation of vector type
// CHK_WARN: warning: implicit truncation of vector type
// CHK_WARN: warning: implicit truncation of vector type
// CHK_WARN: warning: implicit truncation of vector type
// CHK_WARN: warning: implicit truncation of vector type
// CHK_WARN: warning: implicit truncation of vector type
// CHK_WARN-NOT: warning: implicit truncation of vector type
// CHK_NOCRASH: NocrashMain

float val1;
float val2;
float val3;

float2 vec2;
float3 vec3;
float4 vec4;

void TakeItOut(out float2 it) {
  it = val1;
}

void TakeItIn(inout float3 it) {
  it = val2;
}

void TakeItIn2(inout float4 it) {
  it += val3;
}

void TakeEmOut(out float2 em) {
  em = vec2;
}

void TakeEmIn(inout float3 em) {
  em = vec3;
}

void TakeEmIn2(inout float4 em) {
  em += vec4;
}


float2 RunTest(float it, float em)
{
  float c = it;
  TakeItOut(it);
  TakeItIn(it);
  TakeItIn2(it);

  TakeEmOut(em);
  TakeEmIn(em);
  TakeEmIn2(em);
  return float2(it, em);
}

float2 NocrashMain(float it: A, float em: B) : SV_Target
{
  return RunTest(it, em);
}

// Missing out semantic to force filecheck to read stderr and see the warnings.
float2 WarnMain(float it: A, float em: B)
{
  return RunTest(it, em);
}

