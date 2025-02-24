enable f16;

struct Outputs {
  @location(1)
  a : f16,
  @location(2)
  b : vec4<f16>,
}

@fragment
fn frag_main(@location(1) loc1 : f16, @location(2) loc2 : vec4<f16>) -> Outputs {
  return Outputs((loc1 * 2), (loc2 * 3));
}
