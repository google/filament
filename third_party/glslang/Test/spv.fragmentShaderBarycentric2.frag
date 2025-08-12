#version 320 es
#extension GL_NV_fragment_shader_barycentric : require

precision highp float;

layout(location = 0) pervertexNV in float vertexIDs[3];
layout(location = 1) pervertexNV in float vertexIDs2[3];

      
layout(location = 1) out float value;
      
void main () {
    value = (gl_BaryCoordNoPerspNV.x * vertexIDs[0] +
             gl_BaryCoordNoPerspNV.y * vertexIDs[1] +
             gl_BaryCoordNoPerspNV.z * vertexIDs[2]);

    value += (gl_BaryCoordNoPerspNV.x * vertexIDs2[0] +
             gl_BaryCoordNoPerspNV.y * vertexIDs2[1] +
             gl_BaryCoordNoPerspNV.z * vertexIDs2[2]);

}
