# Viewer Library

The **Viewer Library** (`libs/viewer`) provides a high-level abstraction for configuring and rendering Filament scenes. It is used by tools like `gltf_viewer` to load assets, manage settings, and drive the rendering loop.

## Features

- **Settings Management**: Centralized configuration for View, Camera, Lights, and Materials via the `Settings` struct.
- **JSON Serialization**: Full support for loading and saving settings via JSON.
- **Automation**: `AutomationEngine` allows scripting the viewer with a sequence of JSON-based test cases (batch mode).
- **GUI Integration**: Built-in support for `imgui` via `ViewerGui` and `Settings` binding.

## JSON Settings Schema

The viewer settings can be configured using a JSON object. This is used for `gltf_viewer --settings` or in automation specs.

### Root Object

The root object contains the following categories:

| Key | Type | Description |
| :--- | :--- | :--- |
| `view` | Object | Post-processing and rendering quality settings. |
| `camera` | Object | **[NEW]** Explicit camera control (pose, projection, exposure). |
| `lighting` | Object | **[NEW]** Environment and dynamic light settings. |
| `viewer` | Object | Global viewer options (skybox, background, scaling). |
| `animation` | Object | **[NEW]** Animation playback control. |
| `material` | Object | Material overrides. |

---

### Camera Settings (`camera`)

Allows explicit control over the camera. If `enabled` is false, the viewer uses its default orbit camera logic (auto-scaling/centering).

```json
"camera": {
    "enabled": true,                  // Must be true to use these explicit settings
    "projection": "PERSPECTIVE",      // "PERSPECTIVE" or "ORTHO"
    "center": [0, 0, 0],              // World-space look-at point
    "lookAt": [0, 0, -1],             // World-space eye position (confusingly named 'lookAt' in internal legacy, often 'eye')
    "up": [0, 1, 0],                  // Up vector
    "near": 0.1,                      // Near plane
    "far": 100.0,                     // Far plane
    "focalLength": 28.0,              // Focal length in mm (Perspective only)
    "fov": 0.0,                       // Field of view in degrees (overrides focalLength if > 0)
    "aperture": 16.0,                 // f-stop
    "shutterSpeed": 125.0,            // 1/seconds
    "sensitivity": 100.0,             // ISO
    "focusDistance": 10.0,            // Focus distance in world units
    "scaling": [1.0, 1.0],            // Custom projection matrix scaling (mostly for Ortho)
    "shift": [0.0, 0.0]               // Custom projection matrix shift
}
```

### Lighting Settings (`lighting`)

Controls the Image Based Lighting (IBL), the Sun, and additional dynamic lights.

```json
"lighting": {
    "iblIntensity": 30000.0,
    "iblRotation": 0.0,               // Rotation in degrees
    "enableSunlight": true,
    "enableShadows": true,
    "sunlight": {                     // **[NEW]** Nested sunlight properties
        "intensity": 100000.0,
        "color": [0.98, 0.92, 0.89],
        "direction": [0.6, -1.0, -0.8],
        "sunHaloSize": 10.0,
        "sunHaloFalloff": 80.0,
        "sunAngularRadius": 1.9,
        "castShadows": true,
        "shadowOptions": {            // Per-light shadow options
             "mapSize": 1024,
             "shadowCascades": 1,
             "stable": false
        }
    },
    "lights": [                       // **[NEW]** Array of custom lights
        {
            "type": "POINT",          // "POINT", "SPOT", "FOCUSED_SPOT", "DIRECTIONAL", "SUN"
            "position": [0, 2, 0],
            "color": [1, 0, 0],
            "intensity": 5000.0,
            "falloff": 10.0,
            "castShadows": true,
            "shadowOptions": { "mapSize": 512 }
        },
        {
            "type": "SPOT",
            "position": [2, 5, 2],
            "direction": [0, -1, 0],
            "spotInner": 0.5,         // Inner cone angle (radians)
            "spotOuter": 0.8          // Outer cone angle (radians)
        }
    ]
}
```

### View Settings (`view`)

Standard Filament view settings.

```json
"view": {
    "postProcessingEnabled": true,
    "antiAliasing": "FXAA",           // "NONE", "FXAA"
    "msaa": {
        "enabled": true,
        "sampleCount": 4
    },
    "ssao": { "enabled": true, ... },
    "bloom": { "enabled": true, ... },
    "dof": { "enabled": false, ... },
    "vignette": { "enabled": false, ... },
    "colorGrading": {
        "toneMapping": "ACES_LEGACY", // "LINEAR", "ACES", "FILMIC", "PBR_NEUTRAL", etc.
        "exposure": 0.0,
        "gamma": [1.0, 1.0, 1.0]
    }
}
```

### Viewer Options (`viewer`)

General app-level settings.

```json
"viewer": {
    "skyboxEnabled": true,
    "backgroundColor": [0, 0, 0],     // Used if skybox is disabled
    "autoScaleEnabled": true,         // Fit model to unit cube
    "groundPlaneEnabled": false
}
```

### Animation Settings (`animation`)

Control glTF animation playback.

```json
"animation": {
    "enabled": true,
    "speed": 1.0,
    "time": -1.0                      // If >= 0, forces animation to this specific time (seconds)
}
```
