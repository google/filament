// RUN: %dxc -E main -T gs_6_0 %s | FileCheck %s

// Make sure load input has precise.
// CHECK: OutputTopology=point
// CHECK:loadInput.f32(i32 4, i32 0, i32 0, i8 0, i32 {{.*}}), !dx.precise
// Make sure fadd not have fast.
// CHECK:fadd float %3, 1.000000e+00

struct MyStruct
{
  precise  float4 pos : SV_Position;
};


[maxvertexcount(12)]
void main(point float4 array[1] : COORD, inout PointStream<MyStruct> OutputStream0)
{
  float4 r = array[0];
  MyStruct a = (MyStruct)0;

  a.pos = array[r.x] + 1;

  OutputStream0.Append(a);
  OutputStream0.RestartStrip();

}
