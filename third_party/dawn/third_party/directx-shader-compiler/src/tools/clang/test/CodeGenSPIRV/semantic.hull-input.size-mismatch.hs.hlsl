// RUN: %dxc -T hs_6_0 -E main %s -spirv -verify


struct ControlPoint { float4 position : POSITION; };

struct HullPatchOut {
    float edge [3] : SV_TessFactor;
    float inside : SV_InsideTessFactor;
};

HullPatchOut HullConst (InputPatch<ControlPoint,2> v) { /* expected-error{{Patch constant function's input patch input should have 3 elements, but has 2.}} */
  return (HullPatchOut)0;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[patchconstantfunc("HullConst")]
[outputcontrolpoints(1)]
ControlPoint main(InputPatch<ControlPoint,3> v, uint id : SV_OutputControlPointID) {
  return v[id];
}
