#version 310 es
precision mediump float;

float lut[4] = float[](1.0, 4.0, 3.0, 2.0);

struct Foo
{
	float a;
	float b;
};
Foo foos[2] = Foo[](Foo(10.0, 20.0), Foo(30.0, 40.0));

layout(location = 0) out vec4 FragColor;
layout(location = 0) flat in int line;

void main()
{	
   FragColor = vec4(lut[line]);
   FragColor += foos[line].a * foos[1 - line].a;
}
