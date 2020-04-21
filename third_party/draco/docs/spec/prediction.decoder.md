## Prediction Decoder

### ParsePredictionData()

~~~~~
void ParsePredictionData() {
  seq_att_dec_prediction_scheme[curr_att_dec][curr_att]                               I8
  if (seq_att_dec_prediction_scheme[curr_att_dec][curr_att] != PREDICTION_NONE) {
    seq_att_dec_prediction_transform_type[curr_att_dec][curr_att]                     I8
    seq_int_att_dec_compressed[curr_att_dec][curr_att]                                UI8
  }
}
~~~~~
{:.draco-syntax}


### DecodePortableAttributes()

~~~~~
void DecodePortableAttributes() {
  for (i = 0; i < att_dec_num_attributes.back(); ++i) {
    curr_att = i;
    ParsePredictionData();
    if (seq_att_dec_prediction_scheme[curr_att_dec][i] != PREDICTION_NONE) {
      SequentialIntegerAttributeDecoder_DecodeIntegerValues();
    }
  }
}
~~~~~
{:.draco-syntax}


### DecodeDataNeededByPortableTransforms()

~~~~~
void DecodeDataNeededByPortableTransforms() {
  for (i = 0; i < att_dec_num_attributes.back(); ++i) {
    curr_att = i;
    if (seq_att_dec_decoder_type[curr_att_dec][curr_att] ==
        SEQUENTIAL_ATTRIBUTE_ENCODER_NORMALS) {
      ParseQuantizationBits();
    } else if (seq_att_dec_decoder_type[curr_att_dec][curr_att] ==
               SEQUENTIAL_ATTRIBUTE_ENCODER_QUANTIZATION) {
      ParseQuantizationData();
    }
  }
}
~~~~~
{:.draco-syntax}


### ParseWrapTransformData()

~~~~~
void ParseWrapTransformData() {
  pred_trasnform_wrap_min[curr_att_dec][curr_att]                                     I32
  pred_trasnform_wrap_max[curr_att_dec][curr_att]                                     I32
}
~~~~~
{:.draco-syntax}


### ParseNormalOctahedronCanonicalizedTransformData()

~~~~~
void ParseNormalOctahedronCanonicalizedTransformData() {
  pred_trasnform_normal_max_q_val[curr_att_dec][curr_att]                             I32
  unused_center_value                                                                 I32
}
~~~~~
{:.draco-syntax}


### DecodeTransformData()

~~~~~
void DecodeTransformData() {
  transform_type = seq_att_dec_prediction_transform_type[curr_att_dec][curr_att];
  if (transform_type == PREDICTION_TRANSFORM_WRAP) {
    ParseWrapTransformData();
  } else if (transform_type ==
             PREDICTION_TRANSFORM_NORMAL_OCTAHEDRON_CANONICALIZED) {
    ParseNormalOctahedronCanonicalizedTransformData();
  }
}
~~~~~
{:.draco-syntax}


### ParsePredictionRansData()

~~~~~
void ParsePredictionRansData() {
  prediction_rans_prob_zero                                                           UI8
  sz = prediction_rans_data_size                                                      varUI32
  prediction_rans_data_buffer                                                         UI8[sz]
}
~~~~~
{:.draco-syntax}


### ParseConstrainedMultiNumFlags()

~~~~~
void ParseConstrainedMultiNumFlags() {
  constrained_multi_num_flags                                                         varUI32
}
~~~~~
{:.draco-syntax}


### DecodePredictionData_ConstrainedMulti()

~~~~~
void DecodePredictionData_ConstrainedMulti() {
  for (i = 0; i < kMaxNumParallelograms; ++i) {
    ParseConstrainedMultiNumFlags();
    if (constrained_multi_num_flags > 0) {
      ParsePredictionRansData();
      RansInitDecoder(ans_decoder_, prediction_rans_data_buffer,
                      prediction_rans_data_size, L_RANS_BASE);
      for (j = 0; j < constrained_multi_num_flags; ++j) {
        RabsDescRead(ans_decoder_, prediction_rans_prob_zero, &val);
        is_crease_edge_[i][j] = val > 0;
      }
    }
  }
  pred_cons_multi_is_cease_edge[curr_att_dec][curr_att] = is_crease_edge_;
}
~~~~~
{:.draco-syntax}


### ParseTexCoordsNumOrientations()

~~~~~
void ParseTexCoordsNumOrientations() {
  tex_coords_num_orientations                                                         UI32
}
~~~~~
{:.draco-syntax}


### DecodePredictionData_TexCoords()

~~~~~
void DecodePredictionData_TexCoords() {
  ParseTexCoordsNumOrientations();
  ParsePredictionRansData();
  RansInitDecoder(ans_decoder_, prediction_rans_data_buffer,
                  prediction_rans_data_size, L_RANS_BASE);
  last_orientation = true;
  for (i = 0; i < tex_coords_num_orientations; ++i) {
    RabsDescRead(ans_decoder_, prediction_rans_prob_zero, &val);
    if (val == 0)
      last_orientation = !last_orientation;
    orientations.push_back(last_orientation);
  }
  pred_tex_coords_orientations[curr_att_dec][curr_att] = orientations;
}
~~~~~
{:.draco-syntax}


### DecodePredictionData_GeometricNormal()

~~~~~
void DecodePredictionData_GeometricNormal() {
  ParsePredictionRansData();
  RansInitDecoder(ans_decoder_, prediction_rans_data_buffer,
                  prediction_rans_data_size, L_RANS_BASE);
  num_values = att_dec_num_values_to_decode[curr_att_dec][curr_att];
  for (i = 0; i < num_values; ++i) {
    RabsDescRead(ans_decoder_, prediction_rans_prob_zero, &val);
    flip_normal_bits.push_back(val > 0);
  }
  pred_transform_normal_flip_normal_bits[curr_att_dec][curr_att] = flip_normal_bits;
}
~~~~~
{:.draco-syntax}


### DecodePredictionData()

~~~~~
void DecodePredictionData(method) {
  if (method == MESH_PREDICTION_CONSTRAINED_MULTI_PARALLELOGRAM) {
    DecodePredictionData_ConstrainedMulti();
  } else if (method == MESH_PREDICTION_TEX_COORDS_PORTABLE) {
    DecodePredictionData_TexCoords();
  } else if (method == MESH_PREDICTION_GEOMETRIC_NORMAL) {
    DecodeTransformData();
    DecodePredictionData_GeometricNormal();
  }
  if (method != MESH_PREDICTION_GEOMETRIC_NORMAL) {
    DecodeTransformData();
  }
}
~~~~~
{:.draco-syntax}


### PredictionSchemeTransform_ComputeOriginalValue()

~~~~~
void PredictionSchemeTransform_ComputeOriginalValue(pred_vals, corr_vals,
                                                    out_orig_vals) {
  transform_type = eq_att_dec_prediction_transform_type[curr_att_dec][curr_att];
  if (transform_type == PREDICTION_TRANSFORM_WRAP) {
    PredictionSchemeWrapDecodingTransform_ComputeOriginalValue(
        pred_vals, corr_vals, out_orig_vals);
  } else if (transform_type ==
             PREDICTION_TRANSFORM_NORMAL_OCTAHEDRON_CANONICALIZED) {
    PredictionSchemeNormalOctahedronCanonicalizedDecodingTransform_ComputeOriginalValue(
        pred_vals, corr_vals, out_orig_vals);
  }
}
~~~~~
{:.draco-syntax}


### PredictionSchemeDifference_ComputeOriginalValues()

~~~~~
void PredictionSchemeDifference_ComputeOriginalValues(num_values) {
  num_components = GetNumComponents();
  signed_values = seq_int_att_dec_symbols_to_signed_ints[curr_att_dec][curr_att];
  size = num_components * num_values;
  zero_vals.assign(num_components, 0);
  out_values = signed_values;
  PredictionSchemeTransform_ComputeOriginalValue(
      &zero_vals[0], &signed_values[0], &out_values[0]);
  for (i = num_components; i < size; i += num_components) {
    PredictionSchemeTransform_ComputeOriginalValue(
        &out_values[i - num_components], &signed_values[i], &out_values[i]);
  }
  seq_int_att_dec_original_values[curr_att_dec][curr_att] = out_values;
}
~~~~~
{:.draco-syntax}


### PredictionScheme_ComputeOriginalValues()

~~~~~
void PredictionScheme_ComputeOriginalValues(method, num_values) {
  if (method == PREDICTION_DIFFERENCE) {
    PredictionSchemeDifference_ComputeOriginalValues(num_values);
  } else if (method == MESH_PREDICTION_PARALLELOGRAM) {
    MeshPredictionSchemeParallelogramDecoder_ComputeOriginalValues(num_values);
  } else if (method == MESH_PREDICTION_CONSTRAINED_MULTI_PARALLELOGRAM) {
    MeshPredictionSchemeConstrainedMultiParallelogramDecoder_ComputeOriginalValues(
        num_values);
  } else if (method == MESH_PREDICTION_TEX_COORDS_PORTABLE) {
    MeshPredictionSchemeTexCoordsPortableDecoder_ComputeOriginalValues(num_values);
  } else if (method == MESH_PREDICTION_GEOMETRIC_NORMAL) {
    MeshPredictionSchemeGeometricNormalDecoder_ComputeOriginalValues(num_values);
  }
}
~~~~~
{:.draco-syntax}
