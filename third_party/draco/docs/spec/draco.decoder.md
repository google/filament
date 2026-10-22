## Draco Decoder

### Decode()

~~~~~
void Decode() {
  ParseHeader();
  if (flags & METADATA_FLAG_MASK)
    DecodeMetadata();
  DecodeConnectivityData();
  DecodeAttributeData();
}
~~~~~
{:.draco-syntax}


### ParseHeader()

~~~~~
ParseHeader() {
  draco_string                                                                        UI8[5]
  major_version                                                                       UI8
  minor_version                                                                       UI8
  encoder_type                                                                        UI8
  encoder_method                                                                      UI8
  flags                                                                               UI16
}
~~~~~
{:.draco-syntax}

