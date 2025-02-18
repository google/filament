struct FragmentOutputs {
  @location(0) loc0 : i32,
  @builtin(frag_depth) frag_depth : f32,
  @location(1) loc1 : u32,
  @location(2) loc2 : f32,
  @builtin(sample_mask) sample_mask : u32,
  @location(3) loc3 : vec4<f32>,
};

@fragment
fn main() -> FragmentOutputs {
  return FragmentOutputs(1, 2.0, 1u, 1.0, 2u, vec4<f32>(1.0, 2.0, 3.0, 4.0));
}
