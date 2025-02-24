// RUN: %dxc  -HV 202x %s -P %t.v202x.hlsl.pp
// RUN: FileCheck --input-file=%t.v202x.hlsl.pp %s --check-prefix=LATEST
// LATEST: 2029

// RUN: %dxc  -HV 2016 %s -P %t.v2016.hlsl.pp
// RUN: FileCheck --input-file=%t.v2016.hlsl.pp %s --check-prefix=HV16
// HV16: 2016

// RUN: %dxc  -HV 2017 %s -P %t.v2017.hlsl.pp
// RUN: FileCheck --input-file=%t.v2017.hlsl.pp %s --check-prefix=HV17
// HV17: 2017

// RUN: %dxc  -HV 2018 %s -P %t.v2018.hlsl.pp
// RUN: FileCheck --input-file=%t.v2018.hlsl.pp %s --check-prefix=HV18
// HV18: 2018

// RUN: %dxc  -HV 2021 %s -P %t.v2021.hlsl.pp
// RUN: FileCheck --input-file=%t.v2021.hlsl.pp %s --check-prefix=HV21
// HV21: 2021

// Verify the default version:
// RUN: %dxc  %s -P %t.default.hlsl.pp
// RUN: FileCheck --input-file=%t.v2021.hlsl.pp %s --check-prefix=Default
// Default: 2021

__HLSL_VERSION
