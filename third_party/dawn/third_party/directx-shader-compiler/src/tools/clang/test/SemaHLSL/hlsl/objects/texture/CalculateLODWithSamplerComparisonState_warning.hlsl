// RUN: dxc -Tlib_6_7 %s -Wno-error=hlsl-availability -verify

SamplerComparisonState s;
Texture1D t;

// Make sure direct call to CalculateLevelOfDetail and CalculateLevelOfDetailUnclamped get warning with -Wno-error.

export
float foo(float a) {
  return t.CalculateLevelOfDetail(s, a) + // expected-warning {{overload of intrinsic CalculateLevelOfDetail with SamplerComparisonState requires shader model 6.8 or greater}}
    t.CalculateLevelOfDetailUnclamped(s, a); // expected-warning {{overload of intrinsic CalculateLevelOfDetailUnclamped with SamplerComparisonState requires shader model 6.8 or greater}}
}

// Make sure unused function call to CalculateLevelOfDetail and CalculateLevelOfDetailUnclamped don't get warning with -Wno-error.
float bar(float a) {
  return t.CalculateLevelOfDetail(s, a) +
    t.CalculateLevelOfDetailUnclamped(s, a);
}

// Make sure nested call to CalculateLevelOfDetail and CalculateLevelOfDetailUnclamped get warning with -Wno-error.
float foo2(float a) {
  return t.CalculateLevelOfDetail(s, a) + // expected-warning {{overload of intrinsic CalculateLevelOfDetail with SamplerComparisonState requires shader model 6.8 or greater}}
    t.CalculateLevelOfDetailUnclamped(s, a); // expected-warning {{overload of intrinsic CalculateLevelOfDetailUnclamped with SamplerComparisonState requires shader model 6.8 or greater}}
}

export
float bar2(float a) {
  return foo2(a);
}

// Make sure only report warning on a call once.
export
float bar3(float a) {
  return foo2(a);
}
// Make sure report warning with -Wno-error when derivatives not supported.

[shader("pixel")]
float ps(float a:A) : SV_Target {
  return t.CalculateLevelOfDetail(s, a) + // expected-warning {{overload of intrinsic CalculateLevelOfDetail with SamplerComparisonState requires shader model 6.8 or greater}}
    t.CalculateLevelOfDetailUnclamped(s, a); // expected-warning {{overload of intrinsic CalculateLevelOfDetailUnclamped with SamplerComparisonState requires shader model 6.8 or greater}}
}

[shader("vertex")]
float4 vs(float a:A) : SV_Position {
  return t.CalculateLevelOfDetail(s, a) + // expected-warning {{overload of intrinsic CalculateLevelOfDetail with SamplerComparisonState requires shader model 6.8 or greater}}
    t.CalculateLevelOfDetailUnclamped(s, a); // expected-warning {{overload of intrinsic CalculateLevelOfDetailUnclamped with SamplerComparisonState requires shader model 6.8 or greater}}
}
