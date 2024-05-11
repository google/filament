#version 310 es

struct Foo
{
   mat3x4 MVP0;
   mat3x4 MVP1;
};

layout(std140, binding = 0) uniform UBO
{
   layout(row_major) Foo foo;
};

layout(location = 0) in vec4 v0;
layout(location = 1) in vec4 v1;
layout(location = 0) out vec3 V0;
layout(location = 1) out vec3 V1;

void main()
{
   Foo f = foo;
   vec3 a = v0 * f.MVP0;
   vec3 b = v1 * f.MVP1;
   V0 = a;
   V1 = b;
}
