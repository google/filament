#version 450

layout(constant_id = 0) const int arraySize = 3;

layout(binding = 0, rgba32i) uniform iimage2D images[arraySize];

layout(binding = 4) uniform constant_block
{
	vec4 foo;
	int bar;
} constants[4];

layout(binding = 8) buffer storage_block
{
	uvec4 baz;
	ivec2 quux;
} storage[2];

void main()
{
	storage[0].baz = uvec4(constants[3].foo);
	storage[1].quux = imageLoad(images[2], ivec2(constants[1].bar)).xy;
}
