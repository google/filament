struct FragIn {
  @location(0) a : f32,
  @builtin(sample_mask) mask : u32,
};

@fragment
fn main(in : FragIn, @location(1) b : f32) -> FragIn {
  if (in.mask == 0u) {
    return in;
  }
  return FragIn(b, 1u);
}
