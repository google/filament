// flags: --overrides o=0
override o : i32 = i32();

@compute @workgroup_size(1)
fn main() {
    if o == 2 {
        _ = o;
    }
}
