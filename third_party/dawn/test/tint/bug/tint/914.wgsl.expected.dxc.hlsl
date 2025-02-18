groupshared float mm_Asub[64][64];
groupshared float mm_Bsub[64][64];

void tint_zero_workgroup_memory(uint local_idx) {
  {
    for(uint idx = local_idx; (idx < 4096u); idx = (idx + 256u)) {
      uint i = (idx / 64u);
      uint i_1 = (idx % 64u);
      mm_Asub[i][i_1] = 0.0f;
      mm_Bsub[i][i_1] = 0.0f;
    }
  }
  GroupMemoryBarrierWithGroupSync();
}

ByteAddressBuffer firstMatrix : register(t0);
ByteAddressBuffer secondMatrix : register(t1);
RWByteAddressBuffer resultMatrix : register(u2);
cbuffer cbuffer_uniforms : register(b3) {
  uint4 uniforms[1];
};

float mm_readA(uint row, uint col) {
  bool tint_tmp = (row < uniforms[0].x);
  if (tint_tmp) {
    tint_tmp = (col < uniforms[0].y);
  }
  if ((tint_tmp)) {
    uint tint_symbol_7 = 0u;
    firstMatrix.GetDimensions(tint_symbol_7);
    uint tint_symbol_8 = ((tint_symbol_7 - 0u) / 4u);
    float result = asfloat(firstMatrix.Load((4u * min(((row * uniforms[0].y) + col), (tint_symbol_8 - 1u)))));
    return result;
  }
  return 0.0f;
}

float mm_readB(uint row, uint col) {
  bool tint_tmp_1 = (row < uniforms[0].y);
  if (tint_tmp_1) {
    tint_tmp_1 = (col < uniforms[0].z);
  }
  if ((tint_tmp_1)) {
    uint tint_symbol_9 = 0u;
    secondMatrix.GetDimensions(tint_symbol_9);
    uint tint_symbol_10 = ((tint_symbol_9 - 0u) / 4u);
    float result = asfloat(secondMatrix.Load((4u * min(((row * uniforms[0].z) + col), (tint_symbol_10 - 1u)))));
    return result;
  }
  return 0.0f;
}

void mm_write(uint row, uint col, float value) {
  bool tint_tmp_2 = (row < uniforms[0].x);
  if (tint_tmp_2) {
    tint_tmp_2 = (col < uniforms[0].z);
  }
  if ((tint_tmp_2)) {
    uint tint_symbol_12 = 0u;
    resultMatrix.GetDimensions(tint_symbol_12);
    uint tint_symbol_13 = ((tint_symbol_12 - 0u) / 4u);
    uint index = (col + (row * uniforms[0].z));
    resultMatrix.Store((4u * min(index, (tint_symbol_13 - 1u))), asuint(value));
  }
}

uint tint_div(uint lhs, uint rhs) {
  return (lhs / ((rhs == 0u) ? 1u : rhs));
}

struct tint_symbol_5 {
  uint3 local_id : SV_GroupThreadID;
  uint local_invocation_index : SV_GroupIndex;
  uint3 global_id : SV_DispatchThreadID;
};

void main_inner(uint3 local_id, uint3 global_id, uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  uint tileRow = (local_id.y * 4u);
  uint tileCol = (local_id.x * 4u);
  uint globalRow = (global_id.y * 4u);
  uint globalCol = (global_id.x * 4u);
  uint numTiles = (tint_div((uniforms[0].y - 1u), 64u) + 1u);
  float acc[16] = (float[16])0;
  float ACached = 0.0f;
  float BCached[4] = (float[4])0;
  {
    for(uint index = 0u; (index < 16u); index = (index + 1u)) {
      acc[min(index, 15u)] = 0.0f;
    }
  }
  uint ColPerThreadA = 4u;
  uint tileColA = (local_id.x * ColPerThreadA);
  uint RowPerThreadB = 4u;
  uint tileRowB = (local_id.y * RowPerThreadB);
  {
    for(uint t = 0u; (t < numTiles); t = (t + 1u)) {
      {
        for(uint innerRow = 0u; (innerRow < 4u); innerRow = (innerRow + 1u)) {
          {
            for(uint innerCol = 0u; (innerCol < ColPerThreadA); innerCol = (innerCol + 1u)) {
              uint inputRow = (tileRow + innerRow);
              uint inputCol = (tileColA + innerCol);
              uint tint_symbol = inputRow;
              uint tint_symbol_1 = inputCol;
              mm_Asub[min(tint_symbol, 63u)][min(tint_symbol_1, 63u)] = mm_readA((globalRow + innerRow), ((t * 64u) + inputCol));
            }
          }
        }
      }
      {
        for(uint innerRow = 0u; (innerRow < RowPerThreadB); innerRow = (innerRow + 1u)) {
          {
            for(uint innerCol = 0u; (innerCol < 4u); innerCol = (innerCol + 1u)) {
              uint inputRow = (tileRowB + innerRow);
              uint inputCol = (tileCol + innerCol);
              uint tint_symbol_2 = innerCol;
              uint tint_symbol_3 = inputCol;
              mm_Bsub[min(tint_symbol_2, 63u)][min(tint_symbol_3, 63u)] = mm_readB(((t * 64u) + inputRow), (globalCol + innerCol));
            }
          }
        }
      }
      GroupMemoryBarrierWithGroupSync();
      {
        for(uint k = 0u; (k < 64u); k = (k + 1u)) {
          {
            for(uint inner = 0u; (inner < 4u); inner = (inner + 1u)) {
              BCached[min(inner, 3u)] = mm_Bsub[min(k, 63u)][min((tileCol + inner), 63u)];
            }
          }
          {
            for(uint innerRow = 0u; (innerRow < 4u); innerRow = (innerRow + 1u)) {
              ACached = mm_Asub[min((tileRow + innerRow), 63u)][min(k, 63u)];
              {
                for(uint innerCol = 0u; (innerCol < 4u); innerCol = (innerCol + 1u)) {
                  uint index = ((innerRow * 4u) + innerCol);
                  acc[min(index, 15u)] = (acc[min(index, 15u)] + (ACached * BCached[min(innerCol, 3u)]));
                }
              }
            }
          }
        }
      }
      GroupMemoryBarrierWithGroupSync();
    }
  }
  {
    for(uint innerRow = 0u; (innerRow < 4u); innerRow = (innerRow + 1u)) {
      {
        for(uint innerCol = 0u; (innerCol < 4u); innerCol = (innerCol + 1u)) {
          uint index = ((innerRow * 4u) + innerCol);
          mm_write((globalRow + innerRow), (globalCol + innerCol), acc[min(index, 15u)]);
        }
      }
    }
  }
}

[numthreads(16, 16, 1)]
void main(tint_symbol_5 tint_symbol_4) {
  main_inner(tint_symbol_4.local_id, tint_symbol_4.global_id, tint_symbol_4.local_invocation_index);
  return;
}
