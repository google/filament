// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK-NOT: OpName %fnNoCaller "fnNoCaller"

// CHECK: [[voidf:%[0-9]+]] = OpTypeFunction %void
// CHECK: [[intfint:%[0-9]+]] = OpTypeFunction %int %_ptr_Function_int
// CHECK: [[intfintint:%[0-9]+]] = OpTypeFunction %int %_ptr_Function_int %_ptr_Function_int

// CHECK-NOT: %fnNoCaller = OpFunction
void fnNoCaller() {}

int fnOneParm(int v) { return v; }

int fnTwoParm(int m, int n) { return m + n; }

int fnCallOthers(int v) { return fnOneParm(v); }

// Recursive functions are disallowed by the front end.

// CHECK: %main = OpFunction %void None [[voidf]]
void main() {
// CHECK-LABEL: %bb_entry = OpLabel
// CHECK-NEXT: %v = OpVariable %_ptr_Function_int Function
    int v;
// CHECK-NEXT: [[oneParam:%[a-zA-Z0-9_]+]] = OpVariable %_ptr_Function_int Function
// CHECK-NEXT: [[twoParam1:%[a-zA-Z0-9_]+]] = OpVariable %_ptr_Function_int Function
// CHECK-NEXT: [[twoParam2:%[a-zA-Z0-9_]+]] = OpVariable %_ptr_Function_int Function
// CHECK-NEXT: [[nestedParam1:%[a-zA-Z0-9_]+]] = OpVariable %_ptr_Function_int Function
// CHECK-NEXT: [[nestedParam2:%[a-zA-Z0-9_]+]] = OpVariable %_ptr_Function_int Function

// CHECK-NEXT: OpStore [[oneParam]] %int_1
// CHECK-NEXT: [[call0:%[0-9]+]] = OpFunctionCall %int %fnOneParm [[oneParam]]
// CHECK-NEXT: OpStore %v [[call0]]
    v = fnOneParm(1); // Pass in constant; use return value

// CHECK-NEXT: [[v0:%[0-9]+]] = OpLoad %int %v
// CHECK-NEXT: OpStore [[twoParam1]] [[v0]]
// CHECK-NEXT: [[v1:%[0-9]+]] = OpLoad %int %v
// CHECK-NEXT: OpStore [[twoParam2]] [[v1]]
// CHECK-NEXT: [[call2:%[0-9]+]] = OpFunctionCall %int %fnTwoParm [[twoParam1]] [[twoParam2]]
    fnTwoParm(v, v);  // Pass in variable; ignore return value

// CHECK-NEXT: OpStore [[nestedParam1]] %int_1
// CHECK-NEXT: [[call3:%[0-9]+]] = OpFunctionCall %int %fnOneParm [[nestedParam1]]
// CHECK-NEXT: OpStore [[nestedParam2]] [[call3]]
// CHECK-NEXT: [[call4:%[0-9]+]] = OpFunctionCall %int %fnCallOthers [[nestedParam2]]
// CHECK-NEXT: OpReturn
// CHECK-NEXT: OpFunctionEnd
    fnCallOthers(fnOneParm(1)); // Nested function calls
}

// CHECK-NOT: %fnNoCaller = OpFunction

/* For int fnOneParm(int v) { return v; } */

// %fnOneParm = OpFunction %int None [[intfint]]
// %v_0 = OpFunctionParameter %_ptr_Function_int
// %bb_entry_0 = OpLabel
// [[v2:%[0-9]+]] = OpLoad %int %v_0
// OpReturnValue [[v2]]
// OpFunctionEnd


// CHECK-NOT: %fnNoCaller = OpFunction

/* For int fnTwoParm(int m, int n) { return m + n; } */

// %fnTwoParm = OpFunction %int None %27
// %m = OpFunctionParameter %_ptr_Function_int
// %n = OpFunctionParameter %_ptr_Function_int
// %bb_entry_1 = OpLabel
// [[m0:%[0-9]+]] = OpLoad %int %m
// [[n0:%[0-9]+]] = OpLoad %int %n
// [[add0:%[0-9]+]] = OpIAdd %int [[m0]] [[n0]]
// OpReturnValue [[add0]]
// OpFunctionEnd

// CHECK-NOT: %fnNoCaller = OpFunction

/* For int fnCallOthers(int v) { return fnOneParm(v); } */

// %fnCallOthers = OpFunction %int None [[intfintint]]
// %v_1 = OpFunctionParameter %_ptr_Function_int
// %bb_entry_2 = OpLabel
// [[param0:%[0-9]+]] = OpVariable %_ptr_Function_int Function
// [[v3:%[0-9]+]] = OpLoad %int %v_1
// OpStore [[param0]] [[v3]]
// [[call5:%[0-9]+]] = OpFunctionCall %int %fnOneParm [[param0]]
// OpReturnValue [[call5]]
// OpFunctionEnd

// CHECK-NOT: %fnNoCaller = OpFunction
