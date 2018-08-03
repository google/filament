#version 100

attribute vec4 Position;
varying vec4 vout_color;
varying vec3 vout_normal;

void main()
{
    gl_Position = Position;
    vout_color = vec4(1.0);
    vout_normal = vec3(0.5);
}

