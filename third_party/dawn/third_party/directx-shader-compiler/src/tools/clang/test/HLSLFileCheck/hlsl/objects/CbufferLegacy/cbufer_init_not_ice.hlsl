// RUN: %dxc -E main -T vs_6_0  %s 2>&1 | FileCheck %s

// Make sure it compiles, and make sure cbuffer load isn't optimized away
// CHECK: call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %{{.*}}, i32 0)

// The following is const by default for cbuffer, but not the way clang normally
// interprets a const initialized global, since the initializer will be thrown away.
uint CB_One = 1;

// Simplified repro:
int2 main() : OUTPUT {
  const uint ConstVal = CB_One;
  return int(ConstVal);
}

int2 main2(int2 input : INPUT) : OUTPUT {
  const uint ConstFactor = CB_One;
  int2 result = input;
  [unroll]
  for (uint LoopIdx = 0; LoopIdx < 2; LoopIdx++)
  {
    const int2 Offset = (LoopIdx == 0 ? 1 : -1) * (int(ConstFactor) * result);
    result += Offset;
  }
  return result;
}

