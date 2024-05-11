#version 310 es

struct Light
{
    vec3 Position;
    float Radius;
    vec4 Color;
};

uniform vec4 UBO[6];
layout(location = 0) in vec4 aVertex;
layout(location = 0) out vec4 vColor;
layout(location = 1) in vec3 aNormal;

void main()
{
    gl_Position = mat4(UBO[0], UBO[1], UBO[2], UBO[3]) * aVertex;
    vColor = vec4(0.0);
    vec3 _39 = aVertex.xyz - UBO[4].xyz;
    vColor += ((UBO[5] * clamp(1.0 - (length(_39) / UBO[4].w), 0.0, 1.0)) * dot(aNormal, normalize(_39)));
}

