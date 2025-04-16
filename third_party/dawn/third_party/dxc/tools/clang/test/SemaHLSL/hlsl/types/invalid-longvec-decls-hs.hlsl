// RUN: %dxc -DTYPE=float -DNUM=7 -T hs_6_9 -verify %s

struct HsConstantData {
  float Edges[3] : SV_TessFactor;
  vector <float, 7> vec;
};

struct LongVec {
  float4 f;
  vector<TYPE,NUM> vec;
};

HsConstantData PatchConstantFunction( // expected-error{{vectors of over 4 elements in patch constant function return type are not supported}}
				      vector<TYPE,NUM> vec : V, // expected-error{{vectors of over 4 elements in patch constant function parameters are not supported}}
				      LongVec lv : L) { // expected-error{{vectors of over 4 elements in patch constant function parameters are not supported}}
  return (HsConstantData)0;
}

[domain("tri")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(32)]
[patchconstantfunc("PatchConstantFunction")]
void main() {
}
