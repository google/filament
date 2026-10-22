// RUN: %dxc -DINTRIN=InterlockedAdd -T ps_6_6 %s | FileCheck %s -check-prefix=CHECK32
// RUN: %dxc -DINTRIN=InterlockedMin -T ps_6_6 %s | FileCheck %s -check-prefix=CHECK32
// RUN: %dxc -DINTRIN=InterlockedMax -T ps_6_6 %s | FileCheck %s -check-prefix=CHECK32
// RUN: %dxc -DINTRIN=InterlockedAnd -T ps_6_6 %s | FileCheck %s -check-prefix=CHECK32
// RUN: %dxc -DINTRIN=InterlockedOr -T ps_6_6 %s | FileCheck %s -check-prefix=CHECK32
// RUN: %dxc -DINTRIN=InterlockedXor -T ps_6_6 %s | FileCheck %s -check-prefix=CHECK32

// RUN: %dxc -DINTRIN=InterlockedAdd64 -T ps_6_6 %s | FileCheck %s -check-prefix=CHECK64
// RUN: %dxc -DINTRIN=InterlockedMin64 -T ps_6_6 %s | FileCheck %s -check-prefix=CHECK64
// RUN: %dxc -DINTRIN=InterlockedMax64 -T ps_6_6 %s | FileCheck %s -check-prefix=CHECK64
// RUN: %dxc -DINTRIN=InterlockedAnd64 -T ps_6_6 %s | FileCheck %s -check-prefix=CHECK64
// RUN: %dxc -DINTRIN=InterlockedOr64 -T ps_6_6 %s | FileCheck %s -check-prefix=CHECK64
// RUN: %dxc -DINTRIN=InterlockedXor64 -T ps_6_6 %s | FileCheck %s -check-prefix=CHECK64

// RUN: %dxilver 1.6 | %dxc -DINTRIN=InterlockedAdd64 -T ps_6_5 %s | FileCheck %s -check-prefix=ERRCHECK
// RUN: %dxilver 1.6 | %dxc -DINTRIN=InterlockedMin64 -T ps_6_5 %s | FileCheck %s -check-prefix=ERRCHECK
// RUN: %dxilver 1.6 | %dxc -DINTRIN=InterlockedMax64 -T ps_6_5 %s | FileCheck %s -check-prefix=ERRCHECK
// RUN: %dxilver 1.6 | %dxc -DINTRIN=InterlockedAnd64 -T ps_6_5 %s | FileCheck %s -check-prefix=ERRCHECK
// RUN: %dxilver 1.6 | %dxc -DINTRIN=InterlockedOr64 -T ps_6_5 %s | FileCheck %s -check-prefix=ERRCHECK
// RUN: %dxilver 1.6 | %dxc -DINTRIN=InterlockedXor64 -T ps_6_5 %s | FileCheck %s -check-prefix=ERRCHECK

// Verify that the first arg determines the overload and the others can be what they will

RWByteAddressBuffer res;

void main( uint a : A, uint b: B, uint c :C) : SV_Target
{
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
  uint ix = 0;

  // CHECK32: call i32 @dx.op.atomicBinOp.i32
  // CHECK32: call i32 @dx.op.atomicBinOp.i32
  // CHECK32: call i32 @dx.op.atomicBinOp.i32
  // CHECK32: call i32 @dx.op.atomicBinOp.i32
  // CHECK64: call i64 @dx.op.atomicBinOp.i64
  // CHECK64: call i64 @dx.op.atomicBinOp.i64
  // CHECK64: call i64 @dx.op.atomicBinOp.i64
  // CHECK64: call i64 @dx.op.atomicBinOp.i64
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  res.INTRIN( ix++, uv, uv2 );
  res.INTRIN( ix++, iv, iv2 );
  res.INTRIN( ix++, luv, luv2 );
  res.INTRIN( ix++, liv, liv2 );

  // CHECK32: call i32 @dx.op.atomicBinOp.i32
  // CHECK32: call i32 @dx.op.atomicBinOp.i32
  // CHECK32: call i32 @dx.op.atomicBinOp.i32
  // CHECK32: call i32 @dx.op.atomicBinOp.i32
  // CHECK64: call i64 @dx.op.atomicBinOp.i64
  // CHECK64: call i64 @dx.op.atomicBinOp.i64
  // CHECK64: call i64 @dx.op.atomicBinOp.i64
  // CHECK64: call i64 @dx.op.atomicBinOp.i64
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  res.INTRIN( ix++, iv, uv2 );
  res.INTRIN( ix++, uv, iv2 );
  res.INTRIN( ix++, liv, luv2 );
  res.INTRIN( ix++, luv, liv2 );

  // CHECK32: call i32 @dx.op.atomicBinOp.i32
  // CHECK32: call i32 @dx.op.atomicBinOp.i32
  // CHECK32: call i32 @dx.op.atomicBinOp.i32
  // CHECK32: call i32 @dx.op.atomicBinOp.i32
  // CHECK64: call i64 @dx.op.atomicBinOp.i64
  // CHECK64: call i64 @dx.op.atomicBinOp.i64
  // CHECK64: call i64 @dx.op.atomicBinOp.i64
  // CHECK64: call i64 @dx.op.atomicBinOp.i64
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  res.INTRIN( ix++, uv, luv2 );
  res.INTRIN( ix++, iv, liv2 );
  res.INTRIN( ix++, luv, uv2 );
  res.INTRIN( ix++, liv, iv2 );

  // CHECK32: call i32 @dx.op.atomicBinOp.i32
  // CHECK32: call i32 @dx.op.atomicBinOp.i32
  // CHECK32: call i32 @dx.op.atomicBinOp.i32
  // CHECK32: call i32 @dx.op.atomicBinOp.i32
  // CHECK64: call i64 @dx.op.atomicBinOp.i64
  // CHECK64: call i64 @dx.op.atomicBinOp.i64
  // CHECK64: call i64 @dx.op.atomicBinOp.i64
  // CHECK64: call i64 @dx.op.atomicBinOp.i64
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  res.INTRIN( ix++, 1.0, luv2 );
  res.INTRIN( ix++, 2.0, liv2 );
  res.INTRIN( ix++, 3.0, uv2 );
  res.INTRIN( ix++, 4.0, iv2 );
}
