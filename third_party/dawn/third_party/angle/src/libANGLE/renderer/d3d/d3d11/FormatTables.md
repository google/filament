# D3D11 back-end format tables

The D3D11 back-end uses a code generator to initialize D3D11 format info.
The generator is [`gen_texture_format_table.py`](gen_texture_format_table.py)
and uses data stored in [`texture_format_data.json`](texture_format_data.json)
and [`texture_format_map.json`](texture_format_map.json). The "format map"
maps from a GLES front-end format to the ANGLE format which represents the
format we use in D3D11. The "format data" indicates which DXGI formats we
use to represent an ANGLE format in the D3D11 back-end.

Note that the "format data" can also encode support for fallback formats.
The `supportTest` attribute indicates a runtime check we can use to determine
if we should use the `fallbackFormat` instead of the default format.
