#version 460
#extension GL_EXT_ray_tracing : require

layout(location = 0) rayPayloadInEXT Foo { float a; float b; } payload;
hitAttributeEXT Foo2 { float a; float b; } hit;

void main()
{
	payload.a = hit.a;
	payload.b = hit.b;
}
