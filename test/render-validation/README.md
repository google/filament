# Render Validation Framework & Tools

This directory contains the tools necessary to automate, process, and analyze render validation tests for Filament on Android devices.

The workflow is supported by three main Python scripts:

1. **`validation_app.py`**: A Terminal User Interface (TUI) to orchestrate `adb` commands. It connects to physical devices to run tests, generate goldens, and retrieve result `.zip` bundles.
2. **`process_results.py`**: A processor that takes raw device result `.zip` files and compiles them into a static web application for easy visual comparison and diffing.
3. **`recalculate.py`**: A tuning utility that allows you to mathematically re-evaluate existing test results against new tolerance heuristics (e.g., blurring or shifting) without needing to re-run the tests on physical hardware.

---

## 1. Terminal User Interface (TUI)

The `validation_app.py` script provides a text-based interface to manage connected Android devices. It wraps the underlying ADB intents required to control the on-device validation application.

### Setup & Requirements
Install the TUI dependencies:
```bash
pip install -r requirements.txt
```

Run the application:
```bash
python validation_app.py
```

### Underlying ADB Intents

If you prefer to bypass the TUI, you can fully control the validation app directly from your host machine. The app listens for specific intent extras when launched.

**General Command Structure:**
```bash
adb shell am start -n com.google.android.filament.validation/.MainActivity <extras>
```

### Supported Intent Extras (Booleans)
- `--ez auto_run true`: Immediately runs the loaded or default test upon startup.
- `--ez generate_goldens true`: Forces the app to generate new golden reference images for the current test instead of comparing against existing ones.
- `--ez auto_export true`: Automatically packages the generated tests/goldens into a `.zip` archive (`Default_Test_<timestamp>.zip`) to the app's files directory when finished.
- `--ez auto_export_results true`: Automatically packages the comparison results and diff images into a `.zip` archive (`results_<timestamp>.zip`) to the app's files directory when finished.

### Supported Intent Extras (Strings)
- `--es zip_path <filename.zip>`: Loads a specific test `.zip` bundle.
  - If you pass an absolute path, it loads from there.
  - If you pass just a filename (e.g. `Default_Test_123.zip`), it intelligently searches the app's external files directory (`/sdcard/Android/data/.../files`) to find it.

### Example ADB Workflows
**Generate new goldens and export them as a test bundle:**
```bash
adb shell am start -n com.google.android.filament.validation/.MainActivity \
  --ez auto_run true \
  --ez generate_goldens true \
  --ez auto_export true
```

**Run an existing specific test bundle and export the results:**
```bash
adb shell am start -n com.google.android.filament.validation/.MainActivity \
  --es zip_path "Default_Test_123.zip" \
  --ez auto_run true \
  --ez auto_export_results true
```

### Manual File Management via ADB
Depending on the Android version and device storage policy, the app's file location resides either on standard External Storage or strictly within inside its Internal Sandbox.

**App's External Storage (Default for most devices):**
- **Push a test bundle to device:** `adb push <filename.zip> /sdcard/Android/data/com.google.android.filament.validation/files/`
- **Pull a test bundle from device:** `adb pull /sdcard/Android/data/com.google.android.filament.validation/files/<filename.zip> .`
- **Pull a result bundle from device:** `adb pull /sdcard/Android/data/com.google.android.filament.validation/files/results_<timestamp>.zip .`

**App's Internal Storage (If external is unavailable):**
*Note: Due to security restrictions, you must use `run-as` to pipe data into/out of the application's secure sandbox.*
- **Push a test bundle to device:**
  1. `adb push <filename.zip> /sdcard/Download/`
  2. `adb shell "run-as com.google.android.filament.validation cp /sdcard/Download/<filename.zip> files/"`
- **Pull a test or result bundle from device:**
  `adb shell "run-as com.google.android.filament.validation cat files/<filename.zip>" > <filename.zip>`

---

## 2. Web Results Viewer (`process_results.py`)

The project includes a static web viewer to visualize and compare test results across different devices. The viewer supports high-resolution image comparison with zoom/pan controls and dynamic diffing.

### Setup & Requirements
The results processor requires `numpy` and `Pillow`. These are not included in the main `requirements.txt` to keep the TUI dependencies minimal.

1. Install processing dependencies:
   ```bash
   pip install numpy Pillow
   ```

### Process Result Bundles
The `process_results.py` script takes a directory of `.zip` result files (exported from the Android app) and generates a static web folder.

```bash
# Usage: python process_results.py <input_zip_dir> <output_web_dir>
python process_results.py ./my_results ./web_output
```

This script:
- Extracts images and metadata from the result zips.
- Generates thumbnails for efficient browser performance.
- Packages the exact tolerance configurations for the web viewer.

### View Results
Because the viewer uses ES modules and fetches data, it must be served via a web server.

```bash
cd ./web_output
python3 -m http.server 1234
```

Navigate to `http://localhost:1234` in your desktop browser.

## 3. Tuning Tolerances (`recalculate.py`)
If you want to interactively tune the tolerance heuristics (e.g., adjust shift or blur radius) without re-running the tests on physical devices, you can use the `recalculate.py` script. This script applies a new set of tolerance parameters to the existing rendered and golden images, mathematically mirroring the C++ runtime's evaluation, and produces a new set of result zip files.

```bash
# Usage: python recalculate.py <input_zip_dir> <new_heuristics.json> <output_zip_dir>
python recalculate.py ./my_results new_heuristics.json ./tuned_results
```

- `<input_zip_dir>`: The directory containing the original per-device `.zip` result files.
- `<new_heuristics.json>`: A JSON file containing the updated tolerance parameters for the tests you want to adjust. It should follow the structure of the `tests` array in `default_test.json`. Tests not listed will automatically fall back to their original tolerances from the bundle.
- `<output_zip_dir>`: The directory where the updated `.zip` result files will be saved. These files have the exact same format as the input and can be directly passed into `process_results.py`.

### Web Viewer Features
- **Tabular Overview**: Compare results across multiple devices and test runs in a single grid.
- **High-Res Viewer**: Click any thumbnail to open a full-size modal.
  - **Zoom & Pan**: Use the mouse wheel to zoom and left-click-drag to pan around the render.
  - **Comparison Modes**: Cycle between "Rendered", "Golden", and "Diff" views.
- **Dynamic JS Diffing**: The `imagediff` algorithm (including `shiftRadius`, `blurRadius`, and complex tolerance trees) is implemented in JavaScript and computed on-the-fly.
- **Fail Highlighting**: Toggle "Highlight Failing Pixels" in the Diff view to see exactly which pixels exceeded the tolerance threshold in pure red.
- **Contrast Control**: Use the contrast slider to amplify subtle rendering differences.

