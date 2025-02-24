@compute @workgroup_size(1)
fn main() {
  const in = vec2(1.25, 3.75);
  let res = modf(in);
  let fract : vec2<f32> = res.fract;
  let whole : vec2<f32> = res.whole;
}
