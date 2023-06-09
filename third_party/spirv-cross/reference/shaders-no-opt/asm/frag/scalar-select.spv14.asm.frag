#version 450

struct _15
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
    _15 _32 = false ? _15(0.0) : _15(1.0);
    float _33[2] = true ? float[](0.0, 1.0) : float[](1.0, 0.0);
}

