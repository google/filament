#version 100
precision mediump float;
precision highp int;

varying highp float scalar;
varying highp vec2 vector;

void main()
{
    highp float sipart;
    sipart = float(int(scalar));
    gl_FragData[0].x = sipart;
    gl_FragData[0].y = scalar - sipart;
    highp vec2 vipart;
    vipart = vec2(ivec2(vector));
    highp vec2 _35 = vector - vipart;
    gl_FragData[0].z = _35.x;
    gl_FragData[0].w = _35.y;
}

