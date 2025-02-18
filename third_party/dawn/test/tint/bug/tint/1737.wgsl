// flags: --overrides wgsize=10

override wgsize : u32;
var<workgroup> a : array<f32, wgsize>; // Accepted
var<workgroup> b : array<f32, wgsize * 2>; // Rejected

fn f() {
    let x = a[0];
    let y = b[0];
}
