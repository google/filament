
## Attributes Decoder

### ParseAttributeDecodersData()

~~~~~
void ParseAttributeDecodersData() {
  num_attributes_decoders                                                             UI8
  if (encoder_method == MESH_EDGEBREAKER_ENCODING) {
    for (i = 0; i < num_attributes_decoders; ++i) {
      att_dec_data_id[i]                                                              UI8
      att_dec_decoder_type[i]                                                         UI8
      att_dec_traversal_method[i]                                                     UI8
    }
  }
  for (i = 0; i < num_attributes_decoders; ++i) {
    att_dec_num_attributes[i]                                                         varUI32
    for (j = 0; j < att_dec_num_attributes[i]; ++j) {
      att_dec_att_type[i][j]                                                          UI8
      att_dec_data_type[i][j]                                                         UI8
      att_dec_num_components[i][j]                                                    UI8
      att_dec_normalized[i][j]                                                        UI8
      att_dec_unique_id[i][j]                                                         varUI32
    }
    for (j = 0; j < att_dec_num_attributes[i]; ++j) {
      seq_att_dec_decoder_type[i][j]                                                  UI8
    }
  }
}
~~~~~
{:.draco-syntax }


### DecodeAttributeData()

~~~~~
void DecodeAttributeData() {
  ParseAttributeDecodersData();
  vertex_visited_point_ids.assign(num_attributes_decoders, 0);
  curr_att_dec = 0;
  if (encoder_method == MESH_EDGEBREAKER_ENCODING) {
    DecodeAttributeSeams();
    for (i = 0; i < num_encoded_vertices + num_encoded_split_symbols; ++i) {
      if (is_vert_hole_[i]) {
        UpdateVertexToCornerMap(i);
      }
    }
    for (i = 1; i < num_attributes_decoders; ++i) {
      curr_att_dec = i;
      RecomputeVerticesInternal();
    }
    Attribute_AssignPointsToCorners();
  }
  for (i = 0; i < num_attributes_decoders; ++i) {
    curr_att_dec = i;
    is_face_visited_.assign(num_faces, false);
    is_vertex_visited_.assign(num_faces * 3, false);
    GenerateSequence();
    if (encoder_method == MESH_EDGEBREAKER_ENCODING) {
      UpdatePointToAttributeIndexMapping();
    }
  }
  for (i = 0; i < num_attributes_decoders; ++i) {
    for (j = 0; j < att_dec_num_attributes[i]; ++j) {
      att_dec_num_values_to_decode[i][j] =
          encoded_attribute_value_index_to_corner_map[i].size();
    }
  }
  for (i = 0; i < num_attributes_decoders; ++i) {
    curr_att_dec = i;
    DecodePortableAttributes();
    DecodeDataNeededByPortableTransforms();
    TransformAttributesToOriginalFormat();
  }
}
~~~~~
{:.draco-syntax }


### RecomputeVerticesInternal()

~~~~~
void RecomputeVerticesInternal() {
  attr = curr_att_dec - 1;
  num_new_vertices = 0;
  attr_face_to_vertex.push_back(face_to_vertex);
  corner_to_vertex_map_[curr_att_dec].assign(
      attr_face_to_vertex[attr][0].size() * 3, -1);
  for (v = 0; v < num_encoded_vertices + num_encoded_split_symbols; ++v) {
    c = vertex_corners_[v];
    if (c < 0)
      continue;
    first_vert_id = num_new_vertices++;
    first_c = c;
    if (IsVertexOnAttributeSeam(attr, v)) {
      act_c = SwingLeft(curr_att_dec, first_c);
      while (act_c >= 0) {
        first_c = act_c;
        act_c = SwingLeft(curr_att_dec, act_c);
      }
    }
    corner_to_vertex_map_[curr_att_dec][first_c] = first_vert_id;
    vertex_to_left_most_corner_map_[attr].push_back(first_c);
    act_c = SwingRight(0, first_c);
    while (act_c >= 0 && act_c != first_c) {
      next_act_c = Next(act_c);
      if (IsCornerOppositeToSeamEdge(next_act_c)) {
        first_vert_id = num_new_vertices++;
        vertex_to_left_most_corner_map_[attr].push_back(act_c);
      }
      corner_to_vertex_map_[curr_att_dec][act_c] = first_vert_id;
      act_c = SwingRight(0, act_c);
    }
  }

  for (i = 0; i < corner_to_vertex_map_[curr_att_dec].size(); i += 3) {
    face = i / 3;
    attr_face_to_vertex[attr][0][face] = corner_to_vertex_map_[curr_att_dec][i];
    attr_face_to_vertex[attr][1][face] = corner_to_vertex_map_[curr_att_dec][i + 1];
    attr_face_to_vertex[attr][2][face] = corner_to_vertex_map_[curr_att_dec][i + 2];
  }
}
~~~~~
{:.draco-syntax }


### Attribute_AssignPointsToCorners()

~~~~~
void Attribute_AssignPointsToCorners() {
  point_to_corner_map_count = 0;
  for (v = 0; v < num_encoded_vertices + num_encoded_split_symbols; ++v) {
    c = vertex_corners_[v];
    if (c < 0)
      continue;
    deduplication_first_corner = c;
    if (is_vert_hole_[v]) {
      deduplication_first_corner = c;
    } else {
      for (i = 1; i < num_attributes_decoders; ++i) {
        attr_id = i - 1;
        if (!IsCornerOnAttributeSeam(0, attr_id, c))
          continue;
        vert_id = corner_to_vertex_map_[i][c];
        act_c = SwingRight(0, c);
        seam_found = false;
        while (act_c != c) {
          act_vert_id = corner_to_vertex_map_[i][act_c];
          if (act_vert_id != vert_id) {
            deduplication_first_corner = act_c;
            seam_found = true;
            break;
          }
          act_c = SwingRight(0, act_c);
        }
        if (seam_found)
          break;
      }
    }

    c = deduplication_first_corner;
    corner_to_point_map[c] = point_to_corner_map_count++;
    prev_c = c;
    c = SwingRight(0, c);
    while (c >= 0 && c != deduplication_first_corner) {
      attribute_seam = false;
      for (i = 1; i < num_attributes_decoders; ++i) {
        vert_id = corner_to_vertex_map_[i][c];
        prev_vert_id = corner_to_vertex_map_[i][prev_c];
        if (vert_id != prev_vert_id) {
          attribute_seam = true;
          break;
        }
      }
      if (attribute_seam) {
        corner_to_point_map[c] = point_to_corner_map_count++;
      } else {
        corner_to_point_map[c] = corner_to_point_map[prev_c];
      }
      prev_c = c;
      c = SwingRight(0, c);
    }
  }
}
~~~~~
{:.draco-syntax }


### SequentialGenerateSequence()

~~~~~
void SequentialGenerateSequence() {
  for (i = 0; i < num_points; ++i) {
    encoded_attribute_value_index_to_corner_map[curr_att_dec][i] = i;
  }
}
~~~~~
{:.draco-syntax }


### EdgebreakerGenerateSequence()

~~~~~
void EdgebreakerGenerateSequence() {
  if (att_dec_traversal_method[curr_att_dec] == MESH_TRAVERSAL_PREDICTION_DEGREE) {
    prediction_degree_.assign(num_encoded_vertices + num_encoded_split_symbols, 0);
  }
  for (i = 0; i < num_faces; ++i) {
    if (att_dec_traversal_method[curr_att_dec] == MESH_TRAVERSAL_DEPTH_FIRST) {
      if (curr_att_dec == 0) {
        EdgeBreakerTraverser_ProcessCorner(3 * i);
      } else {
        EdgeBreakerAttributeTraverser_ProcessCorner(3 * i);
      }
    } else {
      PredictionDegree_TraverseFromCorner(3 * i);
    }
  }
}
~~~~~
{:.draco-syntax }


### GenerateSequence()

~~~~~
void GenerateSequence() {
  if (encoder_method == MESH_EDGEBREAKER_ENCODING)
    EdgebreakerGenerateSequence();
  else
    SequentialGenerateSequence();
}
~~~~~
{:.draco-syntax }


### UpdatePointToAttributeIndexMapping()

~~~~~
void UpdatePointToAttributeIndexMapping() {
  indices_map_.assign(num_faces * 3, -1);
  for (f = 0; f < num_faces; ++f) {
    for (p = 0; p < 3; ++p) {
      corner = (f * 3) + p;
      point_id = corner_to_point_map[corner];
      CornerToVerts(curr_att_dec, corner, &vert, &next, &prev);
      att_entry_id =
          vertex_to_encoded_attribute_value_index_map[curr_att_dec][vert];
      indices_map_[point_id] = att_entry_id;
    }
  }
}
~~~~~
{:.draco-syntax }


### TransformAttributesToOriginalFormat_StoreValues()

~~~~~
void TransformAttributesToOriginalFormat_StoreValues() {
  num_components = GetNumComponents();
  num_values = att_dec_num_values_to_decode[curr_att_dec][curr_att];
  portable_attribute_data = seq_int_att_dec_original_values[curr_att_dec][curr_att];
  for (i = 0; i < num_values; ++i) {
    for (c = 0; c < num_components; ++c) {
      out_values.push_back(portable_attribute_data[(i * num_components) + c]);
    }
  }
  seq_int_att_dec_dequantized_values[curr_att_dec][curr_att] = out_values;
}
~~~~~
{:.draco-syntax }


### TransformAttributesToOriginalFormat()

~~~~~
void TransformAttributesToOriginalFormat() {
  for (i = 0; i < att_dec_num_attributes.back(); ++i) {
    curr_att = i;
    dec_type = seq_att_dec_decoder_type[curr_att_dec][curr_att];
    if (dec_type == SEQUENTIAL_ATTRIBUTE_ENCODER_NORMALS) {
      TransformAttributesToOriginalFormat_Normal();
    } else if (dec_type == SEQUENTIAL_ATTRIBUTE_ENCODER_INTEGER) {
      TransformAttributesToOriginalFormat_StoreValues();
    } else {
      SequentialQuantizationAttributeDecoder_DequantizeValues();
    }
  }
}
~~~~~
{:.draco-syntax }
