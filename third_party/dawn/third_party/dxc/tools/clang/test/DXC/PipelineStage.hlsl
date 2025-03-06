// RUN: %dxc -T vs_6_5 -P -Fi %t.vs_65.hlsl.pp %s
// RUN: FileCheck --input-file=%t.vs_65.hlsl.pp %s --check-prefix=VS_65
// VS_65:vs 6 5

// RUN: %dxc -T vs_6_6 -P -Fi %t.vs_66.hlsl.pp %s
// RUN: FileCheck --input-file=%t.vs_66.hlsl.pp %s --check-prefix=VS_66
// VS_66:vs 6 6

// RUN: %dxc -T ps_6_5 -P -Fi %t.ps_65.hlsl.pp %s
// RUN: FileCheck --input-file=%t.ps_65.hlsl.pp %s --check-prefix=PS_65
// PS_65:ps 6 5

// RUN: %dxc -T ps_6_6 -P -Fi %t.ps_66.hlsl.pp %s
// RUN: FileCheck --input-file=%t.ps_66.hlsl.pp %s --check-prefix=PS_66
// PS_66:ps 6 6

// RUN: %dxc -T gs_6_5 -P -Fi %t.gs_65.hlsl.pp %s
// RUN: FileCheck --input-file=%t.gs_65.hlsl.pp %s --check-prefix=GS_65
// GS_65:gs 6 5

// RUN: %dxc -T gs_6_6 -P -Fi %t.gs_66.hlsl.pp %s
// RUN: FileCheck --input-file=%t.gs_66.hlsl.pp %s --check-prefix=GS_66
// GS_66:gs 6 6

// RUN: %dxc -T hs_6_5 -P -Fi %t.hs_65.hlsl.pp %s
// RUN: FileCheck --input-file=%t.hs_65.hlsl.pp %s --check-prefix=HS_65
// HS_65:hs 6 5

// RUN: %dxc -T hs_6_6 -P -Fi %t.hs_66.hlsl.pp %s
// RUN: FileCheck --input-file=%t.hs_66.hlsl.pp %s --check-prefix=HS_66
// HS_66:hs 6 6

// RUN: %dxc -T ds_6_5 -P -Fi %t.ds_65.hlsl.pp %s
// RUN: FileCheck --input-file=%t.ds_65.hlsl.pp %s --check-prefix=DS_65
// DS_65:ds 6 5

// RUN: %dxc -T ds_6_6 -P -Fi %t.ds_66.hlsl.pp %s
// RUN: FileCheck --input-file=%t.ds_66.hlsl.pp %s --check-prefix=DS_66
// DS_66:ds 6 6

// RUN: %dxc -T cs_6_5 -P -Fi %t.cs_65.hlsl.pp %s
// RUN: FileCheck --input-file=%t.cs_65.hlsl.pp %s --check-prefix=CS_65
// CS_65:cs 6 5

// RUN: %dxc -T cs_6_6 -P -Fi %t.cs_66.hlsl.pp %s
// RUN: FileCheck --input-file=%t.cs_66.hlsl.pp %s --check-prefix=CS_66
// CS_66:cs 6 6

// RUN: %dxc -T lib_6_5 -P -Fi %t.lib_65.hlsl.pp %s
// RUN: FileCheck --input-file=%t.lib_65.hlsl.pp %s --check-prefix=LIB_65
// LIB_65:lib 6 5

// RUN: %dxc -T lib_6_6 -P -Fi %t.lib_66.hlsl.pp %s
// RUN: FileCheck --input-file=%t.lib_66.hlsl.pp %s --check-prefix=LIB_66
// LIB_66:lib 6 6

// RUN: %dxc -T ms_6_5 -P -Fi %t.ms_65.hlsl.pp %s
// RUN: FileCheck --input-file=%t.ms_65.hlsl.pp %s --check-prefix=MS_65
// MS_65:ms 6 5

// RUN: %dxc -T ms_6_6 -P -Fi %t.ms_66.hlsl.pp %s
// RUN: FileCheck --input-file=%t.ms_66.hlsl.pp %s --check-prefix=MS_66
// MS_66:ms 6 6

// RUN: %dxc -T as_6_5 -P -Fi %t.as_65.hlsl.pp %s
// RUN: FileCheck --input-file=%t.as_65.hlsl.pp %s --check-prefix=AS_65
// AS_65:as 6 5

// RUN: %dxc -T as_6_6 -P -Fi %t.as_66.hlsl.pp %s
// RUN: FileCheck --input-file=%t.as_66.hlsl.pp %s --check-prefix=AS_66
// AS_66:as 6 6


// VS PS GS HS DS CS LIB MS AS

#if __SHADER_TARGET_STAGE == __SHADER_STAGE_VERTEX
#define __SHADER_STAGE_PREFIX vs
#elif __SHADER_TARGET_STAGE == __SHADER_STAGE_PIXEL
#define __SHADER_STAGE_PREFIX ps
#elif __SHADER_TARGET_STAGE == __SHADER_STAGE_GEOMETRY
#define __SHADER_STAGE_PREFIX gs
#elif __SHADER_TARGET_STAGE == __SHADER_STAGE_HULL
#define __SHADER_STAGE_PREFIX hs
#elif __SHADER_TARGET_STAGE == __SHADER_STAGE_DOMAIN
#define __SHADER_STAGE_PREFIX ds
#elif __SHADER_TARGET_STAGE == __SHADER_STAGE_COMPUTE
#define __SHADER_STAGE_PREFIX cs
#elif __SHADER_TARGET_STAGE == __SHADER_STAGE_AMPLIFICATION
#define __SHADER_STAGE_PREFIX as
#elif __SHADER_TARGET_STAGE == __SHADER_STAGE_MESH
#define __SHADER_STAGE_PREFIX ms
#elif __SHADER_TARGET_STAGE == __SHADER_STAGE_LIBRARY
#define __SHADER_STAGE_PREFIX lib
#endif

__SHADER_STAGE_PREFIX __SHADER_TARGET_MAJOR __SHADER_TARGET_MINOR
