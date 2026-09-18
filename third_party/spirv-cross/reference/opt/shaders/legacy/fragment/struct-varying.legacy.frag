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
    gl_FragData[0] = ((((vin_a + vin_b.xxyy) + vin_a) + vin_b.yyxx) + vin_a) + vin_b.xxyy;
}

