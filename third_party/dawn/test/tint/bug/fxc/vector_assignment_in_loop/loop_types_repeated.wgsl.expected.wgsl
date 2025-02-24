@compute @workgroup_size(1, 1, 1)
fn main() {
  var v2f : vec2<f32>;
  var v2f_2 : vec2<f32>;
  var v3i : vec3<i32>;
  var v3i_2 : vec3<i32>;
  var v4u : vec4<u32>;
  var v4u_2 : vec4<u32>;
  var v2b : vec2<bool>;
  var v2b_2 : vec2<bool>;
  for(var i : i32 = 0; (i < 2); i = (i + 1)) {
    v2f[i] = 1.0;
    v3i[i] = 1;
    v4u[i] = 1u;
    v2b[i] = true;
    v2f_2[i] = 1.0;
    v3i_2[i] = 1;
    v4u_2[i] = 1u;
    v2b_2[i] = true;
  }
}
