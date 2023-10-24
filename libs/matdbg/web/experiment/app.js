/*
* Copyright (C) 2023 The Android Open Source Project
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

// Maps to backend to the languages allowed for that backend.
const LANGUAGE_CHOICES = {
    'opengl': ['glsl'],
    'vulkan': ['glsl', 'spirv'],
    'metal': ['msl'],
};

// CSS constants
const FOREGROUND_COLOR = '#fafafa';
const INACTIVE_COLOR = '#9a9a9a';
const DARKER_INACTIVE_COLOR = '#6f6f6f';
const LIGHTER_INACTIVE_COLOR = '#d9d9d9';
const UNSELECTED_COLOR = '#dfdfdf';
const BACKGROUND_COLOR = '#5362e5';
const HOVER_BACKGROUND_COLOR = '#b3c2ff';

// Set up the Monaco editor. See also CodeViewer
const kMonacoBaseUrl = 'https://cdnjs.cloudflare.com/ajax/libs/monaco-editor/0.25.2/min/';
require.config({
    paths: { "vs": `${kMonacoBaseUrl}vs` },
    'vs/css': { disabled: true },
});
window.MonacoEnvironment = {
    getWorkerUrl: function() {
      return `data:text/javascript;charset=utf-8,${encodeURIComponent(`
          self.MonacoEnvironment = {
              baseUrl: '${kMonacoBaseUrl}'
          };
          importScripts('${kMonacoBaseUrl}vs/base/worker/workerMain.js');`
      )}`;
    }
};

class Button extends LitElement {
    static get styles() {
        return css`
            :host {
                display: flex;
            }
            .main {
                border: solid 2px ${unsafeCSS(BACKGROUND_COLOR)};
                border-radius: 5px;
                font-size: 16px;
                display: flex;
                align-items: center;
                justify-content: center;
                height: 30px;
                padding: 1px 8px;
                color: ${unsafeCSS(BACKGROUND_COLOR)};
                margin: 5px 10px;
                width: 100px;
            }
            .main:hover {
                background: ${unsafeCSS(HOVER_BACKGROUND_COLOR)};
            }
            .enabled {
                cursor: pointer;
            }
            .disabled:hover {
                background: ${unsafeCSS(LIGHTER_INACTIVE_COLOR)};
            }
            .disabled {
                color: ${unsafeCSS(INACTIVE_COLOR)};
                border: solid 2px ${unsafeCSS(INACTIVE_COLOR)};
                background: ${unsafeCSS(LIGHTER_INACTIVE_COLOR)};
            }
        `;
    }
    static get properties() {
        return {
            label: {type: String, attribute: 'label'},
            enabled: {type: Boolean, attribute: 'enabled'},
        }
    }

    constructor() {
        super();
        this.label = '';
        this.enabled = false;
    }

    _onClick(ev) {
        this.dispatchEvent(new CustomEvent('button-clicked', {bubbles: true, composed: true}));
    }

    render() {
        let divClass = 'main';
        if (this.enabled) {
            divClass += ' enabled';
        } else {
            divClass += ' disabled';
        }
        return html`
            <div class="${divClass}" @click="${this._onClick}">
                ${this.label}
            </div>
        `;
    }
}
customElements.define("custom-button", Button);

class CodeViewer extends LitElement {
    static get styles() {
        return css`
            :host {
                background: white;
                width:100%;
                padding-top: 10px;
                display: flex;
                flex-direction: column;
            }
            #editor {
                width: 100%;
                height: 100%;
            }
            #bottom-row {
                width: 100%;
                display: flex;
                height: 60px;
                flex-direction: column;
                align-items: flex-end;
                justify-content: center;
                border-top: solid 1px ${unsafeCSS(BACKGROUND_COLOR)};
            }
            .hide {
                display: none;
            }
            .reminder {
                height: 100%;
                width: 100%;
                display: flex;
                flex-direction: row;
                align-items: center;
                justify-content: center;
                font-size: 20px;
                color: ${unsafeCSS(BACKGROUND_COLOR)};
            }
            .stateText {
                color: ${unsafeCSS(INACTIVE_COLOR)};
                padding: 0 10px;
            }
        `;
    }

    static get properties() {
        return {
            connected: {type: Boolean, attribute: 'connected'},
            code: {type: String, state: true},
            active: {type: Boolean, attribute: 'active'},
            modified: {type: Boolean, attribute: 'modified'},
        }
    }

    get _editorDiv() {
        return this.renderRoot.querySelector('#editor');
    }

    firstUpdated() {
        const innerStyle = document.createElement('style');
        innerStyle.innerText = `@import "${kMonacoBaseUrl}/vs/editor/editor.main.css";`;
        this.renderRoot.appendChild(innerStyle);

        require(["vs/editor/editor.main"],  () => {
            this.editor = monaco.editor.create(this._editorDiv, {
                language: "cpp",
                scrollBeyondLastLine: false,
                readOnly: false,
                minimap: { enabled: false },
                automaticLayout: true
            });
            const KeyMod = monaco.KeyMod, KeyCode = monaco.KeyCode;
            this.editor.onDidChangeModelContent(this._onEdit.bind(this));
            this.editor.addCommand(KeyMod.CtrlCmd | KeyCode.KEY_S, this._rebuild.bind(this));
        });
    }

    _onEdit(edit) {
        // If the edit is the loading of the entire code, we ignore the edit.
        if (edit.changes[0].text.length == this.code.length) {
            return;
        }
        this.dispatchEvent(new CustomEvent(
            'shader-edited',
            {detail: this.editor.getValue(), bubbles: true, composed: true}
        ));
    }

    _rebuild() {
        this.dispatchEvent(new CustomEvent(
            'rebuild-shader',
            {detail: this.editor.getValue(), bubbles: true, composed: true}
        ));
    }

    updated(props) {
        if (props.has('code') && this.code.length > 0) {
            this.editor.setValue(this.code);
        }
    }

    constructor() {
        super();
        this.code = '';
        this.active = false;
        this.modified = false;
        this.addEventListener('button-clicked', this._rebuild.bind(this));
    }

    render() {
        let divClass = '';
        let reminder = null;
        if (this.code.length == 0) {
            divClass += ' hide';
            reminder = (() => html`<div class="reminder">Please select a shader in the side panel.</div>`)();
        }
        let stateText = null;
        if (!this.connected) {
            stateText = 'disconnected';
        } else if (this.code.length > 0 && !this.active) {
            stateText = 'inactive variant/shader';
        } else if (this.code.length > 0 &&!this.modified) {
            stateText = 'source unmodified';
        }

        const stateDiv = stateText ? (() => html`
            <div class="stateText">${stateText}</div>
        `)(): null;

        return html`
            <div class="${divClass}" id="editor"></div>
            ${reminder ?? nothing}
            <div id="bottom-row">
                <div style="display:flex;flex-direction:row;align-items:center">
                    ${stateDiv ?? nothing}
                    <custom-button class="${divClass}" label="Rebuild" ?enabled="${this.active && this.modified}"></custom-button>
                </div>
            </div>
        `;
    }
}
customElements.define("code-viewer", CodeViewer);

class MaterialSidePanel extends LitElement {
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
            .material-section {
                font-size: 16px;
                color: ${UNSELECTED_COLOR};
                margin-top: 5px;
            }
            .materials {
                display: flex;
                flex-direction: column;
                margin-bottom: 10px;
                font-size: 12px;
                color: ${UNSELECTED_COLOR};
            }
            .material_variant_language:hover {
                text-decoration: underline;
            }
            .material_variant_language {
                cursor: pointer;
            }
            .selected {
                font-weight: bolder;
                color: ${FOREGROUND_COLOR};
            }
            .inactive {
                color: ${INACTIVE_COLOR};
            }
            .variant-list {
                padding-left: 20px;
            }
            .language {
                margin: 0 4px;
            }
            .languages {
                padding-left: 20px;
                flex-direction: row;
                display: flex;
            }
            hr {
                display: block;
                height: 1px;
                border: 0px;
                border-top: 1px solid ${UNSELECTED_COLOR};
                padding: 0;
                width: 100%;
                margin: 3px 0 8px 0;
            }
        `;
    }

    static get properties() {
        return {
            connected: {type: Boolean, attribute: 'connected'},
            currentMaterial: {type: String, attribute: 'current-material'},
            currentShaderIndex: {type: Number, attribute: 'current-shader-index'},
            currentBackend: {type: String, attribute: 'current-backend'},
            currentLanguage: {type: String, attribute: 'current-language'},

            database: {type: Object, state: true},
            materials: {type: Array, state: true},
            activeShaders: {type: Object, state: true},

            variants: {type: Array, state: true},
        }
    }

    constructor() {
        super();
        this.connected = false;
        this.materials = [];
        this.database = {};
        this.activeShaders = {};
        this.variants = [];
    }

    updated(props) {
        if (props.has('database')) {
            const items = [];

            // Names need not be unique, so we display a numeric suffix for non-unique names.
            // To achieve stable ordering of anonymous materials, we first sort by matid.
            const labels = new Set();
            const matids = Object.keys(this.database).sort();
            const duplicatedLabels = {};
            for (const matid of matids) {
                const name = this.database[matid].name || kUntitledPlaceholder;
                if (labels.has(name)) {
                    duplicatedLabels[name] = 0;
                } else {
                    labels.add(name);
                }
            }

            this.materials = matids.map((matid) => {
                const material = this.database[matid];
                let name = material.name || kUntitledPlaceholder;
                if (name in duplicatedLabels) {
                    const index = duplicatedLabels[name];
                    name = `${name} (${index})`;
                    duplicatedLabels[name] = index + 1;
                }
                return {
                    matid: matid,
                    name: name,
                    domain: material.shading.material_domain === "surface" ? "surface" : "postpro",
                    active: material.active,
                };
            });
        }
        if (props.has('currentMaterial')) {
            if (this.currentBackend && this.database && this.activeShaders && this.currentMaterial) {
                const material = this.database[this.currentMaterial];
                const activeVariants = this.activeShaders[this.currentMaterial].variants;
                const materialShaders = material[this.currentBackend];
                let variants = [];
                for (const [index, shader] of materialShaders.entries()) {
                    const active = activeVariants.indexOf(shader.variant) >= 0;
                    variants.push({
                        active,
                        shader,
                    });
                }
                this.variants = variants;
            }
        }
    }

    _handleMaterialClick(matid, ev) {
        this.dispatchEvent(new CustomEvent('select-material', {detail: matid, bubbles: true, composed: true}));
    }

    _handleVariantClick(shaderIndex, ev) {
        this.dispatchEvent(new CustomEvent('select-variant', {detail: shaderIndex, bubbles: true, composed: true}));
    }

    _handleLanguageClick(lang, ev) {
        this.dispatchEvent(new CustomEvent('select-language', {detail: lang, bubbles: true, composed: true}));
    }

    _buildLanguagesDiv(isActive) {
        const languages = LANGUAGE_CHOICES[this.currentBackend];
        if (!languages || languages.length == 1) {
            return null;
        }
        const languagesDiv = languages.map((lang) => {
            const isLanguageSelected = lang === this.currentLanguage;
            let divClass =
                'material_variant_language language' +
                   (isLanguageSelected  ? ' selected' : '') +
                   (!isActive ? ' inactive' : '');
            const onClickLanguage = this._handleLanguageClick.bind(this, lang);
            lang = (isLanguageSelected ? '● ' : '') + lang;
            return html`
                <div class="${divClass}" @click="${onClickLanguage}">
                    ${lang}
                </div>
            `;
        });
        return html`<div class="languages">${languagesDiv}</div>`;
    }

    _buildShaderDiv(showAllShaders) {
        if (!this.variants) {
            return null;
        }
        let variants =
            this.variants
                .sort((a, b) => {
                    // Place the active variants up top.
                    if (a.active && !b.active) return -1;
                    if (b.active && !a.active) return 1;
                    return 0;
                })
                .map((variant) => {
                    let divClass = 'material_variant_language';
                    const shaderIndex = +variant.shader.index;
                    const isVariantSelected = this.currentShaderIndex === shaderIndex;
                    const isActive = variant.shader.active;
                    if (isVariantSelected) {
                        divClass += ' selected';
                    }
                    if (!isActive) {
                        divClass += ' inactive';
                    }
                    const onClickVariant = this._handleVariantClick.bind(this, shaderIndex);
                    // Handle the case where variantString is empty (default variant?)
                    let vstring = (variant.shader.variantString || '').trim();
                    if (vstring.length > 0) {
                        vstring = `[${vstring}]`;
                    }
                    let languagesDiv = isVariantSelected ? this._buildLanguagesDiv(isActive) : null;
                    const stage = (isVariantSelected ? '● ' : '') + variant.shader.pipelineStage;
                    return html`
                        <div class="${divClass}" @click="${onClickVariant}">
                            <div>${stage} ${vstring} </div>
                        </div>
                        ${languagesDiv ?? nothing}
                    `
                });
        return html`<div class="variant-list">${variants}</div>`;
    }

    render() {
        const sections = (title, domain) => {
            const mats = this.materials.filter((m) => m.domain == domain).map((mat) => {
                const material = this.database[mat.matid];
                const onClick = this._handleMaterialClick.bind(this, mat.matid);
                let divClass = 'material_variant_language';
                let shaderDiv = null;
                const isMaterialSelected = mat.matid === this.currentMaterial;
                if (isMaterialSelected) {
                    divClass += ' selected';
                    // If we are looking at an inactive material, show all shaders regardless.
                    const showAllShaders = !material.active;
                    shaderDiv = this._buildShaderDiv(showAllShaders);
                }
                if (!material.active) {
                    divClass += " inactive";
                }
                const matName = (isMaterialSelected ? '● ' : '') + mat.name;
                return html`
                    <div class="${divClass}" @click="${onClick}" data-id="${mat.matid}">
                        ${matName}
                    </div>
                    ${shaderDiv ?? nothing}
                `;
            });
            return html`
                <div class="materials">
                    <div class="material-section">
                        ${title}
                    </div>
                    <hr />
                    ${mats}
                </div>
            `;
        };
        return html`
            <style>${this.dynamicStyle()}</style>
            <div class="container">
                <div class="title">matdbg</div>
                ${sections("Surface", "surface")}
                ${sections("Post-processing", "postpro")}
            </div>
        `;
    }

}
customElements.define("material-sidepanel", MaterialSidePanel);

class MatdbgViewer extends LitElement {
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

    get _codeviewer() {
        return this.renderRoot.querySelector('#code-viewer');
    }

    async init() {
        const isConnected = () => this.connected;
        statusLoop(
            isConnected,
            async (status, data) => {
                this.connected = status == STATUS_CONNECTED || status == STATUS_MATERIAL_UPDATED;

                if (status == STATUS_MATERIAL_UPDATED) {
                    let matInfo = await fetchMaterial(matid);
                    this.database[matInfo.matid] = matInfo;
                    this.database = this.database;
                }
            }
        );

        activeShadersLoop(
            isConnected,
            (activeShaders) => {
                this.activeShaders = activeShaders;
            }
        );

        let materials = await fetchMaterials();
        this.database = materials;
    }

    _getShader() {
        if (!this.currentLanguage || this.currentShaderIndex < 0 || !this.currentBackend) {
            return null;
        }
        const material = (this.database && this.currentMaterial) ? this.database[this.currentMaterial] : null;
        if (!material) {
            return null;
        }
        const shaders = material[this.currentBackend];
        return shaders[this.currentShaderIndex];
    }

    constructor() {
        super();
        this.connected = false;
        this.activeShaders = {};
        this.database = {};
        this.currentShaderIndex = -1;
        this.currentMaterial = null;
        this.currentLanguage = null;
        this.currentBackend = null;
        this.init();

        this.addEventListener('select-material',
            (ev) => {
                this.currentMaterial = ev.detail;
            }
        );
        this.addEventListener('select-variant',
            (ev) => {
                this.currentShaderIndex = ev.detail;
            }
        );
        this.addEventListener('select-language',
            (ev) => {
                this.currentLanguage = ev.detail;
            }
        );

        this.addEventListener('rebuild-shader',
            (ev) => {
                const shader = this._getShader();
                if (!shader) {
                    return
                }
                rebuildMaterial(
                    this.currentMaterial, this.currentBackend, this.currentShaderIndex, ev.detail);

                shader.modified = false;
                // Trigger an update
                this.database = this.database;
            }
        );

        this.addEventListener('shader-edited',
            (ev) => {
                const shader = this._getShader();
                if (shader) {
                    shader.modified = true;
                    // Trigger an update
                    this.database = this.database;
                }
            }
        );
    }

    static get properties() {
        return {
            connected: {type: Boolean, state: true},
            database: {type: Object, state: true},
            activeShaders: {type: Object, state: true},
            currentLanguage: {type: String, state: true},
            currentMaterial: {type: String, state: true},
            // Each material has a list of variants compiled for it, this index tracks a position in the list.
            currentShaderIndex: {type: Number, state: true},
            currentBackend: {type: String, state: true},
        }
    }

    updated(props) {
        // Set a language if there hasn't been one set.
        if (props.has('currentBackend') && this.currentBackend) {
            const choices = LANGUAGE_CHOICES[this.currentBackend];
            if (choices.indexOf(this.currentLanguage) < 0) {
                this.currentLanguage = choices[0];
            }
        }
        if (props.has('currentMaterial')) {
            // Try to find a default shader index
            if ((this.currentMaterial in this.activeShaders) && this.currentBackend) {
                const material = this.database[this.currentMaterial];
                const activeVariants = this.activeShaders[this.currentMaterial].variants;
                const materialShaders = material[this.currentBackend];
                for (let shader in materialShaders) {
                    let ind = activeVariants.indexOf(+shader);
                    if (ind >= 0) {
                        this.currentShaderIndex = +shader;
                        break;
                    }
                }
            } else if (this.currentMaterial) {
                const material = this.database[this.currentMaterial];
                // Just pick the first variant in this materials list.
                this.currentShaderIndex = 0;
            }
        }
        if ((props.has('currentMaterial') || props.has('currentBackend') ||
             props.has('currentShaderIndex') || props.has('currentLanguage')) &&
            (this.currentMaterial && this.currentBackend && this.currentShaderIndex >= 0&&
             this.currentLanguage)) {
            (async () => {
                this._codeviewer.code = await fetchShaderCode(
                    this.currentMaterial, this.currentBackend, this.currentLanguage,
                    this.currentShaderIndex);
                const shader = this._getShader();
                if (shader) {
                    shader.modified = false;
                    this.database = this.database;
                }
            })();
        }
        if (props.has('activeShaders') || props.has('database')) {
            // The only active materials are the ones with active variants.
            Object.values(this.database).forEach((material) => {
                material.active = false;
            });
            for (matid in this.activeShaders) {
                if (!this.database[matid]) {
                    continue;
                }
                let material = this.database[matid];
                const backend = this.activeShaders[matid].backend;
                const variants = this.activeShaders[matid].variants;
                for (let shader of material[backend]) {
                    shader.active = variants.indexOf(shader.variant) > -1;
                    material.active = material.active || shader.active;
                }
            }
            if (this.activeShaders) {
                let backends = {};
                for (let matid in this.activeShaders) {
                    const backend = this.activeShaders[matid].backend;
                    if (backend in backends) {
                        backends[backend] = backends[backend] + 1;
                    } else {
                        backends[backend] = 1;
                    }
                }
                let backendList = Object.keys(backends);
                if (backendList.length > 0) {
                    this.currentBackend = backendList[0];
                }
            }

            this._sidepanel.database = this.database;
            this._sidepanel.activeShaders = this.activeShaders;
        }
        if (props.has('connected') && this.connected) {
            (async () => {
                for (const matId of await fetchMatIds()) {
                    const matInfo = await fetchMaterial(matid);
                    this.database[matInfo.matid] = matInfo;
                    this.database = this.database;
                }
            })();
        }
    }

    render() {
        const shader = this._getShader();
        return html`
            <material-sidepanel id="sidepanel"
                     ?connected="${this.connected}"
                     current-language="${this.currentLanguage}"
                     current-shader-index="${this.currentShaderIndex}"
                     current-material="${this.currentMaterial}"
                     current-backend="${this.currentBackend}" >
            </material-sidepanel>
            <code-viewer id="code-viewer"
                 ?active=${shader && shader.active}
                 ?modified=${shader && shader.modified}
                 ?connected="${this.connected}">
            </code-viewer>
        `;
    }
}
customElements.define("matdbg-viewer", MatdbgViewer);
