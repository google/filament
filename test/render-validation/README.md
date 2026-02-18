# Render Validation Sample & TUI

This project is an Android application for validating Filament render behavior on-device, primarily using bundled tests and goldens. It operates via `adb` intents to automate the generation, exporting, and running of test bundles.

## Automated Execution via ADB Intents

You can fully control the validation app directly from your host machine without touching the device screen. The app listens for specific intent extras when launched.

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

## Python Terminal UI (TUI) Dashboard

To make managing tests and results easier, a Python-based Textual TUI (`validation_app.py`) is provided in the `validation_tui` directory. It automatically polls the connected device using ADB, acts as a GUI for the intent commands above, and handles downloading/uploading `.zip` bundles to circumvent Android's scoped storage limits.

### Setup
1. Ensure you have Python 3 and `adb` installed and in your PATH.
2. Navigate to the TUI directory: `cd test/render-validation`
3. Create a virtual environment: `python3 -m venv venv`
4. Activate it: `source venv/bin/activate` (Mac/Linux) or `venv\Scripts\activate` (Windows)
5. Install requirements: `pip install -r requirements.txt` (Installs the `textual` framework)

### Usage
Start the dashboard by running:
```bash
python validation_app.py
```

### TUI Features
- **Auto-Polling Mechanism**: Syncs the file lists with your device every 2 seconds.
- **Generate Test/Result Buttons**: One-click execution of the `am start` intents.
- **Upload Local Test Bundle**: Automatically pushes a local `.zip` file from your PC to the correct directory on the Android device.
- **Per-File Actions**:
  - `▶` (Load): Restarts the app with `--es zip_path <filename>` to set it as the active test on device.
  - `↓` (Download): Pulls the `.zip` to your PC's current working directory.
  - `✎` (Rename): Quickly renames the file directly on the Android file system.
  - `✗` (Delete): Quickly removes the file from the Android device to free up storage.
