// RUN: %dxc -T ps_6_6 %s | FileCheck %s -check-prefix=CHECK

// Verify that the second and third args determine the overload and the others can be what they will
// When either of these is not int64, fallback to the old overload with its casts

RWByteAddressBuffer res;

void main( uint a : A, uint b: B, uint c :C) : SV_Target
{
  uint uv = b - c;
  uint uv2 = b + c;
  uint ouv = 0;
  int iv = b / c;
  int iv2 = b * c;
  int oiv = 0;
  uint64_t bb = b;
  uint64_t cc = c;
  uint64_t luv = bb * cc;
  uint64_t luv2 = bb / cc;
  uint64_t oluv = 0;
  int64_t liv = bb + cc;
  int64_t liv2 = bb - cc;
  int64_t oliv = 0;
  uint ix = 0;

  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: call i64 @dx.op.atomicCompareExchange.i64
  // CHECK: call i64 @dx.op.atomicCompareExchange.i64
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  res.InterlockedCompareExchange( ix++, uv, uv2, ouv );
  res.InterlockedCompareExchange( ix++, iv, iv2, oiv );
  res.InterlockedCompareExchange64( ix++, luv, luv2, ouv );
  res.InterlockedCompareExchange64( ix++, liv, liv2, oiv );

  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  res.InterlockedCompareExchange( ix++, iv, iv2, oiv );
  res.InterlockedCompareExchange( ix++, iv, uv2, ouv );
  res.InterlockedCompareExchange( ix++, uv, iv2, oiv );
  res.InterlockedCompareExchange( ix++, uv, uv2, ouv );
  res.InterlockedCompareExchange( ix++, uv, iv2, oiv );
  res.InterlockedCompareExchange( ix++, iv, uv2, ouv );

  // test some literals with 32-bit overloads
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  res.InterlockedCompareExchange( ix++, 1.0, 2.0, oiv );
  res.InterlockedCompareExchange( ix++, 1.0, uv2, ouv );
  res.InterlockedCompareExchange( ix++, uv, 2.0, oiv );

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
  res.InterlockedCompareExchange64( ix++, liv, liv2, oliv );
  res.InterlockedCompareExchange64( ix++, liv, luv2, oluv );
  res.InterlockedCompareExchange64( ix++, luv, liv2, oliv );
  res.InterlockedCompareExchange64( ix++, luv, luv2, oluv );
  res.InterlockedCompareExchange64( ix++, luv, liv2, oliv );
  res.InterlockedCompareExchange64( ix++, liv, luv2, oluv );

  // Test some literals with 64-bit overloads
  // CHECK: call i64 @dx.op.atomicCompareExchange.i64
  // CHECK: call i64 @dx.op.atomicCompareExchange.i64
  // CHECK: call i64 @dx.op.atomicCompareExchange.i64
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // ERRCHECK: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  res.InterlockedCompareExchange64( ix++, 1.0, 2.0, oliv );
  res.InterlockedCompareExchange64( ix++, liv, 2.0, oluv );
  res.InterlockedCompareExchange64( ix++, 1.0, liv2, oliv );

  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  res.InterlockedCompareExchange( ix++, luv, luv2, ouv );
  res.InterlockedCompareExchange( ix++, luv, uv2, ouv );
  res.InterlockedCompareExchange( ix++, uv, luv2, ouv );
  res.InterlockedCompareExchange( ix++, uv, uv2, oluv );
  res.InterlockedCompareExchange( ix++, liv, liv2, oiv );
  res.InterlockedCompareExchange( ix++, liv, iv2, oiv );
  res.InterlockedCompareExchange( ix++, iv, liv2, oiv );
  res.InterlockedCompareExchange( ix++, iv, iv2, oliv );

  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  res.InterlockedCompareExchange( ix++, uv, uv2, oluv );
  res.InterlockedCompareExchange( ix++, uv, luv2, oluv );
  res.InterlockedCompareExchange( ix++, luv, uv2, oluv );
  res.InterlockedCompareExchange( ix++, luv, luv2, ouv );
  res.InterlockedCompareExchange( ix++, iv, iv2, oliv );
  res.InterlockedCompareExchange( ix++, iv, liv2, oliv );
  res.InterlockedCompareExchange( ix++, liv, iv2, oliv );
  res.InterlockedCompareExchange( ix++, liv, liv2, oiv );

  // CHECK: call i64 @dx.op.atomicCompareExchange.i64
  // CHECK: call i64 @dx.op.atomicCompareExchange.i64
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
  res.InterlockedCompareExchange64( ix++, luv, luv2, ouv );
  res.InterlockedCompareExchange64( ix++, luv, uv2, ouv );
  res.InterlockedCompareExchange64( ix++, uv, luv2, ouv );
  res.InterlockedCompareExchange64( ix++, uv, uv2, oluv );
  res.InterlockedCompareExchange64( ix++, liv, liv2, oiv );
  res.InterlockedCompareExchange64( ix++, liv, iv2, oiv );
  res.InterlockedCompareExchange64( ix++, iv, liv2, oiv );
  res.InterlockedCompareExchange64( ix++, iv, iv2, oliv );

  // CHECK: call i64 @dx.op.atomicCompareExchange.i64
  // CHECK: call i64 @dx.op.atomicCompareExchange.i64
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
  res.InterlockedCompareExchange64( ix++, uv, uv2, oluv );
  res.InterlockedCompareExchange64( ix++, uv, luv2, oluv );
  res.InterlockedCompareExchange64( ix++, luv, uv2, oluv );
  res.InterlockedCompareExchange64( ix++, luv, luv2, ouv );
  res.InterlockedCompareExchange64( ix++, iv, iv2, oliv );
  res.InterlockedCompareExchange64( ix++, iv, liv2, oliv );
  res.InterlockedCompareExchange64( ix++, liv, iv2, oliv );
  res.InterlockedCompareExchange64( ix++, liv, liv2, oiv );
}
