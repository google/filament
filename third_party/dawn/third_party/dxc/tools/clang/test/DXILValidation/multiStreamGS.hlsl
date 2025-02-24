// RUN: %dxc -E main -T gs_6_0 %s | FileCheck %s

// CHECK:; Output signature:
// CHECK:;
// CHECK:; Name                 Index   Mask Register SysValue  Format   Used
// CHECK:; -------------------- ----- ------ -------- -------- ------- ------
// CHECK:; m0:SV_Position           0   xyzw        0      POS   float   xyzw
// CHECK:; m0:AAA                   0   xy          1     NONE   float   xy
// CHECK:; m1:XXX                   0   xyz         0     NONE    uint   xyz
// CHECK:; m1:PPP                   0   xyzw        1     NONE   float   xyzw
// CHECK:; m1:PPP                   1   xyzw        2     NONE   float   xyzw
// CHECK:; m1:PPP                   2   xyzw        3     NONE   float   xyzw
// CHECK:; m1:YYY                   0   xyz         4     NONE    uint   xyz
// CHECK:; m2:SV_Position           0   xyzw        0      POS   float   xyzw
// CHECK:; m2:AAA                   0   xy          1     NONE   float   xy


// CHECK: OutputStreamMask=7

// CHECK: emitStream(i32 97, i8 0)
// CHECK: cutStream(i32 98, i8 0)
// CHECK: emitStream(i32 97, i8 1)
// CHECK: cutStream(i32 98, i8 1)
// CHECK: emitStream(i32 97, i8 1)
// CHECK: cutStream(i32 98, i8 1)
// CHECK: emitStream(i32 97, i8 2)
// CHECK: cutStream(i32 98, i8 2)

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
void main(point float4 array[1] : COORD, inout PointStream<MyStruct> OutputStream0,
                                         inout PointStream<MyStruct2> OutputStream1,
                                         inout PointStream<MyStruct> OutputStream2)
{
 float4 r = array[0];
  MyStruct a = (MyStruct)0;
  MyStruct2 b = (MyStruct2)0;
  a.pos = array[r.x];
  a.a = r.xy;
  b.X = r.xyz;
  b.Y = a.pos.xyz;
  b.p[2] = a.pos * 44;
  if (g1) {
    OutputStream0.Append(a);
    OutputStream0.RestartStrip();
  } else {
    b.X.x = r.w;
    OutputStream1.Append(b);
    OutputStream1.RestartStrip();
  }
  OutputStream1.Append(b);
  OutputStream1.RestartStrip();

  OutputStream2.Append(a);
  OutputStream2.RestartStrip();
}
