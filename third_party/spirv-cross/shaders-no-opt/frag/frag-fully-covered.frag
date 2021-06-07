#version 450
#extension GL_NV_conservative_raster_underestimation : require

layout(location = 0) out vec4 FragColor;

void main()
{
	if (!gl_FragFullyCoveredNV)
		discard;
	FragColor = vec4(1.0);
}
