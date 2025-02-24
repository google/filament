/*// RUN: %testasm %s /Fo %t.dxbc*/
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted

hs_5_0
hs_decls
dcl_input_control_point_count   4
dcl_output_control_point_count  32
dcl_tessellator_domain             domain_quad
dcl_tessellator_partitioning       partitioning_fractional_odd
dcl_tessellator_output_primitive   output_triangle_cw
dcl_hs_max_tessfactor              64.f
hs_control_point_phase
dcl_input v[4][0].xyzw
dcl_input v[4][1].xy
dcl_input v[4][2].xyz
dcl_input vOutputControlPointID
dcl_input vPrim
dcl_output o0.xyzw
dcl_output o1.xy
dcl_output o2.xyz
dcl_temps 1
udiv NULL, r0.x, vOutputControlPointID, 4
mov o0.xyzw, v[r0.x][0].xyzw
mov o1.xy,   v[r0.x][1].xyxx
mov o2.xyz,  v[r0.x][2].xyzx
hs_fork_phase
dcl_input vcp[4][0].xyzw
dcl_input vcp[4][1].xy
dcl_input vcp[4][2].xyz
dcl_input vocp[32][0].xyzw
dcl_input vocp[32][1].xy
dcl_input vocp[32][2].xyz
dcl_hs_fork_phase_instance_count 4
dcl_input vForkInstanceID
dcl_input vPrim
dcl_indexRange o[0], o[3]
dcl_temps 1
dcl_indexableTemp x0[4], 1
dcl_output_sv o0.x, finalQuadUeq0EdgeTessFactor
dcl_output_sv o1.x, finalQuadVeq0EdgeTessFactor
dcl_output_sv o2.x, finalQuadUeq1EdgeTessFactor
dcl_output_sv o3.x, finalQuadVeq1EdgeTessFactor
mov x0[0].x, 2.0f
mov x0[1].x, 4.0f
mov x0[2].x, 15.0f
mov x0[3].x, 6.0f
mov r0.x, vForkInstanceID
mov o[r0.x].x, x0[r0.x].x
hs_fork_phase
dcl_input vcp[4][0].xyzw
dcl_input vcp[4][1].xy
dcl_input vcp[4][2].xyz
dcl_input vocp[32][0].xyzw
dcl_input vocp[32][1].xy
dcl_input vocp[32][2].xyz
dcl_hs_fork_phase_instance_count 4
dcl_input vForkInstanceID
dcl_input vPrim
dcl_indexRange o[0], o[3]
dcl_temps 1
dcl_indexableTemp x0[4], 1
dcl_output o0.y
dcl_output o1.y
dcl_output o2.y
dcl_output o3.y
mov x0[0].x, 12.0f
mov x0[1].x, 32.0f
mov x0[2].x, 15.0f
mov x0[3].x, 5.0f
mov r0.x, vForkInstanceID
mov o[r0.x].y, x0[r0.x].x
hs_join_phase
dcl_input vcp[4][0].xyzw
dcl_input vcp[4][1].xy
dcl_input vcp[4][2].xyz
dcl_input vocp[32][0].xyzw
dcl_input vocp[32][1].xy
dcl_input vocp[32][2].xyz
dcl_input vpc[0].xy
dcl_input vpc[1].xy
dcl_input vpc[2].xy
dcl_input vpc[3].xy
dcl_indexRange vpc[0], vpc[3]
dcl_output_sv o4.x, finalQuadUInsideTessFactor
dcl_output_sv o5.x, finalQuadVInsideTessFactor
dcl_output o4.y
dcl_output o5.y
dcl_input vPrim
mov o4.x, 12.0f
mov o5.x, 6.0f
mov o4.y, 0.0f
mov o5.y, 0.0f
