#version 310 es
precision mediump float;
precision highp int;

uniform vec4 UBO[4];
layout(location = 0) out vec3 FragColor;
layout(location = 0) flat in vec3 vNormal;

void main()
{
    mat4 _19 = mat4(UBO[0], UBO[1], UBO[2], UBO[3]);
    FragColor = mat3(_19[0].xyz, _19[1].xyz, _19[2].xyz) * vNormal;
}

