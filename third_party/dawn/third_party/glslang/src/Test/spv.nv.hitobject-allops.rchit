#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_NV_shader_invocation_reorder : enable
#extension GL_NV_ray_tracing_motion_blur : enable
layout(location = 1) rayPayloadEXT vec4 payload;
layout(location = 2) rayPayloadEXT pBlock { vec2 val1; vec2 val2; };
layout(location = 2) hitObjectAttributeNV vec2 attr;
layout(location = 3) hitObjectAttributeNV hBlock { float attrval;};
layout(binding = 0) uniform accelerationStructureEXT as;
layout(binding = 1) buffer block {
	float op;
};
void main()
{
	hitObjectNV hObj;
	hitObjectNV hObjHit, hObjMiss, hObjNop;
	attr = vec2(1.0);
	attrval = 2.0;
	hitObjectTraceRayNV(hObj, as, 1U, 1U, 1U, 1U, 1U, vec3(0.5), 0.5, vec3(1), 1.0, 1);
	hitObjectTraceRayMotionNV(hObj, as, 1U, 1U, 1U, 1U, 1U, vec3(0.5), 0.5, vec3(1), 1.0, 10.0, 2);
	hitObjectRecordHitNV(hObj, as, 1, 1, 1, 2U, 2U, 2U, vec3(1), 1.0f, vec3(2), 2.0f, 2);
	hitObjectRecordHitMotionNV(hObj, as, 1, 1, 1, 2U, 2U, 2U, vec3(1), 1.0f, vec3(2), 2.0f, 4.0f, 2);
	hitObjectRecordHitWithIndexNV(hObjHit, as, 1, 1, 1, 2U, 2U, vec3(1), 1.0f, vec3(2), 2.0f, 3);
	hitObjectRecordHitWithIndexMotionNV(hObjHit, as, 1, 1, 1, 2U, 2U, vec3(1), 1.0f, vec3(2), 2.0f, 4.0f, 3);
	hitObjectRecordEmptyNV(hObjNop);
	hitObjectRecordMissNV(hObjMiss, 1U, vec3(0.5), 2.0, vec3(1.5), 5.0);
	hitObjectRecordMissMotionNV(hObjMiss, 1U, vec3(0.5), 2.0, vec3(1.5), 5.0, 8.0f);
	hitObjectExecuteShaderNV(hObjHit, 2);
	if (hitObjectIsHitNV(hObj)) { 
		op = 1.0f;
	} else if (hitObjectIsMissNV(hObj)) {
		op = 2.0f;
	} else if (hitObjectIsEmptyNV(hObj)) {
		op = 3.0f;
	}
	

	float tmin = hitObjectGetRayTMinNV(hObjHit);
	float tmax = hitObjectGetRayTMaxNV(hObjHit);
	vec3 orig = hitObjectGetWorldRayOriginNV(hObjHit);
	vec3 dir = hitObjectGetWorldRayDirectionNV(hObjHit);
	vec3 oorig = hitObjectGetObjectRayOriginNV(hObjHit);
	vec3 odir = hitObjectGetObjectRayDirectionNV(hObjHit);
	mat4x3 otw = hitObjectGetObjectToWorldNV(hObjHit);
	mat4x3 wto = hitObjectGetWorldToObjectNV(hObjHit);
	int cid = hitObjectGetInstanceCustomIndexNV(hObjMiss);
	int iid = hitObjectGetInstanceIdNV(hObjNop);
	int pid = hitObjectGetPrimitiveIndexNV(hObj);
	int gid = hitObjectGetGeometryIndexNV(hObj);
	uint hkind = hitObjectGetHitKindNV(hObj);
	hitObjectGetAttributesNV(hObj, 2);
	uvec2 handle = hitObjectGetShaderRecordBufferHandleNV(hObj);
	uint rid = hitObjectGetShaderBindingTableRecordIndexNV(hObj);
	
}
