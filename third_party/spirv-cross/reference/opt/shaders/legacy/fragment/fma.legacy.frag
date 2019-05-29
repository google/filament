#version 100
precision mediump float;
precision highp int;

varying highp vec4 vA;
varying highp vec4 vB;
varying highp vec4 vC;

void main()
{
    gl_FragData[0] = vA * vB + vC;
}

