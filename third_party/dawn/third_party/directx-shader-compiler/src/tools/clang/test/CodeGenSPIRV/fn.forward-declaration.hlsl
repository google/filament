// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct foo
{
  float memberFunction() {
    float MulBy3(float f);
    return MulBy3(.25);
  }
};

float4 main() : SV_Target
{
  foo f;
  float a = f.memberFunction();

  float MulBy2(float f);

  // CHECK: OpFunctionCall %float %MulBy2 %param_var_f
  return float4(MulBy2(.25), 1, 0, 1);
}

// Definitions of foo::memberFunction
// CHECK: %foo_memberFunction = OpFunction %float None
// CHECK: OpFunctionCall %float %MulBy3 %param_var_f_0
// CHECK: OpFunctionEnd

// CHECK: %MulBy2 = OpFunction %float None
float MulBy2(float f) {
  return f*2;
}

// CHECK: %MulBy3 = OpFunction %float None
float MulBy3(float f) {
  return f*3;
}

