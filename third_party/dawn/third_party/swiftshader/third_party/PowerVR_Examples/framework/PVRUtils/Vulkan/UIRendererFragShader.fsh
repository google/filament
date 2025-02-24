#version 320 es
layout(constant_id = 0) const int sRGB_Output = 0;

layout(set = 0, binding = 0) uniform mediump sampler2D fontTexture;
layout(std140,set = 2, binding = 0)uniform Material
{
    highp mat4 uvMatrix;
    mediump vec4 varColor;
    bool alphaMode;
};
layout(location = 0)in mediump vec2 texCoord;
layout(location = 0)out mediump vec4 oColor;
void main()
{
    mediump vec4 vTex = texture(fontTexture, texCoord);
    if(alphaMode)
    {
        oColor = vec4(varColor.rgb, varColor.a * vTex.a);
    }
    else
    {
        oColor = vec4(varColor * vTex);
    }
    
    if(sRGB_Output == 0)
    {
        oColor.rgb = pow(oColor.rgb, vec3(0.4545454545));// do gamma correction for linear output. 
    }
}
