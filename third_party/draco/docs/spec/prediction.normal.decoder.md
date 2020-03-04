## Normal Prediction Decoder

### GetPositionForDataId()

~~~~~
void GetPositionForDataId(data_id, pos) {
  corner = encoded_attribute_value_index_to_corner_map[curr_att_dec][data_id];
  point_id = corner_to_point_map[corner];
  mapped_index = indices_map_[0][point_id];
  pos_orig = seq_int_att_dec_original_values[0][0];
  for (i = 0; i < 3; ++i) {
    pos.push_back(pos_orig[(mapped_index * 3) + i]);
  }
}
~~~~~
{:.draco-syntax}


### GetPositionForCorner()

~~~~~
void GetPositionForCorner(ci, pos) {
  CornerToVerts(curr_att_dec, ci, &vert_id, &n, &p);
  data_id = vertex_to_encoded_attribute_value_index_map[curr_att_dec][vert_id];
  GetPositionForDataId(data_id, pos);
}
~~~~~
{:.draco-syntax}


### MeshPredictionSchemeGeometricNormalPredictorArea_ComputePredictedValue()

~~~~~
void MeshPredictionSchemeGeometricNormalPredictorArea_ComputePredictedValue(
    corner_id, predicted_value_) {
  GetPositionForCorner(corner_id, &pos_cent);
  normal.assign(3, 0);
  corner = corner_id;
  start_corner_ = corner;
  left_traversal_ = true;
  while (corner >= 0) {
    c_next = Next(corner);
    c_prev = Previous(corner);
    GetPositionForCorner(c_next, &pos_next);
    GetPositionForCorner(c_prev, &pos_prev);
    SubtractVectors(pos_next, pos_cent, &delta_next);
    SubtractVectors(pos_prev, pos_cent, &delta_prev);
    CrossProduct(delta_next, delta_prev, &cross);
    AddVectors(normal, cross, &temp_norm);
    for (i = 0; i < temp_norm.size(); ++i) {
      normal[i] = temp_norm[i];
    }
    if (left_traversal_) {
      left_c = SwingLeft(curr_att_dec, corner);
      corner = left_c;
      if (corner < 0) {
        right_c = SwingRight(curr_att_dec, start_corner_);
        corner = right_c;
        left_traversal_ = false;
      } else if (corner == start_corner_) {
        corner = kInvalidCornerIndex;
      }
    } else {
      right_c = SwingRight(curr_att_dec, corner);
      corner = right_c;
    }
  }
  AbsSum(normal, &abs_sum);
  upper_bound = 1 << 29;
  if (abs_sum > upper_bound) {
    quotient = abs_sum / upper_bound;
    DivideScalar(normal, quotient, &vec_div);
    for (i = 0; i < vec_div.size(); ++i) {
      normal[i] = vec_div[i];
    }
  }
  predicted_value_[0] = normal[0];
  predicted_value_[1] = normal[1];
  predicted_value_[2] = normal[2];
}
~~~~~
{:.draco-syntax}


### CanonicalizeIntegerVector()

~~~~~
void CanonicalizeIntegerVector(vec, center_value_) {
  abs_sum = Abs(vec[0]) + Abs(vec[1]) + Abs(vec[2]);
  if (abs_sum == 0) {
    vec[0] = center_value_;
  } else {
    vec[0] = (vec[0] * center_value_) / abs_sum;
    vec[1] = (vec[1] * center_value_) / abs_sum;
    if (vec[2] >= 0) {
      vec[2] = center_value_ - Abs(vec[0]) - Abs(vec[1]);
    } else {
      vec[2] = -(center_value_ - Abs(vec[0]) - Abs(vec[1]));
    }
  }
}
~~~~~
{:.draco-syntax}


### CanonicalizeOctahedralCoords()

~~~~~
void CanonicalizeOctahedralCoords(s, t, out_s,
                                  out_t, center_value_, max_value_) {
  if ((s == 0 && t == 0) || (s == 0 && t == max_value_) ||
      (s == max_value_ && t == 0)) {
    s = max_value_;
    t = max_value_;
  } else if (s == 0 && t > center_value_) {
    t = center_value_ - (t - center_value_);
  } else if (s == max_value_ && t < center_value_) {
    t = center_value_ + (center_value_ - t);
  } else if (t == max_value_ && s < center_value_) {
    s = center_value_ + (center_value_ - s);
  } else if (t == 0 && s > center_value_) {
    s = center_value_ - (s - center_value_);
  }
  out_s = s;
  out_t = t;
}
~~~~~
{:.draco-syntax}


### IntegerVectorToQuantizedOctahedralCoords()

~~~~~
void IntegerVectorToQuantizedOctahedralCoords(
    int_vec, out_s, out_t, center_value_, max_value_) {
  if (int_vec[0] >= 0) {
    s = (int_vec[1] + center_value_);
    t = (int_vec[2] + center_value_);
  } else {
    if (int_vec[1] < 0) {
      s = Abs(int_vec[2]);
    } else {
      s = (max_value_ - Abs(int_vec[2]));
    }
    if (int_vec[2] < 0) {
      t = Abs(int_vec[1]);
    } else {
      t = (max_value_ - Abs(int_vec[1]));
    }
  }
  CanonicalizeOctahedralCoords(s, t, out_s, out_t, center_value_, max_value_);
}
~~~~~
{:.draco-syntax}


### MeshPredictionSchemeGeometricNormalDecoder_ComputeOriginalValues()

~~~~~
void MeshPredictionSchemeGeometricNormalDecoder_ComputeOriginalValues(num_values) {
  signed_values = seq_int_att_dec_symbols_to_signed_ints[curr_att_dec][curr_att];
  encoded_max_quantized_value =
      pred_trasnform_normal_max_q_val[curr_att_dec][curr_att];
  quantization_bits_ = MostSignificantBit(encoded_max_quantized_value) + 1;
  max_quantized_value_ = (1 << quantization_bits_) - 1;
  max_value_ = max_quantized_value_ - 1;
  center_value_ = max_value_ / 2;
  corner_map_size = num_values;
  flip_normal_bits = pred_transform_normal_flip_normal_bits[curr_att_dec][curr_att];
  out_values = signed_values;
  for (data_id = 0; data_id < corner_map_size; ++data_id) {
    corner_id = encoded_attribute_value_index_to_corner_map[curr_att_dec][data_id];
    MeshPredictionSchemeGeometricNormalPredictorArea_ComputePredictedValue(
        corner_id, &pred_normal_3d);
    CanonicalizeIntegerVector(pred_normal_3d, center_value_);
    if (flip_normal_bits[data_id]) {
      for (i = 0; i < pred_normal_3d.size(); ++i) {
        pred_normal_3d[i] = -pred_normal_3d[i];
      }
    }
    IntegerVectorToQuantizedOctahedralCoords(&pred_normal_3d[0],
        &pred_normal_oct[0], &pred_normal_oct[1], center_value_, max_value_);
    data_offset = data_id * 2;
    PredictionSchemeNormalOctahedronCanonicalizedDecodingTransform_ComputeOriginalValue(
         &pred_normal_oct[0], &out_values[data_offset], &out_values[data_offset]);
  }
  seq_int_att_dec_original_values[curr_att_dec][curr_att] = out_values;
}
~~~~~
{:.draco-syntax}
