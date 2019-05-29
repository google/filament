#version 450

layout(binding = 0) uniform texture2DArray uTex;
layout(binding = 1) uniform samplerShadow uShadow;
layout(location = 0) in vec4 vUV;
layout(location = 0) out float FragColor;

void main()
{
	FragColor = textureGrad(sampler2DArrayShadow(uTex, uShadow), vUV, vec2(0.0), vec2(0.0));
}
