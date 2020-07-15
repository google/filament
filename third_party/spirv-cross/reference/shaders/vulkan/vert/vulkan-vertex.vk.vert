#version 310 es
#ifdef GL_ARB_shader_draw_parameters
#extension GL_ARB_shader_draw_parameters : enable
#endif

#ifdef GL_ARB_shader_draw_parameters
#define SPIRV_Cross_BaseInstance gl_BaseInstanceARB
#else
uniform int SPIRV_Cross_BaseInstance;
#endif

void main()
{
    gl_Position = vec4(1.0, 2.0, 3.0, 4.0) * float(gl_VertexID + (gl_InstanceID + SPIRV_Cross_BaseInstance));
}

