@fragment
fn main1() -> @builtin(frag_depth) f32 {
  return 1.0;
}

@fragment
fn main2() -> @builtin(sample_mask) u32 {
  return 1u;
}
