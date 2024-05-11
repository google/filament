#version 100

attribute vec4 attr_int4;
attribute float attr_int1;

void main()
{
    gl_Position.x = float(int(attr_int4[int(attr_int1)]));
    gl_Position.y = 0.0;
    gl_Position.z = 0.0;
    gl_Position.w = 0.0;
}

