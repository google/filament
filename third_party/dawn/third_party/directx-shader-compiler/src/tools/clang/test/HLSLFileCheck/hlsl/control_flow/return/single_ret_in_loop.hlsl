// RUN: %dxc -Emain -opt-enable structurize-returns -Tps_6_0 %s | FileCheck %s

// Make sure not crash.
// CHECK:call %dx.types.ResRet.i32 @dx.op.bufferLoad

struct ST
{
 uint a;
};


StructuredBuffer<ST> buf;

uint main(uint i:I) : SV_Target {
 for(;;)
 {
  if(i<buf[i].a)
   i++;
  else
   return buf[i].a;
 }
}