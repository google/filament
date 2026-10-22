#version 450

layout(binding = 0) uniform sampler2D uTextures[2 * 3 * 1];

layout(location = 1) in vec2 vUV;
layout(location = 0) out vec4 FragColor;
layout(location = 0) flat in int vIndex;

void main()
{
    vec4 values3[2 * 3 * 1];
    for (int z = 0; z < 2; z++)
    {
        for (int y = 0; y < 3; y++)
        {
            for (int x = 0; x < 1; x++)
            {
                values3[z * 3 * 1 + y * 1 + x] = texture(uTextures[z * 3 * 1 + y * 1 + x], vUV);
            }
        }
    }
    FragColor = (values3[1 * 3 * 1 + 2 * 1 + 0] + values3[0 * 3 * 1 + 2 * 1 + 0]) + values3[(vIndex + 1) * 3 * 1 + 2 * 1 + vIndex];
}

