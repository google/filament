## Prediction Wrap Transform

### PredictionSchemeWrapTransformBase_ClampPredictedValue()

~~~~~
void PredictionSchemeWrapTransformBase_ClampPredictedValue(predicted_val,
                                                           clamped_value_) {
  num_components = GetNumComponents();
  min_value_ = pred_trasnform_wrap_min[curr_att_dec][curr_att];
  max_value_ = pred_trasnform_wrap_max[curr_att_dec][curr_att];
  for (i = 0; i < num_components; ++i) {
    if (predicted_val[i] > max_value_)
      clamped_value_[i] = max_value_;
    else if (predicted_val[i] < min_value_)
      clamped_value_[i] = min_value_;
    else
      clamped_value_[i] = predicted_val[i];
  }
}
~~~~~
{:.draco-syntax}


### PredictionSchemeWrapDecodingTransform_ComputeOriginalValue()

~~~~~
void PredictionSchemeWrapDecodingTransform_ComputeOriginalValue(
    predicted_vals, corr_vals, out_original_vals) {
  num_components = GetNumComponents();
  min = pred_trasnform_wrap_min[curr_att_dec][curr_att];
  max = pred_trasnform_wrap_max[curr_att_dec][curr_att];
  max_dif_ = 1 + max - min;
  PredictionSchemeWrapTransformBase_ClampPredictedValue(predicted_vals,
                                                        clamped_vals);
  for (i = 0; i < num_components; ++i) {
    out_original_vals[i] = clamped_vals[i] + corr_vals[i];
    if (out_original_vals[i] > max)
      out_original_vals[i] -= max_dif_;
    else if (out_original_vals[i] < min)
      out_original_vals[i] += max_dif_;
  }
}
~~~~~
{:.draco-syntax}
