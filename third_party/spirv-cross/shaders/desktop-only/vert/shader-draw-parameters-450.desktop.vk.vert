#version 450
#extension GL_ARB_shader_draw_parameters : enable

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    gl_Position = vec4(gl_BaseVertexARB, gl_BaseInstanceARB, gl_DrawIDARB, 1);
}
