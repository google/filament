var<private> v : f32;

fn x(p : ptr<private, f32>) {
  *(p) = 0.0;
}

fn g() {
  x(&(v));
}

@fragment
fn f() {
  g();
}
