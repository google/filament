#version 450
#extension GL_ARB_shader_viewport_layer_array : require

layout(location = 0) out vec4 color;

void main()
{
	color = vec4(gl_ViewportIndex);
	gl_FragDepth = 1.25;
}
