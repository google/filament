#version 450
#extension GL_EXT_shader_16bit_storage : require
#extension GL_EXT_shader_8bit_storage : require

in ins
{
    layout(location = 0) u8vec2 m0;
    layout(location = 1) u16vec2 m1;
} f_in;

layout(location = 0) out uvec4 f_out;

void main()
{
    f_out = uvec4(uvec2(f_in.m1), uvec2(f_in.m0));
}