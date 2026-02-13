
// SimulatedSkybox.js
// Ported from samples/utils/SimulatedSkybox.cpp

class SimulatedSkybox {
  constructor(engine) {
    this.engine = engine;

    // Default Parameters
    this.sunDirection = [0, 1, 0];
    this.sunIntensity = 100000.0;
    this.turbidity = 2.0;
    this.rayleigh = 1.0;
    this.mieCoefficient = 1.0;
    this.mieG = 0.8;
    this.ozone = 0.0;
    this.msFactors = [0.1, 0.5, 0.0];
    this.contrast = 1.0;
    this.nightColor = [0.0, 0.0003, 0.00075];
    this.shimmerControl = [0.0, 20.0, 0.1];
    this.cloudControl = [0.0, 0.1, 8000.0, 0.0];
    this.cloudControl2 = [0.0, 0.0, 0.0, 0.0];
    this.waterControl = [50.0, 1.0, 1.0, 4.0]; // x=Strength, y=Speed, z=DerivativeTrick, w=Octaves
    this.starControl = [0.001, 1.0, 350.0, 0.01]; // x=Density, y=Enabled, z=Frequency, w=PixelScale
    this.focalLength = 24.0;
    this.height = 1000.0;
    this.planetRadius = 6360.0;

    // Sun Halo
    // x=cos(rad), y=limbDarkening, z=intensity, w=enabled
    // Sun Halo
    this.sunHalo = [Math.cos(0.5 * Math.PI / 180.0), 0.5, 1.0, 1.0];

    // Moon Parameters (Mapped to Secondary Sun)
    this.moonDirection = [-0.2, 0.8, -0.2]; // Default Moon Pos
    this.moonIntensity = 1.0; // Scale Factor (1.0 = Physical Peak)
    // x=cos(rad), y=sin(rad) [Precision Fix], z=intensity, w=enabled
    this.moonHalo = [Math.cos(0.5 * Math.PI / 180.0), Math.sin(0.5 * Math.PI / 180.0), 1.0, 0.0]; // Disabled by default

    this.moonTextureObj = null;
    this.moonNormalObj = null;
    this.milkyWayTextureObj = null;

    // Milky Way Parameters
    // x=Intensity, y=Saturation, z=Unused
    this.milkyWayControl = [1.0, 1.0, 0.07];
    this.milkyWayEnabled = true;
    this.milkyWayRotation = [1, 0, 0, 0, 1, 0, 0, 0, 1]; // Identity by default
    this.initEntity();
  }

  async loadMaterial(url) {
    console.log("Loading material from:", url);
    const response = await fetch(url);
    const buffer = await response.arrayBuffer();
    this.material = this.engine.createMaterial(new Uint8Array(buffer));
    this.materialInstance = this.material.createInstance();

    // Re-bind the entity with the loaded material
    const rcm = this.engine.getRenderableManager();
    const instance = rcm.getInstance(this.entity);
    rcm.setMaterialInstanceAt(instance, 0, this.materialInstance);

    console.log("Material loaded and bound.");

    // Load Moon Texture
    try {
      const texUrl = 'assets/moon_disk.png';
      const Texture = Filament.Texture;
      const TextureSampler = Filament.TextureSampler;
      const PixelDataFormat = Filament.PixelDataFormat;
      const PixelDataType = Filament.PixelDataType;
      const TextureUsage = Filament.Texture$Usage;
      const TextureFormat = Filament.Texture$InternalFormat;
      const MinFilter = Filament.MinFilter;
      const MagFilter = Filament.MagFilter;
      const WrapMode = Filament.WrapMode;
      const texResponse = await fetch(texUrl);
      const texBlob = await texResponse.blob();
      const bitmap = await createImageBitmap(texBlob);

      const width = bitmap.width;
      const height = bitmap.height;

      this.moonTextureObj = Texture.Builder()
        .width(width)
        .height(height)
        .levels(0xff)
        .format(TextureFormat.SRGB8_A8)
        .usage(TextureUsage.SAMPLEABLE.value | TextureUsage.UPLOADABLE.value | TextureUsage.GEN_MIPMAPPABLE.value)
        .build(this.engine);

      // Extract Data using a Canvas
      const canvas = document.createElement('canvas');
      canvas.width = width;
      canvas.height = height;
      const ctx = canvas.getContext('2d');
      ctx.drawImage(bitmap, 0, 0);
      const imageData = ctx.getImageData(0, 0, width, height);

      // Use RGBA data directly (Uint8ClampedArray)
      // Filament supports RGBA upload for RGB/SRGB internal formats (drops alpha).
      // This matches Canvas layout (Top-Down, RGBA) and is 4-byte aligned.
      const pbd = Filament.PixelBuffer(
        new Uint8Array(imageData.data.buffer), // Wrap buffer to ensure Uint8Array
        PixelDataFormat.RGBA,
        PixelDataType.UBYTE
      );

      this.moonTextureObj.setImage(this.engine, 0, pbd);
      this.moonTextureObj.generateMipmaps(this.engine);

      const sampler = new TextureSampler(
        MinFilter.LINEAR,
        MagFilter.LINEAR,
        WrapMode.CLAMP_TO_EDGE
      );

      this.materialInstance.setTextureParameter('moonTexture', this.moonTextureObj, sampler);

    } catch (e) {
      console.error("Failed to load moon texture:", e);
    }

    // Load Moon Normal
    try {
      const texUrl = 'assets/moon_normal.png';
      const Texture = Filament.Texture;
      const TextureSampler = Filament.TextureSampler;
      const PixelDataFormat = Filament.PixelDataFormat;
      const PixelDataType = Filament.PixelDataType;
      const TextureUsage = Filament.Texture$Usage;
      const TextureFormat = Filament.Texture$InternalFormat;
      const MinFilter = Filament.MinFilter;
      const MagFilter = Filament.MagFilter;
      const WrapMode = Filament.WrapMode;
      const texResponse = await fetch(texUrl);
      const texBlob = await texResponse.blob();
      const bitmap = await createImageBitmap(texBlob);

      const width = bitmap.width;
      const height = bitmap.height;

      this.moonNormalObj = Texture.Builder()
        .width(width)
        .height(height)
        .levels(0xff)
        .format(TextureFormat.RGBA8)
        .usage(TextureUsage.SAMPLEABLE.value | TextureUsage.UPLOADABLE.value | TextureUsage.GEN_MIPMAPPABLE.value)
        .build(this.engine);

      // Extract Data using a Canvas
      const canvas = document.createElement('canvas');
      canvas.width = width;
      canvas.height = height;
      const ctx = canvas.getContext('2d');
      ctx.drawImage(bitmap, 0, 0);
      const imageData = ctx.getImageData(0, 0, width, height);

      // Use RGBA data directly
      const pbd = Filament.PixelBuffer(
        new Uint8Array(imageData.data.buffer),
        PixelDataFormat.RGBA,
        PixelDataType.UBYTE
      );

      this.moonNormalObj.setImage(this.engine, 0, pbd);
      this.moonNormalObj.generateMipmaps(this.engine);

      const sampler = new TextureSampler(
        MinFilter.LINEAR_MIPMAP_LINEAR,
        MagFilter.LINEAR,
        WrapMode.CLAMP_TO_EDGE
      );

      this.materialInstance.setTextureParameter('moonNormal', this.moonNormalObj, sampler);

    } catch (e) {
      console.error("Failed to load moon normal:", e);
    }

    // Load Milky Way Texture
    try {
      const texUrl = 'assets/milkyway.png';
      const Texture = Filament.Texture;
      const TextureSampler = Filament.TextureSampler;
      const PixelDataFormat = Filament.PixelDataFormat;
      const PixelDataType = Filament.PixelDataType;
      const TextureUsage = Filament.Texture$Usage;
      const TextureFormat = Filament.Texture$InternalFormat;
      const MinFilter = Filament.MinFilter;
      const MagFilter = Filament.MagFilter;
      const WrapMode = Filament.WrapMode;
      const texResponse = await fetch(texUrl);
      const texBlob = await texResponse.blob();
      const bitmap = await createImageBitmap(texBlob);

      const width = bitmap.width;
      const height = bitmap.height;

      this.milkyWayTextureObj = Texture.Builder()
        .width(width)
        .height(height)
        .levels(0xff)
        .format(TextureFormat.SRGB8_A8)
        .usage(TextureUsage.SAMPLEABLE.value | TextureUsage.UPLOADABLE.value | TextureUsage.GEN_MIPMAPPABLE.value)
        .build(this.engine);

      // Extract Data using a Canvas
      const canvas = document.createElement('canvas');
      canvas.width = width;
      canvas.height = height;
      const ctx = canvas.getContext('2d');
      ctx.drawImage(bitmap, 0, 0);
      try {
        const imageData = ctx.getImageData(0, 0, width, height);

        // Use RGBA data directly
        const pbd = Filament.PixelBuffer(
          new Uint8Array(imageData.data.buffer),
          PixelDataFormat.RGBA,
          PixelDataType.UBYTE
        );

        this.milkyWayTextureObj.setImage(this.engine, 0, pbd);
        this.milkyWayTextureObj.generateMipmaps(this.engine);

        const sampler = new TextureSampler(
          MinFilter.LINEAR_MIPMAP_LINEAR,
          MagFilter.LINEAR,
          WrapMode.REPEAT // Equirectangular wraps horizontally
        );

        this.materialInstance.setTextureParameter('milkyWayTexture', this.milkyWayTextureObj, sampler);
      } catch (err) {
        console.warn("Milky Way texture data access failed (CORS?):", err);
      }

    } catch (e) {
      console.error("Failed to load milky way texture:", e);
    }

    this.updateCoefficients();
  }

  initEntity() {
    const EntityManager = Filament.EntityManager;
    const RenderableManager = Filament.RenderableManager;
    const VertexBuffer = Filament.VertexBuffer;
    const IndexBuffer = Filament.IndexBuffer;
    const AttributeType = Filament.VertexBuffer$AttributeType;
    const VertexAttribute = Filament.VertexAttribute;
    const PrimitiveType = Filament.RenderableManager$PrimitiveType;
    const IndexType = Filament.IndexBuffer$IndexType;

    this.entity = EntityManager.get().create();

    // 3 vertices for full screen triangle
    // coords: -1,-1 to 3,-1 to -1,3
    const TRIANGLE_VERTICES = new Float32Array([
      -1.0, -1.0,
      3.0, -1.0,
      -1.0, 3.0
    ]);

    const TRIANGLE_INDICES = new Uint16Array([0, 1, 2]);

    this.vb = VertexBuffer.Builder()
      .vertexCount(3)
      .bufferCount(1)
      .attribute(VertexAttribute.POSITION, 0, AttributeType.FLOAT2, 0, 8)
      .build(this.engine);

    this.vb.setBufferAt(this.engine, 0, TRIANGLE_VERTICES);

    this.ib = IndexBuffer.Builder()
      .indexCount(3)
      .bufferType(IndexType.USHORT)
      .build(this.engine);

    this.ib.setBuffer(this.engine, TRIANGLE_INDICES);

    // Build the Renderable without material; it will be set later by loadMaterial.


    RenderableManager.Builder(1)
      .geometry(0, PrimitiveType.TRIANGLES, this.vb, this.ib)
      .culling(false)
      .castShadows(false)
      .receiveShadows(false)
      .priority(7) // Skybox priority

      .build(this.engine, this.entity);
  }

  setSunPosition(direction) {
    // normalize
    const len = Math.hypot(direction[0], direction[1], direction[2]);
    if (len > 0) {
      this.sunDirection = [direction[0] / len, direction[1] / len, direction[2] / len];
    } else {
      this.sunDirection = [0, 1, 0];
    }
    this.updateCoefficients();
  }

  setSunIntensity(intensity) {
    this.sunIntensity = Math.max(0.0, intensity);
    this.updateCoefficients();
  }

  setTurbidity(turbidity) {
    this.turbidity = Math.max(0.0, turbidity);
    this.updateCoefficients();
  }

  setRayleigh(rayleigh) {
    this.rayleigh = Math.max(0.0, rayleigh);
    this.updateCoefficients();
  }

  setMieCoefficient(mie) {
    this.mieCoefficient = Math.max(0.0, mie);
    this.updateCoefficients();
  }

  setMieG(g) {
    this.mieG = Math.max(0.0, g);
    this.updateCoefficients();
  }

  setOzone(strength) {
    this.ozone = Math.max(0.0, strength);
    this.updateCoefficients();
  }

  setMultiScattering(r, m) {
    this.msFactors[0] = Math.max(0.0, Math.min(2.0, r));
    this.msFactors[1] = Math.max(0.0, Math.min(2.0, m));
    this.updateCoefficients();
  }

  setHorizonGlow(strength) {
    this.msFactors[2] = Math.max(0.0, Math.min(1.0, strength));
    this.updateCoefficients();
  }

  setContrast(contrast) {
    this.contrast = contrast;
    this.updateCoefficients();
  }

  setNightColor(color) {
    this.nightColor = color;
    this.updateCoefficients();
  }

  setSunRadius(degrees) {
    const rad = degrees * (Math.PI / 180.0);
    this.sunHalo[0] = Math.cos(rad);
    this.updateCoefficients();
  }

  setSunDiskIntensity(intensity) {
    this.sunHalo[2] = Math.max(0.0, intensity);
    this.updateCoefficients();
  }

  setSunLimbDarkening(strength) {
    this.sunHalo[1] = Math.max(0.0, strength);
    this.updateCoefficients();
  }

  setSunDiskEnabled(enabled) {
    this.sunHalo[3] = enabled ? 1.0 : 0.0;
    this.updateCoefficients();
  }

  setShimmerControl(strength, frequency, maskHeight) {
    this.shimmerControl[0] = Math.max(0.0, strength);
    this.shimmerControl[1] = Math.max(0.0, frequency);
    this.shimmerControl[2] = Math.max(0.001, maskHeight);
    this.updateCoefficients();
  }

  setCloudControl(coverage, density, height, speed) {
    this.cloudControl[0] = Math.max(0.0, Math.min(1.0, coverage));
    this.cloudControl[1] = Math.max(0.0, density);
    this.cloudControl[2] = Math.max(1000.0, height);
    // JS speed adjustment logic matches C++: speed * (0.05 / 72.0)
    this.cloudControl[3] = speed * (0.05 / 72.0);
    this.updateCoefficients();
  }

  setCloudShapeEvolution(speed) {
    this.cloudControl2[0] = speed;
    this.updateCoefficients();
  }

  setCloudVolumetricLighting(enabled) {
    this.cloudControl2[1] = enabled ? 1.0 : 0.0;
    this.updateCoefficients();
  }

  setWaterControl(strength, speed, derivativeTrick, octaves) {
    this.waterControl[0] = Math.max(0.0, strength);
    this.waterControl[1] = Math.max(0.0, speed);
    this.waterControl[2] = derivativeTrick;
    this.waterControl[3] = Math.max(1.0, Math.min(8.0, octaves));
    this.updateCoefficients();
  }

  setStarControl(density, enabled) {
    // Compensate for grid frequency reduction (350 -> 100)
    // Fewer cells = fewer stars, so we increase density threshold.
    // Factor ~ (350/100)^2 = 12.25
    const compensatedDensity = density * 12.0;
    this.starControl[0] = Math.max(0.0, Math.min(1.0, compensatedDensity));
    this.starControl[1] = enabled ? 1.0 : 0.0;
    this.updateCoefficients();
  }

  setFocalLength(mm) {
    this.focalLength = Math.max(1.0, mm);
    this.updateStarFrequency();
  }

  setResolution(height) {
    this.height = Math.max(1.0, height);
    this.updateStarFrequency();
  }

  updateStarFrequency() {
    // World-Anchored Stars
    // z = Fixed Frequency (World Space Grid)
    // w = Pixel Scale (Screen Space Radius)

    // Fixed Frequency: Defines the "Universe" coordinate system. 
    // Reduced to 100.0 to allow larger stars without clipping (square artifacts).
    this.starControl[2] = 100.0;

    // Pixel Scale in Radians
    // We use linear scaling (24/f) instead of atan(fov) to ensure star size remains
    // constant in pixels across all focal lengths (Perspective Projection).
    const fovFactor = 24.0 / this.focalLength;
    const pixelScale = (1.0 / this.height) * fovFactor;

    // Pass to shader (w component)
    // Target radius: 1.3 pixels (Diameter 2.6 pixels)
    // Visible but sharp.
    this.starControl[3] = pixelScale * 1.3;

    this.updateCoefficients();
  }

  setMoonPosition(direction) {
    // normalize
    const len = Math.hypot(direction[0], direction[1], direction[2]);
    if (len > 0) {
      this.moonDirection = [direction[0] / len, direction[1] / len, direction[2] / len];
    }
    this.updateCoefficients();
  }

  setMoonIntensity(intensity) {
    this.moonIntensity = Math.max(0.0, intensity);
    this.updateCoefficients();
  }

  setMoonRadius(degrees) {
    const rad = degrees * (Math.PI / 180.0);
    this.moonHalo[0] = Math.cos(rad);
    this.moonHalo[1] = Math.sin(rad);
    this.updateCoefficients();
  }

  setMoonEnabled(enabled) {
    this.moonHalo[3] = enabled ? 1.0 : 0.0;
    this.updateCoefficients();
  }

  setMilkyWayControl(intensity, saturation, blackPoint) {
    this.milkyWayControl[0] = Math.max(0.0, intensity);
    this.milkyWayControl[1] = Math.max(0.0, saturation);
    if (blackPoint !== undefined) {
      this.milkyWayControl[2] = Math.max(0.0, blackPoint);
    }
    this.updateCoefficients();
  }

  setMilkyWayEnabled(enabled) {
    this.milkyWayEnabled = !!enabled;
    this.updateCoefficients();
  }

  setMilkyWayRotation(rotationMatrix) {
    if (rotationMatrix && rotationMatrix.length === 9) {
      this.milkyWayRotation = rotationMatrix;
      this.updateCoefficients();
    }
  }

  updateCoefficients() {
    if (!this.materialInstance) {
      console.warn("updateCoefficients called before material loaded");
      return;
    }


    // 1. Rayleigh Coefficients
    const F_PI = Math.PI;
    const lambda = [680e-9, 550e-9, 440e-9];
    const n = 1.0003;
    const N = 2.545e25;
    const term = (8.0 * Math.pow(F_PI, 3.0) * Math.pow(n * n - 1.0, 2.0)) / (3.0 * N);

    const depthR = [
      term / Math.pow(lambda[0], 4.0),
      term / Math.pow(lambda[1], 4.0),
      term / Math.pow(lambda[2], 4.0)
    ].map(v => v * 8000.0 * this.rayleigh);

    // 2. Mie Coefficients
    const mieAlpha = 1.3;
    const mieBase = 2.0e-5 * this.turbidity;
    const depthM = [
      mieBase * Math.pow(550e-9 / lambda[0], mieAlpha),
      mieBase * Math.pow(550e-9 / lambda[1], mieAlpha),
      mieBase * Math.pow(550e-9 / lambda[2], mieAlpha)
    ].map(v => v * 1200.0 * this.mieCoefficient);

    // Fake Ozone
    const ozone = [0.0, this.ozone * 0.1, 0.0];

    // Sun Fade (Horizon)
    const cutoffAngle = 96.0 * (F_PI / 180.0);
    const steepness = 1.5;
    const zenithFade = 1.0 - Math.exp(-(cutoffAngle / steepness));
    const zenithAngle = Math.acos(Math.max(-1.0, Math.min(1.0, this.sunDirection[1])));
    const sunFade = Math.max(0.0, 1.0 - Math.exp(-((cutoffAngle - zenithAngle) / steepness))) / zenithFade;

    const physicalSunIntensity = this.sunIntensity * sunFade;

    // Radiance Conversion for Sun Halo
    // Solid Angle = 2 * PI * (1 - cos(angularRadius))
    const solidAngle = 2.0 * F_PI * (1.0 - this.sunHalo[0]);
    const radianceConversion = 1.0 / Math.max(1e-9, solidAngle);
    const sunHaloUpload = [...this.sunHalo];
    sunHaloUpload[2] *= radianceConversion;

    // Cloud Intersection
    const r = this.planetRadius;
    const h = this.cloudControl[2] * 0.001; // m -> km
    const intersectC = r * r - (r + h) * (r + h);
    const cloudUniform = [...this.cloudControl];
    cloudUniform[2] = intersectC;

    // Shimmer Uniform
    const shimmerUniform = [...this.shimmerControl, r];

    // Multi-Scattering Vector
    const isotropicPhase = 0.25;
    const msVector = depthR.map((v, i) => (v * this.msFactors[0] + depthM[i] * this.msFactors[1]) * isotropicPhase);

    // Upload
    this.materialInstance.setFloat3Parameter('sunDirection', new Float32Array(this.sunDirection));
    this.materialInstance.setFloat3Parameter('depthR', new Float32Array(depthR));
    this.materialInstance.setFloat3Parameter('depthM', new Float32Array(depthM));
    this.materialInstance.setFloat3Parameter('ozone', new Float32Array(ozone));
    this.materialInstance.setFloat4Parameter('sunHalo', new Float32Array(sunHaloUpload));

    this.materialInstance.setFloat4Parameter('multiScatParams', new Float32Array([...msVector, this.msFactors[2]]));

    // Mie Phase
    const g2 = this.mieG * this.mieG;
    this.materialInstance.setFloat2Parameter('miePhaseParams', new Float32Array([1.0 + g2, -2.0 * this.mieG]));

    this.materialInstance.setFloatParameter('contrast', this.contrast);

    const nightColorScaled = this.nightColor.map(v => v * this.sunIntensity); // Lux scaling
    this.materialInstance.setFloat3Parameter('nightColor', new Float32Array(nightColorScaled));

    this.materialInstance.setFloat4Parameter('shimmerControl', new Float32Array(shimmerUniform));
    this.materialInstance.setFloat4Parameter('cloudControl', new Float32Array(cloudUniform));
    this.materialInstance.setFloat4Parameter('cloudControl2', new Float32Array(this.cloudControl2));
    this.materialInstance.setFloat4Parameter('waterControl', new Float32Array(this.waterControl));
    this.materialInstance.setFloat4Parameter('starControl', new Float32Array(this.starControl));

    this.materialInstance.setFloatParameter('sunIntensity', physicalSunIntensity);

    // Moon Upload (Secondary Sun)
    this.materialInstance.setFloat3Parameter('sunDirection2', new Float32Array(this.moonDirection));

    // Calculate Moon Phase Factor (Lambertian Sphere)
    // We model the moon as a Lambertian sphere to calculate its integrated brightness (illuminance)
    // based on the phase angle (angle between Sun-Moon and Observer-Moon vectors).
    //
    // Phase Angle (alpha):
    // For a distant observer (Earth), the phase angle can be approximated as the angle between
    // the vector to the Sun and the vector to the Earth (from the Moon).
    // cos(alpha) = -dot(L_moon, L_sun)
    //
    // Lambertian Phase Law:
    // The integrated flux of a lit sphere varies as:
    // Phi(alpha) = (1/PI) * (sin(alpha) + (PI - alpha) * cos(alpha))
    // This gives 1.0 at Full Moon (alpha=0) and 0.0 at New Moon (alpha=PI).

    const dotSM = this.sunDirection[0] * this.moonDirection[0] +
      this.sunDirection[1] * this.moonDirection[1] +
      this.sunDirection[2] * this.moonDirection[2];

    // Final Intensity = Peak * Scale (No Phase Factor - Phase is handled in Shader via N.L)
    const MOON_PEAK_LUX = 1.0;
    const finalMoonIntensity = MOON_PEAK_LUX * this.moonIntensity;

    this.materialInstance.setFloatParameter('sunIntensity2', finalMoonIntensity);

    // Scale Milky Way by Sun Intensity (Pre-exposed) to match dynamic range
    // Calibration: 1.0 User Intensity ~ 0.025 Lux Relative (approx 2.5% of Sun Pixel Value at Day)
    // At Sunny 16 (Sun ~ 2.6), this gives ~0.065 pixel brightness, which is visible but dim.
    const mwIntensity = this.milkyWayEnabled ? this.milkyWayControl[0] : 0.0;
    const mwUniform = [
      mwIntensity * this.sunIntensity * 0.025,
      this.milkyWayControl[1],
      this.milkyWayControl[2]
    ];
    this.materialInstance.setFloat3Parameter('milkyWayControl', new Float32Array(mwUniform));
    this.materialInstance.setMat3Parameter('milkyWayRotation', new Float32Array(this.milkyWayRotation));

    // Moon Halo Upload (Disk Visualization)
    // Multiplier = 1.0 / SolidAngle
    const moonSolidAngle = 2.0 * F_PI * (1.0 - this.moonHalo[0]);
    const moonRadConv = 1.0 / Math.max(1e-9, moonSolidAngle);
    const moonHaloUpload = [...this.moonHalo];
    moonHaloUpload[2] *= moonRadConv;
    this.materialInstance.setFloat4Parameter('sunHalo2', new Float32Array(moonHaloUpload));

    // Solar Eclipse (CPU Calculation)
    const sunRadius = Math.acos(this.sunHalo[0]);
    const moonRadius = Math.acos(this.moonHalo[0]);
    // Dot product of Sun and Moon directions
    const dot = this.sunDirection[0] * this.moonDirection[0] +
      this.sunDirection[1] * this.moonDirection[1] +
      this.sunDirection[2] * this.moonDirection[2];
    const separation = Math.acos(Math.max(-1.0, Math.min(1.0, dot)));

    let eclipseFactor = 1.0;
    // Only calculate if moon is enabled
    if (this.moonHalo[3] > 0.5) {
      const overlap = this.areaIntersection(sunRadius, moonRadius, separation);
      const sunArea = Math.PI * sunRadius * sunRadius;
      // Ensure we don't divide by zero and clamp result
      const ratio = overlap / Math.max(1e-9, sunArea);
      eclipseFactor = 1.0 - Math.max(0.0, Math.min(1.0, ratio));

    }

    // Safety check for NaN
    if (isNaN(eclipseFactor)) {
      console.warn("SimulatedSkybox: eclipseFactor is NaN, resetting to 1.0");
      eclipseFactor = 1.0;
    }



    this.materialInstance.setFloatParameter('eclipseFactor', eclipseFactor);
  }

  areaIntersection(r1, r2, d) {
    // Circle intersection area
    // r1, r2: radii
    // d: distance between centers

    // Case 1: Too far apart
    if (d >= r1 + r2) {
      return 0.0;
    }
    // Case 2: One inside another
    if (d <= Math.abs(r1 - r2)) {
      return Math.PI * Math.min(r1, r2) * Math.min(r1, r2);
    }

    const r1sq = r1 * r1;
    const r2sq = r2 * r2;

    // Law of Cosines for sector angles
    // c1 = (d^2 + r1^2 - r2^2) / (2 * d * r1)
    // c2 = (d^2 + r2^2 - r1^2) / (2 * d * r2)
    // We clamp to [-1, 1] to avoid NaN from floating point errors
    const c1 = Math.max(-1.0, Math.min(1.0, (d * d + r1sq - r2sq) / (2.0 * d * r1)));
    const c2 = Math.max(-1.0, Math.min(1.0, (d * d + r2sq - r1sq) / (2.0 * d * r2)));

    const part1 = r1sq * Math.acos(c1);
    const part2 = r2sq * Math.acos(c2);

    // Heron's formula for the triangle area * 2
    // The sqrt term represents the area of the two triangles formed by the chord and centers.


    // Robust sqrt
    const val = (-d + r1 + r2) * (d + r1 - r2) * (d - r1 + r2) * (d + r1 + r2);
    const part3 = 0.5 * Math.sqrt(Math.max(0.0, val));

    return part1 + part2 - part3;
  }
}
