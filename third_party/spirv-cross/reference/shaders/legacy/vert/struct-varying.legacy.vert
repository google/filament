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
    Output s = Output(vec4(0.5), vec2(0.25));
    {
        Output vout = s;
        vout_a = vout.a;
        vout_b = vout.b;
    }
    {
        Output vout = s;
        vout_a = vout.a;
        vout_b = vout.b;
    }
    Output tmp = Output(vout_a, vout_b);
    vout_a = tmp.a;
    vout_b = tmp.b;
    vout_a.x = 1.0;
    vout_b.y = 1.0;
    float c = vout_a.x;
}

