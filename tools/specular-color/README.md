# specular-color

`specular-color` computes the base color of conductors based on spectral data.
The base color is output in linear and sRGB formats.

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
