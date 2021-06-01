#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec2 vUV;
layout(location = 1) flat in int vIndex;

layout(set = 0, binding = 0) uniform texture2D uTex[];
layout(set = 1, binding = 0) uniform sampler Immut;

void main()
{
	FragColor = texture(nonuniformEXT(sampler2D(uTex[vIndex], Immut)), vUV);
}
