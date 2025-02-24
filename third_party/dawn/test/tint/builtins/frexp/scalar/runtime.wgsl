@compute @workgroup_size(1)
fn main() {
    let in = 1.25;
    let res = frexp(in);
    let fract : f32 = res.fract;
    let exp : i32 = res.exp;
}
