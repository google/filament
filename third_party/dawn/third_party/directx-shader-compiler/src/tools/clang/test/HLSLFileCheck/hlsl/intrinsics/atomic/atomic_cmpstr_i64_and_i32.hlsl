// RUN: %dxc -E CSMain -T cs_6_6 %s | FileCheck %s -check-prefix=GSCHECK
// RUN: %dxc -T ps_6_6 -DMEMTYPE=RWBuffer %s | FileCheck %s -check-prefixes=CHECK,TYCHECK
// RUN: %dxc -T ps_6_6 -DMEMTYPE=RWStructuredBuffer %s | FileCheck %s -check-prefix=CHECK
// RUN: %dxc -T ps_6_6 -DMEMTYPE=RWTexture1D %s | FileCheck %s -check-prefix=CHECK

// RUN: %dxilver 1.6 | %dxc -E CSMain -T cs_6_5 %s | FileCheck %s -check-prefix=ERRCHECK
// RUN: %dxilver 1.6 | %dxc -T ps_6_5 -DMEMTYPE=RWBuffer %s | FileCheck %s -check-prefix=ERRCHECK
// RUN: %dxilver 1.6 | %dxc -T ps_6_5 -DMEMTYPE=RWStructuredBuffer %s | FileCheck %s -check-prefix=ERRCHECK
// RUN: %dxilver 1.6 | %dxc -T ps_6_5 -DMEMTYPE=RWTexture1D %s | FileCheck %s -check-prefix=ERRCHECK

// Verify that the first arg determines the overload and the others can be what they will

#ifdef MEMTYPE
MEMTYPE<uint>     resU;
MEMTYPE<int>      resI;
MEMTYPE<uint64_t> resU64;
MEMTYPE<int64_t>  resI64;
#else
groupshared uint     resU[256];
groupshared int      resI[256];
groupshared uint64_t resU64[256];
groupshared int64_t  resI64[256];
#endif

// TYCHECK: Note: shader requires additional functionality:
// TYCHECK: 64-bit Atomics on Typed Resources
// GSCHECK: Note: shader requires additional functionality:
// GSCHECK: 64-bit Atomics on Group Shared

void dotest( uint a, uint b, uint c)
{
  resU[a] = a;
  resI[a] = a;
  resU64[a] = a;
  resI64[a] = a;

  uint uv = b - c;
  uint uv2 = b + c;
  int iv = b / c;
  int iv2 = b * c;
  uint64_t bb = b;
  uint64_t cc = c;
  uint64_t luv = bb * cc;
  uint64_t luv2 = bb / cc;
  int64_t liv = bb + cc;
  int64_t liv2 = bb - cc;

  // test some basic examples
  // GSCHECK: cmpxchg i32
  // GSCHECK: cmpxchg i32
  // GSCHECK: cmpxchg i64
  // GSCHECK: cmpxchg i64
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: call i64 @dx.op.atomicCompareExchange.i64
  // CHECK: call i64 @dx.op.atomicCompareExchange.i64
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  InterlockedCompareStore( resU[a], uv, uv2);
  InterlockedCompareStore( resI[a], iv, iv2 );
  InterlockedCompareStore( resU64[a], luv, luv2);
  InterlockedCompareStore( resI64[a], liv, liv2);

  // test some signed and unsigned variables
  // GSCHECK: cmpxchg i32
  // GSCHECK: cmpxchg i32
  // GSCHECK: cmpxchg i32
  // GSCHECK: cmpxchg i32
  // GSCHECK: cmpxchg i32
  // GSCHECK: cmpxchg i32
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  InterlockedCompareStore( resU[a], iv, iv2 );
  InterlockedCompareStore( resU[a], iv, uv2 );
  InterlockedCompareStore( resU[a], uv, iv2 );
  InterlockedCompareStore( resI[a], uv, uv2 );
  InterlockedCompareStore( resI[a], uv, iv2 );
  InterlockedCompareStore( resI[a], iv, uv2 );

  // test some literals with 32-bit resources
  // GSCHECK: cmpxchg i32
  // GSCHECK: cmpxchg i32
  // GSCHECK: cmpxchg i32
  // GSCHECK: cmpxchg i32
  // GSCHECK: cmpxchg i32
  // GSCHECK: cmpxchg i32
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  InterlockedCompareStore( resU[a], 1.0, 2.0 );
  InterlockedCompareStore( resU[a], iv, 2.0 );
  InterlockedCompareStore( resU[a], 1.0, iv2 );
  InterlockedCompareStore( resI[a], 1.0, 2.0 );
  InterlockedCompareStore( resI[a], 1.0, iv2 );
  InterlockedCompareStore( resI[a], iv, 2.0 );

  // test some basic 64-bit variables
  // GSCHECK: cmpxchg i64
  // GSCHECK: cmpxchg i64
  // GSCHECK: cmpxchg i64
  // GSCHECK: cmpxchg i64
  // GSCHECK: cmpxchg i64
  // GSCHECK: cmpxchg i64
  // CHECK: call i64 @dx.op.atomicCompareExchange.i64
  // CHECK: call i64 @dx.op.atomicCompareExchange.i64
  // CHECK: call i64 @dx.op.atomicCompareExchange.i64
  // CHECK: call i64 @dx.op.atomicCompareExchange.i64
  // CHECK: call i64 @dx.op.atomicCompareExchange.i64
  // CHECK: call i64 @dx.op.atomicCompareExchange.i64
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  InterlockedCompareStore( resU64[a], liv, liv2 );
  InterlockedCompareStore( resU64[a], liv, luv2 );
  InterlockedCompareStore( resU64[a], luv, liv2 );
  InterlockedCompareStore( resI64[a], luv, luv2 );
  InterlockedCompareStore( resI64[a], luv, liv2 );
  InterlockedCompareStore( resI64[a], liv, luv2 );

  // test some literals with 64-bit resources
  // GSCHECK: cmpxchg i64
  // GSCHECK: cmpxchg i64
  // GSCHECK: cmpxchg i64
  // GSCHECK: cmpxchg i64
  // GSCHECK: cmpxchg i64
  // GSCHECK: cmpxchg i64
  // CHECK: call i64 @dx.op.atomicCompareExchange.i64
  // CHECK: call i64 @dx.op.atomicCompareExchange.i64
  // CHECK: call i64 @dx.op.atomicCompareExchange.i64
  // CHECK: call i64 @dx.op.atomicCompareExchange.i64
  // CHECK: call i64 @dx.op.atomicCompareExchange.i64
  // CHECK: call i64 @dx.op.atomicCompareExchange.i64
  InterlockedCompareStore( resU64[a], 1.0, 2.0 );
  InterlockedCompareStore( resU64[a], iv, 2.0 );
  InterlockedCompareStore( resU64[a], 1.0, iv2 );
  InterlockedCompareStore( resI64[a], 1.0, 2.0 );
  InterlockedCompareStore( resI64[a], 1.0, iv2 );
  InterlockedCompareStore( resI64[a], iv, 2.0 );

  // test some mixed 32 and 64-bit variables with 32-bit resources
  // GSCHECK: cmpxchg i32
  // GSCHECK: cmpxchg i32
  // GSCHECK: cmpxchg i32
  // GSCHECK: cmpxchg i32
  // GSCHECK: cmpxchg i32
  // GSCHECK: cmpxchg i32
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  InterlockedCompareStore( resU[a], luv, luv2 );
  InterlockedCompareStore( resU[a], luv, uv2 );
  InterlockedCompareStore( resU[a], uv, luv2 );
  InterlockedCompareStore( resI[a], liv, liv2 );
  InterlockedCompareStore( resI[a], liv, iv2 );
  InterlockedCompareStore( resI[a], iv, liv2 );

  // test some mixed 32 and 64-bit variables with 64-bit resources
  // GSCHECK: cmpxchg i64
  // GSCHECK: cmpxchg i64
  // GSCHECK: cmpxchg i64
  // GSCHECK: cmpxchg i64
  // GSCHECK: cmpxchg i64
  // GSCHECK: cmpxchg i64
  // CHECK: call i64 @dx.op.atomicCompareExchange.i64
  // CHECK: call i64 @dx.op.atomicCompareExchange.i64
  // CHECK: call i64 @dx.op.atomicCompareExchange.i64
  // CHECK: call i64 @dx.op.atomicCompareExchange.i64
  // CHECK: call i64 @dx.op.atomicCompareExchange.i64
  // CHECK: call i64 @dx.op.atomicCompareExchange.i64
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  InterlockedCompareStore( resU64[a], uv, uv2 );
  InterlockedCompareStore( resU64[a], uv, luv2 );
  InterlockedCompareStore( resU64[a], luv, uv2 );
  InterlockedCompareStore( resI64[a], iv, iv2 );
  InterlockedCompareStore( resI64[a], iv, liv2 );
  InterlockedCompareStore( resI64[a], liv, iv2 );
}

void main( uint a : A, uint b: B, uint c :C) : SV_Target
{
  dotest(a,b,c);
}

[numthreads(1,1,1)]
void CSMain( uint3 gtid : SV_GroupThreadID)
{
  dotest(gtid.x, gtid.y, gtid.z);
}
