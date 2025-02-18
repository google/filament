enable f16;

var<private> u = mat2x3<f16>(1.0h, 2.0h, 3.0h, 4.0h, 5.0h, 6.0h);

fn f() {
  var v : mat2x3<f32> = mat2x3<f32>(u);
}
