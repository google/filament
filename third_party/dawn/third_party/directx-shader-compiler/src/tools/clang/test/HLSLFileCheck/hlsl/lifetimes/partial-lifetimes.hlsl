// RUN: %dxc -E main -opt-enable partial-lifetime-markers -T cs_6_0 -enable-lifetime-markers -fcgl %s | FileCheck %s -check-prefix=CHECK0
// RUN: %dxc -E main -opt-enable partial-lifetime-markers -T cs_6_0 -enable-lifetime-markers -fcgl %s | FileCheck %s -check-prefix=CHECK1
// RUN: %dxc -E main -opt-disable partial-lifetime-markers -T cs_6_0 -enable-lifetime-markers -fcgl %s | FileCheck %s -check-prefix=NEGATIVE

// This test is to make sure the partial-lifetime-markers optimization switch works as intended.

// Make sure these telltale signs of lifetime marker related cfg changes do not occur
// CHECK0-NOT: alloca i32
// CHECK0-NOT: switch i32
// CHECK0-NOT: call void @llvm.lifetime.end(

// Make sure lifetime.start is still generated
// CHECK1: call void @llvm.lifetime.start(

// Make sure turning off partial-lifetime-markers make these cfg modifications reappear again
// NEGATIVE-DAG: alloca i32
// NEGATIVE-DAG: switch i32
// NEGATIVE-DAG: call void @llvm.lifetime.end(

struct D {
 float3 d_member;
};

struct A {
  float4 a_member;
};

struct B {
    uint flags;
} ;

struct C {
    uint c_member;
};

StructuredBuffer   <D> srv0 : register(t0) ;
StructuredBuffer   <B> srv1 : register(t1) ;
RWStructuredBuffer <C> uav0 : register(u0) ;

[RootSignature("DescriptorTable(SRV(t0,numDescriptors=10)),DescriptorTable(UAV(u0,numDescriptors=10))")]
[numthreads (64, 1, 1)]
void main(uint3 dtid : SV_DispatchThreadID) {
 if (dtid.x < 10) {
    A decal = (A)0;
    {
      const D d = srv0[0];
      if (!d.d_member.x)
        return;
    }
    B b = srv1[0];
    if (b.flags & 1) {
      InterlockedMax(uav0[0].c_member, 10) ;
      return;
    }

    InterlockedMax(uav0[0].c_member, 20) ;
  }
  InterlockedMax(uav0[0].c_member, 30) ;
}

