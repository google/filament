#version 300 es

#define ANTIALIAS_STRENGTH 2.
#define OUTLINE_WIDTH .25

uniform mediump vec4 myColor;

in mediump vec2 roadCoords;

out mediump vec4 oColor;

void main(void)
{
	mediump float distance = 1.0 - abs(roadCoords.x);
	if (roadCoords.y > 0.5) distance = distance * (1.0 -(roadCoords.y - 0.5) * 2.0);
	
	// We are using the rate of change of the texture coordinate, in order to detect the "zoom level", and vary the "ramp" of our blending accordingly.
	// You can increase ANTIALIAS_STRENGTH to exaggerate the effect and see the actual technique.
	// You can decrease ANTIALIAS_STRENGTH to make lines a bit crisper in lower zoom levels at the cost of more aliasing.
	// Recommended value is 2 or 4 for an even stronger (but blurrier) effect.
	// Or, you can experiment with different functions.
	mediump float blend_range = ANTIALIAS_STRENGTH
	 * sqrt(
		 dFdx(roadCoords.x) * dFdx(roadCoords.x) + 
		 dFdy(roadCoords.x) * dFdy(roadCoords.x));
	mediump float blend_halfrng = 0.5 * blend_range;

	mediump float blend_distance = ((distance - blend_halfrng) / blend_range) + 0.5;
	blend_distance = clamp(blend_distance, 0.0, 1.0);
	
	mediump float outline_distance = ((distance - OUTLINE_WIDTH - blend_halfrng) / blend_range) + 0.5;
	
	oColor.rgb = mix(vec3(0.0, 0.0, 0.0), myColor.rgb, clamp(outline_distance, 0.0, 1.0));
	oColor.a = blend_distance;
}



