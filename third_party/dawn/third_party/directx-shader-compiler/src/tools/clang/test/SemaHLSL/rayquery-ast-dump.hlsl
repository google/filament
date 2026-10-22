// RUN: %dxc -T vs_6_9 -E main -ast-dump %s | FileCheck %s

RaytracingAccelerationStructure RTAS;


float main(RayDesc rayDesc : RAYDESC) : OUT {
  RayQuery<0, RAYQUERY_FLAG_NONE> rayQuery1;
  RayQuery<RAY_FLAG_FORCE_OMM_2_STATE, RAYQUERY_FLAG_ALLOW_OPACITY_MICROMAPS> rayQuery2;
  rayQuery1.TraceRayInline(RTAS, 1, 2, rayDesc);
  rayQuery2.TraceRayInline(RTAS, RAY_FLAG_FORCE_OPAQUE|RAY_FLAG_FORCE_OMM_2_STATE, 2, rayDesc);
  return 0;
}

// CHECK: -DeclStmt 0x{{.+}}
// CHECK-NEXT: `-VarDecl 0x{{.+}} used rayQuery1 'RayQuery<0, RAYQUERY_FLAG_NONE>':'RayQuery<0, 0>' callinit
// CHECK-NEXT:  `-CXXConstructExpr 0x{{.+}} 'RayQuery<0, RAYQUERY_FLAG_NONE>':'RayQuery<0, 0>' 'void ()'
// CHECK-NEXT: -DeclStmt 0x{{.+}} 
// CHECK-NEXT: `-VarDecl 0x{{.+}} used rayQuery2 'RayQuery<RAY_FLAG_FORCE_OMM_2_STATE, RAYQUERY_FLAG_ALLOW_OPACITY_MICROMAPS>':'RayQuery<1024, 1>' callinit
// CHECK-NEXT:  `-CXXConstructExpr 0x{{.+}} 'RayQuery<RAY_FLAG_FORCE_OMM_2_STATE, RAYQUERY_FLAG_ALLOW_OPACITY_MICROMAPS>':'RayQuery<1024, 1>' 'void ()'
// CHECK-NEXT: -CXXMemberCallExpr 0x{{.+}} 'void'
// CHECK-NEXT: -MemberExpr 0x{{.+}} '<bound member function type>' .TraceRayInline
// CHECK-NEXT:  `-DeclRefExpr 0x{{.+}} 'RayQuery<0, RAYQUERY_FLAG_NONE>':'RayQuery<0, 0>' lvalue Var 0x{{.+}} 'rayQuery1' 'RayQuery<0, RAYQUERY_FLAG_NONE>':'RayQuery<0, 0>'

// CHECK: -CXXMemberCallExpr 0x{{.+}} 'void'
// CHECK-NEXT: -MemberExpr 0x{{.+}} '<bound member function type>' .TraceRayInline
// CHECK-NEXT: `-DeclRefExpr 0x{{.+}} 'RayQuery<RAY_FLAG_FORCE_OMM_2_STATE, RAYQUERY_FLAG_ALLOW_OPACITY_MICROMAPS>':'RayQuery<1024, 1>' lvalue Var 0x{{.+}} 'rayQuery2' 'RayQuery<RAY_FLAG_FORCE_OMM_2_STATE, RAYQUERY_FLAG_ALLOW_OPACITY_MICROMAPS>':'RayQuery<1024, 1>'
