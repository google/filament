// RUN: %dxc -T cs_6_5 -E main -fspv-target-env=vulkan1.2 -fspv-debug=vulkan-with-source -spirv %s | FileCheck %s

// CHECK: [[rayQueryKHR_name_id:%[0-9]+]] = OpString "@rayQueryKHR"
// CHECK: [[rayQueryKHR_linkage_name_id:%[0-9]+]] = OpString "rayQueryKHR"
// CHECK: [[debug_info_none_id:%[0-9]+]] = OpExtInst %void [[_:%[0-9]+]] DebugInfoNone
// CHECK: [[_:%[0-9]+]] = OpExtInst %void [[_:%[0-9]+]] DebugTypeComposite [[rayQueryKHR_name_id]] %uint_0 [[_:%[0-9]+]] %uint_0 %uint_0 [[_:%[0-9]+]] [[rayQueryKHR_linkage_name_id]] [[debug_info_none_id]] %uint_3{{$}}

[numthreads(64, 1, 1)]
void main() {
    RayQuery<RAY_FLAG_NONE> q;
}
