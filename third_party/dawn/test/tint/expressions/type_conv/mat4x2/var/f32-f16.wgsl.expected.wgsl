enable f16;

var<private> u = mat4x2<f32>(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);

fn f() {
  var v : mat4x2<f16> = mat4x2<f16>(u);
}
