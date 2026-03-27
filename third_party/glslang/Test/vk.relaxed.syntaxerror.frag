#version 460

layout (location = 0) in vec4 io;

out vec4 o;

struct OpaqueInStruct
{
	sampler2D smp;
};

uniform OpaqueInStruct uniformStruct;

vec4 foo(OpaqueInStruct);

void main() {
    o = io + foo(uniformStruct);
}

vec4 foo(OpaqueInStruct sampler) { // unnamed variable and invalid SAMPLER token
    return a + vec4(test);
}

