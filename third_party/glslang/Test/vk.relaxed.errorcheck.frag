#version 460

layout (location = 0) in vec4 io;

out vec4 o;

// default uniforms will be gathered into a uniform block
uniform vec4 a;     // declared in both stages with different types

uniform float test; // declared twice in this compilation unit
uniform vec2 test;

vec4 foo() {
    return a + vec4(test);
}

void main() {
    o = io + foo();
}
