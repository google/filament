enable f16;

fn get_f16() -> f16 {
  return 1.0h;
}

fn f() {
  var v2 : vec2<f16> = vec2<f16>(get_f16());
  var v3 : vec3<f16> = vec3<f16>(get_f16());
  var v4 : vec4<f16> = vec4<f16>(get_f16());
}
