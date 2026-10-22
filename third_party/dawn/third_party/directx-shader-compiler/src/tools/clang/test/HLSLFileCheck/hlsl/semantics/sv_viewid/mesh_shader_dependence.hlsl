// RUN: %dxc -E main -T ms_6_5 %s | FileCheck %s

// RUN: %dxc -E main -T ms_6_5 -DVIEWID %s | FileCheck %s -check-prefix=VIEWID

// RUN: %dxc -E main -T ms_6_5 -DVIEWID -DVIEWID_A %s | FileCheck %s -check-prefix=VIEWID_A

// RUN: %dxc -E main -T ms_6_5 -DVIEWID -DVIEWID_B  %s | FileCheck %s -check-prefix=VIEWID_B

// RUN: %dxc -E main -T ms_6_5 -DVIEWID -DVIEWID_C  %s | FileCheck %s -check-prefix=VIEWID_C

// RUN: %dxc -E main -T ms_6_5 -DVIEWID -DVIEWID_D  %s | FileCheck %s -check-prefix=VIEWID_D


// CHECK:; Number of inputs: 0, outputs: 17, primitive outputs: 25
// CHECK:; Outputs dependent on ViewId: {  }
// CHECK:; Primitive Outputs dependent on ViewId: {  }
// CHECK:; Inputs contributing to computation of Outputs:
// CHECK:; Inputs contributing to computation of Primitive Outputs:

// VIEWID:; Number of inputs: 0, outputs: 17, primitive outputs: 25
// VIEWID:; Outputs dependent on ViewId: {  }
// VIEWID:; Primitive Outputs dependent on ViewId: {  }
// VIEWID:; Inputs contributing to computation of Outputs:
// VIEWID:; Inputs contributing to computation of Primitive Outputs:

// VIEWID_A:; Number of inputs: 0, outputs: 17, primitive outputs: 25
// VIEWID_A:; Outputs dependent on ViewId: { 12 }
// VIEWID_A:; Primitive Outputs dependent on ViewId: {  }
// VIEWID_A:; Inputs contributing to computation of Outputs:
// VIEWID_A:; Inputs contributing to computation of Primitive Outputs:


// VIEWID_B:; Number of inputs: 0, outputs: 17, primitive outputs: 25
// VIEWID_B:; Outputs dependent on ViewId: {  }
// VIEWID_B:; Primitive Outputs dependent on ViewId: { 0, 1, 2, 3, 4, 8, 12, 16, 20, 24 }
// VIEWID_B:; Inputs contributing to computation of Outputs:
// VIEWID_B:; Inputs contributing to computation of Primitive Outputs:


// VIEWID_C:; Number of inputs: 0, outputs: 17, primitive outputs: 25
// VIEWID_C:; Outputs dependent on ViewId: { 8 }
// VIEWID_C:; Primitive Outputs dependent on ViewId: {  }
// VIEWID_C:; Inputs contributing to computation of Outputs:
// VIEWID_C:; Inputs contributing to computation of Primitive Outputs:


// VIEWID_D:; Number of inputs: 0, outputs: 17, primitive outputs: 25
// VIEWID_D:; Outputs dependent on ViewId: {  }
// VIEWID_D:; Primitive Outputs dependent on ViewId: { 24 }
// VIEWID_D:; Inputs contributing to computation of Outputs:
// VIEWID_D:; Inputs contributing to computation of Primitive Outputs:

#define MAX_VERT 32
#define MAX_PRIM 16
#define NUM_THREADS 32
struct MeshPerVertex {
    float4 position : SV_Position;
    float color[4] : COLOR;
};

struct MeshPerPrimitive {
    float normal : NORMAL;
    float malnor : MALNOR;
    float alnorm : ALNORM;
    float ormaln : ORMALN;
    int layer[6] : LAYER;
    bool cullPrimitive : SV_CullPrimitive;
};

struct MeshPayload {
    float normal;
    float malnor;
    float alnorm;
    float ormaln;
    int layer[6];
};

groupshared float gsMem[MAX_PRIM];

[numthreads(NUM_THREADS, 1, 1)]
[outputtopology("triangle")]
void main(
            out indices uint3 primIndices[MAX_PRIM],
            out vertices MeshPerVertex verts[MAX_VERT],
            out primitives MeshPerPrimitive prims[MAX_PRIM],
            in payload MeshPayload mpl,
            in uint tig : SV_GroupIndex
#ifdef VIEWID
			,in uint vid : SV_ViewID
#endif
         )
{
    SetMeshOutputCounts(MAX_VERT, MAX_PRIM);
    MeshPerVertex ov;
    if (tig % 2) {
        ov.position = float4(4.0,5.0,6.0,7.0);
        ov.color[0] = 4.0;
        ov.color[1] = 5.0;
#ifdef VIEWID_A
		ov.color[2] = vid;
#else
        ov.color[2] = 6.0;
#endif
        ov.color[3] = 7.0;
    } else {
        ov.position = float4(14.0,15.0,16.0,17.0);
        ov.color[0] = 14.0;
#ifdef VIEWID_C
        ov.color[1] = vid;
#else
        ov.color[1] = 15.0;
#endif
        ov.color[2] = 16.0;
        ov.color[3] = 17.0;
    }

#ifdef VIEWID_B
    if (vid % 3) {
#else
	if (tig % 3) {
#endif
      primIndices[tig / 3] = uint3(tig, tig + 1, tig + 2);
      MeshPerPrimitive op;
      op.normal = mpl.normal;
      op.malnor = gsMem[tig / 3 + 1];
      op.alnorm = mpl.alnorm;
      op.ormaln = mpl.ormaln;
      op.layer[0] = mpl.layer[0];
      op.layer[1] = mpl.layer[1];
      op.layer[2] = mpl.layer[2];
      op.layer[3] = mpl.layer[3];
      op.layer[4] = mpl.layer[4];
#ifdef VIEWID_D
      op.layer[5] = mpl.layer[5] + vid;
#else
      op.layer[5] = mpl.layer[5];
#endif
      op.cullPrimitive = false;
      gsMem[tig / 3] = op.normal;
      prims[tig / 3] = op;
    }
    verts[tig] = ov;
}
