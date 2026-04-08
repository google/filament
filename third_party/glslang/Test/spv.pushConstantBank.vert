#version 460
#extension GL_NV_push_constant_bank : enable

layout(std430, binding = 0) buffer ResultData {
    uint bank[8];
} resultData;

layout(push_constant) uniform PushConstantBank0 {
    uint data;
} bank0;

layout(push_constant, bank = 1) uniform PushConstantBank1 {
    uint data;
} bank1;

layout(push_constant, bank = 2) uniform PushConstantBank2 {
    uint data;
} bank2;

layout(push_constant, bank = 3) uniform PushConstantBank3 {
    uint data;
} bank3;

layout(push_constant, bank = 4) uniform PushConstantBank4 {
    uint data;
} bank4;

layout(push_constant, bank = 5) uniform PushConstantBank5 {
    uint data;
} bank5;

layout(push_constant, bank = 6, member_offset = 64) uniform PushConstantBank6 {
    uint data;
} bank6_offset;

void main() {
    resultData.bank[0] = bank0.data;
    resultData.bank[1] = bank1.data;
    resultData.bank[2] = bank2.data;
    resultData.bank[3] = bank3.data;
    resultData.bank[4] = bank4.data;
    resultData.bank[5] = bank5.data;
    resultData.bank[6] = bank6_offset.data;
    gl_Position = vec4(0, 0, 0, 0);
}