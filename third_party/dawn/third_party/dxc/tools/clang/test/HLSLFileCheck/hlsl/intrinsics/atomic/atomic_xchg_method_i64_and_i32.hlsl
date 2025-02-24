// RUN: %dxc -T ps_6_6 %s | FileCheck %s -check-prefix=CHECK

// RUN: %dxilver 1.6 | %dxc -T ps_6_5 %s | FileCheck %s -check-prefix=ERRCHECK

// Verify that the second arg determines the overload and the others can be what they will

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

  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  res.InterlockedExchange( ix++, iv, iv2 );
  res.InterlockedExchange( ix++, iv, uv2 );
  res.InterlockedExchange( ix++, uv, iv2 );
  res.InterlockedExchange( ix++, uv, uv2 );
  res.InterlockedExchange( ix++, uv, iv2 );
  res.InterlockedExchange( ix++, iv, uv2 );

  // Test some literals with 32-bit overloads
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  res.InterlockedExchange( ix++, 1.0, iv2 );
  res.InterlockedExchange( ix++, 2.0, uv2 );

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
  res.InterlockedExchange64( ix++, liv, liv2 );
  res.InterlockedExchange64( ix++, liv, luv2 );
  res.InterlockedExchange64( ix++, luv, liv2 );
  res.InterlockedExchange64( ix++, luv, luv2 );
  res.InterlockedExchange64( ix++, luv, liv2 );
  res.InterlockedExchange64( ix++, liv, luv2 );

  // Test some literals with 64-bit overloads
  // CHECK: call i64 @dx.op.atomicBinOp.i64
  // CHECK: call i64 @dx.op.atomicBinOp.i64
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  res.InterlockedExchange64( ix++, 1.0, liv2 );
  res.InterlockedExchange64( ix++, 2.0, luv2 );

  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  res.InterlockedExchange( ix++, luv, luv2 );
  res.InterlockedExchange( ix++, luv, uv2 );
  res.InterlockedExchange( ix++, uv, luv2 );
  res.InterlockedExchange( ix++, liv, liv2 );
  res.InterlockedExchange( ix++, liv, iv2 );
  res.InterlockedExchange( ix++, iv, liv2 );

  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  res.InterlockedExchange( ix++, uv, uv2 );
  res.InterlockedExchange( ix++, uv, luv2 );
  res.InterlockedExchange( ix++, luv, uv2 );
  res.InterlockedExchange( ix++, iv, iv2 );
  res.InterlockedExchange( ix++, iv, liv2 );
  res.InterlockedExchange( ix++, liv, iv2 );

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
  res.InterlockedExchange64( ix++, luv, luv2 );
  res.InterlockedExchange64( ix++, luv, uv2 );
  res.InterlockedExchange64( ix++, uv, luv2 );
  res.InterlockedExchange64( ix++, liv, liv2 );
  res.InterlockedExchange64( ix++, liv, iv2 );
  res.InterlockedExchange64( ix++, iv, liv2 );

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
  res.InterlockedExchange64( ix++, uv, uv2 );
  res.InterlockedExchange64( ix++, uv, luv2 );
  res.InterlockedExchange64( ix++, luv, uv2 );
  res.InterlockedExchange64( ix++, iv, iv2 );
  res.InterlockedExchange64( ix++, iv, liv2 );
  res.InterlockedExchange64( ix++, liv, iv2 );
}
