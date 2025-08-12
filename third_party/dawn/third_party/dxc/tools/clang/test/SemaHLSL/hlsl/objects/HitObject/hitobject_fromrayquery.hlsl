// RUN: %dxc -T lib_6_9 -E main %s -fcgl | FileCheck %s --check-prefix FCGL
// RUN: %dxc -T lib_6_9 -E main %s -ast-dump-implicit | FileCheck %s --check-prefix AST

// AST: | | |-FunctionTemplateDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> FromRayQuery
// AST-NEXT: | | | |-TemplateTypeParmDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> class TResult
// AST-NEXT: | | | |-TemplateTypeParmDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> class Trq
// AST-NEXT: | | | |-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> implicit FromRayQuery 'TResult (Trq) const' static
// AST-NEXT: | | | | `-ParmVarDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> rq 'Trq'
// AST-NEXT: | | | `-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> used FromRayQuery 'dx::HitObject (RayQuery<RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH>)' static
// AST-NEXT: | | |   |-TemplateArgument type 'dx::HitObject'
// AST-NEXT: | | |   |-TemplateArgument type 'RayQuery<RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH>':'RayQuery<5, 0>'
// AST-NEXT: | | |   |-ParmVarDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> rq 'RayQuery<RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH>':'RayQuery<5, 0>'
// AST-NEXT: | | |   |-HLSLIntrinsicAttr {{[^ ]+}} <<invalid sloc>> Implicit "op" "" 363
// AST-NEXT: | | |   `-AvailabilityAttr {{[^ ]+}} <<invalid sloc>> Implicit  6.9 0 0 ""

// AST: | | |-FunctionTemplateDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> FromRayQuery
// AST-NEXT: | | | |-TemplateTypeParmDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> class TResult
// AST-NEXT: | | | |-TemplateTypeParmDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> class Trq
// AST-NEXT: | | | |-TemplateTypeParmDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> class THitKind
// AST-NEXT: | | | |-TemplateTypeParmDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> class TAttributes
// AST-NEXT: | | | |-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> implicit FromRayQuery 'TResult (Trq, THitKind, TAttributes) const' static
// AST-NEXT: | | | | |-ParmVarDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> rq 'Trq'
// AST-NEXT: | | | | |-ParmVarDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> HitKind 'THitKind'
// AST-NEXT: | | | | `-ParmVarDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> Attributes 'TAttributes'
// AST-NEXT: | | | `-CXXMethodDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> used FromRayQuery 'dx::HitObject (RayQuery<RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH>, unsigned int, CustomAttrs)' static
// AST-NEXT: | | |   |-TemplateArgument type 'dx::HitObject'
// AST-NEXT: | | |   |-TemplateArgument type 'RayQuery<RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH>':'RayQuery<5, 0>'
// AST-NEXT: | | |   |-TemplateArgument type 'unsigned int'
// AST-NEXT: | | |   |-TemplateArgument type 'CustomAttrs'
// AST-NEXT: | | |   |-ParmVarDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> rq 'RayQuery<RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH>':'RayQuery<5, 0>'
// AST-NEXT: | | |   |-ParmVarDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> HitKind 'unsigned int'
// AST-NEXT: | | |   |-ParmVarDecl {{[^ ]+}} <<invalid sloc>> <invalid sloc> Attributes 'CustomAttrs'
// AST-NEXT: | | |   |-HLSLIntrinsicAttr {{[^ ]+}} <<invalid sloc>> Implicit "op" "" 363
// AST-NEXT: | | |   `-AvailabilityAttr {{[^ ]+}} <<invalid sloc>> Implicit  6.9 0 0 ""

// FCGL: call void @"dx.hl.op..void (i32, %dx.types.HitObject*, %\22class.RayQuery<5, 0>\22*)"(i32 363, %dx.types.HitObject* %[[HITPTR0:[^ ]+]], %"class.RayQuery<5, 0>"* %[[RQ:[^ ]+]])
// FCGL-NEXT: call void @"\01?Use@@YAXVHitObject@dx@@@Z"(%dx.types.HitObject* %[[HITPTR0]])
// FCGL: call void @"dx.hl.op..void (i32, %dx.types.HitObject*, %\22class.RayQuery<5, 0>\22*, i32, %struct.CustomAttrs*)"(i32 363, %dx.types.HitObject* %[[HITPTR1:[^ ]+]], %"class.RayQuery<5, 0>"* %[[RQ]], i32 16, %struct.CustomAttrs* %{{[^ ]+}})
// FCGL-NEXT: call void @"\01?Use@@YAXVHitObject@dx@@@Z"(%dx.types.HitObject* %[[HITPTR1]])

RaytracingAccelerationStructure RTAS;
RWStructuredBuffer<float> UAV : register(u0);

RayDesc MakeRayDesc() {
  RayDesc desc;
  desc.Origin = float3(0, 0, 0);
  desc.Direction = float3(1, 0, 0);
  desc.TMin = 0.0f;
  desc.TMax = 9999.0;
  return desc;
}

struct CustomAttrs {
  float x;
  float y;
};

void Use(in dx::HitObject hit) {
  dx::MaybeReorderThread(hit);
}

[shader("raygeneration")]
void main() {
  RayQuery<RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> q;
  RayDesc ray = MakeRayDesc();
  q.TraceRayInline(RTAS, RAY_FLAG_NONE, 0xFF, ray);

  Use(dx::HitObject::FromRayQuery(q));

  CustomAttrs attrs = {1.f, 2.f};
  Use(dx::HitObject::FromRayQuery(q, 16, attrs));
}
