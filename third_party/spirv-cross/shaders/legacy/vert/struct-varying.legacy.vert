#version 310 es

struct Output
{
   vec4 a;
   vec2 b;
};

layout(location = 0) out Output vout;

void main()
{
   Output s = Output(vec4(0.5), vec2(0.25));

   // Write whole struct.
   vout = s;
   // Write whole struct again, checks for scoping.
   vout = s;

   // Read it back.
   Output tmp = vout;

   // Write elements individually.
   vout.a = tmp.a;
   vout.b = tmp.b;

   // Write individual elements.
   vout.a.x = 1.0;
   vout.b.y = 1.0;

   // Read individual elements.
   float c = vout.a.x;
}
