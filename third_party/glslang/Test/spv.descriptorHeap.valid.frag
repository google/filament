#version 460 core

#extension GL_EXT_descriptor_heap : enable
#extension GL_EXT_nonuniform_qualifier : enable

// Sampler array aliased to the sampler heap
uniform sampler heapSampler[];

// Different image arrays aliased to the image heap
uniform texture2D heapTexture2D[];
uniform texture3D heapTexture3D[];

// Different buffer arrays aliased to the buffer heap
buffer StorageBufferA {
    vec4 a;
} heapStorageBufferA[];
buffer StorageBufferB {
    vec3 b;
} heapStorageBufferB[];
uniform UniformBuffer {
    vec4 colorOffset;
} heapUniformBuffer[];

layout (location = 0) in vec2 uvs;
layout (location = 1) flat in uint index;

layout (location = 0) out vec4 fragColor;

void main()
{
    fragColor = texture(sampler2D(heapTexture2D[27], heapSampler[0]), uvs);
    fragColor += heapUniformBuffer[nonuniformEXT(index)].colorOffset;
    heapStorageBufferA[1].a = vec4(1, 2, 3, 4);
    fragColor.x += heapStorageBufferB[nonuniformEXT(index)].b.x;
}
