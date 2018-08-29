To update tinyexr grab the latest from [GitHub](https://github.com/syoyo/tinyexr) and remove
the following directories to save some disk space:

- `examples`
- `jni`
- `test`

Make sure the LICENSE file remains and is up to date.

We also made a small change to `LoadEXRFromMemory` by adding this right after it calls
`LoadEXRImageFromMemory`:

    // This utility function does not yet support tiled images.
    if (exr_image.tiles) {
        ret = TINYEXR_ERROR_UNSUPPORTED_FORMAT;
        tinyexr::SetErrorMessage("Tiled EXR images are not yet supported", err);
        return ret;
    }
