#version 450

#ifndef SPIRV_CROSS_CONSTANT_ID_0
#define SPIRV_CROSS_CONSTANT_ID_0 0
#endif
const int omap_r = SPIRV_CROSS_CONSTANT_ID_0;
#ifndef SPIRV_CROSS_CONSTANT_ID_1
#define SPIRV_CROSS_CONSTANT_ID_1 1
#endif
const int omap_g = SPIRV_CROSS_CONSTANT_ID_1;
#ifndef SPIRV_CROSS_CONSTANT_ID_2
#define SPIRV_CROSS_CONSTANT_ID_2 2
#endif
const int omap_b = SPIRV_CROSS_CONSTANT_ID_2;
#ifndef SPIRV_CROSS_CONSTANT_ID_3
#define SPIRV_CROSS_CONSTANT_ID_3 3
#endif
const int omap_a = SPIRV_CROSS_CONSTANT_ID_3;

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec4 vColor;

void main()
{
    FragColor = vec4(vColor[omap_r], vColor[omap_g], vColor[omap_b], vColor[omap_a]);
}

