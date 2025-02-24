#version 310 es


struct Uniforms {
  uvec2 aShape;
  uvec2 bShape;
  uvec2 outShape;
};

layout(binding = 0, std430)
buffer Matrix_1_ssbo {
  uint numbers[];
} firstMatrix;
layout(binding = 1, std430)
buffer Matrix_2_ssbo {
  uint numbers[];
} secondMatrix;
layout(binding = 2, std430)
buffer Matrix_3_ssbo {
  uint numbers[];
} resultMatrix;
layout(binding = 3, std140)
uniform uniforms_block_1_ubo {
  Uniforms inner;
} v;
void main_inner(uvec3 global_id) {
  uvec2 resultCell = uvec2(global_id.y, global_id.x);
  uint dimInner = v.inner.aShape.y;
  uint dimOutter = v.inner.outShape.y;
  uint result = 0u;
  {
    uint i = 0u;
    while(true) {
      if ((i < dimInner)) {
      } else {
        break;
      }
      uint a = (i + (resultCell.x * dimInner));
      uint b = (resultCell.y + (i * dimOutter));
      uint v_1 = result;
      uint v_2 = min(a, (uint(firstMatrix.numbers.length()) - 1u));
      uint v_3 = firstMatrix.numbers[v_2];
      uint v_4 = min(b, (uint(secondMatrix.numbers.length()) - 1u));
      result = (v_1 + (v_3 * secondMatrix.numbers[v_4]));
      {
        i = (i + 1u);
      }
      continue;
    }
  }
  uint index = (resultCell.y + (resultCell.x * dimOutter));
  uint v_5 = min(index, (uint(resultMatrix.numbers.length()) - 1u));
  resultMatrix.numbers[v_5] = result;
}
layout(local_size_x = 2, local_size_y = 2, local_size_z = 1) in;
void main() {
  main_inner(gl_GlobalInvocationID);
}
