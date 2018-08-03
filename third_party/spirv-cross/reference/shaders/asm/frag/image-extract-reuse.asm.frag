#version 450

layout(binding = 0) uniform sampler2D uTexture;

layout(location = 0) out ivec2 Size;

void main()
{
    Size = textureSize(uTexture, 0) + textureSize(uTexture, 1);
}

