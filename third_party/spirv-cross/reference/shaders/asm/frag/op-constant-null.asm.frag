#version 310 es
precision mediump float;
precision highp int;

struct D
{
    vec4 a;
    float b;
};

const vec4 _41[4] = vec4[](vec4(0.0), vec4(0.0), vec4(0.0), vec4(0.0));

layout(location = 0) out float FragColor;

void main()
{
    float a = 0.0;
    vec4 b = vec4(0.0);
    mat2x3 c = mat2x3(vec3(0.0), vec3(0.0));
    D d = D(vec4(0.0), 0.0);
    FragColor = a;
}

