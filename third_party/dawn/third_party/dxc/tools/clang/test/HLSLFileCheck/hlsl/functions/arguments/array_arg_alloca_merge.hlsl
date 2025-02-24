// RUN: %dxc -E main -T ps_6_0  %s | FileCheck %s

// Tests that no alloca is left for copy parameter.
// CHECK-NOT: alloca

float4 buf[20];
float4 buf2[20];

float4 BufLd(float4 b[20], int i) {

  return b[i];

}

float4 BufLd2(float4 b[20], int i) {

  return b[i] + BufLd(buf2, i+1);

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