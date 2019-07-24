#version 450
#extension GL_EXT_scalar_block_layout : require

layout(std430, binding = 0) uniform UBO
{
	float a[1];
	vec2 b[2];
};

layout(std430, binding = 1) uniform UBOEnhancedLayout
{
	float c[1];
	vec2 d[2];
	layout(offset = 10000) float e;
};

layout(location = 0) flat in int vIndex;
layout(location = 0) out float FragColor;

void main()
{
	FragColor = a[vIndex] + c[vIndex] + e;
}
