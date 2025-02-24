#version 320 es

layout(location = 0) out mediump vec4 oColor;

layout(location = 0) in mediump vec4 fragColor;

layout(constant_id = 0) const int DoGammaCorrection = 0;

void main(void)
{
	mediump vec3 color = fragColor.rgb;
	if(DoGammaCorrection == 1)
	{
		color = pow(color, vec3(0.4545454545)); // Do gamma correction
	}
	oColor = vec4(color.rgb, 1.0);
}