// Validation template including valid operations and invalid destinations
// that the validator test will swap in to ensure that the validator produces
// appropriate errors.

// Valid resources for atomics to create valid output that will later be manipulated to test the validator
RWStructuredBuffer<uint> rw_structbuf;
RWBuffer<uint> rw_buf;
RWTexture1D<uint> rw_tex;

// SRVs to plug into atomics
StructuredBuffer<uint> ro_structbuf;
Buffer<uint> ro_buf;
Texture1D<uint> ro_tex;

const groupshared uint cgs_var = 0;
const groupshared uint cgs_arr[3] = {0, 0, 0};

groupshared uint gs_var;

RWStructuredBuffer<uint> output; // just something to keep the variables alive

cbuffer CB {
  uint cb_var;
}
uint cb_gvar;

#if __SHADER_TARGET_STAGE == __SHADER_STAGE_LIBRARY
void init(out uint i); // To force an alloca pointer to use with atomic op
#else
void init(out uint i) {i = 0;}
#endif

[shader("compute")]
[numthreads(1,1,1)]
void main(uint ix : SV_GroupIndex) {

  uint res;
  init(res);

  // Token usages of the invalid resources and variables so they are available in the output
  res += cb_var + cb_gvar + cgs_var + ro_structbuf[ix] + ro_buf[ix] + ro_tex[ix] + cgs_arr[ix];

  InterlockedAdd(rw_structbuf[ix], 1);
  InterlockedCompareStore(rw_structbuf[ix], 1, 2);

  InterlockedAdd(rw_buf[ix], 1);
  InterlockedCompareStore(rw_buf[ix], 1, 2);

  InterlockedAdd(rw_tex[ix], 1);
  InterlockedCompareStore(rw_tex[ix], 1, 2);

  InterlockedAdd(gs_var, 1);
  InterlockedCompareStore(gs_var, 1, 2);

  output[ix] = res;
}
