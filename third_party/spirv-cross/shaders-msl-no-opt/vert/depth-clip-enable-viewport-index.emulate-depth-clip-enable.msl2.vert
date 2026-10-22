#version 450
#extension GL_ARB_shader_viewport_layer_array : require

void main()
{
	gl_Position = vec4(0.0, 0.0, 0.25, 1.0);
	gl_ViewportIndex = 2;
}
