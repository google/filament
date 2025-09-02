// RUN: %dxc -T lib_6_9 -E main %s | FileCheck %s --check-prefix DXIL

// DXIL: %[[APTR:[^ ]+]] = alloca %struct.CustomAttrs, align 4
// DXIL: %[[NOP:[^ ]+]] = call %dx.types.HitObject @dx.op.hitObject_MakeNop(i32 266)  ; HitObject_MakeNop()
// DXIL: call void @dx.op.hitObject_Attributes.struct.CustomAttrs(i32 289, %dx.types.HitObject %[[NOP]], %struct.CustomAttrs* nonnull %[[APTR]])  ; HitObject_Attributes(hitObject,attributes)
// DXIL: %[[VPTR:[^ ]+]] = getelementptr inbounds %struct.CustomAttrs, %struct.CustomAttrs* %[[APTR]], i32 0, i32 0
// DXIL: %{{[^ ]+}} = load <4 x float>, <4 x float>* %[[VPTR]], align 4
// DXIL: %[[IPTR:[^ ]+]] = getelementptr inbounds %struct.CustomAttrs, %struct.CustomAttrs* %[[APTR]], i32 0, i32 1
// DXIL: %{{[^ ]+}} = load i32, i32* %[[IPTR]], align 4
// DXIL: ret void

RWByteAddressBuffer outbuf;

struct
CustomAttrs {
  float4 v;
  int y;
};

[shader("raygeneration")]
void main() {
  dx::HitObject hit;
  CustomAttrs attrs;
  hit.GetAttributes(attrs);
  float sum = attrs.v.x + attrs.v.y + attrs.v.z + attrs.v.w + attrs.y;
  outbuf.Store(0, sum);
}
