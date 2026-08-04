//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[LightManager](index.md)

# LightManager

[main]\
open class [LightManager](index.md)

LightManager allows you to create a light source in the scene, such as a sun or street lights. 

 At least one light must be added to a scene in order to see anything (unless the [UNLIT](../-material/-shading/-u-n-l-i-t/index.md) is used). 

# Creation and destruction

 A Light component is created using the [LightManager.Builder](-builder/index.md) and destroyed by calling [destroy](destroy.md). 

```kotlin
 Engine engine = Engine.create();
 int sun = EntityManager.get().create();

 LightManager.Builder(Type.SUN)
             .castShadows(true)
             .build(engine, sun);

 engine.getLightManager().destroy(sun);

```

# Light types

 Lights come in three flavors: 

- directional lights
- point lights
- spot lights

## Directional lights

 Directional lights have a direction, but don't have a position. All light rays are parallel and come from infinitely far away and from everywhere. Typically a directional light is used to simulate the sun. 

 Directional lights and spot lights are able to cast shadows. 

 To create a directional light use [DIRECTIONAL](-type/-d-i-r-e-c-t-i-o-n-a-l/index.md) or [SUN](-type/-s-u-n/index.md), both are similar, but the later also draws a sun's disk in the sky and its reflection on glossy objects. 

**warning:** Currently, only a single directional light is supported. If several directional lights are added to the scene, the dominant one will be used. 

## Point lights

 Unlike directional lights, point lights have a position but emit light in all directions. The intensity of the light diminishes with the inverse square of the distance to the light. [falloff](-builder/falloff.md) controls the distance beyond which the light has no more influence. 

 A scene can have multiple point lights. 

## Spot lights

 Spot lights are similar to point lights but the light they emit is limited to a cone defined by [spotLightCone](-builder/spot-light-cone.md) and the light's direction. 

 A spot light is therefore defined by a position, a direction and inner and outer cones. The spot light's influence is limited to inside the outer cone. The inner cone defines the light's falloff attenuation. 

 A physically correct spot light is a little difficult to use because changing the outer angle of the cone changes the illumination levels, as the same amount of light is spread over a changing volume. The coupling of illumination and the outer cone means that an artist cannot tweak the influence cone of a spot light without also changing the perceived illumination. It therefore makes sense to provide artists with a parameter to disable this coupling. This is the difference between [FOCUSED_SPOT](-type/-f-o-c-u-s-e-d_-s-p-o-t/index.md) (physically correct) and [SPOT](-type/-s-p-o-t/index.md) (decoupled). 

# Performance considerations

 Generally, adding lights to the scene hurts performance, however filament is designed to be able to handle hundreds of lights in a scene under certain conditions. Here are some tips to keep good performance. 

- Prefer spot lights to point lights and use the smallest outer cone angle possible.
- Use the smallest possible falloff distance for point and spot lights. Performance is very sensitive to overlapping lights. The falloff distance essentially defines a sphere of influence for the light, so try to position point and spot lights such that they don't overlap too much.
- On the other hand, a scene can contain hundreds of non overlapping lights without incurring a significant overhead.

## Types

| Name | Summary |
|---|---|
| [Builder](-builder/index.md) | [main]<br>open class [Builder](-builder/index.md)<br>Use Builder to construct a Light object instance |
| [ShadowCascades](-shadow-cascades/index.md) | [main]<br>open class [ShadowCascades](-shadow-cascades/index.md) |
| [ShadowOptions](-shadow-options/index.md) | [main]<br>open class [ShadowOptions](-shadow-options/index.md)<br>Control the quality / performance of the shadow map associated to this light |
| [Type](-type/index.md) | [main]<br>enum [Type](-type/index.md)<br>Denotes the type of the light being created. |

## Properties

| Name | Summary |
|---|---|
| [EFFICIENCY_FLUORESCENT](-e-f-f-i-c-i-e-n-c-y_-f-l-u-o-r-e-s-c-e-n-t.md) | [main]<br>val [EFFICIENCY_FLUORESCENT](-e-f-f-i-c-i-e-n-c-y_-f-l-u-o-r-e-s-c-e-n-t.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html) = 0.0878f<br>Typical efficiency of a fluorescent light bulb (8. |
| [EFFICIENCY_HALOGEN](-e-f-f-i-c-i-e-n-c-y_-h-a-l-o-g-e-n.md) | [main]<br>val [EFFICIENCY_HALOGEN](-e-f-f-i-c-i-e-n-c-y_-h-a-l-o-g-e-n.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html) = 0.0707f<br>Typical efficiency of an halogen light bulb (7. |
| [EFFICIENCY_INCANDESCENT](-e-f-f-i-c-i-e-n-c-y_-i-n-c-a-n-d-e-s-c-e-n-t.md) | [main]<br>val [EFFICIENCY_INCANDESCENT](-e-f-f-i-c-i-e-n-c-y_-i-n-c-a-n-d-e-s-c-e-n-t.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html) = 0.022f<br>Typical efficiency of an incandescent light bulb (2. |
| [EFFICIENCY_LED](-e-f-f-i-c-i-e-n-c-y_-l-e-d.md) | [main]<br>val [EFFICIENCY_LED](-e-f-f-i-c-i-e-n-c-y_-l-e-d.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html) = 0.1171f<br>Typical efficiency of a LED light bulb (11. |

## Functions

| Name | Summary |
|---|---|
| [destroy](destroy.md) | [main]<br>open fun [destroy](destroy.md)(entity: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))<br>Destroys this component from the given entity |
| [getColor](get-color.md) | [main]<br>open fun [getColor](get-color.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;<br>Returns the light color |
| [getComponentCount](get-component-count.md) | [main]<br>open fun [getComponentCount](get-component-count.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Returns the number of components in the LightManager, note that components are not guaranteed to be active. |
| [getDirection](get-direction.md) | [main]<br>open fun [getDirection](get-direction.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;<br>returns the light's direction in world space |
| [getFalloff](get-falloff.md) | [main]<br>open fun [getFalloff](get-falloff.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>returns the falloff distance of this light. |
| [getInnerConeAngle](get-inner-cone-angle.md) | [main]<br>open fun [getInnerConeAngle](get-inner-cone-angle.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html) |
| [getInstance](get-instance.md) | [main]<br>open fun [getInstance](get-instance.md)(entity: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Gets an Instance representing the Light component associated with the given Entity. |
| [getIntensity](get-intensity.md) | [main]<br>open fun [getIntensity](get-intensity.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>returns the light's luminous intensity in *lumens*. |
| [getLightChannel](get-light-channel.md) | [main]<br>open fun [getLightChannel](get-light-channel.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), channel: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Returns whether a light channel is enabled on a specified renderable. |
| [getNativeObject](get-native-object.md) | [main]<br>open fun [getNativeObject](get-native-object.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [getOuterConeAngle](get-outer-cone-angle.md) | [main]<br>open fun [getOuterConeAngle](get-outer-cone-angle.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html) |
| [getPosition](get-position.md) | [main]<br>open fun [getPosition](get-position.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;<br>returns the light's position in world space |
| [getSunAngularRadius](get-sun-angular-radius.md) | [main]<br>open fun [getSunAngularRadius](get-sun-angular-radius.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>returns the angular radius if the sun in degrees. |
| [getSunHaloFalloff](get-sun-halo-falloff.md) | [main]<br>open fun [getSunHaloFalloff](get-sun-halo-falloff.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>returns the halo falloff of a Type.SUN light as a dimensionless value. |
| [getSunHaloSize](get-sun-halo-size.md) | [main]<br>open fun [getSunHaloSize](get-sun-halo-size.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>returns the halo size of a Type.SUN light as a multiplier of the sun angular radius. |
| [getType](get-type.md) | [main]<br>open fun [getType](get-type.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [LightManager.Type](-type/index.md) |
| [hasComponent](has-component.md) | [main]<br>open fun [hasComponent](has-component.md)(entity: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Returns whether a particular Entity is associated with a component of this LightManager |
| [isShadowCaster](is-shadow-caster.md) | [main]<br>open fun [isShadowCaster](is-shadow-caster.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>returns whether this light casts shadows. |
| [setColor](set-color.md) | [main]<br>open fun [setColor](set-color.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), linearR: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), linearG: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), linearB: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html))<br>Dynamically updates the light's hue as linear sRGB |
| [setDirection](set-direction.md) | [main]<br>open fun [setDirection](set-direction.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), x: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), y: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), z: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html))<br>Dynamically updates the light's direction  The light direction is specified in world space and should be a unit vector. |
| [setFalloff](set-falloff.md) | [main]<br>open fun [setFalloff](set-falloff.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), falloff: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html))<br>Set the falloff distance for point lights and spot lights. |
| [setIntensity](set-intensity.md) | [main]<br>open fun [setIntensity](set-intensity.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), intensity: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html))<br>open fun [setIntensity](set-intensity.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), watts: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), efficiency: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html))<br>Dynamically updates the light's intensity. |
| [setIntensityCandela](set-intensity-candela.md) | [main]<br>open fun [setIntensityCandela](set-intensity-candela.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), intensity: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html))<br>Dynamically updates the light's intensity in candela. |
| [setLightChannel](set-light-channel.md) | [main]<br>open fun [setLightChannel](set-light-channel.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), channel: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), enable: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html))<br>Enables or disables a light channel. |
| [setPosition](set-position.md) | [main]<br>open fun [setPosition](set-position.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), x: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), y: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), z: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html))<br>Dynamically updates the light's position. |
| [setShadowCaster](set-shadow-caster.md) | [main]<br>open fun [setShadowCaster](set-shadow-caster.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), shadowCaster: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html))<br>Whether this Light casts shadows (disabled by default) **warning:**[POINT](-type/-p-o-i-n-t/index.md) cannot cast shadows. |
| [setSpotLightCone](set-spot-light-cone.md) | [main]<br>open fun [setSpotLightCone](set-spot-light-cone.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), inner: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), outer: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html))<br>Dynamically updates a spot light's cone as angles |
| [setSunAngularRadius](set-sun-angular-radius.md) | [main]<br>open fun [setSunAngularRadius](set-sun-angular-radius.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), angularRadius: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html))<br>Dynamically updates the angular radius of a Type.SUN light The Sun as seen from Earth has an angular size of 0.526° to 0. |
| [setSunHaloFalloff](set-sun-halo-falloff.md) | [main]<br>open fun [setSunHaloFalloff](set-sun-halo-falloff.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), haloFalloff: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html))<br>Dynamically updates the halo falloff of a Type.SUN light. |
| [setSunHaloSize](set-sun-halo-size.md) | [main]<br>open fun [setSunHaloSize](set-sun-halo-size.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), haloSize: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html))<br>Dynamically updates the halo radius of a Type.SUN light. |
