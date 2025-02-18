struct Inner {
  b : bool,
  v : vec4<i32>,
  m : mat3x3<f32>,
}

struct Outer {
  a : array<Inner, 4>,
}

var<workgroup> v : Outer;

fn foo() -> Outer {
  return workgroupUniformLoad(&(v));
}
