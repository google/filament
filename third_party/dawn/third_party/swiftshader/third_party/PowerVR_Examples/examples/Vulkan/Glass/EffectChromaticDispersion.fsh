#version 320 es

layout(set = 0, binding = 1) uniform mediump sampler2D sParaboloids;
layout(set = 0, binding = 2) uniform mediump samplerCube sSkybox;

layout(location = 0) in highp vec3 RefractDirRed;
layout(location = 1) in highp vec3 RefractDirGreen;
layout(location = 2) in highp vec3 RefractDirBlue;

layout(location = 0) out mediump vec4 outColor;

void main()
{
	// Sample refraction to skybox
	mediump vec4 RefractSky;
	RefractSky.r = texture(sSkybox, RefractDirRed).r;
	RefractSky.g = texture(sSkybox, RefractDirGreen).g;
	RefractSky.b = texture(sSkybox, RefractDirBlue).b;
	
	// Sample refraction to paraboloids
	mediump vec4 Refraction;

	// Red
	highp vec3 vkRefractDirRed = RefractDirRed;
	vkRefractDirRed.y = -vkRefractDirRed.y;
	highp vec3 Normalised = normalize(vkRefractDirRed);
	Normalised.xy /= abs(Normalised.z) + 1.0;
	Normalised.xy = Normalised.xy * 0.495 + 0.5;
	Normalised.x *= 0.5;
	Normalised.x += sign(-Normalised.z) * 0.25 + 0.25;
	mediump vec4 RefractRed = texture(sParaboloids, Normalised.xy);

	Refraction.r = mix(RefractSky.r, RefractRed.r, RefractRed.a);

	// Green
	highp vec3 vkRefractDirGreen = RefractDirGreen;
	vkRefractDirGreen.y = -vkRefractDirGreen.y;
	Normalised = normalize(vkRefractDirGreen);
	Normalised.xy /= abs(Normalised.z) + 1.0;
	Normalised.xy = Normalised.xy * 0.495 + 0.5;
	Normalised.x *= 0.5;
	Normalised.x += sign(-Normalised.z) * 0.25 + 0.25;
	mediump vec4 RefractGreen = texture(sParaboloids, Normalised.xy);

	Refraction.g = mix(RefractSky.g, RefractGreen.g, RefractGreen.a);

	// Blue
	highp vec3 vkRefractDirBlue = RefractDirBlue;
	vkRefractDirBlue.y = -vkRefractDirBlue.y;
	Normalised = normalize(vkRefractDirBlue);
	Normalised.xy /= abs(Normalised.z) + 1.0;
	Normalised.xy = Normalised.xy * 0.495 + 0.5;
	Normalised.x *= 0.5;
	Normalised.x += sign(-Normalised.z) * 0.25 + 0.25;
	mediump vec4 RefractBlue = texture(sParaboloids, Normalised.xy);
	Refraction.b = mix(RefractSky.b, RefractBlue.b, RefractBlue.a);
	outColor.rgb = Refraction.rgb;
	outColor.a = 1.0;
}
