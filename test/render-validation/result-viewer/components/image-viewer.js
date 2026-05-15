// Copyright (C) 2026 The Android Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

import { html, css, LitElement } from 'lit';
import './tiff-viewer.js';

const RES_EQUAL = "equal";
const RES_MISMATCHED_DIMENSIONS = "mismatched dimensions";
const RES_DIFFERENT_PIXELS = "different pixels";
const RES_NOT_READY = "not ready";

export class ImageViewer extends LitElement {
  static properties = {
    runData: { type: Object },
    deviceName: { type: String },
    isOpen: { type: Boolean },

    diffResult: { type: Object },
    magnifierEnabled: { type: Boolean },
    highlightFailing: { type: Boolean },
    currentDiffImageData: { type: Object },

    viewMode: { type: String },
    toggleState: { type: String },
    autoAlternate: { type: Boolean },

    zoom: { type: Number },
    offsetX: { type: Number },
    offsetY: { type: Number }
  };

  static styles = css`
    :host {
      display: block;
    }
    .modal-overlay {
      position: fixed;
      top: 0;
      left: 0;
      width: 100vw;
      height: 100vh;
      background: rgba(255, 255, 255, 0.95);
      z-index: 1000;
      display: flex;
      flex-direction: column;
      color: black;
      overflow-y: auto;
    }
    .header {
      padding: 15px 20px;
      display: flex;
      justify-content: space-between;
      align-items: center;
      background: #f8f9fa;
      border-bottom: 1px solid #ddd;
      box-shadow: 0 2px 4px rgba(0,0,0,0.1);
      z-index: 10;
      position: sticky;
      top: 0;
    }
    .title {
      font-size: 1.2em;
      font-weight: bold;
    }
    .controls {
      display: flex;
      gap: 15px;
      align-items: center;
    }
    .btn {
      background: #fff;
      color: #333;
      border: 1px solid #ccc;
      padding: 8px 16px;
      border-radius: 4px;
      cursor: pointer;
      font-size: 0.9em;
      transition: background 0.2s;
    }
    .btn:hover { background: #eee; }
    .close-btn {
      background: #e74c3c;
      border-color: #c0392b;
      color: white;
    }
    .close-btn:hover { background: #c0392b; color: white; }

    .main-container {
      display: flex;
      flex-direction: column;
      align-items: center;
      padding: 20px;
    }
    .viewer-container {
      display: flex;
      flex-direction: row;
      position: relative;
      width: 100%;
      justify-content: center;
      gap: 10px;
    }
    .control-panel {
      margin-top: 20px;
      padding: 15px;
      background: #f8f9fa;
      border: 1px solid #ddd;
      border-radius: 8px;
      display: flex;
      flex-direction: column;
      align-items: center;
      gap: 10px;
    }
    .viewer-wrap {
      flex: 1;
      max-width: 33%;
      display: flex;
      flex-direction: column;
      align-items: center;
    }
    .viewer-wrap.full-width {
      max-width: 512px;
    }
    .viewer-label {
      font-weight: bold;
      margin-bottom: 10px;
      font-size: 1.1em;
    }
    tiff-viewer {
      width: 100%;
      border: 1px solid #ccc;
      border-radius: 4px;
      background: #fff;
    }
  `;

  constructor() {
    super();
    this.isOpen = false;
    this.magnifierEnabled = true;
    this.highlightFailing = false;
    this.originalDiffImageData = null;
    this.currentDiffImageData = null;
    this.diffResult = null;

    this.leftImageLoaded = false;
    this.rightImageLoaded = false;

    this.viewMode = 'side-by-side';
    this.toggleState = 'rendered';
    this.autoAlternate = false;
    this._alternateInterval = null;

    this.zoom = 1;
    this.offsetX = 0;
    this.offsetY = 0;
    this._handleKeyDown = this._handleKeyDown.bind(this);

    this.addEventListener(
      'image-loaded',
      (ev) => {
        if (!this.runData) return;
        if (ev.detail.url === this.runData.golden) {
          this.leftImageLoaded = true;
        }
        if (ev.detail.url === this.runData.rendered) {
          this.rightImageLoaded = true;
        }
        if (this.leftImageLoaded && this.rightImageLoaded) {
          this._triggerDiff();
        }
      }
    );
  }

  connectedCallback() {
    super.connectedCallback();
    window.addEventListener('keydown', this._handleKeyDown);
  }

  disconnectedCallback() {
    super.disconnectedCallback();
    window.removeEventListener('keydown', this._handleKeyDown);
  }

  _handleKeyDown(e) {
    if (!this.isOpen) return;
    // Don't interfere with standard input elements if any are added later
    if (e.target.tagName === 'INPUT' || e.target.tagName === 'TEXTAREA') return;

    const step = 20 / this.zoom;
    let handled = true;

    switch (e.key.toLowerCase()) {
      case 'r':
        this.zoom = Math.min(this.zoom * 1.2, 50);
        break;
      case 'f':
        this.zoom = Math.max(this.zoom / 1.2, 0.1);
        break;
      case 'a':
        this.offsetX += step;
        break;
      case 'd':
        this.offsetX -= step;
        break;
      case 'w':
        this.offsetY -= step;
        break;
      case 's':
        this.offsetY += step;
        break;
        
      default:
        handled = false;
        break;
    }

    if (handled) {
      // Prevent default scrolling for WASD
      e.preventDefault();
      // Force update of lit element properties
      this.zoom = this.zoom; 
      this.offsetX = this.offsetX;
      this.offsetY = this.offsetY;
    }
  }

  resetView() {
    this.zoom = 1;
    this.offsetX = 0;
    this.offsetY = 0;
  }

  open(deviceName, runData) {
    this.deviceName = deviceName;
    this.runData = runData;
    this.isOpen = true;
    this.resetView();

    this.diffResult = null;
    this.currentDiffImageData = null;
    this.originalDiffImageData = null;
    this.leftImageLoaded = false;
    this.rightImageLoaded = false;
    this.highlightFailing = false;

    this.viewMode = 'side-by-side';
    this.toggleState = 'rendered';
    this.autoAlternate = false;
    if (this._alternateInterval) {
      clearInterval(this._alternateInterval);
      this._alternateInterval = null;
    }

    document.body.style.overflow = 'hidden';
  }

  close() {
    this.isOpen = false;
    document.body.style.overflow = '';
    if (this._alternateInterval) {
      clearInterval(this._alternateInterval);
      this._alternateInterval = null;
    }
  }

  _computeDiff() {
    const tiffViewerLeft = this.shadowRoot.querySelector('#viewer-left');
    const tiffViewerRight = this.shadowRoot.querySelector('#viewer-right');

    if (!tiffViewerLeft || !tiffViewerRight) {
      return { "result": RES_NOT_READY };
    }

    const canvasLeft = tiffViewerLeft.shadowRoot.querySelector('canvas');
    const canvasRight = tiffViewerRight.shadowRoot.querySelector('canvas');

    if (!canvasLeft || !canvasRight) {
      return { "result": RES_NOT_READY };
    }

    const imgLeft = tiffViewerLeft.imgdata;
    const imgRight = tiffViewerRight.imgdata;

    if (!imgLeft || !imgRight) {
      return { "result": RES_NOT_READY };
    }

    if (imgLeft.width !== imgRight.width || imgLeft.height !== imgRight.height) {
      console.error("Images have different dimensions");
      return {
        "result": RES_MISMATCHED_DIMENSIONS,
        "explanation": "Images have different dimensions "  +
          "left=(" + imgLeft.width + ", " + imgLeft.height + ") " +
          "right=(" + imgRight.width + ", " + imgRight.height + ")",
      };
    }

    const width = imgLeft.width;
    const height = imgLeft.height;
    const goldenData = imgLeft.data;
    const renderedData = imgRight.data;
    const imgDiff = new Uint8ClampedArray(width * height * 4);
    const maxDiff = [0, 0, 0, 0];

    const config = this.runData?.config;
    const highlight = this.highlightFailing;

    const blurCacheG = new Map();
    const blurCacheR = new Map();
    if (highlight && config) {
        blurCacheG.set(0, goldenData);
        blurCacheR.set(0, renderedData);

        const applyBlur = (data, radius) => {
            if (radius === 0) return data;
            const out = new Uint8ClampedArray(width * height * 4);
            for (let y = 0; y < height; y++) {
                for (let x = 0; x < width; x++) {
                    let r=0, g=0, b=0, a=0, count=0;
                    for (let dy = -radius; dy <= radius; dy++) {
                        for (let dx = -radius; dx <= radius; dx++) {
                            let nx = Math.max(0, Math.min(width - 1, x + dx));
                            let ny = Math.max(0, Math.min(height - 1, y + dy));
                            let idx = (ny * width + nx) * 4;
                            r += data[idx];
                            g += data[idx+1];
                            b += data[idx+2];
                            a += data[idx+3];
                            count++;
                        }
                    }
                    let oidx = (y * width + x) * 4;
                    out[oidx] = r/count;
                    out[oidx+1] = g/count;
                    out[oidx+2] = b/count;
                    out[oidx+3] = 255;
                }
            }
            return out;
        };

        const getUniqueBlurs = (cfg, out = new Set()) => {
            if (!cfg) return out;
            if (cfg.mode === "LEAF" || !cfg.mode) {
                out.add(cfg.blurRadius || 0);
            } else if (cfg.children) {
                for (let c of cfg.children) getUniqueBlurs(c, out);
            }
            return out;
        };

        const blurs = getUniqueBlurs(config);
        for (let rad of blurs) {
            if (rad > 0 && !blurCacheG.has(rad)) {
                blurCacheG.set(rad, applyBlur(goldenData, rad));
                blurCacheR.set(rad, applyBlur(renderedData, rad));
            }
        }
    }

    const checkPixel = (x, y, cfg) => {
        const mode = cfg.mode || "LEAF";
        if (mode === "LEAF") {
            const shiftRad = cfg.shiftRadius || 0;
            const blurRad = cfg.blurRadius || 0;
            const maxAllowedDiff = (cfg.maxAbsDiff || 0.0) * 255.0;
            const channelMask = cfg.channelMask !== undefined ? cfg.channelMask : 15;

            const gData = blurCacheG.get(blurRad);
            const rData = blurCacheR.get(blurRad);

            const activeCh = [];
            if (channelMask & 1) activeCh.push(0);
            if (channelMask & 2) activeCh.push(1);
            if (channelMask & 4) activeCh.push(2);
            if (channelMask & 8) activeCh.push(3);

            let rIdx = (y * width + x) * 4;
            let rc = [rData[rIdx], rData[rIdx+1], rData[rIdx+2], rData[rIdx+3]];

            for (let dy = -shiftRad; dy <= shiftRad; dy++) {
                for (let dx = -shiftRad; dx <= shiftRad; dx++) {
                    let nx = Math.max(0, Math.min(width - 1, x + dx));
                    let ny = Math.max(0, Math.min(height - 1, y + dy));
                    let idx = (ny * width + nx) * 4;

                    let match = true;
                    for (let c of activeCh) {
                        if (Math.abs(gData[idx+c] - rc[c]) > maxAllowedDiff) {
                            match = false;
                            break;
                        }
                    }
                    if (match) return true;
                }
            }
            return false;
        } else if (mode === "AND") {
            for (let child of (cfg.children || [])) {
                if (!checkPixel(x, y, child)) return false;
            }
            return true;
        } else if (mode === "OR") {
            for (let child of (cfg.children || [])) {
                if (checkPixel(x, y, child)) return true;
            }
            return false;
        }
        return false;
    };

    for (let i = 0; i < width * height; i++) {
      const idx = i * 4;
      const x = i % width;
      const y = Math.floor(i / width);

      let rDiff = Math.abs(goldenData[idx] - renderedData[idx]);
      let gDiff = Math.abs(goldenData[idx+1] - renderedData[idx+1]);
      let bDiff = Math.abs(goldenData[idx+2] - renderedData[idx+2]);
      let aDiff = Math.abs(goldenData[idx+3] - renderedData[idx+3]);

      maxDiff[0] = Math.max(maxDiff[0], rDiff);
      maxDiff[1] = Math.max(maxDiff[1], gDiff);
      maxDiff[2] = Math.max(maxDiff[2], bDiff);
      maxDiff[3] = Math.max(maxDiff[3], aDiff);

      if (highlight) {
        let pass = true;
        if (config) {
            pass = checkPixel(x, y, config);
        } else {
            pass = (rDiff === 0 && gDiff === 0 && bDiff === 0 && aDiff === 0);
        }

        if (!pass) {
            imgDiff[idx] = 255;
            imgDiff[idx+1] = 0;
            imgDiff[idx+2] = 0;
            imgDiff[idx+3] = 255;
        } else {
            imgDiff[idx] = rDiff;
            imgDiff[idx+1] = gDiff;
            imgDiff[idx+2] = bDiff;
            imgDiff[idx+3] = 255;
        }
      } else {
          imgDiff[idx] = rDiff;
          imgDiff[idx+1] = gDiff;
          imgDiff[idx+2] = bDiff;
          imgDiff[idx+3] = 255;
      }
    }

    if (maxDiff[0] == 0 && maxDiff[1] == 0 && maxDiff[2] == 0 && maxDiff[3] == 0) {
      return {
        "result": RES_EQUAL,
        "explanation": "Equal",
        "dim": {"width": width, "height": height },
      }
    }

    return {
      "result": RES_DIFFERENT_PIXELS,
      "explanation": "Images are different",
      "dim": {"width": width, "height": height },
      "maxDiff": maxDiff,
      "diffImg": imgDiff,
    };
  }

  _triggerDiff() {
    const diff = this._computeDiff();
    if (diff.result == RES_DIFFERENT_PIXELS) {
      this.diffResult = diff;
      this.originalDiffImageData = null;
      const multDiv = this.shadowRoot.querySelector('#diffMultiplier');
      if (multDiv) {
        this._updateDiffCanvas(this.diffResult, multDiv.value);
      } else {
        this._updateDiffCanvas(this.diffResult, 1);
      }
    } else {
      this.diffResult = null;
      this.currentDiffImageData = null;
    }
  }

  _updateDiffCanvas(diffResult, mult) {
    const diffImgCopy = diffResult.diffImg.slice();
    for (let i = 0; i < diffImgCopy.length; i += 4) {
      for (let j = 0; j < 3; j++) {
        diffImgCopy[i + j] = Math.min(255, mult * diffImgCopy[i + j]);
      }
      diffImgCopy[i + 3] = 255;
    }

    const imgData = new ImageData(diffImgCopy, diffResult.dim.width, diffResult.dim.height);

    if (!this.originalDiffImageData) {
      this.originalDiffImageData = new ImageData(diffResult.diffImg.slice(), diffResult.dim.width, diffResult.dim.height);
    }
    this.currentDiffImageData = imgData;
  }

  _onGlobalMouseLeave(event) {
    for (const name of ['left', 'right', 'diff']) {
      const viewer = this.shadowRoot.querySelector('#viewer-' + name);
      if (viewer) {
        const mag = viewer.shadowRoot?.getElementById('magnifier');
        if (mag) {
          mag.hide();
        }
      }
    }
  }

  _onGlobalMouseMove(event) {
    if (!this.magnifierEnabled) return;
    for (const name of ['left', 'right', 'diff']) {
      const viewer = this.shadowRoot.querySelector('#viewer-' + name);
      if (!viewer) continue;

      const canvas = viewer.shadowRoot?.querySelector('canvas');
      if (!canvas) continue;

      const imageData = viewer.imgdata;
      if (!imageData) continue;

      const magnifier = viewer.shadowRoot?.getElementById('magnifier');

      if (!this._isMouseOverAnyView(event)) {
        magnifier.hide();
        continue;
      }

      const rect = canvas.getBoundingClientRect();
      const { imageX, imageY, mouseX, mouseY } = this._calculateEquivalentPosition(event, rect, imageData);

      this._lastImageX = imageX;
      this._lastImageY = imageY;

      const origData = name == 'diff' ? this.originalDiffImageData : null;
      viewer.updateMagnifier(imageX, imageY, origData);
    }
  }

  _isMouseOverElement(event, rect) {
    return event.clientX >= rect.left &&
           event.clientX <= rect.right &&
           event.clientY >= rect.top &&
           event.clientY <= rect.bottom;
  }

  _isMouseOverAnyView(event) {
    const checkView = (id) => {
        const viewer = this.shadowRoot.querySelector(id);
        if (viewer) {
            const canvas = viewer.shadowRoot?.querySelector('canvas');
            if (canvas && this._isMouseOverElement(event, canvas.getBoundingClientRect())) return true;
        }
        return false;
    }
    return checkView('#viewer-left') || checkView('#viewer-right') || checkView('#viewer-diff');
  }

  _calculateEquivalentPosition(event, targetRect, targetImageData) {
    let sourceRect = null;
    let sourceImageData = null;

    const getSource = (id) => {
        const viewer = this.shadowRoot.querySelector(id);
        if (viewer) {
            const canvas = viewer.shadowRoot?.querySelector('canvas');
            if (canvas && this._isMouseOverElement(event, canvas.getBoundingClientRect())) {
                return { rect: canvas.getBoundingClientRect(), img: viewer.imgdata };
            }
        }
        return null;
    };

    let src = getSource('#viewer-left') || getSource('#viewer-right') || (this.diffResult ? getSource('#viewer-diff') : null);

    if (src) {
        sourceRect = src.rect;
        sourceImageData = src.img;
    }

    if (!sourceRect || !sourceImageData) {
      sourceRect = targetRect;
      sourceImageData = targetImageData;
    }

    const sourceMouseX = event.clientX - sourceRect.left;
    const sourceMouseY = event.clientY - sourceRect.top;
    const sourceScaleX = sourceImageData.width / sourceRect.width;
    const sourceScaleY = sourceImageData.height / sourceRect.height;
    const sourceImageX = Math.floor(sourceMouseX * sourceScaleX);
    const sourceImageY = Math.floor(sourceMouseY * sourceScaleY);

    const targetScaleX = targetImageData.width / targetRect.width;
    const targetScaleY = targetImageData.height / targetRect.height;
    const targetMouseX = sourceImageX / targetScaleX;
    const targetMouseY = sourceImageY / targetScaleY;

    return {
      imageX: sourceImageX,
      imageY: sourceImageY,
      mouseX: targetMouseX,
      mouseY: targetMouseY
    };
  }

  _updateMagnifiersForToggle() {
    if (!this.magnifierEnabled || this._lastImageX === undefined) return;
    const activeViewerId = this.toggleState === 'rendered' ? '#viewer-right' : '#viewer-left';
    const viewer = this.shadowRoot.querySelector(activeViewerId);
    if (!viewer || !viewer.imgdata) return;
    const origData = null; // No origData needed for golden/rendered
    viewer.updateMagnifier(this._lastImageX, this._lastImageY, origData);
  }

  render() {
    if (!this.isOpen || !this.runData) return html``;

    const showDiff = !!this.diffResult;

    const onMultiplierChange = (ev) => {
      const multiplierValue = this.shadowRoot.querySelector('#multiplierValue');
      multiplierValue.textContent = ev.target.value;
      this._updateDiffCanvas(this.diffResult, ev.target.value);
    };

    const onMagnifierToggle = (ev) => {
      this.magnifierEnabled = ev.target.checked;
    };

    const onHighlightToggle = (ev) => {
      this.highlightFailing = ev.target.checked;
      this._triggerDiff();
    };

    const toggleViewMode = async () => {
      this.viewMode = this.viewMode === 'side-by-side' ? 'toggle' : 'side-by-side';
      if (this.viewMode === 'toggle') {
        this.autoAlternate = true;
        if (!this._alternateInterval) {
          this._alternateInterval = setInterval(async () => {
            this.toggleState = this.toggleState === 'rendered' ? 'golden' : 'rendered';
            await this.updateComplete;
            this._updateMagnifiersForToggle();
          }, 2000);
        }
      } else {
        this.autoAlternate = false;
        if (this._alternateInterval) {
          clearInterval(this._alternateInterval);
          this._alternateInterval = null;
        }
      }
      await this.updateComplete;
      if (this.viewMode === 'toggle') {
          this._updateMagnifiersForToggle();
      }
    };

    const switchToggleState = async () => {
      if (this.autoAlternate) return;
      this.toggleState = this.toggleState === 'rendered' ? 'golden' : 'rendered';
      await this.updateComplete;
      this._updateMagnifiersForToggle();
    };

    const onAutoAlternateChange = (ev) => {
      this.autoAlternate = ev.target.checked;
      if (this.autoAlternate) {
        this._alternateInterval = setInterval(async () => {
          this.toggleState = this.toggleState === 'rendered' ? 'golden' : 'rendered';
          await this.updateComplete;
          this._updateMagnifiersForToggle();
        }, 2000);
      } else {
        if (this._alternateInterval) {
            clearInterval(this._alternateInterval);
            this._alternateInterval = null;
        }
      }
    };
      console.log(this.zoom);

    return html`
      <div class="modal-overlay">
        <div class="header">
          <div class="title">${this.deviceName} - ${this.runData.testName}</div>
          <div style="font-size: 0.9em; color: #666; margin: 0 15px;">
            <strong>View Controls:</strong> W (Pan Up), S (Pan Down), A (Pan Left), D (Pan Right), R (Zoom In), F (Zoom Out)
          </div>
          <div class="controls">
            <button class="btn" @click="${this.resetView}">Reset View</button>
            <button class="btn" @click="${toggleViewMode}">
              ${this.viewMode === 'side-by-side' ? 'Switch to Toggle Mode' : 'Switch to Side-by-Side'}
            </button>
            <button class="btn close-btn" @click="${this.close}">Close</button>
          </div>
        </div>

        <div class="main-container">
            <div class="viewer-container ${this.viewMode === 'toggle' ? 'toggle-mode' : ''}" @mousemove="${this._onGlobalMouseMove}" @mouseleave="${this._onGlobalMouseLeave}">
                <div class="viewer-wrap ${this.viewMode === 'toggle' ? 'full-width' : ''}"
                     style="${this.viewMode === 'toggle' && this.toggleState !== 'golden' ? 'display: none;' : ''} overflow: hidden;">
                    <div class="viewer-label">Golden</div>
                    <tiff-viewer id="viewer-left" class="viewer"
                                fileurl="${this.runData.golden}"
                                ?magnifier-enabled="${this.magnifierEnabled}"
                                disable-mouse-handlers
                                style="transform: translate(${this.offsetX}px, ${this.offsetY}px) scale(${this.zoom}); transform-origin: center;"></tiff-viewer>
                </div>

                <div class="viewer-wrap" style="${(!showDiff || this.viewMode === 'toggle') ? 'display: none;' : ''} overflow: hidden;">
                    <div class="viewer-label">Diff</div>
                    <tiff-viewer id="viewer-diff" class="viewer"
                                name="diff"
                                .srcdata="${this.currentDiffImageData}"
                                ?magnifier-enabled="${this.magnifierEnabled}"
                                disable-mouse-handlers
                                style="transform: translate(${this.offsetX}px, ${this.offsetY}px) scale(${this.zoom}); transform-origin: center;"></tiff-viewer>
                </div>

                <div class="viewer-wrap ${this.viewMode === 'toggle' ? 'full-width' : ''}"
                     style="${this.viewMode === 'toggle' && this.toggleState !== 'rendered' ? 'display: none;' : ''} overflow: hidden;">
                    <div class="viewer-label">Rendered</div>
                    <tiff-viewer id="viewer-right" class="viewer"
                                fileurl="${this.runData.rendered}"
                                ?magnifier-enabled="${this.magnifierEnabled}"
                                disable-mouse-handlers
                                style="transform: translate(${this.offsetX}px, ${this.offsetY}px) scale(${this.zoom}); transform-origin: center;"></tiff-viewer>
                </div>
            </div>

            <div class="control-panel" style="${(this.viewMode === 'toggle' || !showDiff) ? 'display: none;' : ''}">
                <div>
                    <strong>Difference Multiplier:</strong> <span id="multiplierValue">1</span>x
                </div>
                <input type="range" min="1" max="100" value="1" id="diffMultiplier" @input=${onMultiplierChange} style="width: 200px;">

                <div style="display: flex; gap: 20px; margin-top: 10px;">
                    <label style="cursor: pointer; display: flex; align-items: center; gap: 5px;">
                        <input type="checkbox" .checked="${this.magnifierEnabled}" @change=${onMagnifierToggle}>
                        Enable Magnifier
                    </label>
                    <label style="cursor: pointer; display: flex; align-items: center; gap: 5px;">
                        <input type="checkbox" .checked="${this.highlightFailing}" @change=${onHighlightToggle}>
                        Highlight Failing Pixels (Red)
                    </label>
                </div>
            </div>

            <div class="control-panel" style="${this.viewMode === 'side-by-side' ? 'display: none;' : ''}">
                <div style="display: flex; gap: 20px; align-items: center;">
                    <button class="btn" @click="${switchToggleState}" ?disabled="${this.autoAlternate}">
                        Switch to ${this.toggleState === 'rendered' ? 'Golden' : 'Rendered'}
                    </button>
                    <label style="cursor: pointer; display: flex; align-items: center; gap: 5px;">
                        <input type="checkbox" .checked="${this.autoAlternate}" @change=${onAutoAlternateChange}>
                        Auto-Alternate (2s)
                    </label>
                    <label style="cursor: pointer; display: flex; align-items: center; gap: 5px;">
                        <input type="checkbox" .checked="${this.magnifierEnabled}" @change=${onMagnifierToggle}>
                        Enable Magnifier
                    </label>
                </div>
            </div>
        </div>
      </div>
    `;
  }
}

customElements.define('image-viewer', ImageViewer);
