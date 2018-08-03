#version 310 es
precision mediump float;

layout(std140, binding = 0) uniform UBO0
{
   vec4 a;
   vec4 b;
};

layout(std140, binding = 0) uniform UBO1
{
   ivec4 c;
   ivec4 d;
};

layout(std140, binding = 0) uniform UBO2
{
   uvec4 e;
   uvec4 f;
};

layout(location = 0) out vec4 FragColor;

void main()
{
   FragColor = vec4(c) + vec4(d) + vec4(e) + vec4(f) + a + b; 
}
