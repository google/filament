#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_ray_query : enable
#extension GL_NV_displacement_micromap : enable
layout(binding = 1) uniform accelerationStructureEXT as;
layout(binding = 0) buffer block {
	vec3 op_pos;
	vec2 op_bary;
	uint op_hit;
};
void main()
{
	op_pos =  gl_HitMicroTriangleVertexPositionsNV[0];
	op_pos += gl_HitMicroTriangleVertexPositionsNV[1];
	op_pos += gl_HitMicroTriangleVertexPositionsNV[2];

	op_bary =  gl_HitMicroTriangleVertexBarycentricsNV[0];
	op_bary += gl_HitMicroTriangleVertexBarycentricsNV[1];
	op_bary += gl_HitMicroTriangleVertexBarycentricsNV[2];

	op_hit = gl_HitKindEXT;
	op_hit &= gl_HitKindFrontFacingTriangleEXT |
	          gl_HitKindBackFacingTriangleEXT |
	          gl_HitKindFrontFacingMicroTriangleNV |
	          gl_HitKindBackFacingMicroTriangleNV;
}
