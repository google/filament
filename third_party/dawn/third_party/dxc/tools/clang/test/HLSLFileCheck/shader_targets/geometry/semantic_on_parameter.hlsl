// RUN: %dxc -E main -T gs_6_0 %s | FileCheck %s

// Make sure only one semnatic index created.
// CHECK:; COORD                    0   xyzw        0     NONE   float   xyzw
// CHECK-NOT:; COORD                    1   xyzw        0     NONE   float   xyzw

struct MyStruct
{
    float4 pos : SV_Position;
    float2 a : AAA;
};

struct MyStruct2
{
    uint3 X : XXX;
    float4 p[3] : PPP;
    uint3 Y : YYY;
};

int g1;

[maxvertexcount(12)]
void main(line float4 array[2] : COORD, inout PointStream<MyStruct> OutputStream0)
{
 float4 r = array[0];
  MyStruct a = (MyStruct)0;
  MyStruct2 b = (MyStruct2)0;
  a.pos = array[r.x];
  a.a = r.xy;
  b.X = r.xyz;
  b.Y = a.pos.xyz;
  b.p[2] = a.pos * 44;
  OutputStream0.Append(a);
  OutputStream0.RestartStrip();
}
