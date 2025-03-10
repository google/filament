// RUN: %dxc -T ps_6_4 -E main %s | FileCheck %s

// SV_CullPrimitive just lowers to false, with nothing in signature
// CHECK: ; Input signature:
// CHECK-NEXT: ;
// CHECK-NEXT: ; Name                 Index   Mask Register SysValue  Format   Used
// CHECK-NEXT: ; -------------------- ----- ------ -------- -------- ------- ------
// CHECK-NEXT: ; no parameters
// CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 0)

uint main(bool b : SV_CullPrimitive) : SV_Target {
  return b;
}