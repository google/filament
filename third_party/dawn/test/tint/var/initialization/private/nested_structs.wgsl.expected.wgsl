struct S1 {
  i : i32,
}

struct S2 {
  s1 : S1,
}

struct S3 {
  s2 : S2,
}

const C = 42;

var<private> P = S3(S2(S1(C)));

@group(0) @binding(0) var<storage, read_write> out : i32;

@compute @workgroup_size(1)
fn main() {
  out = P.s2.s1.i;
}
