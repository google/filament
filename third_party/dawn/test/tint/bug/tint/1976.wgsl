@group(0) @binding(0) var texture0 : texture_multisampled_2d<f32>;

struct Results {
    colorSamples : array<f32, 4>,
}
@group(0) @binding(2) var<storage, read_write> results : Results;

@compute @workgroup_size(1) fn main() {
    results.colorSamples[0] = textureLoad(texture0, vec2i(0, 0), 0).x;
}
