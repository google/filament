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
    for (int _96 = 0; _96 < 4; )
    {
        vec3 _68 = aVertex.xyz - Light(UBO[_96 * 2 + 4].xyz, UBO[_96 * 2 + 4].w, UBO[_96 * 2 + 5]).Position;
        vColor += ((UBO[_96 * 2 + 5] * clamp(1.0 - (length(_68) / Light(UBO[_96 * 2 + 4].xyz, UBO[_96 * 2 + 4].w, UBO[_96 * 2 + 5]).Radius), 0.0, 1.0)) * dot(aNormal, normalize(_68)));
        _96++;
        continue;
    }
}

