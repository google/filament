# SPECGEN: Spectral Integration Matrix Generator for Real-Time Dispersion

## Theoretical Background

### 1. Spectral Rendering in RGB

Real-time rendering typically operates in a tristimulus color space (like sRGB). However, physical phenomena like dispersion (refraction splitting light by wavelength) are inherently spectral. To simulate this in an RGB pipeline, we approximate the continuous spectral integration using a finite sum of weighted samples.

The fundamental equation for perceived color \\(C\\) is:

$$ C = \int L(\lambda) \cdot r(\lambda) d\lambda $$

where:
*   \\(L(\lambda)\\) is the spectral radiance reaching the eye.
*   \\(r(\lambda)\\) is the sensor response function (e.g., CIE 1931 \\(\bar{x}, \bar{y}, \bar{z}\\) matching functions).

### 2. Basis Transformation Strategy

We assume the input light is defined in linear sRGB. To process it spectrally:

1.  Convert sRGB to CIE XYZ (D65 white point).
2.  Assume the spectral distribution of the light is a sum of impulses or narrow bands weighted by the XYZ components (or simplified directly from sRGB).
3.  Apply spectral effects (like wavelength-dependent refraction).
4.  Integrate back to XYZ using the CIE Color Matching Functions (CMFs).
5.  Convert final XYZ back to sRGB.

This tool pre-calculates a set of matrices (\\(K_n\\)) that combine these steps. For a set of \\(N\\) sample wavelengths \\(\{ \lambda_0, \dots, \lambda_{N-1} \}\\), the final color is:

$$ C_{final} = \sum_{n=0}^{N-1} (K_n \cdot C_{input}) $$

Each matrix \\(K_n\\) represents the contribution of the \\(n\\)-th spectral sample to the final image, accounting for the conversion to/from XYZ and the spectral weight of that sample.

**Derivation of \\(K_n\\):**

$$ K_n = M_{XYZ \to sRGB} \cdot \text{Diag}(W_n) \cdot M_{sRGB \to XYZ} $$

where \\(W_n\\) is the "spectral weight" vector \\((x, y, z)\\) for wavelength \\(\lambda_n\\):

$$ W_n = \text{CMF}(\lambda_n) \cdot \text{weight}_n $$

### 3. Normalization (Energy Conservation)

To ensure that a white input \\((1, 1, 1)\\) results in a white output \\((1, 1, 1)\\) when no dispersion occurs (i.e., all samples land on the same pixel), we must normalize the weights.

We require: \\(\sum K_n = I\\)

This implies:

$$ \sum ( M_{XYZ \to sRGB} \cdot \text{Diag}(W_n) \cdot M_{sRGB \to XYZ} ) = I $$
$$ M_{XYZ \to sRGB} \cdot \sum \text{Diag}(W_n) \cdot M_{sRGB \to XYZ} = I $$
$$ \sum \text{Diag}(W_n) = M_{sRGB \to XYZ} \cdot I \cdot M_{XYZ \to sRGB} $$
$$ \sum \text{Diag}(W_n) = I $$
*(since \\(M \cdot M^{-1} = I\\))*

Therefore, we normalize \\(W_n\\) such that:

$$ \sum W_{n,x} = 1.0, \quad \sum W_{n,y} = 1.0, \quad \sum W_{n,z} = 1.0 $$

**Note:** We do **NOT** normalize to the D65 white point \\((0.95047, 1.0, 1.08883)\\). The \\(M_{sRGB \to XYZ}\\) matrix already handles the conversion from linear sRGB \\((1, 1, 1)\\) to D65 XYZ. If we normalized \\(W_n\\) to D65, we would effectively be applying the white point twice, resulting in a tinted image. By normalizing \\(\sum W_n\\) to \\((1, 1, 1)\\), we ensure that the energy is conserved through the spectral transformation pipeline.

In practice, we calculate the raw sum of \\(W_n\\) from the quadrature weights and CMFs, then compute a correction factor:

$$ \text{Correction} = \frac{1.0}{\sum W_{n,raw}} $$
$$ W_{n,final} = W_{n,raw} \cdot \text{Correction} $$

### 4. Dispersion and IOR Offsets

The Index of Refraction (IOR) varies with wavelength. We use the Abbe number (\\(V_d\\)) to parameterize this variation relative to a base IOR (\\(n_D\\)) at 589.3nm.

**Cauchy Dispersion Model:**

$$ n(\lambda) = A + \frac{B}{\lambda^2} $$

Using the definition of Abbe number \\(V_d = \frac{n_D - 1}{n_F - n_C}\\), we can derive:

$$ n(\lambda) = n_D + \frac{n_D - 1}{V_d} \cdot \text{Offset}(\lambda) $$

Where \\(\text{Offset}(\lambda)\\) is pre-calculated by this tool:

$$ \text{Offset}(\lambda) = \frac{ \frac{1}{\lambda^2} - \frac{1}{\lambda_D^2} }{ \frac{1}{\lambda_F^2} - \frac{1}{\lambda_C^2} } $$

This allows the shader to compute the specific IOR for each sample efficiently:

```glsl
float ior_n = baseIOR + dispersionFactor * offsets[n];
```

