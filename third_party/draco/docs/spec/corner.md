## Corners

### Next()

~~~~~
int Next(corner) {
  if (corner < 0)
    return corner;
  return ((corner % 3) == 2) ? corner - 2 : corner + 1;
}
~~~~~
{:.draco-syntax}


### Previous()

~~~~~
int Previous(corner) {
  if (corner < 0)
    return corner;
  return ((corner % 3) == 0) ? corner + 2 : corner - 1;
}
~~~~~
{:.draco-syntax}


### PosOpposite()

~~~~~
int PosOpposite(c) {
  if (c >= opposite_corners_.size())
    return -1;
  return opposite_corners_[c];
}
~~~~~
{:.draco-syntax}


### AttrOpposite()

~~~~~
int AttrOpposite(attr, corner) {
  if (IsCornerOppositeToSeamEdge(corner))
    return -1;
  return PosOpposite(corner);
}
~~~~~
{:.draco-syntax}


### Opposite()

~~~~~
int Opposite(att_dec, c) {
  if (att_dec == 0 || att_dec_decoder_type[att_dec] == MESH_VERTEX_ATTRIBUTE)
    return PosOpposite(c);
  return AttrOpposite(att_dec - 1, c);
}
~~~~~
{:.draco-syntax}


### GetLeftCorner()

~~~~~
int GetLeftCorner(corner_id) {
  if (corner_id < 0)
    return kInvalidCornerIndex;
  return PosOpposite(Previous(corner_id));
}
~~~~~
{:.draco-syntax}


### GetRightCorner()

~~~~~
int GetRightCorner(corner_id) {
  if (corner_id < 0)
    return kInvalidCornerIndex;
  return PosOpposite(Next(corner_id));
}
~~~~~
{:.draco-syntax}


### SwingRight()

~~~~~
int SwingRight(attr_dec, corner) {
  return Previous(Opposite(attr_dec, Previous(corner)));
}
~~~~~
{:.draco-syntax}


### SwingLeft()

~~~~~
int SwingLeft(attr_dec, corner) {
  return Next(Opposite(attr_dec, Next(corner)));
}
~~~~~
{:.draco-syntax}


### CornerToVert()

~~~~~
int CornerToVert(att_dec, corner_id) {
  CornerToVerts(att_dec, corner_id, &v, &n, &p);
  return v;
}
~~~~~
{:.draco-syntax}


### CornerToVertsInternal()

~~~~~
void CornerToVertsInternal(ftv, corner_id, v, n, p) {
  local = corner_id % 3;
  face = corner_id / 3;
  if (local == 0) {
    v = ftv[0][face];
    n = ftv[1][face];
    p = ftv[2][face];
  } else if (local == 1) {
    v = ftv[1][face];
    n = ftv[2][face];
    p = ftv[0][face];
  } else if (local == 2) {
    v = ftv[2][face];
    n = ftv[0][face];
    p = ftv[1][face];
  }
}
~~~~~
{:.draco-syntax}


### CornerToVerts()

~~~~~
void CornerToVerts(att_dec, corner_id, v, n, p) {
  if (att_dec == 0) {
    return CornerToVertsInternal(face_to_vertex, corner_id, v, n, p);
  } else {
    if (att_dec_decoder_type[att_dec] == MESH_VERTEX_ATTRIBUTE) {
      return CornerToVertsInternal(face_to_vertex, corner_id, v, n, p);
    } else {
      return CornerToVertsInternal(attr_face_to_vertex[att_dec - 1], corner_id,
                                   v, n, p);
    }
  }
}
~~~~~
{:.draco-syntax}


### SetOppositeCorners()

~~~~~
void SetOppositeCorners(c, opp_c) {
  opposite_corners_[c] = opp_c;
  opposite_corners_[opp_c] = c;
}
~~~~~
{:.draco-syntax}


### MapCornerToVertex()

~~~~~
void MapCornerToVertex(corner_id, vert_id) {
  corner_to_vertex_map_[0][corner_id] = vert_id;
  if (vert_id >= 0) {
    vertex_corners_[vert_id] = corner_id;
  }
}
~~~~~
{:.draco-syntax}


### UpdateVertexToCornerMap()

~~~~~
void UpdateVertexToCornerMap(vert) {
  first_c = vertex_corners_[vert];
  if (first_c < 0)
    return;
  act_c = SwingLeft(0, first_c);
  c = first_c;
  while (act_c >= 0 && act_c != first_c) {
    c = act_c;
    act_c = SwingLeft(0, act_c);
  }
  if (act_c != first_c) {
    vertex_corners_[vert] = c;
  }
}
~~~~~
{:.draco-syntax}
