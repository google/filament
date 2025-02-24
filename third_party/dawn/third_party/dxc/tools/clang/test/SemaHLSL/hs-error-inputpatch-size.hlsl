// RUN: %dxc -T hs_6_0 -E main -verify %s

struct ControlPoint {
  float position : MY_BOOL;
};

struct PatchData {
    float edge [3] : SV_TessFactor;
    float inside : SV_InsideTessFactor;
};

PatchData HullConst () { return (PatchData)0; }

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[patchconstantfunc("HullConst")]
[outputcontrolpoints(3)]
ControlPoint main(InputPatch<ControlPoint, 0> v, // expected-error {{InputPatch element count must be greater than 0}}
                  uint id : SV_OutputControlPointID) {
  return v[id];
}
