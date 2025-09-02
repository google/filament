// RUN: %dxc -E main -T as_6_8 -spirv %s -E main -fspv-target-env=vulkan1.1spirv1.4 | FileCheck %s

struct S {
  uint a;
};

groupshared S s;
// CHECK: %s = OpVariable {{.*}} TaskPayloadWorkgroupEXT

[numthreads(1, 1, 1)]
void main()
{
// CHECK: OpEmitMeshTasksEXT %uint_1 %uint_1 %uint_1 %s
	DispatchMesh(1, 1, 1, s);
}
