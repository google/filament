struct S {
  a : i32,
};

@group(0) @binding(0)
var<uniform> v : S;

@compute @workgroup_size(1)
fn main() {
  let p : ptr<uniform, i32> = &v.a;
  let u : i32 = *p + 1;
}
