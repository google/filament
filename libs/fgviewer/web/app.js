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

import {LitElement, html, css, unsafeCSS, nothing} from "https://unpkg.com/lit@2.8.0?module";

const kUntitledPlaceholder = "untitled";

// CSS constants
const FOREGROUND_COLOR = '#fafafa';
const INACTIVE_COLOR = '#6f6f6f';
const UNSELECTED_COLOR = '#dfdfdf';
const BACKGROUND_COLOR = '#5362e5';
const REGULAR_FONT_SIZE = 12;

// Constants for color operations
const READ_COLOR = '#3ac224';
const WRITE_COLOR = '#d43232';
const NO_ACCESS_COLOR = '#d8dde754';
const READ_WRITE_COLOR = '#ffeb99';
const DEFAULT_COLOR = '#ffffff';
const SUBRESOURCE_COLOR = '#d3d3d3';

const RESOURCE_USAGE_TYPE_READ = 'read';
const RESOURCE_USAGE_TYPE_WRITE = 'write';
const RESOURCE_USAGE_TYPE_NO_ACCESS = 'no-access';
const RESOURCE_USAGE_TYPE_READ_WRITE = 'read-write';

const IS_SUBRESOURCE_KEY = 'is_subresource'

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
        const slot = (() => html`
            <slot></slot>`)();
        return html`
            <div class="container">
                <div class="section-title expander" @click="${this._showClick}">
                    <span>${this.title}</span> <span>${expandedIcon}</span>
                </div>
                <hr/>
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
                position: fixed;
                background: ${this.connected ? BACKGROUND_COLOR : INACTIVE_COLOR};
                width: 20%;
                height: 100%;
                max-width: 250px;
                min-width: 250px;
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

    _handleFrameGraphClick(ev) {
        this.dispatchEvent(new CustomEvent('select-framegraph', {
            detail: ev,
            bubbles: true,
            composed: true
        }));
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
                return html`
                    <menu-section title="${title}">${fgs}</menu-section>`;
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
                display: block;
                flex-grow: 1;
                width: 80%;
            }

            .table-container {
                max-height: 100%;
                max-width: 100%;
                overflow: auto;
                border: 1px solid #ddd;
                margin-left: 300px;
            }

            .scrollable-table {
                width: 100%;
                height: 100%;
                border-collapse: collapse;
            }

            .collapsible {
                background-color: #f9f9f9;
            }

            .toggle-icon {
                cursor: pointer;
                margin-right: 5px;
                user-select: none;
            }

            .hidden {
                display: none;
            }

            .scrollable-table td,
            .scrollable-table th {
                padding: 12px;
                text-align: left;
                border: 1px solid #ddd;
            }

            .scrollable-table tr {
                position: sticky;
                padding: 12px;
                text-align: left;
                border: 1px solid #ddd;
                left: 0;
                z-index: 1;
            }

            .sticky-col {
                position: sticky;
                left: 0;
                z-index: 1;
            }

            th {
                min-width: 100px;
                background-color: #f2f2f2;
                padding: 12px;
                text-align: left;
                border: 1px solid #ddd;
            }
        `;
    }

    static get properties() {
        return {
            frameGraphData: {type: Object, state: true}, // Expecting a JSON frame graph structure
        };
    }

    constructor() {
        super();
        this.frameGraphData = null;
    }

    updated(props) {
        if (props.has('frameGraphData')) {
            this.requestUpdate();
        }
    }

    _getCellColor(type, defaultColor) {
        switch (type) {
            case RESOURCE_USAGE_TYPE_READ:
                return READ_COLOR;
            case RESOURCE_USAGE_TYPE_WRITE:
                return WRITE_COLOR;
            case RESOURCE_USAGE_TYPE_NO_ACCESS:
                return NO_ACCESS_COLOR;
            case RESOURCE_USAGE_TYPE_READ_WRITE:
                return READ_WRITE_COLOR;
            default:
                return defaultColor;
        }
    }

    _toggleCollapse(resourceIndex) {
        const subresourceRows = this.shadowRoot.querySelectorAll(`[id^="subresource-${resourceIndex}-"]`);
        const icon = this.shadowRoot.querySelector(`#resource-${resourceIndex} .toggle-icon`);
        const isHidden = subresourceRows[0]?.classList.contains('hidden');
        subresourceRows.forEach(row => row.classList.toggle('hidden'));
        icon.textContent = isHidden ? '▼' : '▶';
    }

    _getRowHtml(allPasses, resourceId, defaultColor) {
        return allPasses.map((passName, index) => {
            const passData = this.frameGraphData.passes[index];
            const isRead = passData?.reads.includes(resourceId);
            const isWrite = passData?.writes.includes(resourceId);
            let type = null;
            // TODO: prevent using name to query
            const getPassData = (index) => this.frameGraphData.passes.find(pass => pass.name === name);
            const hasUsed = (name) => {
                const passData = getPassData(name);
                return passData?.reads.includes(resourceId) || passData?.writes.includes(resourceId);
            };
            const hasBeenUsedBefore = allPasses.slice(0, index).some(hasUsed);
            const willBeUsedLater = allPasses.slice(index + 1).some(hasUsed);

            if (isRead && isWrite) type = RESOURCE_USAGE_TYPE_READ_WRITE;
            else if (isRead) type = RESOURCE_USAGE_TYPE_READ;
            else if (isWrite) type = RESOURCE_USAGE_TYPE_WRITE;
            else if (hasBeenUsedBefore && willBeUsedLater) type = RESOURCE_USAGE_TYPE_NO_ACCESS;
            return html`
                <td style="background-color: ${unsafeCSS(this._getCellColor(type, defaultColor))};">
                    ${type ?? nothing}
                </td>`;
        });
    }

    _isSubresourceOfParent(resource, parentResource){
        return resource.properties?.some(prop =>
                prop.key === IS_SUBRESOURCE_KEY &&
                Number(prop.value) === parentResource.id)
    }

    render() {
        if (!this.frameGraphData || !this.frameGraphData.passes || !this.frameGraphData.resources) return nothing;
        const allPasses = this.frameGraphData.passes.map(pass => pass.name);
        const resources = Object.values(this.frameGraphData.resources);
        return html`
            <div class="table-container">
                <table class="scrollable-table">
                    <thead>
                    <tr>
                        <th class="sticky-col">Resources/Passes</th>
                        ${allPasses.map(pass => html`
                            <th>${pass}</th>`)}
                    </tr>
                    </thead>
                    <tbody>
                    ${resources.map((resource, resourceIndex) => {
                        const isSubresource = resource.properties?.some(prop => prop.key === IS_SUBRESOURCE_KEY);
                        if (isSubresource) return nothing;

                        const hasSubresources = resources.some(subresource => this._isSubresourceOfParent(subresource, resource));
                        return html`
                            <tr id="resource-${resourceIndex}">
                                <th class="sticky-col">
                                    ${hasSubresources ? html`
                                        <span
                                            class="toggle-icon"
                                            @click="${() => this._toggleCollapse(resourceIndex)}"
                                        >▶</span>` : nothing}
                                    ${resource.name}
                                </th>
                                ${this._getRowHtml(allPasses, resource.id, DEFAULT_COLOR)}
                            </tr>
                            ${resources.filter(subresource => this._isSubresourceOfParent(subresource, resource)
                            ).map((subresource, subIndex) => html`
                                <tr id="subresource-${resourceIndex}-${subIndex}"
                                    class="collapsible hidden">
                                    <td class="sticky-col"
                                        style="background-color: ${SUBRESOURCE_COLOR}">
                                        ${subresource.name}
                                    </td>
                                    ${this._getRowHtml(allPasses, subresource.id, SUBRESOURCE_COLOR)}
                                </tr>
                            `)}
                        `;
                    })}
                    </tbody>
                </table>
            </div>
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

    get _sidePanel() {
        return this.renderRoot.querySelector('#sidepanel');
    }

    get _framegraphTable() {
        return this.renderRoot.querySelector('#table');
    }

    async init() {
        const isConnected = () => this.connected;
        statusLoop(
                isConnected,
                async (status, fgid) => {
                    this.connected = status == STATUS_CONNECTED || status == STATUS_FRAMEGRAPH_UPDATED;

                    if (status == STATUS_FRAMEGRAPH_UPDATED) {
                        let fgInfo = await fetchFrameGraph(fgid);
                        this.database[fgInfo.fgid] = fgInfo;
                        this._framegraphTable.frameGraphData = fgInfo;
                    }
                }
        );

        let framegraphs = await fetchFrameGraphs();
        this.database = framegraphs;
    }

    _getFrameGraph() {
        const framegraph = (this.database && this.currentFrameGraph) ?
                this.database[this.currentFrameGraph] : null;
        return framegraph;
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
    }

    static get properties() {
        return {
            connected: {type: Boolean, state: true},
            database: {type: Object, state: true},
            currentFrameGraph: {type: String, state: true},
        }
    }

    updated(props) {
        if (props.has('currentFrameGraph') || props.has('database')) {
            const framegraph = this._getFrameGraph();
            this._framegraphTable.frameGraphData = framegraph;
            this._sidePanel.database = this.database;
        }
    }


    render() {
        return html`
            <framegraph-sidepanel id="sidepanel"
                ?connected="${this.connected}"
                current-framegraph="${this.currentFrameGraph}" >
            </framegraph-sidepanel>
            <framegraph-table id="table" 
                ?connected="${this.connected}"
                current-framegraph="${this.currentFrameGraph}"
            </framegraph-table>
        `;
    }
}

customElements.define("framegraph-viewer", FrameGraphViewer);
