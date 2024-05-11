#version 310 es

struct Foo
{
    mat3x4 MVP0;
    mat3x4 MVP1;
};

uniform vec4 UBO[8];
layout(location = 0) in vec4 v0;
layout(location = 1) in vec4 v1;
layout(location = 0) out vec3 V0;
layout(location = 1) out vec3 V1;

void main()
{
    Foo _20 = Foo(transpose(mat4x3(UBO[0].xyz, UBO[1].xyz, UBO[2].xyz, UBO[3].xyz)), transpose(mat4x3(UBO[4].xyz, UBO[5].xyz, UBO[6].xyz, UBO[7].xyz)));
    V0 = v0 * _20.MVP0;
    V1 = v1 * _20.MVP1;
}

