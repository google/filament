#version 320 es

layout (location = 0)in highp vec4 myVertex;
layout (location = 1)in mediump vec2 myUV;
layout(std140,set = 1, binding = 0)uniform MVP
{
    highp mat4 myMVPMatrix;
};

layout(std140,set = 2, binding = 0)uniform Material
{
	highp mat4 uvMatrix;
    mediump vec4 varColor;
    bool alphaMode;
};

layout(location = 0) out mediump vec2 texCoord;
void main()
{ 
    gl_Position = myMVPMatrix * myVertex;
    texCoord = (uvMatrix * vec4(myUV,1.0,1.0)).xy;
}
