// RUN: %dxc -E main1 -T gs_6_0 %s | FileCheck %s -check-prefix=CHECK1
// CHECK1: OutputTopology=point
// RUN: %dxc -E main2 -T gs_6_0 %s | FileCheck %s -check-prefix=CHECK2
// CHECK2: OutputTopology=line
// RUN: %dxc -E main3 -T gs_6_0 %s | FileCheck %s -check-prefix=CHECK3
// CHECK3: OutputTopology=triangle
// RUN: %dxc -E main4 -T gs_6_0 %s | FileCheck %s -check-prefix=CHECK4
// CHECK4: OutputTopology=line
// RUN: %dxc -E main5 -T gs_6_0 %s | FileCheck %s -check-prefix=CHECK5
// CHECK5: OutputTopology=triangle





struct MyStruct
{
  precise  float4 pos : SV_Position;
};


[maxvertexcount(12)]
void main1(point float4 array[1] : COORD, inout PointStream<MyStruct> OutputStream0)
{
  float4 r = array[0];
  MyStruct a = (MyStruct)0;

  a.pos = array[r.x] + 1;

  OutputStream0.Append(a);
  OutputStream0.RestartStrip();

}

[maxvertexcount(12)]
void main2(line float4 array[2] : COORD, inout LineStream<MyStruct> OutputStream0)
{
  float4 r = array[0];
  MyStruct a = (MyStruct)0;

  a.pos = array[r.x] + 1;

  OutputStream0.Append(a);
  OutputStream0.RestartStrip();
}

[maxvertexcount(12)]
void main3(triangle float4 array[3] : COORD, inout TriangleStream<MyStruct> OutputStream0)
{
  float4 r = array[0];
  MyStruct a = (MyStruct)0;

  a.pos = array[r.x] + 1;

  OutputStream0.Append(a);
  OutputStream0.RestartStrip();
}

[maxvertexcount(12)]
void main4(lineadj float4 array[4] : COORD, inout LineStream<MyStruct> OutputStream0)
{
  float4 r = array[0];
  MyStruct a = (MyStruct)0;

  a.pos = array[r.x] + 1;

  OutputStream0.Append(a);
  OutputStream0.RestartStrip();
}

[maxvertexcount(12)]
void main5(triangleadj float4 array[6] : COORD, inout TriangleStream<MyStruct> OutputStream0)
{
  float4 r = array[0];
  MyStruct a = (MyStruct)0;

  a.pos = array[r.x] + 1;

  OutputStream0.Append(a);
  OutputStream0.RestartStrip();
}


