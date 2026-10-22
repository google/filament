#version 460

#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_ray_query : enable

layout(binding = 0) uniform accelerationStructureEXT topLevelAS;

void initFn(rayQueryEXT rayQuery) {
  vec3  rayOrigin    = vec3(0, 0, 1);
  vec3  rayDirection = vec3(0, 0,-1);
  float rayDistance  = 2.0;
  rayQueryInitializeEXT(rayQuery, topLevelAS, gl_RayFlagsTerminateOnFirstHitEXT, 0xFF, rayOrigin, 0.001, rayDirection, rayDistance);
}

uint proceeFn(rayQueryEXT rayQuery) {
  while(rayQueryProceedEXT(rayQuery))
    ;
  return rayQueryGetIntersectionTypeEXT(rayQuery, true);
}

void main() {
  rayQueryEXT rayQuery;

  initFn(rayQuery);
  proceeFn(rayQuery);
}
