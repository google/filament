# Filament Tools Gradle Plugin

## About

The **Filament Tools Gradle Plugin** helps integrate Filament into your Android project. It
automates the use of Filament's command-line tools (`matc`, `cmgen`, and `filamesh`).

This plugin handles:
- **Material Compilation**: Compiles `.mat` material definitions into `.filamat` binaries.
- **IBL Generation**: Generates Image-Based Lighting assets from HDR environment maps.
- **Mesh Compilation**: Converts models into Filament's efficient `.filamesh` binary format. *Note:
  This tool is no longer recommended; instead, use glTF and Filament's `gltfio` library for model
  loading.*

The plugin hooks directly into the Android build lifecycle (via `preBuild`) and supports incremental
builds, so assets are only recompiled when source files change.

## Usage

Apply the plugin in your module's `build.gradle` file and configure the `filament` block. You can
specify inputs and outputs for any combination of materials, IBLs, or meshes. If a path is not
configured, the corresponding task will be disabled.

### Example Configuration

```groovy
plugins {
    id 'filament-plugin'
}

filament {
    materialInputDir = project.layout.projectDirectory.dir("src/main/materials")
    materialOutputDir = project.layout.projectDirectory.dir("src/main/assets/materials")

    iblInputFile = project.layout.projectDirectory.file("path/to/environment.hdr")
    iblOutputDir = project.layout.projectDirectory.dir("src/main/assets/envs")

    meshInputFile = project.layout.projectDirectory.file("path/to/model.obj")
    meshOutputDir = project.layout.projectDirectory.dir("src/main/assets/models")
}
```

### Configuration Details

- **materialInputDir**: The directory containing your source material definitions (`.mat` files).
- **materialOutputDir**: The directory where the compiled material files (`.filamat`) will be
  generated.
- **iblInputFile**: The source high-dynamic-range image file (e.g., `.hdr` or `.exr`) used to
  generate Image Based Lighting assets.
- **iblOutputDir**: The directory where the generated IBL assets (typically `.ktx` files) will be
  placed.
- **meshInputFile**: The source mesh file (e.g., `.obj`) to be compiled.
- **meshOutputDir**: The directory where the compiled mesh file (`.filamesh`) will be generated.

Automatically adds tasks to your Android build to compile materials, generate an IBL, and compile a
mesh. The plugin hooks into `preBuild` to ensure assets are generated before the application is
built.

### Configuration Flags

You can control specific compilation options using Gradle properties (e.g., in `gradle.properties`
or via command line `-P`).

- **`com.google.android.filament.exclude-vulkan`**  When set to `true`, the Vulkan backend is
  excluded from the compiled materials. This can be useful to reduce the size of the generated
  assets if your application does not target Vulkan.  *Default: `false` (Vulkan is included)*

- **`com.google.android.filament.include-webgpu`**  When set to `true`, the WebGPU backend is
  included in the compiled materials. Use this if you intend to use the materials in a context
  supporting WebGPU.  *Default: `false`*

## Tools Configuration

The Filament Tools plugin requires some binary tools to be available: `matc`, `cmgen`, and
`filamesh`.

There are three ways to configure Filament tools:

1. **Point to a local path directly**

```groovy
filament {
    matc {
        path = "/path/to/matc"
    }

    cmgen {
        path = "/path/to/cmgen"
    }

    filamesh {
        path = "/path/to/filamesh"
    }
}
```

2. **Point to a Maven artifact**

```groovy
filament {
    matc {
        // The minor version (the middle number) must match the Filament dependency's.
        artifact = 'com.google.android.filament:matc:1.68.5'
    }

    cmgen {
        artifact = 'com.google.android.filament:cmgen:1.68.5'
    }
}
```

*Note that the `filamesh` artifact is not hosted on Maven Central, so it must be provided locally or
via another mechanism if needed.*

Gradle will automatically handle downloading the tool appropriate for your machine (MacOS/Linux/Windows) from Maven.

3. **Set the `com.google.android.filament.tools-dir` Gradle property**

This will override any other configuration. Gradle will attempt to locate the tools under
`<tools-dir>/bin` (e.g., `.../bin/matc`, `.../bin/cmgen`, `.../bin/filamesh`).
