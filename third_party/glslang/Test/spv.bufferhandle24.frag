#version 460
#extension GL_EXT_buffer_reference : enable


layout(buffer_reference, buffer_reference_align = 4) buffer A {
    uint x;
};

layout(buffer_reference, buffer_reference_align = 16) buffer B {
    A a;
};

layout(set=0, binding=0) buffer SSBO {
    B b;
};

void main() {
    b.a.x = 0;
}