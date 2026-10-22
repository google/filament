#version 310 es
precision mediump float;
layout(location = 0) out vec4 FragColor;

layout(location = 0) flat in int index;

struct Foobar { float a; float b; };

vec4 resolve(Foobar f)
{
   return vec4(f.a + f.b);
}

void main()
{
   const vec4 foo[3] = vec4[](vec4(1.0), vec4(2.0), vec4(3.0));
   const vec4 foobars[2][2] = vec4[][](vec4[](vec4(1.0), vec4(2.0)), vec4[](vec4(8.0), vec4(10.0)));
   const Foobar foos[2] = Foobar[](Foobar(10.0, 40.0), Foobar(90.0, 70.0));

   FragColor = foo[index] + foobars[index][index + 1] + resolve(Foobar(10.0, 20.0)) + resolve(foos[index]);
}
