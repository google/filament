// RUN: %dxc -T hs_6_9 -verify %s

struct HsConstantData {
  float Edges[3] : SV_TessFactor;
  dx::HitObject hit;
};

struct LongVec {
  float4 f;
  dx::HitObject hit;
};

HsConstantData
PatchConstantFunction(
    // expected-error@-1{{object 'dx::HitObject' is not allowed in patch constant function return type}}
    // expected-note@5{{'dx::HitObject' field declared here}}
	  dx::HitObject hit : V,
    // expected-error@-1{{object 'dx::HitObject' is not allowed in patch constant function parameters}}
	  LongVec lv : L)
    // expected-error@-1{{object 'dx::HitObject' is not allowed in patch constant function parameters}}
    // expected-note@10{{'dx::HitObject' field declared here}}
{
  HsConstantData empty;
  return empty;
}

[domain("tri")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(32)]
[patchconstantfunc("PatchConstantFunction")]
void main() {
}
