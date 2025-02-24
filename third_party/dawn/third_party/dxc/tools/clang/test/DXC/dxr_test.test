// Basic Rewriter Smoke Test
// RUN: %dxr -remove-unused-globals %S/Inputs/smoke.hlsl -Emain | FileCheck %s --check-prefix=DXR

// DXR:int f2(int g)
// DXR-not:g_unused
