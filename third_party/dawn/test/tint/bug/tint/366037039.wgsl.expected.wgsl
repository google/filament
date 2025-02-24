struct S {
  a : vec3u,
  b : u32,
  c : array<vec3u, 4>,
}

@group(0) @binding(0) var<uniform> ubuffer : S;

@group(0) @binding(1) var<storage, read_write> sbuffer : S;

var<workgroup> wbuffer : S;

fn foo() {
  let u = ubuffer;
  let s = sbuffer;
  let w = sbuffer;
  sbuffer = S();
  wbuffer = S();
}
