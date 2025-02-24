uniform mediump sampler2D sTexture;

uniform mediump vec3 ViewLightDirection;
uniform mediump vec3 AlbedoModulation;
uniform mediump float SpecularExponent;  //How smooth is the object
uniform mediump float Metallicity;  //How much is the specular light colored by the object's color
uniform mediump float Reflectivity; //How much of the light is diffuse/specular

varying mediump vec3 ViewNormal;
varying mediump vec2 TexCoord;

const mediump float AMBIENT = 0.125;

void main()
{
   // Overall color of the material. AlbedoModulation is not strictly required (as this information will 
	//normally come from a texture), but it is very helpful for a "tweakable" shader

    mediump vec3 albedo = texture2D(sTexture, TexCoord).rgb * AlbedoModulation;

	//Add an ambient factor (greatly helps the look in general, otherwise high reflectivity makes the 
	//the object pitch black...)
	//Diffuse factor       (colored)  : AMBIENT * albedo * (1 - reflectivity)
	//Specular, metallic   (colored)  : AMBIENT * albedo * reflectivity * metallicity
	// ==> Colored ambient = AMBIENT * albedo * (1 + reflectivity * (metallicity  - 1))
	
	//Specular, non-metallic (white)  : AMBIENT * reflectivity * (1 - metallicity)

	mediump float colorAmbientFactor = Reflectivity * (Metallicity - 1.) + 1.;
	mediump float whiteAmbientFactor = Reflectivity * (1. - Metallicity);

	mediump vec3 ambient = vec3(AMBIENT * whiteAmbientFactor) + albedo * AMBIENT * colorAmbientFactor;

    mediump vec3 normal = normalize(ViewNormal);
	mediump float n_dot_l = max(dot(normal, -ViewLightDirection), 0.);
	mediump vec3 color = n_dot_l * albedo * (1. - Reflectivity); 

	if (n_dot_l > .0) //Skip if no specular needed...
	{
		mediump vec3 viewDirection = vec3(0.,0.,-1);
		mediump vec3 reflectedLightDirection = reflect(-ViewLightDirection, normal);
		mediump float v_dot_r = max(dot(viewDirection, reflectedLightDirection), 0.0);
		
		//Most metallic:    Specular Color = albedo color
		//Most un-metallic: Specular Color = white

		mediump float specularIntensity = max(pow(v_dot_r, SpecularExponent) * Reflectivity, 0.);

		mediump float metallicSpecularIntensity = Metallicity * specularIntensity;
		mediump float plasticSpecularIntensity = (1. - Metallicity) * specularIntensity;

		mediump vec3 specularColor = vec3(plasticSpecularIntensity) + metallicSpecularIntensity * albedo;
		color += specularColor;
	}
	gl_FragColor = vec4(color, 1.);
}