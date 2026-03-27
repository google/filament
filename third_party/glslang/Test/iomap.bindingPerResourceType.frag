#version 460

layout(std140) uniform UBO1 {
    vec4 a;
};

layout(std430) buffer SSBO1 {
    float c[40];
};

layout(rgba8) uniform image2D Image1;

uniform sampler2D Sampler1;

layout(std140) uniform UBO2 {
    vec4 b;
};

layout(std430) buffer SSBO2 {
    vec2 d[];
};

layout(rgba8) uniform image2D Image2;

uniform sampler2D Sampler2;

layout(location = 0) out vec4 Output;

void main()
{
    vec4 result = vec4(0.0);
    result += a;
    result += b;
    result.x += c[5];
    result.yz += d[5];
    result += imageLoad(Image1, ivec2(5, 10));
    result += imageLoad(Image2, ivec2(5, 10));
    result += texture(Sampler1, vec2(0.5, 0.5));
    result += texture(Sampler2, vec2(0.5, 0.5));
    Output = result;
}
