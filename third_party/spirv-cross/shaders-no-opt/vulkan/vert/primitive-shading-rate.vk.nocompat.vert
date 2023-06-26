#version 450
#extension GL_EXT_fragment_shading_rate : require

void main()
{
	gl_PrimitiveShadingRateEXT = 3;
	gl_Position = vec4(1.0);
}
