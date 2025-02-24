fn X(a : vec2f, b : vec2f) {
}

fn Y() -> vec2f {
  return vec2f();
}

fn f() {
  var v : vec2f;
  X(vec2(), v);
  X(vec2(), Y());
}
