struct S {
  v : vec3<i32>,
}

var<private> P : S;

fn f() {
  P.v = vec3<i32>(1, 2, 3);
  P.v.x = 1;
  P.v.y = 2;
  P.v.z = 3;
}
