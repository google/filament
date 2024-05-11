## Prediction Normal Transform

### ModMax()

~~~~~
int32_t ModMax(x, center_value_, max_quantized_value_) {
  if (x > center_value_)
    return x - max_quantized_value_;
  if (x < -center_value_)
    return x + max_quantized_value_;
  return x;
}
~~~~~
{:.draco-syntax}


### InvertDiamond()

~~~~~
void InvertDiamond(s, t, center_value_) {
  sign_s = 0;
  sign_t = 0;
  if (s >= 0 && t >= 0) {
    sign_s = 1;
    sign_t = 1;
  } else if (s <= 0 && t <= 0) {
    sign_s = -1;
    sign_t = -1;
  } else {
    sign_s = (s > 0) ? 1 : -1;
    sign_t = (t > 0) ? 1 : -1;
  }
  corner_point_s = sign_s * center_value_;
  corner_point_t = sign_t * center_value_;
  s = 2 * s - corner_point_s;
  t = 2 * t - corner_point_t;
  if (sign_s * sign_t >= 0) {
    temp = s;
    s = -t;
    t = -temp;
  } else {
    temp = s;
    s = t;
    t = temp;
  }
  s = (s + corner_point_s) / 2;
  t = (t + corner_point_t) / 2;
}
~~~~~
{:.draco-syntax}


### GetRotationCount()

~~~~~
void GetRotationCount(pred, count) {
  sign_x = pred[0];
  sign_y = pred[1];
  rotation_count = 0;
  if (sign_x == 0) {
    if (sign_y == 0) {
      rotation_count = 0;
    } else if (sign_y > 0) {
      rotation_count = 3;
    } else {
      rotation_count = 1;
    }
  } else if (sign_x > 0) {
    if (sign_y >= 0) {
      rotation_count = 2;
    } else {
      rotation_count = 1;
    }
  } else {
    if (sign_y <= 0) {
      rotation_count = 0;
    } else {
      rotation_count = 3;
    }
  }
  count = rotation_count;
}
~~~~~
{:.draco-syntax}


### RotatePoint()

~~~~~
void RotatePoint(p, rotation_count, out_p) {
  switch (rotation_count) {
    case 1:
      out_p.push_back(p[1]);
      out_p.push_back(-p[0]);
      return;
    case 2:
      out_p.push_back(-p[0]);
      out_p.push_back(-p[1]);
      return;
    case 3:
      out_p.push_back(-p[1]);
      out_p.push_back(p[0]);
      return;
    default:
      out_p.push_back(p[0]);
      out_p.push_back(p[1]);
      return;
  }
}
~~~~~
{:.draco-syntax}


### IsInBottomLeft()

~~~~~
bool IsInBottomLeft(p) {
  if (p[0] == 0 && p[1] == 0)
    return true;
  return (p[0] < 0 && p[1] <= 0);
}
~~~~~
{:.draco-syntax}


### PredictionSchemeNormalOctahedronCanonicalizedDecodingTransform_ComputeOriginalValue2()

~~~~~
void PredictionSchemeNormalOctahedronCanonicalizedDecodingTransform_ComputeOriginalValue2(
    pred_in, corr, out, center_value_, max_quantized_value_) {
  t.assign(2, center_value_);
  SubtractVectors(pred_in, t, &pred);
  pred_is_in_diamond = Abs(pred[0]) + Abs(pred[1]) <= center_value_;
  if (!pred_is_in_diamond) {
    InvertDiamond(&pred[0], &pred[1], center_value_);
  }
  pred_is_in_bottom_left = IsInBottomLeft(pred);
  GetRotationCount(pred, &rotation_count);
  if (!pred_is_in_bottom_left) {
    RotatePoint(pred, rotation_count, &temp_rot);
    for (i = 0; i < temp_rot.size(); ++i) {
      pred[i] = temp_rot[i];
    }
  }

  AddVectors(pred, corr, &orig);
  orig[0] = ModMax(orig[0], center_value_, max_quantized_value_);
  orig[1] = ModMax(orig[1], center_value_, max_quantized_value_);
  if (!pred_is_in_bottom_left) {
    reverse_rotation_count = (4 - rotation_count) % 4;
    RotatePoint(orig, reverse_rotation_count, &temp_rot);
    for (i = 0; i < temp_rot.size(); ++i) {
      orig[i] = temp_rot[i];
    }
  }
  if (!pred_is_in_diamond) {
    InvertDiamond(&orig[0], &orig[1], center_value_);
  }
  AddVectors(orig, t, out);
}
~~~~~
{:.draco-syntax}


### PredictionSchemeNormalOctahedronCanonicalizedDecodingTransform_ComputeOriginalValue()

~~~~~
void PredictionSchemeNormalOctahedronCanonicalizedDecodingTransform_ComputeOriginalValue(
    pred_vals, corr_vals, out_orig_vals) {
  encoded_max_quantized_value =
      pred_trasnform_normal_max_q_val[curr_att_dec][curr_att];
  quantization_bits_ = MostSignificantBit(encoded_max_quantized_value) + 1;
  max_quantized_value_ = (1 << quantization_bits_) - 1;
  max_value_ = max_quantized_value_ - 1;
  center_value_ = max_value_ / 2;

  pred.push_back(pred_vals[0]);
  pred.push_back(pred_vals[1]);
  corr.push_back(corr_vals[0]);
  corr.push_back(corr_vals[1]);
  PredictionSchemeNormalOctahedronCanonicalizedDecodingTransform_ComputeOriginalValue2(
      pred, corr, &orig, center_value_, max_quantized_value_);
  out_orig_vals[0] = orig[0];
  out_orig_vals[1] = orig[1];
}
~~~~~
{:.draco-syntax}
