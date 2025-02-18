enable f16;

struct S {
  v : vec3<f16>,
}

@group(0) @binding(0) var<storage, read_write> U : S;

fn f() {
  U.v = vec3<f16>(1.0h, 2.0h, 3.0h);
  U.v.x = 1.0h;
  U.v.y = 2.0h;
  U.v.z = 3.0h;
}
