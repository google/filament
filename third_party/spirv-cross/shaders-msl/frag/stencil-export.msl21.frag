#version 450
#extension GL_ARB_shader_stencil_export : require

layout(location = 0) out vec4 MRT0;
layout(location = 1) out vec4 MRT1;
void update_stencil()
{
	gl_FragStencilRefARB += 10;
}

void main()
{
	MRT0 = vec4(1.0);
	MRT1 = vec4(1.0, 0.0, 1.0, 1.0);
	gl_FragStencilRefARB = 100;
	update_stencil();
}
