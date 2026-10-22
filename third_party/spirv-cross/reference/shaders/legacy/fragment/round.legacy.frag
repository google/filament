#version 100
precision mediump float;
precision highp int;

varying highp vec4 vA;
varying highp float vB;

void main()
{
    gl_FragData[0] = floor(vA + vec4(0.5));
    gl_FragData[0] *= floor(vB + float(0.5));
}

