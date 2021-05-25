#version 450

struct Bar
{
	float v[2];
	float w;
};

layout(location = 0) out V
{
	float a;
	float b[2];
	Bar c[2];
	Bar d;
};

void main()
{
	a = 1.0;
	b[0] = 2.0;
	b[1] = 3.0;
	c[0].v[0] = 4.0;
	c[0].v[1] = 5.0;
	c[0].w = 6.0;
	c[1].v[0] = 7.0;
	c[1].v[1] = 8.0;
	c[1].w = 9.0;
	d.v[0] = 10.0;
	d.v[1] = 11.0;
	d.w = 12.0;
}
