/*
* Copyright (C) 2025 The Android Open Source Project
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

import { LitElement, html, css, unsafeCSS, nothing } from "https://unpkg.com/lit@2.8.0?module";

const kUntitledPlaceholder = "untitled";

// CSS constants
const FOREGROUND_COLOR = '#fafafa';
const INACTIVE_COLOR = '#9a9a9a';
const DARKER_INACTIVE_COLOR = '#6f6f6f';
const LIGHTER_INACTIVE_COLOR = '#d9d9d9';
const UNSELECTED_COLOR = '#dfdfdf';
const BACKGROUND_COLOR = '#5362e5';
const HOVER_BACKGROUND_COLOR = '#b3c2ff';
const CODE_VIEWER_BOTTOM_ROW_HEIGHT = 60;
const REGULAR_FONT_SIZE = 12;

// Constants for color coding operations
const READ_COLOR = '#cce5ff';
const WRITE_COLOR = '#d4edda';
const CREATE_COLOR = '#f8d7da';
const READ_WRITE_COLOR = '#ffeb99';
const DEFAULT_COLOR = '#ffffff';

class MenuSection extends LitElement {
    static get properties() {
        return {
            showing: {type: Boolean, state: true},
            title: {type: String, attribute: 'title'},
        };
    }

    static get styles() {
        return css`
            :host {
                font-size: ${unsafeCSS(REGULAR_FONT_SIZE)}px;
                color: ${unsafeCSS(UNSELECTED_COLOR)};
            }
            .section-title {
                font-size: 16px;
                color: ${unsafeCSS(UNSELECTED_COLOR)};
                cursor: pointer;
            }
            .container {
                margin-bottom: 20px;
            }
            hr {
                display: block;
                height: 1px;
                border: 0px;
                border-top: 1px solid ${unsafeCSS(UNSELECTED_COLOR)};
                padding: 0;
                width: 100%;
                margin: 3px 0 8px 0;
            }
            .expander {
                display: flex;
                flex-direction: row;
                align-items: center;
                justify-content: space-between;
            }
        `;
    }

    _showClick() {
        this.showing = !this.showing;
    }

    constructor() {
        super();
        this.showing = true;
    }

    render() {
        const expandedIcon = this.showing ? '－' : '＋';
        const slot = (() => html`<slot></slot>`)();
        return html`
            <div class="container">
                <div class="section-title expander" @click="${this._showClick}">
                    <span>${this.title}</span> <span>${expandedIcon}</span>
                </div>
                <hr />
                ${this.showing ? slot : []}
            </div>
        `;
    }
}
customElements.define('menu-section', MenuSection);

class FrameGraphSidePanel extends LitElement {
    // Setting the style in render() has poor performance implications.  We use it simply to avoid
    // having another container descending from the root to host the background color.
    dynamicStyle() {
        return `
            :host {
                background: ${this.connected ? BACKGROUND_COLOR : DARKER_INACTIVE_COLOR};
                width:100%;
                max-width: 250px;
                min-width: 180px;
                padding: 10px 20px;
                overflow-y: auto;
            }
            .title {
                color: white;
                width: 100%;
                text-align: center;
                margin: 0 0 10px 0;
                font-size: 20px;
            }
            .framegraphs {
                display: flex;
                flex-direction: column;
                margin-bottom: 20px;
                font-size: ${REGULAR_FONT_SIZE}px;
                color: ${UNSELECTED_COLOR};
            }
            .framegraph:hover {
                text-decoration: underline;
            }
            .framegraph {
                cursor: pointer;
            }
            .selected {
                font-weight: bolder;
                color: ${FOREGROUND_COLOR};
            }
        `;
    }

    static get properties() {
        return {
            connected: {type: Boolean, attribute: 'connected'},
            currentFrameGraph: {type: String, attribute: 'current-framegraph'},

            database: {type: Object, state: true},
            framegraphs: {type: Array, state: true},
        }
    }

    constructor() {
        super();
        this.connected = false;
        this.framegraphs = [];
        this.database = {};
    }

    updated(props) {
        if (props.has('database')) {
            const items = [];
            // Names need not be unique, so we display a numeric suffix for non-unique names.
            // To achieve stable ordering of anonymous framegraphs, we first sort by fgid.
            const labels = new Set();
            const fgids = Object.keys(this.database).sort();
            const duplicatedLabels = {};
            for (const fgid of fgids) {
                const name = this.database[fgid].viewName || kUntitledPlaceholder;
                if (labels.has(name)) {
                    duplicatedLabels[name] = 0;
                } else {
                    labels.add(name);
                }
            }

            this.framegraphs = fgids.map((fgid) => {
                const framegraph = this.database[fgid];
                let name = framegraph.viewName || kUntitledPlaceholder;
                if (name in duplicatedLabels) {
                    const index = duplicatedLabels[name];
                    duplicatedLabels[name] = index + 1;
                    name = `${name} (${index})`;
                }
                return {
                    fgid: fgid,
                    name: name,
                    domain: "views"
                };
            });
        }
    }

    _handleFrameGraphClick(fgid, ev) {
        this.dispatchEvent(new CustomEvent('select-framegraph', {detail: fgid, bubbles: true, composed: true}));
    }

    render() {
        const sections = (title) => {
            const fgs = this.framegraphs
                    .map((fg) => {
                        const framegraph = this.database[fg.fgid];
                        const onClick = this._handleFrameGraphClick.bind(this, fg.fgid);
                        const isFrameGraphSelected = fg.fgid === this.currentFrameGraph;
                        const fgName = (isFrameGraphSelected ? '● ' : '') + framegraph.viewName;
                        return html`
                            <div class="framegraph" @click="${onClick}" data-id="${fg.fgid}">
                                ${fgName}
                            </div>
                        `;
                    });
            if (fgs.length > 0) {
                return html`<menu-section title="${title}">${fgs}</menu-section>`;
            }
            return null;
        };

        return html`
            <style>${this.dynamicStyle()}</style>
            <div class="container">
                <div class="title">fgviewer</div>
                ${sections("Views", "views") ?? nothing}
            </div>
        `;
    }

}
customElements.define("framegraph-sidepanel", FrameGraphSidePanel);

class FrameGraphTable extends LitElement {
    static get styles() {
        return css`
            :host {
                display: flex;
                flex-grow: 1;
                padding: 10px;
                overflow: auto;
            }
            #editor {
                width: 100%;
                height: 100%;
            }
            .table-container {
                width: 100%;
                max-height: 100%;
                overflow: auto;
            }
            table {
                width: 100%;
                border-collapse: collapse;
            }
            th, td {
                border: 1px solid #ddd;
                text-align: center;
                padding: 8px;
            }
            th {
                background-color: #f2f2f2;
            }
        `;
    }


    static get properties() {
        return {
            frameGraphData: { type: Object, state: true }, // Expecting a JSON frame graph structure
            expectedWidth: {type: Number, attribute: 'expected-width'},
            expectedHeight: {type: Number, attribute: 'expected-height'},
        };
    }

    get _tableDiv() {
        return this.renderRoot.querySelector('#editor');
    }

    constructor() {
        super();
        this.frameGraphData = null;
        this.expectedWidth = 0;
        this.expectedHeight = 0;
    }

    updated(props) {
        if ((props.has('expectedWidth') || props.has('expectedHeight')) &&
                (this.expectedWidth > 0 && (this.expectedHeight - CODE_VIEWER_BOTTOM_ROW_HEIGHT) > 0)) {
            const actualWidth = Math.floor(this.expectedWidth);
            const actualHeight = (Math.floor(this.expectedHeight) - CODE_VIEWER_BOTTOM_ROW_HEIGHT);
            // this._tableDiv.style.width = actualWidth + 'px';
            // this._tableDiv.style.height = actualHeight + 'px';
        }
    }

    _getCellColor(type) {
        switch (type) {
            case 'read': return READ_COLOR;
            case 'write': return WRITE_COLOR;
            case 'create': return CREATE_COLOR;
            case 'read-write': return READ_WRITE_COLOR;
            default: return DEFAULT_COLOR;
        }
    }

    _buildTable() {
        if (!this.frameGraphData || !this.frameGraphData.passes || !this.frameGraphData.resources) return nothing;

        const allPasses = this.frameGraphData.passes.map(pass => pass.name);
        const resources = Object.values(this.frameGraphData.resources);

        return html`
            <div class="table-container" id="editor">
                <table>
                    <thead>
                    <tr>
                        <th>Resources</th>
                        ${allPasses.map(pass => html`<th>${pass}</th>`)}
                    </tr>
                    </thead>
                    <tbody>
                    ${resources.map(resource => html`
                        <tr>
                            <td>${resource.name}</td>
                            ${allPasses.map(passName => {
                                const passData = this.frameGraphData.passes.find(pass => pass.name === passName);
                                const isRead = passData?.reads.includes(resource.id);
                                const isWrite = passData?.writes.includes(resource.id);
                                let type = null;
                                if (isRead && isWrite) type = 'read-write';
                                else if (isRead) type = 'read';
                                else if (isWrite) type = 'write';
                                const color = type ? this._getCellColor(type) : DEFAULT_COLOR;
                                return html`<td style="background-color: ${unsafeCSS(color)};">${type ?? nothing}</td>`;
                            })}
                        </tr>
                    `)}
                    </tbody>
                </table>
            </div>
        `;
    }

    render() {
        return html`
          ${this._buildTable()}
        `;
    }
}
customElements.define("framegraph-table", FrameGraphTable);

class FrameGraphViewer extends LitElement {
    static get styles() {
        return css`
            :host {
                height: 100%;
                width: 100%;
                display: flex;
            }
        `;
    }

    get _sidepanel() {
        return this.renderRoot.querySelector('#sidepanel');
    }

    get _framegraph_table() {
        return this.renderRoot.querySelector('#table');
    }

    async init() {
        const isConnected = () => this.connected;
        statusLoop(
                isConnected,
                async (status, data) => {
                    this.connected = status == STATUS_CONNECTED || status == STATUS_FRAMEGRAPH_UPDATED;

                    if(status == STATUS_FRAMEGRAPH_UPDATED){
                        let fgInfo = await fetchFrameGraph(fgid);
                        this.database[fgInfo.fgid] = fgInfo;
                    }
                }
        );

        let framegraphs = await fetchFrameGraphs();
        this.database = framegraphs;
    }

    _getFrameGraph(){
        const framegraph = (this.database && this.currentFrameGraph) ? this.database[this.currentFrameGraph] : null;
        return framegraph;
    }

    _onResize() {
        const rect = this._sidepanel.getBoundingClientRect();
        this.tableExpectedWidth = window.innerWidth - rect.width - 1;
        this.tableExpectedHeight = window.innerHeight;
    }

    firstUpdated() {
        this._onResize();
    }

    constructor() {
        super();
        this.connected = false;
        this.database = {};
        this.currentFrameGraph = null;
        this.init();

        this.addEventListener('select-framegraph',
                (ev) => {
                    this.currentFrameGraph = ev.detail;
                }
        );

        addEventListener('resize', this._onResize.bind(this));
    }

    static get properties() {
        return {
            connected: {type: Boolean, state: true},
            database: {type: Object, state: true},
            currentFrameGraph: {type: String, state: true},
        }
    }

    updated(props){
        if (props.has('currentFrameGraph')) {
            const framegraph = this.database[this.currentFrameGraph];
            this._framegraph_table.frameGraphData = framegraph;
        }
        this._sidepanel.database = this.database;
    }


    render() {
        const framegraph = this._getFrameGraph();
        return html`
            <framegraph-sidepanel id="sidepanel"
                ?connected="${this.connected}"
                current-framegraph="${this.currentFrameGraph}"
            </framegraph-sidepanel>
            <framegraph-table id="table" 
                ?connected="${this.connected}"
                ?current-framegraph="${this.currentFrameGraph}" 
                expected-width="${this.tableExpectedWidth}" 
                expected-height="${this.tableExpectedHeight}" >
            </framegraph-table>>
        `;
    }
}

customElements.define("framegraph-viewer", FrameGraphViewer);
