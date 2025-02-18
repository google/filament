struct Uniforms {
  aShape : vec2<u32>,
  bShape : vec2<u32>,
  outShape : vec2<u32>,
}

struct Matrix {
  numbers : array<u32>,
}

@group(0) @binding(0) var<storage, read> firstMatrix : Matrix;

@group(0) @binding(1) var<storage, read> secondMatrix : Matrix;

@group(0) @binding(2) var<storage, read_write> resultMatrix : Matrix;

@group(0) @binding(3) var<uniform> uniforms : Uniforms;

@compute @workgroup_size(2, 2, 1)
fn main(@builtin(global_invocation_id) global_id : vec3<u32>) {
  let resultCell : vec2<u32> = vec2<u32>(global_id.y, global_id.x);
  let dimInner : u32 = uniforms.aShape.y;
  let dimOutter : u32 = uniforms.outShape.y;
  var result : u32 = 0u;
  for(var i : u32 = 0u; (i < dimInner); i = (i + 1u)) {
    let a : u32 = (i + (resultCell.x * dimInner));
    let b : u32 = (resultCell.y + (i * dimOutter));
    result = (result + (firstMatrix.numbers[a] * secondMatrix.numbers[b]));
  }
  let index : u32 = (resultCell.y + (resultCell.x * dimOutter));
  resultMatrix.numbers[index] = result;
}
