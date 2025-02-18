struct S {
  v : vec3<f32>,
}

var<private> P : S;

fn f() {
  P.v = vec3<f32>(1.0, 2.0, 3.0);
  P.v.x = 1.0;
  P.v.y = 2.0;
  P.v.z = 3.0;
}
