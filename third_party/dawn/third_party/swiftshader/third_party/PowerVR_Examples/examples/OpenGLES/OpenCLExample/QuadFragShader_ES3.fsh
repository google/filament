#version 300 es
uniform mediump sampler2D sTexture;
in mediump vec2 TexCoord;

layout(location = 0) out mediump vec4 oColor;

void main()
{
	mediump vec3 color = texture(sTexture, TexCoord).rgb;
#ifndef FRAMEBUFFER_SRGB
	color = pow(color, vec3(0.4545454545)); // Do gamma correction
#endif	
	oColor = vec4(color, 1.);
}
