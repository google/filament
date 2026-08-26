// RUN: %dxc -T cs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct R {
  int a;
  void incr() { ++a; }
};

// CHECK: %rwsb = OpVariable %_ptr_Uniform_type_RWStructuredBuffer_R Uniform
RWStructuredBuffer<R> rwsb;

struct S {
  int a;
  void incr() { ++a; }
};

// CHECK: %gs = OpVariable %_ptr_Workgroup_S Workgroup
groupshared S gs;

// CHECK: %st = OpVariable %_ptr_Private_S Private
static S st;

void decr(inout R foo) {
  foo.a--;
};

void decr2(inout S foo) {
  foo.a--;
};

void int_decr(out int foo) {
  ++foo;
}

// CHECK: %gsarr = OpVariable %_ptr_Workgroup__arr_S_uint_10 Workgroup
groupshared S gsarr[10];

// CHECK: %starr = OpVariable %_ptr_Private__arr_S_uint_10 Private
static S starr[10];

[numthreads(1, 1, 1)]
void main() {
// CHECK:    %fn = OpVariable %_ptr_Function_S Function
  S fn;

// CHECK: %fnarr = OpVariable %_ptr_Function__arr_S_uint_10 Function
  S fnarr[10];

// CHECK:   %arr = OpVariable %_ptr_Function__arr_int_uint_10 Function
  int arr[10];

// CHECK:      [[rwsb:%[0-9]+]] = OpAccessChain %_ptr_Uniform_R %rwsb %int_0 %uint_0
// CHECK-NEXT:      {{%[0-9]+}} = OpFunctionCall %void %R_incr [[rwsb]]
  rwsb[0].incr();

// CHECK: OpFunctionCall %void %S_incr %gs
  gs.incr();

// CHECK: OpFunctionCall %void %S_incr %st
  st.incr();

// CHECK: OpFunctionCall %void %S_incr %fn
  fn.incr();

// CHECK:      [[rwsb_0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_R %rwsb %int_0 %uint_0
// CHECK-NEXT:      {{%[0-9]+}} = OpFunctionCall %void %decr [[rwsb_0]]
  decr(rwsb[0]);

// CHECK: OpFunctionCall %void %decr2 %gs
  decr2(gs);

// CHECK: OpFunctionCall %void %decr2 %st
  decr2(st);

// CHECK: OpFunctionCall %void %decr2 %fn
  decr2(fn);

// CHECK:      [[gsarr:%[0-9]+]] = OpAccessChain %_ptr_Workgroup_S %gsarr %int_0
// CHECK-NEXT:       {{%[0-9]+}} = OpFunctionCall %void %S_incr [[gsarr]]
  gsarr[0].incr();

// CHECK:      [[starr:%[0-9]+]] = OpAccessChain %_ptr_Private_S %starr %int_0
// CHECK-NEXT:       {{%[0-9]+}} = OpFunctionCall %void %S_incr [[starr]]
  starr[0].incr();

// CHECK:      [[fnarr:%[0-9]+]] = OpAccessChain %_ptr_Function_S %fnarr %int_0
// CHECK-NEXT:       {{%[0-9]+}} = OpFunctionCall %void %S_incr [[fnarr]]
  fnarr[0].incr();

// CHECK:      [[gsarr_0:%[0-9]+]] = OpAccessChain %_ptr_Workgroup_S %gsarr %int_0
// CHECK-NEXT:       {{%[0-9]+}} = OpFunctionCall %void %decr2 [[gsarr_0]]
  decr2(gsarr[0]);

// CHECK:      [[starr_0:%[0-9]+]] = OpAccessChain %_ptr_Private_S %starr %int_0
// CHECK-NEXT:       {{%[0-9]+}} = OpFunctionCall %void %decr2 [[starr_0]]
  decr2(starr[0]);

// CHECK:      [[fnarr_0:%[0-9]+]] = OpAccessChain %_ptr_Function_S %fnarr %int_0
// CHECK-NEXT:       {{%[0-9]+}} = OpFunctionCall %void %decr2 [[fnarr_0]]
  decr2(fnarr[0]);

// CHECK:        [[arr:%[0-9]+]] = OpAccessChain %_ptr_Function_int %arr %int_0
// CHECK-NEXT: [[arr_0:%[0-9]+]] = OpLoad %int [[arr]]
// CHECK-NEXT: [[arr_1:%[0-9]+]] = OpIAdd %int [[arr_0]] %int_1
// CHECK-NEXT:                  OpStore [[arr]] [[arr_1]]
// CHECK-NEXT:       {{%[0-9]+}} = OpFunctionCall %void %int_decr [[arr]]
  int_decr(++arr[0]);
}
