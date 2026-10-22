// RUN: %dxc -T cs_6_0 -Od -HV 2018 %s | FileCheck %s
// A test of explicit loop unrolling on a loop that uses a wave op in a break block

// CHECK: void @main
// CHECK: @dx.op.cbufferLoadLegacy.f32
// CHECK: @dx.op.waveActiveOp.f32
// CHECK: @dx.op.cbufferLoadLegacy.i32
// CHECK: br i1
// CHECK: @dx.op.cbufferLoadLegacy.f32
// CHECK: @dx.op.waveActiveOp.f32
// CHECK: @dx.op.cbufferLoadLegacy.i32
// CHECK: br i1
// CHECK: @dx.op.cbufferLoadLegacy.f32
// CHECK: @dx.op.waveActiveOp.f32
// CHECK: @dx.op.cbufferLoadLegacy.i32
// CHECK: br i1
// CHECK: @dx.op.cbufferLoadLegacy.f32
// CHECK: @dx.op.waveActiveOp.f32
// CHECK: @dx.op.cbufferLoadLegacy.i32
// CHECK: br i1
// CHECK: @dx.op.cbufferLoadLegacy.f32
// CHECK: @dx.op.waveActiveOp.f32
// CHECK: @dx.op.cbufferLoadLegacy.i32
// CHECK: br i1
// CHECK: @dx.op.cbufferLoadLegacy.f32
// CHECK: @dx.op.waveActiveOp.f32
// CHECK-NOT: @dx.op.waveActiveOp.f32
// CHECK: br
// CHECK: ret void

RWStructuredBuffer<float> u0;
uint C;
float f;
[numthreads(64,1,1)]
void main(uint GI : SV_GroupIndex)
{
    float r = 0;
    [unroll]
    for (int i = 0; i < C && i < 64; ++i) {
        r += WaveActiveSum(f);
        if (i > 4) {
          r *= 2;
          break;
        }
    }

    u0[GI] = r;
}
