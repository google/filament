// RUN: %dxc -T lib_6_3 -DDEST=ro_structbuf %s | %FileCheck %s
// RUN: %dxc -T lib_6_3 -DDEST=ro_structbuf %s | %FileCheck %s
// RUN: %dxc -T lib_6_3 -DDEST=ro_buf %s | %FileCheck %s
// RUN: %dxc -T lib_6_3 -DDEST=ro_buf %s | %FileCheck %s
// RUN: %dxc -T lib_6_3 -DDEST=ro_tex %s | %FileCheck %s
// RUN: %dxc -T lib_6_3 -DDEST=ro_tex %s | %FileCheck %s


// Test that errors on atomic dest params will still fire when used in exported
// functions in a library shader. Limits testing to one each of binop and xchg

StructuredBuffer<uint> ro_structbuf;
Buffer<uint> ro_buf;
Texture1D<uint> ro_tex;

RWStructuredBuffer<float4> output; // just something to keep the variables live

// CHECK: error: no matching function for call to 'InterlockedAdd'
// CHECK: note: candidate function not viable: 1st argument {{.*}} would lose const qualifier
// CHECK: error: no matching function for call to 'InterlockedAdd'
// CHECK: note: candidate function not viable: 1st argument {{.*}} would lose const qualifier
// CHECK: error: no matching function for call to 'InterlockedAdd'
// CHECK: note: candidate function not viable: 1st argument {{.*}} would lose const qualifier
export void AtomicAdd(StructuredBuffer<uint> res, uint val) {InterlockedAdd(res[0], val);}
export void AtomicAdd(Buffer<uint> res, uint val) {InterlockedAdd(res[0], val);}
export void AtomicAdd(Texture1D<uint> res, uint val) {InterlockedAdd(res[0], val);}

// CHECK: error: no matching function for call to 'InterlockedCompareStore'
// CHECK: note: candidate function not viable: 1st argument {{.*}} would lose const qualifier
// CHECK: error: no matching function for call to 'InterlockedCompareStore'
// CHECK: note: candidate function not viable: 1st argument {{.*}} would lose const qualifier
// CHECK: error: no matching function for call to 'InterlockedCompareStore'
// CHECK: note: candidate function not viable: 1st argument {{.*}} would lose const qualifier
export void AtomicCompareStore(StructuredBuffer<uint> res, uint comp, uint val) {InterlockedCompareStore(res[0], comp, val);}
export void AtomicCompareStore(Buffer<uint> res, uint comp, uint val) {InterlockedCompareStore(res[0], comp, val);}
export void AtomicCompareStore(Texture1D<uint> res, uint comp, uint val) {InterlockedCompareStore(res[0], comp, val);}

[numthreads(1,1,1)]
void main(uint ix : SV_GroupIndex) {

  int val = 1;
  uint comp = 1;
  uint orig;

  // Add
  AtomicAdd(DEST, val);

  // CompareStore
  AtomicCompareStore(DEST, comp, val);

  output[ix] = (float)DEST[0];
}
