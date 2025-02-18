alias MyArray = array<f32, 10>;

const c1 = 1;

const c2 = 1u;

const c3 = 1.0;

const c4 = vec3<i32>(1, 1, 1);

const c5 = vec3<u32>(1u, 1u, 1u);

const c6 = vec3<f32>(1.0, 1.0, 1.0);

const c7 = mat3x3<f32>(vec3<f32>(1.0, 1.0, 1.0), vec3<f32>(1.0, 1.0, 1.0), vec3<f32>(1.0, 1.0, 1.0));

const c9 = MyArray();

@fragment
fn main() -> @location(0) vec4<f32> {
  var v1 = c1;
  var v2 = c2;
  var v3 = c3;
  var v4 = c4;
  var v5 = c5;
  var v6 = c6;
  var v7 = c7;
  var v9 = c9;
  return vec4<f32>(0.0, 0.0, 0.0, 0.0);
}
