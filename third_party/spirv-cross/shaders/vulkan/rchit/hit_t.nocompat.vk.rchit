#version 460
#extension GL_NV_ray_tracing : require

layout(location = 0) rayPayloadInNV float payload;

void main()
{
	payload = gl_HitTNV;
}
