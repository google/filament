#version 450

layout(binding = 0) uniform sampler1D TextureBase;
layout(binding = 1) uniform sampler1D TextureDetail;

layout(location = 0) in vec4 VertGeom;

layout(location = 0) out vec4 FragColor0;
layout(location = 1) out vec4 FragColor1;

void main()
{
	vec4 texSample0 = texture(TextureBase, VertGeom.x);
	vec4 texSample1 = textureOffset(TextureDetail, VertGeom.x, 3);
	
    ivec4 iResult0 = floatBitsToInt(texSample0);
    ivec4 iResult1 = floatBitsToInt(texSample1);
    FragColor0 = (intBitsToFloat(iResult0) * intBitsToFloat(iResult1));    
    
    uvec4 uResult0 = floatBitsToUint(texSample0);
    uvec4 uResult1 = floatBitsToUint(texSample1);
    FragColor1 = (uintBitsToFloat(uResult0) * uintBitsToFloat(uResult1));    
}
