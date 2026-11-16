# zbloat

This tool makes it easy to analyze the composition of Android applications that use Filament.

- Filament materials are shown in the treemap if `resgen --json` was used in the build.
- The input path can be a `.so` or `.a` file, a folder, or a zip archive (apk or aar).
- If the path is a zip or folder, interactively finds the file to analyze.
- The generated web report is a self-contained HTML file.
- Reports the gzipped size of all Filament materials.

**Note that the executable must be built with debugging information.**

For example, from a mac, you can generate a self-contained HTML file by typing this from the
Filament repo root:

    ./tools/zbloat/zbloat.py ./android/filament-android
    open index.html

This tool uses Python 3, `nm`, and `objdump`.

Many thanks to Evan Martin for his interactive treemap widget.
