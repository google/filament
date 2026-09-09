// RUN: %dxc -Emain -Tgs_6_0 %s | %opt -S -hlsl-dxil-debug-instrumentation,parameter0=1,parameter1=2 | %FileCheck %s

// Check that the instance/primid detection was emitted:

// CHECK: [[PrimId:[^ ]+]] = call i32 @dx.op.primitiveID.i32(i32 108)
// CHECK: [[CompareToPrimId:[^ ]+]] = icmp eq i32 [[PrimId]], 1
// CHECK: [[GSInstanceId:[^ ]+]] = call i32 @dx.op.gsInstanceID.i32(i32 100)
// CHECK: [[CompareToInstanceId:[^ ]+]] = icmp eq i32 [[GSInstanceId]], 2
// CHECK: [[CompareBoth:[^ ]+]] = and i1 [[CompareToPrimId]], [[CompareToInstanceId]]

struct MyStruct
{
  float4 pos : SV_Position;
  float2 a : AAA;
};


[maxvertexcount(12)][instance(6)]
void main(point float4 array[1] : COORD, inout PointStream<MyStruct> OutputStream0)
{
  MyStruct a = (MyStruct)0;
  OutputStream0.Append(a);
  OutputStream0.RestartStrip();
}
