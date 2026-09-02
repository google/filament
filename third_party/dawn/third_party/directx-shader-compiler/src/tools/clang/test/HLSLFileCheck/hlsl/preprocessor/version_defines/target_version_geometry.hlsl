// RUN: %dxc -O0 -HV 2017 -T gs_6_1 %s | FileCheck %s
// CHECK: fadd
// CHECK: fadd
// CHECK: fadd
// CHECK: fadd

struct Out
{
  float4 pos : SV_Position;
};

cbuffer cb : register(b0) {
  float foo;
};

[maxvertexcount(3)]
void main(inout PointStream<Out> OutputStream0)
{
  Out output = (Out)0;
  float x = foo;

  // Version should be as specified by -HV
#if defined(__HLSL_VERSION) && __HLSL_VERSION == 2017
  x += 1;
#else
  x -= 1;
#endif
#if defined(__SHADER_TARGET_STAGE) && __SHADER_TARGET_STAGE == __SHADER_STAGE_GEOMETRY
  x += 1;
#else
  x -= 1;
#endif
#if defined(__SHADER_TARGET_MAJOR) && __SHADER_TARGET_MAJOR == 6
  x += 1;
#else
  x -= 1;
#endif
#if defined(__SHADER_TARGET_MINOR) && __SHADER_TARGET_MINOR == 1
  x += 1;
#else
  x -= 1;
#endif

  output.pos.x = x;

  OutputStream0.Append(output);
  OutputStream0.RestartStrip();
}
