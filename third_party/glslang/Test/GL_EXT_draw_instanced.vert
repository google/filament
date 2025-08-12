#version 120
#extension GL_EXT_draw_instanced : require
#define ID gl_InstanceID

uniform mat4 gtf_ModelViewProjectionMatrix;
uniform vec3 instanceOffsets[3];
uniform vec4 va[gl_MaxVertexAttribs];
vec4 color;

void main (void)
{
        vec4 vertex = vec4(va[0].xy / 3.0, va[0].zw) + vec4(instanceOffsets[ID], 1.0);
        color = vec4(0, 0, 0, 0);
        for (int i = 1; i < gl_MaxVertexAttribs; i++)
                color += va[i];
        gl_Position = gtf_ModelViewProjectionMatrix * vertex;
        gl_PointSize = 1.0;
}