// RUN: %dxc -T ps_6_6 %s | FileCheck %s

// Test structured buffers with more complicated struct members and 64-bit atomics

// A simple structure with 64-bit integer in the middle of two other members
struct simple {
  bool thisVariableIsFalse;
  uint64_t i;
  uint64_t3x1 longEnding[4];
};

struct complex {
  double4 d;
  simple s;
  int64_t i;
  simple ss[3];
  float2 theEnd;
};

RWStructuredBuffer<simple> simpBuf;
RWStructuredBuffer<simple[3]> simpArrBuf;
RWStructuredBuffer<complex> cplxBuf;
RWStructuredBuffer<complex[3]> cplxArrBuf;

void main( uint a : A, uint b: B, uint c :C, uint d :D) : SV_Target
{
  int64_t liv = a + b;
  int64_t liv2 = 0, liv3 = 0;
  uint64_t loc_arr[4];

  // CHECK: call i64 @dx.op.atomicBinOp.i64
  // CHECK: call i64 @dx.op.atomicBinOp.i64
  // CHECK: call i64 @dx.op.atomicBinOp.i64
  // CHECK: call i64 @dx.op.atomicBinOp.i64
  // CHECK: call i64 @dx.op.atomicBinOp.i64
  // CHECK: call i64 @dx.op.atomicBinOp.i64
  InterlockedAdd( simpBuf[a].i, liv );
  InterlockedAdd( simpArrBuf[a][b].i, liv );
  InterlockedAdd( cplxBuf[a].i, liv );
  InterlockedAdd( cplxBuf[a].s.i, liv );
  InterlockedAdd( cplxBuf[a].ss[b].i, liv );
  InterlockedAdd( cplxBuf[a].ss[b].longEnding[c][d].x, liv);

  // CHECK: call i64 @dx.op.atomicBinOp.i64
  // CHECK: call i64 @dx.op.atomicBinOp.i64
  // CHECK: call i64 @dx.op.atomicBinOp.i64
  // CHECK: call i64 @dx.op.atomicBinOp.i64
  // CHECK: call i64 @dx.op.atomicBinOp.i64
  // CHECK: call i64 @dx.op.atomicBinOp.i64
  InterlockedExchange( simpBuf[a].i, liv, liv2 );
  InterlockedExchange( simpArrBuf[a][b].i, liv2, liv );
  InterlockedExchange( cplxBuf[a].i, liv, liv2 );
  InterlockedExchange( cplxBuf[a].s.i, liv2, liv );
  InterlockedExchange( cplxBuf[a].ss[b].i, liv, liv2 );
  InterlockedExchange( cplxBuf[a].ss[b].longEnding[c][d].x, liv, liv2);

  // CHECK: call i64 @dx.op.atomicCompareExchange.i64
  // CHECK: call i64 @dx.op.atomicCompareExchange.i64
  // CHECK: call i64 @dx.op.atomicCompareExchange.i64
  // CHECK: call i64 @dx.op.atomicCompareExchange.i64
  // CHECK: call i64 @dx.op.atomicCompareExchange.i64
  // CHECK: call i64 @dx.op.atomicCompareExchange.i64
  InterlockedCompareStore( simpBuf[a].i, liv, liv2 );
  InterlockedCompareStore( simpArrBuf[a][b].i, liv2, liv );
  InterlockedCompareStore( cplxBuf[a].i, liv, liv2 );
  InterlockedCompareStore( cplxBuf[a].s.i, liv2, liv );
  InterlockedCompareStore( cplxBuf[a].ss[b].i, liv, liv2 );
  InterlockedCompareStore( cplxBuf[a].ss[b].longEnding[c][d].x, liv2, liv);

  // CHECK: call i64 @dx.op.atomicCompareExchange.i64
  // CHECK: call i64 @dx.op.atomicCompareExchange.i64
  // CHECK: call i64 @dx.op.atomicCompareExchange.i64
  // CHECK: call i64 @dx.op.atomicCompareExchange.i64
  // CHECK: call i64 @dx.op.atomicCompareExchange.i64
  // CHECK: call i64 @dx.op.atomicCompareExchange.i64
  InterlockedCompareExchange( simpBuf[a].i, liv, liv2, liv3 );
  InterlockedCompareExchange( simpArrBuf[a][b].i, liv2, liv3, liv );
  InterlockedCompareExchange( cplxBuf[a].i, liv3, liv2, liv );
  InterlockedCompareExchange( cplxBuf[a].s.i, liv2, liv, liv3 );
  InterlockedCompareExchange( cplxBuf[a].ss[b].i, liv2, liv3, liv );
  InterlockedCompareExchange( cplxBuf[a].ss[b].longEnding[c][d].x, liv3, liv, liv2 );
}
