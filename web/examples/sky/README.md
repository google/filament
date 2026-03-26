# Analytic Skybox Sample

This sample demonstrates a **fully procedural, single-pass skybox shader** capable of simulating a dynamic day-night cycle, atmospheric scattering, volumetric clouds, and water reflections. 

It is designed for graphics engineers and technical artists who need a lightweight yet physically plausible environment background without relying on static HDRI textures.

## Features & Performance

The shader uses a "Uber-Shader" approach where all features are computed per-pixel. Features can be toggled or tuned via uniforms to balance quality vs performance.

| Feature | Cost | Control | Description |
| :--- | :---: | :--- | :--- |
| **Atmosphere** | 🟡 **Medium** | `turbidity`, `rayleigh`, `mie` | Analytic Rayleigh & Mie scattering. Physically based colors. |
| **Sun Disk** | 🟢 **Low** | `sunHalo` (Radius, Limb) | Analytic sphere intersection with limb darkening. Conservation of energy (Lux). |
| **Moon & Earthshine** | 🟢 **Low** | `sunHalo2`, `moonIntensity` | Resolved moon disk with geometric phases and dynamic Earthshine. |
| **Stars** | 🟢 **Low** | `starControl` (Density) | High-frequency procedural noise. Occluded by clouds/moon. |
| **Clouds** | 🔴 **High** | `cloudControl` (Coverage, Density) | 4-Octave 3D Fractal Brownian Motion (FBM). Dominates the cost when enabled. |
| **Heat Shimmer** | 🟡 **Medium** | `shimmerControl` | UV perturbation near the horizon to simulate mirages. |
| **Water Reflection** | 🟣 **Very High** | `waterControl` | **Renders the sky twice**. Includes procedural waves (FBM) and fresnel. |

> **Note**: Rendering water (`V.y < 0`) is significantly more expensive (~2.5x) than the sky because it requires re-evaluating the atmospheric scattering and cloud noise for the reflection vector.

## Shader Techniques

### 1. Analytic Atmospheric Scattering
Based on the **Hoffman & Preetham** model. It solves the single-scattering integral analytically for air molecules (Rayleigh) and aerosols (Mie).
- **Rayleigh**: Produces the deep blue sky and red sunset colors.
- **Mie**: Produces the white halo around the sun and general haziness.
- **Optimization**: Uses a simplified optical depth approximation ("Air Mass") to avoid expensive ray-marching.

### 2. Procedural Clouds (3D Noise)
Clouds are rendered as a spherical shell at a specific altitude.
- **Technique**: Ray-sphere intersection finds the entry point, then **3D FBM Noise** determines density.
- **Lighting**: Uses a "Silver Lining" approximation (strong forward scattering) and Beers-Lambert attenuation for dark underbellies.
- **Animation**: The noise coordinate logic helps simulate wind drift and shape evolution over time.

### 3. Infinite Water Ocean
When looking below the horizon, the shader switches to "Water Mode".
- **Geometry**: A flat plane at $y=0$.
- **Waves**: Generated using **Derivative-Based Noise** (or Finite Difference). This creates slope vectors that perturb the normal without needing actual geometry.
- **Reflection**: A ray is cast from the water surface back into the sky ($R = \text{reflect}(V, N)$). The sky function is called again with $R$ to get the reflected color.

### 4. Dynamic Tone Mapping
Applies a custom tone mapping curve that varies with Sun Elevation.
- **Noon**: Linear/Gamma (Standard).
- **Sunset**: Higher contrast curve to compress the dynamic range and enhance the rich sunset oranges/purples.

## Integration

To use this in your own Filament application:

1.  **Compile the Material**:
    Use `matc` to compile `simulated_skybox.mat` into a `.filamat` file.
    ```bash
    matc -p mobile -a opengl -o assets/simulated_skybox.filamat simulated_skybox.mat
    ```

2.  **Load in JavaScript/C++**:
    Create a Skybox entity and assign the material.

    ```javascript
    // JavaScript Example
    const material = engine.createMaterial('assets/simulated_skybox.filamat');
    const skybox = engine.createSkybox(material);
    scene.setSkybox(skybox);
    ```

3.  **Update Uniforms**:
    The shader requires specific uniforms (Sun Direction, Time, etc.) to be updated every frame. See `SimulatedSkybox.js` for a reference implementation of the uniform buffer management.

## References

*   **Hoffman & Preetham (2002)**: *"Real-time Light-Atmosphere Interactions"*
*   **Henyey & Greenstein (1941)**: *"Diffuse radiation in the galaxy"* (Mie Phase Function)
*   **Kasten & Young (1989)**: *"Revised optical air mass tables"*
*   **Three.js / Sky.js**: Empirical adjustments for "Golden Hour" aesthetics.
