// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK:%[[ID:[0-9]+]] = call i32 @dx.op.loadInput
// CHECK:lshr i32 %[[ID]], 1
// CHECK:and i32 %[[ID]], 1
// Make sure only 1 cb load.
// CHECK:call %dx.types.CBufRet.i64 @dx.op.cbufferLoadLegacy.i64
// CHECK-NOT:call %dx.types.CBufRet

cbuffer Pack
{
    int64_t2 __packed[16];
};

static int64_t arrayReallyWant[32] = (int64_t[32])__packed;

float main(int i:I) : SV_Target {
  return arrayReallyWant[i];
}