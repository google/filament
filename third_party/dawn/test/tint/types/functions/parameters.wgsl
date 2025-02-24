struct S {
  a : f32,
};

fn foo(
  param_bool : bool,
  param_i32 : i32,
  param_u32 : u32,
  param_f32 : f32,
  param_v2i32 : vec2<i32>,
  param_v3u32 : vec3<u32>,
  param_v4f32 : vec4<f32>,
  param_m2x3 : mat2x3<f32>,
  param_arr : array<f32, 4>,
  param_struct : S,
  param_ptr_f32 : ptr<function, f32>,
  param_ptr_vec : ptr<function, vec4<f32>>,
  param_ptr_arr : ptr<function, array<f32, 4>>,
) {}

@compute @workgroup_size(1)
fn main() {
    let a = array<f32, 4>();
    var b = 1.f;
    var c = vec4f();
    var d = array<f32, 4>();

    foo(true, 1i, 1u, 1f, vec2i(3), vec3u(4), vec4f(5), mat2x3f(), a, S(), &b, &c, &d);
}
