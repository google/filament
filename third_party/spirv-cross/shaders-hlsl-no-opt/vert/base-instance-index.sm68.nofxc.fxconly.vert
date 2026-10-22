#version 450
#extension GL_ARB_shader_draw_parameters : require

void main()
{
	gl_Position = vec4(gl_BaseInstanceARB);
}
