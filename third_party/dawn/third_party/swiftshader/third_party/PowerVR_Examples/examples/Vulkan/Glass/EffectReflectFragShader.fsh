#version 320 es

layout(set = 0, binding = 1) uniform mediump sampler2D sParaboloids;
layout(set = 0, binding = 2) uniform mediump samplerCube sSkybox;
layout(location = 0) in highp vec3 ReflectDir;
layout(location = 0) out mediump vec4 outColor;
void main()
{
	// Sample reflection to skybox
	mediump vec4 ReflectSky = texture(sSkybox, ReflectDir);

	highp vec3 vkReflectDir = ReflectDir;
	vkReflectDir.y = -vkReflectDir.y;
	highp vec3 Normalised = normalize(vkReflectDir);
	Normalised.xy /= abs(Normalised.z) + 1.0;
	Normalised.xy = Normalised.xy * 0.495 + 0.5;
	Normalised.x *= 0.5;
	Normalised.x += sign(-Normalised.z) * 0.25 + 0.25;
	// Sample reflection to paraboloids
	mediump vec4 Reflection = texture(sParaboloids, Normalised.xy);
	// Combine skybox reflection with paraboloid reflection
	Reflection.rgb = mix(ReflectSky.rgb, Reflection.rgb, Reflection.a);
	outColor.rgb = Reflection.rgb;
	outColor.a = 1.0;
}
