struct frexp_result_f32 {
  f : f32,
}

var<private> a : frexp_result_f32;
var<private> b = frexp(1);

@fragment
fn main() -> @location(0) vec4f {
  return vec4f(a.f, b.fract, 0, 0);
}
