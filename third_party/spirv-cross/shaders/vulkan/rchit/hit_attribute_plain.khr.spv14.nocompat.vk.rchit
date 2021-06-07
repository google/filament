#version 460
#extension GL_EXT_ray_tracing : require

layout(location = 0) rayPayloadInEXT vec2 payload;
hitAttributeEXT vec2 hit;

void main()
{
	payload = hit;
}
