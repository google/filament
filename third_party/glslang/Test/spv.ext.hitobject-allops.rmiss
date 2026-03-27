#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_shader_invocation_reorder : enable
#extension GL_NV_ray_tracing_motion_blur : enable
#extension GL_EXT_ray_query : enable
layout(location = 1) rayPayloadEXT vec4 payload;
layout(location = 2) rayPayloadEXT pBlock { vec2 val1; vec2 val2; };
layout(location = 2) hitObjectAttributeEXT vec2 attr;
layout(location = 3) hitObjectAttributeEXT hBlock { float attrval;};
layout(binding = 0) uniform accelerationStructureEXT as;
layout(binding = 1) buffer block {
	float op;
};
void main()
{
	hitObjectEXT hObj;
	hitObjectEXT hObjHit, hObjMiss, hObjNop;
	rayQueryEXT rq;
	attr = vec2(1.0);
	attrval = 2.0;
	hitObjectTraceRayEXT(hObj, as, 1U, 1U, 1U, 1U, 1U, vec3(0.5), 0.5, vec3(1), 1.0, 1);
	hitObjectTraceRayMotionEXT(hObj, as, 1U, 1U, 1U, 1U, 1U, vec3(0.5), 0.5, vec3(1), 1.0, 10.0, 2);
	hitObjectRecordEmptyEXT(hObjNop);
	hitObjectRecordMissEXT(hObjMiss, 1U, 1U, vec3(0.5), 2.0, vec3(1.5), 5.0);
	hitObjectRecordMissMotionEXT(hObjMiss,1U, 1U, vec3(0.5), 2.0, vec3(1.5), 5.0, 8.0f);
	hitObjectExecuteShaderEXT(hObjHit, 1);
	if (hitObjectIsHitEXT(hObj)) { 
		op = 1.0f;
	} else if (hitObjectIsMissEXT(hObj)) {
		op = 2.0f;
	} else if (hitObjectIsEmptyEXT(hObj)) {
		op = 3.0f;
	}
	
	uint rayflags = hitObjectGetRayFlagsEXT(hObjHit);
	float tmin = hitObjectGetRayTMinEXT(hObjHit);
	float tmax = hitObjectGetRayTMaxEXT(hObjHit);
	vec3 orig = hitObjectGetWorldRayOriginEXT(hObjHit);
	vec3 dir = hitObjectGetWorldRayDirectionEXT(hObjHit);
	vec3 oorig = hitObjectGetObjectRayOriginEXT(hObjHit);
	vec3 odir = hitObjectGetObjectRayDirectionEXT(hObjHit);
	mat4x3 otw = hitObjectGetObjectToWorldEXT(hObjHit);
	mat4x3 wto = hitObjectGetWorldToObjectEXT(hObjHit);
	int cid = hitObjectGetInstanceCustomIndexEXT(hObjMiss);
	int iid = hitObjectGetInstanceIdEXT(hObjNop);
	int pid = hitObjectGetPrimitiveIndexEXT(hObj);
	int gid = hitObjectGetGeometryIndexEXT(hObj);
	uint hkind = hitObjectGetHitKindEXT(hObj);
	hitObjectGetAttributesEXT(hObj, 2);
	uvec2 handle = hitObjectGetShaderRecordBufferHandleEXT(hObj);
	uint rid = hitObjectGetShaderBindingTableRecordIndexEXT(hObj);
        hitObjectRecordFromQueryEXT(hObjHit, rq, 1U, 2);
	
}
