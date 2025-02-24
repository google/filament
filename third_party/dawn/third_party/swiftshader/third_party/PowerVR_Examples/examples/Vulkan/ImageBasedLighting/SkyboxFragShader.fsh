#version 320 es

layout(set = 0, binding = 0) uniform mediump samplerCube skybox;
layout(location = 0) in mediump vec3 RayDir;
layout(location = 0) out mediump vec4 outColor;

const highp  float ExposureBias = 1.0;

layout(std140, set = 0, binding = 1) uniform Dynamic
{
	highp mat4 InvVPMatrix;
	highp vec4 EyePos;
	highp float exposure;
};

void main()
{
	mediump vec3 toneMappedColor = min(texture(skybox, RayDir).rgb, 50. / exposure);
	toneMappedColor *= exposure;

	// http://filmicworlds.com/blog/filmic-tonemapping-operators/
	// Our favorite is the optimized formula by Jim Hejl and Richard Burgess-Dawson
	// We particularly like its high contrast and the fact that it is very cheap, with
	// only 4 mads and a reciprocal.
	mediump vec3 x = max(vec3(0.), toneMappedColor - vec3(0.004));
	toneMappedColor = (x * (6.2 * x + .49)) / (x * (6.175 * x + 1.7) + 0.06);

	outColor = vec4(toneMappedColor, 1.0);
}
