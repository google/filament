## Parallelogram Prediction Decoder

### GetParallelogramEntries()

~~~~~
void GetParallelogramEntries(ci, opp_entry,
                             next_entry, prev_entry) {
  CornerToVerts(curr_att_dec, ci, &v, &n, &p);
  opp_entry = vertex_to_encoded_attribute_value_index_map[curr_att_dec][v];
  next_entry = vertex_to_encoded_attribute_value_index_map[curr_att_dec][n];
  prev_entry = vertex_to_encoded_attribute_value_index_map[curr_att_dec][p];
}
~~~~~
{:.draco-syntax}


### ComputeParallelogramPrediction()

~~~~~
bool ComputeParallelogramPrediction(data_entry_id, ci, in_data,
                                    num_components, out_prediction) {
  oci = Opposite(curr_att_dec, ci);
  if (oci < 0)
    return false;
  GetParallelogramEntries(oci, &vert_opp, &vert_next, &vert_prev);
  if (vert_opp < data_entry_id && vert_next < data_entry_id &&
      vert_prev < data_entry_id) {
    v_opp_off = vert_opp * num_components;
    v_next_off = vert_next * num_components;
    v_prev_off = vert_prev * num_components;
    for (c = 0; c < num_components; ++c) {
      out_prediction[c] = (in_data[v_next_off + c] + in_data[v_prev_off + c]) -
          in_data[v_opp_off + c];
    }
    return true;
  }
  return false;
}
~~~~~
{:.draco-syntax}


### MeshPredictionSchemeParallelogramDecoder_ComputeOriginalValues()

~~~~~
void MeshPredictionSchemeParallelogramDecoder_ComputeOriginalValues(num_values) {
  signed_values = seq_int_att_dec_symbols_to_signed_ints[curr_att_dec][curr_att];
  num_components = GetNumComponents();
  pred_vals.assign(num_components, 0);
  out_values = signed_values;
  PredictionSchemeWrapDecodingTransform_ComputeOriginalValue(pred_vals,
      &signed_values[0], &out_values[0]);
  corner_map_size = num_values;
  for (p = 1; p < corner_map_size; ++p) {
    corner_id = encoded_attribute_value_index_to_corner_map[curr_att_dec][p];
    dst_offset = p * num_components;
    if (!ComputeParallelogramPrediction(p, corner_id, &out_values[0],
                                        num_components, pred_vals)) {
      src_offset = (p - 1) * num_components;
      PredictionSchemeWrapDecodingTransform_ComputeOriginalValue(
          &out_values[src_offset], &signed_values[dst_offset],
          &out_values[dst_offset]);
    } else {
      PredictionSchemeWrapDecodingTransform_ComputeOriginalValue(
          pred_vals, &signed_values[dst_offset], &out_values[dst_offset]);
    }
  }
  seq_int_att_dec_original_values[curr_att_dec][curr_att] = out_values;
}
~~~~~
{:.draco-syntax}
