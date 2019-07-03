/*
 * Copyright (C) 2018 The Android Open Source Project
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

import * as Filament from "filament";
import * as glm from "gl-matrix";

Filament.init([], () => {
    window["vec3"] = glm.vec3;
    window["app"] = new App(document.getElementsByTagName('canvas')[0]);
});

class App {
    public readonly camera: Filament.Camera;
    private canvas: HTMLCanvasElement;
    private engine: Filament.Engine;
    private scene: Filament.Scene;
    private view: Filament.View;
    private swapChain: Filament.SwapChain;
    private renderer: Filament.Renderer;
    private bgcolor: number[];

    constructor(canvas) {
      this.canvas = canvas;
      this.engine = Filament.Engine.create(canvas);
      this.scene = this.engine.createScene();
      this.swapChain = this.engine.createSwapChain();
      this.renderer = this.engine.createRenderer();
      this.camera = this.engine.createCamera();
      this.view = this.engine.createView();
      this.view.setCamera(this.camera);
      this.view.setScene(this.scene);

      const eye = [0, 0, 4], center = [0, 0, 0], up = [0, 1, 0];
      this.camera.lookAt(eye, center, up);
      this.resize();

      this.bgcolor = [0.2, 0.4, 0.6, 1.0];

      this.resize = this.resize.bind(this);
      this.render = this.render.bind(this);
      window.requestAnimationFrame(this.render);
      window.addEventListener('resize', this.resize);
    }

    render() {
        this.view.setClearColor(this.bgcolor);
        this.bgcolor[1] = (this.bgcolor[1] + 0.01) % 1.0;
        this.renderer.render(this.swapChain, this.view);
        window.requestAnimationFrame(this.render);
    }

    resize() {
        const dpr = window.devicePixelRatio;
        const width = this.canvas.width = window.innerWidth * dpr;
        const height = this.canvas.height = window.innerHeight * dpr;
        this.view.setViewport([0, 0, width, height]);
        const aspect = width / height;
        const Fov = Filament.Camera$Fov, fov = aspect < 1 ? Fov.HORIZONTAL : Fov.VERTICAL;
        this.camera.setProjectionFov(45, aspect, 1.0, 10.0, fov);
    }
  }
