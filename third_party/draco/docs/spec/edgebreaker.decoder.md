## EdgeBreaker Decoder

### ParseEdgebreakerConnectivityData()

~~~~~
void ParseEdgebreakerConnectivityData() {
  edgebreaker_traversal_type                                                          UI8
  num_encoded_vertices                                                                varUI32
  num_faces                                                                           varUI32
  num_attribute_data                                                                  UI8
  num_encoded_symbols                                                                 varUI32
  num_encoded_split_symbols                                                           varUI32
}
~~~~~
{:.draco-syntax }


### ParseTopologySplitEvents()

~~~~~
void ParseTopologySplitEvents() {
  num_topology_splits                                                                 varUI32
  for (i = 0; i < num_topology_splits; ++i) {
    source_id_delta[i]                                                                varUI32
    split_id_delta[i]                                                                 varUI32
  }
  for (i = 0; i < num_topology_splits; ++i) {
    source_edge_bit[i]                                                                f[1]
  }
  ResetBitReader();
}
~~~~~
{:.draco-syntax }


### DecodeEdgebreakerConnectivityData()

~~~~~
void DecodeEdgebreakerConnectivityData() {
  curr_att_dec = 0;
  curr_att = 0;
  ParseEdgebreakerConnectivityData();
  DecodeTopologySplitEvents();
  EdgebreakerTraversalStart();
  DecodeEdgeBreakerConnectivity();
}
~~~~~
{:.draco-syntax }

### GetNumComponents()

~~~~~
int GetNumComponents() {
  decoder_type = seq_att_dec_decoder_type[curr_att_dec][curr_att];
  if (decoder_type == SEQUENTIAL_ATTRIBUTE_ENCODER_NORMALS) {
    prediction_scheme = seq_att_dec_prediction_scheme[curr_att_dec][curr_att];
    if (prediction_scheme == PREDICTION_DIFFERENCE) {
      return 2;
    }
  }
  return att_dec_num_components[curr_att_dec][curr_att];
}
~~~~~
{:.draco-syntax }


### ProcessSplitData()

~~~~~
void ProcessSplitData() {
  last_id = 0;
  for (i = 0; i < source_id_delta.size(); ++i) {
    source_symbol_id[i] = source_id_delta[i] + last_id;
    split_symbol_id[i] = source_symbol_id[i] - split_id_delta[i];
    last_id = source_symbol_id[i];
  }
}
~~~~~
{:.draco-syntax }


### DecodeTopologySplitEvents()

~~~~~
void DecodeTopologySplitEvents() {
  ParseTopologySplitEvents();
  ProcessSplitData();
}
~~~~~
{:.draco-syntax }


### IsTopologySplit()

~~~~~
bool IsTopologySplit(encoder_symbol_id, out_face_edge,
                     out_encoder_split_symbol_id) {
  if (source_symbol_id.back() != encoder_symbol_id)
    return false;
  out_face_edge = source_edge_bit.pop_back();
  out_encoder_split_symbol_id = split_symbol_id.pop_back();
  source_symbol_id.pop_back();
  return true;
}
~~~~~
{:.draco-syntax }


### ReplaceVerts()

~~~~~
void ReplaceVerts(from, to) {
  for (i = 0; i < face_to_vertex[0].size(); ++i) {
    if (face_to_vertex[0][i] == from) {
      face_to_vertex[0][i] = to;
    }
    if (face_to_vertex[1][i] == from) {
      face_to_vertex[1][i] = to;
    }
    if (face_to_vertex[2][i] == from) {
      face_to_vertex[2][i] = to;
    }
  }
}
~~~~~
{:.draco-syntax }


### UpdateCornersAfterMerge()

~~~~~
void UpdateCornersAfterMerge(c, v) {
  opp_corner = PosOpposite(c);
  if (opp_corner >= 0) {
    corner_n = Next(opp_corner);
    while (corner_n >= 0) {
      MapCornerToVertex(corner_n, v);
      corner_n = SwingLeft(0, corner_n);
    }
  }
}
~~~~~
{:.draco-syntax }


### NewActiveCornerReached()

~~~~~
void NewActiveCornerReached(new_corner, symbol_id) {
  check_topology_split = false;
  switch (last_symbol_) {
    case TOPOLOGY_C:
      {
        corner_a = active_corner_stack.back();
        corner_b = Previous(corner_a);
        while (PosOpposite(corner_b) >= 0) {
          b_opp = PosOpposite(corner_b);
          corner_b = Previous(b_opp);
        }
        SetOppositeCorners(corner_a, new_corner + 1);
        SetOppositeCorners(corner_b, new_corner + 2);
        active_corner_stack.back() = new_corner;
      }
      vert = CornerToVert(curr_att_dec, Next(corner_a));
      next = CornerToVert(curr_att_dec, Next(corner_b));
      prev = CornerToVert(curr_att_dec, Previous(corner_a));
      if (edgebreaker_traversal_type == VALENCE_EDGEBREAKER) {
        vertex_valences_[next] += 1;
        vertex_valences_[prev] += 1;
      }
      face_to_vertex[0].push_back(vert);
      face_to_vertex[1].push_back(next);
      face_to_vertex[2].push_back(prev);
      is_vert_hole_[vert] = false;
      MapCornerToVertex(new_corner, vert);
      MapCornerToVertex(new_corner + 1, next);
      MapCornerToVertex(new_corner + 2, prev);
      break;
    case TOPOLOGY_S:
      {
        corner_b = active_corner_stack.pop_back();
        for (i = 0; i < topology_split_id.size(); ++i) {
          if (topology_split_id[i] == symbol_id) {
            active_corner_stack.push_back(split_active_corners[i]);
          }
        }
        corner_a = active_corner_stack.back();
        SetOppositeCorners(corner_a, new_corner + 2);
        SetOppositeCorners(corner_b, new_corner + 1);
        active_corner_stack.back() = new_corner;
      }

      vert = CornerToVert(curr_att_dec, Previous(corner_a));
      next = CornerToVert(curr_att_dec, Next(corner_a));
      prev = CornerToVert(curr_att_dec, Previous(corner_b));
      MapCornerToVertex(new_corner, vert);
      MapCornerToVertex(new_corner + 1, next);
      MapCornerToVertex(new_corner + 2, prev);
      corner_n = Next(corner_b);
      vertex_n = CornerToVert(curr_att_dec, corner_n);
      if (edgebreaker_traversal_type == VALENCE_EDGEBREAKER) {
        vertex_valences_[vert] += vertex_valences_[vertex_n];
      }
      ReplaceVerts(vertex_n, vert);
      if (edgebreaker_traversal_type == VALENCE_EDGEBREAKER) {
        vertex_valences_[next] += 1;
        vertex_valences_[prev] += 1;
      }
      face_to_vertex[0].push_back(vert);
      face_to_vertex[1].push_back(next);
      face_to_vertex[2].push_back(prev);
      UpdateCornersAfterMerge(new_corner + 1, vert);
      vertex_corners_[vertex_n] = kInvalidCornerIndex;
      break;
    case TOPOLOGY_R:
      {
        corner_a = active_corner_stack.back();
        opp_corner = new_corner + 2;
        SetOppositeCorners(opp_corner, corner_a);
        active_corner_stack.back() = new_corner;
      }
      check_topology_split = true;
      vert = CornerToVert(curr_att_dec, Previous(corner_a));
      next = CornerToVert(curr_att_dec, Next(corner_a));
      prev = ++last_vert_added;
      if (edgebreaker_traversal_type == VALENCE_EDGEBREAKER) {
        vertex_valences_[vert] += 1;
        vertex_valences_[next] += 1;
        vertex_valences_[prev] += 2;
      }

      face_to_vertex[0].push_back(vert);
      face_to_vertex[1].push_back(next);
      face_to_vertex[2].push_back(prev);

      MapCornerToVertex(new_corner + 2, prev);
      MapCornerToVertex(new_corner, vert);
      MapCornerToVertex(new_corner + 1, next);
      break;
    case TOPOLOGY_L:
      {
        corner_a = active_corner_stack.back();
        opp_corner = new_corner + 1;
        SetOppositeCorners(opp_corner, corner_a);
        active_corner_stack.back() = new_corner;
      }
      check_topology_split = true;
      vert = CornerToVert(curr_att_dec, Next(corner_a));
      next = ++last_vert_added;
      prev = CornerToVert(curr_att_dec, Previous(corner_a));
      if (edgebreaker_traversal_type == VALENCE_EDGEBREAKER) {
        vertex_valences_[vert] += 1;
        vertex_valences_[next] += 2;
        vertex_valences_[prev] += 1;
      }

      face_to_vertex[0].push_back(vert);
      face_to_vertex[1].push_back(next);
      face_to_vertex[2].push_back(prev);

      MapCornerToVertex(new_corner + 2, prev);
      MapCornerToVertex(new_corner, vert);
      MapCornerToVertex(new_corner + 1, next);
      break;
    case TOPOLOGY_E:
      active_corner_stack.push_back(new_corner);
      check_topology_split = true;
      vert = last_vert_added + 1;
      next = vert + 1;
      prev = next + 1;
      if (edgebreaker_traversal_type == VALENCE_EDGEBREAKER) {
        vertex_valences_[vert] += 2;
        vertex_valences_[next] += 2;
        vertex_valences_[prev] += 2;
      }
      face_to_vertex[0].push_back(vert);
      face_to_vertex[1].push_back(next);
      face_to_vertex[2].push_back(prev);
      last_vert_added = prev;
      MapCornerToVertex(new_corner, vert);
      MapCornerToVertex(new_corner + 1, next);
      MapCornerToVertex(new_corner + 2, prev);
      break;
  }

  if (edgebreaker_traversal_type == VALENCE_EDGEBREAKER) {
    // Compute the new context that is going to be used
    // to decode the next symbol.
    active_valence = vertex_valences_[next];
    if (active_valence < MIN_VALENCE) {
      clamped_valence = MIN_VALENCE;
    } else if (active_valence > MAX_VALENCE) {
      clamped_valence = MAX_VALENCE;
    } else {
      clamped_valence = active_valence;
    }
    active_context_ = (clamped_valence - MIN_VALENCE);
  }

  if (check_topology_split) {
    encoder_symbol_id = num_encoded_symbols - symbol_id - 1;
    while (IsTopologySplit(encoder_symbol_id, &split_edge,
                           &enc_split_id)) {
      act_top_corner = active_corner_stack.back();
      if (split_edge == RIGHT_FACE_EDGE) {
        new_active_corner = Next(act_top_corner);
      } else {
        new_active_corner = Previous(act_top_corner);
      }
      // Convert the encoder split symbol id to decoder symbol id.
      dec_split_id = num_encoded_symbols - enc_split_id - 1;
      topology_split_id.push_back(dec_split_id);
      split_active_corners.push_back(new_active_corner);
    }
  }
}
~~~~~
{:.draco-syntax }


### ParseEdgebreakerStandardSymbol()

~~~~~
void ParseEdgebreakerStandardSymbol() {
  symbol = eb_symbol_buffer.ReadBits(1);
  if (symbol != TOPOLOGY_C) {
    // Else decode two additional bits.
    symbol_suffix = eb_symbol_buffer.ReadBits(2);
    symbol |= (symbol_suffix << 1);
  }
  last_symbol_ = symbol;
}
~~~~~
{:.draco-syntax }


### EdgebreakerDecodeSymbol()

~~~~~
void EdgebreakerDecodeSymbol() {
  if (edgebreaker_traversal_type == VALENCE_EDGEBREAKER) {
    EdgebreakerValenceDecodeSymbol();
  } else if (edgebreaker_traversal_type == STANDARD_EDGEBREAKER) {
    ParseEdgebreakerStandardSymbol();
  }
}
~~~~~
{:.draco-syntax }


### DecodeEdgeBreakerConnectivity()

~~~~~
void DecodeEdgeBreakerConnectivity() {
  is_vert_hole_.assign(num_encoded_vertices + num_encoded_split_symbols, true);
  last_vert_added = -1;
  for (i = 0; i < num_encoded_symbols; ++i) {
    EdgebreakerDecodeSymbol();
    corner = 3 * i;
    NewActiveCornerReached(corner, i);
  }
  ProcessInteriorEdges();
}
~~~~~
{:.draco-syntax }


### ProcessInteriorEdges()

~~~~~
void ProcessInteriorEdges() {
  RansInitDecoder(ans_decoder_, eb_start_face_buffer,
      eb_start_face_buffer_size, L_RANS_BASE);

  while (active_corner_stack.size() > 0) {
    corner_a = active_corner_stack.pop_back();
    RabsDescRead(ans_decoder_,
        eb_start_face_buffer_prob_zero, &interior_face);
    if (interior_face) {
      corner_b = Previous(corner_a);
      while (PosOpposite(corner_b) >= 0) {
        b_opp = PosOpposite(corner_b);
        corner_b = Previous(b_opp);
      }
      corner_c = Next(corner_a);
      while (PosOpposite(corner_c) >= 0) {
        c_opp = PosOpposite(corner_c);
        corner_c = Next(c_opp);
      }
      new_corner = face_to_vertex[0].size() * 3;
      SetOppositeCorners(new_corner, corner_a);
      SetOppositeCorners(new_corner + 1, corner_b);
      SetOppositeCorners(new_corner + 2, corner_c);

      CornerToVerts(0, corner_a, &temp_v, &next_a, &temp_p);
      CornerToVerts(0, corner_b, &temp_v, &next_b, &temp_p);
      CornerToVerts(0, corner_c, &temp_v, &next_c, &temp_p);
      MapCornerToVertex(new_corner, next_b);
      MapCornerToVertex(new_corner + 1, next_c);
      MapCornerToVertex(new_corner + 2, next_a);
      face_to_vertex[0].push_back(next_b);
      face_to_vertex[1].push_back(next_c);
      face_to_vertex[2].push_back(next_a);

      // Mark all three vertices as interior.
      is_vert_hole_[next_b] = false;
      is_vert_hole_[next_c] = false;
      is_vert_hole_[next_a] = false;
    }
  }
}
~~~~~
{:.draco-syntax }
