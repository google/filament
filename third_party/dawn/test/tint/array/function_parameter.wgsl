@group(0) @binding(0) var<storage, read_write> s: f32;

fn f1(a : array<f32, 4>) -> f32 {
  return a[3];
}

fn f2(a : array<array<f32, 4>, 3>) -> f32 {
  return a[2][3];
}

fn f3(a : array<array<array<f32, 4>, 3>, 2>) -> f32 {
  return a[1][2][3];
}

@compute @workgroup_size(1)
fn main() {
  let a1 : array<f32, 4> = array<f32, 4>();
  let a2 : array<array<f32, 4>, 3> = array<array<f32, 4>, 3>();
  let a3 : array<array<array<f32, 4>, 3>, 2> = array<array<array<f32, 4>, 3>, 2>();
  let v1 : f32 = f1(a1);
  let v2 : f32 = f2(a2);
  let v3 : f32 = f3(a3);

  s = v1 + v2 + v3;
}
