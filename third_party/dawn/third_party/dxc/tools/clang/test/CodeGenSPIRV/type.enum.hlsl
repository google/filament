// RUN: %dxc -T cs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

//CHECK:      %First = OpVariable %_ptr_Private_int Private %int_0
//CHECK-NEXT: %Second = OpVariable %_ptr_Private_int Private %int_1
//CHECK-NEXT: %Third = OpVariable %_ptr_Private_int Private %int_3
//CHECK-NEXT: %Fourth = OpVariable %_ptr_Private_int Private %int_n1
enum Number {
  First,
  Second,
  Third = 3,
  Fourth = -1,
};

//CHECK:      %a = OpVariable %_ptr_Private_int Private
//CHECK: %b = OpVariable %_ptr_Workgroup_int Workgroup
//CHECK: %c = OpVariable %_ptr_Uniform_type_AppendStructuredBuffer_ Uniform

//CHECK:      [[second:%[0-9]+]] = OpLoad %int %Second
//CHECK-NEXT:                   OpStore %a [[second]]
static ::Number a = Second;
groupshared Number b;
AppendStructuredBuffer<Number> c;

void testParam(Number param) {}
void testParamTypeCast(int param) {}

[numthreads(1, 1, 1)]
void main() {
//CHECK:      [[a:%[0-9]+]] = OpLoad %int %a
//CHECK-NEXT:              OpStore %foo [[a]]
  int foo = a;

//CHECK:      [[fourth:%[0-9]+]] = OpLoad %int %Fourth
//CHECK-NEXT:                   OpStore %b [[fourth]]
  b = Fourth;

//CHECK:          [[c:%[0-9]+]] = OpAccessChain %_ptr_Uniform_int %c %uint_0
//CHECK-NEXT: [[third:%[0-9]+]] = OpLoad %int %Third
//CHECK-NEXT:                  OpStore [[c]] [[third]]
  c.Append(Third);

//CHECK:          [[c_0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_int %c %uint_0 %57
//CHECK-NEXT: [[third_0:%[0-9]+]] = OpLoad %int %Third
//CHECK-NEXT:                  OpStore [[c_0]] [[third_0]]
  c.Append(Number::Third);

  Number d;
//CHECK:      [[d:%[0-9]+]] = OpLoad %int %d
//CHECK-NEXT:              OpSelectionMerge %switch_merge None
//CHECK-NEXT:              OpSwitch [[d]] %switch_default 0 %switch_0 1 %switch_1
  switch (d) {
    case First:
      d = Second;
      break;
    case Second:
      d = First;
      break;
    default:
      d = Third;
      break;
  }

//CHECK:      [[fourth_0:%[0-9]+]] = OpLoad %int %Fourth
//CHECK-NEXT:                   OpStore %e [[fourth_0]]
  static ::Number e = Fourth;

//CHECK:          [[d_0:%[0-9]+]] = OpLoad %int %d
//CHECK-NEXT: [[third_1:%[0-9]+]] = OpLoad %int %Third
//CHECK-NEXT:                  OpSLessThan %bool [[d_0]] [[third_1]]
  if (d < Third) {
//CHECK:       [[first:%[0-9]+]] = OpLoad %int %First
//CHECK-NEXT: [[second_0:%[0-9]+]] = OpLoad %int %Second
//CHECK-NEXT:    [[add:%[0-9]+]] = OpIAdd %int [[first]] [[second_0]]
//CHECK-NEXT:                   OpStore %d [[add]]
    d = First + Second;
  }

//CHECK:      [[foo:%[0-9]+]] = OpLoad %int %foo
//CHECK-NEXT: [[foo_0:%[0-9]+]] = OpBitcast %int [[foo]]
//CHECK-NEXT:                OpStore %d [[foo_0]]
  if (First < Third)
    d = (Number)foo;

//CHECK:      [[a_0:%[0-9]+]] = OpLoad %int %a
//CHECK-NEXT:              OpStore %param_var_param [[a_0]]
//CHECK-NEXT:              OpFunctionCall %void %testParam %param_var_param
  testParam(a);

//CHECK:      [[second_1:%[0-9]+]] = OpLoad %int %Second
//CHECK-NEXT:                   OpStore %param_var_param_0 [[second_1]]
//CHECK-NEXT:                   OpFunctionCall %void %testParam %param_var_param_0
  testParam(Second);

//CHECK:      [[a_1:%[0-9]+]] = OpLoad %int %a
//CHECK-NEXT:              OpStore %param_var_param_1 [[a_1]]
//CHECK-NEXT:              OpFunctionCall %void %testParamTypeCast %param_var_param_1
  testParamTypeCast(a);

//CHECK:      OpStore %param_var_param_2 %int_1
//CHECK-NEXT: OpFunctionCall %void %testParamTypeCast %param_var_param_2
  testParamTypeCast(Second);

//CHECK:        [[a_2:%[0-9]+]] = OpLoad %int %a
//CHECK-NEXT:   [[a_3:%[0-9]+]] = OpConvertSToF %float [[a_2]]
//CHECK-NEXT: [[sin:%[0-9]+]] = OpExtInst %float {{%[0-9]+}} Sin [[a_3]]
//CHECK-NEXT:                OpStore %bar [[sin]]
  float bar = sin(a);
}
