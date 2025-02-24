#version 310 es

layout (binding = 9) uniform samplerCube skyboxMap;
layout (location = 0) out mediump vec4 outColor;

layout(location = 0) in mediump vec3 inRayDir;

layout (location = 3) uniform mediump float exposure;

void main()
{
	// Sample skybox cube map
	mediump vec3 RayDir = inRayDir;
    
	mediump vec4 color = texture(skyboxMap, RayDir);
	
	// This seemingly strange clamp is to ensure that the final colour stays within the constraints
	// of 16-bit floats (~640000) with a bit to spare, as the tone mapping calculations squares 
	// this number. Taking account compiler optimisations which may overflow intermediate results
	// into undefined values, we should clamp this value so that its maximum is safely lower than that.
	// It does not affect the final image otherwise, as the clamp will only bring the value to
	// around ~50, which is already saturated.
	// It is important to remember that this clamping must only happen last minute, JUST before tone mapping,
	// as we would want to have the full brightness available for post processing calculations (e.g. bloom)

	mediump vec3 toneMappedColor = min(color.rgb, 50. / exposure);
	toneMappedColor *= exposure;

	
	// http://filmicworlds.com/blog/filmic-tonemapping-operators/
	// Our favourite is the optimized formula by Jim Hejl and Richard Burgess-Dawson
	// We particularly like its high contrast and the fact that it is very cheap, with
	// only 4 mads and a reciprocal.
	mediump vec3 x = max(vec3(0.), toneMappedColor - vec3(0.004));
	toneMappedColor = (x * (6.2 * x + .49)) / (x * (6.175 * x + 1.7) + 0.06);

	outColor = vec4(toneMappedColor, 1.0);
}
