// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpDecorate %specConst SpecId 10
[[vk::constant_id(10)]]
// CHECK: %specConst = OpSpecConstant %int 12
const int specConst = 12;

// TODO: The frontend parsing hits assertion failures saying cannot evaluating
// as constant int for the following usages.
/*
cbuffer Data {
    float4 pos[specConst];
    float4 tex[specConst + 5];
};
*/

// CHECK: [[add:%[0-9]+]] = OpSpecConstantOp %int IAdd %specConst %int_3
static const int val = specConst + 3;

// CHECK-LABEL:  %main = OpFunction
// CHECK:                OpStore %val [[add]]
void main() {

}
