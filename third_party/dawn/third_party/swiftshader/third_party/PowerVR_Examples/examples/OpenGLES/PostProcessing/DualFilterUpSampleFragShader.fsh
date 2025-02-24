#version 310 es

layout(binding = 0) uniform mediump sampler2D sTexture;
layout(location = 0) in mediump vec2 vTexCoords[9];
layout(location = 0) out mediump float oColor;

void main()
{
	oColor = texture(sTexture, vTexCoords[0]).r;
	oColor += texture(sTexture, vTexCoords[1]).r * 2.0;
	oColor += texture(sTexture, vTexCoords[2]).r;
	oColor += texture(sTexture, vTexCoords[3]).r * 2.0;
	oColor += texture(sTexture, vTexCoords[4]).r;
	oColor += texture(sTexture, vTexCoords[5]).r * 2.0;
	oColor += texture(sTexture, vTexCoords[6]).r;
	oColor += texture(sTexture, vTexCoords[7]).r * 2.0;
	oColor *= 0.08333333333333333333333333333333;
}