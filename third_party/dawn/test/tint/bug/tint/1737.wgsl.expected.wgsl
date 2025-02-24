override wgsize : u32;

var<workgroup> a : array<f32, wgsize>;

var<workgroup> b : array<f32, (wgsize * 2)>;

fn f() {
  let x = a[0];
  let y = b[0];
}
