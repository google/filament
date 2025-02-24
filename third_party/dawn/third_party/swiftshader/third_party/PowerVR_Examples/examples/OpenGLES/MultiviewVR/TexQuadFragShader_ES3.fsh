#version 300 es
uniform mediump sampler2DArray sTexture;
uniform int layerIndex;
in mediump vec2 HighResTexCoord;
in mediump vec2 LowResTexCoord;
out mediump vec4 oColor;
void main()
{
    mediump vec3 highResSample = texture(sTexture, vec3(HighResTexCoord,layerIndex + 2)).rgb;
    mediump vec3 lowResSample = texture(sTexture, vec3(LowResTexCoord,layerIndex)).rgb;
    // calculate the squared distance to middle of screen
    // centre of the screen: (-0.5 + 1.5) / 2.0 = 0.5
    mediump vec2 dist = vec2(0.5) - HighResTexCoord;
    mediump float squareDist = dot(dist,dist);
    // High resolution texture is used when distance from centre is less than 0.5(0.25 is 0.5 squared) in texture coordinates. 
    // When the distance is less than 0.2 (0.04 is 0.2 squared), only the high res texture will be used.
    mediump float lerp = smoothstep(-0.25, -0.04, -squareDist);
    mediump vec3 color = mix(lowResSample * vec3(0.4, 0.2, 0.2), highResSample * vec3(0.6, 0.6, 1.0) + vec3(0.0, 0.0, 0.0), lerp);
#ifndef FRAMEBUFFER_SRGB
	color = pow(color, vec3(0.4545454545)); // Do gamma correction
#endif
	oColor = vec4(color, 1.0);
}
