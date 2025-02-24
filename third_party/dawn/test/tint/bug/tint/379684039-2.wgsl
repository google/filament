var<private> idx: u32;

struct FSUniformData {
    k: array<vec4f, 7>,
    size: vec2i,
}
struct FSUniforms {
    fsUniformData: array<FSUniformData>,
}

@group(0) @binding(2) var<storage, read> _storage : FSUniforms;
fn main() {
    var vec: vec2<i32> = vec2<i32>(0);
    loop {
        if vec.y >= _storage.fsUniformData[idx].size.y {
            break;
        }
    }
}
