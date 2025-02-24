// RUN: %dxc -T cs_6_0 -DDEST=ro_babuf -DIDX=0 %s | %FileCheck %s -check-prefixes=CHKERR
// RUN: %dxc -T cs_6_0 -DDEST=rw_babuf -DIDX=0 %s | %FileCheck %s
// RUN: %dxc -T cs_6_0 -DDEST=rw_babuf -DIDX=SC_IDX %s | %FileCheck %s
// RUN: %dxc -T cs_6_0 -DDEST=rw_babuf -DIDX=C_IDX %s | %FileCheck %s
// RUN: %dxc -T cs_6_0 -DDEST=rw_babuf -DIDX=ix %s | %FileCheck %s


// Test various Interlocked ops using read-only byteaddressbuffers and const params
// The way the dest param of atomic ops is lowered is unique and missed a lot of
// these invalid uses.

RWByteAddressBuffer rw_babuf;
ByteAddressBuffer ro_babuf;

RWStructuredBuffer<float4> output; // just something to keep the variables live

static const uint SC_IDX = 6;

uint C_IDX;

[numthreads(1,1,1)]
void main(uint ix : SV_GroupIndex) {

  int val = 1;
  uint comp = 1;
  uint orig;

  // add
  // CHKERR: error: no member named 'InterlockedAdd' in 'ByteAddressBuffer'
  // CHKERR: error: no member named 'InterlockedAdd' in 'ByteAddressBuffer'
  // CHECK: call i32 @dx.op.atomicBinOp.i32(i32 78,
  // CHECK: call i32 @dx.op.atomicBinOp.i32(i32 78,
  DEST.InterlockedAdd(IDX, val);
  DEST.InterlockedAdd(IDX, val, orig);

  // min
  // CHKERR: error: no member named 'InterlockedMin' in 'ByteAddressBuffer'
  // CHKERR: error: no member named 'InterlockedMin' in 'ByteAddressBuffer'
  // CHECK: call i32 @dx.op.atomicBinOp.i32(i32 78,
  // CHECK: call i32 @dx.op.atomicBinOp.i32(i32 78,
  DEST.InterlockedMin(IDX, val);
  DEST.InterlockedMin(IDX, val, orig);

  // max
  // CHKERR: error: no member named 'InterlockedMax' in 'ByteAddressBuffer'
  // CHKERR: error: no member named 'InterlockedMax' in 'ByteAddressBuffer'
  // CHECK: call i32 @dx.op.atomicBinOp.i32(i32 78,
  // CHECK: call i32 @dx.op.atomicBinOp.i32(i32 78,
  DEST.InterlockedMax(IDX, val);
  DEST.InterlockedMax(IDX, val, orig);

  // and
  // CHKERR: error: no member named 'InterlockedAnd' in 'ByteAddressBuffer'
  // CHKERR: error: no member named 'InterlockedAnd' in 'ByteAddressBuffer'
  // CHECK: call i32 @dx.op.atomicBinOp.i32(i32 78,
  // CHECK: call i32 @dx.op.atomicBinOp.i32(i32 78,
  DEST.InterlockedAnd(IDX, val);
  DEST.InterlockedAnd(IDX, val, orig);

  // or
  // CHKERR: error: no member named 'InterlockedOr' in 'ByteAddressBuffer'
  // CHKERR: error: no member named 'InterlockedOr' in 'ByteAddressBuffer'
  // CHECK: call i32 @dx.op.atomicBinOp.i32(i32 78,
  // CHECK: call i32 @dx.op.atomicBinOp.i32(i32 78,
  DEST.InterlockedOr(IDX, val);
  DEST.InterlockedOr(IDX, val, orig);

  // xor
  // CHKERR: error: no member named 'InterlockedXor' in 'ByteAddressBuffer'
  // CHKERR: error: no member named 'InterlockedXor' in 'ByteAddressBuffer'
  // CHECK: call i32 @dx.op.atomicBinOp.i32(i32 78,
  // CHECK: call i32 @dx.op.atomicBinOp.i32(i32 78,
  DEST.InterlockedXor(IDX, val);
  DEST.InterlockedXor(IDX, val, orig);

  // compareStore
  // CHKERR: error: no member named 'InterlockedCompareStore' in 'ByteAddressBuffer'
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32(i32 79,
  DEST.InterlockedCompareStore(IDX, comp, val);

  // exchange
  // CHKERR: error: no member named 'InterlockedExchange' in 'ByteAddressBuffer'
  // CHECK: call i32 @dx.op.atomicBinOp.i32(i32 78,
  DEST.InterlockedExchange(IDX, val, orig);

  // compareExchange
  // CHKERR: error: no member named 'InterlockedCompareExchange' in 'ByteAddressBuffer'
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32(i32 79,
  DEST.InterlockedCompareExchange(IDX, comp, val, orig);

  output[ix] = (float)DEST.Load<float4>(IDX);
}
