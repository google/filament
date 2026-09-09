// RUN: %dxc -T cs_6_6 %s | FileCheck %s

// A test to verify that 64-bit atomic binary operation intrinsics select the right variant

groupshared int64_t gs[256];
RWBuffer<int64_t> tb;
RWStructuredBuffer<int64_t> sb;
RWByteAddressBuffer rb;
RWTexture1D<int64_t> tex1;
RWTexture2D<int64_t> tex2;
RWTexture3D<int64_t> tex3;

groupshared uint64_t ugs[256];
RWBuffer<uint64_t> utb;
RWStructuredBuffer<uint64_t> usb;
RWTexture1D<uint64_t> utex1;
RWTexture2D<uint64_t> utex2;
RWTexture3D<uint64_t> utex3;

[numthreads(1,1,1)]
void main( uint3 gtid : SV_GroupThreadID)
{
  uint a = gtid.x;
  uint b = gtid.y;
  uint64_t luv = a * b;
  int64_t liv = a + b;
  int64_t liv2 = 0, liv3 = 0;
  uint ix = 0;

  // CHECK: atomicrmw add i64
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 0
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 0
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 0
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 0
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 0
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 0
  InterlockedAdd( gs[a], liv );
  InterlockedAdd( tb[a], liv );
  InterlockedAdd( sb[a], liv );
  InterlockedAdd( tex1[a], liv );
  InterlockedAdd( tex2[gtid.xy], liv );
  InterlockedAdd( tex3[gtid], liv );
  rb.InterlockedAdd64( ix++, liv );

  // CHECK: atomicrmw and i64
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 1
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 1
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 1
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 1
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 1
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 1
  InterlockedAnd( gs[a], liv );
  InterlockedAnd( tb[a], liv );
  InterlockedAnd( sb[a], liv );
  InterlockedAnd( tex1[a], liv );
  InterlockedAnd( tex2[gtid.xy], liv );
  InterlockedAnd( tex3[gtid], liv );
  rb.InterlockedAnd64( ix++, liv );

  // CHECK: atomicrmw or i64
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 2
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 2
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 2
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 2
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 2
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 2
  InterlockedOr( gs[a], liv );
  InterlockedOr( tb[a], liv );
  InterlockedOr( sb[a], liv );
  InterlockedOr( tex1[a], liv );
  InterlockedOr( tex2[gtid.xy], liv );
  InterlockedOr( tex3[gtid], liv );
  rb.InterlockedOr64( ix++, liv );

  // CHECK: atomicrmw xor i64
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 3
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 3
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 3
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 3
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 3
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 3
  InterlockedXor( gs[a], liv );
  InterlockedXor( tb[a], liv );
  InterlockedXor( sb[a], liv );
  InterlockedXor( tex1[a], liv );
  InterlockedXor( tex2[gtid.xy], liv );
  InterlockedXor( tex3[gtid], liv );
  rb.InterlockedXor64( ix++, liv );

  // CHECK: atomicrmw min i64
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 4
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 4
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 4
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 4
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 4
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 4
  InterlockedMin( gs[a], liv );
  InterlockedMin( tb[a], liv );
  InterlockedMin( sb[a], liv );
  InterlockedMin( tex1[a], liv );
  InterlockedMin( tex2[gtid.xy], liv );
  InterlockedMin( tex3[gtid], liv );
  rb.InterlockedMin64( ix++, liv );

  // CHECK: atomicrmw max i64
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 5
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 5
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 5
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 5
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 5
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 5
  InterlockedMax( gs[a], liv );
  InterlockedMax( tb[a], liv );
  InterlockedMax( sb[a], liv );
  InterlockedMax( tex1[a], liv );
  InterlockedMax( tex2[gtid.xy], liv );
  InterlockedMax( tex3[gtid], liv );
  rb.InterlockedMax64( ix++, liv );

  // CHECK: atomicrmw umin i64
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 6
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 6
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 6
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 6
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 6
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 6
  InterlockedMin( ugs[a], luv );
  InterlockedMin( utb[a], luv );
  InterlockedMin( usb[a], luv );
  InterlockedMin( utex1[a], liv );
  InterlockedMin( utex2[gtid.xy], liv );
  InterlockedMin( utex3[gtid], liv );
  rb.InterlockedMin64( ix++, luv );

  // CHECK: atomicrmw umax i64
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 7
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 7
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 7
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 7
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 7
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 7
  InterlockedMax( ugs[a], luv );
  InterlockedMax( utb[a], luv );
  InterlockedMax( usb[a], luv );
  InterlockedMax( utex1[a], liv );
  InterlockedMax( utex2[gtid.xy], liv );
  InterlockedMax( utex3[gtid], liv );
  rb.InterlockedMax64( ix++, luv );

  // CHECK: atomicrmw xchg i64
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 8
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 8
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 8
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 8
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 8
  // CHECK: call i64 @dx.op.atomicBinOp.i64(i32 78, %dx.types.Handle %{{[0-9]*}}, i32 8
  InterlockedExchange( gs[a], liv, liv2 );
  InterlockedExchange( tb[a], liv2, liv );
  InterlockedExchange( sb[a], liv, liv2 );
  InterlockedExchange( tex1[a], liv2, liv );
  InterlockedExchange( tex2[gtid.xy], liv, liv2 );
  InterlockedExchange( tex3[gtid], liv2, liv );
  rb.InterlockedExchange64( ix++, liv, liv2 );

  // CHECK: cmpxchg i64
  // CHECK: call i64 @dx.op.atomicCompareExchange.i64(i32 79
  // CHECK: call i64 @dx.op.atomicCompareExchange.i64(i32 79
  // CHECK: call i64 @dx.op.atomicCompareExchange.i64(i32 79
  // CHECK: call i64 @dx.op.atomicCompareExchange.i64(i32 79
  // CHECK: call i64 @dx.op.atomicCompareExchange.i64(i32 79
  // CHECK: call i64 @dx.op.atomicCompareExchange.i64(i32 79
  InterlockedCompareStore( gs[a], liv, liv2 );
  InterlockedCompareStore( tb[a], liv2, liv );
  InterlockedCompareStore( sb[a], liv, liv2 );
  InterlockedCompareStore( tex1[a], liv2, liv );
  InterlockedCompareStore( tex2[gtid.xy], liv2, liv );
  InterlockedCompareStore( tex3[gtid], liv, liv2 );
  rb.InterlockedCompareStore64( ix++, liv2, liv );

  // CHECK: cmpxchg i64
  // CHECK: call i64 @dx.op.atomicCompareExchange.i64(i32 79
  // CHECK: call i64 @dx.op.atomicCompareExchange.i64(i32 79
  // CHECK: call i64 @dx.op.atomicCompareExchange.i64(i32 79
  // CHECK: call i64 @dx.op.atomicCompareExchange.i64(i32 79
  // CHECK: call i64 @dx.op.atomicCompareExchange.i64(i32 79
  // CHECK: call i64 @dx.op.atomicCompareExchange.i64(i32 79
  InterlockedCompareExchange( gs[a], liv, liv2, liv3 );
  InterlockedCompareExchange( tb[a], liv2, liv3, liv );
  InterlockedCompareExchange( sb[a], liv2, liv, liv3 );
  InterlockedCompareExchange( tex1[a], liv2, liv3, liv );
  InterlockedCompareExchange( tex2[gtid.xy], liv2, liv, liv3 );
  InterlockedCompareExchange( tex3[gtid], liv, liv2, liv3 );
  rb.InterlockedCompareExchange64( ix++, liv2, liv3, liv );
}
