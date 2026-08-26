// RUN: %dxc -Tlib_6_5 /Od /Zi %s | %FileCheck %s -check-prefixes=BEFORE
// RUN: %dxc -Tlib_6_5 /Od /Zi %s | %opt -S -dxil-dbg-value-to-dbg-declare | %FileCheck %s -check-prefixes=AFTER

// In this shader, globalStruct.FloatArray will be the only member of globalStruct stored in an llvm global, but Accumulator will force a 
// DILocalVariable for globalStruct in main (because there will be dbg.value/declare for Accumulator), 
// but we expect the above pass to find it and reference it by a new dbg-dot-declare.
// But there still won't be a bit-piece for FloatArray (we're going to add one)
// 
// BEFORE-NOT:dbg.declare
// BEFORE-NOT:!DIExpression(DW_OP_bit_piece, 64, 32)
// BEFORE-NOT:!DIExpression(DW_OP_bit_piece, 96, 32)
// BEFORE:ret void
// BEFORE:!DILocalVariable(tag: DW_TAG_arg_variable, name: "global.globalStruct"

// After the pass is run, the should be a dbg.declare for bit-piece for both members of FloatArray:
// AFTER:dbg.declare{{.*}}!DIExpression(DW_OP_bit_piece, 64, 32)
// AFTER:dbg.declare{{.*}}!DIExpression(DW_OP_bit_piece, 96, 32)

RWStructuredBuffer<float> floatRWUAV : register(u0);

struct GlobalStruct {
  int IntArray[2];
  float FloatArray[2];
  float Accumulator;
};

static GlobalStruct globalStruct;

[shader("compute")]
[numthreads(1, 1, 1)]
void main() {
  globalStruct.IntArray[0] = floatRWUAV[0];
  globalStruct.IntArray[1] = floatRWUAV[1];
  globalStruct.FloatArray[0] = floatRWUAV[2];
  globalStruct.FloatArray[1] = floatRWUAV[3];
  globalStruct.Accumulator = 0;

  uint killSwitch = 0;

  [loop] // do not unroll this
      while (true) {
    globalStruct.Accumulator += globalStruct.FloatArray[killSwitch % 2];

    if (killSwitch++ == 4)
      break;
  }

  floatRWUAV[0] = globalStruct.Accumulator + globalStruct.IntArray[0] + globalStruct.IntArray[1];
}