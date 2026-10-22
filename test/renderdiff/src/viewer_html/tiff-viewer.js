// Copyright (C) 2025 The Android Open Source Project
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

import { LitElement, html, css } from "https://cdn.jsdelivr.net/gh/lit/dist@3/all/lit-all.min.js";

class ImageMagnifier extends LitElement {
  static styles = css`
    :host {
      position: absolute;
      pointer-events: none;
      z-index: 1000;
      display: none;
    }

    :host([visible]) {
      display: block;
    }

    .magnifier {
      width: 150px;
      height: 150px;
      border: 2px solid #000;
      border-radius: 75px;
      background: white;
      box-shadow: 0 4px 8px rgba(0,0,0,0.3);
      position: relative;
    }

    .magnifier-canvas {
      width: 100%;
      height: 100%;
      border-radius: 73px;
    }

    .pixel-info {
      position: absolute;
      top: -40px;
      left: 15px;
      background: rgba(0,0,0,0.8);
      color: white;
      padding: 4px 8px;
      border-radius: 4px;
      font-family: monospace;
      font-size: 9px;
      white-space: pre-line;
      max-width: 300px;
    }
  `;

  static properties = {
    visible: {type: Boolean, reflect: true},
  };

  constructor() {
    super();
    this.visible = false;
  }

  render() {
    return html`
      <div class="magnifier">
        <canvas class="magnifier-canvas" id="magnifierCanvas"></canvas>
        <div class="pixel-info" id="pixelInfo"></div>
      </div>
    `;
  }

  updateMagnifier(imageData, parentRect, imageX, imageY, originalImageData = null) {
    const zoomFactor = 8;
    if (!imageData) return;

    if (imageX < 0 || imageX >= imageData.width || imageY < 0 || imageY >= imageData.height) {
      this.visible = false;
      return;
    }

    const pixelIndex = (imageY * imageData.width + imageX) * 4;
    const r = imageData.data[pixelIndex];
    const g = imageData.data[pixelIndex + 1];
    const b = imageData.data[pixelIndex + 2];
    const a = imageData.data[pixelIndex + 3];

    let pixelInfoText = `(${r}, ${g}, ${b}, ${a})\n@ (${imageX}, ${imageY})`;

    // If original image data is provided, show the unmultiplied values too
    if (originalImageData) {
      const origR = originalImageData.data[pixelIndex];
      const origG = originalImageData.data[pixelIndex + 1];
      const origB = originalImageData.data[pixelIndex + 2];
      const origA = originalImageData.data[pixelIndex + 3];
      pixelInfoText = `Orig: (${origR}, ${origG}, ${origB}, ${origA})\nMult: (${r}, ${g}, ${b}, ${a})\n@ (${imageX}, ${imageY})`;
    }


    const magnifierSize = 150;
    const sourceSize = magnifierSize / zoomFactor;
    const halfSource = sourceSize / 2;

    const sourceX = Math.max(0, Math.min(imageData.width - sourceSize, imageX - halfSource));
    const sourceY = Math.max(0, Math.min(imageData.height - sourceSize, imageY - halfSource));

    const magnifierCanvas = this.shadowRoot.getElementById('magnifierCanvas');
    const pixelInfo = this.shadowRoot.getElementById('pixelInfo');

    magnifierCanvas.width = magnifierSize;
    magnifierCanvas.height = magnifierSize;
    const magnifierCtx = magnifierCanvas.getContext('2d');

    magnifierCtx.imageSmoothingEnabled = false;

    const tempCanvas = document.createElement('canvas');
    tempCanvas.width = imageData.width;
    tempCanvas.height = imageData.height;
    const tempCtx = tempCanvas.getContext('2d');
    tempCtx.putImageData(imageData, 0, 0);

    magnifierCtx.drawImage(
      tempCanvas,
      sourceX, sourceY, sourceSize, sourceSize,
      0, 0, magnifierSize, magnifierSize
    );

    const centerX = magnifierSize / 2;
    const centerY = magnifierSize / 2;
    const lineWidth = 1;
    const boxWidth = zoomFactor + lineWidth;
    magnifierCtx.strokeStyle = 'red';
    magnifierCtx.lineWidth = lineWidth;
    magnifierCtx.beginPath();
    magnifierCtx.moveTo(centerX, centerY);
    magnifierCtx.lineTo(centerX + boxWidth, centerY);
    magnifierCtx.lineTo(centerX + boxWidth, centerY + boxWidth);
    magnifierCtx.lineTo(centerX, centerY + boxWidth);
    magnifierCtx.lineTo(centerX, centerY);
    magnifierCtx.stroke();

    // Position relative to the TiffViewer container
    this.style.left = Math.round(-centerX +
                                 (imageX / imageData.width) * parentRect.width -
                                 boxWidth) + 'px';
    this.style.top = Math.round(-centerY +
                                (imageY / imageData.height) * parentRect.height -
                                boxWidth) + 'px';
    pixelInfo.textContent = pixelInfoText;

    this.visible = true;
  }

  hide() {
    this.visible = false;
  }
}

customElements.define('image-magnifier', ImageMagnifier);

// Generated by Gemini with some modifications
export class TiffViewer extends LitElement {
  static styles = css`
    :host {
      display: block;
      position: relative;
    }
    canvas {
      border: 1px solid #ccc;
      width: 100%;
      height: 100%;
    }
  `;

  static properties = {
    fileurl: {type: String, attribute: 'fileurl'},
    failedToFetch: {type: Boolean },
    magnifierEnabled: {type: Boolean, attribute: 'magnifier-enabled'},
    disableMouseHandlers: {type: Boolean, attribute: 'disable-mouse-handlers'},
    srcdata: {type: Object, attribute: 'srcdata'},
  };

  constructor() {
    super();
    this.fileurl = null;
    this.failedToFetch = false;
    this.magnifierEnabled = false;
    this.disableMouseHandlers = false;
    this.imgdata = null;
    this.srcdata = null;
    this.canvasRect = null;
  }

  render() {
    if (this.failedToFetch) {
      return html``;
    }
    return html`
      <canvas id="tiffCanvas"
              @mousemove="${this._onMouseMove}"
              @mouseenter="${this._onMouseEnter}"
              @mouseleave="${this._onMouseLeave}">
      </canvas>
      <image-magnifier id="magnifier"></image-magnifier>
    `;
  }

  updated(props) {
    if (props.has('fileurl') && this.fileurl) {
      this._updateImage(this.fileurl);
      return;
    }
    if (props.has('srcdata') && this.srcdata) {
      this._drawImage(this.srcdata);
    }
  }

  _drawImage(imageData) {
    const canvas = this.shadowRoot.getElementById('tiffCanvas');
    const ctx = canvas.getContext('2d');
    canvas.width = imageData.width;
    canvas.height = imageData.height;
    ctx.putImageData(imageData, 0, 0);
    this.imgdata = imageData;
  }

  async _updateImage(fileurl) {
    this.failedToFetch = false;
    let fileblob = null;
    try {
      let res = await fetch(this.fileurl);
      if (!res.ok) {
        throw new Error(`Could not find ${this.fileurl}`);
      }
      fileblob = await res.arrayBuffer();
    } catch (error) {
      this.failedToFetch = true;
    }
    if (!fileblob) {
      const event = new CustomEvent('url-miss', {
        detail: { value: fileurl },
        bubbles: true,
        composed: true,
      });
      this.dispatchEvent(event);
      return;
    } else {
      const event = new CustomEvent('url-hit', {
        detail: { value: fileurl },
        bubbles: true,
        composed: true,
      });
      this.dispatchEvent(event);
    }
    try {
      const arrayBuffer = fileblob;
      const ifds = UTIF.decode(arrayBuffer); // Parse TIFF data

      if (!ifds || ifds.length === 0) {
        console.error('Could not decode TIFF file. No image data found.');
        this._clearCanvas();
        return;
      }

      // By default, render the first image in the TIFF (IFD)
      const firstImage = ifds[0];
      UTIF.decodeImage(arrayBuffer, firstImage, ifds); // Decode the actual pixel data

      const rgba = UTIF.toRGBA8(firstImage); // Convert to RGBA

      // This needs to be present so that the ImageData has the right dimension to begin with
      // (without initial resizing due to flex's css styling and then resized again in _drawImage()).
      const canvas = this.shadowRoot.getElementById('tiffCanvas');
      canvas.width = firstImage.width;
      canvas.height = firstImage.height;

      // The default mode would set alpha to 1 so that RGB differences would be displayed as non-transparent
      for (let i = 0; i < firstImage.width; i++) {
        for (let j = 0; j < firstImage.height; j++) {
          rgba[(j * firstImage.width * 4) + (i * 4) + 3] = 255;
        }
      }

      const imageData = new ImageData(new Uint8ClampedArray(rgba), firstImage.width, firstImage.height);
      this.imgdata = imageData;

      this.dispatchEvent(new CustomEvent('image-loaded', {
        bubbles: true,
        composed: true,
        detail: {
          url: this.fileurl,
          img: imageData,
        }
      }));
      this._drawImage(imageData);
    } catch (error) {
      console.error('Error processing TIFF file:', error);
      this._clearCanvas();
    }
  }

  _clearCanvas() {
    const canvas = this.shadowRoot.getElementById('tiffCanvas');
    if (canvas) {
      const ctx = canvas.getContext('2d');
      ctx.clearRect(0, 0, canvas.width, canvas.height);
    }
  }

  _onMouseEnter(event) {
    if (this.disableMouseHandlers || !this.magnifierEnabled || !this.imgdata) return;
    this.canvasRect = event.target.getBoundingClientRect();
  }

  _onMouseLeave(event) {
    if (this.disableMouseHandlers || !this.magnifierEnabled) return;
    const magnifier = this.shadowRoot.getElementById('magnifier');
    magnifier.hide();
  }

  _onMouseMove(event) {
    if (this.disableMouseHandlers || !this.canvasRect) return;
    const rect = this.canvasRect;

    const scaleX = this.imgdata.width / rect.width;
    const scaleY = this.imgdata.height / rect.height;

    const mouseX = event.clientX - rect.left;
    const mouseY = event.clientY - rect.top;

    const imageX = Math.floor(mouseX * scaleX);
    const imageY = Math.floor(mouseY * scaleY);

    this.updateMagnifier(imageX, imageY);
  }

  updateMagnifier(imageX, imageY, origData = null) {
    let rect = null;
    const canvas = this.shadowRoot.getElementById('tiffCanvas');
    if (canvas) {
      rect = canvas.getBoundingClientRect();
    }
    if (!this.magnifierEnabled || !this.imgdata || !rect) return;
    const magnifier = this.shadowRoot.getElementById('magnifier');
    magnifier.updateMagnifier(this.imgdata, rect, imageX, imageY, origData);
  }
}

customElements.define('tiff-viewer', TiffViewer);
