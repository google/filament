// RUN: %dxc -E HSmain -T hs_6_0 %s | FileCheck %s

struct HSPerPatchData
{
  float edges[3] : SV_TessFactor;
  float inside   : SV_InsideTessFactor;
};

bool recurse(int x) {
  if (x) {
    return !recurse(x+1);
  } 
  return true;
}

// actual selected HSPerPatchFunc1 for HSMain1 and HSMain3
HSPerPatchData patchfn()
{
  HSPerPatchData d;

  d.edges[0] = -5;
  d.edges[1] = -6;
  d.edges[2] = -7;
  d.inside = -8;

  HSPerPatchData e;

  e.edges[0] = -5;
  e.edges[1] = -6;
  e.edges[2] = -7;
  e.inside = -9;

  if (recurse(e.inside)) {
    return d;
  }
  return e;
}

[shader("hull")]
[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("patchfn")]
// CHECK: error: recursive functions are not allowed: function 'patchfn' calls recursive function 'recurse'
void HSmain(uint ix : SV_OutputControlPointID)
{
  return;
}

