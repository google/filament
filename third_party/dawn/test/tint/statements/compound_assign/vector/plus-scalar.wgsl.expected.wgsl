struct S {
  a : vec4<f32>,
}

@group(0) @binding(0) var<storage, read_write> v : S;

fn foo() {
  v.a += 2.0;
}
