// Rewrite unchanged result:
int unroll_noarg() {
  int result = 2;
  [unroll]
  for (int i = 0; i < 100; i++) 
    result++;
  return result;
}


int unroll_zero() {
  int result = 2;
  [unroll]
  for (int i = 0; i < 100; i++) 
    result++;
  return result;
}


int unroll_one() {
  int result = 2;
  [unroll(1)]
  for (int i = 0; i < 100; i++) 
    result++;
  return result;
}


int short_unroll() {
  int result = 2;
  [unroll(2)]
  for (int i = 0; i < 100; i++) 
    result++;
  return result;
}


int long_unroll() {
  int result = 2;
  [unroll(200)]
  for (int i = 0; i < 100; i++) 
    result++;
  return result;
}


RWByteAddressBuffer bab;
const int bab_address;
const bool g_bool;
const uint g_uint;
const uint g_dealiasTableOffset;
const uint g_dealiasTableSize;
int uav() {
  uint i;
  [allow_uav_condition]
  for (i = g_dealiasTableOffset; i < g_dealiasTableSize; ++i) {
  }
  return i;
}


struct HSFoo {
  float3 pos : POSITION;
};
Texture2D<float4> tex1[10] : register(t20, space10);
[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(16)]
[patchconstantfunc("PatchFoo")]
HSFoo HSMain(InputPatch<HSFoo, 16> p, uint i : SV_OutputControlPointID, uint PatchID : SV_PrimitiveID) {
  HSFoo output;
  float4 r = float4(p[PatchID].pos, 1);
  r += tex1[r.x].Load(r.xyz);
  output.pos = p[i].pos + r.xyz;
  return output;
}


const float4 cp4[2];
const int4 i4;
const float4 cp5;
const float4x4 m4;
const float f;
struct global_struct {
  float4 cp5[5];
};
struct main_output {
  float4 p0 : SV_Position0;
};
float4 myexpr() {
  return cp5;
}


static const float4 f4_const = float4(1, 2, 3, 4);
const bool b;
const int clip_index;
static const bool b_true = true;
const global_struct gs;
const float4 f4;
[clipplanes(f4, cp4[0], gs.cp5[2])]
float4 clipplanes_good();
[clipplanes((f4), cp4[(0)], (gs).cp5[2], ((gs).cp5[2]))]
float4 clipplanes_good_parens();
[earlydepthstencil]
float4 main() : SV_Target0 {
  int val = 2;
  val = 2;
  [loop]
  do {
    val *= 2;
  } while (val < 10);
  [fastopt]
  while (val > 10)
  {
    val--;
  }
  [branch]
  if (g_bool) {
    val += 4;
  }
  [flatten]
  if (!g_bool) {
    val += 4;
  }
  [flatten]
  switch (g_uint) {
  case 1:
    val += 101;
    break;
  case 2:
  case 3:
    val += 102;
    break;
    break;
  }
  [branch]
  switch (g_uint) {
  case 1:
    val += 101;
    break;
  case 2:
  case 3:
    val += 102;
    break;
    break;
  }
  [forcecase]
  switch (g_uint) {
  case 1:
    val += 101;
    break;
  case 2:
  case 3:
    val += 102;
    break;
    break;
  }
  [call]
  switch (g_uint) {
  case 1:
    val += 101;
    break;
  case 2:
  case 3:
    val += 102;
    break;
    break;
  }
  val += long_unroll();
  val += short_unroll();
  val += uav();
  return val;
}


[noinline]
bool Test_noinline() {
  return true;
}


