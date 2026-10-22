
## Connectivity Decoder

### DecodeConnectivityData()

~~~~~
void DecodeConnectivityData() {
  if (encoder_method == MESH_SEQUENTIAL_ENCODING)
    DecodeSequentialConnectivityData();
  else if (encoder_method == MESH_EDGEBREAKER_ENCODING)
    DecodeEdgebreakerConnectivityData();
}

~~~~~
{:.draco-syntax }
