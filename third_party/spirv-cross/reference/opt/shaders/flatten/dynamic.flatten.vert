#version 310 es

struct Light
{
    vec3 Position;
    float Radius;
    vec4 Color;
};

uniform vec4 UBO[12];
layout(location = 0) in vec4 aVertex;
layout(location = 0) out vec4 vColor;
layout(location = 1) in vec3 aNormal;

void main()
{
    gl_Position = mat4(UBO[0], UBO[1], UBO[2], UBO[3]) * aVertex;
    vColor = vec4(0.0);
    for (int _82 = 0; _82 < 4; )
    {
        vec3 _54 = aVertex.xyz - UBO[_82 * 2 + 4].xyz;
        vColor += ((UBO[_82 * 2 + 5] * clamp(1.0 - (length(_54) / UBO[_82 * 2 + 4].w), 0.0, 1.0)) * dot(aNormal, normalize(_54)));
        _82++;
        continue;
    }
}

