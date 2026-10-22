#version 450

layout(binding = 0) uniform isampler1D tex1d;
layout(binding = 1) uniform isampler2D tex2d;
layout(binding = 2) uniform isampler3D tex3d;
layout(binding = 3) uniform isamplerCube texCube;
layout(binding = 4) uniform isampler2DArray tex2dArray;
layout(binding = 5) uniform isamplerCubeArray texCubeArray;
layout(binding = 6) uniform isamplerBuffer texBuffer;

void main()
{
	// OpImageSampleImplicitLod
	vec4 c = texture(tex1d, 0.0);
	c = texture(tex2d, vec2(0.0, 0.0));
	c = texture(tex3d, vec3(0.0, 0.0, 0.0));
	c = texture(texCube, vec3(0.0, 0.0, 0.0));
	c = texture(tex2dArray, vec3(0.0, 0.0, 0.0));
	c = texture(texCubeArray, vec4(0.0, 0.0, 0.0, 0.0));

	// OpImageSampleProjImplicitLod
	c = textureProj(tex1d, vec2(0.0, 1.0));
	c = textureProj(tex2d, vec3(0.0, 0.0, 1.0));
	c = textureProj(tex3d, vec4(0.0, 0.0, 0.0, 1.0));

	// OpImageSampleExplicitLod
	c = textureLod(tex1d, 0.0, 0.0);
	c = textureLod(tex2d, vec2(0.0, 0.0), 0.0);
	c = textureLod(tex3d, vec3(0.0, 0.0, 0.0), 0.0);
	c = textureLod(texCube, vec3(0.0, 0.0, 0.0), 0.0);
	c = textureLod(tex2dArray, vec3(0.0, 0.0, 0.0), 0.0);
	c = textureLod(texCubeArray, vec4(0.0, 0.0, 0.0, 0.0), 0.0);

	// OpImageSampleProjExplicitLod
	c = textureProjLod(tex1d, vec2(0.0, 1.0), 0.0);
	c = textureProjLod(tex2d, vec3(0.0, 0.0, 1.0), 0.0);
	c = textureProjLod(tex3d, vec4(0.0, 0.0, 0.0, 1.0), 0.0);

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
}
