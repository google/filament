# specular-color

`specular-color` computes the base color of conductors based on spectral data.
The base color is output in linear and sRGB formats. The base color is the reflectance
at normal incidence (0°) and is often noted f0.

`specular-color` can also compute the perceived color of a conductor at another angle,
set to ~82° by default. This value is particularly useful when used in combination with
the Lazanyi-Schlick model to better approximate the behavior of metallic surfaces at
grazing angles. See Hoffman 2019, "Fresnel Equations Considered Harmful".

## Usage

```
$ specular-color <spectral data file>
```

The spectral data files can be obtained from
[Refractive Index](https://refractiveindex.info/?shelf=3d&book=metals&page=brass).

For instance, to compute the base color of gold:

```
$ specular-color data/gold.txt
```

To set the second angle, use `-a` to specify the angle in degrees:

```
$ specular-color -a 75 data/gold.txt
```
