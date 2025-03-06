#version 310 es
precision mediump float;
precision highp int;

const float _16[16] = float[](1.0, 2.0, 3.0, 4.0, 1.0, 2.0, 3.0, 4.0, 1.0, 2.0, 3.0, 4.0, 1.0, 2.0, 3.0, 4.0);
const vec4 _60[4] = vec4[](vec4(0.0), vec4(1.0), vec4(8.0), vec4(5.0));

layout(location = 0) out float FragColor;
layout(location = 0) flat in mediump int index;

void main()
{
    vec4 foobar[4] = _60;
    vec4 baz[4] = _60;
    FragColor = _16[index];
    if (index < 10)
    {
        FragColor += _16[index ^ 1];
    }
    else
    {
        FragColor += _16[index & 1];
    }
    bool _63 = index > 30;
    if (_63)
    {
        FragColor += _60[index & 3].y;
    }
    else
    {
        FragColor += _60[index & 1].x;
    }
    if (_63)
    {
        foobar[1].z = 20.0;
    }
    mediump int _91 = index & 3;
    FragColor += foobar[_91].z;
    baz = vec4[](vec4(20.0), vec4(30.0), vec4(50.0), vec4(60.0));
    FragColor += baz[_91].z;
}

