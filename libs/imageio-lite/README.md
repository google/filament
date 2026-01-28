# imageio-lite

`imageio-lite` is a lightweight image input/output library for Filament.

## Design Philosophy

*   **No Third-Party Dependencies:** Unlike `imageio`, this library must NOT depend on external libraries like `libpng`, `tinyexr`, etc. All image encoding/decoding logic must be implemented within this library or rely only on Filament's core libraries (`utils`, `math`, `image`).
*   **Lite:** It is intended for basic import/export tasks where dragging in full-blown image libraries is undesirable (e.g., to keep binary size small or build simple).

## Supported Formats

*   **TIFF:** Read and Write support.
