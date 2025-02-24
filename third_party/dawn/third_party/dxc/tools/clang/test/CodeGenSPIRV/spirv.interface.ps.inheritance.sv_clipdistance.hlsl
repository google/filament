// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct Parent {
  float clipDistance : SV_ClipDistance;
};

struct PSInput : Parent
{ };

float main(PSInput input) : SV_TARGET
{
// CHECK:  [[ptr0:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_ClipDistance %uint_0
// CHECK: [[load0:%[0-9]+]] = OpLoad %float [[ptr0]]

// CHECK: [[parent:%[0-9]+]] = OpCompositeConstruct %Parent [[load0]]
// CHECK:  [[input:%[0-9]+]] = OpCompositeConstruct %PSInput [[parent]]


// CHECK: [[access0:%[0-9]+]] = OpAccessChain %_ptr_Function_Parent %input %uint_0
// CHECK: [[access1:%[0-9]+]] = OpAccessChain %_ptr_Function_float [[access0]] %int_0
// CHECK:   [[load1:%[0-9]+]] = OpLoad %float [[access1]]
    return input.clipDistance;
}

