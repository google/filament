# Light Cookies Feature

## Overview
Light cookies have been added to Filament to support textured light projections for spot and point lights. This feature allows lights to project patterns or images, similar to gobos in theatrical lighting.

## API Usage

### Setting a Light Cookie
```cpp
#include <filament/LightManager.h>
#include <filament/Texture.h>

// Create a texture for the light cookie
filament::Texture* cookieTexture = filament::Texture::Builder()
    .width(512)
    .height(512)
    .format(Texture::InternalFormat::RGB8)
    .build(*engine);

// Create a spot light with a cookie
utils::Entity light = utils::EntityManager::get().create();
filament::LightManager::Builder(LightManager::Type::SPOT)
    .position({0, 5, 0})
    .direction({0, -1, 0})
    .intensity(100000)
    .spotLightCone(M_PI / 8, M_PI / 4)
    .lightCookie(cookieTexture)  // Set the cookie texture
    .build(*engine, light);
```

### Updating Light Cookie at Runtime
```cpp
auto& lightManager = engine->getLightManager();
auto instance = lightManager.getInstance(lightEntity);

// Change or remove the cookie
lightManager.setLightCookie(instance, newCookieTexture);
// or
lightManager.setLightCookie(instance, nullptr);  // Remove cookie
```

## Implementation Details

### Modified Files
1. **filament/include/filament/LightManager.h**
   - Added `lightCookie(Texture*)` method to Builder class
   - Added forward declaration for Texture class

2. **filament/src/components/LightManager.h**
   - Added `COOKIE_TEXTURE` field to component data structure
   - Added `setLightCookie()` and `getLightCookie()` methods
   - Expanded component storage to include Texture pointer

3. **filament/src/components/LightManager.cpp**
   - Implemented `lightCookie()` builder method
   - Implemented `setLightCookie()` setter method
   - Added cookie texture initialization in `create()`

4. **shaders/src/surface_light_punctual.fs**
   - Added TODO comment showing where cookie sampling will be implemented
   - Placeholder for texture sampling logic

## Supported Light Types
- ✅ **Spot Lights** (Type::SPOT, Type::FOCUSED_SPOT)
- ✅ **Point Lights** (Type::POINT)
- ❌ **Directional Lights** (Type::DIRECTIONAL, Type::SUN) - Not supported

## Technical Notes

### Texture Requirements
- **Format**: RGB or RGBA textures recommended
- **Size**: Power-of-two sizes (e.g., 256x256, 512x512, 1024x1024)
- **Type**: 2D textures (Sampler::SAMPLER_2D)
- **Filtering**: Bilinear or trilinear filtering recommended

### Shader Implementation (Future Work)
The shader infrastructure requires:
1. Texture binding in the render loop
2. Passing texture indices through the light UBO
3. UV coordinate computation:
   - **Spot Lights**: Project world position through light's view-projection matrix
   - **Point Lights**: Use normalized light direction to compute UV coordinates

Example shader pseudocode:
```glsl
// For spot lights
vec4 lightSpacePos = lightProjection * vec4(worldPosition, 1.0);
vec2 cookieUV = lightSpacePos.xy / lightSpacePos.w * 0.5 + 0.5;

// For point lights  
vec3 dir = normalize(worldPosition - lightPosition);
vec2 cookieUV = vec2(
    atan(dir.z, dir.x) / (2.0 * PI) + 0.5,
    asin(dir.y) / PI + 0.5
);

// Sample and modulate
vec3 cookieColor = texture(lightCookieTexture, cookieUV).rgb;
light.colorIntensity.rgb *= cookieColor;
```

## Performance Considerations
- Light cookies add texture sampling overhead per-light per-fragment
- Use mipmaps to improve cache coherency
- Smaller textures (256x256 or 512x512) are usually sufficient
- Consider light cookie LOD based on distance

## Future Enhancements
- [ ] Full shader implementation with texture binding
- [ ] Cubemap support for point lights (more accurate)
- [ ] Animated cookies (texture arrays or 3D textures)
- [ ] Cookie intensity scaling parameter
- [ ] Cookie rotation parameter

## Example Use Cases
1. **Window patterns** - Simulate light coming through windows
2. **Foliage shadows** - Project tree shadows from lights
3. **Gobo effects** - Stage lighting patterns
4. **Caustics** - Water or glass refraction patterns
5. **Atmospheric effects** - Dust motes, god rays

## Backward Compatibility
This feature is fully backward compatible:
- Lights without cookies work exactly as before
- Default cookie value is `nullptr` (no cookie)
- No performance impact when cookies are not used
