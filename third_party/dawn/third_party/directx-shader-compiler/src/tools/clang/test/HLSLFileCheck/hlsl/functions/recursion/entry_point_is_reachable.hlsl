// RUN: %dxc -E HSmain -T hs_6_0 %s | FileCheck %s

struct HSPerPatchData
{
  float edges[3] : SV_TessFactor;
  float inside   : SV_InsideTessFactor;
};

[shader("hull")]
[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("patchfn")]
void HSmain(uint ix : SV_OutputControlPointID);


// actual selected HSPerPatchFunc1 for HSMain1 and HSMain3
HSPerPatchData patchfn()
{
  HSPerPatchData d;

  d.edges[0] = -5;
  d.edges[1] = -6;
  d.edges[2] = -7;
  d.inside = -8;
// CHECK: error: entry function 'HSmain' should not be reachable from patch constant function 'patchfn'  
  HSmain(3);
  return d;
}

[shader("hull")]
[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("patchfn")]
void HSmain(uint ix : SV_OutputControlPointID)
{  
  return;
}

