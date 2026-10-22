#version 450
#extension GL_NV_fragment_shader_barycentric : require

layout(location = 0) out vec2 value;

layout(set = 0, binding = 0) readonly buffer Vertices
{
	vec2 uvs[];
};
      
void main () {
	int prim = gl_PrimitiveID;
	vec2 uv0 = uvs[3 * prim + 0];
	vec2 uv1 = uvs[3 * prim + 1];
	vec2 uv2 = uvs[3 * prim + 2];
    value = gl_BaryCoordNoPerspNV.x * uv0 + gl_BaryCoordNoPerspNV.y * uv1 + gl_BaryCoordNoPerspNV.z * uv2;
}
