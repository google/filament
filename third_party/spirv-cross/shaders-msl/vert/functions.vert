#version 310 es

layout(std140) uniform UBO
{
    uniform mat4 uMVP;
    uniform vec3 rotDeg;
    uniform vec3 rotRad;
    uniform ivec2 bits;
};

layout(location = 0) in vec4 aVertex;
layout(location = 1) in vec3 aNormal;

layout(location = 0) out vec3 vNormal;
layout(location = 1) out vec3 vRotDeg;
layout(location = 2) out vec3 vRotRad;
layout(location = 3) out ivec2 vLSB;
layout(location = 4) out ivec2 vMSB;

void main()
{
    gl_Position = inverse(uMVP) * aVertex;
    vNormal = aNormal;
    vRotDeg = degrees(rotRad);
    vRotRad = radians(rotDeg);
    vLSB = findLSB(bits);
    vMSB = findMSB(bits);
}
