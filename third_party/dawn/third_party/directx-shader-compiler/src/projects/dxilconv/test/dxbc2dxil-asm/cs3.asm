/*// RUN: %testasm %s /Fo %t.dxbc*/
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted

cs_5_0
dcl_globalFlags refactoringAllowed
dcl_constantbuffer cb0[1], immediateIndexed
dcl_input vThreadIDInGroup.xyz
dcl_temps 3
dcl_tgsm_raw g0, 1024
dcl_thread_group 4, 2, 3

ishl r0.x,  vThreadIDInGroup.z, l(2)
store_raw g0.xy, r0.x, cb0[0].wzyx

sync_g
sync_ugroup
sync_uglobal
sync_g_t
sync_ugroup_t
sync_uglobal_t
sync_ugroup_g
sync_uglobal_g
sync_ugroup_g_t
sync_uglobal_g_t

ld_raw r0.xz, r0.x, g0.zxwy

imm_atomic_iadd r2.x, g0, r1.xyxx, vThreadIDInGroup.x
atomic_or g0, r1.xyxx, vThreadIDInGroup.x

atomic_cmp_store g0, r1.xyxx, vThreadIDInGroup.y, vThreadIDInGroup.x
imm_atomic_cmp_exch r1.x, g0, r1.xyxx, vThreadIDInGroup.y, vThreadIDInGroup.x

ret
