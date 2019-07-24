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
    {
        Output vout = Output(vec4(0.5), vec2(0.25));
        vout_a = vout.a;
        vout_b = vout.b;
    }
    {
        Output vout = Output(vec4(0.5), vec2(0.25));
        vout_a = vout.a;
        vout_b = vout.b;
    }
    Output _22 = Output(vout_a, vout_b);
    vout_a = _22.a;
    vout_b = _22.b;
    vout_a.x = 1.0;
    vout_b.y = 1.0;
}

