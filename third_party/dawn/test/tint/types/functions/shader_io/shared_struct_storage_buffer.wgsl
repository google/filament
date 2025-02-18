struct S {
  @align(64) @location(0) f : f32,
  @size(32) @location(1) @interpolate(flat) u : u32,
  @align(128) @builtin(position) v : vec4<f32>,
};

@group(0) @binding(0)
var<storage, read_write> output : S;

@fragment
fn frag_main(input : S) {
  let f : f32 = input.f;
  let u : u32 = input.u;
  let v : vec4<f32> = input.v;
  output = input;
}
