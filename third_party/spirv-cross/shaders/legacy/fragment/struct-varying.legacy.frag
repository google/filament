#version 310 es
precision highp float;

struct Inputs
{
   vec4 a;
   vec2 b;
};

layout(location = 0) in Inputs vin;
layout(location = 0) out vec4 FragColor;

void main()
{
   // Read struct once.
   Inputs v0 = vin;
   // Read struct again.
   Inputs v1 = vin;

   // Read members individually.
   vec4 a = vin.a;
   vec4 b = vin.b.xxyy;

   FragColor = v0.a + v0.b.xxyy + v1.a + v1.b.yyxx + a + b;
}
