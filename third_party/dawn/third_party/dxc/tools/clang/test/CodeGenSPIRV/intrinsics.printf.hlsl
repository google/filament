// RUN: %dxc -T cs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:                OpExtension "SPV_KHR_non_semantic_info"
// CHECK: [[set:%[0-9]+]] = OpExtInstImport "NonSemantic.DebugPrintf"

// CHECK: [[format1:%[0-9]+]] = OpString "first string"
// CHECK: [[format2:%[0-9]+]] = OpString "second string"
// CHECK: [[format3:%[0-9]+]] = OpString "please print this message."
// CHECK: [[format4:%[0-9]+]] = OpString "Variables are: %d %d %.2f"
// CHECK: [[format5:%[0-9]+]] = OpString "Integers are: %d %d %d"
// CHECK: [[format6:%[0-9]+]] = OpString "More: %d %d %d %d %d %d %d %d %d %d"

const string first = "first string";
string second = "second string";

[numthreads(1,1,1)]
void main() {
// CHECK: {{%[0-9]+}} = OpExtInst %void [[set]] 1 [[format1]]
  printf(first);
// CHECK: {{%[0-9]+}} = OpExtInst %void [[set]] 1 [[format2]]
  printf(second);
// CHECK: {{%[0-9]+}} = OpExtInst %void [[set]] 1 [[format3]]
  printf("please print this message.");
// CHECK: {{%[0-9]+}} = OpExtInst %void [[set]] 1 [[format4]] %uint_1 %uint_2 %float_1_5
  printf("Variables are: %d %d %.2f", 1u, 2u, 1.5f);
// CHECK: {{%[0-9]+}} = OpExtInst %void [[set]] 1 [[format5]] %int_1 %int_2 %int_3
  printf("Integers are: %d %d %d", 1, 2, 3);
// CHECK: {{%[0-9]+}} = OpExtInst %void [[set]] 1 [[format6]] %int_1 %int_2 %int_3 %int_4 %int_5 %int_6 %int_7 %int_8 %int_9 %int_10
  printf("More: %d %d %d %d %d %d %d %d %d %d", 1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
}

