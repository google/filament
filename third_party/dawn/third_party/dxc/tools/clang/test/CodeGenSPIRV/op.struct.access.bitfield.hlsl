// RUN: %dxc -T ps_6_6 -E main -HV 2021 -fcgl  %s -spirv | FileCheck %s

struct S1 {
  uint f1 : 1;
  uint f2 : 8;
};

struct S2 {
  int f1 : 2;
  int f2 : 9;
};

struct S3 {
  int f1;
  int f2 : 1;
  int f3;
};

// CHECK: OpMemberName %S1 0 "f1"
// CHECK-NOT: OpMemberName %S1 1 "f2"
// CHECK: OpMemberName %S2 0 "f1"
// CHECK-NOT: OpMemberName %S2 1 "f2"

// CHECK: %S1 = OpTypeStruct %uint
// CHECK: %S2 = OpTypeStruct %int

void main() {
  // CHECK: [[s1_var:%[a-zA-Z0-9_]+]] = OpVariable %_ptr_Function_S1 Function
  // CHECK: [[s2_var:%[a-zA-Z0-9_]+]] = OpVariable %_ptr_Function_S2 Function
  // CHECK: [[s3_var:%[a-zA-Z0-9_]+]] = OpVariable %_ptr_Function_S3 Function
  S1 s1;
  S2 s2;
  S3 s3;

  // CHECK: [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_uint [[s1_var]] %int_0
  // CHECK: [[load:%[0-9]+]] = OpLoad %uint [[ptr]]
  // CHECK: [[insert:%[0-9]+]] = OpBitFieldInsert %uint [[load]] %uint_1 %uint_0 %uint_1
  // CHECK: OpStore [[ptr]] [[insert]]
  s1.f1 = 1;

  // CHECK: [[ptr_0:%[0-9]+]] = OpAccessChain %_ptr_Function_uint [[s1_var]] %int_0
  // CHECK: [[load_0:%[0-9]+]] = OpLoad %uint [[ptr_0]]
  // CHECK: [[insert_0:%[0-9]+]] = OpBitFieldInsert %uint [[load_0]] %uint_2 %uint_1 %uint_8
  // CHECK: OpStore [[ptr_0]] [[insert_0]]
  s1.f2 = 2;

  // CHECK: [[ptr_1:%[0-9]+]] = OpAccessChain %_ptr_Function_int [[s2_var]] %int_0
  // CHECK: [[load_1:%[0-9]+]] = OpLoad %int [[ptr_1]]
  // CHECK: [[insert_1:%[0-9]+]] = OpBitFieldInsert %int [[load_1]] %int_3 %uint_0 %uint_2
  // CHECK: OpStore [[ptr_1]] [[insert_1]]
  s2.f1 = 3;

  // CHECK: [[ptr_2:%[0-9]+]] = OpAccessChain %_ptr_Function_int [[s2_var]] %int_0
  // CHECK: [[load_2:%[0-9]+]] = OpLoad %int [[ptr_2]]
  // CHECK: [[insert_2:%[0-9]+]] = OpBitFieldInsert %int [[load_2]] %int_4 %uint_2 %uint_9
  // CHECK: OpStore [[ptr_2]] [[insert_2]]
  s2.f2 = 4;

  // CHECK: [[ptr_3:%[0-9]+]] = OpAccessChain %_ptr_Function_uint [[s1_var]] %int_0
  // CHECK: [[load_3:%[0-9]+]] = OpLoad %uint [[ptr_3]]
  // CHECK: [[extract:%[0-9]+]] = OpBitFieldUExtract %uint [[load_3]] %uint_0 %uint_1
  // CHECK: OpStore %t1 [[extract]]
  uint t1 = s1.f1;

  // CHECK: [[ptr_4:%[0-9]+]] = OpAccessChain %_ptr_Function_uint [[s1_var]] %int_0
  // CHECK: [[load_4:%[0-9]+]] = OpLoad %uint [[ptr_4]]
  // CHECK: [[extract_0:%[0-9]+]] = OpBitFieldUExtract %uint [[load_4]] %uint_1 %uint_8
  // CHECK: OpStore %t2 [[extract_0]]
  uint t2 = s1.f2;

  // CHECK: [[ptr_5:%[0-9]+]] = OpAccessChain %_ptr_Function_int [[s2_var]] %int_0
  // CHECK: [[load_5:%[0-9]+]] = OpLoad %int [[ptr_5]]
  // CHECK: [[extract_1:%[0-9]+]] = OpBitFieldSExtract %int [[load_5]] %uint_0 %uint_2
  // CHECK: OpStore %t3 [[extract_1]]
  int t3 = s2.f1;

  // CHECK: [[ptr_6:%[0-9]+]] = OpAccessChain %_ptr_Function_int [[s2_var]] %int_0
  // CHECK: [[load_6:%[0-9]+]] = OpLoad %int [[ptr_6]]
  // CHECK: [[extract_2:%[0-9]+]] = OpBitFieldSExtract %int [[load_6]] %uint_2 %uint_9
  // CHECK: OpStore %t4 [[extract_2]]
  int t4 = s2.f2;

  // CHECK: [[ptr_7:%[0-9]+]] = OpAccessChain %_ptr_Function_int [[s2_var]] %int_0
  // CHECK: [[load_7:%[0-9]+]] = OpLoad %int [[ptr_7]]
  // CHECK: [[extract_3:%[0-9]+]] = OpBitFieldSExtract %int [[load_7]] %uint_2 %uint_9
  // CHECK: [[cast:%[0-9]+]] = OpBitcast %uint [[extract_3]]
  // CHECK: OpStore %t5 [[cast]]
  uint t5 = s2.f2;

  // CHECK: [[ptr_8:%[0-9]+]] = OpAccessChain %_ptr_Function_int [[s3_var]] %int_0
  // CHECK: OpStore [[ptr_8]] %int_3
  s3.f1 = 3;

  // CHECK: [[ptr_9:%[0-9]+]] = OpAccessChain %_ptr_Function_int [[s3_var]] %int_1
  // CHECK: [[load_8:%[0-9]+]] = OpLoad %int [[ptr_9]]
  // CHECK: [[insert_3:%[0-9]+]] = OpBitFieldInsert %int [[load_8]] %int_4 %uint_0 %uint_1
  // CHECK: OpStore [[ptr_9]] [[insert_3]]
  s3.f2 = 4;

  // CHECK: [[ptr_10:%[0-9]+]] = OpAccessChain %_ptr_Function_int [[s3_var]] %int_2
  // CHECK: OpStore [[ptr_10]] %int_5
  s3.f3 = 5;
}

