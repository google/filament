var<private> v : mat2x3<f32>;

@compute @workgroup_size(1)
fn main() {
    _ = v;
}
