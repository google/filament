var<private> v2f : vec2<f32>;

var<private> v3i : vec3<i32>;

var<private> v4u : vec4<u32>;

var<private> v2b : vec2<bool>;

fn foo() {
  for(var i : i32 = 0; (i < 2); i = (i + 1)) {
    v2f[i] = 1.0;
    v3i[i] = 1;
    v4u[i] = 1u;
    v2b[i] = true;
  }
}

@compute @workgroup_size(1, 1, 1)
fn main() {
  for(var i : i32 = 0; (i < 2); i = (i + 1)) {
    foo();
  }
}
