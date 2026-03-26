# Web Samples and Tutorials

This directory contains the web-based samples, tutorials, and the JavaScript wrapper for Filament.

## Structure

- `examples/`
  Contains the source for the WebGL tutorials, HTML samples, materials, and assets.
  These examples use `filament.js` and `filament.wasm` to demonstrate various features of the engine.

- `filament-js/`
  Contains the JavaScript bindings for Filament, generated via Emscripten.

## Building the Web Examples

To build the WebGL targets and compile all required materials and assets, you will need the Emscripten SDK installed and activated.

1. **Activate Emscripten SDK:**
   Make sure you have `EMSDK` in your environment.
   ```bash
   cd path/to/emsdk
   ./emsdk activate latest
   source ./emsdk_env.sh
   ```

2. **Run the Build Script:**
   From the root directory of the repository, execute the build script targeting WebGL:
   ```bash
   ./build.sh -p webgl release
   ```

   This will:
   - Compile the C++ engine to `filament.wasm` and `filament.js`.
   - Build all materials (`.mat` to `.filamat`) and process textures required by the examples.
   - Output everything into `out/cmake-webgl-release/examples/`.

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
