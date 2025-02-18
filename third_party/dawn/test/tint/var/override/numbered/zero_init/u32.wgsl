// flags: --overrides 1234=0
@id(1234) override o : u32 = u32();

@compute @workgroup_size(1)
fn main() {
    if o == 1 {
        _ = o;
    }
}
