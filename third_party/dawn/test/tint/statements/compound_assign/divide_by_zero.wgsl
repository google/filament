var<private> a : i32;
var<private> b : f32;

fn foo(maybe_zero : i32) {
  a /= maybe_zero;
  a %= maybe_zero;

  b /= 0.0;
  b %= 0.0;
  b /= f32(maybe_zero);
  b %= f32(maybe_zero);
}
