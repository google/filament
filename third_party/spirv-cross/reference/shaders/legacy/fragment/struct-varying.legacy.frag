#version 100
precision mediump float;
precision highp int;

struct Inputs
{
    highp vec4 a;
    highp vec2 b;
};

varying highp vec4 vin_a;
varying highp vec2 vin_b;

void main()
{
    Inputs v0 = Inputs(vin_a, vin_b);
    Inputs v1 = Inputs(vin_a, vin_b);
    highp vec4 a = vin_a;
    highp vec4 b = vin_b.xxyy;
    gl_FragData[0] = ((((v0.a + v0.b.xxyy) + v1.a) + v1.b.yyxx) + a) + b;
}

