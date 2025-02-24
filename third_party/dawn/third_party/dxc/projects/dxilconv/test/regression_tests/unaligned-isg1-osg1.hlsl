// RUN: %dxbc2dxil %t.bin /o %t.dxo.converted
// RUN: %dxa --listparts %t.dxo.converted | %FileCheck %s

// This file has no HLSL source, it is a binary shader result from 9on12.
// There should be a corresponding .bin file checked in next to this one.

// Original DXBC has unaligned sizes for ISG1 and OSG1:
// Part count: 3
// #0 - ISG1 (49 bytes)
// #1 - OSG1 (93 bytes)
// #2 - SHDR (444 bytes)

// Make sure converted DXIL pads the parts for alignment:
// CHECK: ISG1 (52 bytes)
// CHECK: OSG1 (96 bytes)
