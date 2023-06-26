#version 460
#extension GL_NV_ray_tracing : require

struct Foo { float a; float b; };

layout(location = 0) rayPayloadInNV Foo payload;
hitAttributeNV Foo hit;

void main()
{
	payload = hit;
}
