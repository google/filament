#version 320 es

layout(set = 0, binding = 0) uniform mediump samplerCube sSkybox;
layout(location = 0) in mediump vec3 RayDir;
layout(location = 0) out mediump vec4 outColor;

void main()
{
	// Sample skybox cube map
	outColor = texture(sSkybox, RayDir);
}
