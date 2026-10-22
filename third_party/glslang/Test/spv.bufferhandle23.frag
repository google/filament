#version 450
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_buffer_reference : enable

layout(buffer_reference) buffer S4;

layout(buffer_reference, buffer_reference_align = 8, std430) readonly buffer S3 {
    S4 e;
};

// Set buffer_reference_align to 8 and it generates valid code
layout(buffer_reference, buffer_reference_align = 4, std430) readonly buffer S4 {
    uint f;
};

void main() {
    S3 x;
    S4 y = x.e;
}