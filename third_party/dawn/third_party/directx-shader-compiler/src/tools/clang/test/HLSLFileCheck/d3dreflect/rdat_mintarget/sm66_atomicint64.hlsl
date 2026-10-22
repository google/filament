// RUN: %dxilver 1.8 | %dxc -T lib_6_8 %s | %D3DReflect %s | %FileCheck %s -check-prefixes=RDAT,RDAT18
// RUN: %dxilver 1.7 | %dxc -T lib_6_7 -validator-version 1.7 %s | %D3DReflect %s | FileCheck %s -check-prefixes=RDAT,RDAT17

// Ensure min shader target incorporates optional features used

// RDAT: FunctionTable[{{.*}}] = {

// SM 6.6+

///////////////////////////////////////////////////////////////////////////////
// ShaderFeatureInfo_AtomicInt64OnTypedResource (0x400000) = 4194304
// Typed resource atomics produce an intrinsic that requires 6.6 
// producing the correct version requirement without considering the flag.

// RDAT-LABEL: UnmangledName: "atomic_typed"
// RDAT:   FeatureInfo1: (Int64Ops | AtomicInt64OnTypedResource)
// RDAT:   FeatureInfo2: 0
// RDAT: MinShaderTarget: 0x60066

RWBuffer<uint64_t> RWBuf : register(u0, space0);

[noinline] export
void atomic_typed() {
  uint64_t original;
  InterlockedExchange(RWBuf[0], 12, original);
}

///////////////////////////////////////////////////////////////////////////////
// ShaderFeatureInfo_AtomicInt64OnGroupShared (0x800000) = 8388608

// RDAT-LABEL: UnmangledName: "atomic_groupshared"
// ShaderFeatureInfo_AtomicInt64OnGroupShared (0x800000) = 8388608
// + ShaderFeatureInfo_Int64Ops (0x8000) = 0x808000 = 8421376
// RDAT:   FeatureInfo1: (Int64Ops | AtomicInt64OnGroupShared)
// RDAT18:   FeatureInfo2: (Opt_RequiresGroup)
// Old: no Opt_RequiresGroup flag
// RDAT17:   FeatureInfo2: 0
// MinShaderTarget: (Compute(5) << 16) + (SM 6.6 ((6 << 4) + 6)) = 0x50066 = 327782
// RDAT18: MinShaderTarget: 0x50066
// Old: 6.0
// RDAT17: MinShaderTarget: 0x50060

RWByteAddressBuffer BAB : register(u1, space0);
groupshared int64_t gs;

[shader("compute")]
[numthreads(1,1,1)]
void atomic_groupshared(uint tidx : SV_GroupIndex) {
  if (tidx == 0)
    gs = 0;
  GroupMemoryBarrierWithGroupSync();
  uint64_t original;
  InterlockedExchange(gs, tidx, original);
  BAB.Store(tidx * 4, original);
}

///////////////////////////////////////////////////////////////////////////////
// ShaderFeatureInfo_AtomicInt64OnHeapResource (0x10000000) = 268435456

// RDAT-LABEL: UnmangledName: "atomic_heap"
// RDAT:   FeatureInfo1: (Int64Ops | ResourceDescriptorHeapIndexing | AtomicInt64OnHeapResource)
// RDAT:   FeatureInfo2: 0
// RDAT17: ShaderStageFlag: (Pixel | Vertex | Geometry | Hull | Domain | Compute | Library | RayGeneration | Intersection | AnyHit | ClosestHit | Miss | Callable | Mesh | Amplification)
// RDAT18: ShaderStageFlag: (Pixel | Vertex | Geometry | Hull | Domain | Compute | Library | RayGeneration | Intersection | AnyHit | ClosestHit | Miss | Callable | Mesh | Amplification | Node)
// RDAT:   MinShaderTarget: 0x60066

[noinline] export
void atomic_heap() {
  uint64_t original;
  RWStructuredBuffer<uint64_t> SB64 = ResourceDescriptorHeap[0];
  InterlockedExchange(SB64[0], 12, original);
}

// RDAT-LABEL: UnmangledName: "atomic_heap_raygen"
// RDAT18: FeatureInfo1: (Int64Ops | ResourceDescriptorHeapIndexing | AtomicInt64OnHeapResource)
// Old: missed called function
// RDAT17: FeatureInfo1: 0
// RDAT:   FeatureInfo2: 0
// RDAT:   ShaderStageFlag: (RayGeneration)
// RDAT18: MinShaderTarget: 0x70066
// Old: 6.0
// RDAT17: MinShaderTarget: 0x70060

[shader("raygeneration")]
void atomic_heap_raygen() {
  atomic_heap();
}
