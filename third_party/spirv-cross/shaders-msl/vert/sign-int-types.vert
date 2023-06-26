#version 310 es

layout(std140) uniform UBO
{
    uniform mat4 uMVP;
    uniform vec4 uFloatVec4;
    uniform vec3 uFloatVec3;
    uniform vec2 uFloatVec2;
    uniform float uFloat;
    uniform ivec4 uIntVec4;
    uniform ivec3 uIntVec3;
    uniform ivec2 uIntVec2;
    uniform int uInt;
};

layout(location = 0) in vec4 aVertex;

layout(location = 0) out vec4 vFloatVec4;
layout(location = 1) out vec3 vFloatVec3;
layout(location = 2) out vec2 vFloatVec2;
layout(location = 3) out float vFloat;
layout(location = 4) flat out ivec4 vIntVec4;
layout(location = 5) flat out ivec3 vIntVec3;
layout(location = 6) flat out ivec2 vIntVec2;
layout(location = 7) flat out int vInt;

void main()
{
    gl_Position = uMVP * aVertex;
    vFloatVec4 = sign(uFloatVec4);
    vFloatVec3 = sign(uFloatVec3);
    vFloatVec2 = sign(uFloatVec2);
    vFloat = sign(uFloat);
    vIntVec4 = sign(uIntVec4);
    vIntVec3 = sign(uIntVec3);
    vIntVec2 = sign(uIntVec2);
    vInt = sign(uInt);
}
