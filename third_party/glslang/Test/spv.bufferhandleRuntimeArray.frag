#version 460
#extension GL_EXT_buffer_reference : require

layout(buffer_reference) buffer A {
    uint x;
 };

layout(buffer_reference) buffer B { A a[]; };

layout(push_constant) uniform PushConstants {
    B b;
    uint c;
};

void main() {}