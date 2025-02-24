// flags: --overrides 1234=0
@id(1234) override o : i32;

@compute @workgroup_size(1)
fn main() {
    if o == 0 {
        _ = o;
    }
}
