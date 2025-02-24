#version 310 es

layout(binding = 0) uniform mediump sampler2D sTexture;

layout(location = 0) in mediump vec2 vTexCoords[4];

layout(location = 0) out mediump float oLuminance;

void main()
{
	mediump vec4 luminanceValues;
	luminanceValues.x = texture(sTexture, vTexCoords[0]).r;
	luminanceValues.y = texture(sTexture, vTexCoords[1]).r;
	luminanceValues.z = texture(sTexture, vTexCoords[2]).r;
	luminanceValues.w = texture(sTexture, vTexCoords[3]).r;

	oLuminance = (luminanceValues.x + luminanceValues.y + luminanceValues.z + luminanceValues.w) * 0.25;
}