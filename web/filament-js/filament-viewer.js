/*
* Copyright (C) 2021 The Android Open Source Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

// If you are bundling this with rollup, webpack, or esbuild, the following URL should be trimmed.
import { LitElement, html, css } from "https://unpkg.com/lit@2.8.0?module";

// This little utility checks if the Filament module is ready for action.
// If so, it immediately calls the given function. If not, it asks the Filament
// loader to call it as soon as the module becomes ready.
class FilamentTasks {
    add(callback) {
        if (Filament.isReady) {
            callback();
        } else {
            Filament.init([], callback);
        }
    }
}

// FilamentViewer is a fairly limited web component for showing glTF models with Filament.
//
// To embed a 3D viewer in your web page, simply add something like the following to your HTML,
// similar to an <img> element.
//
//     <filament-viewer src="FlightHelmet.gltf" />
//
// In addition to the src URL, attributes can be used to set up an optional IBL and skybox.
// The documentation for these attributes is in the FilamentViewer constructor.
//
// MISSING FEATURES
// ----------------
// None of the following features are implemented. They would be easy to add.
// - Replace gltumble and Trackball with camutils Manipulator (i.e. enable scroll-to-zoom)
// - Write a documentation page (might be neat if the doc page has instances of the actual viewer)
// - Fix the import at the top of the file to support webpack / rollup / esbuild
// - Expose more animation properties (e.g. enable / disable, selected index)
// - Expose camera properties, glTF camera selection, and clear color
// - Expose more IBL properties
// - Expose directional light properties
// - Optional turntable animation
//
class FilamentViewer extends LitElement {
    constructor() {
        super();

        // LitElement properties:
        this.src = null;          // Path to glTF file.
        this.alt = null;          // Alternate canvas content.
        this.ibl = null;          // Path to image based light ktx.
        this.sky = null;          // Path to skybox ktx.
        this.enableDrop = null;   // Enables drag and drop.
        this.intensity = 30000;   // Intensity of the image based light.
        this.materialVariant = 0; // Index of material variant.

        // Private properties:
        this.filamentTasks = new FilamentTasks();
        this.canvasId = "filament-viewer-canvas";
        this.overlayId = "filament-viewer-overlay";
        this.srcBlob = null;
    }

    static get properties() {
        return {
            src: { type: String },
            alt: { type: String },
            ibl: { type: String },
            sky: { type: String },
            enableDrop: { type: Boolean },
            intensity: { type: Number },
            materialVariant: { type: Number },
        }
    }

    firstUpdated() {
        // At this point in the lit-element lifecycle, the "render" has taken place, which simply
        // means the canvas element now exists. However the Filament wasm module may or may not be
        // fully loaded, which is why we use the task manager.
        const canvas = this.shadowRoot.getElementById(this.canvasId);
        if (canvas.parentNode.host.parentElement.tagName === "FILAMENT-VIEWER") {
            console.error("Do not nest FilamentViewer, this is unsupported.");
            console.error("Try placing each viewer in a wrapper element.");
            return;
        }
        this.filamentTasks.add(this._startFilament.bind(this));

        const overlay = this.shadowRoot.getElementById(this.overlayId);
        ["dragenter", "dragover", "dragleave", "drop"].forEach(eventName => {
            overlay.addEventListener(eventName, e => { e.preventDefault(); e.stopPropagation() }, false)
        });
        if (this.enableDrop) {
            overlay.addEventListener("drop", this._dropHandler.bind(this), false);
        }
    }

    updated(props) {
        if (props.has("src")) this.filamentTasks.add(this._loadAsset.bind(this));
        if (props.has("ibl")) this.filamentTasks.add(this._loadIbl.bind(this));
        if (props.has("sky")) this.filamentTasks.add(this._loadSky.bind(this));
        if (props.has("enableDrop")) this._updateOverlay();
        if (props.has("intensity") && this.indirectLight) {
            this.indirectLight.setIntensity(this.intensity);
        }
        if (props.has("materialVariant") && this.asset) this._applyMaterialVariant();
    }

    static get styles() {
        return css`
            :host {
                display: inline-block;
                width: 300px;
                height: 300px;
                border: solid 1px black;
                position: relative;
            }
            canvas {
                background: #D9D9D9; /* This is consistent with the default clear color. */
            }
            canvas, .overlay {
                width: 100%;
                height: 100%;
                position: absolute;
            }
            .overlay {
                text-align: center;
                padding: 10px;
            }`;
    }

    render() {
        return html`
            <canvas part="canvas" alt="${this.alt}" id="${this.canvasId}"></canvas>
            <div class="overlay" part="overlay" id="${this.overlayId}"></div>
        `;
    }

    _dropHandler(dragEvent) {
        if (!dragEvent.dataTransfer) return;
        this.srcBlob = null;
        this.srcBlobResources = {};
        for (const file of dragEvent.dataTransfer.files) {
            if (file.name.endsWith(".glb") || file.name.endsWith(".gltf")) {
                this.srcBlob = file;
            } else {
                this.srcBlobResources[file.name] = file;
            }
        }
        if (this.srcBlob) {
            this._loadAsset();
        } else {
            console.error("Please include a glTF file.");
        }
    };

    _updateOverlay() {
        const overlay = this.shadowRoot.getElementById(this.overlayId);
        if (!this.enableDrop || this.asset) {
            overlay.innerHTML = "";
        } else {
            overlay.innerHTML = "Drop glb or file set here.";
        }
    }

    _startFilament() {
        const LightType = Filament.LightManager$Type;

        const canvas = this.shadowRoot.getElementById(this.canvasId);
        const overlay = this.shadowRoot.getElementById(this.overlayId);

        this.engine = Filament.Engine.create(canvas);
        this.scene = this.engine.createScene();
        this.sunlight = Filament.EntityManager.get().create();
        this.scene.addEntity(this.sunlight);
        this.loader = this.engine.createAssetLoader();
        this.cameraEntity = Filament.EntityManager.get().create();
        this.camera = this.engine.createCamera(this.cameraEntity);
        this.swapChain = this.engine.createSwapChain();
        this.renderer = this.engine.createRenderer();
        this.view = this.engine.createView();
        this.view.setVignetteOptions({ midPoint: 0.7, enabled: true });
        this.view.setCamera(this.camera);
        this.view.setScene(this.scene);

        // If gltumble has been loaded, use it.
        if (window.Trackball) {
            this.trackball = new Trackball(overlay, { startSpin: 0.0 });
        }

        // This color is consistent with the default CSS background color.
        this.renderer.setClearOptions({ clearColor: [0.8, 0.8, 0.8, 1.0], clear: true });

        Filament.LightManager.Builder(LightType.SUN)
            .direction([0, -.7, -.7])
            .sunAngularRadius(1.9)
            .castShadows(true)
            .sunHaloSize(10.0)
            .sunHaloFalloff(80.0)
            .build(this.engine, this.sunlight);

        // TODO: if ResizeObserver is not supported, then we should call _onResized and
        // pass (canvas.clientWidth * window.devicePixelRatio) for the dimensions.
        var ro = new ResizeObserver(entries => {
            const width = entries[0].devicePixelContentBoxSize[0].inlineSize;
            const height = entries[0].devicePixelContentBoxSize[0].blockSize;
            this._onResized(width, height);
        });
        ro.observe(canvas);

        window.requestAnimationFrame(this._renderFrame.bind(this));
    }

    _onResized(width, height) {
        const Fov = Filament.Camera$Fov;
        const canvas = this.shadowRoot.getElementById(this.canvasId);
        canvas.width = width;
        canvas.height = height;
        this.view.setViewport([0, 0, width, height]);
        const y = -0.125, eye = [0, y, 1.5], center = [0, y, 0], up = [0, 1, 0];
        this.camera.lookAt(eye, center, up);
        const aspect = width / height;
        const fov = aspect < 1 ? Fov.HORIZONTAL : Fov.VERTICAL;
        this.camera.setProjectionFov(25, aspect, 1.0, 10.0, fov);
    }

    _loadIbl() {
        if (!this.ibl) {
            return;
        }
        if (this.indirectLight) {
            console.info("FilamentViewer does not allow the IBL to be changed.");
            return;
        }
        fetch(this.ibl).then(response => {
            return response.arrayBuffer();
        }).then(arrayBuffer => {
            const ktxData = new Uint8Array(arrayBuffer);
            this.indirectLight = this.engine.createIblFromKtx1(ktxData);
            this.indirectLight.setIntensity(this.intensity);
            this.scene.setIndirectLight(this.indirectLight);
        });
    }

    _loadSky() {
        if (!this.sky) {
            return;
        }
        if (this.skybox) {
            console.info("FilamentViewer does not allow the skybox to be changed.");
            return;
        }
        fetch(this.sky).then(response => {
            return response.arrayBuffer();
        }).then(arrayBuffer => {
            const ktxData = new Uint8Array(arrayBuffer);
            this.skybox = this.engine.createSkyFromKtx1(ktxData);
            this.scene.setSkybox(this.skybox);
        });
    }

    _loadAsset() {
        const zoffset = 4;

        if (this.asset) {
            this.scene.removeEntities(this.asset.getEntities());
            this.animator = null;
            this.asset = null;
        }

        // If we have neither a URL nor a dropped file, leave early.
        if (!this.src && !this.srcBlob) {
            this._updateOverlay();
            return;
        }

        // Dropping a glb file is simple because there are no external resources.
        if (this.srcBlob && this.srcBlob.name.endsWith(".glb")) {
            this.srcBlob.arrayBuffer().then(buffer => {
                this.asset = this.loader.createAsset(new Uint8Array(buffer));
                const aabb = this.asset.getBoundingBox();
                this.assetRoot = this.asset.getRoot();
                this.unitCubeTransform = Filament.fitIntoUnitCube(aabb, zoffset);
                this.asset.loadResources();
                this.animator = this.asset.getInstance().getAnimator();
                this.animationStartTime = Date.now();
                this._updateOverlay();
            });
            return;
        }

        // Dropping a fileset requires pushing each resource to ResourceLoader.
        if (this.srcBlob && this.srcBlob.name.endsWith(".gltf")) {

            const config = {
                normalizeSkinningWeights: true,
                asyncInterval: 30
            };

            const doneAddingResources = (resourceLoader, stbProvider, ktx2Provider) => {
                this.srcBlobResources = {};
                resourceLoader.asyncBeginLoad(this.asset);
                const timer = setInterval(() => {
                    resourceLoader.asyncUpdateLoad();
                    const progress = resourceLoader.asyncGetLoadProgress();
                    if (progress >= 1) {
                        clearInterval(timer);
                        resourceLoader.delete();
                        stbProvider.delete();
                        ktx2Provider.delete();
                        this.animator = this.asset.getInstance().getAnimator();
                        this.animationStartTime = Date.now();
                    }
                }, config.asyncInterval);
            };

            this.srcBlob.arrayBuffer().then(buffer => {
                this.asset = this.loader.createAsset(new Uint8Array(buffer));
                const aabb = this.asset.getBoundingBox();
                this.assetRoot = this.asset.getRoot();
                this.unitCubeTransform = Filament.fitIntoUnitCube(aabb, zoffset);

                const resourceLoader = new Filament.gltfio$ResourceLoader(this.engine,
                    config.normalizeSkinningWeights);

                const stbProvider = new Filament.gltfio$StbProvider(this.engine);
                const ktx2Provider = new Filament.gltfio$Ktx2Provider(this.engine);

                resourceLoader.addStbProvider("image/jpeg", stbProvider);
                resourceLoader.addStbProvider("image/png", stbProvider);
                resourceLoader.addKtx2Provider("image/ktx2", ktx2Provider);

                let remaining = Object.keys(this.srcBlobResources).length;
                for (const name in this.srcBlobResources) {
                    this.srcBlobResources[name].arrayBuffer().then(buffer => {
                        const desc = getBufferDescriptor(new Uint8Array(buffer));
                        resourceLoader.addResourceData(name, getBufferDescriptor(desc));
                        if (--remaining === 0) {
                            doneAddingResources(resourceLoader, stbProvider, ktx2Provider);
                        }
                    });
                }

                this._updateOverlay();
            });

            return;
        }

        // If we reach this point, we are loading from a src URL rather than drag-and-drop.

        fetch(this.src).then(response => {
            return response.arrayBuffer();
        }).then(arrayBuffer => {
            const modelData = new Uint8Array(arrayBuffer);
            this.asset = this.loader.createAsset(modelData);
            const aabb = this.asset.getBoundingBox();
            this.assetRoot = this.asset.getRoot();
            this.unitCubeTransform = Filament.fitIntoUnitCube(aabb, zoffset);

            const basePath = '' + new URL(this.src, document.location);

            this.asset.loadResources(() => {
                this.animator = this.asset.getInstance().getAnimator();
                this.animationStartTime = Date.now();
                this._applyMaterialVariant();
            }, null, basePath);

            this._updateOverlay();
        });
    }

    _updateAsset() {
        // Invoke the first glTF animation if it exists.
        if (this.animator) {
            if (this.animator.getAnimationCount() > 0) {
                const ms = Date.now() - this.animationStartTime;
                this.animator.applyAnimation(0, ms / 1000);
            }
            this.animator.updateBoneMatrices();
        }

        // Apply the root transform of the model.
        const tcm = this.engine.getTransformManager();
        const inst = tcm.getInstance(this.assetRoot);
        let rootTransform = this.unitCubeTransform;
        if (this.trackball) {
            rootTransform = Filament.multiplyMatrices(rootTransform, this.trackball.getMatrix());
        }
        tcm.setTransform(inst, rootTransform);
        inst.delete();

        // Add renderable entities to the scene as they become ready.
        while (true) {
            const entity = this.asset.popRenderable();
            if (entity.getId() == 0) {
                entity.delete();
                break;
            }
            this.scene.addEntity(entity);
            entity.delete();
        }
    }

    _renderFrame() {
        // Apply transforms and add entities to the scene.
        if (this.asset) {
            this._updateAsset();
        }

        // Render the view and update the Filament engine.
        if (this.renderer.beginFrame(this.swapChain)) {
            this.renderer.renderView(this.view);
            this.renderer.endFrame();
        }
        this.engine.execute();

        window.requestAnimationFrame(this._renderFrame.bind(this));
    }

    _applyMaterialVariant() {
        if (!this.hasAttribute("materialVariant")) {
            return;
        }
        const instance = this.asset.getInstance();
        const names = instance.getMaterialVariantNames();
        const index = this.materialVariant;
        if (index < 0 || index >= names.length) {
            console.error(`Material variant ${index} does not exist in this asset.`);
            return;
        }
        console.info(this.src, `Applying material variant: ${names[index]}`);
        instance.applyMaterialVariant(index);
    }
}

customElements.define("filament-viewer", FilamentViewer);
