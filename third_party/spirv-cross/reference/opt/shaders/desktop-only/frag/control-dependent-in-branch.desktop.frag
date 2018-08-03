#version 450

layout(binding = 0) uniform sampler2D uSampler;

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec4 vInput;

void main()
{
    FragColor = vInput;
    vec4 _23 = texture(uSampler, vInput.xy);
    vec4 _26 = dFdx(vInput);
    vec4 _29 = dFdy(vInput);
    vec4 _32 = fwidth(vInput);
    vec4 _35 = dFdxCoarse(vInput);
    vec4 _38 = dFdyCoarse(vInput);
    vec4 _41 = fwidthCoarse(vInput);
    vec4 _44 = dFdxFine(vInput);
    vec4 _47 = dFdyFine(vInput);
    vec4 _50 = fwidthFine(vInput);
    vec2 _56 = textureQueryLod(uSampler, vInput.zw);
    if (vInput.y > 10.0)
    {
        FragColor += _23;
        FragColor += _26;
        FragColor += _29;
        FragColor += _32;
        FragColor += _35;
        FragColor += _38;
        FragColor += _41;
        FragColor += _44;
        FragColor += _47;
        FragColor += _50;
        FragColor += _56.xyxy;
    }
}

