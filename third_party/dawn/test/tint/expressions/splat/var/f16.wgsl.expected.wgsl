enable f16;

fn f() {
  var v = (1.0h + 2.0h);
  var v2 : vec2<f16> = vec2<f16>(v);
  var v3 : vec3<f16> = vec3<f16>(v);
  var v4 : vec4<f16> = vec4<f16>(v);
}
