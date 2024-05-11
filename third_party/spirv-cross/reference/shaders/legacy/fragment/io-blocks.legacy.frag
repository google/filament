#version 100
precision mediump float;
precision highp int;

varying vec4 vin_color;
varying highp vec3 vin_normal;

void main()
{
    gl_FragData[0] = vin_color + vin_normal.xyzz;
}

