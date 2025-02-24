#version 320 es

layout(set = 0, binding = 0) uniform mediump sampler2D sTexture;
	
layout(location = 0) in mediump float LightIntensity;
layout(location = 1) in mediump vec2 TexCoord;
layout(location = 0) out mediump vec4 oColor;

void main()
{
	oColor.rgb = texture(sTexture, TexCoord).rgb * LightIntensity;
	oColor.a = 1.0;
}