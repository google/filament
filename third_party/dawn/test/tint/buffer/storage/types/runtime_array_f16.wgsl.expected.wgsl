enable f16;

@group(0) @binding(0) var<storage, read> in : array<f16>;

@group(0) @binding(1) var<storage, read_write> out : array<f16>;

@compute @workgroup_size(1)
fn main() {
  out[0] = in[0];
}
