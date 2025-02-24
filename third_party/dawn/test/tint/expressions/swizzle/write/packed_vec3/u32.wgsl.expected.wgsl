struct S {
  v : vec3<u32>,
}

@group(0) @binding(0) var<storage, read_write> U : S;

fn f() {
  U.v = vec3<u32>(1u, 2u, 3u);
  U.v.x = 1u;
  U.v.y = 2u;
  U.v.z = 3u;
}
