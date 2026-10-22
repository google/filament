#version 310 es

layout(location = 0) in int Index1;
layout(location = 1) in int Index2;

vec4 consume_constant_arrays2(vec4 positions[4], vec4 positions2[4])
{
    vec4 indexable[4] = positions;
    vec4 indexable_1[4] = positions2;
    return indexable[Index1] + indexable_1[Index2];
}

vec4 consume_constant_arrays(vec4 positions[4], vec4 positions2[4])
{
    return consume_constant_arrays2(positions, positions2);
}

void main()
{
    vec4 LUT2[4];
    LUT2[0] = vec4(10.0);
    LUT2[1] = vec4(11.0);
    LUT2[2] = vec4(12.0);
    LUT2[3] = vec4(13.0);
    gl_Position = consume_constant_arrays(vec4[](vec4(0.0), vec4(1.0), vec4(2.0), vec4(3.0)), LUT2);
}

