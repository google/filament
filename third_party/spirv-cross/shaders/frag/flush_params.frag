#version 310 es
precision mediump float;

layout(location = 0) out vec4 FragColor;

struct Structy
{
   vec4 c;
};

void foo2(out Structy f)
{
   f.c = vec4(10.0);
}

Structy foo()
{
   Structy f;
   foo2(f);
   return f;
}

void main()
{
   Structy s = foo();
   FragColor = s.c;
}
