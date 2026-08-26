// RUN: dxc -Tlib_6_7 -Wno-hlsl-availability  %s -verify

SamplerComparisonState s;
Texture1D t;

// Make sure -Wno-hlsl-availability suppresses the error.
// expected-no-diagnostics

export
float foo(float a) {
  return t.CalculateLevelOfDetail(s, a) +
    t.CalculateLevelOfDetailUnclamped(s, a);
    }

float bar(float a) {
  return t.CalculateLevelOfDetail(s, a) +
    t.CalculateLevelOfDetailUnclamped(s, a);
}

float foo2(float a) {
  return t.CalculateLevelOfDetail(s, a) +
    t.CalculateLevelOfDetailUnclamped(s, a);
}

export
float bar2(float a) {
  return foo2(a);
}

[shader("pixel")]
float ps(float a:A) : SV_Target {
  return t.CalculateLevelOfDetail(s, a) +
    t.CalculateLevelOfDetailUnclamped(s, a);
}

[shader("vertex")]
float4 vs(float a:A) : SV_Position {
  return t.CalculateLevelOfDetail(s, a) +
    t.CalculateLevelOfDetailUnclamped(s, a);
}
