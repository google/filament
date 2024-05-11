#version 450

layout(binding = 0) uniform sampler1D tex1d;
layout(binding = 1) uniform texture2D tex2d;
layout(binding = 2) uniform sampler3D tex3d;
layout(binding = 3) uniform textureCube texCube;
layout(binding = 4) uniform sampler2DArray tex2dArray;
layout(binding = 5) uniform samplerCubeArray texCubeArray;
layout(binding = 6) uniform samplerBuffer texBuffer;

layout(binding = 7) uniform sampler2DShadow depth2d;
layout(binding = 8) uniform samplerCubeShadow depthCube;
layout(binding = 9) uniform texture2DArray depth2dArray;
layout(binding = 10) uniform samplerCubeArrayShadow depthCubeArray;

layout(binding = 11) uniform sampler defaultSampler;
layout(binding = 12) uniform samplerShadow shadowSampler;

layout(location = 0) out vec4 fragColor;

vec4 do_samples(sampler1D t1, texture2D t2, sampler3D t3, textureCube tc, sampler2DArray t2a, samplerCubeArray tca, samplerBuffer tb, sampler2DShadow d2, samplerCubeShadow dc, texture2DArray d2a, samplerCubeArrayShadow dca)
{
	// OpImageSampleImplicitLod
	vec4 c = texture(t1, 0.0);
	c = texture(sampler2D(t2, defaultSampler), vec2(0.0, 0.0));
	c = texture(t3, vec3(0.0, 0.0, 0.0));
	c = texture(samplerCube(tc, defaultSampler), vec3(0.0, 0.0, 0.0));
	c = texture(t2a, vec3(0.0, 0.0, 0.0));
	c = texture(tca, vec4(0.0, 0.0, 0.0, 0.0));

	// OpImageSampleDrefImplicitLod
	c.r = texture(d2, vec3(0.0, 0.0, 1.0));
	c.r = texture(dc, vec4(0.0, 0.0, 0.0, 1.0));
	c.r = texture(sampler2DArrayShadow(d2a, shadowSampler), vec4(0.0, 0.0, 0.0, 1.0));
	c.r = texture(dca, vec4(0.0, 0.0, 0.0, 0.0), 1.0);

	// OpImageSampleProjImplicitLod
	c = textureProj(t1, vec2(0.0, 1.0));
	c = textureProj(sampler2D(t2, defaultSampler), vec3(0.0, 0.0, 1.0));
	c = textureProj(t3, vec4(0.0, 0.0, 0.0, 1.0));

	// OpImageSampleProjDrefImplicitLod
	c.r = textureProj(d2, vec4(0.0, 0.0, 1.0, 1.0));

	// OpImageSampleExplicitLod
	c = textureLod(t1, 0.0, 0.0);
	c = textureLod(sampler2D(t2, defaultSampler), vec2(0.0, 0.0), 0.0);
	c = textureLod(t3, vec3(0.0, 0.0, 0.0), 0.0);
	c = textureLod(samplerCube(tc, defaultSampler), vec3(0.0, 0.0, 0.0), 0.0);
	c = textureLod(t2a, vec3(0.0, 0.0, 0.0), 0.0);
	c = textureLod(tca, vec4(0.0, 0.0, 0.0, 0.0), 0.0);

	// OpImageSampleDrefExplicitLod
	c.r = textureLod(d2, vec3(0.0, 0.0, 1.0), 0.0);

	// OpImageSampleProjExplicitLod
	c = textureProjLod(t1, vec2(0.0, 1.0), 0.0);
	c = textureProjLod(sampler2D(t2, defaultSampler), vec3(0.0, 0.0, 1.0), 0.0);
	c = textureProjLod(t3, vec4(0.0, 0.0, 0.0, 1.0), 0.0);

	// OpImageSampleProjDrefExplicitLod
	c.r = textureProjLod(d2, vec4(0.0, 0.0, 1.0, 1.0), 0.0);

	// OpImageFetch
	c = texelFetch(t1, 0, 0);
	c = texelFetch(sampler2D(t2, defaultSampler), ivec2(0, 0), 0);
	c = texelFetch(t3, ivec3(0, 0, 0), 0);
	c = texelFetch(t2a, ivec3(0, 0, 0), 0);

	// Show that this transformation doesn't apply to Buffer images.
	c = texelFetch(tb, 0);

	// OpImageGather
	c = textureGather(sampler2D(t2, defaultSampler), vec2(0.0, 0.0), 0);
	c = textureGather(samplerCube(tc, defaultSampler), vec3(0.0, 0.0, 0.0), 1);
	c = textureGather(t2a, vec3(0.0, 0.0, 0.0), 2);
	c = textureGather(tca, vec4(0.0, 0.0, 0.0, 0.0), 3);

	// OpImageDrefGather
	c = textureGather(d2, vec2(0.0, 0.0), 1.0);
	c = textureGather(dc, vec3(0.0, 0.0, 0.0), 1.0);
	c = textureGather(sampler2DArrayShadow(d2a, shadowSampler), vec3(0.0, 0.0, 0.0), 1.0);
	c = textureGather(dca, vec4(0.0, 0.0, 0.0, 0.0), 1.0);
	return c;
}

void main()
{
	fragColor = do_samples(tex1d, tex2d, tex3d, texCube, tex2dArray, texCubeArray, texBuffer, depth2d, depthCube, depth2dArray, depthCubeArray);
}
