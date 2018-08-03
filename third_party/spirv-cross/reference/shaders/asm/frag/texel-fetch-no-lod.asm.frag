#version 450

layout(binding = 0) uniform sampler2D uTexture;

layout(location = 0) out vec4 FragColor;

void main()
{
    FragColor = texelFetch(uTexture, ivec2(gl_FragCoord.xy), 0);
}

