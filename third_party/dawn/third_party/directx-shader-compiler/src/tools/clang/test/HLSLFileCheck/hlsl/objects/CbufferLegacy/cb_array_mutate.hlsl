// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK:%[[ID:[0-9]+]] = call i32 @dx.op.loadInput
// CHECK:lshr i32 %[[ID]], 2
// CHECK:and i32 %[[ID]], 3
// Make sure only 1 cb load.
// CHECK:call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32
// CHECK-NOT:call %dx.types.CBufRet

cbuffer Pack
{
    int4 __packed[16];
};

static int arrayReallyWant[64] = (int[64])__packed;

float main(int i:I) : SV_Target {
  return arrayReallyWant[i];
}