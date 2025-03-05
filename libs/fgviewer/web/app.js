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

const IS_SUBRESOURCE_KEY = 'is_subresource_of'

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
            .resource-title {
                display: flex;
                flex-direction: column;
                margin-bottom: 5px;
                font-size: ${REGULAR_FONT_SIZE}px;
                color: ${UNSELECTED_COLOR};
            }
            .resource-content {
                display: flex;
                flex-direction: column;
                font-size: ${REGULAR_FONT_SIZE}px;
                color: ${UNSELECTED_COLOR};
            }
        `;
    }

    static get properties() {
        return {
            connected: {type: Boolean, attribute: 'connected'},
            selectedFrameGraph: {type: String, attribute: 'selected-framegraph'},
            selectedResourceId: {type: Number, attribute: 'selected-resource'},

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
            const fgids = Object.keys(this.database).sort();
            const labelCount = {};

            this.framegraphs = fgids.map(fgid => {
                const name = this.database[fgid].viewName || kUntitledPlaceholder;
                const uniqueName = labelCount[name] !== undefined
                    ? `${name} (${labelCount[name]++})`
                    : (labelCount[name] = 0, name);
                return { fgid, name: uniqueName };
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

    _findCurrentResource() {
        if (!this.selectedFrameGraph)
            return null;
        const frameGraph = this.database[this.selectedFrameGraph];
        return Object.values(frameGraph?.resources)
            .find(resource => resource.id === this.selectedResourceId) || null;
    }

    render() {
        const renderFrameGraphs = (title) => {
            if (!this.framegraphs.length) return nothing;

            return html`
                <menu-section title="${title}">
                    ${this.framegraphs.map(({ fgid, name }) => html`
                        <div class="framegraph" 
                            @click="${() => this._handleFrameGraphClick(fgid)}" 
                            data-id="${fgid}">
                            ${fgid === this.selectedFrameGraph ? '● ' : ''}${name}
                        </div>
                    `)}
                </menu-section>
            `;
        };

        const renderResourceDetails = (title) => {
            const currentResource = this._findCurrentResource();
            if (!currentResource) return nothing;

            return html`
                <menu-section title="${title}">
                    <div class="resource-title">${currentResource.id}: ${currentResource.name}</div>
                    ${currentResource.properties?.map(({ key, value }) => html`
                <div class="resource-content">${key}: ${value}</div>
            `)}
                </menu-section>
            `;
        };

        return html`
            <style>${this.dynamicStyle()}</style>
            <div class="container">
                <div class="title">fgviewer</div>
                ${renderFrameGraphs("Views") ?? nothing}
                ${renderResourceDetails("Resource Details") ?? nothing}
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
            .scrollable-table td,
            .scrollable-table th {
                padding: 12px;
                text-align: left;
                border: 1px solid #ddd;
            }

            .selected {
                text-decoration: underline;
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

            .resource:hover {
                text-decoration: underline;
            }
            .resource {
                cursor: pointer;
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
            selectedResourceId: {type: Number, attribute: 'selected-resource'},
        };
    }

    constructor() {
        super();
        this.frameGraphData = null;
        this.selectedResourceId = -1;
        this.collapsedResources = new Set();
    }

    updated(props) {
        if (props.has('frameGraphData')) {
            this.requestUpdate();
        }
    }

    _getCellColor(type) {
        return {
            [RESOURCE_USAGE_TYPE_READ]: READ_COLOR,
            [RESOURCE_USAGE_TYPE_WRITE]: WRITE_COLOR,
            [RESOURCE_USAGE_TYPE_NO_ACCESS]: NO_ACCESS_COLOR,
            [RESOURCE_USAGE_TYPE_READ_WRITE]: READ_WRITE_COLOR
        }[type] || DEFAULT_COLOR;
    }

    _toggleCollapse(resourceIndex) {
        if (this.collapsedResources.has(resourceIndex)) {
            this.collapsedResources.delete(resourceIndex);
        }
        else {
            this.collapsedResources.add(resourceIndex);
        }
        this.requestUpdate();
    }

    _handleResourceClick(ev) {
        this.dispatchEvent(new CustomEvent('select-resource', {
            detail: ev,
            bubbles: true,
            composed: true
        }));
    }

    _renderResourceUsage(allPasses, resourceIds, defaultColor) {
        return allPasses.map((passData, index) => {
            const isRead = resourceIds.some(resourceId => passData?.reads.includes(resourceId));
            const isWrite = resourceIds.some(resourceId => passData?.writes.includes(resourceId));
            let type = null;

            const hasUsed = (passData) => {
                return resourceIds.some(resourceId =>
                    passData?.reads.includes(resourceId) || passData?.writes.includes(resourceId)
                );
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

    _isSubresource(resource) {
        return resource.properties?.some(prop => prop.key === IS_SUBRESOURCE_KEY);
    }

    _renderResourceRows(resources, allPasses) {
        return resources
            .filter(resource => !this._isSubresource(resource))
            .map((resource, resourceIndex) => this._renderResourceRow(resource, resourceIndex, resources, allPasses));
    }

    _renderResourceRow(resource, resourceIndex, resources, allPasses) {
        const subresourceIds = resources
            .filter(subresource => this._isSubresourceOfParent(subresource, resource))
            .map(subresource => subresource.id);

        const hasSubresources = subresourceIds.length > 0;
        const isCollapsed = this.collapsedResources.has(resourceIndex);
        // Show the aggregated resource usage when the subresources are collapsed.
        const resourceIds = isCollapsed ? [resource.id, ...subresourceIds]:[resource.id];

        const onClickResource = () => this._handleResourceClick(resource.id);
        const selectedStyle = resource.id === this.selectedResourceId ? "selected" : "";

        return html`
            <tr id="resource-${resourceIndex}">
                <th class="sticky-col resource ${selectedStyle}" @click="${onClickResource}">
                    ${hasSubresources 
                        ? html`
                            <span class="toggle-icon"
                                  @click="${(e) => { e.stopPropagation(); this._toggleCollapse(resourceIndex); }}">
                              ${isCollapsed ? '▶' : '▼'}
                            </span>` 
                        : nothing}
                    ${resource.name}
                    ${hasSubresources && isCollapsed ? html`(${subresourceIds.length})` : nothing}
                </th>
                ${this._renderResourceUsage(allPasses, resourceIds, DEFAULT_COLOR)}
            </tr>
            ${!isCollapsed ? this._renderSubresourceRows(resources, resource, resourceIndex, allPasses) : nothing}
        `;
    }


    _renderSubresourceRows(resources, parentResource, resourceIndex, allPasses) {
        return resources
            .filter(subresource => this._isSubresourceOfParent(subresource, parentResource))
            .map((subresource, subIndex) => this._renderSubresourceRow(subresource, resourceIndex, subIndex, allPasses));
    }

    _renderSubresourceRow(subresource, resourceIndex, subIndex, allPasses) {
        const onClickResource = () => this._handleResourceClick(subresource.id);
        const selectedStyle = subresource.id === this.selectedResourceId ? "selected" : "";

        return html`
        <tr id="subresource-${resourceIndex}-${subIndex}" class="collapsible">
            <td class="sticky-col resource ${selectedStyle}"
                @click="${onClickResource}"
                style="background-color: ${SUBRESOURCE_COLOR}">
                ${subresource.name}
            </td>
            ${this._renderResourceUsage(allPasses, [subresource.id], SUBRESOURCE_COLOR)}
        </tr>
    `;
    }

    render() {
        if (!this.frameGraphData?.passes || !this.frameGraphData?.resources) return nothing;

        const allPasses = this.frameGraphData.passes;
        const resources = Object.values(this.frameGraphData.resources);

        return html`
            <div class="table-container">
                <table class="scrollable-table">
                    <thead>
                    <tr>
                        <th class="sticky-col">Resources/Passes</th>
                        ${allPasses.map(pass => html`<th>${pass.name}</th>`)}
                    </tr>
                    </thead>
                    <tbody>
                    ${this._renderResourceRows(resources, allPasses)}
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
        const framegraph = (this.database && this.selectedFrameGraph) ?
                this.database[this.selectedFrameGraph] : null;
        return framegraph;
    }

    constructor() {
        super();
        this.connected = false;
        this.database = {};
        this.selectedFrameGraph = null;
        this.selectedResourceId = -1;
        this.init();

        this.addEventListener('select-framegraph',
                (ev) => {
                    this.selectedFrameGraph = ev.detail;
                }
        );

        this.addEventListener('select-resource',
            (ev) => {
                this.selectedResourceId = ev.detail;
            }
        );
    }

    static get properties() {
        return {
            connected: {type: Boolean, state: true},
            database: {type: Object, state: true},
            selectedFrameGraph: {type: String, state: true},
            selectedResourceId: {type: Number, state: true},
        }
    }

    updated(props) {
        if (props.has('selectedFrameGraph') || props.has('database')) {
            const framegraph = this._getFrameGraph();
            this._framegraphTable.frameGraphData = framegraph;
            this._sidePanel.database = this.database;
        }
    }


    render() {
        return html`
            <framegraph-sidepanel id="sidepanel"
                ?connected="${this.connected}"
                selected-framegraph="${this.selectedFrameGraph}" 
                selected-resource="${this.selectedResourceId}">
            </framegraph-sidepanel>
            <framegraph-table id="table" 
                ?connected="${this.connected}"
                selected-framegraph="${this.selectedFrameGraph}" 
                selected-resource="${this.selectedResourceId}">
            </framegraph-table>
        `;
    }
}

customElements.define("framegraph-viewer", FrameGraphViewer);
