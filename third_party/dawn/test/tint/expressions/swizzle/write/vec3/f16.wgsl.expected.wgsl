enable f16;

struct S {
  v : vec3<f16>,
}

var<private> P : S;

fn f() {
  P.v = vec3<f16>(1.0h, 2.0h, 3.0h);
  P.v.x = 1.0h;
  P.v.y = 2.0h;
  P.v.z = 3.0h;
}
