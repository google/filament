// RUN: %dxc -T lib_6_3 %s | FileCheck %s

// CHECK: define void @main()
// CHECK: call i32 @dx.op.loadInput.i32
// CHECK: call void
// CHECK-SAME: sreg
// CHECK: call i32
// CHECK-SAME: ureg
// CHECK: call void @dx.op.storeOutput.i32

// dxc -T lib_6_3 -Fo use-TempReg.dxl
// dxl -E=main -T vs_6_0 -Fo shader.dxo TempReg.dxl;use-TempReg.dxl

#include "TempReg.hlslh"

[shader("vertex")]
uint main(uint In : IN) : OUT {
  sreg(0, In);
  return ureg(0);
}

