#version 310 es

// comments note the 16b alignment boundaries (see GL spec 7.6.2.2 Standard Uniform Block Layout)
layout(std140) uniform UBO
{
    // 16b boundary
    vec4 A;
    // 16b boundary
    vec2 B0;
    vec2 B1;
    // 16b boundary
    float C0;
    // 16b boundary (vec3 is aligned to 16b)
    vec3 C1;
    // 16b boundary
    vec3 D0;
    float D1;
    // 16b boundary
    float E0;
    float E1;
    float E2;
    float E3;
    // 16b boundary
    float F0;
    vec2 F1;
    // 16b boundary (vec2 before us is aligned to 8b)
    float F2;
};

layout(location = 0) out vec4 oA;
layout(location = 1) out vec4 oB;
layout(location = 2) out vec4 oC;
layout(location = 3) out vec4 oD;
layout(location = 4) out vec4 oE;
layout(location = 5) out vec4 oF;

void main()
{
    gl_Position = vec4(0.0);

    oA = A;
    oB = vec4(B0, B1);
    oC = vec4(C0, C1) + vec4(C1.xy, C1.z, C0);	// not packed
    oD = vec4(D0, D1) + vec4(D0.xy, D0.z, D1);	// packed - must convert for swizzle
    oE = vec4(E0, E1, E2, E3);
    oF = vec4(F0, F1, F2);
}
