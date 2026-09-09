// RUN: %dxc -T ps_6_7 -E PS -fcgl %s | FileCheck %s
// RUN: %dxc -T ps_6_7 -E PS %s | FileCheck %s -check-prefix=CHECKDXIL

// IncrementCounter called before GetRenderTargetSampleCount.
// Don't be sensitive to HL Opcode because those can change.
// make sure that we are detecting the .rn attribute in the call
// CHECK: call float [[EvaluateAttributeCentroid:@"[^".]+\.[^.]+\.[^.]+\.rn[^"]+"]](i32
// ^ matches call i32 @"dx.hl.op.rn.i32 (i32)"(i32 19), !dbg !45 ; line:33 col:14
// CHECK: call float [[QuadReadAcrossX:@"[^"]+"]](i32

// Ensure HL declarations are not collapsed when attributes differ
// CHECK-DAG: declare float [[EvaluateAttributeCentroid]]({{.*}}) #[[AttrEvauluateAttributeCentroid:[0-9]+]]
// CHECK-DAG: declare float [[QuadReadAcrossX]]({{.*}}) #[[AttrQuadReadAcrossX:[0-9]+]]

// Ensure correct attributes for each HL intrinsic
// CHECK-DAG: attributes #[[AttrEvauluateAttributeCentroid]] = { nounwind readnone }
// CHECK-DAG: attributes #[[AttrQuadReadAcrossX]] = { nounwind }

// Ensure EvaluateAttributeCentroid not eliminated in final DXIL:
// CHECKDXIL: call float @dx.op.evalCentroid.f32(
// CHECKDXIL: call float @dx.op.quadOp.f32(

StructuredBuffer<int> buf[]: register(t3);
RWStructuredBuffer<int> uav;

// test read-none attr
int PS(float a : A, int b : B) : SV_Target
{
  int res = 0;
  
  for (;;) {    
    float x = EvaluateAttributeCentroid(a);
    x += QuadReadAcrossX(x);
    
    if (a != x) {
      res += buf[(int)x][b];
      break;
    }
  }
  return res;
}
