// RUN: %dxc -T cs_6_3 -E main -fspv-target-env=vulkan1.2 -fcgl  %s -spirv | FileCheck %s

// Ignore "init" Expr of RayQuery in the AST traversal.

// CHECK: OpCapability RayQueryKHR
// CHECK: OpExtension "SPV_KHR_ray_query"

void Fun() {
  RayQuery<0> RayQ;
}

struct SomeStruct {
  void DummyMethod() {};
};

[numthreads(1, 1, 1)]
void main() {
  SomeStruct Payload;
  Payload.DummyMethod();

// CHECK:     %RayQ0 = OpVariable %_ptr_Function_rayQueryKHR Function
// CHECK-NOT: OpStore %RayQ0
  RayQuery<0> RayQ0;

// CHECK:     %RayQ1 = OpVariable %_ptr_Function_rayQueryKHR Function
// CHECK-NOT: OpStore %RayQ1
  RayQuery<0> RayQ1 = RayQuery<0>();

// CHECK: %RayQ2 = OpVariable %_ptr_Function_rayQueryKHR Function
// CHECK: [[rayq0:%[a-zA-Z0-9_]+]] = OpLoad {{%[a-zA-Z0-9_]+}} %RayQ0
// CHECK: OpStore %RayQ2 [[rayq0]]
  RayQuery<0> RayQ2 = RayQ0;

// CHECK: %RayQ = OpVariable %_ptr_Function_rayQueryKHR Function
  Fun();
}
