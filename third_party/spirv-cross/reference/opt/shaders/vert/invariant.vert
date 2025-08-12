#version 310 es

invariant gl_Position;

layout(location = 0) in vec4 vInput0;
layout(location = 1) in vec4 vInput1;
layout(location = 2) in vec4 vInput2;
layout(location = 0) invariant out vec4 vColor;

void main()
{
    vec4 _21 = vInput1 * vInput2 + vInput0;
    gl_Position = _21;
    vec4 _27 = vInput0 - vInput1;
    vec4 _29 = _27 * vInput2;
    vColor = _29;
}

