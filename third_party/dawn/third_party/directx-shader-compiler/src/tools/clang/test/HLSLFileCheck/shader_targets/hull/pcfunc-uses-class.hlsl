// RUN: %dxc -E main -T hs_6_0  %s | FileCheck %s

// Make sure input control point is not 0.
// CHECK: !{void ()* @"\01?HSPerPatchFunc@@YA?AVHSPerPatchData@@XZ", i32 1

// Ensure class works when detecting Patch Constant function
class HSPerPatchData
{
    // We at least have to specify tess factors per patch
    // As we're tesselating triangles, there will be 4 tess factors
    // In real life case this might contain face normal, for example
	float	edges[3] : SV_TessFactor;
	float	inside   : SV_InsideTessFactor;
};



// This overload is a patch constant function candidate because it has an
// output with the SV_TessFactor semantic. However, the compiler should
// *not* select it because there is another overload defined later in this
// translation unit (which is the old compiler's behavior). If it did, then
// the semantic checker will report an error due to this overload's input
// having 32 elements (versus the expected 3).
HSPerPatchData HSPerPatchFunc()
{
  HSPerPatchData d;

  d.edges[0] = -5;
  d.edges[1] = -6;
  d.edges[2] = -7;
  d.inside = -8;

  return d;
}



// hull per-control point shader
[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[patchconstantfunc("HSPerPatchFunc")]
[outputcontrolpoints(3)]
void main( const uint id : SV_OutputControlPointID )
{
}

