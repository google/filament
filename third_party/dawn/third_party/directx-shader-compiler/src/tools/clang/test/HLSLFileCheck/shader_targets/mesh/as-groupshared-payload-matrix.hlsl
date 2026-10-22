// RUN: %dxc -E main -T as_6_5 %s | FileCheck %s

// CHECK: define void @main

struct MeshPayload
{
  int4 data;
  bool2x2 mat;
};

struct GSStruct
{
  row_major bool2x2 mat;
  int4 vecmat;
  MeshPayload pld[2];
};

groupshared GSStruct gs[2];

row_major bool2x2 row_mat_array[2];

int i, j;

[numthreads(4,1,1)]
void main(uint gtid : SV_GroupIndex)
{
  // write to dynamic row/col
  gs[j].pld[i].mat[gtid >> 1][gtid & 1] = (int)gtid;
  gs[j].vecmat[gtid] = (int)gtid;

  int2x2 mat = gs[j].pld[i].mat;
  gs[j].pld[i].mat = (bool2x2)gs[j].vecmat;

  // subscript + constant GEP for component
  gs[j].pld[i].mat[1].x = mat[1].y;
  mat[0].y = gs[j].pld[i].mat[0].x;

  // dynamic subscript + constant component index
  gs[j].pld[i].mat[gtid & 1].x = mat[gtid & 1].y;
  mat[gtid & 1].y = gs[j].pld[i].mat[gtid & 1].x;

  // dynamic subscript + GEP for component
  gs[j].pld[i].mat[gtid & 1] = mat[gtid & 1].y;
  mat[gtid & 1].y = gs[j].pld[i].mat[gtid & 1].x;

  // subscript element
  gs[j].pld[i].mat._m01_m10 = mat[1];
  mat[0] = gs[j].pld[i].mat._m00_m11;

  // dynamic index of subscript element vector
  mat[0].x = gs[j].pld[i].mat._m00_m11_m10[gtid & 1];
  gs[j].pld[i].mat._m11_m10[gtid & 1] = gtid;

  // Dynamic index into vector
  int idx = gs[j].vecmat.x;
  gs[j].pld[i].mat[1][idx] = mat[1].y;
  mat[0].y = gs[j].pld[i].mat[0][idx];
  int2 vec = gs[j].mat[0];
  int2 multiplied = mul(mat, vec);
  gs[j].pld[i].data = multiplied.xyxy;
  DispatchMesh(1,1,1,gs[j].pld[i]);
}
