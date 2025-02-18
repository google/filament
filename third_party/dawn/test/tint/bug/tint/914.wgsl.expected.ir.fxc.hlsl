struct main_inputs {
  uint3 local_id : SV_GroupThreadID;
  uint tint_local_index : SV_GroupIndex;
  uint3 global_id : SV_DispatchThreadID;
};


ByteAddressBuffer firstMatrix : register(t0);
ByteAddressBuffer secondMatrix : register(t1);
RWByteAddressBuffer resultMatrix : register(u2);
cbuffer cbuffer_uniforms : register(b3) {
  uint4 uniforms[1];
};
groupshared float mm_Asub[64][64];
groupshared float mm_Bsub[64][64];
float mm_readA(uint row, uint col) {
  bool v = false;
  if ((row < uniforms[0u].x)) {
    v = (col < uniforms[0u].y);
  } else {
    v = false;
  }
  if (v) {
    uint v_1 = 0u;
    firstMatrix.GetDimensions(v_1);
    float result = asfloat(firstMatrix.Load((0u + (min(((row * uniforms[0u].y) + col), ((v_1 / 4u) - 1u)) * 4u))));
    return result;
  }
  return 0.0f;
}

float mm_readB(uint row, uint col) {
  bool v_2 = false;
  if ((row < uniforms[0u].y)) {
    v_2 = (col < uniforms[0u].z);
  } else {
    v_2 = false;
  }
  if (v_2) {
    uint v_3 = 0u;
    secondMatrix.GetDimensions(v_3);
    float result = asfloat(secondMatrix.Load((0u + (min(((row * uniforms[0u].z) + col), ((v_3 / 4u) - 1u)) * 4u))));
    return result;
  }
  return 0.0f;
}

void mm_write(uint row, uint col, float value) {
  bool v_4 = false;
  if ((row < uniforms[0u].x)) {
    v_4 = (col < uniforms[0u].z);
  } else {
    v_4 = false;
  }
  if (v_4) {
    uint index = (col + (row * uniforms[0u].z));
    uint v_5 = 0u;
    resultMatrix.GetDimensions(v_5);
    resultMatrix.Store((0u + (min(index, ((v_5 / 4u) - 1u)) * 4u)), asuint(value));
  }
}

uint tint_div_u32(uint lhs, uint rhs) {
  return (lhs / (((rhs == 0u)) ? (1u) : (rhs)));
}

void main_inner(uint3 local_id, uint3 global_id, uint tint_local_index) {
  {
    uint v_6 = 0u;
    v_6 = tint_local_index;
    while(true) {
      uint v_7 = v_6;
      if ((v_7 >= 4096u)) {
        break;
      }
      mm_Asub[(v_7 / 64u)][(v_7 % 64u)] = 0.0f;
      mm_Bsub[(v_7 / 64u)][(v_7 % 64u)] = 0.0f;
      {
        v_6 = (v_7 + 256u);
      }
      continue;
    }
  }
  GroupMemoryBarrierWithGroupSync();
  uint tileRow = (local_id.y * 4u);
  uint tileCol = (local_id.x * 4u);
  uint globalRow = (global_id.y * 4u);
  uint globalCol = (global_id.x * 4u);
  uint numTiles = (tint_div_u32((uniforms[0u].y - 1u), 64u) + 1u);
  float acc[16] = (float[16])0;
  float ACached = 0.0f;
  float BCached[4] = (float[4])0;
  {
    uint index = 0u;
    while(true) {
      if ((index < 16u)) {
      } else {
        break;
      }
      uint v_8 = min(index, 15u);
      acc[v_8] = 0.0f;
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
              uint v_9 = min(innerCol, 63u);
              mm_Bsub[v_9][min(inputCol, 63u)] = mm_readB(((t * 64u) + inputRow), (globalCol + innerCol));
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
      GroupMemoryBarrierWithGroupSync();
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
              uint v_10 = min(inner, 3u);
              uint v_11 = min(k, 63u);
              uint v_12 = min((tileCol + inner), 63u);
              BCached[v_10] = mm_Bsub[v_11][v_12];
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
              uint v_13 = min((tileRow + innerRow), 63u);
              uint v_14 = min(k, 63u);
              ACached = mm_Asub[v_13][v_14];
              {
                uint innerCol = 0u;
                while(true) {
                  if ((innerCol < 4u)) {
                  } else {
                    break;
                  }
                  uint index = ((innerRow * 4u) + innerCol);
                  float v_15 = acc[min(index, 15u)];
                  float v_16 = ACached;
                  uint v_17 = min(innerCol, 3u);
                  acc[min(index, 15u)] = (v_15 + (v_16 * BCached[v_17]));
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
      GroupMemoryBarrierWithGroupSync();
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

[numthreads(16, 16, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.local_id, inputs.global_id, inputs.tint_local_index);
}

