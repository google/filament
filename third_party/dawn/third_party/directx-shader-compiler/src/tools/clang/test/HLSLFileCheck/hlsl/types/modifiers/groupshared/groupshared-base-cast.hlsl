// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// This tests cast of derived to base when derived is groupshared.
// Different use cases can hit different code paths, hence the variety of
// uses here:
//    - calling base method
//    - vector element assignment on base member
//    - casting to base and passing to function
// The barrier and write to RWBuf prevents optimizations from eliminating
// groupshared use, considering this dead-code, or detecting a race condition.

// CHECK: @[[gs0:.+]] = addrspace(3) global i32 undef
// CHECK: @[[gs1:.+]] = addrspace(3) global i32 undef
// CHECK: @[[gs2:.+]] = addrspace(3) global i32 undef
// CHECK: store i32 1, i32 addrspace(3)* @[[gs0]], align 4
// CHECK: store i32 2, i32 addrspace(3)* @[[gs1]], align 4
// CHECK: store i32 3, i32 addrspace(3)* @[[gs2]], align 4

// CHECK: %[[l0:[^ ]+]] = load i32, i32 addrspace(3)* @[[gs0]], align 4
// CHECK: %[[l1:[^ ]+]] = load i32, i32 addrspace(3)* @[[gs1]], align 4
// CHECK: %[[l2:[^ ]+]] = load i32, i32 addrspace(3)* @[[gs2]], align 4
// CHECK: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %{{.+}}, i32 %{{.+}}, i32 undef, i32 %[[l0]], i32 %[[l1]], i32 %[[l2]], i32 undef, i8 7)


class Base {
  uint3 u;
  void set_u_y(uint value) { u.y = value; }
};
class Derived : Base {
  float bar;
};

groupshared Derived gs_derived;
RWByteAddressBuffer RWBuf;

void UpdateBase_z(inout Base b, uint value) {
  b.u.z = value;
}

[numthreads(2, 1, 1)]
void main(uint3 groupThreadID: SV_GroupThreadID) {
  if (groupThreadID.x == 0) {
    gs_derived.u.x = 1;
    gs_derived.set_u_y(2);
    UpdateBase_z((Base)gs_derived, 3);
  }
  GroupMemoryBarrierWithGroupSync();
  uint addr = groupThreadID.x * 4;
  RWBuf.Store3(addr, gs_derived.u);
}
