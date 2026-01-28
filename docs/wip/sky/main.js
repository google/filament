
// main.js

Filament.init(['assets/simulated_skybox.filamat'], () => {
  window.app = new App(document.getElementsByTagName('canvas')[0]);
});

class App {
  constructor(canvas) {
    this.canvas = canvas;
    const engine = this.engine = Filament.Engine.create(this.canvas);
    this.scene = engine.createScene();

    this.skybox = new SimulatedSkybox(engine);
    this.skybox.entity = this.skybox.entity; // Ensuring access if needed
    this.scene.addEntity(this.skybox.entity);

    // Load the material explicitly since we passed it to init but SimulatedSkybox needs to bind it
    // Actually SimulatedSkybox.loadMaterial fetches it. 
    // Since we already loaded it in Filament.init, we can arguably just use it if we had a way to access the asset.
    // But Filament.init assets are for internal or easy access via assets object if configured?
    // Let's just let SimulatedSkybox fetch it again or use a blob if we wanted.
    // Simpler: Just let SimulatedSkybox fetch it.
    this.skybox.loadMaterial('assets/simulated_skybox.filamat').then(() => {
      this.initGUI();
    });

    this.swapChain = engine.createSwapChain();
    this.renderer = engine.createRenderer();
    this.camera = engine.createCamera(Filament.EntityManager.get().create());

    this.view = engine.createView();
    this.view.setCamera(this.camera);
    this.view.setScene(this.scene);
    // Color Grading
    const ColorGrading = Filament.ColorGrading;
    const ToneMapping = Filament.ColorGrading$ToneMapping;
    this.colorGrading = ColorGrading.Builder()
      .toneMapping(ToneMapping.ACES_LEGACY)
      .build(engine);
    this.view.setColorGrading(this.colorGrading);
    this.view.setPostProcessingEnabled(true); // Essential for tone mapping

    // Bloom
    this.view.setBloomOptions({
      enabled: false,
      lenseFlare: false
    });

    // Clear color is not really visible behind skybox, but black is standard
    this.renderer.setClearOptions({ clearColor: [0.0, 0.0, 0.0, 1.0], clear: true });

    // Camera handling (Exposure)
    this.params = {
      aperture: 16.0,
      shutterSpeed: 125.0,
      iso: 100.0,
      sunTheta: Math.acos(0.0), // Default Height 0.0 (Horizon)
      sunPhi: 0.0,
      focalLength: 24.0, // mm
      sunIntensity: 100000.0 // Base intensity
    };

    this.camState = {
      theta: Math.PI / 2, // Look at +X (Sun Position at Phi=0)
      phi: 0.0,
      dragging: false,
      lastX: 0,
      lastY: 0
    };

    this.initControls(); // Initialize controls immediately

    this.resize();
    window.addEventListener('resize', this.resize.bind(this));

    this.render = this.render.bind(this);
    window.requestAnimationFrame(this.render);
  }

  getExposure() {
    // Formula: 1.0 / ( 1.2 * (N^2 / t) * (S / 100) )
    // t = 1/shutterSpeed
    const N = this.params.aperture;
    const t = 1.0 / this.params.shutterSpeed;
    const S = this.params.iso;
    const ev100_linear = (N * N) / t * (100.0 / S);
    const exposure = 1.0 / (1.2 * ev100_linear);
    return exposure;
  }

  updateCameraExposure() {
    this.camera.setExposure(this.params.aperture, 1.0 / this.params.shutterSpeed, this.params.iso);
    // Also update Sun Intensity because it needs to be pre-exposed
    this.updateSunIntensity();
  }

  updateSunIntensity() {
    const exposure = this.getExposure();
    const preExposedIntensity = this.params.sunIntensity * exposure;
    this.skybox.setSunIntensity(preExposedIntensity);
  }

  updateCameraProjection() {
    const width = this.canvas.width;
    const height = this.canvas.height;
    const aspect = width / height;
    this.camera.setLensProjection(this.params.focalLength, aspect, 0.1, 5000.0);
  }

  initGUI() {
    const gui = new lil.GUI({ title: "Analytic Skybox" });
    const self = this;
    const sky = this.skybox;

    // Initialize local params from skybox defaults
    // Initialize local params from skybox defaults
    // REMOVED: Do not overwrite this.params from sky.sunDirection (Zenith)
    // const currentDir = sky.sunDirection;
    // this.params.sunTheta = ...

    const updateSun = () => {
      const theta = this.params.sunTheta;
      const phi = this.params.sunPhi;
      const x = Math.sin(theta) * Math.cos(phi);
      const y = Math.cos(theta);
      const z = Math.sin(theta) * Math.sin(phi);
      sky.setSunPosition([x, y, z]);
    };

    const sunFolder = gui.addFolder('Sun');
    // Helper for "Sun Height" cosine slider like C++
    this.sunHeightParam = { height: Math.cos(this.params.sunTheta) };
    sunFolder.add(this.sunHeightParam, 'height', -0.2, 1.0).name('Height (Cos)').onChange(v => {
      this.params.sunTheta = Math.acos(v);
      updateSun();
    });
    sunFolder.add(this.params, 'sunPhi', 0.0, Math.PI * 2).name('Azimuth').onChange(updateSun);
    // Updated: Controls params.sunIntensity and triggers updateSunIntensity
    sunFolder.add(this.params, 'sunIntensity', 0.0, 500000.0).onChange(v => this.updateSunIntensity());

    const sunDisk = sunFolder.addFolder('Disk');
    // We need local proxy for sunRadius due to conversion
    this.diskParams = {
      radius: 1.2,
      enabled: true // Enable sun disk
    };
    sky.setSunDiskEnabled(true);
    sky.setSunRadius(1.2);
    sunDisk.add(this.diskParams, 'enabled').onChange(v => sky.setSunDiskEnabled(v));
    sunDisk.add(this.diskParams, 'radius', 0.0, 5.0).onChange(v => sky.setSunRadius(v));
    sunDisk.add(sky.sunHalo, 1, 0.0, 2.0).name('Limb Darkening').onChange(v => sky.setSunLimbDarkening(v));
    sunDisk.add(sky.sunHalo, 2, 0.0, 100.0).name('Intensity Boost').onChange(v => sky.setSunDiskIntensity(v));

    const atmFolder = gui.addFolder('Atmosphere');
    atmFolder.add(sky, 'turbidity', 1.0, 10.0).onChange(v => sky.setTurbidity(v));
    atmFolder.add(sky, 'rayleigh', 0.0, 10.0).onChange(v => sky.setRayleigh(v));
    atmFolder.add(sky, 'mieCoefficient', 0.0, 10.0).onChange(v => sky.setMieCoefficient(v));
    // Set Ozone default to 0.25
    sky.setOzone(0.25);
    atmFolder.add(sky, 'ozone', 0.0, 1.0).onChange(v => sky.setOzone(v));
    atmFolder.add(sky, 'mieG', 0.0, 0.999).onChange(v => sky.setMieG(v));

    const artFolder = gui.addFolder('Artistic');
    // Set Horizon Glow default to 1.0
    sky.setHorizonGlow(1.0);
    sky.msFactors[2] = 1.0;
    // Set Contrast default to 0.85
    sky.setContrast(0.85);

    artFolder.add(sky.msFactors, 0, 0.0, 2.0).name('MS Rayleigh').onChange(v => sky.setMultiScattering(v, sky.msFactors[1]));
    artFolder.add(sky.msFactors, 1, 0.0, 2.0).name('MS Mie').onChange(v => sky.setMultiScattering(sky.msFactors[0], v));
    artFolder.add(sky.msFactors, 2, 0.0, 1.0).name('Horizon Glow').onChange(v => sky.setHorizonGlow(v));
    artFolder.add(sky, 'contrast', 0.1, 2.0).onChange(v => sky.setContrast(v));

    artFolder.addColor(sky, 'nightColor').onChange(v => sky.setNightColor(v));

    const shmFolder = artFolder.addFolder('Shimmer');
    // Set Shimmer Strength default to 0.0
    sky.setShimmerControl(0.0, sky.shimmerControl[1], sky.shimmerControl[2]);

    shmFolder.add(sky.shimmerControl, 0, 0.0, 0.1).name('Strength').onChange(v => sky.setShimmerControl(v, sky.shimmerControl[1], sky.shimmerControl[2]));
    shmFolder.add(sky.shimmerControl, 1, 1.0, 100.0).name('Frequency').onChange(v => sky.setShimmerControl(sky.shimmerControl[0], v, sky.shimmerControl[2]));
    shmFolder.add(sky.shimmerControl, 2, 0.01, 0.5).name('Mask Height').onChange(v => sky.setShimmerControl(sky.shimmerControl[0], sky.shimmerControl[1], v));

    const cloudFolder = gui.addFolder('Clouds');
    this.cParams = {
      volumetrics: sky.cloudControl2[1] > 0.5,
      coverage: 0.4,
      density: 0.02,
      height: sky.cloudControl[2],
      speed: 50.0,
      evolution: 0.02
    };
    // Apply Cloud Defaults
    sky.setCloudControl(0.4, 0.02, this.cParams.height, 50.0);
    sky.setCloudShapeEvolution(0.02);

    cloudFolder.add(this.cParams, 'volumetrics').onChange(v => sky.setCloudVolumetricLighting(v));
    cloudFolder.add(this.cParams, 'coverage', 0.0, 1.0).onChange(v => sky.setCloudControl(v, this.cParams.density, this.cParams.height, this.cParams.speed));
    cloudFolder.add(this.cParams, 'density', 0.0, 1.0).onChange(v => sky.setCloudControl(this.cParams.coverage, v, this.cParams.height, this.cParams.speed));
    cloudFolder.add(this.cParams, 'height', 2000.0, 20000.0).onChange(v => sky.setCloudControl(this.cParams.coverage, this.cParams.density, v, this.cParams.speed));
    // Reverse speed calc: w = speed * (0.05 / 72.0)
    cloudFolder.add(this.cParams, 'speed', 0.0, 200.0).onChange(v => sky.setCloudControl(this.cParams.coverage, this.cParams.density, this.cParams.height, v));
    cloudFolder.add(this.cParams, 'evolution', 0.0, 2.0).onChange(v => sky.setCloudShapeEvolution(v));

    const waterFolder = gui.addFolder('Water');
    this.wParams = {
      derivativeTrick: true,
      strength: 50.0,
      speed: 1.0,
      octaves: 4.0
    };
    // Initialize defaults
    sky.setWaterControl(50.0, 1.0, 1.0, 4.0); // 1.0 = Derivative Trick On, 4 octaves

    const updateWater = () => {
      sky.setWaterControl(this.wParams.strength, this.wParams.speed, this.wParams.derivativeTrick ? 1.0 : 0.0, this.wParams.octaves);
    };

    waterFolder.add(this.wParams, 'derivativeTrick').name('Derivative Trick').onChange(updateWater);
    waterFolder.add(this.wParams, 'strength', 10.0, 100.0).onChange(updateWater);
    waterFolder.add(this.wParams, 'speed', 0.0, 5.0).onChange(updateWater);
    waterFolder.add(this.wParams, 'octaves', 1, 8, 1).name('Octaves').onChange(updateWater);
    waterFolder.close();

    const starFolder = gui.addFolder('Stars');
    this.sParams = {
      enabled: true,
      density: 1.0
    };
    // Initialize defaults (Density 1.0, Enabled True)
    sky.setStarControl(1.0, true);

    const updateStars = () => {
      sky.setStarControl(this.sParams.density, this.sParams.enabled);
    };

    starFolder.add(this.sParams, 'enabled').name('Enabled').onChange(updateStars);
    starFolder.add(this.sParams, 'density', 0.0, 1.0).name('Density').onChange(updateStars);
    starFolder.close();

    const camFolder = gui.addFolder('Camera');
    camFolder.add(this.params, 'focalLength', 8.0, 300.0).name('Focal Length').onChange(() => this.updateCameraProjection());
    camFolder.add(this.params, 'aperture', 1.4, 32.0).onChange(() => this.updateCameraExposure());
    camFolder.add(this.params, 'shutterSpeed', 1.0, 1000.0).onChange(() => this.updateCameraExposure());
    camFolder.add(this.params, 'iso', 50.0, 3200.0).onChange(() => this.updateCameraExposure());

    const bloomFolder = camFolder.addFolder('Bloom');
    this.bParams = {
      enabled: false,
      lensFlare: false
    };

    const updateBloom = () => {
      this.view.setBloomOptions({
        enabled: this.bParams.enabled,
        lensFlare: this.bParams.lensFlare
      });
    };

    bloomFolder.add(this.bParams, 'enabled').onChange(updateBloom);
    bloomFolder.add(this.bParams, 'lensFlare').onChange(updateBloom);
    bloomFolder.close();

    // Collapse folders by default
    sunDisk.close();
    atmFolder.close();
    artFolder.close();
    // shmFolder is inside artFolder, so it's hidden, but we can close it too if we want
    shmFolder.close();
    cloudFolder.close();
    // camFolder left open? User didn't specify, but "Artistic, shimmer and clouds" + "Disk, Atmosphere" were requested.
    // So Camera might stay open or close. Let's keep Camera open for now as it wasn't listed.

    // Initial sync
    updateSun();
    this.updateCameraExposure(); // This will trigger updateSunIntensity too

    // Check URL for config
    const urlParams = new URLSearchParams(window.location.search);
    if (urlParams.has('config')) {
      try {
        const state = JSON.parse(atob(urlParams.get('config')));
        this.applyURLState(state);
        // Update GUI
        gui.controllers.forEach(c => c.updateDisplay());
        // Recursive update for folders
        gui.folders.forEach(f => {
          f.controllers.forEach(c => c.updateDisplay());
          // And sub-folders if any (Shimmer/Bloom)
          f.folders.forEach(sf => sf.controllers.forEach(sc => sc.updateDisplay()));
        });
      } catch (e) {
        console.error("Failed to load config:", e);
      }
    }

    const shareParams = {
      copyUrl: () => {
        const state = this.getURLState();
        const str = btoa(JSON.stringify(state));
        const url = `${window.location.origin}${window.location.pathname}?config=${str}`;
        navigator.clipboard.writeText(url).then(() => {
          alert("Configuration URL copied to clipboard!");
        }).catch(err => {
          console.error('Could not copy text: ', err);
          prompt("Copy this URL:", url);
        });
      }
    };
    gui.add(shareParams, 'copyUrl').name('Share Configuration');
  }

  getURLState() {
    // Serialize current state (Minified)
    // Mapping:
    // p: params (Camera) -> a:aperture, ss:shutterSpeed, i:iso, st:sunTheta, sp:sunPhi, fl:focalLength, si:sunIntensity
    // c: cParams (Clouds) -> v:volumetrics, co:coverage, d:density, h:height, s:speed, e:evolution
    // w: wParams (Water) -> dt:derivativeTrick, st:strength, s:speed, o:octaves
    // s: sParams (Stars) -> e:enabled, d:density
    // b: bParams (Bloom) -> e:enabled, lf:lensFlare
    // k: sky (Skybox) -> t:turbidity, r:rayleigh, mc:mieCoefficient, mg:mieG, o:ozone, ms:msFactors, co:contrast, nc:nightColor, sh:shimmerControl, hl:sunHalo

    const p = this.params;
    const c = this.cParams;
    const w = this.wParams;
    const s = this.sParams;
    const b = this.bParams;
    const sk = this.skybox;

    return {
      p: { a: p.aperture, ss: p.shutterSpeed, i: p.iso, st: p.sunTheta, sp: p.sunPhi, fl: p.focalLength, si: p.sunIntensity },
      c: { v: c.volumetrics, co: c.coverage, d: c.density, h: c.height, s: c.speed, e: c.evolution },
      w: { dt: w.derivativeTrick, st: w.strength, s: w.speed, o: w.octaves },
      s: { e: s.enabled, d: s.density },
      b: { e: b.enabled, lf: b.lensFlare },
      k: {
        t: sk.turbidity,
        r: sk.rayleigh,
        mc: sk.mieCoefficient,
        mg: sk.mieG,
        o: sk.ozone,
        ms: [...sk.msFactors],
        co: sk.contrast,
        nc: [...sk.nightColor],
        sh: [...sk.shimmerControl],
        hl: [...sk.sunHalo]
      }
    };
  }

  applyURLState(state) {
    const p = state.p;
    const c = state.c;
    const w = state.w;
    const s = state.s;
    const b = state.b;
    const k = state.k;

    if (p) {
      if (p.a !== undefined) this.params.aperture = p.a;
      if (p.ss !== undefined) this.params.shutterSpeed = p.ss;
      if (p.i !== undefined) this.params.iso = p.i;
      if (p.st !== undefined) this.params.sunTheta = p.st;
      if (p.sp !== undefined) this.params.sunPhi = p.sp;
      if (p.fl !== undefined) this.params.focalLength = p.fl;
      if (p.si !== undefined) this.params.sunIntensity = p.si;
    }

    if (c) {
      if (c.v !== undefined) this.cParams.volumetrics = c.v;
      if (c.co !== undefined) this.cParams.coverage = c.co;
      if (c.d !== undefined) this.cParams.density = c.d;
      if (c.h !== undefined) this.cParams.height = c.h;
      if (c.s !== undefined) this.cParams.speed = c.s;
      if (c.e !== undefined) this.cParams.evolution = c.e;
    }

    if (w) {
      if (w.dt !== undefined) this.wParams.derivativeTrick = w.dt;
      if (w.st !== undefined) this.wParams.strength = w.st;
      if (w.s !== undefined) this.wParams.speed = w.s;
      if (w.o !== undefined) this.wParams.octaves = w.o;
    }

    if (s) {
      if (s.e !== undefined) this.sParams.enabled = s.e;
      if (s.d !== undefined) this.sParams.density = s.d;
    }

    if (b) {
      if (b.e !== undefined) this.bParams.enabled = b.e;
      if (b.lf !== undefined) this.bParams.lensFlare = b.lf;
    }

    const sky = this.skybox;
    if (k) {
      if (k.t !== undefined) sky.setTurbidity(k.t);
      if (k.r !== undefined) sky.setRayleigh(k.r);
      if (k.mc !== undefined) sky.setMieCoefficient(k.mc);
      if (k.mg !== undefined) sky.setMieG(k.mg);
      if (k.o !== undefined) sky.setOzone(k.o);

      if (k.ms) { sky.setMultiScattering(k.ms[0], k.ms[1]); sky.setHorizonGlow(k.ms[2]); }
      if (k.co !== undefined) sky.setContrast(k.co);
      if (k.nc) sky.setNightColor(k.nc);
      if (k.sh) sky.setShimmerControl(k.sh[0], k.sh[1], k.sh[2]);

      // Sun Halo
      const savedHalo = k.hl;
      if (savedHalo && savedHalo.length === 4) {
        sky.sunHalo[0] = savedHalo[0];
        sky.sunHalo[1] = savedHalo[1];
        sky.sunHalo[2] = savedHalo[2];
        sky.sunHalo[3] = savedHalo[3];

        // Update derived UI params for Sun Disk
        const rad = Math.acos(Math.max(-1.0, Math.min(1.0, savedHalo[0])));
        this.diskParams.radius = rad * (180.0 / Math.PI);
        this.diskParams.enabled = savedHalo[3] > 0.5;
      }

      sky.updateCoefficients();
    }

    // Update derived Sun Height param for UI
    if (this.sunHeightParam) {
      this.sunHeightParam.height = Math.cos(this.params.sunTheta);
    }

    // Apply Local Params via Setters
    sky.setCloudControl(this.cParams.coverage, this.cParams.density, this.cParams.height, this.cParams.speed);
    sky.setCloudVolumetricLighting(this.cParams.volumetrics);
    sky.setCloudShapeEvolution(this.cParams.evolution);

    sky.setWaterControl(this.wParams.strength, this.wParams.speed, this.wParams.derivativeTrick ? 1.0 : 0.0, this.wParams.octaves);
    sky.setStarControl(this.sParams.density, this.sParams.enabled);

    this.view.setBloomOptions({
      enabled: this.bParams.enabled,
      lensFlare: this.bParams.lensFlare
    });

    // Update Sun Position from Params
    const theta = this.params.sunTheta;
    const phi = this.params.sunPhi;
    const x = Math.sin(theta) * Math.cos(phi);
    const y = Math.cos(theta);
    const z = Math.sin(theta) * Math.sin(phi);
    sky.setSunPosition([x, y, z]);
    this.updateSunIntensity();
  }

  initControls() {

    // listeners only
    this.canvas.addEventListener('mousedown', e => {
      this.camState.dragging = true;
      this.camState.lastX = e.clientX;
      this.camState.lastY = e.clientY;
    });

    window.addEventListener('mouseup', () => {
      this.camState.dragging = false;
    });

    window.addEventListener('mousemove', e => {
      if (!this.camState.dragging) return;
      const dx = e.clientX - this.camState.lastX;
      const dy = e.clientY - this.camState.lastY;
      this.camState.lastX = e.clientX;
      this.camState.lastY = e.clientY;

      const sensitivity = 0.005;
      this.camState.theta -= dx * sensitivity;
      this.camState.phi += dy * sensitivity;
      // Clamp pitch to avoid flip [ -PI/2, PI/2 ]
      this.camState.phi = Math.max(-Math.PI / 2 + 0.01, Math.min(Math.PI / 2 - 0.01, this.camState.phi));
    });


  }

  render() {
    // Update Camera LookAt
    const r = 1.0;
    const theta = this.camState.theta;
    const phi = this.camState.phi;

    // Convert spherical to cartesian
    // Y is UP. Z is Forward.
    // At phi=0, y=0. forward vector should correspond to theta.
    // Let's say theta=0 is -Z.
    const y = Math.sin(phi);
    const h = Math.cos(phi);
    const x = h * Math.sin(theta);
    const z = -h * Math.cos(theta);

    const eye = [0, 0, 0];
    const center = [x, y, z];
    const up = [0, 1, 0];
    this.camera.lookAt(eye, center, up);

    this.renderer.render(this.swapChain, this.view);
    window.requestAnimationFrame(this.render);
  }

  resize() {
    const dpr = window.devicePixelRatio;
    const width = this.canvas.width = window.innerWidth * dpr;
    const height = this.canvas.height = window.innerHeight * dpr;
    this.view.setViewport([0, 0, width, height]);

    const aspect = width / height;
    // near=0.1, far=5000.0
    this.camera.setLensProjection(this.params.focalLength, aspect, 0.1, 5000.0);
  }
}
