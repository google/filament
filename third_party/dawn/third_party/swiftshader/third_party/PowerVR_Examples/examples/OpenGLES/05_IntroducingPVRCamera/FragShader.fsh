#if defined(ANDROID)

#extension GL_OES_EGL_image_external : require
uniform mediump samplerExternalOES Sampler;

#elif defined(IOS)

uniform mediump sampler2D SamplerY;
uniform mediump sampler2D SamplerUV;

#else

uniform mediump sampler2D Sampler;

#endif

varying mediump vec2 vTexCoord;

void main()
{
	mediump vec3 color = vec3 (0.,0.,0.);
	// This is the black borders if we move out of range
	if ((vTexCoord.x >=0.) && (vTexCoord.y >=0.) && (vTexCoord.x <=1.) && (vTexCoord.y <=1.))
	{
#if !defined(IOS)
		color = texture2D(Sampler, vTexCoord).rgb;
#else
		mediump vec3 yuv = vec3(0.,0.,0.);
		yuv.x  = texture2D(SamplerY,  vTexCoord).r;
		yuv.yz = texture2D(SamplerUV, vTexCoord).ra - vec2(0.5, 0.5);

		// BT.709 - Convert from yuv to rgb
		color = mat3(1.0,     1.0,      1.0,
					 0.0,     -.18732,  1.8556,
					 1.57481, -.46813,  0.0) * yuv;
#endif
	}
	//Any effect can be applied here - for example, a night vision effect...
	mediump float intensity = dot(vec3(0.30, 0.59, 0.11), color);
	gl_FragColor.xyz = intensity * vec3(0.2, 1.0, .2);
	gl_FragColor.w = 1.0;
	// NO gamma correction, the values from the camera coming in are sRGB values
}