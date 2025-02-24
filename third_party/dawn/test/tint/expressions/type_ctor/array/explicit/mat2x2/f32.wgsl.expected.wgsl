var<private> arr = array<mat2x2<f32>, 2>(mat2x2<f32>(1.0f, 2.0f, 3.0f, 4.0f), mat2x2<f32>(5.0f, 6.0f, 7.0f, 8.0f));

fn f() {
  var v = arr;
}
