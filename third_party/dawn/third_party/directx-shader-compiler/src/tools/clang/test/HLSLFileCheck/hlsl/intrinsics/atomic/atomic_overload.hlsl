// RUN: %dxc -no-warnings -T cs_6_0 -DTYPE=double  %s | %FileCheck %s -check-prefixes=INTFAIL,FLTFAIL
// RUN: %dxc -no-warnings -T cs_6_2 -DTYPE=float16_t -enable-16bit-types  %s | %FileCheck %s -check-prefixes=INTFAIL,FLTFAIL
// RUN: %dxc -no-warnings -T cs_6_2 -DTYPE=int16_t -enable-16bit-types  %s | %FileCheck %s -check-prefixes=INTFAIL,FLTFAIL
// RUN: %dxc -no-warnings -T cs_6_2 -DTYPE=uint16_t -enable-16bit-types  %s | %FileCheck %s -check-prefixes=INTFAIL,FLTFAIL
// RUN: %dxc -no-warnings -T cs_6_0 -DTYPE=bool  %s | %FileCheck %s -check-prefixes=INTFAIL,FLTFAIL
// RUN: %dxilver 1.6 | %dxc -no-warnings -T cs_6_5 -DTYPE=int64_t  %s | %FileCheck %s -check-prefix=VALFAIL
// RUN: %dxilver 1.6 | %dxc -no-warnings -T cs_6_5 -DTYPE=uint64_t  %s | %FileCheck %s -check-prefix=VALFAIL


// RUN: %dxc -no-warnings -T cs_6_0 -DTYPE=float  %s | %FileCheck %s -check-prefixes=INTFAIL,
// RUN: %dxc -no-warnings -T cs_6_0 -DTYPE=half  %s | %FileCheck %s -check-prefixes=INTFAIL

// RUN: %dxc -no-warnings -T cs_6_6 -DTYPE=int64_t  %s | %FileCheck %s -check-prefixes=INTCHK
// RUN: %dxc -no-warnings -T cs_6_6 -DTYPE=uint64_t  %s | %FileCheck %s -check-prefixes=INTCHK
// RUN: %dxc -no-warnings -T cs_6_0 -DTYPE=int  %s | %FileCheck %s -check-prefixes=INTCHK
// RUN: %dxc -no-warnings -T cs_6_0 -DTYPE=uint  %s | %FileCheck %s -check-prefixes=INTCHK


// Test various Interlocked ops using different memory types with invalid types

RWBuffer<TYPE> rw_res;
groupshared TYPE gs_res;
RWByteAddressBuffer ba_res;

RWStructuredBuffer<float4> output;

[numthreads(1,1,1)]
void main(uint ix : SV_GroupIndex) {
  int val = 1;
  TYPE comp = 1;
  TYPE orig;

  // add
  // INTFAIL: error: no matching function for call to 'InterlockedAdd'
  // INTFAIL: note: candidate function not viable: no known conversion from
  // INTFAIL: error: no matching function for call to 'InterlockedAdd'
  // INTFAIL: note: candidate function not viable: no known conversion from
  // INTFAIL: error: no matching function for call to 'InterlockedAdd'
  // INTFAIL: note: candidate function not viable: no known conversion from
  // INTFAIL: error: no matching function for call to 'InterlockedAdd'
  // INTFAIL: note: candidate function not viable: no known conversion from

  // VALFAIL: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // VALFAIL: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // VALFAIL: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // VALFAIL: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'

  // INTCHK: call i{{[63][24]}} @dx.op.atomicBinOp.i{{[63][24]}}
  // INTCHK: call i{{[63][24]}} @dx.op.atomicBinOp.i{{[63][24]}}
  // INTCHK: atomicrmw add i{{[63][24]}}
  // INTCHK: atomicrmw add i{{[63][24]}}
  InterlockedAdd(rw_res[0], val);
  InterlockedAdd(rw_res[0], val, orig);
  InterlockedAdd(gs_res, val);
  InterlockedAdd(gs_res, val, orig);

  // min
  // INTFAIL: error: no matching function for call to 'InterlockedMin'
  // INTFAIL: note: candidate function not viable: no known conversion from
  // INTFAIL: error: no matching function for call to 'InterlockedMin'
  // INTFAIL: note: candidate function not viable: no known conversion from
  // INTFAIL: error: no matching function for call to 'InterlockedMin'
  // INTFAIL: note: candidate function not viable: no known conversion from
  // INTFAIL: error: no matching function for call to 'InterlockedMin'
  // INTFAIL: note: candidate function not viable: no known conversion from

  // VALFAIL: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // VALFAIL: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // VALFAIL: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // VALFAIL: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'

  // INTCHK: call i{{[63][24]}} @dx.op.atomicBinOp.i{{[63][24]}}
  // INTCHK: call i{{[63][24]}} @dx.op.atomicBinOp.i{{[63][24]}}
  // INTCHK: atomicrmw {{u?}}min i{{[63][24]}}
  // INTCHK: atomicrmw {{u?}}min i{{[63][24]}}
  InterlockedMin(rw_res[0], val);
  InterlockedMin(rw_res[0], val, orig);
  InterlockedMin(gs_res, val);
  InterlockedMin(gs_res, val, orig);

  // max
  // INTFAIL: error: no matching function for call to 'InterlockedMax'
  // INTFAIL: note: candidate function not viable: no known conversion from
  // INTFAIL: error: no matching function for call to 'InterlockedMax'
  // INTFAIL: note: candidate function not viable: no known conversion from
  // INTFAIL: error: no matching function for call to 'InterlockedMax'
  // INTFAIL: note: candidate function not viable: no known conversion from
  // INTFAIL: error: no matching function for call to 'InterlockedMax'
  // INTFAIL: note: candidate function not viable: no known conversion from

  // VALFAIL: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // VALFAIL: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // VALFAIL: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // VALFAIL: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'

  // INTCHK: call i{{[63][24]}} @dx.op.atomicBinOp.i{{[63][24]}}
  // INTCHK: call i{{[63][24]}} @dx.op.atomicBinOp.i{{[63][24]}}
  // INTCHK: atomicrmw {{u?}}max i{{[63][24]}}
  // INTCHK: atomicrmw {{u?}}max i{{[63][24]}}
  InterlockedMax(rw_res[0], val);
  InterlockedMax(rw_res[0], val, orig);
  InterlockedMax(gs_res, val);
  InterlockedMax(gs_res, val, orig);

  // and
  // INTFAIL: error: no matching function for call to 'InterlockedAnd'
  // INTFAIL: note: candidate function not viable: no known conversion from
  // INTFAIL: error: no matching function for call to 'InterlockedAnd'
  // INTFAIL: note: candidate function not viable: no known conversion from
  // INTFAIL: error: no matching function for call to 'InterlockedAnd'
  // INTFAIL: note: candidate function not viable: no known conversion from
  // INTFAIL: error: no matching function for call to 'InterlockedAnd'
  // INTFAIL: note: candidate function not viable: no known conversion from

  // VALFAIL: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // VALFAIL: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // VALFAIL: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // VALFAIL: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'

  // INTCHK: call i{{[63][24]}} @dx.op.atomicBinOp.i{{[63][24]}}
  // INTCHK: call i{{[63][24]}} @dx.op.atomicBinOp.i{{[63][24]}}
  // INTCHK: atomicrmw and i{{[63][24]}}
  // INTCHK: atomicrmw and i{{[63][24]}}
  InterlockedAnd(rw_res[0], val);
  InterlockedAnd(rw_res[0], val, orig);
  InterlockedAnd(gs_res, val);
  InterlockedAnd(gs_res, val, orig);

  // or
  // INTFAIL: error: no matching function for call to 'InterlockedOr'
  // INTFAIL: note: candidate function not viable: no known conversion from
  // INTFAIL: error: no matching function for call to 'InterlockedOr'
  // INTFAIL: note: candidate function not viable: no known conversion from
  // INTFAIL: error: no matching function for call to 'InterlockedOr'
  // INTFAIL: note: candidate function not viable: no known conversion from
  // INTFAIL: error: no matching function for call to 'InterlockedOr'
  // INTFAIL: note: candidate function not viable: no known conversion from

  // VALFAIL: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // VALFAIL: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // VALFAIL: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // VALFAIL: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'

  // INTCHK: call i{{[63][24]}} @dx.op.atomicBinOp.i{{[63][24]}}
  // INTCHK: call i{{[63][24]}} @dx.op.atomicBinOp.i{{[63][24]}}
  // INTCHK: atomicrmw or i{{[63][24]}}
  // INTCHK: atomicrmw or i{{[63][24]}}
  InterlockedOr(rw_res[0], val);
  InterlockedOr(rw_res[0], val, orig);
  InterlockedOr(gs_res, val);
  InterlockedOr(gs_res, val, orig);

  // xor
  // INTFAIL: error: no matching function for call to 'InterlockedXor'
  // INTFAIL: note: candidate function not viable: no known conversion from
  // INTFAIL: error: no matching function for call to 'InterlockedXor'
  // INTFAIL: note: candidate function not viable: no known conversion from
  // INTFAIL: error: no matching function for call to 'InterlockedXor'
  // INTFAIL: note: candidate function not viable: no known conversion from
  // INTFAIL: error: no matching function for call to 'InterlockedXor'
  // INTFAIL: note: candidate function not viable: no known conversion from

  // VALFAIL: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // VALFAIL: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // VALFAIL: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // VALFAIL: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'

  // INTCHK: call i{{[63][24]}} @dx.op.atomicBinOp.i{{[63][24]}}
  // INTCHK: call i{{[63][24]}} @dx.op.atomicBinOp.i{{[63][24]}}
  // INTCHK: atomicrmw xor i{{[63][24]}}
  // INTCHK: atomicrmw xor i{{[63][24]}}
  InterlockedXor(rw_res[0], val);
  InterlockedXor(rw_res[0], val, orig);
  InterlockedXor(gs_res, val);
  InterlockedXor(gs_res, val, orig);

  // compareStore
  // INTFAIL: error: no matching function for call to 'InterlockedCompareStore'
  // INTFAIL: note: candidate function not viable: no known conversion from
  // INTFAIL: error: no matching function for call to 'InterlockedCompareStore'
  // INTFAIL: note: candidate function not viable: no known conversion from

  // VALFAIL: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // VALFAIL: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'

  // INTCHK: call i{{[63][24]}} @dx.op.atomicCompareExchange.i{{[63][24]}}
  // INTCHK: cmpxchg i{{[63][24]}}
  InterlockedCompareStore(rw_res[0], comp, val);
  InterlockedCompareStore(gs_res, comp, val);

  // exchange
  // FLTFAIL: error: no matching function for call to 'InterlockedExchange'
  // FLTFAIL: note: candidate function not viable: no known conversion from
  // FLTFAIL: error: no matching function for call to 'InterlockedExchange'
  // FLTFAIL: note: candidate function not viable: no known conversion from

  // VALFAIL: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // VALFAIL: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'

  // INTCHK: call i{{[63][24]}} @dx.op.atomicBinOp.i{{[63][24]}}
  // INTCHK: atomicrmw xchg i{{[63][24]}}
  InterlockedExchange(rw_res[0], val, orig);
  InterlockedExchange(gs_res, val, orig);

  // compareExchange
  // INTFAIL: error: no matching function for call to 'InterlockedCompareExchange'
  // INTFAIL: note: candidate function not viable: no known conversion from
  // INTFAIL: error: no matching function for call to 'InterlockedCompareExchange'
  // INTFAIL: note: candidate function not viable: no known conversion from

  // VALFAIL: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'
  // VALFAIL: error: opcode '64-bit atomic operations' should only be used in 'Shader Model 6.6+'

  // INTCHK: call i{{[63][24]}} @dx.op.atomicCompareExchange.i{{[63][24]}}
  // INTCHK: cmpxchg i{{[63][24]}}
  InterlockedCompareExchange(rw_res[0], comp, val, orig);
  InterlockedCompareExchange(gs_res, comp, val, orig);

  output[ix] = (float)rw_res[0] + gs_res;
}
