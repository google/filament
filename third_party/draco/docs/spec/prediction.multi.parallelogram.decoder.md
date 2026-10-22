## Multi Parallelogram Prediction Decoder

### MeshPredictionSchemeConstrainedMultiParallelogramDecoder_ComputeOriginalValues()

~~~~~
void MeshPredictionSchemeConstrainedMultiParallelogramDecoder_ComputeOriginalValues(
      num_values) {
  signed_values = seq_int_att_dec_symbols_to_signed_ints[curr_att_dec][curr_att];
  num_components = GetNumComponents();
  for (i = 0; i < kMaxNumParallelograms; ++i) {
    pred_vals[i].resize(num_components, 0);
  }
  out_values = signed_values;
  PredictionSchemeTransform_ComputeOriginalValue(
      pred_vals[0], &signed_values[0], &out_values[0]);
  is_crease_edge_pos.assign(kMaxNumParallelograms, 0);
  corner_map_size = num_values;
  for (p = 1; p < corner_map_size; ++p) {
    start_corner_id = encoded_attribute_value_index_to_corner_map[curr_att_dec][p];
    corner_id = start_corner_id;
    num_parallelograms = 0;
    first_pass = true;
    while (corner_id >= 0) {
      if (ComputeParallelogramPrediction(p, corner_id, &out_values[0],
          num_components, &(pred_vals[num_parallelograms][0]))) {
        ++num_parallelograms;
        if (num_parallelograms == kMaxNumParallelograms)
          break;
      }
      if (first_pass) {
        corner_id = SwingLeft(curr_att_dec, corner_id);
      } else {
        corner_id = SwingRight(curr_att_dec, corner_id);
      }
      if (corner_id == start_corner_id) {
        break;
      }
      if (corner_id < 0 && first_pass) {
        first_pass = false;
        corner_id = SwingRight(curr_att_dec, start_corner_id);
      }
    }
    is_crease_edge_ = pred_cons_multi_is_cease_edge[curr_att_dec][curr_att];
    num_used_parallelograms = 0;
    if (num_parallelograms > 0) {
      for (i = 0; i < num_components; ++i) {
        multi_pred_vals[i] = 0;
      }
      for (i = 0; i < num_parallelograms; ++i) {
        context = num_parallelograms - 1;
        is_crease = is_crease_edge_[context][is_crease_edge_pos[context]++];
        if (!is_crease) {
          ++num_used_parallelograms;
          for (j = 0; j < num_components; ++j) {
            multi_pred_vals[j] += pred_vals[i][j];
          }
        }
      }
    }
    dst_offset = p * num_components;
    if (num_used_parallelograms == 0) {
      src_offset = (p - 1) * num_components;
      PredictionSchemeTransform_ComputeOriginalValue(&out_values[src_offset],
          &signed_values[dst_offset], &out_values[dst_offset]);
    } else {
      for (c = 0; c < num_components; ++c) {
        multi_pred_vals[c] /= num_used_parallelograms;
      }
      PredictionSchemeTransform_ComputeOriginalValue(multi_pred_vals,
          &signed_values[dst_offset], &out_values[dst_offset]);
    }
  }
  seq_int_att_dec_original_values[curr_att_dec][curr_att] = out_values;
}

~~~~~
{:.draco-syntax}
