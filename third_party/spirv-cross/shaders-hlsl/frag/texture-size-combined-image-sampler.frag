#version 450
layout(set = 0, binding = 0) uniform texture2D uTex;
layout(set = 0, binding = 1) uniform sampler uSampler;
layout(location = 0) out ivec2 FooOut;

void main()
{
	FooOut = textureSize(sampler2D(uTex, uSampler), 0);
}
