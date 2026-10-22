#version 450

layout(location = 0) flat in float vFloat;
layout(location = 1) flat in int vInt;
layout(location = 0) out vec4 Float;
layout(location = 1) out ivec4 Int;
layout(location = 2) out vec4 Float2;
layout(location = 3) out ivec4 Int2;

void main()
{
	Float = vec4(vFloat) * 2.0;
	Int = ivec4(vInt) * 2;
	Float2 = vec4(10.0);
	Int2 = ivec4(10);
}
