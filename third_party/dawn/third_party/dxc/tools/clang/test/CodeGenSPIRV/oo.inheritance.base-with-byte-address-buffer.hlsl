// RUN: %dxc -T ps_6_0 -E main -fvk-use-dx-layout -fspv-reflect -fspv-target-env=vulkan1.1 -fcgl  %s -spirv | FileCheck %s

// We want to check that we correctly set "contains-alias-component" when the
// class has a parent with ByteAddressBuffer, which later affects the result of
// `loadIfAliasVarRef(..)` by letting it emit a load instruction of the pointer.
// As a result, it prevents users of `loadIfAliasVarRef(..)` from just
// referencing an access chain without a proper load.

struct Base {
  ByteAddressBuffer buffer;
};

struct Child : Base {
  float load(in uint offset) {
// CHECK:          %param_this = OpFunctionParameter %_ptr_Function_Child
// CHECK:        [[base:%[a-zA-Z0-9_]+]] = OpAccessChain %_ptr_Function_Base %param_this %uint_0
// CHECK: [[ptrToBuffer:%[a-zA-Z0-9_]+]] = OpAccessChain %_ptr_Function__ptr_Uniform_type_ByteAddressBuffer [[base]] %int_0

// This test case is mainly intended to confirm that we emit the following instruction
// CHECK: [[buffer:%[a-zA-Z0-9_]+]] = OpLoad %_ptr_Uniform_type_ByteAddressBuffer [[ptrToBuffer]]

// CHECK: OpAccessChain %_ptr_Uniform_uint [[buffer]] %uint_0
    return asfloat(buffer.Load(offset));
  }
};

void main(out float target : SV_Target) {
  Child foo;
  target = foo.load(0);
}
