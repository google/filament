#version 450
#extension GL_EXT_fragment_shader_barycentric : require

layout(location = 0) out vec2 value;
layout(location = 0) pervertexEXT in vec2 vUV[3];
layout(location = 1) pervertexEXT in vec2 vUV2[3];
layout(location = 2) pervertexEXT in Foo
{
    vec2 a;
    vec2 b;
} foo[3];


void main()
{
    value = ((vUV[0] * gl_BaryCoordEXT.x) + (vUV[1] * gl_BaryCoordEXT.y)) + (vUV[2] * gl_BaryCoordEXT.z);
    value += (((vUV2[0] * gl_BaryCoordNoPerspEXT.x) + (vUV2[1] * gl_BaryCoordNoPerspEXT.y)) + (vUV2[2] * gl_BaryCoordNoPerspEXT.z));
    value += (foo[0].a * gl_BaryCoordEXT.x);
    value += (foo[0].b * gl_BaryCoordEXT.y);
    value += (foo[1].a * gl_BaryCoordEXT.z);
    value += (foo[1].b * gl_BaryCoordEXT.x);
    value += (foo[2].a * gl_BaryCoordEXT.y);
    value += (foo[2].b * gl_BaryCoordEXT.z);
}

