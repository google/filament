#version 450
#extension GL_NV_fragment_shader_barycentric : require

layout(binding = 0, std430) readonly buffer Vertices
{
    vec2 uvs[];
} _19;

layout(location = 0) out vec2 value;

void main()
{
    int prim = gl_PrimitiveID;
    vec2 uv0 = _19.uvs[(3 * prim) + 0];
    vec2 uv1 = _19.uvs[(3 * prim) + 1];
    vec2 uv2 = _19.uvs[(3 * prim) + 2];
    value = ((uv0 * gl_BaryCoordNV.x) + (uv1 * gl_BaryCoordNV.y)) + (uv2 * gl_BaryCoordNV.z);
    value += (((uv0 * gl_BaryCoordNoPerspNV.x) + (uv1 * gl_BaryCoordNoPerspNV.y)) + (uv2 * gl_BaryCoordNoPerspNV.z));
}

