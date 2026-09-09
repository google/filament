// RUN: %dxc %s -fcgl -spirv -T ps_6_8 -fspv-target-env=vulkan1.1spirv1.4 | FileCheck %s



struct WithBool {
  bool b;
};

struct StructWithBool {
  WithBool wb;
};

struct StructWithoutBool {
  int a;
};

struct OuterStruct {
  StructWithBool a[2];
  WithBool b;
  StructWithoutBool c;
  StructWithoutBool d[2];
} S;


// CHECK: %GetStruct = OpFunction %OuterStruct_0 None %34
// CHECK: %bb_entry_0 = OpLabel
// CHECK: [[ld:%[0-9]+]] = OpLoad %OuterStruct %39

// The array `a` must be split up because it contains a bool that needs a
// conversion from int to bool.
// CHECK: [[arr_with_bool:%[0-9]+]] = OpCompositeExtract %_arr_StructWithBool_uint_2 [[ld]] 0
// CHECK: [[struct_with_bool:%[0-9]+]] = OpCompositeExtract %StructWithBool [[arr_with_bool]] 0
// CHECK: [[with_bool:%[0-9]+]] = OpCompositeExtract %WithBool [[struct_with_bool]] 0
// CHECK: [[int:%[0-9]+]] = OpCompositeExtract %uint [[with_bool]] 0
// CHECK: [[bool:%[0-9]+]] = OpINotEqual %bool [[int]] %uint_0
// CHECK: [[with_bool:%[0-9]+]] = OpCompositeConstruct %WithBool_0 [[bool]]
// CHECK: [[struct_with_bool:%[0-9]+]] = OpCompositeConstruct %StructWithBool_0 [[with_bool]]

// Skip second element of the array. It is more of the same.
// CHECK: [[a:%[0-9]+]] = OpCompositeConstruct %_arr_StructWithBool_0_uint_2 [[struct_with_bool]] {{%.*}}

// The struct `b` must be split up for the same reason.
// CHECK: [[with_bool:%[0-9]+]] = OpCompositeExtract %WithBool [[ld]] 1
// CHECK: [[int:%[0-9]+]] = OpCompositeExtract %uint [[with_bool]] 0
// CHECK: [[bool:%[0-9]+]] = OpINotEqual %bool [[int]] %uint_0
// CHECK: [[b:%[0-9]+]] = OpCompositeConstruct %WithBool_0 [[bool]]

// The struct `c` can use OpCopyLogical.
// CHECK: %59 = OpCompositeExtract %StructWithoutBool [[ld]] 2
// CHECK: [[c:%[0-9]+]] = OpCopyLogical %StructWithoutBool_0 %59

// The array `d` can use OpCopyLogical.
// CHECK: %61 = OpCompositeExtract %_arr_StructWithoutBool_uint_2 [[ld]] 3
// CHECK: [[d:%[0-9]+]] = OpCopyLogical %_arr_StructWithoutBool_0_uint_2 %61

// CHECK: [[r:%[0-9]+]] = OpCompositeConstruct %OuterStruct_0 [[a]] [[b]] [[c]] [[d]]
// CHECK: OpStore {{%.*}} [[r]]
// CHECK: OpFunctionEnd

OuterStruct GetStruct() { return S; }

uint main() : SV_TARGET
{
  GetStruct();
  return 0;
}

