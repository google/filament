#version 450

layout(binding = 0) uniform sampler2DArrayShadow s2da;

layout(location = 0) out vec4 c_out;

layout(location = 0) in vec4 tc;

void main()
{
    float c = textureLod(s2da, tc, 0);
    c_out = vec4(c);
}
