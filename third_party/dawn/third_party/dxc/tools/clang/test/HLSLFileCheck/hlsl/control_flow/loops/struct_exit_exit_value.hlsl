// RUN: %dxc -Zi -E main -Od -T ps_6_0 -opt-enable structurize-loop-exits-for-unroll %s -DFORCE_UNROLL | FileCheck %s
// RUN: %dxc -Zi -E main -T ps_6_0 -opt-enable structurize-loop-exits-for-unroll %s -DFORCE_UNROLL | FileCheck %s

// note: not testing the path without [unroll]. Can't manage to make a loop small enough for the optimizer to want to unroll.

// CHECK: %{{.+}} = call float @dx.op.unary.f32(i32 13

// CHECK: dx.struct_exit.cond_body
// CHECK: call void @dx.op.textureStore.f32(i32 67

// CHECK: %{{.+}} = call float @dx.op.unary.f32(i32 13

// CHECK: dx.struct_exit.cond_body
// CHECK: call void @dx.op.textureStore.f32(i32 67

// CHECK: %{{.+}} = call float @dx.op.unary.f32(i32 13

// CHECK: dx.struct_exit.cond_body
// CHECK: call void @dx.op.textureStore.f32(i32 67

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

  UNROLL for(uint i = 1; i <= 3; i++) {

    if ((a * i) & c) {
      ret += sin(i * b); // check for sin

      if ((a * i) & b) {

        int offset = i; // This value doesn't dominate latch, is loop dependent,
                        // and therefore must be propagated through to loop latch
                        // so the hoisted loop exit can use it.
        if (i % 2 == 0) {
          offset = 1;
        }

        if ((c | a) & b) {
          // loop exit here
          uav0[i + offset] += a;
          return 1;
        }

        uav1[i + offset] += a;
      }
      uav1[i] += a;
    }
  }

  return ret + array[0];
}

// Exclude quoted source file (see readme)
// CHECK-LABEL: {{!"[^"]*\\0A[^"]*"}}

