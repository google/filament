#version 310 es

struct Light
{
    vec3 Position;
    float Radius;

    vec4 Color;
};

layout(std140) uniform UBO
{
    mat4 uMVP;

    Light lights[4];
};

layout(location = 0) in vec4 aVertex;
layout(location = 1) in vec3 aNormal;
layout(location = 0) out vec4 vColor;

void main()
{
    gl_Position = uMVP * aVertex;

    vColor = vec4(0.0);

    for (int i = 0; i < 4; ++i)
    {
        Light light = lights[i];
        vec3 L = aVertex.xyz - light.Position;
        vColor += dot(aNormal, normalize(L)) * (clamp(1.0 - length(L) / light.Radius, 0.0, 1.0) * lights[i].Color);
    }
}
