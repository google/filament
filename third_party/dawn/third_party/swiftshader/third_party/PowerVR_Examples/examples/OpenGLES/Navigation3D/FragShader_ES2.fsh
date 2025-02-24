varying mediump vec4 fragColor;

void main(void)
{
	mediump vec3 color = fragColor.rgb;

#ifdef GAMMA_CORRECTION
	color = pow(color, vec3(0.4545454545)); // Do gamma correction
#endif
	gl_FragColor = vec4(color, 1.0);
}