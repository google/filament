# Web Samples and Tutorials

This directory contains the web-based samples, tutorials, and the JavaScript wrapper for Filament.

## Structure

- `examples/`
  Contains the source for the Web tutorials, HTML samples, materials, and assets.
  These examples use `filament.js` and `filament.wasm` to demonstrate various features of the engine.

- `filament-js/`
  Contains the JavaScript bindings for Filament, generated via Emscripten.

## Building the Web Examples

To build the Web targets and compile all required materials and assets, you will need the Emscripten SDK installed and activated.

1. **Activate Emscripten SDK:**
   Make sure you have `EMSDK` in your environment.
   ```bash
   cd path/to/emsdk
   ./emsdk activate latest
   source ./emsdk_env.sh
   ```

2. **Run the Build Script:**
   From the root directory of the repository, execute the build script targeting Web:
   ```bash
   ./build.sh -p wasm release
   ```

   This will:
   - Compile the C++ engine to `filament.wasm` and `filament.js`.
   - Build all materials (`.mat` to `.filamat`) and process textures required by the examples.
   - Output everything into `out/cmake-wasm-release/examples/`.

## Running the Examples

Because of CORS restrictions and the need to serve WebAssembly files with the correct MIME type, you must serve the files via a local web server.

1. **Install Python Requirements:**
   The serve script dynamically renders Markdown tutorials on-the-fly using `mistletoe`. It is highly recommended to use a Python virtual environment to install dependencies.
   ```bash
   python3 -m venv .venv
   source .venv/bin/activate
   pip install -r web/examples/requirements.txt
   ```

2. **Start the Server:**
   From the root of the repository, run the `serve.py` script:
   ```bash
   ./web/examples/serve.py
   ```

3. **View the Examples:**
   Open your browser and navigate to `http://localhost:8000`. You will see an index page listing all the generated tutorials and samples.

## Python Script: `serve.py`

The custom `serve.py` script in the `web/examples/` directory performs the following:
- Automatically detects the built files in the `out/` directory.
- Generates a main entry `index.html` listing all the available samples and tutorials.
- Handles server-side rendering of `.md` tutorial files into HTML using an embedded template.

## Porting a Web Sample to Official Docs (`docs_src`)

If you want your web sample or tutorial to appear on the official Filament documentation website via
`mdbook`, follow these steps:

1. **Map the Built HTML:**
   Add your generated `.html` or `.md` file to the mapping in `docs_src/build/duplicates.json`. This
   tells the documentation build script to copy your sample into the `mdbook` structure.
   ```json
   "out/cmake-wasm-release/web/examples/examples/your_sample/your_sample.html": {
       "dest": "samples/web/your_sample.md"
   }
   ```

2. **Add to the Navigation Menu:**
   Link your sample in the table of contents by adding it to `docs_src/src_mdbook/src/SUMMARY.md`
   under the "Web Tutorials" or "Web Samples" section.

3. **Generate a Thumbnail Image:**
   Add your sample's name to the `samples` array in `docs_src/build/snapshot_samples.py`. Then,
   manually run this script (`python3 snapshot_samples.py`). It will launch a headless browser,
   wait for your scene to render, and snap a 100x100 preview image.

4. **Dynamic Asset Loading (Optional):**
   When `mdbook` serves your sample, the assets (`.filamat`, textures) are segregated into a
   `web/assets/` directory.
   - For `<script src="...">` or `<img src="...">` tags embedded in the HTML, the paths will be
     automatically rewritten by `docs_src/build/copy_web_docs.py`.
   - However, if you load files dynamically within your JavaScript code (e.g., using `fetch()`), you
     **must** prepend `(window.FILAMENT_ASSET_DIR || '')` to the file URL.
   - If you need `window.FILAMENT_ASSET_DIR` to be properly populated, make sure to add logic to
     `docs_src/build/copy_web_docs.py` to inject the proper path prefix for your sample.
