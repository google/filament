// RUN: %dxc -T cs_6_0 %s -enable-lifetime-markers -opt-enable partial-lifetime-markers | FileCheck %s -check-prefixes=CHECK,POSITIVE
// RUN: %dxc -T cs_6_0 %s -disable-lifetime-markers -opt-disable partial-lifetime-markers | FileCheck %s -check-prefixes=CHECK,NEGATIVE
// RUN: %dxc -T cs_6_0 %s -disable-lifetime-markers -opt-enable partial-lifetime-markers | FileCheck %s -check-prefixes=CHECK,NEGATIVE

// RUN: %dxc -T cs_6_0 %s -enable-lifetime-markers -opt-enable partial-lifetime-markers -fcgl | FileCheck %s -check-prefixes=FCGL-PARTIAL
// RUN: %dxc -T cs_6_0 %s -enable-lifetime-markers -opt-disable partial-lifetime-markers -fcgl | FileCheck %s -check-prefixes=FCGL-FULL
// RUN: %dxc -T cs_6_0 %s -disable-lifetime-markers -opt-enable partial-lifetime-markers -fcgl | FileCheck %s -check-prefixes=FCGL-NONE
// RUN: %dxc -T cs_6_0 %s -disable-lifetime-markers -opt-disable partial-lifetime-markers -fcgl | FileCheck %s -check-prefixes=FCGL-NONE

// FCGL-PARTIAL: call void @llvm.lifetime.start
// FCGL-PARTIAL-NOT: call void @llvm.lifetime.end

// FCGL-FULL: call void @llvm.lifetime.start
// FCGL-FULL: call void @llvm.lifetime.end

// FCGL-NONE-NOT: call void @llvm.lifetime


//=======================================================================================================
// This is a test for lifetime marker's ability to sever value dependency with previous iteration of loops for
// variables that are declared inside of loops. In this test, variable `state` is declared inside of the
// loop.

// Because the definition for `state` is conditional, the compiler assumes its value can be from
// the previous iteration:
//
//  loop:
//    %last_iteration_val = phi float [ %new_val, %if.end.b ], [ undef, <preheader> ]
//    br i1, %if.then.a, %if.end.a
//  
//  if.then.a:
//    %new_val.then = ...
//
//  if.end.a:
//    %new_val = phi float [ %last_iteration_val, %loop ], [ %new_val.then, %if.then.0 ]
//    br i1, %if.then.b, %if.end.b
//
//  if.then.b:
//    store %new_val
//    br %if.end.b
//
//  if.end.b:
//    br i1, <loop-exit>, %loop
//  
//
// mem2reg has special logic for handling lifetime.start, where it basically assumes it as a
// def for the alloca. As a result, the ir ends up looking like this:
//
//  loop:
//    // Notice the phi is gone.
//    br i1, %if.then.a, %if.end.a
//  
//  if.then.a:
//    %new_val.then = ...
//
//  if.end.a:
//    // The undef is now here!
//    %new_val = phi float [ undef, %loop ], [ %new_val.then, %if.then.0 ]
//    br i1, %if.then.b, %if.end.b
//
//  if.then.b:
//    store %new_val
//    br %if.end.b
//
//  if.end.b:
//    br i1, <loop-exit>, %loop
//
//
// When `state` is a big struct, and when the loop has large amounts of temporary values, the
// phis for previous loop iterations can cause very large register pressure.
//
// This test verifies the lifetime markers' effect on this pattern, and verifies the newly added
// partial-lifetime-markers optimization switch works the same.
//

struct S { float before_; float x0; float after_; };
RWTexture1D<float> output : register(u0);
RWTexture1D<float> input  : register(u1);
RWTexture1D<float> conds  : register(u2);
RWTexture1D<float> conds2 : register(u3);
cbuffer cb : register(b0) {
  int N;
};

[RootSignature("CBV(b0), DescriptorTable(UAV(u0, numDescriptors=10))")]
[numthreads(4,4,1)]
// @CHECK: @main
void main() {
  [loop]
  for (int i = 0; i < N; ++i) {
    // CHECK-NOT: phi float
    // CHECK: phi i32
    //   make sure the float phi is not there when LTM is enabled, and there when LTM is disabled.
    // POSITIVE-NOT: phi float
    // NEGATIVE-NEXT: phi float
    S state;

    // CHECK: br i1
    [branch]
    if (conds[i]) {
      state.x0 = input[i+0];
      // CHECK: br label
    }

    // CHECK: phi float
    // CHECK: br i1
    [branch]
    if (conds2[i]) {
      output[i+0] = state.x0;
      // CHECK: br label
    }
  }
}

 