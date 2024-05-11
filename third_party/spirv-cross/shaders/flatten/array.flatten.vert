#version 310 es

layout(std140) uniform UBO
{
    vec4 A4[5][4][2];
    mat4 uMVP;
    vec4 A1[2];
    vec4 A2[2][3];
    float A3[3];
    vec4 Offset;
};
layout(location = 0) in vec4 aVertex;

void main()
{
    vec4 a4 = A4[2][3][1]; // 2 * (4 * 2) + 3 * 2 + 1 = 16 + 6 + 1 = 23.
    vec4 offset = A2[1][1] + A1[1] + A3[2];
    gl_Position = uMVP * aVertex + Offset + offset;
}
