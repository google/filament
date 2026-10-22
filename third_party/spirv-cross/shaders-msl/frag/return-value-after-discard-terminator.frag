#version 450

layout(set = 0, binding = 0, std430) buffer buff_t
{
    int m0[1024];
} buff;

layout(location = 0) out vec4 frag_clr;

void main()
{
    ivec2 frag_coord = ivec2(ivec4(gl_FragCoord).xy);
    int buff_idx = (frag_coord.y * 32) + frag_coord.x;
    frag_clr = vec4(0.0, 0.0, 1.0, 1.0);
    buff.m0[buff_idx] = 1;
    discard;
}
