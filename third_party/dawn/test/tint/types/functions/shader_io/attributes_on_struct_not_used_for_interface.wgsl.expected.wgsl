struct S {
  @align(64) @location(0)
  f : f32,
  @size(32) @location(1) @interpolate(flat)
  u : u32,
  @align(128) @builtin(position)
  v : vec4<f32>,
}

@group(0) @binding(0) var<storage, read_write> output : S;

@fragment
fn frag_main() {
  output = S(1, 2, vec4(3));
}
