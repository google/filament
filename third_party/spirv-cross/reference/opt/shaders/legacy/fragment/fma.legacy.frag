#version 100
precision mediump float;
precision highp int;

varying highp vec4 vA;
varying highp vec4 vB;
varying highp vec4 vC;

void main()
{
    highp vec4 _17 = vA * vB + vC;
    gl_FragData[0] = _17;
    gl_FragData[0] = _17 * (vB * vC + vA);
}

