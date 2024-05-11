
## Sequential Integer Attribute Decoder

### ConvertSymbolToSignedInt()

~~~~~
int ConvertSymbolToSignedInt(val) {
  is_positive = !(val & 1);
  val >>= 1;
  if (is_positive) {
    return val;
  }
  val = -val - 1;
  return val;
}
~~~~~
{:.draco-syntax }


### ConvertSymbolsToSignedInts()

~~~~~
void ConvertSymbolsToSignedInts() {
  decoded_symbols = seq_int_att_dec_decoded_values[curr_att_dec][curr_att];
  for (i = 0; i < decoded_symbols.size(); ++i) {
    val = ConvertSymbolToSignedInt(decoded_symbols[i]);
    seq_int_att_dec_symbols_to_signed_ints[i] = val;
  }
}
~~~~~
{:.draco-syntax }


### SequentialIntegerAttributeDecoder_DecodeIntegerValues()

~~~~~
void SequentialIntegerAttributeDecoder_DecodeIntegerValues() {
  num_components = GetNumComponents();
  num_entries = att_dec_num_values_to_decode[curr_att_dec][curr_att];
  num_values = num_entries * num_components;
  if (seq_int_att_dec_compressed[curr_att_dec][curr_att] > 0) {
    DecodeSymbols(num_values, num_components, &decoded_symbols);
  }
  seq_int_att_dec_decoded_values[curr_att_dec][curr_att] = decoded_symbols;
  if (num_values > 0) {
    if (seq_att_dec_prediction_transform_type[curr_att_dec][curr_att] ==
          PREDICTION_TRANSFORM_NORMAL_OCTAHEDRON_CANONICALIZED) {
      decoded_symbols = seq_int_att_dec_decoded_values[curr_att_dec][curr_att];
      for (i = 0; i < decoded_symbols.size(); ++i) {
        signed_vals[i] = decoded_symbols[i];
      }
      seq_int_att_dec_symbols_to_signed_ints[curr_att_dec][curr_att] = signed_vals;
    } else {
      ConvertSymbolsToSignedInts();
    }
  }
  if (seq_att_dec_prediction_scheme[curr_att_dec][curr_att] != PREDICTION_NONE) {
    DecodePredictionData(seq_att_dec_prediction_scheme[curr_att_dec][curr_att]);
    PredictionScheme_ComputeOriginalValues(
        seq_att_dec_prediction_scheme[curr_att_dec][curr_att], num_entries);
  }
}
~~~~~
{:.draco-syntax }
