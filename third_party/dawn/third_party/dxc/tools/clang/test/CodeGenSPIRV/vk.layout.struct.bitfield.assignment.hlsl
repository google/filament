// RUN: %dxc -T vs_6_0 -E main -HV 2021 -fcgl  %s -spirv | FileCheck %s

// Sanity check.
struct S1 {
    uint f1 : 1;
};

struct S2 {
    uint f1 : 1;
    uint f2 : 3;
    uint f3 : 8;
    uint f4 : 1;
};

struct S3 {
    uint f1 : 1;
     int f2 : 1;
    uint f3 : 1;
};

void main() : A {
  S1 s1;
// CHECK:     [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_uint %s1 %int_0
// CHECK:     [[load:%[0-9]+]] = OpLoad %uint [[ptr]]
// CHECK:     [[insert:%[0-9]+]] = OpBitFieldInsert %uint [[load]] %uint_1 %uint_0 %uint_1
// CHECK:     OpStore [[ptr]] [[insert]]
  s1.f1 = 1;

  S2 s2;
// CHECK:     [[ptr_0:%[0-9]+]] = OpAccessChain %_ptr_Function_uint %s2 %int_0
// CHECK:     [[load_0:%[0-9]+]] = OpLoad %uint [[ptr_0]]
// CHECK:     [[insert_0:%[0-9]+]] = OpBitFieldInsert %uint [[load_0]] %uint_1 %uint_0 %uint_1
// CHECK:     OpStore [[ptr_0]] [[insert_0]]
  s2.f1 = 1;
// CHECK:     [[ptr_1:%[0-9]+]] = OpAccessChain %_ptr_Function_uint %s2 %int_0
// CHECK:     [[load_1:%[0-9]+]] = OpLoad %uint [[ptr_1]]
// CHECK:     [[insert_1:%[0-9]+]] = OpBitFieldInsert %uint [[load_1]] %uint_5 %uint_1 %uint_3
// CHECK:     OpStore [[ptr_1]] [[insert_1]]
  s2.f2 = 5;
// CHECK:     [[ptr_2:%[0-9]+]] = OpAccessChain %_ptr_Function_uint %s2 %int_0
// CHECK:     [[load_2:%[0-9]+]] = OpLoad %uint [[ptr_2]]
// CHECK:     [[insert_2:%[0-9]+]] = OpBitFieldInsert %uint [[load_2]] %uint_2 %uint_4 %uint_8
// CHECK:     OpStore [[ptr_2]] [[insert_2]]
  s2.f3 = 2;

// CHECK:     [[ptr_3:%[0-9]+]] = OpAccessChain %_ptr_Function_uint %s2 %int_0
// CHECK:     [[load_3:%[0-9]+]] = OpLoad %uint [[ptr_3]]
// CHECK:     [[insert_3:%[0-9]+]] = OpBitFieldInsert %uint [[load_3]] %uint_1 %uint_12 %uint_1
// CHECK:     OpStore [[ptr_3]] [[insert_3]]
  s2.f4 = 1;

  S3 s3;
// CHECK:     [[ptr_4:%[0-9]+]] = OpAccessChain %_ptr_Function_uint %s3 %int_0
// CHECK:     [[load_4:%[0-9]+]] = OpLoad %uint [[ptr_4]]
// CHECK:     [[insert_4:%[0-9]+]] = OpBitFieldInsert %uint [[load_4]] %uint_1 %uint_0 %uint_1
// CHECK:     OpStore [[ptr_4]] [[insert_4]]
  s3.f1 = 1;
// CHECK:     [[ptr_5:%[0-9]+]] = OpAccessChain %_ptr_Function_int %s3 %int_1
// CHECK:     [[load_5:%[0-9]+]] = OpLoad %int [[ptr_5]]
// CHECK:     [[insert_5:%[0-9]+]] = OpBitFieldInsert %int [[load_5]] %int_0 %uint_0 %uint_1
// CHECK:     OpStore [[ptr_5]] [[insert_5]]
  s3.f2 = 0;
// CHECK:     [[ptr_6:%[0-9]+]] = OpAccessChain %_ptr_Function_uint %s3 %int_2
// CHECK:     [[load_6:%[0-9]+]] = OpLoad %uint [[ptr_6]]
// CHECK:     [[insert_6:%[0-9]+]] = OpBitFieldInsert %uint [[load_6]] %uint_1 %uint_0 %uint_1
// CHECK:     OpStore [[ptr_6]] [[insert_6]]
  s3.f3 = 1;

// CHECK:     [[ptr_7:%[0-9]+]] = OpAccessChain %_ptr_Function_uint %s2 %int_0
// CHECK:     [[load_7:%[0-9]+]] = OpLoad %uint [[ptr_7]]
// CHECK:     [[s2f4_extract:%[0-9]+]] = OpBitFieldUExtract %uint [[load_7]] %uint_12 %uint_1
// CHECK:     [[s2f4_sext:%[0-9]+]] = OpBitcast %int [[s2f4_extract]]
// CHECK:     [[ptr_8:%[0-9]+]] = OpAccessChain %_ptr_Function_int %s3 %int_1
// CHECK:     [[load_8:%[0-9]+]] = OpLoad %int [[ptr_8]]
// CHECK:     [[insert_7:%[0-9]+]] = OpBitFieldInsert %int [[load_8]] [[s2f4_sext]] %uint_0 %uint_1
// CHECK:     OpStore [[ptr_8]] [[insert_7]]
  s3.f2 = s2.f4;
}
