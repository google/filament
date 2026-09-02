// RUN: %dxc -E main -Zi -O3 -T ps_6_0 -opt-enable structurize-loop-exits-for-unroll %s | FileCheck %s
// RUN: %dxc -E main -Zi -Od -T ps_6_0 -opt-enable structurize-loop-exits-for-unroll %s -DFORCE_UNROLL | FileCheck %s
// RUN: %dxc -E main -Zi -T ps_6_0 %s -opt-enable structurize-loop-exits-for-unroll -DFORCE_UNROLL | FileCheck %s

// CHECK: %{{.+}} = call float @dx.op.unary.f32(i32 13

// CHECK: dx.struct_exit.cond_body
// CHECK: store float

// CHECK: %{{.+}} = call float @dx.op.unary.f32(i32 13

// CHECK: dx.struct_exit.cond_body
// CHECK: store float

// CHECK: %{{.+}} = call float @dx.op.unary.f32(i32 13

// CHECK: dx.struct_exit.cond_body
// CHECK: store float

#ifdef FORCE_UNROLL
#define UNROLL [unroll]
#else
#define UNROLL
#endif

Texture2D tex0;
RWTexture1D<float> uav0;
RWTexture1D<float> uav1;

const uint idx;

[RootSignature("CBV(b0), DescriptorTable(SRV(t0)), DescriptorTable(UAV(u0), UAV(u1))")]
float main(uint a : A, uint b : B, uint c : C) : SV_Target {

  float ret = 0;
  float array[] = {1.0, 2.0, 3.0,};

  [loop] for (uint j = 0; j < 2; j++) {
    UNROLL for(uint i = 1; i <= 3; i++) {

      if ((a * i) & c) {
        ret += sin(i * b); // check for sin

        if ((a * i) & b) {

          if ((c | a) & b) {
            // loop exit here:
            uav0[i] += a;
            return 1;
          }

          array[(idx + i) % 5] += a; // check that this side-effect is bounded within exit cond
        }
      }
    }
  }

  return ret + array[0];
}

// Exclude quoted source file (see readme)
// CHECK-LABEL: {{!"[^"]*\\0A[^"]*"}}

