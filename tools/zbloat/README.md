# zbloat

This tool makes it easy to analyze the composition of Android applications that use Filament.

- Filament materials are shown in the treemap if `resgen --json` was used in the build.
- The input path can be a so file, folder, or zip archive (apk or aar).
- If the path is a zip or folder, interactively finds the file to analyze.
- The generated web report is a self-contained HTML file.
- Reports the gzipped size of all Filament materials.

**Note that the executable must be built with debugging information.**

For example, from a mac, you can generate a self-contained HTML file by typing this from the
Filament repo root:

    ./tools/zbloat/zbloat.py ./android/filament-android
    open index.html

This tool uses Python, `nm`, and `objdump`.

### Linux, macOS, and Docker

The `nm` tool works slightly differently between macOS and Linux, so a `Dockerfile` is
provided that installs dependencies inside a Linux container. This is also convenient if you
do not have both versions of Python on your system.

The easy way to use docker is to invoke the helper bash script. Simply type `zbloat.sh [args...]`
instead of `zbloat.py [args...]`. The first time you run it, it will be slow but subsequent times
will be fast.

Many thanks to Evan Martin for his interactive treemap widget.
