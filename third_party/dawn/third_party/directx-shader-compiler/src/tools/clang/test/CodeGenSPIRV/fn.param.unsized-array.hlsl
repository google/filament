// RUN: not %dxc -T vs_6_0 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

// CHECK: 7: error: initializer for type 'unsigned int []' unimplemented
void foo(uint value[]) {
  value[0] = 1;
}

float4 main() : SV_Position {
  uint Mem[2];
  foo(Mem);
  return float4(Mem[0], Mem[1], 0, 0);
}
