// RUN: %dxc -fdisable-loc-tracking -E main -opt-enable structurize-returns -T cs_6_0 -enable-lifetime-markers -fcgl %s | FileCheck %s -check-prefix=FCGL
// RUN: %dxc -fdisable-loc-tracking -E main -opt-enable structurize-returns -T cs_6_0 -enable-lifetime-markers %s | FileCheck %s
// RUN: %dxc -fdisable-loc-tracking -E main -opt-enable structurize-returns -T cs_6_0 -enable-lifetime-markers %s | FileCheck %s -check-prefix=WARNING -input-file=stderr
// RUN: %dxc -fdisable-loc-tracking -E main -opt-enable structurize-returns -T cs_6_0 -disable-lifetime-markers -fcgl %s | FileCheck %s -check-prefix=NO-LIFETIME

// Regression test for a bug where program structure is completely messed up when lifetime-markers are enabled and
// -opt-enable structurize-returns is on. The scope information recorded during codegen that structurize-returns uses
// to modify the control flow is incorrect if lifetime-markers are enabled. This test checks that 

//=================================
// The fcgl test checks the return condition alloca bReturn is not generated and the cleanup code for lifetime-markers
// is present.
// FCGL-NOT: bReturned
// FCGL: %cleanup.dest

//=================================
// The non-fcgl test checks the shader is compiled correctly (the bug causes irreducible flow)
// CHECK-DAG: @main

//=================================
// Check a warning was emitted.
// WARNING: structurize-returns skipped function 'main' due to incompatibility with lifetime markers. Use -disable-lifetime-markers to enable structurize-exits on this function.

//=================================
// The last test makes sure structurize-returns runs as expected
// NO-LIFETIME: @main
// NO-LIFETIME: %bReturned = alloca

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
