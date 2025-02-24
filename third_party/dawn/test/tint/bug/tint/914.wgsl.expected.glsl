#version 310 es


struct Uniforms {
  uint dimAOuter;
  uint dimInner;
  uint dimBOuter;
};

layout(binding = 0, std430)
buffer Matrix_1_ssbo {
  float numbers[];
} firstMatrix;
layout(binding = 1, std430)
buffer Matrix_2_ssbo {
  float numbers[];
} secondMatrix;
layout(binding = 2, std430)
buffer Matrix_3_ssbo {
  float numbers[];
} resultMatrix;
layout(binding = 3, std140)
uniform uniforms_block_1_ubo {
  Uniforms inner;
} v;
shared float mm_Asub[64][64];
shared float mm_Bsub[64][64];
float mm_readA(uint row, uint col) {
  bool v_1 = false;
  if ((row < v.inner.dimAOuter)) {
    v_1 = (col < v.inner.dimInner);
  } else {
    v_1 = false;
  }
  if (v_1) {
    uint v_2 = ((row * v.inner.dimInner) + col);
    uint v_3 = min(v_2, (uint(firstMatrix.numbers.length()) - 1u));
    float result = firstMatrix.numbers[v_3];
    return result;
  }
  return 0.0f;
}
float mm_readB(uint row, uint col) {
  bool v_4 = false;
  if ((row < v.inner.dimInner)) {
    v_4 = (col < v.inner.dimBOuter);
  } else {
    v_4 = false;
  }
  if (v_4) {
    uint v_5 = ((row * v.inner.dimBOuter) + col);
    uint v_6 = min(v_5, (uint(secondMatrix.numbers.length()) - 1u));
    float result = secondMatrix.numbers[v_6];
    return result;
  }
  return 0.0f;
}
void mm_write(uint row, uint col, float value) {
  bool v_7 = false;
  if ((row < v.inner.dimAOuter)) {
    v_7 = (col < v.inner.dimBOuter);
  } else {
    v_7 = false;
  }
  if (v_7) {
    uint index = (col + (row * v.inner.dimBOuter));
    uint v_8 = min(index, (uint(resultMatrix.numbers.length()) - 1u));
    resultMatrix.numbers[v_8] = value;
  }
}
uint tint_div_u32(uint lhs, uint rhs) {
  return (lhs / mix(rhs, 1u, (rhs == 0u)));
}
void main_inner(uvec3 local_id, uvec3 global_id, uint tint_local_index) {
  {
    uint v_9 = 0u;
    v_9 = tint_local_index;
    while(true) {
      uint v_10 = v_9;
      if ((v_10 >= 4096u)) {
        break;
      }
      mm_Asub[(v_10 / 64u)][(v_10 % 64u)] = 0.0f;
      mm_Bsub[(v_10 / 64u)][(v_10 % 64u)] = 0.0f;
      {
        v_9 = (v_10 + 256u);
      }
      continue;
    }
  }
  barrier();
  uint tileRow = (local_id.y * 4u);
  uint tileCol = (local_id.x * 4u);
  uint globalRow = (global_id.y * 4u);
  uint globalCol = (global_id.x * 4u);
  uint numTiles = (tint_div_u32((v.inner.dimInner - 1u), 64u) + 1u);
  float acc[16] = float[16](0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float ACached = 0.0f;
  float BCached[4] = float[4](0.0f, 0.0f, 0.0f, 0.0f);
  {
    uint index = 0u;
    while(true) {
      if ((index < 16u)) {
      } else {
        break;
      }
      uint v_11 = min(index, 15u);
      acc[v_11] = 0.0f;
      {
        index = (index + 1u);
      }
      continue;
    }
  }
  uint ColPerThreadA = 4u;
  uint tileColA = (local_id.x * ColPerThreadA);
  uint RowPerThreadB = 4u;
  uint tileRowB = (local_id.y * RowPerThreadB);
  {
    uint t = 0u;
    while(true) {
      if ((t < numTiles)) {
      } else {
        break;
      }
      {
        uint innerRow = 0u;
        while(true) {
          if ((innerRow < 4u)) {
          } else {
            break;
          }
          {
            uint innerCol = 0u;
            while(true) {
              if ((innerCol < ColPerThreadA)) {
              } else {
                break;
              }
              uint inputRow = (tileRow + innerRow);
              uint inputCol = (tileColA + innerCol);
              mm_Asub[min(inputRow, 63u)][min(inputCol, 63u)] = mm_readA((globalRow + innerRow), ((t * 64u) + inputCol));
              {
                innerCol = (innerCol + 1u);
              }
              continue;
            }
          }
          {
            innerRow = (innerRow + 1u);
          }
          continue;
        }
      }
      {
        uint innerRow = 0u;
        while(true) {
          if ((innerRow < RowPerThreadB)) {
          } else {
            break;
          }
          {
            uint innerCol = 0u;
            while(true) {
              if ((innerCol < 4u)) {
              } else {
                break;
              }
              uint inputRow = (tileRowB + innerRow);
              uint inputCol = (tileCol + innerCol);
              uint v_12 = min(innerCol, 63u);
              mm_Bsub[v_12][min(inputCol, 63u)] = mm_readB(((t * 64u) + inputRow), (globalCol + innerCol));
              {
                innerCol = (innerCol + 1u);
              }
              continue;
            }
          }
          {
            innerRow = (innerRow + 1u);
          }
          continue;
        }
      }
      barrier();
      {
        uint k = 0u;
        while(true) {
          if ((k < 64u)) {
          } else {
            break;
          }
          {
            uint inner = 0u;
            while(true) {
              if ((inner < 4u)) {
              } else {
                break;
              }
              uint v_13 = min(inner, 3u);
              uint v_14 = min(k, 63u);
              uint v_15 = min((tileCol + inner), 63u);
              BCached[v_13] = mm_Bsub[v_14][v_15];
              {
                inner = (inner + 1u);
              }
              continue;
            }
          }
          {
            uint innerRow = 0u;
            while(true) {
              if ((innerRow < 4u)) {
              } else {
                break;
              }
              uint v_16 = min((tileRow + innerRow), 63u);
              uint v_17 = min(k, 63u);
              ACached = mm_Asub[v_16][v_17];
              {
                uint innerCol = 0u;
                while(true) {
                  if ((innerCol < 4u)) {
                  } else {
                    break;
                  }
                  uint index = ((innerRow * 4u) + innerCol);
                  float v_18 = acc[min(index, 15u)];
                  float v_19 = ACached;
                  uint v_20 = min(innerCol, 3u);
                  acc[min(index, 15u)] = (v_18 + (v_19 * BCached[v_20]));
                  {
                    innerCol = (innerCol + 1u);
                  }
                  continue;
                }
              }
              {
                innerRow = (innerRow + 1u);
              }
              continue;
            }
          }
          {
            k = (k + 1u);
          }
          continue;
        }
      }
      barrier();
      {
        t = (t + 1u);
      }
      continue;
    }
  }
  {
    uint innerRow = 0u;
    while(true) {
      if ((innerRow < 4u)) {
      } else {
        break;
      }
      {
        uint innerCol = 0u;
        while(true) {
          if ((innerCol < 4u)) {
          } else {
            break;
          }
          uint index = ((innerRow * 4u) + innerCol);
          mm_write((globalRow + innerRow), (globalCol + innerCol), acc[min(index, 15u)]);
          {
            innerCol = (innerCol + 1u);
          }
          continue;
        }
      }
      {
        innerRow = (innerRow + 1u);
      }
      continue;
    }
  }
}
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
void main() {
  main_inner(gl_LocalInvocationID, gl_GlobalInvocationID, gl_LocalInvocationIndex);
}
