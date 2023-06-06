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
    gl_FragData[0] = ((((Inputs(vin_a, vin_b).a + Inputs(vin_a, vin_b).b.xxyy) + Inputs(vin_a, vin_b).a) + Inputs(vin_a, vin_b).b.yyxx) + vin_a) + vin_b.xxyy;
}

