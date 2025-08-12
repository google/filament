#version 100
precision mediump float;
precision highp int;

varying highp float scalar;
varying highp vec2 vector;

void main()
{
    highp float sipart;
    sipart = float(int(scalar));
    highp float sfpart = scalar - sipart;
    highp vec2 _20 = vec2(sipart, sfpart);
    gl_FragData[0].x = _20.x;
    gl_FragData[0].y = _20.y;
    highp vec2 vipart;
    vipart = vec2(ivec2(vector));
    highp vec2 vfpart = vector - vipart;
    gl_FragData[0].z = vfpart.x;
    gl_FragData[0].w = vfpart.y;
}

