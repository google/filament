// RUN: %dxc -HV 2021 -T cs_6_0 -DOP=InterlockedOr           -DDEST=Str %s | FileCheck %s -check-prefix=CHKBIN
// RUN: %dxc -HV 2021 -T cs_6_0 -DOP=InterlockedCompareStore -DDEST=Str %s | FileCheck %s -check-prefix=CHKXCH
// RUN: %dxc -HV 2021 -T cs_6_0 -DOP=InterlockedOr           -DDEST=Gs  %s | FileCheck %s -check-prefix=CHKGSB
// RUN: %dxc -HV 2021 -T cs_6_0 -DOP=InterlockedCompareStore -DDEST=Gs  %s | FileCheck %s -check-prefix=CHKGSX
// RUN: %dxc -HV 2021 -T cs_6_0 -DOP=InterlockedOr           -DDEST=Buf %s | FileCheck %s -check-prefix=CHKERR
// RUN: %dxc -HV 2021 -T cs_6_0 -DOP=InterlockedCompareStore -DDEST=Buf %s | FileCheck %s -check-prefix=CHKERR

// Ensure that atomic operations fail when used with struct members of a typed resource
// The only typed resource that can use structs is RWBuffer
// Use a binary op and an exchange op because they use different code paths

struct Simple {
  uint i;
  uint longEnding[1];
};

struct Complex {
  Simple s;
  uint theEnd;
};

struct TexCoords {
  uint s, t, r, q;
};

#define _PASTE(pre, res) pre##res
#define PASTE(pre, res) _PASTE(pre, res)

RWBuffer<TexCoords> Buf;
RWBuffer<Simple> simpBuf;
RWBuffer<Complex> cplxBuf;

RWStructuredBuffer<TexCoords> Str;
RWStructuredBuffer<Simple> simpStr;
RWStructuredBuffer<Complex> cplxStr;

groupshared TexCoords Gs[1];
groupshared Simple simpGs[1];
groupshared Complex cplxGs[1];

[numthreads(8,8,1)]
void main( uint3 gtid : SV_GroupThreadID , uint gid : SV_GroupIndex)
{
  uint orig = 0;
  uint a = gid;
  uint b = gtid.x;
  uint c = gtid.y;
  uint d = gtid.z;

  // CHKBIN: call i32 @dx.op.atomicBinOp.i32(i32 78
  // CHKBIN: call i32 @dx.op.atomicBinOp.i32(i32 78
  // CHKBIN: call i32 @dx.op.atomicBinOp.i32(i32 78
  // CHKBIN: call i32 @dx.op.atomicBinOp.i32(i32 78

  // CHKXCH: call i32 @dx.op.atomicCompareExchange.i32(i32 79,
  // CHKXCH: call i32 @dx.op.atomicCompareExchange.i32(i32 79,
  // CHKXCH: call i32 @dx.op.atomicCompareExchange.i32(i32 79,
  // CHKXCH: call i32 @dx.op.atomicCompareExchange.i32(i32 79,

  // CHKGSB: atomicrmw or i32 addrspace(3)
  // CHKGSB: atomicrmw or i32 addrspace(3)
  // CHKGSB: atomicrmw or i32 addrspace(3)
  // CHKGSB: atomicrmw or i32 addrspace(3)

  // CHKGSX: cmpxchg i32 addrspace(3)
  // CHKGSX: cmpxchg i32 addrspace(3)
  // CHKGSX: cmpxchg i32 addrspace(3)
  // CHKGSX: cmpxchg i32 addrspace(3)

  // CHKERR: error: Typed resources used in atomic operations must have a scalar element type.
  // CHKERR: error: Typed resources used in atomic operations must have a scalar element type.
  // CHKERR: error: Typed resources used in atomic operations must have a scalar element type.
  // CHKERR: error: Typed resources used in atomic operations must have a scalar element type.

  OP( PASTE( ,DEST)[a].q, 2, orig );
  OP( PASTE(simp, DEST)[a].i, 2, orig );
  OP( PASTE(cplx, DEST)[a].s.i, 2, orig );
  OP( PASTE(cplx, DEST)[a].s.longEnding[d].x, 2, orig );
}
