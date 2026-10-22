#version 310 es
precision mediump float;
precision highp int;

vec4 _33;

layout(location = 0) in vec4 vInput;
layout(location = 0) out vec4 FragColor;

void main()
{
    vec4 _37 = vInput;
    highp vec4 _38 = _37;
    _38.x = 1.0;
    _38.y = 2.0;
    _38.z = 3.0;
    _38.w = 4.0;
    FragColor = _38;
    vec4 _8 = _37;
    _8.x = 1.0;
    _8.y = 2.0;
    _8.z = 3.0;
    _8.w = 4.0;
    FragColor = _8;
    highp vec4 _42 = _37;
    _42.x = 1.0;
    vec4 _12 = _42;
    _12.y = 2.0;
    highp vec4 _43 = _12;
    _43.z = 3.0;
    vec4 _13 = _43;
    _13.w = 4.0;
    FragColor = _13;
    highp vec4 _44 = _37;
    _44.x = 1.0;
    highp vec4 _45 = _44;
    _45.y = 2.0;
    vec4 mp_copy_45 = _45;
    highp vec4 _46 = _45;
    _46.z = 3.0;
    highp vec4 _47 = _46;
    _47.w = 4.0;
    vec4 mp_copy_47 = _47;
    FragColor = _47 + _44;
    FragColor = mp_copy_47 + mp_copy_45;
    highp vec4 _49;
    _49.x = 1.0;
    _49.y = 2.0;
    _49.z = 3.0;
    _49.w = 4.0;
    FragColor = _49;
    highp vec4 _53 = vec4(0.0);
    _53.x = 1.0;
    FragColor = _53;
    highp vec4 _54[2] = vec4[](vec4(0.0), vec4(0.0));
    _54[1].z = 1.0;
    _54[0].w = 2.0;
    FragColor = _54[0];
    FragColor = _54[1];
    highp mat4 _58 = mat4(vec4(0.0), vec4(0.0), vec4(0.0), vec4(0.0));
    _58[1].z = 1.0;
    _58[2].w = 2.0;
    FragColor = _58[0];
    FragColor = _58[1];
    FragColor = _58[2];
    FragColor = _58[3];
    highp vec4 PHI;
    PHI = _46;
    highp vec4 _65 = PHI;
    _65.w = 4.0;
    FragColor = _65;
}

