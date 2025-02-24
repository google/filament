@compute @workgroup_size(1)
fn main() {
  const const_in = 1.25;
  let runtime_in = 1.25;
  var res = modf(const_in);
  res = modf(runtime_in);
  res = modf(const_in);
  let fract : f32 = res.fract;
  let whole : f32 = res.whole;
}
