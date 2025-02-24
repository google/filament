#version 450 core
#extension GL_EXT_buffer_reference : enable

layout(buffer_reference, std430) buffer MeshVertexPositions {
  float data[];
};

struct Mesh {
  MeshVertexPositions positions;
};

layout(set = 0, binding = 0) readonly buffer PerPass_meshes {
  Mesh data[];
} perPass_meshes;

layout(location = 0) out vec4 out_fragColor;

layout(location = 0) in flat uint tri_idx0;

void main() {
    Mesh meshData = perPass_meshes.data[tri_idx0];

    vec3 vertex_pos0 = vec3(meshData.positions.data[3 * tri_idx0],
                            meshData.positions.data[3 * tri_idx0 + 1],
                            meshData.positions.data[3 * tri_idx0 + 2]);
    
    out_fragColor = vec4(vertex_pos0, 1.0);
}
