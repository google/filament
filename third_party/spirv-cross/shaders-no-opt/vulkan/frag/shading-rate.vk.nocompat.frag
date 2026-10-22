#version 450
#extension GL_EXT_fragment_shading_rate : require

layout(location = 0) out uint FragColor;

void main()
{
	FragColor = gl_ShadingRateEXT;
}
