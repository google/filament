struct S {
  field0 : mat3x3f,
}

struct S_1 {
  field0 : array<array<array<S, 83u>, 21u>, 47u>,
}

struct S_2 {
  field0 : array<array<vec3f, 37u>, 95u>,
}

struct S_3 {
  field0 : S_2,
}

struct S_4 {
  field0 : array<vec2i, 56u>,
}

struct S_5 {
  field0 : S_4,
}

struct S_6 {
  field0 : array<array<vec3f, 18u>, 13u>,
}

struct S_7 {
  field0 : array<vec2i, 88u>,
}

var<private> x_75 = array<mat4x4f, 58u>();

var<private> x_82 = array<S_6, 46u>();

var<private> x_85 = array<vec3f, 37u>();

fn main_1() {
  let x_88 = 58u;
  return;
}

@fragment
fn main() {
  main_1();
}
