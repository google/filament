// Verifies that smoothstep with (effective) constant
// values of low > high compiles on all backends

@compute @workgroup_size(1)
fn main() {
    let low = 1.0;
    let high = 0.0;
    let x_val = 0.5;
    let res = smoothstep(low,high, x_val);
}
