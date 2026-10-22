// RUN: %dxc -T ps_6_0 -verify %s

// expected-no-diagnostics
// No diagnostic is expected because this is a non-library target,
// and SubObjects are ignored on non-library targets.

RaytracingPipelineConfig1 rpc = { 32, RAYTRACING_PIPELINE_FLAG_ALLOW_OPACITY_MICROMAPS };

[shader("pixel")]
int main(int i : INDEX) : SV_Target {
  return 1;
}
