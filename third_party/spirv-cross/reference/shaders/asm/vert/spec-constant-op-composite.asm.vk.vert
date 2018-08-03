#version 450

layout(location = 0) flat out int _4;

void main()
{
    vec4 pos = vec4(0.0);
    pos.y += float(((-10) + 2));
    pos.z += float((100u % 5u));
    pos += vec4(ivec4(20, 30, 0, 0));
    vec2 _56 = pos.xy + vec2(ivec2(ivec4(20, 30, 0, 0).y, ivec4(20, 30, 0, 0).x));
    pos = vec4(_56.x, _56.y, pos.z, pos.w);
    gl_Position = pos;
    _4 = ivec4(20, 30, 0, 0).y;
}

