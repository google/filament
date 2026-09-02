// RUN: %dxc -T cs_6_6 -E CS -enable-16bit-types -spirv -fcgl  %s -spirv | FileCheck %s



[numthreads(1, 1, 1)]
void CS()
{
	vector<float, 1> x;

 // CHECK: [[x_val:%[0-9]+]] = OpLoad %float %x
 // CHECK: [[y_val:%[0-9]+]] = OpFConvert %half [[x_val]]
 // CHECK: OpStore %y [[y_val]]
	vector<half, 1> y = (vector<half, 1>)x;
}