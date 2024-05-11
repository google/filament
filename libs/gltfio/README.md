# Description

`gltfio` is a loader library that consumes `gltf` or `glb` content and produces Filament
objects. For usage details, see the docstring for `AssetLoader`.

gltfio has two plug-in interfaces, `TextureProvider` and `MaterialProvider`.  Filament ships with
several ready-to-go implementations described below.

- `MaterialProvider` creates Filament materials in response to certain glTF requirements.
    - [UbershaderProvider](#ubershaderloader) loads pre-built materials.
    - `JitShaderProvider` builds materials at run time using the `filamat` library.
- `TextureProvider` creates and populates Filament `Texture` objects.
    - `StbProvider` uses the STB library to read PNG and JPEG files.
    - `Ktx2Provider` uses the BasisU library to read KTX2 files.

# UbershaderProvider

`UbershaderProvider` is a ready-to-go implementation of the `MaterialProvider` interface that should
be used in applications that need fast startup times. There is no material compilation that
occurs at run time, but the shaders might be relatively large and complex.

At load time, the ubershader loader consumes an *ubershader archive* which is a precompiled set of
materials bundled with formal descriptions of the glTF features that they support.

The `uberz` command line tool consumes a list of `.spec` and `.filamat` files and produces a single
`.uberz` file. For details on these two file formats, see the README in `libs/uberz`.
