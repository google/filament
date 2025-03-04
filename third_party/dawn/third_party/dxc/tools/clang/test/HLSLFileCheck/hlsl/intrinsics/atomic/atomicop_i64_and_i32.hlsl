// RUN: %dxc -DINTRIN=InterlockedAdd -E CSMain -T cs_6_6 %s | FileCheck %s -check-prefix=GSCHECK
// RUN: %dxc -DINTRIN=InterlockedMin -E CSMain -T cs_6_6 %s | FileCheck %s -check-prefix=GSCHECK
// RUN: %dxc -DINTRIN=InterlockedMax -E CSMain -T cs_6_6 %s | FileCheck %s -check-prefix=GSCHECK
// RUN: %dxc -DINTRIN=InterlockedAnd -E CSMain -T cs_6_6 %s | FileCheck %s -check-prefix=GSCHECK
// RUN: %dxc -DINTRIN=InterlockedOr -E CSMain -T cs_6_6 %s | FileCheck %s -check-prefix=GSCHECK
// RUN: %dxc -DINTRIN=InterlockedXor -E CSMain -T cs_6_6 %s | FileCheck %s -check-prefix=GSCHECK

// RUN: %dxc -DMEMTYPE=RWBuffer -DINTRIN=InterlockedAdd -T ps_6_6 %s | FileCheck %s -check-prefixes=CHECK,TYCHECK
// RUN: %dxc -DMEMTYPE=RWBuffer -DINTRIN=InterlockedMin -T ps_6_6 %s | FileCheck %s -check-prefixes=CHECK,TYCHECK
// RUN: %dxc -DMEMTYPE=RWBuffer -DINTRIN=InterlockedMax -T ps_6_6 %s | FileCheck %s -check-prefixes=CHECK,TYCHECK
// RUN: %dxc -DMEMTYPE=RWBuffer -DINTRIN=InterlockedAnd -T ps_6_6 %s | FileCheck %s -check-prefixes=CHECK,TYCHECK
// RUN: %dxc -DMEMTYPE=RWBuffer -DINTRIN=InterlockedOr -T ps_6_6 %s | FileCheck %s -check-prefixes=CHECK,TYCHECK
// RUN: %dxc -DMEMTYPE=RWBuffer -DINTRIN=InterlockedXor -T ps_6_6 %s | FileCheck %s -check-prefixes=CHECK,TYCHECK

// RUN: %dxc -DMEMTYPE=RWStructuredBuffer -DINTRIN=InterlockedAdd -T ps_6_6 %s | FileCheck %s -check-prefix=CHECK
// RUN: %dxc -DMEMTYPE=RWStructuredBuffer -DINTRIN=InterlockedMin -T ps_6_6 %s | FileCheck %s -check-prefix=CHECK
// RUN: %dxc -DMEMTYPE=RWStructuredBuffer -DINTRIN=InterlockedMax -T ps_6_6 %s | FileCheck %s -check-prefix=CHECK
// RUN: %dxc -DMEMTYPE=RWStructuredBuffer -DINTRIN=InterlockedAnd -T ps_6_6 %s | FileCheck %s -check-prefix=CHECK
// RUN: %dxc -DMEMTYPE=RWStructuredBuffer -DINTRIN=InterlockedOr -T ps_6_6 %s | FileCheck %s -check-prefix=CHECK
// RUN: %dxc -DMEMTYPE=RWStructuredBuffer -DINTRIN=InterlockedXor -T ps_6_6 %s | FileCheck %s -check-prefix=CHECK

// RUN: %dxc -DMEMTYPE=RWTexture1D -DINTRIN=InterlockedAdd -T ps_6_6 %s | FileCheck %s -check-prefix=CHECK
// RUN: %dxc -DMEMTYPE=RWTexture1D -DINTRIN=InterlockedMin -T ps_6_6 %s | FileCheck %s -check-prefix=CHECK
// RUN: %dxc -DMEMTYPE=RWTexture1D -DINTRIN=InterlockedMax -T ps_6_6 %s | FileCheck %s -check-prefix=CHECK
// RUN: %dxc -DMEMTYPE=RWTexture1D -DINTRIN=InterlockedAnd -T ps_6_6 %s | FileCheck %s -check-prefix=CHECK
// RUN: %dxc -DMEMTYPE=RWTexture1D -DINTRIN=InterlockedOr -T ps_6_6 %s | FileCheck %s -check-prefix=CHECK
// RUN: %dxc -DMEMTYPE=RWTexture1D -DINTRIN=InterlockedXor -T ps_6_6 %s | FileCheck %s -check-prefix=CHECK

// RUN: %dxilver 1.6 | %dxc -DINTRIN=InterlockedAdd -E CSMain -T cs_6_5 %s | FileCheck %s -check-prefix=ERRCHECK
// RUN: %dxilver 1.6 | %dxc -DINTRIN=InterlockedMin -E CSMain -T cs_6_5 %s | FileCheck %s -check-prefix=ERRCHECK
// RUN: %dxilver 1.6 | %dxc -DINTRIN=InterlockedMax -E CSMain -T cs_6_5 %s | FileCheck %s -check-prefix=ERRCHECK
// RUN: %dxilver 1.6 | %dxc -DINTRIN=InterlockedAnd -E CSMain -T cs_6_5 %s | FileCheck %s -check-prefix=ERRCHECK
// RUN: %dxilver 1.6 | %dxc -DINTRIN=InterlockedOr -E CSMain -T cs_6_5 %s | FileCheck %s -check-prefix=ERRCHECK
// RUN: %dxilver 1.6 | %dxc -DINTRIN=InterlockedXor -E CSMain -T cs_6_5 %s | FileCheck %s -check-prefix=ERRCHECK

// RUN: %dxilver 1.6 | %dxc -DMEMTYPE=RWBuffer -DINTRIN=InterlockedAdd -T ps_6_5 %s | FileCheck %s -check-prefix=ERRCHECK
// RUN: %dxilver 1.6 | %dxc -DMEMTYPE=RWBuffer -DINTRIN=InterlockedMin -T ps_6_5 %s | FileCheck %s -check-prefix=ERRCHECK
// RUN: %dxilver 1.6 | %dxc -DMEMTYPE=RWBuffer -DINTRIN=InterlockedMax -T ps_6_5 %s | FileCheck %s -check-prefix=ERRCHECK
// RUN: %dxilver 1.6 | %dxc -DMEMTYPE=RWBuffer -DINTRIN=InterlockedAnd -T ps_6_5 %s | FileCheck %s -check-prefix=ERRCHECK
// RUN: %dxilver 1.6 | %dxc -DMEMTYPE=RWBuffer -DINTRIN=InterlockedOr -T ps_6_5 %s | FileCheck %s -check-prefix=ERRCHECK
// RUN: %dxilver 1.6 | %dxc -DMEMTYPE=RWBuffer -DINTRIN=InterlockedXor -T ps_6_5 %s | FileCheck %s -check-prefix=ERRCHECK

// RUN: %dxilver 1.6 | %dxc -DMEMTYPE=RWStructuredBuffer -DINTRIN=InterlockedAdd -T ps_6_5 %s | FileCheck %s -check-prefix=ERRCHECK
// RUN: %dxilver 1.6 | %dxc -DMEMTYPE=RWStructuredBuffer -DINTRIN=InterlockedMin -T ps_6_5 %s | FileCheck %s -check-prefix=ERRCHECK
// RUN: %dxilver 1.6 | %dxc -DMEMTYPE=RWStructuredBuffer -DINTRIN=InterlockedMax -T ps_6_5 %s | FileCheck %s -check-prefix=ERRCHECK
// RUN: %dxilver 1.6 | %dxc -DMEMTYPE=RWStructuredBuffer -DINTRIN=InterlockedAnd -T ps_6_5 %s | FileCheck %s -check-prefix=ERRCHECK
// RUN: %dxilver 1.6 | %dxc -DMEMTYPE=RWStructuredBuffer -DINTRIN=InterlockedOr -T ps_6_5 %s | FileCheck %s -check-prefix=ERRCHECK
// RUN: %dxilver 1.6 | %dxc -DMEMTYPE=RWStructuredBuffer -DINTRIN=InterlockedXor -T ps_6_5 %s | FileCheck %s -check-prefix=ERRCHECK

// RUN: %dxilver 1.6 | %dxc -DMEMTYPE=RWTexture1D -DINTRIN=InterlockedAdd -T ps_6_5 %s | FileCheck %s -check-prefix=ERRCHECK
// RUN: %dxilver 1.6 | %dxc -DMEMTYPE=RWTexture1D -DINTRIN=InterlockedMin -T ps_6_5 %s | FileCheck %s -check-prefix=ERRCHECK
// RUN: %dxilver 1.6 | %dxc -DMEMTYPE=RWTexture1D -DINTRIN=InterlockedMax -T ps_6_5 %s | FileCheck %s -check-prefix=ERRCHECK
// RUN: %dxilver 1.6 | %dxc -DMEMTYPE=RWTexture1D -DINTRIN=InterlockedAnd -T ps_6_5 %s | FileCheck %s -check-prefix=ERRCHECK
// RUN: %dxilver 1.6 | %dxc -DMEMTYPE=RWTexture1D -DINTRIN=InterlockedOr -T ps_6_5 %s | FileCheck %s -check-prefix=ERRCHECK
// RUN: %dxilver 1.6 | %dxc -DMEMTYPE=RWTexture1D -DINTRIN=InterlockedXor -T ps_6_5 %s | FileCheck %s -check-prefix=ERRCHECK

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
  int iv = b / c;
  uint64_t bb = b;
  uint64_t cc = c;
  uint64_t luv = bb * cc;
  int64_t liv = bb + cc;

  // GSCHECK: atomicrmw {{[a-z]*}} i32
  // GSCHECK: atomicrmw {{[a-z]*}} i32
  // GSCHECK: atomicrmw {{[a-z]*}} i64
  // GSCHECK: atomicrmw {{[a-z]*}} i64
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i64 @dx.op.atomicBinOp.i64
  // CHECK: call i64 @dx.op.atomicBinOp.i64
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  INTRIN( resU[a], uv );
  INTRIN( resI[a], iv );
  INTRIN( resU64[a], luv );
  INTRIN( resI64[a], liv );

  // GSCHECK: atomicrmw {{[a-z]*}} i32
  // GSCHECK: atomicrmw {{[a-z]*}} i32
  // GSCHECK: atomicrmw {{[a-z]*}} i64
  // GSCHECK: atomicrmw {{[a-z]*}} i64
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i64 @dx.op.atomicBinOp.i64
  // CHECK: call i64 @dx.op.atomicBinOp.i64
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  INTRIN( resU[a], iv );
  INTRIN( resI[a], uv );
  INTRIN( resU64[a], liv );
  INTRIN( resI64[a], luv );

  // GSCHECK: atomicrmw {{[a-z]*}} i32
  // GSCHECK: atomicrmw {{[a-z]*}} i32
  // GSCHECK: atomicrmw {{[a-z]*}} i64
  // GSCHECK: atomicrmw {{[a-z]*}} i64
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i64 @dx.op.atomicBinOp.i64
  // CHECK: call i64 @dx.op.atomicBinOp.i64
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  INTRIN( resU[a], luv );
  INTRIN( resI[a], liv );
  INTRIN( resU64[a], uv );
  INTRIN( resI64[a], iv );

  // GSCHECK: atomicrmw {{[a-z]*}} i32
  // GSCHECK: atomicrmw {{[a-z]*}} i32
  // GSCHECK: atomicrmw {{[a-z]*}} i64
  // GSCHECK: atomicrmw {{[a-z]*}} i64
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i64 @dx.op.atomicBinOp.i64
  // CHECK: call i64 @dx.op.atomicBinOp.i64
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  INTRIN( resU[a], 1.0 );
  INTRIN( resI[a], 2.0 );
  INTRIN( resU64[a], 3.0 );
  INTRIN( resI64[a], 4.0 );
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
