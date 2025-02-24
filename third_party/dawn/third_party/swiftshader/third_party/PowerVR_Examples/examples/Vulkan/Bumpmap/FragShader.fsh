#version 320 es

layout (set = 0, binding = 0) uniform mediump sampler2D sBaseTex;
layout (set = 0, binding = 1) uniform mediump sampler2D sNormalMap;
		
layout(location = 0) in mediump vec3 LightVecTangentSpace;
layout(location = 1) in mediump vec2 TexCoord;
layout(location = 0) out mediump vec4 oColor;

void main()
{
	// read the per-pixel normal from the normal map and expand to [-1, 1]
	mediump vec3 normal = texture(sNormalMap, TexCoord).rgb * 2.0 - 1.0;

    // linear interpolations of normals may cause shortened normals and thus
	// visible artefacts on low-poly models.
	// We omit the normalization here for performance reasons
    
	// calculate diffuse lighting as the cosine of the angle between light
	// direction and surface normal (both in surface local/tangent space)
	// We don't have to clamp to 0 here because the framebuffer write will be clamped
	mediump float lightIntensity = dot(LightVecTangentSpace, normal) + 0.015/*ambient*/;

	// read base texture and modulate with light intensity
	mediump vec3 texColor = texture(sBaseTex, TexCoord).rgb;	
	oColor = vec4(texColor * lightIntensity, 1.0);
}
