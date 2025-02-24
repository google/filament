// RUN: %dxc -T lib_6_5 %s | FileCheck %s

struct HSPerPatchData
{
  float edges[3] : SV_TessFactor;
  float inside   : SV_InsideTessFactor;
};

// actual selected HSPerPatchFunc1 for HSMain1 and HSMain3
HSPerPatchData fooey()
{
  HSPerPatchData d;

  d.edges[0] = -5;
  d.edges[1] = -6;
  d.edges[2] = -7;
  d.inside = -8;
  // CHECK: error: recursive functions are not allowed: function 'fooey' calls recursive function 'fooey'
  fooey();
  return d;
}

[shader("hull")]
[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("fooey")]
void HSmain(uint ix : SV_OutputControlPointID)
{
  return;
}

