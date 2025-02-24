// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

struct S { float val; };

// Constant buffers, space cb
// CHECK-DAG: ; cb                                cbuffer      NA          NA     CB0     cb0,space1     1
// CHECK-DAG: ; cb2                               cbuffer      NA          NA     CB1     cb1,space1     1
cbuffer cb : register(space1) { float cb_val; }
ConstantBuffer<S> cb2 : register(space1);

// Samplers, space s
// CHECK-DAG: ; s                                 sampler      NA          NA      S0      s0,space2     1
SamplerState s : register(space2);

// SRVs, space t
// CHECK-DAG: ; buf                               texture     f32         buf      T0      t0,space3     1
// CHECK-DAG: ; tex                               texture     f32          1d      T1      t1,space3     1
// CHECK-DAG: ; sbuf                              texture  struct         r/o      T2      t2,space3     1
// CHECK-DAG: ; babuf                             texture    byte         r/o      T3      t3,space3     1
// CHECK-DAG: ; tb                                texture     u32     tbuffer      T4      t4,space3     1
// CHECK-DAG: ; tb2                               texture     u32     tbuffer      T5      t5,space3     1
Buffer<float> buf : register(space3);
Texture1D<float> tex : register(space3);
StructuredBuffer<float> sbuf : register(space3);
ByteAddressBuffer babuf : register(space3);
tbuffer tb : register(space3) { float tb_val; }
TextureBuffer<S> tb2 : register(space3);

// UAVs, space u
// CHECK-DAG: ; rwsbuf                                UAV  struct         r/w      U0      u0,space4     1
// CHECK-DAG: ; appbuf                                UAV  struct     r/w+cnt      U1      u1,space4     1
// CHECK-DAG: ; conbuf                                UAV  struct     r/w+cnt      U2      u2,space4     1
// CHECK-DAG: ; rwbabuf                               UAV    byte         r/w      U3      u3,space4     1
RWStructuredBuffer<float> rwsbuf : register(space4);
AppendStructuredBuffer<float> appbuf : register(space4);
ConsumeStructuredBuffer<float> conbuf : register(space4);
RWByteAddressBuffer rwbabuf : register(space4);

// Use all resources so they get allocated
float main() : SV_Target {
  appbuf.Append(0);
  return cb_val + cb2.val
    + buf[0] + tex[0] + sbuf[0] + babuf.Load<float>(0) + tb_val + tb2.val
    + rwsbuf[0] + conbuf.Consume() + rwbabuf.Load<float>(0)
    + tex.Sample(s, 0);
}
