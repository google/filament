# RenderDiff Test Configuration Specification (`FORMAT.md`)

This document defines the schema for RenderDiff test suite configurations used across desktop Python runners ([`test/renderdiff/src/test_config.py`](test_config.py)) and the Android validation harness ([`android/samples/sample-render-validation`](../../android/samples/sample-render-validation)).

---

## 1. Top-Level Structure

A test configuration file is a JSON object with the following top-level fields:

| Field | Type | Required? | Description |
| :--- | :--- | :--- | :--- |
| `name` | `string` | **Yes** | Identifier for the test suite (e.g. `"presubmit"`, `"SampleSuite"`). |
| `renderers` / `backends` | `list[string]` | **Yes** | Target execution backends or platforms (e.g. `["desktop-opengl", "desktop-vulkan"]` on desktop, or `["opengl", "vulkan"]` on Android). |
| `presets` | `list[Preset]` | Optional | List of reusable configurations inherited via `apply_presets`. |
| `tests` | `list[Test]` | **Yes** | List of test definitions to execute. |

---

## 2. Presets (`presets[]`)

Presets allow sharing visual diff tolerances, glTF models/rendering options, or sample binary arguments.

| Field | Type | Required? | Description |
| :--- | :--- | :--- | :--- |
| `name` | `string` | **Yes** | Unique preset identifier. |
| `tolerance` | `dict` | Optional | Visual difference threshold specification (diffimg format). |
| `gltf_test` | `GltfTestBlock` | Optional | Reusable glTF model search paths, model lists, or rendering automation settings. |
| `sample_test` | `SampleTestBlock`| Optional | Reusable sample executable arguments, warmup frames, or animation timesteps. |

> **Note**: A preset may define `gltf_test`, `sample_test`, or neither (pure tolerance preset). It cannot define both.

---

## 3. Tests (`tests[]`)

Each test definition represents a test case and must define **exactly one** of `gltf_test` or `sample_test`.

| Field | Type | Required? | Description |
| :--- | :--- | :--- | :--- |
| `name` | `string` | **Yes** | Unique test name (used in output filenames and results). |
| `description` | `string` | Optional | Explanatory description of the test case. |
| `renderers` / `backends` | `list[string]` | Optional | Overrides suite-level renderers for this specific test. |
| `apply_presets` | `list[string]` | Optional | Ordered list of presets to inherit from. |
| `tolerance` | `dict` | Optional | Test-level diff tolerance (overrides preset tolerance). |
| `gltf_test` | `GltfTestBlock` | **One of** | Present if testing glTF rendering via `gltf_viewer`. |
| `sample_test` | `SampleTestBlock`| **One of** | Present if testing a standalone binary executable. |

---

## 4. `gltf_test` Block

Used when testing 3D model rendering via `gltf_viewer`.

| Field | Type | Required? | Description |
| :--- | :--- | :--- | :--- |
| `model_search_paths` | `list[string]` | Optional | Recursive search paths for `.glb` / `.gltf` models. Typically declared once in a base preset. |
| `models` | `list[string]` | Optional | List of model names to render (e.g. `["lucy", "FlightHelmet"]`). |
| `rendering` | `dict` | Optional | Filament AutomationSpec properties (lighting, camera, post-processing). |

---

## 5. `sample_test` Block

Used when testing standalone sample binaries (e.g. `hellotriangle`, `hellopbr`).

| Field | Type | Required? | Default | Description |
| :--- | :--- | :--- | :--- | :--- |
| `executable` | `string` | **Yes** | *None* | Name of binary executable in the samples build output. |
| `target` | `string` | Optional | `executable` | Identifier used in output `.tif` filenames. |
| `warmup_frames` | `int` | Optional | `10` | Frames to render before pixel readback. |
| `fixed_timestep`| `float` | Optional | `0.0166667` | Fixed virtual animation clock delta per frame in seconds (`1.0 / 60.0`). |
| `args` | `list[string]` | Optional | `[]` | Extra CLI arguments passed directly to the binary (e.g. `["--window-size", "512x512"]`). |

---

## 6. Example Configuration

See [`test/renderdiff/tests/sample.json`](tests/sample.json) for a comprehensive reference example.
