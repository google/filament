
## Rans Decoding

### DecodeSymbols()

~~~~~
void DecodeSymbols(num_symbols, num_components, out_values) {
  scheme                                                                              UI8
  if (scheme == TAGGED_SYMBOLS) {
    DecodeTaggedSymbols(num_symbols, num_components, out_values);
  } else if (scheme == RAW_SYMBOLS) {
    DecodeRawSymbols(num_symbols, out_values);
  }
}
~~~~~
{:.draco-syntax }


### DecodeTaggedSymbols

~~~~~
void DecodeTaggedSymbols(num_values, num_components, out_values) {
  num_symbols_                                                                        varUI32
  BuildSymbolTables(num_symbols_, lut_table_, probability_table_);
  size                                                                                varUI64
  encoded_data                                                                        UI8[size]
  RansInitDecoder(ans_decoder_, &encoded_data[0], size, TAGGED_RANS_BASE);
  for (i = 0; i < num_values; i += num_components) {
    RansRead(ans_decoder_, TAGGED_RANS_BASE, TAGGED_RANS_PRECISION,
             lut_table_, probability_table_, &size);
    for (j = 0; j < num_components; ++j) {
      val                                                                             f[size]
      out_values.push_back(val);
    }
    ResetBitReader();
  }
}
~~~~~
{:.draco-syntax }


### DecodeRawSymbols

~~~~~
void DecodeRawSymbols(num_values, out_values) {
  max_bit_length                                                                      UI8
  num_symbols_                                                                        varUI32
  rans_precision_bits  = (3 * max_bit_length) / 2;
  if (rans_precision_bits > 20)
    rans_precision_bits = 20;
  if (rans_precision_bits < 12)
    rans_precision_bits = 12;
  rans_precision = 1 << rans_precision_bits;
  l_rans_base = rans_precision * 4;
  BuildSymbolTables(num_symbols_, lut_table_, probability_table_);
  size                                                                                varUI64
  buffer                                                                              UI8[size]
  RansInitDecoder(ans_decoder_, &buffer[0], size, l_rans_base);
  for (i = 0; i < num_values; ++i) {
    RansRead(ans_decoder_, l_rans_base, rans_precision,
              lut_table_, probability_table_, &val);
    out_values.push_back(val);
  }
}
~~~~~
{:.draco-syntax }


### BuildSymbolTables

~~~~~
void BuildSymbolTables(num_symbols_, lut_table_, probability_table_) {
  for (i = 0; i < num_symbols_; ++i) {
    // Decode the first byte and extract the number of extra bytes we need to
    // get, or the offset to the next symbol with non-zero probability.
    prob_data                                                                         UI8
    token = prob_data & 3;
    if (token == 3) {
      offset = prob_data >> 2;
      for (j = 0; j < offset + 1; ++j) {
        token_probs[i + j] = 0;
      }
      i += offset;
    } else {
      prob = prob_data >> 2;
      for (j = 0; j < token; ++j) {
        eb                                                                            UI8
        prob = prob | (eb << (8 * (j + 1) - 2));
      }
      token_probs[i] = prob;
    }
  }
  rans_build_look_up_table(&token_probs[0], num_symbols_, lut_table_,
                           probability_table_);
}
~~~~~
{:.draco-syntax }


### rans_build_look_up_table

~~~~~
void rans_build_look_up_table(
    token_probs[], num_symbols, lut_table_, probability_table_) {
  cum_prob = 0;
  act_prob = 0;
  for (i = 0; i < num_symbols; ++i) {
    probability_table_[i].prob = token_probs[i];
    probability_table_[i].cum_prob = cum_prob;
    cum_prob += token_probs[i];
    for (j = act_prob; j < cum_prob; ++j) {
      lut_table_[j] = i;
    }
    act_prob = cum_prob;
  }
}
~~~~~
{:.draco-syntax }


### RansInitDecoder

~~~~~
void RansInitDecoder(ans, buf, offset, l_rans_base) {
  ans.buf = buf;
  x = buf[offset - 1] >> 6;
  if (x == 0) {
    ans.buf_offset = offset - 1;
    ans.state = buf[offset - 1] & 0x3F;
  } else if (x == 1) {
    ans.buf_offset = offset - 2;
    ans.state = mem_get_le16(buf + offset - 2) & 0x3FFF;
  } else if (x == 2) {
    ans.buf_offset = offset - 3;
    ans.state = mem_get_le24(buf + offset - 3) & 0x3FFFFF;
  } else if (x == 3) {
    ans.buf_offset = offset - 4;
    ans.state = mem_get_le32(buf + offset - 4) & 0x3FFFFFFF;
  }
  ans.state += l_rans_base;
}
~~~~~
{:.draco-syntax }


### RansRead

~~~~~
void RansRead(ans, l_rans_base, rans_precision,
              lut_table_, probability_table_, val) {
  while (ans.state < l_rans_base && ans.buf_offset > 0) {
    ans.state = ans.state * IO_BASE + ans.buf[--ans.buf_offset];
  }
  quo = ans.state / rans_precision;
  rem = ans.state % rans_precision;
  fetch_sym(&sym, rem, lut_table_, probability_table_);
  ans.state = quo * sym.prob + rem - sym.cum_prob;
  val = sym.val;
}
~~~~~
{:.draco-syntax }


### fetch_sym

~~~~~
void fetch_sym(sym, rem, lut_table_, probability_table_) {
  symbol = lut_table_[rem];
  sym.val = symbol;
  sym.prob = probability_table_[symbol].prob;
  sym.cum_prob = probability_table_[symbol].cum_prob;
}
~~~~~
{:.draco-syntax }


### RabsDescRead

~~~~~
void RabsDescRead(ans, p0, out_val) {
  p = rabs_ans_p8_precision - p0;
  if (ans.state < rabs_l_base) {
    ans.state = ans.state * IO_BASE + ans.buf[--ans.buf_offset];
  }
  x = ans.state;
  quot = x / rabs_ans_p8_precision;
  rem = x % rabs_ans_p8_precision;
  xn = quot * p;
  val = rem < p;
  if (val) {
    ans.state = xn + rem;
  } else {
    ans.state = x - xn - p;
  }
  out_val = val;
}
~~~~~
{:.draco-syntax }
