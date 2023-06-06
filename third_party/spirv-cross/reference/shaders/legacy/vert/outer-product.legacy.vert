#version 100

varying mat2 m22;
attribute vec2 v2a;
attribute vec2 v2b;
varying mat3 m33;
attribute vec3 v3a;
attribute vec3 v3b;
varying mat4 m44;
attribute vec4 v4a;
attribute vec4 v4b;

void main()
{
    m22 = mat2(v2a * v2b.x, v2a * v2b.y);
    m33 = mat3(v3a * v3b.x, v3a * v3b.y, v3a * v3b.z);
    m44 = mat4(v4a * v4b.x, v4a * v4b.y, v4a * v4b.z, v4a * v4b.w);
}

