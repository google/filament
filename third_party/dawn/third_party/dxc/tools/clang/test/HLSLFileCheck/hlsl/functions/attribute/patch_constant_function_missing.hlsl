// RUN: %dxc -T lib_6_5 %s | FileCheck %s


// actual selected HSPerPatchFunc1 for HSMain1 and HSMain3
float4 fooey()
{
  float4 e;
  float4 d;
  d.x = 4;
  
  return e;
}

[shader("hull")]
// CHECK: error: patch constant function 'NotFooey' must be defined
[patchconstantfunc("NotFooey")]
[outputtopology("point")]
[outputcontrolpoints(1)]
float4 main(float a : A, float b:B) : SV_TARGET
{
  float4 f = b;
  return f;
}

