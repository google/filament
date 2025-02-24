#version 320 es

/*
* Diffuse BRDF: Lambertian Diffuse
	Cdiff / PI

	Cdiff: Diffuse albedo of the material.

* Specular BRDF: Cook-Torrance
	f = D * F * G / (4 * (N.L) * (N.V));

	D: NDF (Normal Distribution function), It computes the distribution of the microfacets for the shaded surface
	F: Describes how light reflects and refracts at the intersection of two different media (most often in computer graphics : Air and the shaded surface)
	G: Defines the shadowing from the microfacets
	N.L:  is the dot product between the normal of the shaded surface and the light direction.
	N.V is the dot product between the normal of the shaded surface and the view direction.
*/

#define PI 3.1415926535897932384626433832795
#define ONE_OVER_PI (1.0 / PI)
#define MODEL_MAT_ARRAY_SIZE 25

layout (location = 0) in highp vec3 inWorldPos;
layout (location = 1) in mediump vec3 inNormal;
layout (location = 2) flat in mediump int inInstanceIndex;

layout (location = 3) in mediump vec2 inTexCoords;
layout (location = 4) in mediump vec3 inTangent;
layout (location = 5) in mediump vec3 inBitTangent;

layout (location = 0) out mediump vec4 outColor;

layout(std140, set = 0, binding = 0) uniform Dynamics
{
	highp mat4 VPMatrix;
	highp vec3 camPos;
	mediump float emissiveIntensity;
	mediump float exposure;
}ubo;

layout (std140, set = 1, binding = 0) uniform UboScene
{
	mediump vec3 lightDir;
	mediump vec3 lightColor;
	uint numPrefilteredMipLevels;
} uboScene;

layout(set = 1, binding = 1) uniform mediump samplerCube irradianceMap;
layout(set = 1, binding = 2) uniform mediump samplerCube prefilteredMap;
layout(set = 1, binding = 3) uniform mediump samplerCube environmentMap;
layout(set = 1, binding = 4) uniform mediump sampler2D brdfLUTmap;

layout(set = 2, binding = 0) uniform mediump sampler2D albedoMap;
layout(set = 2, binding = 1) uniform mediump sampler2D metallicRoughnessMap;
layout(set = 2, binding = 2) uniform mediump sampler2D normalMap;
layout(set = 2, binding = 3) uniform mediump sampler2D emissiveTex;


layout(constant_id=0) const bool HAS_MATERIAL_TEXTURES = false;

struct Material
{
	mediump vec3 albedo;
	mediump float roughness;
	mediump float metallic;
};

layout(std140, set = 2, binding = 4) uniform Materials
{
	Material mat[MODEL_MAT_ARRAY_SIZE];
}materials;

// Normal Distribution function
// http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
mediump float d_GGX(mediump float dotNH, mediump float roughness)
{
	mediump float alpha = roughness * roughness;
	mediump float alpha2 = alpha * alpha;
	mediump float x = dotNH * dotNH * (alpha2 - 1.0) + 1.0;
	return alpha2 / (PI * x * x);
}

mediump float g1(mediump float dotAB, mediump float k)
{
	return dotAB / (dotAB * (1.0 - k) + k);
}

// Geometric Shadowing function
// http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
mediump float g_schlicksmithGGX(mediump float dotNL, mediump float dotNV, mediump float roughness)
{
	mediump float k = (roughness + 1.0);
	k = (k * k) / 8.;
	return g1(dotNL, k) * g1(dotNV, k);
}

// Fresnel function (Shlicks)
// http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
mediump vec3 f_schlick(mediump float cosTheta, mediump vec3 F0)
{
	return F0 + (vec3(1.0) - F0) * pow(2.0, (-5.55473 * cosTheta - 6.98316) * cosTheta);
}

mediump vec3 f_schlickR(mediump float cosTheta, mediump vec3 F0, mediump float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

mediump vec3 computeLight(mediump vec3 V, mediump vec3 N, mediump vec3 F0, mediump vec3 albedo, mediump float metallic, mediump float roughness)
{
	// Directional light
	// NOTE: we negate the light direction here because the direction was from the light source.
	mediump vec3 L = normalize(-uboScene.lightDir);
	// half vector
	mediump vec3 H = normalize (V + L);

	mediump float dotNL = clamp(dot(N, L), 0.0, 1.0);

	mediump vec3 color = vec3(0.0);

	// light contributed only if the angle between the normal and light direction is less than equal to 90 degree.
	if(dotNL > 0.0)
	{
		mediump float dotLH = clamp(dot(L, H), 0.0, 1.0);
		mediump float dotNH = clamp(dot(N, H), 0.0, 1.0);
		mediump float dotNV = clamp(dot(N, V), 0.0, 1.0);
		
		///-------- Specular BRDF: COOK-TORRANCE ---------
		// D = Microfacet Normal distribution.
		mediump float D = d_GGX(dotNH, roughness);

		// G = Geometric Occlusion
		mediump float G = g_schlicksmithGGX(dotNL, dotNV, roughness);

		// F = Surface Reflection
		mediump vec3 F = f_schlick(dotLH, F0);

		mediump vec3 spec = F * ((D * G) / (4.0 * dotNL * dotNV + 0.001/* avoid divide by 0 */));

		///-------- DIFFUSE BRDF ----------
		// kD factor out the lambertian diffuse based on the material's metallicity and fresenal.
		// e.g If the material is fully metallic than it wouldn't have diffuse.
		mediump vec3 kD =  (vec3(1.0) - F) * (1.0 - metallic);
		mediump vec3 diff = kD * albedo * ONE_OVER_PI;

		///-------- DIFFUSE + SPEC ------
		color += (diff + spec) * uboScene.lightColor * dotNL;// scale the final colour based on the angle between the light and the surface normal.
	}
	return color;
}

mediump vec3 prefilteredReflection(mediump float roughness, mediump vec3 R)
{
	// We need to detect where we need to sample from.
	mediump float maxmip = float(uboScene.numPrefilteredMipLevels - 1u);

	mediump float cutoff = 1. / maxmip;

	if(roughness <= cutoff)
	{
		mediump float lod = roughness * maxmip;
		return mix(texture(environmentMap, R).rgb, textureLod(prefilteredMap, R, 0.).rgb, lod);
	}
	else
	{
		mediump float lod = (roughness - cutoff) * maxmip / (1. - cutoff); // Remap to 0..1 on rest of mimpmaps
		return textureLod(prefilteredMap, R, lod).rgb;
	}
}

mediump vec3 computeEnvironmentLighting(mediump vec3 N, mediump vec3 V, mediump vec3 R, mediump vec3 albedo, mediump vec3 F0, mediump float metallic, mediump float roughness)
{
	mediump vec3 specularIR = prefilteredReflection(roughness, R);
	mediump vec2 brdf = texture(brdfLUTmap, vec2(clamp(dot(N, V), 0.0, 1.0), roughness)).rg;

	mediump vec3 F = f_schlickR(max(dot(N, V), 0.0), F0, roughness);

	mediump vec3 diffIR = texture(irradianceMap, N).rgb;
	mediump vec3 kD  = (vec3(1.0) - F) * (1.0 - metallic);// Diffuse factor   
	return albedo * kD  * diffIR + specularIR * (F * brdf.x + brdf.y);
}

mediump vec3 perturbNormal()
{
	// transform the tangent space normal into model space. 
	mediump vec3 tangentNormal = texture(normalMap, inTexCoords).xyz * 2.0 - 1.0;
	mediump vec3 n = normalize(inNormal);
	mediump vec3 t = normalize(inTangent);
	mediump vec3 b = normalize(inBitTangent);
	return normalize(mat3(t, b, n) * tangentNormal);
}

void main()
{
	 mediump vec3 N = HAS_MATERIAL_TEXTURES ? perturbNormal() : normalize(inNormal);

	// calculate the view direction, the direction from the surface towards the camera in world space.
	mediump vec3 V = normalize(ubo.camPos - inWorldPos);
	mediump vec3 R = -normalize(reflect(V, N));

	mediump float metallic;
	mediump float roughness;
	mediump float occlusion;
	mediump vec4 albedo;
	mediump vec3 emissive;

	if(HAS_MATERIAL_TEXTURES)
	{
		mediump vec3 packed = texture(metallicRoughnessMap, inTexCoords).rgb;
		metallic = packed.b;
		roughness = packed.g;
		occlusion = packed.r;
		
		albedo = texture(albedoMap, inTexCoords);
		emissive = texture(emissiveTex, inTexCoords).rgb * ubo.emissiveIntensity;
	}
	else 
	{
		metallic = materials.mat[inInstanceIndex].metallic;
		roughness = materials.mat[inInstanceIndex].roughness;
		albedo = vec4(materials.mat[inInstanceIndex].albedo, 1.0);
		emissive = vec3(0.,0.,0.);
	}

	// The base colour has two different interpretations depending on the value of metalness.
	// When the material is a metal, the base colour is the specific measured reflectance value at normal incidence (F0).
	// For a non-metal the base colour represents the reflected diffuse colour of the material.
	// In this model it is not possible to specify a F0 value for non-metals, and a linear value of 4% (0.04) is used.
	mediump vec3 F0 = mix(vec3(0.04), albedo.rgb, metallic);

	mediump vec3 color = vec3(0.0);

	// calculate the specular contribution of each light sources.
	//mediump vec3 dirLightDiffuseSpec = computeLight(V, N, F0, albedo.rgb, metallic, roughness);
	//colour += dirLightDiffuseSpec;

	/// IBL
	mediump vec3 envLighting = computeEnvironmentLighting(N, V, R, albedo.rgb, F0, metallic, roughness);

	if(HAS_MATERIAL_TEXTURES)
	{
		envLighting *= occlusion;
		color += emissive;
	}
	color += envLighting;

	// This seemingly strange clamp is to ensure that the final colour stays within the constraints
	// of 16-bit floats (13848) with a bit to spare, as the tone mapping calculations squares 
	// this number. It does not affect the final image otherwise, as the clamp will only bring the value to
	// 50. This would already be very close to saturated (producing something like .99 after tone mapping), 
	// but it was trivial to tweak the tone mapping to ensure 50 produces a value >=1.0.
	// It is important to remember that this clamping must only happen last minute, as we need to have
	// the full brightness available for post processing calculations (e.g. bloom)

	mediump vec3 toneMappedColor = min(color.rgb, 50. / ubo.exposure);
	toneMappedColor *= ubo.exposure;

	// http://filmicworlds.com/blog/filmic-tonemapping-operators/
	// Our favourite is the optimized formula by Jim Hejl and Richard Burgess-Dawson
	// We particularly like its high contrast and the fact that it also takes care
	// of Gamma.
	// As mentioned, we modified the value a bit to ensure it saturates at 50.

	mediump vec3 x = max(vec3(0.), toneMappedColor - vec3(0.004));
	toneMappedColor = (x * (6.2 * x + .49)) / (x * (6.175 * x + 1.7) + 0.06);
	outColor = vec4(toneMappedColor, albedo.a);
}