uniform mediump sampler2D sTexture;

varying mediump float LightIntensity;
varying mediump vec2 TexCoord;

void main()
{
	mediump vec3 outColor = texture2D(sTexture, TexCoord).rgb * LightIntensity;
#ifndef FRAMEBUFFER_SRGB
	outColor = pow(outColor, vec3(0.4545454545)); // Do gamma correction
#endif
	gl_FragColor = vec4(outColor, 1.0);
}