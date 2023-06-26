#version 450

layout(binding = 0) uniform sampler1D tex1d;
layout(binding = 1) uniform sampler2D tex2d;
layout(binding = 2) uniform sampler3D tex3d;
layout(binding = 3) uniform samplerCube texCube;

layout(binding = 4) uniform sampler1DShadow tex1dShadow;
layout(binding = 5) uniform sampler2DShadow tex2dShadow;
layout(binding = 6) uniform samplerCubeShadow texCubeShadow;

layout(binding = 7) uniform sampler1DArray tex1dArray;
layout(binding = 8) uniform sampler2DArray tex2dArray;
layout(binding = 9) uniform samplerCubeArray texCubeArray;

layout(binding = 10) uniform samplerShadow samplerDepth;
layout(binding = 11) uniform sampler samplerNonDepth;
layout(binding = 12) uniform texture2D separateTex2d;
layout(binding = 13) uniform texture2D separateTex2dDepth;

layout(location = 0) in float texCoord1d;
layout(location = 1) in vec2 texCoord2d;
layout(location = 2) in vec3 texCoord3d;
layout(location = 3) in vec4 texCoord4d;

layout(location = 0) out vec4 FragColor;

void main()
{
	vec4 texcolor = texture(tex1d, texCoord1d);
	texcolor += textureOffset(tex1d, texCoord1d, 1);
	texcolor += textureLod(tex1d, texCoord1d, 2);
	texcolor += textureGrad(tex1d, texCoord1d, 1.0, 2.0);
	texcolor += textureProj(tex1d, vec2(texCoord1d, 2.0));
	texcolor += texture(tex1d, texCoord1d, 1.0);

	texcolor += texture(tex2d, texCoord2d);
	texcolor += textureOffset(tex2d, texCoord2d, ivec2(1, 2));
	texcolor += textureLod(tex2d, texCoord2d, 2);
	texcolor += textureGrad(tex2d, texCoord2d, vec2(1.0, 2.0), vec2(3.0, 4.0));
	texcolor += textureProj(tex2d, vec3(texCoord2d, 2.0));
	texcolor += texture(tex2d, texCoord2d, 1.0);

	texcolor += texture(tex3d, texCoord3d);
	texcolor += textureOffset(tex3d, texCoord3d, ivec3(1, 2, 3));
	texcolor += textureLod(tex3d, texCoord3d, 2);
	texcolor += textureGrad(tex3d, texCoord3d, vec3(1.0, 2.0, 3.0), vec3(4.0, 5.0, 6.0));
	texcolor += textureProj(tex3d, vec4(texCoord3d, 2.0));
	texcolor += texture(tex3d, texCoord3d, 1.0);

	texcolor += texture(texCube, texCoord3d);
	texcolor += textureLod(texCube, texCoord3d, 2);
	texcolor += texture(texCube, texCoord3d, 1.0);

	texcolor.a += texture(tex1dShadow, vec3(texCoord1d, 0.0, 0.0));
	texcolor.a += texture(tex2dShadow, vec3(texCoord2d, 0.0));
	texcolor.a += texture(texCubeShadow, vec4(texCoord3d, 0.0));

	texcolor += texture(tex1dArray, texCoord2d);
	texcolor += texture(tex2dArray, texCoord3d);
	texcolor += texture(texCubeArray, texCoord4d);

	texcolor += textureGather(tex2d, texCoord2d);
	texcolor += textureGather(tex2d, texCoord2d, 0);
	texcolor += textureGather(tex2d, texCoord2d, 1);
	texcolor += textureGather(tex2d, texCoord2d, 2);
	texcolor += textureGather(tex2d, texCoord2d, 3);

	texcolor += textureGatherOffset(tex2d, texCoord2d, ivec2(1, 1));
	texcolor += textureGatherOffset(tex2d, texCoord2d, ivec2(1, 1), 0);
	texcolor += textureGatherOffset(tex2d, texCoord2d, ivec2(1, 1), 1);
	texcolor += textureGatherOffset(tex2d, texCoord2d, ivec2(1, 1), 2);
	texcolor += textureGatherOffset(tex2d, texCoord2d, ivec2(1, 1), 3);

	texcolor += texelFetch(tex2d, ivec2(1, 2), 0);

	texcolor += texture(sampler2D(separateTex2d, samplerNonDepth), texCoord2d);
	texcolor.a += texture(sampler2DShadow(separateTex2dDepth, samplerDepth), texCoord3d);

	FragColor = texcolor;
}
