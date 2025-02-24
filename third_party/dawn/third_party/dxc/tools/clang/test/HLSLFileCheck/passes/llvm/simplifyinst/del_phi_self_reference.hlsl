// RUN: %dxc -T ps_6_6 %s | FileCheck %s

// Check a shader that previously resulted in a self-referencial add instruction
// when the PHI it previously drew from was eliminated due to poor dead basic block elimination
// The end result is a really dumb shader because most of the code is dead

// CHECK: define void @main()
// CHECK: call void @dx.op.storeOutput.f32(i32 5
// CHECK-NEXT: ret void

float main (uint2 param : P) : SV_Target {

  bool yes = true;

  // Trivially dead conditional block that becomes several basic blocks
  // If BB elimination passes aren't done in the right order,
  // Only some of the unused blocks could be destroyed and update the phi users incorrectly
  if (!yes) {
    for (int i = 0; i < 1; i++) {
      // Force a PHI by breaking the block up
      if (param.y)
        break;
      // This will use a PHI that, when the inital loop block is eliminated,
      // could end up unconditionally adding to itself
      ++param.x;
    }
  }
  return 0;
}
