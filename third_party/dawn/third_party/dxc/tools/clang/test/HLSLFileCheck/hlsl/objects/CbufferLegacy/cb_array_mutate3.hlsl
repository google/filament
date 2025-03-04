// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// Make sure have 4 cb load
// CHECK:call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32
// CHECK:call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32
// CHECK:call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32
// CHECK:call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32
// CHECK-NOT:call %dx.types.CBufRet

cbuffer Pack
{
    int4 __packed[4];
};

static int arrayReallyWant[16] = (int[16])__packed;

float main(int i:I) : SV_Target {
  arrayReallyWant[0] = 3;
  return arrayReallyWant[i];
}