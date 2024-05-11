
## Sequential Quantization Attribute Decoder

### ParseQuantizationBits()

~~~~~
void ParseQuantizationBits() {
  quantized_data_quantization_bits[curr_att_dec][curr_att]                            UI8
}
~~~~~
{:.draco-syntax }


### ParseQuantizationData()

~~~~~
void ParseQuantizationData() {
  num_components = GetNumComponents();
  for (j = 0; j < num_components; ++j) {
    quantized_data_min_values[curr_att_dec][curr_att][i]                              Float
  }
  quantized_data_max_value_df[curr_att_dec][curr_att]                                 Float
  ParseQuantizationBits();
}
~~~~~
{:.draco-syntax }


### DequantizeFloat()

~~~~~
float DequantizeFloat(val, max_quantized_value_factor_, range_) {
  neg = (val < 0);
  if (neg) {
    val = -val;
  }
  norm_value = val * max_quantized_value_factor_;
  if (neg)
    norm_value = -norm_value;
  return norm_value * range_;
}
~~~~~
{:.draco-syntax }


### SequentialQuantizationAttributeDecoder_DequantizeValues()

~~~~~
void SequentialQuantizationAttributeDecoder_DequantizeValues() {
  quantization_bits = quantized_data_quantization_bits[curr_att_dec][curr_att];
  max_quantized_value = (1 << (quantization_bits)) - 1;
  num_components = GetNumComponents();
  quant_val_id = 0;
  range_ = quantized_data_max_value_df[curr_att_dec][curr_att];
  max_quantized_value_factor_ = 1.f / max_quantized_value;
  min_value_ = quantized_data_min_values[curr_att_dec][curr_att];
  original_values = seq_int_att_dec_original_values[curr_att_dec][curr_att];
  num_values = att_dec_num_values_to_decode[curr_att_dec][curr_att];
  for (i = 0; i < num_values; ++i) {
    for (c = 0; c < num_components; ++c) {
      value = DequantizeFloat(original_values[quant_val_id++],
                              max_quantized_value_factor_, range_);
      value = value + min_value_[c];
      att_val[c] = value;
      dequantized_data.push_back(value);
    }
  }
  seq_int_att_dec_dequantized_values[curr_att_dec][curr_att] = dequantized_data;
}
~~~~~
{:.draco-syntax }
