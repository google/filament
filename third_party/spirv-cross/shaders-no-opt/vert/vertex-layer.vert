#version 450
#extension GL_ARB_shader_viewport_layer_array : require

void main()
{
	gl_Layer = gl_InstanceIndex;
}
