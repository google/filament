@group(0) @binding(0)
var<storage, read_write> buffer : u32;

const kArray = array(0u, 1u, 2u, 4u);

fn foo() -> u32 {
    return kArray[buffer];
}

@compute @workgroup_size(1)
fn main() {
    let v = kArray[buffer];
    buffer = v + foo();
}
