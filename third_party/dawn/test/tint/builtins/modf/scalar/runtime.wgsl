@compute @workgroup_size(1)
fn main() {
    let in = 1.25;
    let res = modf(in);
    let fract : f32 = res.fract;
    let whole : f32 = res.whole;
}
