#version 450

layout(binding = 0, std140) uniform uBuffer
{
    vec4 color;
} x_12;

layout(location = 0) out vec4 fragColor;
const vec4 _2_init = vec4(0.0);

void main()
{
    fragColor = _2_init;
    gl_SampleMask[0] = 0;
    fragColor = x_12.color;
    gl_SampleMask[0] = int(uint(6));
    gl_SampleMask[0] = int(uint(gl_SampleMask[0]));
    uint _30_unrolled[1];
    for (int i = 0; i < int(1); i++)
    {
        _30_unrolled[i] = int(gl_SampleMask[i]);
    }
    for (int i = 0; i < int(1); i++)
    {
        gl_SampleMask[i] = int(_30_unrolled[i]);
    }
}

