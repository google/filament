struct FragmentOutputs {
  @builtin(frag_depth) frag_depth : f32,
  @builtin(sample_mask) sample_mask : u32,
};

@fragment
fn main() -> FragmentOutputs {
  return FragmentOutputs(1.0, 1u);
}
