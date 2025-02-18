enable f16;

var<private> u = mat3x3<f32>(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f);

fn f() {
  var v : mat3x3<f16> = mat3x3<f16>(u);
}
