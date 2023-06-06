#version 460
#extension GL_NV_ray_tracing : require

layout(location = 0) rayPayloadInNV vec3 payload;

void main()
{
	payload = gl_WorldToObjectNV * vec4(payload, 1.0);
}
