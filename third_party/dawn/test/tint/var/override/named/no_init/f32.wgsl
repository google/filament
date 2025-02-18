// flags: --overrides o=0
override o : f32;

@compute @workgroup_size(1)
fn main() {
    if o == 0.0 {
        _ = 1;
    }
}
