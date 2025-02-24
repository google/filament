// RUN: %dxc  -T as_6_5 %s | FileCheck %s
// RUN: %dxc  -T as_6_5 %s -fcgl | FileCheck %s --check-prefix=CAST

// Make sure we pass groupshared mesh payload directly
// in to DispatchMesh, with no alloca involved.

// CHECK: define void @main
// CHECK-NOT: alloca
// CHECK-NOT: addrspacecast
// CHECK-NOT: bitcast
// CHECK: call void @dx.op.dispatchMesh.struct.MeshPayload{{[^ ]*}}(i32 173, i32 1, i32 1, i32 1, %struct.MeshPayload{{[^ ]*}} addrspace(3)*
// CHECK-NOT: addrspacecast
// CHECK: ret void

// Make sure addrspacecast is generated.
// CAST: @[[GS:.+]] = external addrspace(3) global %struct.GSStruct
// CAST: define void @main
// CAST: call void @"dx.hl.op..void (i32, i32, i32, i32, %struct.MeshPayload*)"(i32 {{[0-9]+}}, i32 1, i32 1, i32 1, %struct.MeshPayload* addrspacecast (%struct.MeshPayload addrspace(3)* getelementptr inbounds (%struct.GSStruct, %struct.GSStruct addrspace(3)* @[[GS]], i32 0, i32 1) to %struct.MeshPayload*))


struct MeshPayload
{
  uint4 data;
};

struct GSStruct
{
  uint i;
  MeshPayload pld;
};

groupshared GSStruct gs;
GSStruct cb_gs;

[numthreads(4,1,1)]
void main(uint gtid : SV_GroupIndex)
{
  gs = cb_gs;
  DispatchMesh(1,1,1,gs.pld);
}
