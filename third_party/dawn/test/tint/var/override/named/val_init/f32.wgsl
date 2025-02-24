// flags: --overrides o=0
override o : f32 = 1.0;

@compute @workgroup_size(1)
fn main() {
    if o == 0.0 {
        _ = o;
    }
}
