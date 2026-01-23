
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
    this.starControl = [1.0, 1.0]; // x=Density (0-1), y=Enabled (0-1)
    this.planetRadius = 6360.0;

    // Sun Halo
    // x=cos(rad), y=limbDarkening, z=intensity, w=enabled
    this.sunHalo = [Math.cos(0.5 * Math.PI / 180.0), 0.5, 1.0, 1.0];

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

    // We create a dummy material first or wait? 
    // In JS we usually can't block. We'll rely on loadMaterial being called.
    // For now, we build the Renderable without material, then set it later.

    RenderableManager.Builder(1)
      .geometry(0, PrimitiveType.TRIANGLES, this.vb, this.ib)
      .culling(false)
      .castShadows(false)
      .receiveShadows(false)
      .priority(7) // Render behind translucent objects? 7 is skybox priority typically.
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
    this.starControl[0] = Math.max(0.0, Math.min(1.0, density));
    this.starControl[1] = enabled ? 1.0 : 0.0;
    this.updateCoefficients();
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

    const nightColorScaled = this.nightColor.map(v => v * this.sunIntensity);
    this.materialInstance.setFloat3Parameter('nightColor', new Float32Array(nightColorScaled));

    this.materialInstance.setFloat4Parameter('shimmerControl', new Float32Array(shimmerUniform));
    this.materialInstance.setFloat4Parameter('cloudControl', new Float32Array(cloudUniform));
    this.materialInstance.setFloat4Parameter('cloudControl2', new Float32Array(this.cloudControl2));
    this.materialInstance.setFloat4Parameter('waterControl', new Float32Array(this.waterControl));
    this.materialInstance.setFloat2Parameter('starControl', new Float32Array(this.starControl));

    this.materialInstance.setFloatParameter('sunIntensity', physicalSunIntensity);
  }
}
