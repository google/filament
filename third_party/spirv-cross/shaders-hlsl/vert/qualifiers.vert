#version 450

layout(location = 0) flat out float vFlat;
layout(location = 1) centroid out float vCentroid;
layout(location = 2) sample out float vSample;
layout(location = 3) noperspective out float vNoperspective;

layout(location = 4) out Block
{
	flat float vFlat;
	centroid float vCentroid;
	sample float vSample;
	noperspective float vNoperspective;
} vout;

void main()
{
	gl_Position = vec4(1.0);
	vFlat = 0.0;
	vCentroid = 1.0;
	vSample = 2.0;
	vNoperspective = 3.0;
	vout.vFlat = 0.0;
	vout.vCentroid = 1.0;
	vout.vSample = 2.0;
	vout.vNoperspective = 3.0;
}
