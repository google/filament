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

const kUntitledPlaceholder = "Untitled View";

// CSS constants
const FOREGROUND_COLOR = '#fafafa';
const INACTIVE_COLOR = '#6f6f6f';
const UNSELECTED_COLOR = '#dfdfdf';
const BACKGROUND_COLOR = '#5362e5';
const REGULAR_FONT_SIZE = 12;

// Constants for color operations
const READ_COLOR = '#aee1a6';
const WRITE_COLOR = '#df8e8e';
const NO_ACCESS_COLOR = '#bcbcbc';
const READ_WRITE_COLOR = '#ffeb99';
const DEFAULT_COLOR = '#ffffff';
const SUBRESOURCE_COLOR = '#d3d3d3';

const SELECTED_ROW_COLUMN_COLOR = "rgba(180,210,240,.35)";

// Constants for view mode toggle
const VIEW_TOGGLE_INACTIVE_COLOR = '#292929';
const VIEW_TOGGLE_UNSELECTED_COLOR = '#3f4cbe';
const VIEW_TOGGLE_SELECTED_COLOR = '#2c3892';

const RESOURCE_USAGE_TYPE_READ = 'R';
const RESOURCE_USAGE_TYPE_WRITE = 'W';
const RESOURCE_USAGE_TYPE_NO_ACCESS = 'NA';
const RESOURCE_USAGE_TYPE_READ_WRITE = 'RW';

const IS_SUBRESOURCE_KEY = 'is_subresource_of'

// View mode constants
const VIEW_MODE_TABLE = 'table';
const VIEW_MODE_GRAPHVIZ = 'graphviz';

const COLORS = [
  "#FFB3BA",
  "#BAFFC9",
  "#BAE1FF",
  "#FFDFBA",
  "#E2C2FF",
  "#B2F0E8",
  "#DDCBA4",
  "#D4F0B2",
  "#DCD0FF"
];

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
                background: ${this.connected ? BACKGROUND_COLOR : INACTIVE_COLOR};
                height: 100%;
                max-width: 250px;
                min-width: 250px;
                overflow-y: auto;
            }
            .container {
                padding: 10px 20px;
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
            .resource-content {
                display: flex;
                flex-direction: column;
                font-size: ${REGULAR_FONT_SIZE}px;
                color: ${UNSELECTED_COLOR};
            }
            .view-mode-selector {
                display: flex;
                flex-direction: row;
                justify-content: space-between;
                margin-bottom: 15px;
            }
            .view-toggle {
                display: inline-block;
                background-color: ${this.connected ? VIEW_TOGGLE_UNSELECTED_COLOR:VIEW_TOGGLE_INACTIVE_COLOR};
                color: white;
                padding: 5px 10px;
                margin: 5px;
                border-radius: 4px;
                cursor: pointer;
                user-select: none;
                font-size: 12px;
                text-align: center;
                flex: 1;
            }
            .view-toggle.active {
                background-color: ${this.connected ? VIEW_TOGGLE_SELECTED_COLOR:VIEW_TOGGLE_INACTIVE_COLOR};
                font-weight: bold;
            }
            .resource-link:hover {
                text-decoration: underline;
            }
            .render-target-resource {
                display: flex;
                flex-direction: row;
            }
        `;
    }

    static get properties() {
        return {
            connected: {type: Boolean, attribute: 'connected'},
            selectedFrameGraph: {type: String, attribute: 'selected-framegraph'},
            selectedResourceId: {type: Number, attribute: 'selected-resource'},
            selectedPassIndex: {type: Number, attribute: 'selected-pass'},
            viewMode: {type: String, attribute: 'view-mode'},

            database: {type: Object, state: true},
            framegraphs: {type: Array, state: true},
        }
    }

    constructor() {
        super();
        this.connected = false;
        this.framegraphs = [];
        this.database = {};
        this.viewMode = VIEW_MODE_TABLE;
        this.selectedFrameGraph = '';
        this.selectedPassIndex = -1;
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
            // Select the 0-th view as default.
            if (this.selectedFrameGraph.length == 0 && this.framegraphs.length > 0) {
                this._handleFrameGraphClick(this.framegraphs[0].fgid);
            }
        }
    }

    _handleFrameGraphClick(ev) {
        this.dispatchEvent(new CustomEvent('select-framegraph', {
            detail: ev,
            bubbles: true,
            composed: true
        }));
    }

    _handleViewModeClick(mode) {
        this.dispatchEvent(new CustomEvent('change-view-mode', {
            detail: mode,
            bubbles: true,
            composed: true
        }));
    }

    _findCurrentResource() {
        if (!this.selectedFrameGraph) {
            return null;
        }

        const resources = this.database[this.selectedFrameGraph]?.resources;
        if (resources) {
            const resource = resources[this.selectedResourceId];
            if (resource) {
                return resource;
            }
        }
        return null;
    }

    _findCurrentPass() {
        if (!this.selectedFrameGraph || this.selectedPassIndex === -1 ||
                this.selectedPassIndex === undefined) {
            return null;
        }
        const passes = this.database[this.selectedFrameGraph]?.passes;
        return passes ? passes[this.selectedPassIndex] : null;
    }

    _handleResourceLinkClick(ev, id) {
        ev.stopPropagation();
        this.dispatchEvent(new CustomEvent('select-resource', {
            detail: id,
            bubbles: true,
            composed: true
        }));
    }

    _formatResourceId(id) {
        const resources = this.database[this.selectedFrameGraph]?.resources;
        const resource = resources ? resources[id] : null;
        const label = resource ? `${resource.name} [${id}]` : `[${id}]`;
        return html`<span class="resource-link" style="cursor: pointer;"
            @click="${(e) => this._handleResourceLinkClick(e, id)}">${label}</span>`;
    }

    render() {
        const renderFrameGraphs = (title) => {
            if (!this.framegraphs.length)
                return nothing;

            return html`
                <menu-section title="${title}">
                    ${this.selectedFrameGraph ? html`
                    <div class="view-mode-selector">
                        <div class="view-toggle ${this.viewMode === VIEW_MODE_TABLE ? 'active' : ''}"
                             @click="${() => this._handleViewModeClick(VIEW_MODE_TABLE)}">
                            Table Mode
                        </div>
                        <div class="view-toggle ${this.viewMode === VIEW_MODE_GRAPHVIZ ? 'active' : ''}"
                             @click="${() => this._handleViewModeClick(VIEW_MODE_GRAPHVIZ)}">
                            Graph Mode
                        </div>
                    </div>
                    ` : nothing}

                    <div class="framegraphs">
                        ${this.framegraphs.map(({fgid, name}) => {
                            const isSelected = this.selectedFrameGraph === fgid;
                            return html`
                                <div @click="${() => this._handleFrameGraphClick(fgid)}"
                                     class="framegraph ${isSelected ? 'selected' : ''}">
                                    ${isSelected ? '● ' : ''}${name}
                                </div>`;
                        })}
                    </div>
                </menu-section>
            `;
        };

        const renderResourceDetails = (title) => {
            const currentResource = this._findCurrentResource();
            if (!currentResource)
                return nothing;

            return html`
                <menu-section title="${title}">
                    <div class="resource-content">
                        <div><b>Name:</b> ${currentResource.name}
                            <b>[ID=${currentResource.id}]</b></div>
                    </div>
                    ${currentResource.properties.length > 0 ? html`
                        <div class="resource-content">
                            <div><b>Properties:</b></div>
                            <ul>
                                ${currentResource.properties.map(prop => html`
                                    <li>${prop.key}: ${prop.value}</li>
                                `)}
                            </ul>
                        </div>
                    ` : ''}
                </menu-section>
            `;
        };

        const renderPassDetails = (title) => {
            const currentPass = this._findCurrentPass();
            if (!currentPass) return nothing;

            const renderRenderTarget = (rt, index) => {
                const resourceFlags = {}; // id -> { name, flags: [] }

                const processList = (list, flagName) => {
                    if (!list) return;
                    for (const att of list) {
                        if (!resourceFlags[att.id]) {
                            resourceFlags[att.id] = { name: att.name, id: att.id, flags: [] };
                        }
                        resourceFlags[att.id].flags.push(flagName);
                    }
                };

                processList(rt.discardStart, "Discard Start");
                processList(rt.discardEnd, "Discard End");
                processList(rt.clear, "Clear");

                const resources = Object.values(resourceFlags);
                if (resources.length === 0) return nothing;

                return html`
                    <div class="resource-content render-target-resource"
                         style="margin-top: 5px; border-top: 1px dashed #6f6f6f;">
                        <div>(${index})</div>
                        <ul style="margin:0;padding-inline-start:25px">
                            ${resources.map(res => html`
                                <li>
                                    ${this._formatResourceId(res.id)}: ${res.flags.join(", ")}
                                </li>
                            `)}
                        </ul>
                    </div>
                `;
            };

            const joinResources = (ids) => {
              return ids.map((id, i) => html`
                  <li>${this._formatResourceId(id)}</li>`);
            };

            return html`
                <menu-section title="${title}">
                     <div class="resource-content">
                        <div><b>Name:</b> ${currentPass.name} <b>[ID=${currentPass.id}]</b></div>
                    </div>
                    ${currentPass.reads.length > 0 ? html`
                    <div class="resource-content">
                        <div><b>Reads:</b> </div>
                        <ul style="margin: 4px 0;">
                            ${joinResources(currentPass.reads)}
                        </ul>
                    </div>` : nothing}
                    ${currentPass.renderTargets && currentPass.renderTargets.length > 0 ?
                        html`<div><b>Writes (Render Targets):</b></div>
                        ${currentPass.renderTargets.map((rt, i) => renderRenderTarget(rt, i))}` : nothing}
                </menu-section>
            `;
        };

        return html`
            <style>${this.dynamicStyle()}</style>
            <div class="container">
                <div class="title">fgviewer</div>
                ${renderFrameGraphs("Views") ?? nothing}
                ${renderResourceDetails("Resource Details") ?? nothing}
                ${renderPassDetails("Pass Details") ?? nothing}
            </div>
        `;
    }
}

customElements.define("framegraph-sidepanel", FrameGraphSidePanel);

class TooltipElement extends LitElement {
    static get styles() {
        return css`
            :host {
                position: fixed;
                background-color: #333;
                color: #fff;
                padding: 4px 8px;
                border-radius: 4px;
                font-size: 12px;
                pointer-events: none;
                z-index: 10000;
                display: none;
                white-space: nowrap;
                box-shadow: 0 2px 4px rgba(0,0,0,0.2);
            }
            :host([visible]) {
                display: block;
            }
        `;
    }

    static get properties() {
        return {
            text: {type: String},
            visible: {type: Boolean, reflect: true},
            x: {type: Number},
            y: {type: Number}
        };
    }

    updated(changedProps) {
        if (changedProps.has('x') || changedProps.has('y')) {
            // Offset slightly so cursor doesn't cover it
            this.style.left = `${this.x + 10}px`;
            this.style.top = `${this.y + 10}px`;
        }
    }

    render() {
        return html`${this.text}`;
    }
}
customElements.define('tooltip-element', TooltipElement);

class FrameGraphTable extends LitElement {
    static get styles() {
        return css`
            :host {
                display: block;
                font-size: 12px;
                margin-top: 100px;
                margin-left: 20px;
            }
            .grid-table {
                display: flex;
                flex-direction: column;
            }
            .grid-row {
                display: inline-grid;
                /* Adjust column widths here (e.g., first column gets 2 parts, others get 1) */
                grid-template-columns: 2fr 1fr 1fr;
            }
            .grid-cell:first-child {
            }
            .grid-cell {
                padding: 2px;
                min-height: 25px;
                position: relative;
            }
            .top-row {
                border: 1px solid rgba(0, 0, 0, 0);
            }
            .pass-cell {
                position: absolute;
                transform-origin: top left;
                transform: rotateZ(-45deg);
                top: 17px;
                left: 10px;
                width: 150px;
            }
            .collapsible {
                background-color: #f9f9f9;
            }
            .toggle-icon {
                cursor: pointer;
                margin-right: 5px;
                user-select: none;
            }
            .selected {
                text-decoration: underline;
                font-weight: bold;
            }
            .sticky-col {
                text-align: right;
                padding-right: 8px;
            }
            .resource:hover {
                text-decoration: underline;
            }
            .resource {
                cursor: pointer;
            }
        `;
    }

    static get properties() {
        return {
            frameGraphData: {type: Object, state: true}, // Expecting a JSON frame graph structure
            selectedResourceId: {type: Number, attribute: 'selected-resource'},
            selectedPassIndex: {type: Number, attribute: 'selected-pass'},
            tooltipText: {type: String, state: true},
            tooltipVisible: {type: Boolean, state: true},
            tooltipX: {type: Number, state: true},
            tooltipY: {type: Number, state: true}
        };
    }

    constructor() {
        super();
        this.frameGraphData = null;
        this.selectedResourceId = -1;
        this.selectedPassIndex = -1;
        this.expandedResourceSet = new Set();
        this.subresourceToParent = {};
        this._hoverTimeout = null;
    }

    _handleCellMouseEnter(e, text) {
        if (this._hoverTimeout) {
            clearTimeout(this._hoverTimeout);
        }
        this._hoverTimeout = setTimeout(() => {
            this.tooltipText = text;
            this.tooltipVisible = text && text.length > 0;
            this.tooltipX = e.clientX;
            this.tooltipY = e.clientY;
        }, 500); // 500ms delay
    }

    _handleCellMouseLeave() {
        if (this._hoverTimeout) {
            clearTimeout(this._hoverTimeout);
            this._hoverTimeout = null;
        }
        this.tooltipVisible = false;
    }

    _getRowGridTemplateColumnsStyle(numColumns) {
      let out = '185px ';
      const oneCol = '35px ';
      for (let i = 0; i < numColumns; i++) {
        out += oneCol;
      }
      return `grid-template-columns:${out};`;
    }

    updated(props) {
        if (props.has('frameGraphData')) {
            this.subresourceToParent = {};
            if (this.frameGraphData && this.frameGraphData.resources) {
                const resources = Object.values(this.frameGraphData.resources);
                resources.forEach((subres) => {
                    resources.forEach((parent) => {
                        if (this._isSubresourceOfParent(subres, parent.id)) {
                            this.subresourceToParent[subres.id] = parent.id;
                        }
                    });
                });
            }
            this.requestUpdate();
        }

        if (props.has('selectedResourceId')) {
            const parentId = this.subresourceToParent[this.selectedResourceId];
            if (parentId && !this.expandedResourceSet.has(parentId)) {
                this.expandedResourceSet.add(parentId);
                this.requestUpdate();
            }
        }
    }

    _getCellColor(type, defaultColor) {
        return {
            [RESOURCE_USAGE_TYPE_READ]: READ_COLOR,
            [RESOURCE_USAGE_TYPE_WRITE]: WRITE_COLOR,
            [RESOURCE_USAGE_TYPE_NO_ACCESS]: NO_ACCESS_COLOR,
            [RESOURCE_USAGE_TYPE_READ_WRITE]: READ_WRITE_COLOR
        }[type] || defaultColor;
    }

    _toggleCollapse(resourceId) {
        if (this.expandedResourceSet.has(resourceId)) {
            this.expandedResourceSet.delete(resourceId);
        }
        else {
            this.expandedResourceSet.add(resourceId);
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

    _renderResourceUsage(allPasses, resourceIds, defaultColor, lastRow) {
        const passLen = allPasses.length;
        return allPasses.map((passData, index) => {
            const isSelectedPass = index === this.selectedPassIndex;
            const isRead = resourceIds.some(resourceId => passData?.reads.includes(resourceId));
            const isWrite = resourceIds.some(resourceId => passData?.writes.includes(resourceId));
            let restype = null;

            const hasUsed = (passData) => {
                return resourceIds.some(resourceId =>
                    passData?.reads.includes(resourceId) || passData?.writes.includes(resourceId)
                );
            };
            const hasBeenUsedBefore = allPasses.slice(0, index).some(hasUsed);
            const willBeUsedLater = allPasses.slice(index + 1).some(hasUsed);

            if (isRead && isWrite) restype = RESOURCE_USAGE_TYPE_READ_WRITE;
            else if (isRead) restype = RESOURCE_USAGE_TYPE_READ;
            else if (isWrite) restype = RESOURCE_USAGE_TYPE_WRITE;
            else if (hasBeenUsedBefore && willBeUsedLater) restype = RESOURCE_USAGE_TYPE_NO_ACCESS;

            const lastPass = index == passLen - 1;
            const borderStyle = '1px solid #a5a5a5';
            const bgColor = isSelectedPass ? this._getCellColor(restype, SELECTED_ROW_COLUMN_COLOR)
                                           : this._getCellColor(restype, defaultColor);
            const style = `background-color:${bgColor};` +
                          `border-left:${borderStyle};border-top:${borderStyle};` +
                          (lastPass ? `border-right:${borderStyle};` : '') +
                          (lastRow ? `border-bottom:${borderStyle};` : '');

            let tooltip = '';
            if (restype === RESOURCE_USAGE_TYPE_READ) tooltip = 'Read';
            else if (restype === RESOURCE_USAGE_TYPE_WRITE) tooltip = 'Write';
            else if (restype === RESOURCE_USAGE_TYPE_READ_WRITE) tooltip = 'Read-Write';
            else if (restype === RESOURCE_USAGE_TYPE_NO_ACCESS) tooltip = 'No Access';

            return html`
                <div class="grid-cell" style="${style}"
                     @mouseenter="${(e) => this._handleCellMouseEnter(e, tooltip)}"
                     @mouseleave="${this._handleCellMouseLeave}">
                &nbsp;
                </div>`;
        });
    }

    _isSubresourceOfParent(resource, parentResourceId){
        return resource.properties?.some(prop =>
                prop.key === IS_SUBRESOURCE_KEY &&
                Number(prop.value) === parentResourceId)
    }

    _isSubresource(resource) {
        return resource.properties?.some(prop => prop.key === IS_SUBRESOURCE_KEY);
    }

    _renderResourceRows(resources, allPasses) {
        const isExpandedSubresource = (subres) => {
            const parentId = this.subresourceToParent[subres.id];
            return !!parentId && this.expandedResourceSet.has(parentId);
        }
        const allRes = resources
              .filter(resource => (
                !this._isSubresource(resource) || isExpandedSubresource(resource)
              ));

        const allResLen = allRes.length;
        return allRes.map((resource, resourceIndex) =>
          this._renderResourceRow(
            resource, resources, allPasses, resourceIndex == allResLen-1));
    }

    _renderResourceRow(resource, resources, allPasses, lastRow) {
        const subresourceIds = resources
            .filter(subresource => this._isSubresourceOfParent(subresource, resource.id))
            .map(subresource => subresource.id);

        const hasSubresources = subresourceIds.length > 0;
        const isExpanded = this.expandedResourceSet.has(resource.id);
        // Show the aggregated resource usage when the subresources are collapsed.
        const resourceIds = isExpanded ? [resource.id]:[resource.id, ...subresourceIds];

        // This will give the parent and expanded-subresources a coloring hint.
        let groupColorStyle = "";
        if (!!this.subresourceToParent[resource.id]) {
            const colorIndex = this.subresourceToParent[resource.id] % COLORS.length;
            groupColorStyle = "border-right:5px solid " + COLORS[colorIndex];
        } else if (isExpanded) {
            const colorIndex = resource.id % COLORS.length;
            groupColorStyle = "border-right:5px solid " + COLORS[colorIndex];
        }

        const isSelectedResource = resource.id === this.selectedResourceId;
        const onClickResource = () => this._handleResourceClick(resource.id);
        const selectedStyle = isSelectedResource ? "selected" : "";
        const rowStyle = this._getRowGridTemplateColumnsStyle(allPasses.length);
        const defaultColor = isSelectedResource ? SELECTED_ROW_COLUMN_COLOR : DEFAULT_COLOR;
        return html`
            <div class="grid-row" id="resource-${resource.id}" style="${rowStyle}">
                <div class="grid-cell sticky-col resource ${selectedStyle}" style="${groupColorStyle}"
                     @click="${onClickResource}">
                    ${hasSubresources
                        ? html`
                            <span class="toggle-icon"
                                  @click="${(e) => { e.stopPropagation(); this._toggleCollapse(resource.id); }}">
                              ${isExpanded ? '▼' : '▶'}
                            </span>`
                        : nothing}
                    ${resource.name}
                    ${hasSubresources && !isExpanded ? html`(${subresourceIds.length})` : nothing}
                </div>
                ${this._renderResourceUsage(allPasses, resourceIds, defaultColor, lastRow)}
            </div>
        `;
    }

    _handlePassClick(index) {
        this.dispatchEvent(new CustomEvent('select-pass', {
            detail: index,
            bubbles: true,
            composed: true
        }));
    }

    render() {
        if (!this.frameGraphData?.passes || !this.frameGraphData?.resources)
            return nothing;

        const allPasses = this.frameGraphData.passes;
        const resources = Object.values(this.frameGraphData.resources);
        const rowStyle = this._getRowGridTemplateColumnsStyle(allPasses.length);
        const colStyle = "border:1px solid rgba(0,0,0,0)";
        return html`
            <div class="grid-table">
                <div class="grid-row" style="${rowStyle}">
                    <div class="grid-cell" style="${colStyle}">
                    </div>
                    ${allPasses.map((pass, index) => {
                        const isSelected = index === this.selectedPassIndex;
                        const gclass = isSelected ? "grid-cell selected" : "grid-cell";
                        const pclass = isSelected ? "pass-cell selected" : "pass-cell";
                        return html`<div class="${gclass}" style="border:1px solid rgba(0,0,0,0); cursor: pointer;"
                                         @click="${() => this._handlePassClick(index)}">
                              <div class="${pclass}">${pass.name}</div></div>`;
                    })}
                </div>
                ${this._renderResourceRows(resources, allPasses)}
            </div>
            <tooltip-element
                .text="${this.tooltipText}"
                ?visible="${this.tooltipVisible}"
                .x="${this.tooltipX}"
                .y="${this.tooltipY}">
            </tooltip-element>
        `;
    }
}

customElements.define("framegraph-table", FrameGraphTable);

class GraphvizView extends LitElement {
    static get styles() {
        return css`
            :host {
                display: block;
                flex-grow: 1;
                width: 80%;
            }

            .graphviz-container {
                margin-left: 300px;
                height: 100vh;
                width: calc(100% - 300px);
                overflow: auto;
                background-color: white;
            }

            #graph {
                width: 100%;
                height: 100%;
            }
        `;
    }

    static get properties() {
        return {
            graphvizData: {type: String, state: true},
        };
    }

    constructor() {
        super();
        this.graphvizData = '';
    }

    updated(changedProps) {
        if (changedProps.has('graphvizData')) {
            this._renderGraphviz();
        }
    }

    _renderGraphviz() {
        if (!this.graphvizData)
            return;

        const container = this.renderRoot.querySelector('#graphviz-view');
        if (!container)
            return;

        try {
            const viz = d3.select(container)
                .graphviz({ useWorker: false })
                .zoom(true)
                .fit(true);

            viz.renderDot(this.graphvizData);
        } catch (error) {
            console.error('Failed to render graphviz:', error);
            container.innerHTML = `<div class="error">Failed to render graphviz: ${error.message}</div>`;
        }
    }

    render() {
        if(!this.graphvizData)
            return nothing;

        return html`
            <div class="graphviz-container">
                <div id="graphviz-view"></div>
            </div>
        `;
    }
}

customElements.define("graphviz-view", GraphvizView);

class FrameGraphViewer extends LitElement {
    static get styles() {
        return css`
            :host {
                height: 100%;
                width: 100%;
                display: flex;
                flex-direction: row;
            }
        `;
    }

    get _sidePanel() {
        return this.renderRoot.querySelector('#sidepanel');
    }

    get _framegraphTable() {
        return this.renderRoot.querySelector('#table');
    }

    get _graphvizView() {
        return this.renderRoot.querySelector('#graphviz');
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
                        this._graphvizView.graphvizData = fgInfo.graphviz;
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
        this.selectedPassIndex = -1;
        this.viewMode = VIEW_MODE_TABLE;
        this.init();

        this.addEventListener('select-framegraph',
                (ev) => {
                    this.selectedFrameGraph = ev.detail;
                }
        );

        this.addEventListener('select-resource',
            (ev) => {
                this.selectedResourceId = this.selectedResourceId === ev.detail ? -1 : ev.detail;
            }
        );

        this.addEventListener('select-pass',
            (ev) => {
                this.selectedPassIndex = this.selectedPassIndex === ev.detail ? -1 : ev.detail;
            }
        );

        this.addEventListener('change-view-mode',
            (ev) => {
                this.viewMode = ev.detail;
            }
        );
    }

    static get properties() {
        return {
            connected: {type: Boolean, state: true},
            database: {type: Object, state: true},
            selectedFrameGraph: {type: String, state: true},
            selectedResourceId: {type: Number, state: true},
            selectedPassIndex: {type: Number, state: true},
            viewMode: {type: String, state: true},
        }
    }

    updated(props) {
        if (props.has('selectedFrameGraph') || props.has('database')) {
            const framegraph = this._getFrameGraph();
            this._framegraphTable.frameGraphData = framegraph;
            this._graphvizView.graphvizData = framegraph?.graphviz;
            this._sidePanel.database = this.database;
        }
    }


    render() {
        return html`
            <framegraph-sidepanel id="sidepanel"
                ?connected="${this.connected}"
                selected-framegraph="${this.selectedFrameGraph}"
                selected-resource="${this.selectedResourceId}"
                selected-pass="${this.selectedPassIndex}"
                view-mode="${this.viewMode}">
            </framegraph-sidepanel>

            <framegraph-table id="table"
                style="display: ${this.viewMode === VIEW_MODE_TABLE ? 'block' : 'none'};"
                ?connected="${this.connected}"
                selected-framegraph="${this.selectedFrameGraph}"
                selected-resource="${this.selectedResourceId}"
                selected-pass="${this.selectedPassIndex}">
            </framegraph-table>

            <graphviz-view id="graphviz"
                style="display: ${this.viewMode === VIEW_MODE_GRAPHVIZ ? 'block' : 'none'};"
                framegraph-id="${this.selectedFrameGraph}">
            </graphviz-view>
        `;
    }
}

customElements.define("framegraph-viewer", FrameGraphViewer);
