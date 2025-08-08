#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_ray_query : enable
#extension GL_NV_cluster_acceleration_structure : enable
#extension GL_NV_shader_invocation_reorder : enable
layout(binding = 1) uniform accelerationStructureEXT as;
void main()
{
	hitObjectNV hitObj;
	int hitObjId = hitObjectGetClusterIdNV(hitObj);
	int clusterid = gl_ClusterIDNV;
	rayQueryEXT rq;
	int id_candidate = rayQueryGetIntersectionClusterIdNV(rq, false);
	int id_committed = rayQueryGetIntersectionClusterIdNV(rq, true);
	bool test  = (id_candidate == gl_ClusterIDNoneNV) && (id_committed == gl_ClusterIDNoneNV);
}
