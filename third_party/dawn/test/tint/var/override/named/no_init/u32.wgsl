// flags: --overrides o=0
override o : u32;

@compute @workgroup_size(1)
fn main() {
    if o == 1 {
        _ = o;
    }
}
