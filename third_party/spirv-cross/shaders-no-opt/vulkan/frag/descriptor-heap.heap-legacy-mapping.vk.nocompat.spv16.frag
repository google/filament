#version 460
#extension GL_EXT_descriptor_heap : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_samplerless_texture_functions : require
#extension GL_EXT_shader_image_load_formatted : require
#extension GL_EXT_ray_query : require
#extension GL_EXT_scalar_block_layout : require

layout(descriptor_heap) uniform texture1D Images1D[];
layout(descriptor_heap) uniform texture2D Images2D[];
layout(descriptor_heap) uniform texture3D Images3D[];

layout(descriptor_heap) uniform accelerationStructureEXT RTAS[];

layout(descriptor_heap, r32f) writeonly uniform image1D WriteImages1D[];
layout(descriptor_heap, r32f) writeonly uniform image2D WriteImages2D[];
layout(descriptor_heap, r32f) writeonly uniform image3D WriteImages3D[];

layout(descriptor_heap, r32f) readonly uniform image1D RWriteImages1D[];
layout(descriptor_heap, r32f) readonly uniform image2D RWriteImages2D[];
layout(descriptor_heap, r32f) readonly uniform image3D RWriteImages3D[];

layout(descriptor_heap, r32ui) uniform uimage2D ImageAtomics[];

layout(descriptor_heap) uniform image2D WriteImages2DUnknown[];

layout(descriptor_heap) uniform sampler Samplers[];

layout(descriptor_heap, std140) uniform UBO140
{
	float data[];
} ubos140[];

layout(descriptor_heap, std430) uniform UBO430
{
	vec3 data[];
} ubos430[];

layout(descriptor_heap, scalar) uniform UBOScalar
{
	vec3 data[];
} ubosScalar[];

layout(descriptor_heap) readonly buffer SSBOReadOnly
{
	vec4 data;
} ssbosReadOnly[];

layout(descriptor_heap) coherent writeonly buffer SSBOWriteOnly
{
	vec4 data;
} ssbosWriteOnly[];

layout(descriptor_heap) buffer SSBOAtomic
{
	uint data;
	uint data2;
} ssbosAtomic[];

layout(descriptor_heap) buffer SSBO
{
	vec4 data;
} ssbos[];

layout(location = 0) out vec4 FragColor;

rayQueryEXT rq;

void main()
{
	FragColor = vec4(0.0);

	int desc_index = int(gl_FragCoord.x);

	FragColor += texelFetch(Images1D[desc_index + 0], 0, 0);
	FragColor += texelFetch(Images2D[desc_index + 1], ivec2(0), 0);
	FragColor += texelFetch(Images3D[desc_index + 2], ivec3(0), 0);
	FragColor += texture(sampler2D(Images2D[int(gl_FragCoord.x)], Samplers[int(gl_FragCoord.y)]), vec2(0), 0);
	FragColor += texture(sampler2DShadow(Images2D[int(gl_FragCoord.x)], Samplers[int(gl_FragCoord.y)]), vec3(0), 0);

	imageStore(WriteImages1D[desc_index + 3], 0, FragColor);
	imageStore(WriteImages2D[desc_index + 4], ivec2(0), FragColor);
	imageStore(WriteImages3D[desc_index + 5], ivec3(0), FragColor);

	FragColor += imageLoad(RWriteImages1D[desc_index + 6], 0);
	FragColor += imageLoad(RWriteImages2D[desc_index + 7], ivec2(0));
	FragColor += imageLoad(RWriteImages3D[desc_index + 8], ivec3(0));

	FragColor += imageLoad(WriteImages2DUnknown[desc_index + 9], ivec2(0));

	FragColor += ubos140[desc_index + 10].data[1];
	FragColor += ubos430[desc_index + 11].data[1].x;
	FragColor += ubosScalar[desc_index + 12].data[1].x;
	FragColor += ssbosReadOnly[desc_index + 13].data;
	ssbosWriteOnly[desc_index + 14].data = vec4(20.0);
	FragColor += ssbos[desc_index + 15].data;

	imageAtomicAdd(ImageAtomics[desc_index + 16], ivec2(0), 50u);

	rayQueryInitializeEXT(rq, RTAS[desc_index + 17], 0, 0, vec3(0.0), 0.0, vec3(1.0, 0.0, 0.0), 1.0);

	// glslang untyped pointer atomics are broken ...
	//atomicAdd(ssbosAtomic[desc_index].data2, 1);
}
