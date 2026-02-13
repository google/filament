#version 450

out gl_PerVertex
{
	invariant vec4 gl_Position;
};

layout(location = 0) out int o_value;

void main()
{
   o_value = 10;
}
