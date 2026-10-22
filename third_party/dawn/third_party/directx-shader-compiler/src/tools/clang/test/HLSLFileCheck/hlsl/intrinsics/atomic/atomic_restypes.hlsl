// RUN: %dxc -T cs_6_0 -DDEST=ro_structbuf -DIDX=[0] %s | %FileCheck %s -check-prefixes=CHKRES,CHECK
// RUN: %dxc -T cs_6_0 -DDEST=ro_buf -DIDX=[0] %s | %FileCheck %s -check-prefixes=CHKRES,CHECK
// RUN: %dxc -T cs_6_0 -DDEST=ro_tex -DIDX=[0] %s | %FileCheck %s -check-prefixes=CHKRES,CHECK
// RUN: %dxc -T cs_6_0 -DDEST=gs_var -DIDX=    %s | %FileCheck %s
// RUN: %dxc -T cs_6_0 -DDEST=gs_arr -DIDX=[0] %s | %FileCheck %s
// RUN: %dxc -T cs_6_0 -DDEST=cb_var -DIDX=    %s | %FileCheck %s
// RUN: %dxc -T cs_6_0 -DDEST=cb_arr -DIDX=[0] %s | %FileCheck %s
// RUN: %dxc -T cs_6_0 -DDEST=sc_var -DIDX=    %s | %FileCheck %s
// RUN: %dxc -T cs_6_0 -DDEST=sc_arr -DIDX=[0] %s | %FileCheck %s
// These are different because they aren't const, so are caught later
// RUN: %dxc -T cs_6_0 -DDEST=loc_var -DIDX=    %s | %FileCheck %s -check-prefix=CHKLOC
// RUN: %dxc -T cs_6_0 -DDEST=loc_arr -DIDX=[0] %s | %FileCheck %s -check-prefix=CHKLOC
// RUN: %dxc -T cs_6_0 -DDEST=ix -DIDX=    %s | %FileCheck %s -check-prefix=CHKLOC


// Test various Interlocked ops using different invalid destination memory types
// The way the dest param of atomic ops is lowered is unique and missed a lot of
// these invalid uses. There are a few points where the lowering branches depending
// on the memory type, so this tries to cover all those branches:
// groupshared, cbuffers, structbuffers, other resources, and other non-resources

StructuredBuffer<uint> ro_structbuf;
Buffer<uint> ro_buf;
Texture1D<uint> ro_tex;

const groupshared uint gs_var = 0;
const groupshared uint gs_arr[4] = {0, 0, 0, 0};

RWStructuredBuffer<float4> output; // just something to keep the variables live

cbuffer CB {
  uint cb_var;
  uint cb_arr[4];
}

static const uint sc_var = 1;
static const uint sc_arr[4] = {0,1,2,3};

[numthreads(1,1,1)]
void main(uint ix : SV_GroupIndex) {

  uint loc_var;
  uint loc_arr[4];

  int val = 1;
  uint comp = 1;
  uint orig;

  // add
  // CHECK: error: no matching function for call to 'InterlockedAdd'
  // CHKRES: note: candidate function not viable: 1st argument {{.*}} would lose const qualifier
  // CHECK: error: no matching function for call to 'InterlockedAdd'
  // CHKRES: note: candidate function not viable: 1st argument {{.*}} would lose const qualifier
  // CHKLOC: error: Atomic operation targets must be groupshared, Node Record or UAV
  // CHKLOC: error: Atomic operation targets must be groupshared, Node Record or UAV
  InterlockedAdd(DEST IDX, val);
  InterlockedAdd(DEST IDX, val, orig);

  // min
  // CHECK: error: no matching function for call to 'InterlockedMin'
  // CHKRES: note: candidate function not viable: 1st argument {{.*}} would lose const qualifier
  // CHECK: error: no matching function for call to 'InterlockedMin'
  // CHKRES: note: candidate function not viable: 1st argument {{.*}} would lose const qualifier
  // CHKLOC: error: Atomic operation targets must be groupshared, Node Record or UAV
  // CHKLOC: error: Atomic operation targets must be groupshared, Node Record or UAV
  InterlockedMin(DEST IDX, val);
  InterlockedMin(DEST IDX, val, orig);

  // max
  // CHECK: error: no matching function for call to 'InterlockedMax'
  // CHKRES: note: candidate function not viable: 1st argument {{.*}} would lose const qualifier
  // CHECK: error: no matching function for call to 'InterlockedMax'
  // CHKRES: note: candidate function not viable: 1st argument {{.*}} would lose const qualifier
  // CHKLOC: error: Atomic operation targets must be groupshared, Node Record or UAV
  // CHKLOC: error: Atomic operation targets must be groupshared, Node Record or UAV
  InterlockedMax(DEST IDX, val);
  InterlockedMax(DEST IDX, val, orig);

  // and
  // CHECK: error: no matching function for call to 'InterlockedAnd'
  // CHKRES: note: candidate function not viable: 1st argument {{.*}} would lose const qualifier
  // CHECK: error: no matching function for call to 'InterlockedAnd'
  // CHKRES: note: candidate function not viable: 1st argument {{.*}} would lose const qualifier
  // CHKLOC: error: Atomic operation targets must be groupshared, Node Record or UAV
  // CHKLOC: error: Atomic operation targets must be groupshared, Node Record or UAV
  InterlockedAnd(DEST IDX, val);
  InterlockedAnd(DEST IDX, val, orig);

  // or
  // CHECK: error: no matching function for call to 'InterlockedOr'
  // CHKRES: note: candidate function not viable: 1st argument {{.*}} would lose const qualifier
  // CHECK: error: no matching function for call to 'InterlockedOr'
  // CHKRES: note: candidate function not viable: 1st argument {{.*}} would lose const qualifier
  // CHKLOC: error: Atomic operation targets must be groupshared, Node Record or UAV
  // CHKLOC: error: Atomic operation targets must be groupshared, Node Record or UAV
  InterlockedOr(DEST IDX, val);
  InterlockedOr(DEST IDX, val, orig);

  // xor
  // CHECK: error: no matching function for call to 'InterlockedXor'
  // CHKRES: note: candidate function not viable: 1st argument {{.*}} would lose const qualifier
  // CHECK: error: no matching function for call to 'InterlockedXor'
  // CHKRES: note: candidate function not viable: 1st argument {{.*}} would lose const qualifier
  // CHKLOC: error: Atomic operation targets must be groupshared, Node Record or UAV
  // CHKLOC: error: Atomic operation targets must be groupshared, Node Record or UAV
  InterlockedXor(DEST IDX, val);
  InterlockedXor(DEST IDX, val, orig);

  // compareStore
  // CHECK: error: no matching function for call to 'InterlockedCompareStore'
  // CHKRES: note: candidate function not viable: 1st argument {{.*}} would lose const qualifier
  // CHKLOC: error: Atomic operation targets must be groupshared, Node Record or UAV
  InterlockedCompareStore(DEST IDX, comp, val);

  // exchange
  // CHECK: error: no matching function for call to 'InterlockedExchange'
  // CHKRES: note: candidate function not viable: 1st argument {{.*}} would lose const qualifier
  // CHKLOC: error: Atomic operation targets must be groupshared, Node Record or UAV
  InterlockedExchange(DEST IDX, val, orig);

  // compareExchange
  // CHECK: error: no matching function for call to 'InterlockedCompareExchange'
  // CHKRES: note: candidate function not viable: 1st argument {{.*}} would lose const qualifier
  // CHKLOC: error: Atomic operation targets must be groupshared, Node Record or UAV
  InterlockedCompareExchange(DEST IDX, comp, val, orig);

  output[ix] = (float)DEST IDX;
}
