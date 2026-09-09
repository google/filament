// RUN: %dxc -T vs_5_0 -E main -fspv-target-env=vulkan1.1 -fcgl  %s -spirv | FileCheck %s

// CHECK: %bar = OpTypeStruct %empty %mat4v4float
// CHECK: %foo = OpTypeStruct %bar

// CHECK: OpFunctionCall %v4float %foo_get %x

struct empty {
};

struct bar : empty {
  float4x4 trans;
  float4 value() {
    return 0;
  }
};

struct foo : bar {
  float4 value() {
    return 1;
  }

// When foo calls foo::value(), it must use
// this->value() instead of (this's 0th object)->value()

// CHECK:     %foo_get = OpFunction
// CHECK:  %param_this = OpFunctionParameter %_ptr_Function_foo
// CHECK:                OpFunctionCall %v4float %foo_value %param_this

  float4 get() {
    return value();
  }
};

void main(out float4 Position : SV_Position) {
  foo x;
  Position = x.get();
}
