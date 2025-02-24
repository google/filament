#version 320 es

layout(set = 0, binding = 1) uniform mediump sampler2D sParaboloids;
layout(set = 0, binding = 2) uniform mediump samplerCube sSkybox;
layout(location = 0) in highp vec3 RefractDir;
layout(location = 0) out mediump vec4 outColor;

void main()
{
	mediump vec4 RefractSky = texture(sSkybox, RefractDir);

	highp vec3 vkRefractDir = RefractDir;
	vkRefractDir.y = -vkRefractDir.y;
	highp vec3 Normalised = normalize(vkRefractDir);
	Normalised.xy /= abs(Normalised.z) + 1.0;
	Normalised.xy = Normalised.xy * 0.495 + 0.5;
	Normalised.x *= 0.5;
	Normalised.x += sign(-Normalised.z) * 0.25 + 0.25;
	mediump vec4 Refraction = texture(sParaboloids, Normalised.xy);

	Refraction.rgb = mix(RefractSky.rgb, Refraction.rgb, Refraction.a);
	outColor.rgb = Refraction.rgb;
	outColor.a = 1.0;
}
