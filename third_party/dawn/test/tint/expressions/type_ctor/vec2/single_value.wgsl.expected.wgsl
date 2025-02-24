fn v() -> vec2<f32> {
  return vec2(0.0);
}

fn f() {
  let a = vec2(1.0);
  let b = vec2(a);
  let c = vec2(v());
  let d = vec2((a * 2));
}
