@compute @workgroup_size(1)
fn main() {
    const const_in = vec2(1.25, 3.75);
    let runtime_in = vec2(1.25, 3.75);
    var res = frexp(const_in);
    res = frexp(runtime_in);
    res = frexp(const_in);
    let fract : vec2<f32> = res.fract;
    let exp : vec2<i32> = res.exp;
}
