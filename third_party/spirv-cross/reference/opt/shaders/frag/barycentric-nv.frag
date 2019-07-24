#version 450
#extension GL_NV_fragment_shader_barycentric : require

layout(binding = 0, std430) readonly buffer Vertices
{
    vec2 uvs[];
} _19;

layout(location = 0) out vec2 value;

void main()
{
    int _23 = 3 * gl_PrimitiveID;
    int _32 = _23 + 1;
    int _39 = _23 + 2;
    value = ((_19.uvs[_23] * gl_BaryCoordNV.x) + (_19.uvs[_32] * gl_BaryCoordNV.y)) + (_19.uvs[_39] * gl_BaryCoordNV.z);
    value += (((_19.uvs[_23] * gl_BaryCoordNoPerspNV.x) + (_19.uvs[_32] * gl_BaryCoordNoPerspNV.y)) + (_19.uvs[_39] * gl_BaryCoordNoPerspNV.z));
}

