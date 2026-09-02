// RUN: %dxc -E main -T hs_6_0  %s 2>&1 | FileCheck %s

// Make sure generated buffer load for static buffer used in patch constant function.
// CHECK:call %dx.types.ResRet.f32 @dx.op.bufferLoad.f32(i32 68,

struct HSPerPatchData
{
	float	edges[3] : SV_TessFactor;
	float	inside   : SV_InsideTessFactor;
};

Buffer<float> buf;

const static Buffer<float> static_buf = buf;

HSPerPatchData HSPerPatchFunc()
{
  HSPerPatchData d;

  d.edges[0] = -5;
  d.edges[1] = -6;
  d.edges[2] = -7;
  d.inside = static_buf[0];

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