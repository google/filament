#version 450
#extension GL_ARB_shader_viewport_layer_array : require

layout(location = 0) in vec4 coord;

void main()
{
	gl_Position = coord;
	gl_ViewportIndex = int(coord.z);
}
