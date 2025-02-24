// RUN: %dxc -T vs_5_0 -E main -fspv-target-env=vulkan1.1 -fcgl %s -spirv | FileCheck %s

// CHECK: %bar = OpTypeStruct %empty %mat4v4float
// CHECK: %foo = OpTypeStruct %bar

// CHECK: OpFunctionCall %v4float %foo_get %x

struct empty {
};

struct bar : empty {
  float4x4 trans;
  float4 value() {
    return mul(float4(1,1,1,0), trans);
  }
};

struct foo : bar {

// When foo calls bar::value(), it must use
// (this's 0th object)->value() instead of this->value()
// because this's 0th object is the object of the base struct in SPIR-V.

// CHECK:     %foo_get = OpFunction
// CHECK:  %param_this = OpFunctionParameter %_ptr_Function_foo
// CHECK: [[bar:%[a-zA-Z0-9_]+]] = OpAccessChain %_ptr_Function_bar %param_this %uint_0
// CHECK:                OpFunctionCall %v4float %bar_value [[bar]]

  float4 get() {
    return value();
  }
};

void main(out float4 Position : SV_Position)
{
  foo x;
  Position = x.get();
}
