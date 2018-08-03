#version 450
layout(local_size_x = 1) in;

layout(r32f, binding = 0) uniform readonly image2D uImageInF;
layout(r32f, binding = 1) uniform writeonly image2D uImageOutF;
layout(r32i, binding = 2) uniform readonly iimage2D uImageInI;
layout(r32i, binding = 3) uniform writeonly iimage2D uImageOutI;
layout(r32ui, binding = 4) uniform readonly uimage2D uImageInU;
layout(r32ui, binding = 5) uniform writeonly uimage2D uImageOutU;
layout(r32f, binding = 6) uniform readonly imageBuffer uImageInBuffer;
layout(r32f, binding = 7) uniform writeonly imageBuffer uImageOutBuffer;

layout(rg32f, binding = 8) uniform readonly image2D uImageInF2;
layout(rg32f, binding = 9) uniform writeonly image2D uImageOutF2;
layout(rg32i, binding = 10) uniform readonly iimage2D uImageInI2;
layout(rg32i, binding = 11) uniform writeonly iimage2D uImageOutI2;
layout(rg32ui, binding = 12) uniform readonly uimage2D uImageInU2;
layout(rg32ui, binding = 13) uniform writeonly uimage2D uImageOutU2;
layout(rg32f, binding = 14) uniform readonly imageBuffer uImageInBuffer2;
layout(rg32f, binding = 15) uniform writeonly imageBuffer uImageOutBuffer2;

layout(rgba32f, binding = 16) uniform readonly image2D uImageInF4;
layout(rgba32f, binding = 17) uniform writeonly image2D uImageOutF4;
layout(rgba32i, binding = 18) uniform readonly iimage2D uImageInI4;
layout(rgba32i, binding = 19) uniform writeonly iimage2D uImageOutI4;
layout(rgba32ui, binding = 20) uniform readonly uimage2D uImageInU4;
layout(rgba32ui, binding = 21) uniform writeonly uimage2D uImageOutU4;
layout(rgba32f, binding = 22) uniform readonly imageBuffer uImageInBuffer4;
layout(rgba32f, binding = 23) uniform writeonly imageBuffer uImageOutBuffer4;

layout(binding = 24) uniform writeonly image2D uImageNoFmtF;
layout(binding = 25) uniform writeonly uimage2D uImageNoFmtU;
layout(binding = 26) uniform writeonly iimage2D uImageNoFmtI;

void main()
{
    vec4 f = imageLoad(uImageInF, ivec2(gl_GlobalInvocationID.xy));
    imageStore(uImageOutF, ivec2(gl_GlobalInvocationID.xy), f);

    ivec4 i = imageLoad(uImageInI, ivec2(gl_GlobalInvocationID.xy));
    imageStore(uImageOutI, ivec2(gl_GlobalInvocationID.xy), i);

    uvec4 u = imageLoad(uImageInU, ivec2(gl_GlobalInvocationID.xy));
    imageStore(uImageOutU, ivec2(gl_GlobalInvocationID.xy), u);

	vec4 b = imageLoad(uImageInBuffer, int(gl_GlobalInvocationID.x));
	imageStore(uImageOutBuffer, int(gl_GlobalInvocationID.x), b);

    vec4 f2 = imageLoad(uImageInF2, ivec2(gl_GlobalInvocationID.xy));
    imageStore(uImageOutF2, ivec2(gl_GlobalInvocationID.xy), f2);

    ivec4 i2 = imageLoad(uImageInI2, ivec2(gl_GlobalInvocationID.xy));
    imageStore(uImageOutI2, ivec2(gl_GlobalInvocationID.xy), i2);

    uvec4 u2 = imageLoad(uImageInU2, ivec2(gl_GlobalInvocationID.xy));
    imageStore(uImageOutU2, ivec2(gl_GlobalInvocationID.xy), u2);

	vec4 b2 = imageLoad(uImageInBuffer2, int(gl_GlobalInvocationID.x));
	imageStore(uImageOutBuffer2, int(gl_GlobalInvocationID.x), b2);

    vec4 f4 = imageLoad(uImageInF4, ivec2(gl_GlobalInvocationID.xy));
    imageStore(uImageOutF4, ivec2(gl_GlobalInvocationID.xy), f4);

    ivec4 i4 = imageLoad(uImageInI4, ivec2(gl_GlobalInvocationID.xy));
    imageStore(uImageOutI4, ivec2(gl_GlobalInvocationID.xy), i4);

    uvec4 u4 = imageLoad(uImageInU4, ivec2(gl_GlobalInvocationID.xy));
    imageStore(uImageOutU4, ivec2(gl_GlobalInvocationID.xy), u4);

	vec4 b4 = imageLoad(uImageInBuffer4, int(gl_GlobalInvocationID.x));
	imageStore(uImageOutBuffer4, int(gl_GlobalInvocationID.x), b4);

	imageStore(uImageNoFmtF, ivec2(gl_GlobalInvocationID.xy), b2);
	imageStore(uImageNoFmtU, ivec2(gl_GlobalInvocationID.xy), u4);
	imageStore(uImageNoFmtI, ivec2(gl_GlobalInvocationID.xy), i4);
}

