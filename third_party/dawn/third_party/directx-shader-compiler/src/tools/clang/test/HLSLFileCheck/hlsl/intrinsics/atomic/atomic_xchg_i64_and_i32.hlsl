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

  // Test basic examples
  // GSCHECK: atomicrmw xchg i32
  // GSCHECK: atomicrmw xchg i32
  // GSCHECK: atomicrmw xchg i64
  // GSCHECK: atomicrmw xchg i64
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i64 @dx.op.atomicBinOp.i64
  // CHECK: call i64 @dx.op.atomicBinOp.i64
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  InterlockedExchange( resU[a], uv, uv2);
  InterlockedExchange( resI[a], iv, iv2 );
  InterlockedExchange( resU64[a], luv, luv2);
  InterlockedExchange( resI64[a], liv, liv2);

  // Test signed and unsigned
  // GSCHECK: atomicrmw xchg i32
  // GSCHECK: atomicrmw xchg i32
  // GSCHECK: atomicrmw xchg i32
  // GSCHECK: atomicrmw xchg i32
  // GSCHECK: atomicrmw xchg i32
  // GSCHECK: atomicrmw xchg i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  InterlockedExchange( resU[a], iv, iv2 );
  InterlockedExchange( resU[a], iv, uv2 );
  InterlockedExchange( resU[a], uv, iv2 );
  InterlockedExchange( resI[a], uv, uv2 );
  InterlockedExchange( resI[a], uv, iv2 );
  InterlockedExchange( resI[a], iv, uv2 );

  // Test literals with 32 bit resources
  // GSCHECK: atomicrmw xchg i32
  // GSCHECK: atomicrmw xchg i32
  // GSCHECK: atomicrmw xchg i32
  // GSCHECK: atomicrmw xchg i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  InterlockedExchange( resU[a], 1.0, iv2 );
  InterlockedExchange( resU[a], 2.0, uv2 );
  InterlockedExchange( resI[a], 2.0, iv2 );
  InterlockedExchange( resI[a], 1.0, uv2 );

  // Test basic 64-bit variables
  // GSCHECK: atomicrmw xchg i64
  // GSCHECK: atomicrmw xchg i64
  // GSCHECK: atomicrmw xchg i64
  // GSCHECK: atomicrmw xchg i64
  // GSCHECK: atomicrmw xchg i64
  // GSCHECK: atomicrmw xchg i64
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // CHECK: call i64 @dx.op.atomicBinOp.i64
  // CHECK: call i64 @dx.op.atomicBinOp.i64
  // CHECK: call i64 @dx.op.atomicBinOp.i64
  // CHECK: call i64 @dx.op.atomicBinOp.i64
  // CHECK: call i64 @dx.op.atomicBinOp.i64
  // CHECK: call i64 @dx.op.atomicBinOp.i64
  InterlockedExchange( resU64[a], liv, liv2 );
  InterlockedExchange( resU64[a], liv, luv2 );
  InterlockedExchange( resU64[a], luv, liv2 );
  InterlockedExchange( resI64[a], luv, luv2 );
  InterlockedExchange( resI64[a], luv, liv2 );
  InterlockedExchange( resI64[a], liv, luv2 );

  // Test some literals with 64-bit resources
  // GSCHECK: atomicrmw xchg i64
  // GSCHECK: atomicrmw xchg i64
  // GSCHECK: atomicrmw xchg i64
  // GSCHECK: atomicrmw xchg i64
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // CHECK: call i64 @dx.op.atomicBinOp.i64
  // CHECK: call i64 @dx.op.atomicBinOp.i64
  // CHECK: call i64 @dx.op.atomicBinOp.i64
  // CHECK: call i64 @dx.op.atomicBinOp.i64
  InterlockedExchange( resU64[a], 1.0, liv2 );
  InterlockedExchange( resU64[a], 2.0, luv2 );
  InterlockedExchange( resI64[a], 2.5, luv2 );
  InterlockedExchange( resI64[a], 1.5, liv2 );

  // test some mixed 32 and 64-bit variables with 32-bit resources
  // GSCHECK: atomicrmw xchg i32
  // GSCHECK: atomicrmw xchg i32
  // GSCHECK: atomicrmw xchg i32
  // GSCHECK: atomicrmw xchg i32
  // GSCHECK: atomicrmw xchg i32
  // GSCHECK: atomicrmw xchg i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  InterlockedExchange( resU[a], luv, luv2 );
  InterlockedExchange( resU[a], luv, uv2 );
  InterlockedExchange( resU[a], uv, luv2 );
  InterlockedExchange( resI[a], liv, liv2 );
  InterlockedExchange( resI[a], liv, iv2 );
  InterlockedExchange( resI[a], iv, liv2 );

  // Test some mixed 32 and 64-bit variables with 64-bit resources
  // GSCHECK: atomicrmw xchg i64
  // GSCHECK: atomicrmw xchg i64
  // GSCHECK: atomicrmw xchg i64
  // GSCHECK: atomicrmw xchg i64
  // GSCHECK: atomicrmw xchg i64
  // GSCHECK: atomicrmw xchg i64
  // CHECK: call i64 @dx.op.atomicBinOp.i64
  // CHECK: call i64 @dx.op.atomicBinOp.i64
  // CHECK: call i64 @dx.op.atomicBinOp.i64
  // CHECK: call i64 @dx.op.atomicBinOp.i64
  // CHECK: call i64 @dx.op.atomicBinOp.i64
  // CHECK: call i64 @dx.op.atomicBinOp.i64
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  InterlockedExchange( resU64[a], uv, uv2 );
  InterlockedExchange( resU64[a], uv, luv2 );
  InterlockedExchange( resU64[a], luv, uv2 );
  InterlockedExchange( resI64[a], iv, iv2 );
  InterlockedExchange( resI64[a], iv, liv2 );
  InterlockedExchange( resI64[a], liv, iv2 );
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
