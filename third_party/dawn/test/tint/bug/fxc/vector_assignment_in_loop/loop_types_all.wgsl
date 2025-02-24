@compute @workgroup_size(1, 1, 1)
fn main() {
  var v2f : vec2<f32>;
  var v3f : vec3<f32>;
  var v4f : vec4<f32>;
  var v2i : vec2<i32>;
  var v3i : vec3<i32>;
  var v4i : vec4<i32>;
  var v2u : vec2<u32>;
  var v3u : vec3<u32>;
  var v4u : vec4<u32>;
  var v2b : vec2<bool>;
  var v3b : vec3<bool>;
  var v4b : vec4<bool>;

  for (var i : i32 = 0; i < 2; i = i + 1) {
    v2f[i] = 1.0;
    v3f[i] = 1.0;
    v4f[i] = 1.0;
    v2i[i] = 1;
    v3i[i] = 1;
    v4i[i] = 1;
    v2u[i] = 1u;
    v3u[i] = 1u;
    v4u[i] = 1u;
    v2b[i] = true;
    v3b[i] = true;
    v4b[i] = true;
  }
}
