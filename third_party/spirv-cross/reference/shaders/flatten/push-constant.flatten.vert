#version 310 es

uniform vec4 PushMe[6];
layout(location = 1) in vec4 Pos;
layout(location = 0) out vec2 vRot;
layout(location = 0) in vec2 Rot;

void main()
{
    gl_Position = mat4(PushMe[0], PushMe[1], PushMe[2], PushMe[3]) * Pos;
    vRot = (mat2(PushMe[4].xy, PushMe[4].zw) * Rot) + vec2(PushMe[5].z);
}

