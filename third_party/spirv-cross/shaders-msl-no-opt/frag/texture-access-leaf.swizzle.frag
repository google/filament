#version 450

layout(binding = 0) uniform sampler1D tex1d;
layout(binding = 1) uniform sampler2D tex2d;
layout(binding = 2) uniform sampler3D tex3d;
layout(binding = 3) uniform samplerCube texCube;
layout(binding = 4) uniform sampler2DArray tex2dArray;
layout(binding = 5) uniform samplerCubeArray texCubeArray;
layout(binding = 6) uniform samplerBuffer texBuffer;

layout(binding = 7) uniform sampler2DShadow depth2d;
layout(binding = 8) uniform samplerCubeShadow depthCube;
layout(binding = 9) uniform sampler2DArrayShadow depth2dArray;
layout(binding = 10) uniform samplerCubeArrayShadow depthCubeArray;

vec4 doSwizzle()
{
	// OpImageSampleImplicitLod
	vec4 c = texture(tex1d, 0.0);
	c = texture(tex2d, vec2(0.0, 0.0));
	c = texture(tex3d, vec3(0.0, 0.0, 0.0));
	c = texture(texCube, vec3(0.0, 0.0, 0.0));
	c = texture(tex2dArray, vec3(0.0, 0.0, 0.0));
	c = texture(texCubeArray, vec4(0.0, 0.0, 0.0, 0.0));

	// OpImageSampleDrefImplicitLod
	c.r = texture(depth2d, vec3(0.0, 0.0, 1.0));
	c.r = texture(depthCube, vec4(0.0, 0.0, 0.0, 1.0));
	c.r = texture(depth2dArray, vec4(0.0, 0.0, 0.0, 1.0));
	c.r = texture(depthCubeArray, vec4(0.0, 0.0, 0.0, 0.0), 1.0);

	// OpImageSampleProjImplicitLod
	c = textureProj(tex1d, vec2(0.0, 1.0));
	c = textureProj(tex2d, vec3(0.0, 0.0, 1.0));
	c = textureProj(tex3d, vec4(0.0, 0.0, 0.0, 1.0));

	// OpImageSampleProjDrefImplicitLod
	c.r = textureProj(depth2d, vec4(0.0, 0.0, 1.0, 1.0));

	// OpImageSampleExplicitLod
	c = textureLod(tex1d, 0.0, 0.0);
	c = textureLod(tex2d, vec2(0.0, 0.0), 0.0);
	c = textureLod(tex3d, vec3(0.0, 0.0, 0.0), 0.0);
	c = textureLod(texCube, vec3(0.0, 0.0, 0.0), 0.0);
	c = textureLod(tex2dArray, vec3(0.0, 0.0, 0.0), 0.0);
	c = textureLod(texCubeArray, vec4(0.0, 0.0, 0.0, 0.0), 0.0);

	// OpImageSampleDrefExplicitLod
	c.r = textureLod(depth2d, vec3(0.0, 0.0, 1.0), 0.0);

	// OpImageSampleProjExplicitLod
	c = textureProjLod(tex1d, vec2(0.0, 1.0), 0.0);
	c = textureProjLod(tex2d, vec3(0.0, 0.0, 1.0), 0.0);
	c = textureProjLod(tex3d, vec4(0.0, 0.0, 0.0, 1.0), 0.0);

	// OpImageSampleProjDrefExplicitLod
	c.r = textureProjLod(depth2d, vec4(0.0, 0.0, 1.0, 1.0), 0.0);

	// OpImageFetch
	c = texelFetch(tex1d, 0, 0);
	c = texelFetch(tex2d, ivec2(0, 0), 0);
	c = texelFetch(tex3d, ivec3(0, 0, 0), 0);
	c = texelFetch(tex2dArray, ivec3(0, 0, 0), 0);

	// Show that this transformation doesn't apply to Buffer images.
	c = texelFetch(texBuffer, 0);

	// OpImageGather
	c = textureGather(tex2d, vec2(0.0, 0.0), 0);
	c = textureGather(texCube, vec3(0.0, 0.0, 0.0), 1);
	c = textureGather(tex2dArray, vec3(0.0, 0.0, 0.0), 2);
	c = textureGather(texCubeArray, vec4(0.0, 0.0, 0.0, 0.0), 3);

	// OpImageDrefGather
	c = textureGather(depth2d, vec2(0.0, 0.0), 1.0);
	c = textureGather(depthCube, vec3(0.0, 0.0, 0.0), 1.0);
	c = textureGather(depth2dArray, vec3(0.0, 0.0, 0.0), 1.0);
	c = textureGather(depthCubeArray, vec4(0.0, 0.0, 0.0, 0.0), 1.0);

	return c;
}

void main()
{
	vec4 c = doSwizzle();
}
