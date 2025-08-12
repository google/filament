#version 140
#extension GL_ARB_texture_multisample : enable

out float result;
out int result1;
out uint result2;

uniform sampler2DMS data;
uniform sampler2DMSArray data1;
uniform isampler2DMS data2;
uniform usampler2DMSArray data3;
uniform usampler2DMS data4;
uniform isampler2DMSArray data5;
void main()
{
    result = texelFetch(data, ivec2(0), 3).r;
    ivec2 temp = textureSize(data);
    result = texelFetch(data1, ivec3(0), 3).r;
    ivec3 temp1 = textureSize(data1);
	result1 = texelFetch(data2, ivec2(0), 3).r;
    temp = textureSize(data2);
	result2 = texelFetch(data3, ivec3(0), 3).r;
    temp1 = textureSize(data3);
	result2 = texelFetch(data4, ivec2(0), 3).r;
    temp = textureSize(data4);
	result1 = texelFetch(data5, ivec3(0), 3).r;
    temp1 = textureSize(data5);
}
