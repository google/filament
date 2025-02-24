#version 300 es

uniform mediump sampler2D sTexture;

uniform mediump vec3 viewLightDirection;
uniform mediump vec3 albedoModulation;

// How smooth/rough the object is
uniform mediump float specularExponent;

// How much is the specular light coloured by the object's colour
uniform mediump float metallicity; 

// How much of the light is diffuse/specular
uniform mediump float reflectivity; 

in mediump vec3 viewNormal;
in mediump vec2 texCoord;

layout(location = 0) out mediump vec4 oColor;

const mediump float Ambient = 0.125;
const mediump vec3 viewDirection = vec3(0.57735, 0.57735, -0.57735);

void main()
{
    // Overall colour of the material. The use of albedoModulation is not strictly required (as this information would 
	// normally come from a texture), but its helpful for achieving a "tweakable" shader
    mediump vec3 albedo = texture(sTexture, texCoord).rgb * albedoModulation;

	// Add an ambient factor (greatly helps the look in general, otherwise when using a high reflectivity the object appears very dark)

	// Specular, non-metallic (white) ambient = Ambient * reflectivity * (1 - metallicity)
	mediump float whiteAmbientFactor = Ambient * (reflectivity * (1.0 - metallicity));

	// Coloured ambient = Ambient * albedo * (1 + reflectivity * (metallicity - 1))
	mediump vec3 colorAmbientFactor = Ambient * albedo * (1.0 + reflectivity * (metallicity - 1.0));

	// diffuse/specular ambient total
	mediump vec3 ambient = vec3(whiteAmbientFactor + colorAmbientFactor);

    mediump vec3 normal = normalize(viewNormal);
	mediump float n_dot_l = max(dot(normal, -viewLightDirection), 0.0);

	// diffuse factor
	mediump vec3 color = ambient + (n_dot_l) * albedo * (1.0 - reflectivity);

	// Skip if facing away from the light source
	if (n_dot_l > 0.0)
	{
		mediump vec3 reflectedLightDirection = reflect(-viewLightDirection, normal);
		mediump float v_dot_r = max(dot(viewDirection, reflectedLightDirection), 0.0);
		
		// Most metallic: Specular Colour = albedo colour
		// Most non metallic: Specular Colour = white

		mediump float specularIntensity = max(pow(v_dot_r, specularExponent) * reflectivity, 0.0);

		// metallic/specular factor
		mediump float metallicSpecularIntensity = metallicity * specularIntensity;
		mediump float plasticSpecularIntensity = (1.0 - metallicity) * specularIntensity;

		mediump vec3 specularColor = vec3(plasticSpecularIntensity) + metallicSpecularIntensity * albedo;
		color += specularColor;
	}
#ifndef FRAMEBUFFER_SRGB
	color = pow(color, vec3(0.4545454545)); // Gamma correction
#endif
	oColor = vec4(color, 1.0);
}