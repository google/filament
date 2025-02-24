// flags: --overrides o=0
override o : u32 = 1u;

@compute @workgroup_size(1)
fn main() {
    if o == 2 {
        _ = o;
    }
}
