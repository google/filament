// RUN: %dxc -T ps_6_5 -E PS -fcgl %s | FileCheck %s
// RUN: %dxc -T ps_6_5 -E PS %s | FileCheck %s -check-prefix=CHECKDXIL

// QuadAny called before WaveActiveAnyTrue.
// Don't be sensitive to HL Opcode because those can change.
// CHECK: call i1 [[QuadAny:@"[^"]+"]](i32
// CHECK: call i1 [[WaveActiveAnyTrue:@"[^".]+\.[^.]+\.[^.]+\.wave[^"][^"]+"]](i32
// ^ matches  call i1 @"dx.hl.op.wave.i1 (i32, i1)"(i32

// Ensure HL declarations are not collapsed when attributes differ
// CHECK-DAG: declare i1 [[QuadAny]]({{.*}}) #[[AttrQuadAny:[0-9]+]]
// CHECK-DAG: declare i1 [[WaveActiveAnyTrue]]({{.*}}) #[[AttrWaveActiveAnyTrue:[0-9]+]]

// Ensure correct attributes for each HL intrinsic
// CHECK-DAG: attributes #[[AttrQuadAny]] = { nounwind }
// CHECK-DAG: attributes #[[AttrWaveActiveAnyTrue]] = { nounwind "dx.wave-sensitive"="y" }

// Ensure WaveActiveAnyTrue not eliminated in final DXIL:
// CHECKDXIL: call i32 @dx.op.quadOp.i32(
// CHECKDXIL-NEXT: call i32 @dx.op.quadOp.i32(
// CHECKDXIL-NEXT: call i32 @dx.op.quadOp.i32(
// CHECKDXIL: call i1 @dx.op.waveAnyTrue(

RaytracingAccelerationStructure AccelerationStructure : register(t0);
RWByteAddressBuffer log : register(u0);

StructuredBuffer<int> buf[]: register(t3);
RWStructuredBuffer<int> uav;

// test wave attr
int PS(int a : A, int b : B) : SV_Target
{
  int res = 0;
  
  for (;;) {    
    bool u = QuadAny(a);
    u = WaveActiveAnyTrue(a);
    if (a != u) {
      res += buf[(int)u][b];
      break;
    }
  }
  return res;
}


