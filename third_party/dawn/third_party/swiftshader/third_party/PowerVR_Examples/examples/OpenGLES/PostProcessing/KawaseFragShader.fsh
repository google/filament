#version 310 es

layout(binding = 0) uniform mediump sampler2D sTexture;
layout(location = 0) in mediump vec2 vTexCoords[4];
layout(location = 0) out mediump float oColor;

void main()
{
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Developed by Masaki Kawase, Bunkasha Games
	// Used in DOUBLE-S.T.E.A.L. (aka Wreckless)
	// From his GDC2003 Presentation: Frame Buffer Postprocessing Effects in  DOUBLE-S.T.E.A.L (Wreckless)
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	mediump float cOut = 0.0;
	cOut += texture(sTexture, vTexCoords[0]).r;
	cOut += texture(sTexture, vTexCoords[1]).r;
	cOut += texture(sTexture, vTexCoords[2]).r;
	cOut += texture(sTexture, vTexCoords[3]).r;
	cOut *= 0.25;

	oColor = cOut;
}