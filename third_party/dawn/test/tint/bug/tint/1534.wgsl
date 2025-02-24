
struct g {
  a : vec3<u32>,
}

struct h {
  a : u32,
}

@group(0) @binding(0) var<uniform> i : g;

@group(0) @binding(1) var<storage, read_write> j : h;

@compute @workgroup_size(1) fn main() {
  let l = dot(i.a, i.a);
  j.a = i.a.x;
}
