#version 460
#extension GL_NV_ray_tracing : require

layout(location = 0) rayPayloadInNV vec2 payload;
hitAttributeNV vec2 hit;

void main()
{
	payload = hit;
}
