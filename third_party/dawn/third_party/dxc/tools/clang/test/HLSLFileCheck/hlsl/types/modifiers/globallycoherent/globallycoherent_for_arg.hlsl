// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// Make sure not crash.
// CHECK:call %dx.types.ResRet.f32 @dx.op.bufferLoad.f32

globallycoherent RWBuffer<float> u;

float read(RWBuffer<float> buf) {
  return buf[0];
}

float main() : SV_Target {

  return read(u);
}