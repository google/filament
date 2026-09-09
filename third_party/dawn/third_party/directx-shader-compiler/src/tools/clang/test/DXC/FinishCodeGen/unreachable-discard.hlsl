// RUN: %dxc /T ps_6_5 -fcgl %s | FileCheck %s

// Compiling this HLSL would trigger an assertion:
//    While deleting: void (i32, float)* %dx.hl.op..void (i32, float)
//    Use still stuck around after Def is destroyed:  call void @"dx.hl.op..void (i32, float)"(i32 120, float -1.000000e+00), !dbg <0x503000001cc8>
//    Error: assert(use_empty() && "Uses remain when a value is destroyed!")
//    File: <snip>/src/external/DirectXShaderCompiler/lib/IR/Value.cpp(83)
//
// Bug was fixed in CodeGenFunction::EmitDiscardStmt by skipping the emission of
// an unreachable discard.

// CHECK:      define void @main()
// CHECK:      br label %
// CHECK-NOT:  call void @"dx.hl.op..void (i32, float)"
// CHECK:      ret void

void main() {
  while (true) {
  }
  discard;
}
