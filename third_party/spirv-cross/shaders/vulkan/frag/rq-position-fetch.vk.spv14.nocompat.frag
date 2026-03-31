#version 460
#extension GL_EXT_ray_query : enable
#extension GL_EXT_ray_tracing_position_fetch : require

layout(location = 0) out vec3 o_color;
layout(set = 0, binding = 0) uniform accelerationStructureEXT uAS;

void main(void)
{
	float tmin = 0.01;
	float tmax = 1000.0;
	vec3 dir = vec3(1.0, 0.0, 0.0);

	rayQueryEXT query;
	rayQueryInitializeEXT(query, uAS, gl_RayFlagsTerminateOnFirstHitEXT, 0xFF, vec3(0.0), tmin, dir, 1.0);

	rayQueryProceedEXT(query);

	if (rayQueryGetIntersectionTypeEXT(query, true) != gl_RayQueryCommittedIntersectionNoneEXT)
	{
		vec3 vs[3];
		rayQueryGetIntersectionTriangleVertexPositionsEXT(query, true, vs);
		o_color = vs[0] + vs[1] + vs[2];
	}
}
