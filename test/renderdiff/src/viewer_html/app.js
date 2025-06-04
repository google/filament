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
import './tools.js';
import './tiff-viewer.js';

function getGoldenUrl(testResult) {
  return '/g/' + testResult.name;
}

function getCompUrl(testResult) {
  return '/c/' + testResult.name;
}

function getDiffUrl(testResult) {
  return '/d/' + testResult.diff;
}

const DIFF_VIEW = 'diff'
const GOLDEN_VIEW = 'golden'
const RENDERED_VIEW = 'rendered'

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
      return html`<tiff-viewer fileurl="${url}"></tiff-viewer>`;
    }
    return html``;
  }
}
customElements.define('expanded-passed-result', ExpandedPassedResult);

class ExpandedFailedResult extends LitElement {
  static get properties() {
    return {
      content: { type: Object, attribute: 'content' },
      leftViewSelected: { type: String },
      rightViewSelected: { type: String },
    };
  }

  static styles = css`
    .viewer-container {
      display: flex;
      flex-direction: row;
    }
    .viewer {
      margin: 0 5px;
    }
  `;

  constructor() {
    super();
    this.content = null;
    this.leftViewSelected = GOLDEN_VIEW;
    this.rightViewSelected = DIFF_VIEW;
    this.addEventListener('radio-change', (ev) => {
      if (ev.detail.radioId == 'right') {
        this.rightViewSelected = ev.detail.value;
      }
      if (ev.detail.radioId == 'left') {
        this.leftViewSelected = ev.detail.value;
      }
    });
  }

  _viewer(name, choices, current) {
    const url = {
      [GOLDEN_VIEW]: getGoldenUrl,
      [RENDERED_VIEW]: getCompUrl,
      [DIFF_VIEW]: getDiffUrl,
    }[current](this.content);
    return html`
      <div>
        <radio-button-group id="${name}" .choices=${choices} value=${current}></radio-button-group>
        <tiff-viewer class="viewer" fileurl="${url}"></tiff-viewer>
      </div>
    `;
  }

  render() {
    if (!this.content) {
      return html``;
    }
    const v1 = this._viewer("left", [GOLDEN_VIEW, RENDERED_VIEW], this.leftViewSelected);
    const v2 = this._viewer("right", [RENDERED_VIEW, DIFF_VIEW], this.rightViewSelected);
    return html`
      <div class="viewer-container">
        ${v1}
        ${v2}
      </div>
    `;
    return html``;
  }
}
customElements.define('expanded-failed-result', ExpandedFailedResult);

class App extends LitElement {
  static styles = css`
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
      width: 100%;
      align-items: center;
      padding-top: 20px;
    }
    .results {
      display: inline-flex;
      flex-direction: row;
    }
    .results-container {
      display: inline-flex;
      flex-direction: column;
      border: 1px solid black;
      border-radius: 10px;
      background: #cbf1c9;
      padding: 5px;
      align-items: center;
      margin: 10px 0;
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
  `;

  static properties = {
    tests: {type: Array},
    dialogContent: {type: Object},
  };

  async _init() {
    const config = await ((await fetch("/r/")).json());
    config['results'] = config['results'].sort((a, b) => a.name < b.name ? -1 : (a.name > b.name ? 1 : 0));
    this.tests = config['results']
  }

  constructor() {
    super();
    this.tests = [];
    this.dialogContent = null;
    this._init();

    this.addEventListener('dialog-closed', () => {
      this.dialogContent = null;
    });
  }

  updated(props) {
    if (props.has('dialogContent')) {
      let dialog = this.shadowRoot.querySelector("#dialog");
      dialog.open = !!this.dialogContent;
    }
  }

  _onClick(testDetail) {
    this.dialogContent = testDetail;
    this.shadowRoot.querySelector("#dialog").open = true;
  }

  render() {
    let passed = this.tests.filter((t) => t.result == 'ok');
    let failed = this.tests.filter((t) => t.result != 'ok');
    const singleTiff = (url) => {
      return html`<tiff-viewer style="max-width:100px" fileurl="${url}"></tiff-viewer>`;
    };
    const buildTiffs = (ts) => ts.map((t) => {
      const printName = t.name.replace('.tif', '');
      const tiff = singleTiff(getGoldenUrl(t));
      return html`
        <div class="test-item" @click="${()=>this._onClick(t)}">
          <span class="test-label">${printName}</span>
          ${tiff}
        </div>
      `;
    });
    const buildFailedTiffs = (ts) => ts.map((t) => {
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
      ].map((a) => [singleTiff(a[0]), a[1]])
       .map((a) => wrap(...a));
      return html`
        <div class="test-item" @click="${()=>this._onClick(t)}">
          <span class="test-label">${printName}</span>
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
        return html`<expanded-failed-result .content=${this.dialogContent}></expanded-failed-result>`;
      })();
      dialogHeader = this.dialogContent.name;
    }

    return html`
        <div class="app">
           <div class="results-container" style="background:#f5a5a4">
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
        </div>
        <modal-dialog id="dialog">
          <div class="dialog-header" slot="header">${dialogHeader}</div>
          ${dialogSlot}
        </modal-dialog>
    `;
  }
}
customElements.define('diff-app', App);
