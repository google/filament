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

import { LitElement, html, css, repeat } from "https://cdn.jsdelivr.net/gh/lit/dist@3/all/lit-all.min.js";
import './tools.js';
import './tiff-viewer.js';

const urlParams = new URLSearchParams(window.location.search);
const RUN_ID = urlParams.get('run_id');
const URL_PREFIX = RUN_ID ? `/artifacts/${RUN_ID}` : '';

function getGoldenUrl(testResult) {
  return `${URL_PREFIX}/g/` + testResult.name;
}

function getCompUrl(testResult) {
  return `${URL_PREFIX}/c/` + testResult.name;
}

function getDiffUrl(testResult) {
  return `${URL_PREFIX}/d/` + testResult.diff;
}

const DIFF_VIEW = 'diff'
const GOLDEN_VIEW = 'golden'
const RENDERED_VIEW = 'rendered'

const RES_EQUAL = "mismatched dimensions";
const RES_MISMATCHED_DIMENSIONS = "mismatched dimensions";
const RES_DIFFERENT_PIXELS = "different pixels";
const RES_NOT_READY = "not ready";
const FAILED_POSTIFX = ".failed";

class ExpandedPassedResult extends LitElement {
  static get properties() {
    return {
      content: { type: Object, attribute: 'content' },
    };
  }

  constructor() {
    super();
    this.content = null;
  }

  render() {
    if (this.content) {
      const url = getGoldenUrl(this.content);
      return html`
        <div>
          <div style="margin-bottom: 10px;">
            <label>
              <input type="checkbox" id="magnifierToggle" @change="${this._onMagnifierToggle}">
              Enable Magnifier
            </label>
          </div>
          <tiff-viewer fileurl="${url}" id="passedViewer"></tiff-viewer>
        </div>
      `;
    }
    return html``;
  }

  _onMagnifierToggle(ev) {
    const viewer = this.shadowRoot.getElementById('passedViewer');
    if (viewer) {
      viewer.magnifierEnabled = ev.target.checked;
    }
  }
}
customElements.define('expanded-passed-result', ExpandedPassedResult);

class ExpandedFailedResult extends LitElement {
  static get properties() {
    return {
      content: { type: Object, attribute: 'content' },
      tests: { type: Array },
    };
  }

  render() {
    if (!this.content) {
      return html``;
    }
    return html`
      <expanded-comparison-result
        label-with-type
        .left=${this.content}
        .right=${this.content}
        leftViewType="golden"
        rightViewType="rendered"
        .tests=${this.tests}
      </expanded-comparison-result>
    `;
  }
}
customElements.define('expanded-failed-result', ExpandedFailedResult);

class ExpandedComparisonResult extends LitElement {
  static get properties() {
    return {
      labelWithType: {type: Boolean, attribute: "label-with-type" },
      left: { type: Object },
      right: { type: Object },
      tests: { type: Array },
      diffResult: { type: Object },
      leftViewType: { type: String },
      rightViewType: { type: String },
      magnifierEnabled: { type: Boolean },
      currentDiffImageData: {type: Object },
    };
  }

  static styles = css`
    .main-container {
      display: flex;
      flex-direction: column;
      align-items: center;
    }
    .viewer-container {
      display: flex;
      flex-direction: row;
      position: relative;
    }
    .selector {
      margin: 8px 0;
    }
  `;

  constructor() {
    super();
    this.labelWithType = false;
    this.left = null;
    this.right = null;
    this.tests = [];
    this.leftImageLoaded = false;
    this.rightImageLoaded = false;
    this.leftViewType = 'golden';
    this.rightViewType = 'golden';
    this.magnifierEnabled = true;
    this.originalDiffImageData = null;
    this.currentDiffImageData = null;

    this.addEventListener(
      'image-loaded',
      (ev) => {
        if (ev.detail.url == this._getUrl(this.leftViewType, this.left)) {
          this.leftImageLoaded = true;
        }
        if (ev.detail.url == this._getUrl(this.rightViewType, this.right)) {
          this.rightImageLoaded = true;
        }
        if (this.leftImageLoaded && this.rightImageLoaded) {
          this._triggerDiff();
        }
      }
    );
  }

  _getUrl(viewType, test) {
    return viewType == 'rendered' ? getCompUrl(test) : getGoldenUrl(test);
  }

  _viewer(name, choices, current, viewType) {
    const url = this._getUrl(viewType, current);
    const label = this.labelWithType ? viewType : current.name;
    return html`
      <div style="flex: 1; margin: 0 5px;">
        <div>${label}</div>
        <tiff-viewer id="viewer-${name}" class="viewer"
                     fileurl="${url}"
                     ?magnifier-enabled="${this.magnifierEnabled}"
                     disable-mouse-handlers></tiff-viewer>
      </div>
    `;
  }

  _computeDiff() {
    const tiffViewerLeft = this.shadowRoot.querySelector('#viewer-left');
    const tiffViewerRight = this.shadowRoot.querySelector('#viewer-right');

    if (!tiffViewerLeft || !tiffViewerRight) {
      return {
        "result": RES_NOT_READY,
      };
    }

    const canvasLeft = tiffViewerLeft.shadowRoot.querySelector('canvas');
    const canvasRight = tiffViewerRight.shadowRoot.querySelector('canvas');

    if (!canvasLeft || !canvasRight) {
      return {
        "result": RES_NOT_READY,
      };
    }

    const imgLeft = tiffViewerLeft.imgdata;
    const imgRight = tiffViewerRight.imgdata;

    if (imgLeft.width !== imgRight.width || imgLeft.height !== imgRight.height) {
      console.error("Images have different dimensions");
      return {
        "result": RES_MISMATCHED_DIMENSIONS,
        "explanation": "Images have different dimensions "  +
          "left=(" + imgLeft.width + ", " + imgLeft.height + ") " +
          "right=(" + imgRight.width + ", " + imgRight.height + ")",
      };
    }

    const imgDiff = new Uint8ClampedArray(imgLeft.width * imgLeft.height * 4);
    const maxDiff = [0, 0, 0, 0];
    for (let i = 0; i < imgLeft.data.length; i += 4) {
      for (let j = 0; j < 4; j++) {
        maxDiff[j] = Math.max(
          maxDiff[j],
          Math.min(255, Math.abs(imgLeft.data[i + j] - imgRight.data[i + j]))
        );
        imgDiff[i + j] = Math.abs(imgLeft.data[i + j] - imgRight.data[i + j]);
      }
      imgDiff[i + 3] = 255;
    }

    if (maxDiff[0] == 0 &&
        maxDiff[1] == 0 &&
        maxDiff[2] == 0 &&
        maxDiff[3] == 0) {
      return {
        "result": RES_EQUAL,
        "explanation": "Equal",
        "dim": {"width": imgLeft.width, "height": imgLeft.height },
      }
    }

    return {
      "result": RES_DIFFERENT_PIXELS,
      "explanation": "Images are different",
      "dim": {"width": imgLeft.width, "height": imgLeft.height },
      "maxDiff": maxDiff,
      "diffImg": imgDiff,
    };
  }

  _triggerDiff() {
    const diff = this._computeDiff();
    if (diff.result == RES_DIFFERENT_PIXELS) {
      this.diffResult = diff;
      // Reset original diff data when new diff is computed
      this.originalDiffImageData = null;
      const multDiv = this.shadowRoot.querySelector('#diffMultiplier');
      if (multDiv) {
        this._updateDiffCanvas(this.diffResult, multDiv.value);
      }
    } else {
      this.diffResult = null;
      this.currentDiffImageData = null;
    }
  }

  _updateDiffCanvas(diffResult, mult) {
    // Create a fresh copy of the original diff data to avoid mutation.
    const diffImgCopy = diffResult.diffImg.slice();

    // Modify the copy, not the original.
    for (let i = 0; i < diffImgCopy.length; i += 4) {
      for (let j = 0; j < 3; j++) {
        diffImgCopy[i + j] = Math.min(255, mult * diffImgCopy[i + j]);
      }
      diffImgCopy[i + 3] = 255; // Ensure alpha is always 255
    }

    // Create the ImageData from the modified copy.
    const imgData = new ImageData(diffImgCopy, diffResult.dim.width, diffResult.dim.height);

    // Store both original and current diff image data for magnifier
    if (!this.originalDiffImageData) {
      this.originalDiffImageData = new ImageData(diffResult.diffImg.slice(), diffResult.dim.width, diffResult.dim.height);
    }
    this.currentDiffImageData = imgData;
  }

  _onGlobalMouseLeave(event) {
    // Hide all magnifiers when mouse leaves the viewer container
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

      // Get image data from the viewer
      const imageData = viewer.imgdata;
      const magnifier = viewer.shadowRoot?.getElementById('magnifier');

      // Check if mouse is over any of the three views (left, , or diff)
      if (!this._isMouseOverAnyView(event)) {
        magnifier.hide();
        continue;
      }

      const rect = canvas.getBoundingClientRect();

      // Calculate the equivalent position on the image
      const { imageX, imageY, mouseX, mouseY } = this._calculateEquivalentPosition(event, rect, imageData);

      // Position magnifier relative to the canvas within the viewer
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
    // Check if mouse is over left viewer
    const leftViewer = this.shadowRoot.querySelector('#viewer-left');
    if (leftViewer) {
      const leftCanvas = leftViewer.shadowRoot?.querySelector('canvas');
      if (leftCanvas && this._isMouseOverElement(event, leftCanvas.getBoundingClientRect())) {
        return true;
      }
    }

    // Check if mouse is over right viewer
    const rightViewer = this.shadowRoot.querySelector('#viewer-right');
    if (rightViewer) {
      const rightCanvas = rightViewer.shadowRoot?.querySelector('canvas');
      if (rightCanvas && this._isMouseOverElement(event, rightCanvas.getBoundingClientRect())) {
        return true;
      }
    }

    // Check if mouse is over diff viewer
    const diffViewer = this.shadowRoot.querySelector('#viewer-diff');
    if (diffViewer) {
      const diffCanvas = diffViewer.shadowRoot?.querySelector('canvas');
      if (diffCanvas && this._isMouseOverElement(event, diffCanvas.getBoundingClientRect())) {
        return true;
      }
    }
    return false;
  }

  _calculateEquivalentPosition(event, targetRect, targetImageData) {
    // Find which view the mouse is actually over and calculate the equivalent position
    const leftViewer = this.shadowRoot.querySelector('#viewer-left');
    const rightViewer = this.shadowRoot.querySelector('#viewer-right');
    const diffViewer = this.shadowRoot.querySelector('#viewer-diff');

    let sourceRect = null;
    let sourceImageData = null;

    // Determine source view
    if (leftViewer) {
      const leftCanvas = leftViewer.shadowRoot?.querySelector('canvas');
      if (leftCanvas && this._isMouseOverElement(event, leftCanvas.getBoundingClientRect())) {
        sourceRect = leftCanvas.getBoundingClientRect();
        sourceImageData = leftViewer.imgdata;
      }
    }

    if (!sourceRect && rightViewer) {
      const rightCanvas = rightViewer.shadowRoot?.querySelector('canvas');
      if (rightCanvas && this._isMouseOverElement(event, rightCanvas.getBoundingClientRect())) {
        sourceRect = rightCanvas.getBoundingClientRect();
        sourceImageData = rightViewer.imgdata;
      }
    }

    if (!sourceRect && this.diffResult && diffViewer) {
      const diffCanvas = diffViewer.shadowRoot?.querySelector('canvas');
      if (diffCanvas && this._isMouseOverElement(event, diffCanvas.getBoundingClientRect())) {
        sourceRect = diffCanvas.getBoundingClientRect();
        sourceImageData = diffViewer.imgdata;
      }
    }

    if (!sourceRect || !sourceImageData) {
      // Fallback to target rect
      sourceRect = targetRect;
      sourceImageData = targetImageData;
    }

    // Calculate relative position in source
    const sourceMouseX = event.clientX - sourceRect.left;
    const sourceMouseY = event.clientY - sourceRect.top;
    const sourceScaleX = sourceImageData.width / sourceRect.width;
    const sourceScaleY = sourceImageData.height / sourceRect.height;
    const sourceImageX = Math.floor(sourceMouseX * sourceScaleX);
    const sourceImageY = Math.floor(sourceMouseY * sourceScaleY);

    // Convert to target coordinates
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

  render() {
    if (!this.left || !this.right) {
      return html``;
    }

    const showDiff = !!this.diffResult;
    const diffStyle = !showDiff ? "display:none;" : "display:flex; flex-direction: column; margin: 0 5px;";
    const v1 = this._viewer("left", this.tests, this.left, this.leftViewType);
    const v2 = this._viewer("right", this.tests, this.right, this.rightViewType);
    const onMultiplierChange = (ev) => {
      const multiplierValue = this.shadowRoot.querySelector('#multiplierValue');
      multiplierValue.textContent = ev.target.value;
      this._updateDiffCanvas(this.diffResult, ev.target.value);
    };
    const onMagnifierToggle = (ev) => {
      this.magnifierEnabled = ev.target.checked;
    };
    return html`
      <div class="main-container">
        <div class="viewer-container" @mousemove="${this._onGlobalMouseMove}" @mouseleave="${this._onGlobalMouseLeave}">
          ${v1}
          <div style="flex: 1; ${diffStyle}" class="viewer-container">
            <div>&nbsp;</div>
            <tiff-viewer id="viewer-diff" class="viewer"
                         name="diff"
                         .srcdata="${this.currentDiffImageData}"
                         ?magnifier-enabled="${this.magnifierEnabled}"
                         disable-mouse-handlers></tiff-viewer>
          </div>
          ${v2}
        </div>
        <div class="control" style="${diffStyle}">
          <div>
            <div>Difference Multiplier: <span id="multiplierValue">1</span></div>
            <input type="range" min="1" max="100" value="1" id="diffMultiplier" @input=${onMultiplierChange}>
          </div>
          <div style="margin-top: 10px;">
            <label>
              <input type="checkbox" .checked="${this.magnifierEnabled}" @change=${onMagnifierToggle}>
              Enable Magnifier
            </label>
          </div>
        </div>
      </div>
    `;
  }
}
customElements.define('expanded-comparison-result', ExpandedComparisonResult);

class App extends LitElement {
  static styles = css`
    :host {
      display: flex;
      flex-direction: column;
      align-items: center;
    }
    .test-label {
      margin-bottom: 10px;
      font-size: 12px;
    }
    .test-item {
      display: inline-flex;
      flex-direction: column;
      border: 1px solid black;
      border-radius: 10px;
      padding: 10px;
      align-items: center;
      margin: 5px;
    }
    .test-item:hover {
      box-shadow: 2px 3px 2px 0px rgba(40, 40, 40, .5);
      cursor: pointer;
    }
    .app {
      display: flex;
      flex-direction: column;
      max-width: 800px;
      align-items: center;
      padding: 20px 0;
    }
    .results {
      display: flex;
      flex-direction: row;
      flex-wrap: wrap;
    }
    .results-container {
      display: inline-flex;
      flex-direction: column;
      border: 1px solid black;
      border-radius: 10px;
      background: #cbf1c9;
      padding: 5px;
      align-items: center;
      margin: 10px 10px;
    }
    .container-label {
      margin-bottom: 10px;
    }
    .failed-tiffs {
      display: inline-flex;
      flex-direction: row;
    }
    .tiff-wrapper {
      margin: 0 4px;
      display: inline-flex;
      flex-direction: column;
      align-items: center;
    }
    .tiff-wrapper-label {
      margin: 4px 0;
      font-size: 10px;
    }
    .dialog-header {
      font-size: 18px;
      margin-bottom: 10px;
    }
    .compare-button {
      margin: 10px;
      padding: 10px;
      border-radius: 5px;
      background: #4285F4;
      color: white;
      cursor: pointer;
    }
    .compare-button[disabled] {
      background: #ccc;
      cursor: not-allowed;
      margin: 10px 0;
    }
  `;

  static properties = {
    tests: {type: Array},
    dialogContent: {type: Object},
    selectedTests: {type: Array},
    comparisonContent: {type: Object},
    compareMode: {type: Boolean},

    // This is used to cache urls that are not found
    missingFile: {type: Object},
  };

  async _init() {
    const config = await ((await fetch(`${URL_PREFIX}/r/`)).json());
    config['results'] = config['results'].sort((a, b) => a.name < b.name ? -1 : (a.name > b.name ? 1 : 0));
    this.tests = config['results']
  }

  constructor() {
    super();
    this.tests = [];
    this.dialogContent = null;
    this.selectedTests = [];
    this.comparisonContent = null;
    this.compareMode = false;
    this.missingFile = {
      [getDiffUrl('undefined')]: true,
    };
    this._init();

    this.addEventListener('dialog-closed', () => {
      this.dialogContent = null;
      this.comparisonContent = null;
    });

    this.addEventListener('url-hit', (ev) => {
      delete this.missingFile[ev.detail.value];
      this.missingFile = this.missingFile;
      this.requestUpdate();
    });

    this.addEventListener('url-miss', (ev) => {
      this.missingFile[ev.detail.value] = true;
      this.missingFile = this.missingFile;
      this.requestUpdate();
    });
  }

  updated(props) {
    if (props.has('dialogContent') || props.has('comparisonContent')) {
      let dialog = this.shadowRoot.querySelector("#dialog");
      dialog.open = !!this.dialogContent || !!this.comparisonContent;
    }
  }

  _onClick(testDetail, ev) {
    if (this.compareMode) {
      const index = this.selectedTests.indexOf(testDetail);
      if (index > -1) {
        this.selectedTests.splice(index, 1);
      } else {
        if (this.selectedTests.length >= 2) {
          this.selectedTests.shift();
        }
        this.selectedTests.push(testDetail);
      }
      this.requestUpdate();
      return;
    }
    this.dialogContent = testDetail;
    this.shadowRoot.querySelector("#dialog").open = true;
  }

  _onCompare() {
    if (this.selectedTests.length == 2) {
      this.comparisonContent = {
        left: this.selectedTests[0],
        right: this.selectedTests[1],
      };
    }
  }

  _onCompareModeChange(e) {
    this.compareMode = e.target.checked;
    if (!this.compareMode) {
      this.selectedTests = [];
    }
  }

  render() {
    const sortFn = (a, b) => {
      const aparts = a.name.split('.');
      const bparts = b.name.split('.');
      // 0 = test names
      // 1 = backend
      // 2 = model
      for (let i of [0, 2, 1]) {
        if (aparts[i] < bparts[i]) {
          return -1;
        }
        if (aparts[i] > bparts[i]) {
          return 1;
        }
      }
      return 0;
    };
    let passed = this.tests.filter((t) => t.result == 'ok').sort(sortFn);
    let failed = this.tests.filter((t) => t.result != 'ok').sort(sortFn);
    const singleTiff = (url) => {
      return html`<tiff-viewer style="max-width:100px" fileurl="${url}"></tiff-viewer>`;
    };
    const buildTiffs = (ts) => repeat(ts, (t) => t.name, (t) => {
      const printName = t.name.replace('.tif', '');
      const tiff = singleTiff(getGoldenUrl(t));
      const selected = this.selectedTests.includes(t);
      return html`
        <div class="test-item" @click="${(e)=>this._onClick(t, e)}" style=${selected ? "background: #eee" : ""}>
          <span class="test-label">${printName}</span>
          ${tiff}
        </div>
      `;
    });
    const buildFailedTiffs = (ts) => repeat(ts, (t) => t.name, (t) => {
      const printName = t.name.replace('.tif', '');
      const goldenUrl = getGoldenUrl(t);
      const compUrl = getCompUrl(t);
      const diffUrl = getDiffUrl(t);
      const wrap = (tif, label) => {
        return html`
          <div class="tiff-wrapper">
            ${tif}
            <div class="tiff-wrapper-label">${label}</div>
          </div>`;
      }
      const tiffs = [
        [goldenUrl, 'golden'],
        [compUrl, 'rendered'],
        [diffUrl, 'diff']
      ].filter((a) => !this.missingFile[a[0]])
       .map((a) => [singleTiff(a[0]), a[1]])
       .map((a) => wrap(...a));
      return html`
        <div class="test-item" @click="${(e)=>this._onClick(t, e)}" >
          <span class="test-label" title=${t.name}>${printName}</span>
          <div class="failed-tiffs">
            ${tiffs}
          </div>
        </div>
      `;
    });
    const passedTiffs = buildTiffs(passed);
    const failedTiffs = buildFailedTiffs(failed);

    let dialogSlot = null;
    let dialogHeader = '';
    if (this.dialogContent) {
      dialogSlot = (() => {
        if (this.dialogContent.result == 'ok') {
          return html`<expanded-passed-result .content=${this.dialogContent}></expanded-passed-result>`;
        }
        return html`<expanded-failed-result .content=${this.dialogContent} .tests=${this.tests}></expanded-failed-result>`;
      })();
      dialogHeader = this.dialogContent.name;
    } else if (this.comparisonContent) {
      dialogSlot = html`
          <expanded-comparison-result
              .left=${this.comparisonContent.left}
              .right=${this.comparisonContent.right}
              .tests=${this.tests}>
          </expanded-comparison-result>
      `;
      dialogHeader = 'Comparison';
    }

    const compareButton = () => {
      if (!this.compareMode) {
        return html``;
      }
      return html`<button class="compare-button" @click=${this._onCompare} ?disabled=${this.selectedTests.length != 2}>Compare</button>`;
    }

    return html`
        <div class="app">
           <div class="results-container" style="background:#f5a5a4; ${this.compareMode ? 'display:none' : ''}">
             <span class="container-label">Failed</span>
             <div class="results">
               ${failedTiffs}
             </div>
           </div>
           <div class="results-container">
             <span class="container-label">Passed</span>
             <div class="results">
               ${passedTiffs}
             </div>
           </div>
           <label>
             <input type="checkbox" .checked=${this.compareMode} @change=${this._onCompareModeChange}>
             Compare Mode
           </label>
           ${compareButton()}
        </div>
        <modal-dialog id="dialog">
          <div class="dialog-header" slot="header">${dialogHeader}</div>
          ${dialogSlot}
        </modal-dialog>
    `;
  }
}
customElements.define('diff-app', App);
