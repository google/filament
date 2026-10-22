#version 460
#ifdef GL_ARB_shader_draw_parameters
#extension GL_ARB_shader_draw_parameters : enable
#endif

#ifdef GL_ARB_shader_draw_parameters
#define SPIRV_Cross_BaseVertex gl_BaseVertexARB
#else
uniform int SPIRV_Cross_BaseVertex;
#endif
#ifdef GL_ARB_shader_draw_parameters
#define SPIRV_Cross_BaseInstance gl_BaseInstanceARB
#else
uniform int SPIRV_Cross_BaseInstance;
#endif
#ifndef GL_ARB_shader_draw_parameters
#error GL_ARB_shader_draw_parameters is not supported.
#endif

void main()
{
    gl_Position = vec4(float(SPIRV_Cross_BaseVertex), float(SPIRV_Cross_BaseInstance), float(gl_DrawIDARB), 1.0);
}

