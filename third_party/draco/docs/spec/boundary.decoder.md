## Boundary Decoder

### DecodeAttributeSeams()

~~~~~
void DecodeAttributeSeams() {
  for (a = 0; a < num_attributes_decoders - 1; ++a) {
    RansInitDecoder(ans_decoder_,
        attribute_connectivity_decoders_buffer[a],
        attribute_connectivity_decoders_size[a], L_RANS_BASE);
    ans_decoders.push_back(ans_decoder_);
    is_edge_on_seam_[a].assign(face_to_vertex[0].size() * 3, false);
  }

  for (j = 0; j < num_faces; ++j) {
    face_id = j;
    for (k = 0; k < 3; ++k) {
      local = k;
      corner = (j * 3) + k;
      CornerToVerts(0, corner, v, n, p);
      opp_corner = PosOpposite(corner);
      boundary_edge = opp_corner < 0;
      if (!boundary_edge) {
        if (opp_corner >= corner) {
          for (a = 0; a < num_attributes_decoders - 1; ++a) {
            RabsDescRead(ans_decoders[a],
                         attribute_connectivity_decoders_prob_zero[a], &val);
            if (val) {
              att_connectivity_seam_opp[a].push_back(v);
              att_connectivity_seam_src[a].push_back(n);
              att_connectivity_seam_dest[a].push_back(p);
              is_edge_on_seam_[a][corner] = true;
              if (opp_corner >= 0) {
                CornerToVerts(curr_att_dec, opp_corner, &opp_v, &opp_n, &opp_p);
                att_connectivity_seam_opp[a].push_back(opp_v);
                att_connectivity_seam_src[a].push_back(opp_n);
                att_connectivity_seam_dest[a].push_back(opp_p);
                is_edge_on_seam_[a][opp_corner] = true;
              }
            }
          }
        }
      } else {
        for (a = 0; a < num_attributes_decoders - 1; ++a) {
          att_connectivity_seam_opp[a].push_back(v);
          att_connectivity_seam_src[a].push_back(n);
          att_connectivity_seam_dest[a].push_back(p);
          is_edge_on_seam_[a][corner] = true;
        }
      }
    }
  }
}
~~~~~
{:.draco-syntax}


### IsVertexOnAttributeSeam()

~~~~~
bool IsVertexOnAttributeSeam(attr, vert) {
  for (i = 0; i < att_connectivity_seam_src[attr].size(); ++i) {
    if (att_connectivity_seam_src[attr][i] == vert ||
        att_connectivity_seam_dest[attr][i] == vert) {
      return true;
    }
  }
  return false;
}
~~~~~
{:.draco-syntax}


### IsCornerOnSeam()

~~~~~
bool IsCornerOnSeam(corner) {
  CornerToVerts(0, corner, &v, &n, &p);
  return IsVertexOnAttributeSeam(curr_att_dec - 1, v);
}
~~~~~
{:.draco-syntax}


### IsCornerOnAttributeSeam()

~~~~~
bool IsCornerOnAttributeSeam(att_dec, attr, corner) {
  CornerToVerts(att_dec, corner, &v, &n, &p);
  return IsVertexOnAttributeSeam(attr, v);
}
~~~~~
{:.draco-syntax}


### IsCornerOppositeToSeamEdge()

~~~~~
bool IsCornerOppositeToSeamEdge(corner) {
  attr = curr_att_dec - 1;
  return is_edge_on_seam_[attr][corner];
}
~~~~~
{:.draco-syntax}


### IsOnPositionBoundary()

~~~~~
bool IsOnPositionBoundary(vert_id) {
  if (vertex_corners_[vert_id] < 0)
    return true;
  if (att_dec_decoder_type[curr_att_dec] == MESH_VERTEX_ATTRIBUTE)
    return IsCornerOnAttributeSeam(curr_att_dec, curr_att_dec - 1,
                                   vertex_corners_[vert_id]);
  return false;
}
~~~~~
{:.draco-syntax}


### IsOnAttributeBoundary()

~~~~~
bool IsOnAttributeBoundary(vert) {
  corner = vertex_to_left_most_corner_map_[curr_att_dec - 1][vert];
  if (corner < 0)
    return true;
  return IsCornerOnSeam(corner);
}
~~~~~
{:.draco-syntax}


### IsOnBoundary()

~~~~~
bool IsOnBoundary(att_dec, vert_id) {
  if (att_dec == 0 || att_dec_decoder_type[att_dec] == MESH_VERTEX_ATTRIBUTE)
    return IsOnPositionBoundary(vert_id);
  else
    return IsOnAttributeBoundary(vert_id);
}
~~~~~
{:.draco-syntax}

