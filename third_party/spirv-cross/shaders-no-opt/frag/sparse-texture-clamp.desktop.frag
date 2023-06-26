#version 450
#extension GL_ARB_sparse_texture2 : require
#extension GL_ARB_sparse_texture_clamp : require

layout(set = 0, binding = 0) uniform sampler2D uSamp;
layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec2 vUV;

void main()
{
	vec4 texel;
	int code;

	code = sparseTextureClampARB(uSamp, vUV, 1.0, texel, 2.0);
	texel = textureClampARB(uSamp, vUV, 1.0, 2.0);
	code = sparseTextureOffsetClampARB(uSamp, vUV, ivec2(1, 2), 1.0, texel, 2.0);
	texel = textureOffsetClampARB(uSamp, vUV, ivec2(1, 2), 1.0, 2.0);
	code = sparseTextureGradClampARB(uSamp, vUV, vec2(1.0), vec2(2.0), 1.0, texel);
	texel = textureGradClampARB(uSamp, vUV, vec2(1.0), vec2(2.0), 1.0);
	code = sparseTextureGradOffsetClampARB(uSamp, vUV, vec2(1.0), vec2(2.0), ivec2(-1, -2), 1.0, texel);
	texel = textureGradOffsetClampARB(uSamp, vUV, vec2(1.0), vec2(2.0), ivec2(-1, -2), 1.0);
}

