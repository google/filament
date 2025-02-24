uniform mediump sampler2D sTexture;

varying mediump vec4 fragColor;
varying mediump vec2 texCoordOut;

void main(void)
{
	mediump vec4 texColor = texture2D(sTexture, texCoordOut);
	gl_FragColor = vec4(fragColor.rgb * texColor.r, texColor.a);
}