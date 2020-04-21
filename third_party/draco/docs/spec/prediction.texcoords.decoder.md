## TexCoords Prediction Decoder

### IntSqrt()

~~~~~
uint64_t IntSqrt(number) {
  if (number == 0)
    return 0;
  act_number = number;
  square_root = 1;
  while (act_number >= 2) {
    square_root *= 2;
    act_number /= 4;
  }
  do {
    square_root = (square_root + number / square_root) / 2;
  } while (square_root * square_root > number);
  return square_root;
}
~~~~~
{:.draco-syntax}


### GetPositionForEntryId()

~~~~~
void GetPositionForEntryId(entry_id, pos) {
  corner = encoded_attribute_value_index_to_corner_map[curr_att_dec][entry_id];
  point_id = corner_to_point_map[corner];
  mapped_index = indices_map_[0][point_id];
  pos_orig = seq_int_att_dec_original_values[0][0];
  for (i = 0; i < 3; ++i) {
    pos.push_back(pos_orig[(mapped_index * 3) + i]);
  }
}
~~~~~
{:.draco-syntax}


### GetTexCoordForEntryId()

~~~~~
void GetTexCoordForEntryId(entry_id, data, tex_coords) {
  data_offset = entry_id * kTexCoordsNumComponents;
  tex_coords.push_back(data[data_offset]);
  tex_coords.push_back(data[data_offset + 1]);
}
~~~~~
{:.draco-syntax}


### MeshPredictionSchemeTexCoordsPortablePredictor_ComputePredictedValue()

~~~~~
void MeshPredictionSchemeTexCoordsPortablePredictor_ComputePredictedValue(
     corner_id, data, data_id, predicted_value_) {
  CornerToVerts(curr_att_dec, corner_id, &vert_id, &next_vert_id, &prev_vert_id);
  next_data_id =
      vertex_to_encoded_attribute_value_index_map[curr_att_dec][next_vert_id];
  prev_data_id =
      vertex_to_encoded_attribute_value_index_map[curr_att_dec][prev_vert_id];

  if (prev_data_id < data_id && next_data_id < data_id) {
    GetTexCoordForEntryId(next_data_id, data, &n_uv);
    GetTexCoordForEntryId(prev_data_id, data, &p_uv);
    if (p_uv == n_uv) {
      predicted_value_[0] = p_uv[0];
      predicted_value_[1] = p_uv[1];
      return;
    }
    GetPositionForEntryId(data_id, &tip_pos);
    GetPositionForEntryId(next_data_id, &next_pos);
    GetPositionForEntryId(prev_data_id, &prev_pos);
    SubtractVectors(prev_pos, next_pos, &pn);
    Dot(pn, pn, &pn_norm2_squared);

    if (pn_norm2_squared != 0) {
      SubtractVectors(tip_pos, next_pos, &cn);
      Dot(cn, pn, &cn_dot_pn);
      SubtractVectors(p_uv, n_uv, &pn_uv);
      MultiplyScalar(pn_uv, cn_dot_pn, &vec_mult_1);
      MultiplyScalar(n_uv, pn_norm2_squared, &vec_mult_2);
      AddVectors(vec_mult_1, vec_mult_2, &x_uv);
      MultiplyScalar(pn, cn_dot_pn, &vec_mult);
      DivideScalar(vec_mult, pn_norm2_squared, &vec_div);
      AddVectors(next_pos, vec_div, &x_pos);
      SubtractVectors(tip_pos, x_pos, &vec_sub);
      Dot(vec_sub, vec_sub, &cx_norm2_squared);

      temp_vec.push_back(pn_uv[1]);
      temp_vec.push_back(-pn_uv[0]);
      norm_squared = IntSqrt(cx_norm2_squared * pn_norm2_squared);
      MultiplyScalar(temp_vec, norm_squared, &cx_uv);
      orientation = pred_tex_coords_orientations[curr_att_dec][curr_att].pop_back();
      if (orientation)
        AddVectors(x_uv, cx_uv, &temp_vec);
      else
        SubtractVectors(x_uv, cx_uv, &temp_vec);
      DivideScalar(temp_vec, pn_norm2_squared, &predicted_uv);
      predicted_value_[0] = predicted_uv[0];
      predicted_value_[1] = predicted_uv[1];
      return;
    }
  }
  data_offset = 0;
  if (prev_data_id < data_id) {
    data_offset = prev_data_id * kTexCoordsNumComponents;
  }
  if (next_data_id < data_id) {
    data_offset = next_data_id * kTexCoordsNumComponents;
  } else {
    if (data_id > 0) {
      data_offset = (data_id - 1) * kTexCoordsNumComponents;
    } else {
      for (i = 0; i < kTexCoordsNumComponents; ++i) {
        predicted_value_[i] = 0;
      }
      return;
    }
  }
  for (i = 0; i < kTexCoordsNumComponents; ++i) {
    predicted_value_[i] = data[data_offset + i];
  }
}
~~~~~
{:.draco-syntax}


### MeshPredictionSchemeTexCoordsPortableDecoder_ComputeOriginalValues()

~~~~~
void MeshPredictionSchemeTexCoordsPortableDecoder_ComputeOriginalValues(num_values)
{
  signed_values = seq_int_att_dec_symbols_to_signed_ints[curr_att_dec][curr_att];
  num_components = GetNumComponents();
  corner_map_size = num_values;
  out_values = signed_values;
  for (p = 0; p < corner_map_size; ++p) {
    corner_id = encoded_attribute_value_index_to_corner_map[curr_att_dec][p];
    MeshPredictionSchemeTexCoordsPortablePredictor_ComputePredictedValue(
        corner_id, &out_values[0], p, &predicted_value_);
    dst_offset = p * num_components;
    PredictionSchemeWrapDecodingTransform_ComputeOriginalValue(
        &predicted_value_[0], &out_values[dst_offset], &out_values[dst_offset]);
  }
  seq_int_att_dec_original_values[curr_att_dec][curr_att] = out_values;
}
~~~~~
{:.draco-syntax}
