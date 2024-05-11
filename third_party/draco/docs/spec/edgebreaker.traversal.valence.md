
## EdgeBreaker Traversal Valence

### ParseValenceContextCounters()

~~~~~
void ParseValenceContextCounters(index) {
  ebv_context_counters[index]                                                         varUI32
}
~~~~~
{:.draco-syntax }



### EdgeBreakerTraversalValenceStart()

~~~~~
void EdgeBreakerTraversalValenceStart() {
  ParseEdgebreakerTraversalStandardFaceData()
  ParseEdgebreakerTraversalStandardAttributeConnectivityData()
  vertex_valences_.assign(num_encoded_vertices + num_encoded_split_symbols, 0);
  for (i = 0; i < NUM_UNIQUE_VALENCES; ++i) {
    ParseValenceContextCounters(i);
    if (ebv_context_counters[i] > 0) {
      DecodeSymbols(ebv_context_counters[i], 1, &ebv_context_symbols[i]);
    }
  }
}
~~~~~
{:.draco-syntax }



### EdgebreakerValenceDecodeSymbol()

~~~~~
void EdgebreakerValenceDecodeSymbol() {
  if (active_context_ != -1) {
    symbol_id = ebv_context_symbols[active_context_]
                                   [--ebv_context_counters[active_context_]];
    last_symbol_ = edge_breaker_symbol_to_topology_id[symbol_id];
  } else {
    last_symbol_ = TOPOLOGY_E;
  }
}
~~~~~
{:.draco-syntax }

