// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct S {
  ByteAddressBuffer foo;
  uint bar;

  void AccessMember() {
//CHECK:      [[foo:%[0-9]+]] = OpAccessChain %_ptr_Function__ptr_Uniform_type_ByteAddressBuffer %param_this %int_0
//CHECK-NEXT:                OpLoad %_ptr_Uniform_type_ByteAddressBuffer [[foo]]
    foo.Load(0);
  }
};

struct T {
  S first;
  uint second;

  void AccessMemberRecursive() {
//CHECK:      [[foo_0:%[0-9]+]] = OpAccessChain %_ptr_Function__ptr_Uniform_type_ByteAddressBuffer %param_this_0 %int_0 %int_0
//CHECK-NEXT:                OpLoad %_ptr_Uniform_type_ByteAddressBuffer [[foo_0]]
    first.foo.Load(0);
  }
};

void AccessParam(S input) {
//CHECK:      [[foo_1:%[0-9]+]] = OpAccessChain %_ptr_Function__ptr_Uniform_type_ByteAddressBuffer %input %int_0
//CHECK-NEXT:                OpLoad %_ptr_Uniform_type_ByteAddressBuffer [[foo_1]]
  input.foo.Load(0);
}

void main() {
  S a;
  T b;

  a.AccessMember();
  b.AccessMemberRecursive();
  AccessParam(a);
}
