// RUN: %dxc -T cs_6_6 -fcgl %s | FileCheck %s
// RUN: %dxc -T cs_6_6 -Od -fcgl %s | FileCheck --check-prefix=NO_LIFETIMES %s

// Regression test: indexing into a resource heap (or sampler heap) as a
// discarded-value expression (i.e. its result is never read) used to crash
// the compiler. Clang still emits an alloca plus lifetime markers for the
// implicit temporary in that case, and CGHLSLMSHelper::LowerGetResourceFromHeap
// only expected the resource pointer to be used through a resource-typed
// bitcast that is loaded from; it did not expect the i8* bitcast used by
// llvm.lifetime.start/llvm.lifetime.end, which triggered an invalid cast<>
// (see FinishIntrinsics -> LowerGetResourceFromHeap).

// When `-Od` is passed lifetime markers are implicitly disabled, so the
// remainder of the IR changes are irrelevant.
// NO_LIFETIMES: define void @main

// CHECK: [[ResourceX:%.*]] = alloca %struct..Resource
// CHECK: [[SamplerX:%.*]] = alloca %struct..Sampler
// CHECK: [[ResourceY:%.*]] = alloca %struct..Resource
// CHECK: [[SamplerY:%.*]] = alloca %struct..Sampler

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID) {
  // CHECK: [[Start:%.*]] = bitcast %struct..Resource* [[ResourceX]] to i8*
  // CHECK-NEXT:  call void @llvm.lifetime.start(i64 4, i8* [[Start]])
  // CHECK: [[End:%.*]] = bitcast %struct..Resource* [[ResourceX]] to i8*
  // CHECK-NEXT:  call void @llvm.lifetime.end(i64 4, i8* [[End]])
  ResourceDescriptorHeap[tid.x];

  // CHECK: [[Start:%.*]] = bitcast %struct..Sampler* [[SamplerX]] to i8*
  // CHECK-NEXT:  call void @llvm.lifetime.start(i64 4, i8* [[Start]])
  // CHECK: [[End:%.*]] = bitcast %struct..Sampler* [[SamplerX]] to i8*
  // CHECK-NEXT:  call void @llvm.lifetime.end(i64 4, i8* [[End]])
  SamplerDescriptorHeap[tid.x];

  
  // CHECK: [[Start:%.*]] = bitcast %struct..Resource* [[ResourceY]] to i8*
  // CHECK-NEXT:  call void @llvm.lifetime.start(i64 4, i8* [[Start]])
  // CHECK: [[End:%.*]] = bitcast %struct..Resource* [[ResourceY]] to i8*
  // CHECK-NEXT:  call void @llvm.lifetime.end(i64 4, i8* [[End]])
  (void)ResourceDescriptorHeap[tid.y];

  // CHECK: [[Start:%.*]] = bitcast %struct..Sampler* [[SamplerY]] to i8*
  // CHECK-NEXT:  call void @llvm.lifetime.start(i64 4, i8* [[Start]])
  // CHECK: [[End:%.*]] = bitcast %struct..Sampler* [[SamplerY]] to i8*
  // CHECK-NEXT:  call void @llvm.lifetime.end(i64 4, i8* [[End]])
  (void)SamplerDescriptorHeap[tid.y];
}
