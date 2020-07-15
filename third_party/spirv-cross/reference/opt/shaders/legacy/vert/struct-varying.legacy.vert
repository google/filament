#version 100

struct Output
{
    vec4 a;
    vec2 b;
};

varying vec4 vout_a;
varying vec2 vout_b;

void main()
{
    vout_a = Output(vec4(0.5), vec2(0.25)).a;
    vout_b = Output(vec4(0.5), vec2(0.25)).b;
    vout_a = Output(vec4(0.5), vec2(0.25)).a;
    vout_b = Output(vec4(0.5), vec2(0.25)).b;
    Output _22 = Output(vout_a, vout_b);
    vout_a = _22.a;
    vout_b = _22.b;
    vout_a.x = 1.0;
    vout_b.y = 1.0;
}

