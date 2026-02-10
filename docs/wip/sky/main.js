
// main.js

Filament.init(['assets/simulated_skybox.filamat?v=' + Date.now()], () => {
  window.app = new App(document.getElementsByTagName('canvas')[0]);
});

// Helper: Julian Date
function getJD(date) {
  return date.getTime() / 86400000.0 + 2440587.5;
}

// Helper: GMST from Date
function getGMST(date) {
  const JD = getJD(date);
  const D = JD - 2451545.0;
  // GMST = 18.697... + 24.0657... * D
  let gmst = 18.697374558 + 24.06570982441908 * D;
  gmst = gmst % 24.0;
  if (gmst < 0) gmst += 24.0;
  return gmst;
}

// Helper: Matrix Rotation
function rotateX(m, angle) {
  const c = Math.cos(angle);
  const s = Math.sin(angle);
  const m1 = m[1], m2 = m[2];
  const m4 = m[4], m5 = m[5];
  const m7 = m[7], m8 = m[8];
  m[1] = m1 * c - m2 * s;
  m[2] = m1 * s + m2 * c;
  m[4] = m4 * c - m5 * s;
  m[5] = m4 * s + m5 * c;
  m[7] = m7 * c - m8 * s;
  m[8] = m7 * s + m8 * c;
}
function rotateY(m, angle) {
  const c = Math.cos(angle);
  const s = Math.sin(angle);
  const m0 = m[0], m2 = m[2];
  const m3 = m[3], m5 = m[5];
  const m6 = m[6], m8 = m[8];
  m[0] = m0 * c + m2 * s;
  m[2] = -m0 * s + m2 * c;
  m[3] = m3 * c + m5 * s;
  m[5] = -m3 * s + m5 * c;
  m[6] = m6 * c + m8 * s;
  m[8] = -m6 * s + m8 * c;
}
function rotateZ(m, angle) {
  const c = Math.cos(angle);
  const s = Math.sin(angle);
  const m0 = m[0], m1 = m[1];
  const m3 = m[3], m4 = m[4];
  const m6 = m[6], m7 = m[7];
  m[0] = m0 * c - m1 * s;
  m[1] = m0 * s + m1 * c;
  m[3] = m3 * c - m4 * s;
  m[4] = m3 * s + m4 * c;
  m[6] = m6 * c - m7 * s;
  m[7] = m6 * s + m7 * c;
}

// Galactic to Equatorial (J2000)
// This matrix converts Galactic vectors to Equatorial vectors.
// Or effectively, if we want to render Galactic texture from Equatorial View vector V_eq:
// V_gal = Inv(Rot_Gal_to_Eq) * V_eq = Rot_Eq_to_Gal * V_eq.
// The shader does: V_gal = Rotation * V_world.
// So Rotation = Rot_Eq_to_Gal * Rot_World_to_Eq.
//
// Galactic North Pole (J2000): RA = 192.85948, Dec = 27.12825
// Ascending Node: RA = 282.85
// 
// Pre-computed Rotation Matrix (Equatorial -> Galactic)
// Based on standard transformation matrices.
// 
// R_eq_gal = 
// [ -0.054876  -0.873437  -0.483835 ]
// [  0.494109  -0.444830   0.746982 ]
// [ -0.867666  -0.198076   0.455984 ]
//
// Let's use this static definition.
const MAT_EQ_TO_GAL = [
  -0.054876, 0.494109, -0.867666,
  -0.873437, -0.444830, -0.198076,
  -0.483835, 0.746982, 0.455984
];

// Matrix multiplication 3x3
function multiplyMat3(a, b) {
  const out = new Float32Array(9);
  // Row-major or Column-major? Filament is Column-major usually.
  // GLSL is Column-major.
  // Mat3 in array: [col0.x, col0.y, col0.z, col1.x, ...]
  // So a[0] is (0,0), a[1] is (1,0), a[3] is (0,1).
  //
  // out = a * b
  const a00 = a[0], a10 = a[1], a20 = a[2];
  const a01 = a[3], a11 = a[4], a21 = a[5];
  const a02 = a[6], a12 = a[7], a22 = a[8];

  const b00 = b[0], b10 = b[1], b20 = b[2];
  const b01 = b[3], b11 = b[4], b21 = b[5];
  const b02 = b[6], b12 = b[7], b22 = b[8];

  out[0] = a00 * b00 + a01 * b10 + a02 * b20;
  out[1] = a10 * b00 + a11 * b10 + a12 * b20;
  out[2] = a20 * b00 + a21 * b10 + a22 * b20;

  out[3] = a00 * b01 + a01 * b11 + a02 * b21;
  out[4] = a10 * b01 + a11 * b11 + a12 * b21;
  out[5] = a20 * b01 + a21 * b11 + a22 * b21;

  out[6] = a00 * b02 + a01 * b12 + a02 * b22;
  out[7] = a10 * b02 + a11 * b12 + a12 * b22;
  out[8] = a20 * b02 + a21 * b12 + a22 * b22;
  return out;
}

class App {
  constructor(canvas) {
    this.canvas = canvas;
    const engine = this.engine = Filament.Engine.create(this.canvas);
    this.scene = engine.createScene();

    this.skybox = new SimulatedSkybox(engine);
    this.skybox.entity = this.skybox.entity; // Ensuring access if needed
    this.scene.addEntity(this.skybox.entity);

    // Load the material explicitly. SimulatedSkybox.loadMaterial fetches it.

    const matUrl = 'assets/simulated_skybox.filamat?v=' + Date.now();
    this.skybox.loadMaterial(matUrl).then(() => {
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

    this.mwParams = {
      enabled: true,
      intensity: 1.0,
      saturation: 1.0,
      blackPoint: 0.07,
      siderealTime: 0.0, // Hours [0-24]
      latitude: 34.0, // Default Lat
    };

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

    // Moon Exposure
    if (this.mParams) {
      const preExposedMoon = this.mParams.intensity * exposure;
      this.skybox.setMoonIntensity(preExposedMoon);
    }
  }

  updateCameraProjection() {
    const width = this.canvas.width;
    const height = this.canvas.height;
    const aspect = width / height;
    this.camera.setLensProjection(this.params.focalLength, aspect, 0.1, 5000.0);
    if (this.skybox) this.skybox.setFocalLength(this.params.focalLength);
  }

  initGUI() {
    const gui = new lil.GUI({ title: "Analytic Skybox" });
    const self = this;
    const sky = this.skybox;

    // Initialize local params from skybox defaults


    const updateSun = () => {
      const theta = this.params.sunTheta;
      const phi = this.params.sunPhi;
      const x = Math.sin(theta) * Math.cos(phi);
      const y = Math.cos(theta);
      const z = Math.sin(theta) * Math.sin(phi);
      sky.setSunPosition([x, y, z]);
    };

    // Sun UI Proxy
    this.sunUI = {
      azimuth: (this.params.sunPhi * 180.0 / Math.PI) % 360.0,
      height: Math.cos(this.params.sunTheta)
    };
    if (this.sunUI.azimuth < 0) this.sunUI.azimuth += 360.0;

    const sunFolder = gui.addFolder('Sun');

    sunFolder.add(this.sunUI, 'azimuth', 0.0, 360.0, 0.1).name('Azimuth').listen().onChange(v => {
      this.params.sunPhi = v * (Math.PI / 180.0);
      updateSun();
    });

    sunFolder.add(this.sunUI, 'height', -0.2, 1.0).name('Height (Cos)').listen().onChange(v => {
      this.params.sunTheta = Math.acos(v);
      updateSun();
    });

    sunFolder.add(this.params, 'sunIntensity', 0.0, 500000.0).name('Intensity').onChange(v => this.updateSunIntensity());




    const moonFolder = gui.addFolder('Moon');
    this.mParams = {
      enabled: true,
      azimuth: 180.0,
      height: Math.cos(45.0 * Math.PI / 180.0), // Default 45 degrees elevation -> cos(45) ~ 0.707
      radius: 1.2,
      intensity: 6.0
    };

    const updateMoon = () => {
      const az = this.mParams.azimuth * (Math.PI / 180.0);
      const theta = Math.acos(this.mParams.height);
      const phi = az;

      const x = Math.sin(theta) * Math.cos(phi);
      const y = Math.cos(theta);
      const z = Math.sin(theta) * Math.sin(phi);

      sky.setMoonPosition([x, y, z]);
    };

    // Initial Moon Sync
    updateMoon();
    sky.setMoonEnabled(this.mParams.enabled);
    sky.setMoonRadius(this.mParams.radius);
    sky.setMoonIntensity(this.mParams.intensity);

    moonFolder.add(this.mParams, 'enabled').name('Enabled').onChange(v => sky.setMoonEnabled(v));
    moonFolder.add(this.mParams, 'azimuth', 0.0, 360.0, 0.1).name('Azimuth').listen().onChange(updateMoon);
    moonFolder.add(this.mParams, 'height', -0.2, 1.0).name('Height (Cos)').listen().onChange(updateMoon);
    moonFolder.add(this.mParams, 'intensity', 0.0, 1000.0).name('Intensity').onChange(v => this.updateSunIntensity());
    moonFolder.add(this.mParams, 'radius', 0.1, 5.0).name('Radius').onChange(v => sky.setMoonRadius(v));
    moonFolder.close();

    const mwFolder = gui.addFolder('Milky Way');

    const updateMW = () => {
      sky.setMilkyWayEnabled(this.mwParams.enabled);
      sky.setMilkyWayControl(this.mwParams.intensity, this.mwParams.saturation, this.mwParams.blackPoint);

      // Calculate Rotation
      // V_gal = Rot_Eq_to_Gal * Rot_World_to_Eq * V_world

      // World: Y=Up, X=East, Z=South (Filament Camera Convention is different!)
      // In Filament Camera: -Z is Forward.
      // Skybox V direction is World Space direction.
      // Let's assume standard Horizontal Coordinates:
      // Y = Zenith.
      // Z = North? Or South?
      // Usually Z is South in RH Y-up.

      // LST (Local Sidereal Time) converts Hour Angle to RA.
      // LST in Radians.
      const LST = this.mwParams.siderealTime * (Math.PI / 12.0); // Hours to Rad
      const Lat = this.mwParams.latitude * (Math.PI / 180.0);

      // Rotation World (Horizontal) -> Equatorial
      // 1. Rotate around X by -(90 - Lat) to align Equatorial Plane.
      // 2. Rotate around Y (Polar Axis) by -LST.

      // Mat3 Identity
      const rot = [1, 0, 0, 0, 1, 0, 0, 0, 1];

      // Rotate Z by LST (Earth Rotation).
      // Actually, transformation from Horizontal (Az, Alt) to Equatorial (HA, Dec):
      // sin(Dec) = sin(Alt)sin(Lat) - cos(Alt)cos(Lat)cos(Az)
      // ...
      // Let's construct matrix directly.
      // WorldToEq:
      // Rotate X by (Lat - 90 deg) -> brings Pole to Zenith.
      // Rotate Y by -LST (or Z?)

      // Filament Space:
      // +Y = Up
      // Let's match typical skybox conventions.

      // Rot_World_to_Eq = Rot_Z_LST * Rot_X_Lat
      // But we need to use Filament matrix ops which are column major.

      // Let's use simple rotations:
      // 1. Tilt Pole: Rotate X by (Lat - 90).
      // 2. Spin Earth: Rotate Y by LST.

      // Let's iterate until it looks right visually or trust the math.
      // Rot_World_To_Equatorial:
      // R_z(-LST) * R_x(Lat - 90)?

      // Let's build it from scratch in JS using helper.
      // Start Identity.
      // Rotate X (Latitude Tilt).
      // Rotate Y (Sidereal Spin).
      // Note: rotate functions modify in place.

      const mWorldToEq = [1, 0, 0, 0, 1, 0, 0, 0, 1];

      // 1. Tilt for Latitude (Align Celestial Pole)
      // At Lat 90 (North Pole), Zenith is Pole. No tilt needed if Y is Pole?
      // No, Y is Zenith. Pole is Y.
      // At Lat 0 (Equator), Pole is at Horizon (Z?).
      // So we rotate X by (Lat - 90).
      rotateX(mWorldToEq, Lat - Math.PI / 2);

      // 2. Spin for Time (LST)
      // Rotate around new Pole (Y) by LST.
      rotateY(mWorldToEq, LST);

      // Combine with Gal Transform
      // Rot = MAT_EQ_TO_GAL * mWorldToEq
      const finalRot = multiplyMat3(MAT_EQ_TO_GAL, mWorldToEq);

      sky.setMilkyWayRotation(finalRot);
    };

    mwFolder.add(this.mwParams, 'enabled').name('Enabled').onChange(updateMW);
    mwFolder.add(this.mwParams, 'intensity', 0.0, 5.0).onChange(updateMW);
    mwFolder.add(this.mwParams, 'saturation', 0.0, 2.0).onChange(updateMW);
    mwFolder.add(this.mwParams, 'blackPoint', 0.0, 0.5).name('Black Point').onChange(updateMW);
    mwFolder.add(this.mwParams, 'siderealTime', 0.0, 24.0).name('Sidereal Time').listen().onChange(updateMW);
    mwFolder.add(this.mwParams, 'latitude', -90.0, 90.0).name('Latitude').onChange(updateMW);
    mwFolder.close();

    // Initial MW Update
    updateMW();

    this.updateMW = updateMW; // Export for sync

    // We need local proxy for sunRadius due to conversion
    this.diskParams = {
      radius: 1.2
    };
    sky.setSunDiskEnabled(true);
    sky.setSunRadius(1.2);
    sunFolder.add(this.diskParams, 'radius', 0.0, 5.0).onChange(v => sky.setSunRadius(v));
    sunFolder.add(sky.sunHalo, 1, 0.0, 2.0).name('Limb Darkening').onChange(v => sky.setSunLimbDarkening(v));
    sunFolder.add(sky.sunHalo, 2, 0.0, 100.0).name('Intensity Boost').onChange(v => sky.setSunDiskIntensity(v));

    const atmFolder = gui.addFolder('Atmosphere');
    atmFolder.add(sky, 'turbidity', 1.0, 10.0).onChange(v => sky.setTurbidity(v));
    atmFolder.add(sky, 'rayleigh', 0.0, 10.0).onChange(v => sky.setRayleigh(v));
    atmFolder.add(sky, 'mieCoefficient', 0.0, 10.0).onChange(v => sky.setMieCoefficient(v));
    // Set Ozone default to 0.25
    sky.setOzone(0.25);
    atmFolder.add(sky, 'ozone', 0.0, 1.0).onChange(v => sky.setOzone(v));
    atmFolder.add(sky, 'mieG', 0.0, 0.999).onChange(v => sky.setMieG(v));

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
      density: 0.001
    };
    // Initialize defaults (Density 0.001, Enabled True)
    sky.setStarControl(0.001, true);

    const updateStars = () => {
      sky.setStarControl(this.sParams.density, this.sParams.enabled);
    };

    starFolder.add(this.sParams, 'enabled').name('Enabled').onChange(updateStars);
    starFolder.add(this.sParams, 'density', 0.0, 0.01, 0.0001).name('Density').onChange(updateStars);
    starFolder.close();

    const artFolder = gui.addFolder('Artistic');
    // Set Horizon Glow default to 0.0
    sky.setHorizonGlow(0.0);
    sky.msFactors[2] = 0.0;
    // Set Contrast default to 1.0
    sky.setContrast(1.0);

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

    const camFolder = gui.addFolder('Camera');
    camFolder.add(this.params, 'focalLength', 8.0, 300.0).name('Focal Length').onChange(() => this.updateCameraProjection());
    camFolder.add(this.params, 'aperture', 1.4, 32.0).onChange(() => this.updateCameraExposure());
    camFolder.add(this.params, 'shutterSpeed', 1.0, 1000.0).onChange(() => this.updateCameraExposure());
    camFolder.add(this.params, 'iso', 50.0, 3200.0).onChange(() => this.updateCameraExposure());

    const bloomFolder = camFolder.addFolder('Bloom');
    this.bParams = {
      enabled: true,
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

    atmFolder.close();
    artFolder.close();
    // shmFolder is inside artFolder, so it's hidden, but we can close it too if we want
    shmFolder.close();
    cloudFolder.close();
    // camFolder left open by default for convenience.


    // Initial sync
    updateSun();
    this.updateCameraExposure(); // This will trigger updateSunIntensity too
    updateBloom();

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

    const syncFolder = gui.addFolder('Real-Time Sync');
    this.syncParams = {
      enabled: false,
      lat: 0.0,
      lng: 0.0,
      status: 'Disabled'
    };

    const updateSync = () => {
      if (this.syncParams.enabled) {
        if (navigator.geolocation) {
          this.syncParams.status = "Locating...";
          navigator.geolocation.getCurrentPosition(
            (pos) => {
              this.syncParams.lat = pos.coords.latitude;
              this.syncParams.lng = pos.coords.longitude;
              this.syncParams.status = "Active";
              syncFolder.controllers.forEach(c => c.updateDisplay());
            },
            (err) => {
              console.error(err);
              this.syncParams.status = "Error (See Console)";
              this.syncParams.enabled = false;
              syncFolder.controllers.forEach(c => c.updateDisplay());
            }
          );
        } else {
          this.syncParams.status = "Not Supported";
        }
      } else {
        this.syncParams.status = "Disabled";
      }
      syncFolder.controllers.forEach(c => c.updateDisplay());
    };

    syncFolder.add(this.syncParams, 'enabled').name('Enable Sync').onChange(updateSync);
    syncFolder.add(this.syncParams, 'status').name('Status').disable().listen();

    this.syncFolder = syncFolder;

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

  updateRealTimeSync() {
    if (!this.syncParams || !this.syncParams.enabled || !window.SunCalc) return;

    const now = new Date();
    const lat = this.syncParams.lat;
    const lng = this.syncParams.lng;

    // Sun
    const sunPos = window.SunCalc.getPosition(now, lat, lng);
    // Azimuth: South=0, West=PI/2.
    // Skybox Phi: +X=0, +Z=PI/2.
    // If +Z is South:
    // SunAz 0 (South) -> Skybox PI/2 (+Z).
    // SunAz PI/2 (West) -> Skybox PI (-X).
    // So Phi = Az + PI/2.
    const sunPhi = sunPos.azimuth + Math.PI / 2;
    // Altitude: 0=Horizon, PI/2=Zenith.
    // Skybox Theta: 0=Zenith, PI/2=Horizon.
    const sunTheta = Math.PI / 2 - sunPos.altitude;

    this.params.sunPhi = sunPhi;
    this.params.sunTheta = sunTheta;

    // Moon
    const moonPos = window.SunCalc.getMoonPosition(now, lat, lng);
    const moonPhi = moonPos.azimuth + Math.PI / 2;
    const moonTheta = Math.PI / 2 - moonPos.altitude;

    this.mParams.azimuth = (moonPhi * 180.0 / Math.PI) % 360.0;
    this.mParams.height = Math.cos(moonTheta);

    // Milky Way Sync
    const gmst = getGMST(now);
    const lst = (gmst + lng / 15.0 + 24.0) % 24.0;
    this.mwParams.siderealTime = lst;
    this.mwParams.latitude = lat;
    if (this.updateMW) this.updateMW();

    // Update Skybox
    const sky = this.skybox;

    // Update Sun Vector
    const sx = Math.sin(sunTheta) * Math.cos(sunPhi);
    const sy = Math.cos(sunTheta);
    const sz = Math.sin(sunTheta) * Math.sin(sunPhi);
    sky.setSunPosition([sx, sy, sz]);

    // Update Moon Vector
    const mx = Math.sin(moonTheta) * Math.cos(moonPhi);
    const my = Math.cos(moonTheta);
    const mz = Math.sin(moonTheta) * Math.sin(moonPhi);
    sky.setMoonPosition([mx, my, mz]);

    // Update UI Proxies
    if (this.sunUI) {
      this.sunUI.azimuth = (sunPhi * 180.0 / Math.PI) % 360.0;
      if (this.sunUI.azimuth < 0) this.sunUI.azimuth += 360.0;
      this.sunUI.height = Math.cos(sunTheta);
    }
  }

  getURLState() {

    // Update Camera LookAt
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
    const m = this.mParams;
    const sk = this.skybox;

    return {
      p: { a: p.aperture, ss: p.shutterSpeed, i: p.iso, st: p.sunTheta, sp: p.sunPhi, fl: p.focalLength, si: p.sunIntensity },
      c: { v: c.volumetrics, co: c.coverage, d: c.density, h: c.height, s: c.speed, e: c.evolution },
      w: { dt: w.derivativeTrick, st: w.strength, s: w.speed, o: w.octaves },
      s: { e: s.enabled, d: s.density },
      b: { e: b.enabled, lf: b.lensFlare },
      m: { e: m.enabled, az: m.azimuth, h: m.height, r: m.radius, i: m.intensity },
      cm: { t: this.camState.theta, p: this.camState.phi },
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
    const m = state.m;
    const k = state.k;
    const cm = state.cm;

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

    if (cm) {
      if (cm.t !== undefined) this.camState.theta = cm.t;
      if (cm.p !== undefined) this.camState.phi = cm.p;
    }

    // Update derived Sun UI
    if (this.sunUI) {
      this.sunUI.height = Math.cos(this.params.sunTheta);
      this.sunUI.azimuth = (this.params.sunPhi * 180.0 / Math.PI) % 360.0;
      if (this.sunUI.azimuth < 0) this.sunUI.azimuth += 360.0;
    }

    // Apply Local Params via Setters
    sky.setCloudControl(this.cParams.coverage, this.cParams.density, this.cParams.height, this.cParams.speed);
    sky.setCloudVolumetricLighting(this.cParams.volumetrics);
    sky.setCloudShapeEvolution(this.cParams.evolution);

    sky.setWaterControl(this.wParams.strength, this.wParams.speed, this.wParams.derivativeTrick ? 1.0 : 0.0, this.wParams.octaves);
    sky.setStarControl(this.sParams.density, this.sParams.enabled);

    if (m) {
      if (m.e !== undefined) this.mParams.enabled = m.e;
      if (m.az !== undefined) this.mParams.azimuth = m.az;
      // Compat: if 'el' exists (old link) convert to 'h'
      if (m.h !== undefined) {
        this.mParams.height = m.h;
      } else if (m.el !== undefined) {
        // Convert elevation degrees to height cos
        this.mParams.height = Math.cos(m.el * Math.PI / 180.0);
      }
      if (m.r !== undefined) this.mParams.radius = m.r;
      if (m.i !== undefined) this.mParams.intensity = m.i;

      // Sync Moon
      const az = this.mParams.azimuth * (Math.PI / 180.0);
      const theta = Math.acos(this.mParams.height);
      const phi = az;
      const x = Math.sin(theta) * Math.cos(phi);
      const y = Math.cos(theta);
      const z = Math.sin(theta) * Math.sin(phi);

      sky.setMoonPosition([x, y, z]);
      sky.setMoonEnabled(this.mParams.enabled);
      sky.setMoonRadius(this.mParams.radius);
      sky.setMoonIntensity(this.mParams.intensity);
    }

    if (cm) {
      if (cm.t !== undefined) this.camState.theta = cm.t;
      if (cm.p !== undefined) this.camState.phi = cm.p;
    }

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

    // Update Camera Projection (Focal Length) and Exposure (Aperture/Shutter/ISO)
    this.updateCameraProjection();
    this.updateCameraExposure();
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

    // Touch support
    this.canvas.addEventListener('touchstart', e => {
      if (e.touches.length > 0) {
        e.preventDefault(); // Prevent scroll/long-press
        this.camState.dragging = true;
        this.camState.lastX = e.touches[0].clientX;
        this.camState.lastY = e.touches[0].clientY;
      }
    }, { passive: false });

    window.addEventListener('touchend', () => {
      this.camState.dragging = false;
    });

    window.addEventListener('touchmove', e => {
      if (!this.camState.dragging || e.touches.length === 0) return;
      e.preventDefault(); // Prevent scrolling
      const x = e.touches[0].clientX;
      const y = e.touches[0].clientY;
      const dx = x - this.camState.lastX;
      const dy = y - this.camState.lastY;
      this.camState.lastX = x;
      this.camState.lastY = y;

      const sensitivity = 0.005;
      this.camState.theta -= dx * sensitivity;
      this.camState.phi += dy * sensitivity;
      this.camState.phi = Math.max(-Math.PI / 2 + 0.01, Math.min(Math.PI / 2 - 0.01, this.camState.phi));
    }, { passive: false });


  }

  render() {
    this.updateRealTimeSync();
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
    if (this.skybox) this.skybox.setResolution(height);
  }
}
