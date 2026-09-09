// RUN: %dxc -E main -T ps_6_0  %s | FileCheck %s

// Tests that no alloca is left for copy parameter.
// CHECK-NOT: alloca
// CHECK:call %dx.types.ResRet.f32 @dx.op.bufferLoad.f32(i32 68
// CHECK:call %dx.types.ResRet.f32 @dx.op.bufferLoad.f32(i32 68
// CHECK:call %dx.types.ResRet.f32 @dx.op.bufferLoad.f32(i32 68
// CHECK:call %dx.types.ResRet.f32 @dx.op.bufferLoad.f32(i32 68

Buffer<float4> buf[10];
Buffer<float4> buf2[20];

float4 BufLd(Buffer<float4> b[], int i) {

  return b[i][i];

}

float4 BufLd2(Buffer<float4> b[], int i) {

  return b[i][i] + BufLd(buf2, i+1);

}

float4 BufLd3(int i) {

 return BufLd2(buf, i);

}


float4 BufLd4(int i) {

 return BufLd2(buf, i+1);

}

float4 main(int i:I) : SV_Target {
  return BufLd3(i) + BufLd4(i*2);
}