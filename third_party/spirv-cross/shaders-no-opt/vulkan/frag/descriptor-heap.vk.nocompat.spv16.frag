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

layout(descriptor_heap) buffer SSBO
{
	vec4 data;
} ssbos[];

layout(location = 0) out vec4 FragColor;

rayQueryEXT rq;

void main()
{
	FragColor = vec4(0.0);

	FragColor += texelFetch(Images1D[int(gl_FragCoord.x)], 0, 0);
	FragColor += texelFetch(Images2D[int(gl_FragCoord.x)], ivec2(0), 0);
	FragColor += texelFetch(Images3D[int(gl_FragCoord.x)], ivec3(0), 0);
	FragColor += texture(sampler2D(Images2D[int(gl_FragCoord.x)], Samplers[int(gl_FragCoord.y)]), vec2(0), 0);
	FragColor += texture(sampler2DShadow(Images2D[int(gl_FragCoord.x)], Samplers[int(gl_FragCoord.y)]), vec3(0), 0);

	imageStore(WriteImages1D[10], 0, FragColor);
	imageStore(WriteImages2D[20], ivec2(0), FragColor);
	imageStore(WriteImages3D[30], ivec3(0), FragColor);

	FragColor += imageLoad(RWriteImages1D[10], 0);
	FragColor += imageLoad(RWriteImages2D[20], ivec2(0));
	FragColor += imageLoad(RWriteImages3D[30], ivec3(0));

	FragColor += imageLoad(WriteImages2DUnknown[40], ivec2(0));

	FragColor += ubos140[50].data[1];
	FragColor += ubos430[51].data[1].x;
	FragColor += ubosScalar[52].data[1].x;
	FragColor += ssbosReadOnly[60].data;
	ssbosWriteOnly[61].data = vec4(20.0);
	FragColor += ssbos[62].data;

	imageAtomicAdd(ImageAtomics[70], ivec2(0), 50u);

	rayQueryInitializeEXT(rq, RTAS[50], 0, 0, vec3(0.0), 0.0, vec3(1.0, 0.0, 0.0), 1.0);
}
