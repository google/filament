// RUN: %dxc -E main -T vs_6_2 %s | FileCheck %s

// Repro of GitHub #2077

groupshared uint gs_uint;
groupshared int gs_int;
groupshared bool gs_bool;
groupshared float gs_float;
groupshared int16_t gs_int16;
groupshared int64_t gs_int64;

void main()
{
  InterlockedAdd(gs_uint, 1);
  InterlockedAdd(gs_int, 1);
  // All the below crash
  InterlockedAdd(gs_bool, 1);
  InterlockedAdd(gs_float, 1);
  InterlockedAdd(gs_int16, 1);
  InterlockedAdd(gs_int64, 1);
}
