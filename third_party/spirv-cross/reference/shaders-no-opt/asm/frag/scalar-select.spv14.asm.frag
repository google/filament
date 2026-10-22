#version 450

struct _16
{
    float _m0;
};

layout(location = 0) out vec4 FragColor;

void main()
{
    FragColor = false ? vec4(1.0, 1.0, 0.0, 1.0) : vec4(0.0, 0.0, 0.0, 1.0);
    FragColor = vec4(false);
    FragColor = mix(vec4(0.0, 0.0, 0.0, 1.0), vec4(1.0, 1.0, 0.0, 1.0), bvec4(false, true, false, true));
    FragColor = vec4(bvec4(false, true, false, true));
    _16 _36 = false ? _16(0.0) : _16(1.0);
    float _37[2] = true ? float[](0.0, 1.0) : float[](1.0, 0.0);
}

