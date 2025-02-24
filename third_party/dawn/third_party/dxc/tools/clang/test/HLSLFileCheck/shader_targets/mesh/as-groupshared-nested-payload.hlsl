// RUN: %dxc -E amplification -T as_6_5 %s | FileCheck %s

// Make sure we pass groupshared mesh payload directly into DispatchMesh,
// with correct type, and no alloca involved.

// CHECK: define void @amplification
// CHECK-NOT: alloca
// CHECK-NOT: addrspacecast
// CHECK-NOT: bitcast
// CHECK: call void @dx.op.dispatchMesh.struct.MeshPayload{{[^ ]*}}(i32 173, i32 1, i32 1, i32 1, %struct.MeshPayload{{[^ ]*}} addrspace(3)*
// CHECK-NOT: addrspacecast
// CHECK: ret void

struct MeshPayload
{
  float arr[3];
  uint4 data;
};

struct GSStruct
{
  MeshPayload pld;
  MeshPayload pld2;
};

groupshared GSStruct gs;
GSStruct cb_gs;

[numthreads(4,1,1)]
void amplification(uint gtid : SV_GroupIndex)
{
//  gs = cb_gs;
  gs.pld.data[gtid] = gtid;
  DispatchMesh(1,1,1,gs.pld);
}
