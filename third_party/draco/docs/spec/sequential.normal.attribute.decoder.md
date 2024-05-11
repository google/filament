
## Sequential Normal Attribute Decoder

### MostSignificantBit()

~~~~~
int MostSignificantBit(n) {
  msb = -1;
  while (n != 0) {
    msb++;
    n >>= 1;
  }
  return msb;
}
~~~~~
{:.draco-syntax }


### OctaherdalCoordsToUnitVector()

~~~~~
void OctaherdalCoordsToUnitVector(in_s, in_t, out_vector) {
  s = in_s;
  t = in_t;
  spt = s + t;
  smt = s - t;
  x_sign = 1.0;
  if (spt >= 0.5 && spt <= 1.5 && smt >= -0.5 && smt <= 0.5) {
    // Right hemisphere. Don't do anything.
  } else {
    x_sign = -1.0;
    if (spt <= 0.5) {
      s = 0.5 - in_t;
      t = 0.5 - in_s;
    } else if (spt >= 1.5) {
      s = 1.5 - in_t;
      t = 1.5 - in_s;
    } else if (smt <= -0.5) {
      s = in_t - 0.5;
      t = in_s + 0.5;
    } else {
      s = in_t + 0.5;
      t = in_s - 0.5;
    }
    spt = s + t;
    smt = s - t;
  }
  y = 2.0 * s - 1.0;
  z = 2.0 * t - 1.0;
  x = Min(Min(2.0 * spt - 1.0, 3.0 - 2.0 * spt),
      Min(2.0 * smt + 1.0, 1.0 - 2.0 * smt)) * x_sign;
  normSquared = x * x + y * y + z * z;
  if (normSquared < 1e-6) {
    out_vector[0] = 0;
    out_vector[1] = 0;
    out_vector[2] = 0;
  } else {
    const float d = 1.0 / std::sqrt(normSquared);
    out_vector[0] = x * d;
    out_vector[1] = y * d;
    out_vector[2] = z * d;
  }
}
~~~~~
{:.draco-syntax }


### QuantizedOctaherdalCoordsToUnitVector()

~~~~~
void QuantizedOctaherdalCoordsToUnitVector(in_s, in_t, out_vector) {
  encoded_max_quantized_value =
      pred_trasnform_normal_max_q_val[curr_att_dec][curr_att];
  quantization_bits_ = MostSignificantBit(encoded_max_quantized_value) + 1;
  max_quantized_value_ = (1 << quantization_bits_) - 1;
  max_value_ = max_quantized_value_ - 1;
  scale = 1.0 / max_value_;
  OctaherdalCoordsToUnitVector(in_s * scale, in_t * scale, out_vector);
}
~~~~~
{:.draco-syntax }


### TransformAttributesToOriginalFormat_Normal()

~~~~~
void TransformAttributesToOriginalFormat_Normal() {
  quant_val_id = 0;
  portable_attribute_data = seq_int_att_dec_original_values[curr_att_dec][curr_att];
  num_points = att_dec_num_values_to_decode[curr_att_dec][curr_att];
  for (i = 0; i < num_points; ++i) {
    s = portable_attribute_data[quant_val_id++];
    t = portable_attribute_data[quant_val_id++];
    QuantizedOctaherdalCoordsToUnitVector(s, t, att_val);
    for (j = 0; j < 3; ++j) {
      normals.push_back(att_val[j]);
    }
  }
  seq_int_att_dec_dequantized_values[curr_att_dec][curr_att] = normals;
}
~~~~~
{:.draco-syntax }

