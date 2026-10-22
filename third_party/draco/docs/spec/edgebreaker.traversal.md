
## EdgeBreaker Traversal

### ParseEdgebreakerTraversalStandardSymbolData()

~~~~~
void ParseEdgebreakerTraversalStandardSymbolData() {
  sz = eb_symbol_buffer_size                                                          varUI64
  eb_symbol_buffer                                                                    UI8[sz]
}
~~~~~
{:.draco-syntax }


### ParseEdgebreakerTraversalStandardFaceData()

~~~~~
void ParseEdgebreakerTraversalStandardFaceData() {
  eb_start_face_buffer_prob_zero                                                      UI8
  sz = eb_start_face_buffer_size                                                      varUI32
  eb_start_face_buffer                                                                UI8[sz]
}
~~~~~
{:.draco-syntax }


### ParseEdgebreakerTraversalStandardAttributeConnectivityData()

~~~~~
void ParseEdgebreakerTraversalStandardAttributeConnectivityData() {
  for (i = 0; i < num_attribute_data; ++i) {
    attribute_connectivity_decoders_prob_zero[i]                                      UI8
    sz = attribute_connectivity_decoders_size[i]                                      varUI32
    attribute_connectivity_decoders_buffer[i]                                         UI8[sz]
  }
}
~~~~~
{:.draco-syntax }


### DecodeEdgebreakerTraversalStandardData()

~~~~~
void DecodeEdgebreakerTraversalStandardData() {
  ParseEdgebreakerTraversalStandardSymbolData()
  ParseEdgebreakerTraversalStandardFaceData()
  ParseEdgebreakerTraversalStandardAttributeConnectivityData()
}
~~~~~
{:.draco-syntax }


### EdgebreakerTraversalStart()

~~~~~
void EdgebreakerTraversalStart() {
  last_symbol_ = -1;
  active_context_ = -1;
  if (edgebreaker_traversal_type == STANDARD_EDGEBREAKER) {
    DecodeEdgebreakerTraversalStandardData();
  } else if (edgebreaker_traversal_type == VALENCE_EDGEBREAKER) {
    EdgeBreakerTraversalValenceStart();
  }
}
~~~~~
{:.draco-syntax }


### IsFaceVisited()

~~~~~
bool IsFaceVisited(face_id) {
  if (face_id < 0)
    return true;  // Invalid faces are always considered as visited.
  return is_face_visited_[face_id];
}
~~~~~
{:.draco-syntax }


### OnNewVertexVisited()

~~~~~
void OnNewVertexVisited(vertex, corner) {
  encoded_attribute_value_index_to_corner_map[curr_att_dec].push_back(corner);
  vertex_to_encoded_attribute_value_index_map[curr_att_dec][vertex] =
      vertex_visited_point_ids[curr_att_dec];
  vertex_visited_point_ids[curr_att_dec]++;
}
~~~~~
{:.draco-syntax }


### EdgeBreakerTraverser_ProcessCorner()

~~~~~
void EdgeBreakerTraverser_ProcessCorner(corner_id) {
  face = corner_id / 3;
  if (IsFaceVisited(face))
    return;  // Already traversed.
  corner_traversal_stack_.push_back(corner_id);
  next_vert = face_to_vertex[1][face];
  prev_vert = face_to_vertex[2][face];
  if (!is_vertex_visited_[next_vert]) {
    is_vertex_visited_[next_vert] = true;
    next_c = Next(corner_id);
    OnNewVertexVisited(next_vert, next_c);
  }
  if (!is_vertex_visited_[prev_vert]) {
    is_vertex_visited_[prev_vert] = true;
    prev_c = Previous(corner_id);
    OnNewVertexVisited(prev_vert, prev_c);
  }
  while (!corner_traversal_stack_.empty()) {
    corner_id = corner_traversal_stack_.back();
    face_id = corner_id / 3;
    if (corner_id < 0 || IsFaceVisited(face_id)) {
      // This face has been already traversed.
      corner_traversal_stack_.pop_back();
      continue;
    }
    while (true) {
      face_id = corner_id / 3;
      is_face_visited_[face_id] = true;
      vert_id = CornerToVert(0, corner_id);
      if (!is_vertex_visited_[vert_id]) {
        on_boundary = IsOnPositionBoundary(vert_id);
        is_vertex_visited_[vert_id] = true;
        OnNewVertexVisited(vert_id, corner_id);
        if (!on_boundary) {
          corner_id = GetRightCorner(corner_id);
          continue;
        }
      }
      right_corner_id = GetRightCorner(corner_id);
      left_corner_id = GetLeftCorner(corner_id);
      right_face_id = right_corner_id < 0 ? -1 : right_corner_id / 3;
      left_face_id = left_corner_id < 0 ? -1 : left_corner_id / 3;
      if (IsFaceVisited(right_face_id)) {
        if (IsFaceVisited(left_face_id)) {
          corner_traversal_stack_.pop_back();
          break;
        } else {
          corner_id = left_corner_id;
        }
      } else {
        if (IsFaceVisited(left_face_id)) {
          corner_id = right_corner_id;
        } else {
          corner_traversal_stack_.back() = left_corner_id;
          corner_traversal_stack_.push_back(right_corner_id);
          break;
        }
      }
    }
  }
}
~~~~~
{:.draco-syntax }


### EdgeBreakerAttributeTraverser_ProcessCorner()

~~~~~
void EdgeBreakerAttributeTraverser_ProcessCorner(corner_id) {
  face = corner_id / 3;
  if (IsFaceVisited(face))
    return;  // Already traversed.
  corner_traversal_stack_.push_back(corner_id);
  CornerToVerts(curr_att_dec, corner_id, &vert_id, &next_vert, &prev_vert);
  if (!is_vertex_visited_[next_vert]) {
    is_vertex_visited_[next_vert] = true;
    next_c = Next(corner_id);
    OnNewVertexVisited(next_vert, next_c);
  }
  if (!is_vertex_visited_[prev_vert]) {
    is_vertex_visited_[prev_vert] = true;
    prev_c = Previous(corner_id);
    OnNewVertexVisited(prev_vert, prev_c);
  }
  while (!corner_traversal_stack_.empty()) {
    corner_id = corner_traversal_stack_.back();
    face_id = corner_id / 3;
    if (corner_id < 0 || IsFaceVisited(face_id)) {
      corner_traversal_stack_.pop_back();
      continue;
    }
    while (true) {
      face_id = corner_id / 3;
      is_face_visited_[face_id] = true;
      vert_id = CornerToVert(curr_att_dec, corner_id);
      if (!is_vertex_visited_[vert_id]) {
        on_seam = IsOnBoundary(curr_att_dec, vert_id);
        pos_vert_id = CornerToVert(0, corner_id);
        on_boundary = (on_seam) ? on_seam : IsOnPositionBoundary(pos_vert_id);
        is_vertex_visited_[vert_id] = true;
        OnNewVertexVisited(vert_id, corner_id);
        if (!on_boundary) {
          corner_id = GetRightCorner(corner_id);
          continue;
        }
      }
      next_c = Next(corner_id);
      right_seam = IsCornerOppositeToSeamEdge(next_c);
      right_corner_id = (right_seam) ? -1 : GetRightCorner(corner_id);
      prev_c = Previous(corner_id);
      left_seam = IsCornerOppositeToSeamEdge(prev_c);
      left_corner_id = (left_seam) ? -1 : GetLeftCorner(corner_id);
      right_face_id = right_corner_id < 0 ? -1 : right_corner_id / 3;
      left_face_id = left_corner_id < 0 ? -1 : left_corner_id / 3;
        if (IsFaceVisited(left_face_id)) {
          corner_traversal_stack_.pop_back();
          break;
        } else {
          corner_id = left_corner_id;
        }
      } else {
        if (IsFaceVisited(left_face_id)) {
          corner_id = right_corner_id;
        } else {
          corner_traversal_stack_.back() = left_corner_id;
          corner_traversal_stack_.push_back(right_corner_id);
          break;
        }
      }
    }
  }
}
~~~~~
{:.draco-syntax }
