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

const BACKENDS = Object.keys(LANGUAGE_CHOICES);

const MATERIAL_INFO_KEY_TO_STRING = {
    'model': 'shading model',
    'vertex_domain': 'vertex domain',
    'interpolation': 'interpolation',
    'shadow_multiply': 'shadow multiply',
    'specular_antialiasing': 'specular antialiasing',
    'variance':  'variance',
    'threshold': 'threshold',
    'clear_coat_IOR_change': 'clear coat IOR change',
    'blending': 'blending',
    'mask_threshold': 'mask threshold',
    'color_write': 'color write',
    'depth_write': 'depth write',
    'depth_test': 'depth test',
    'double_sided': 'double sided',
    'culling': 'culling',
    'transparency': 'transparency',
};

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

const _validDict = (obj) => {
    return obj && Object.keys(obj).length > 0;
}

const _isMatInfoMode = (database) => {
    return Object.keys(database).length == 1;
}

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
                height: ${unsafeCSS(CODE_VIEWER_BOTTOM_ROW_HEIGHT)}px;
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
            expectedWidth: {type: Number, attribute: 'expected-width'},
            expectedHeight: {type: Number, attribute: 'expected-height'},
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
                automaticLayout: true,

                // Workaround see https://github.com/microsoft/monaco-editor/issues/3217
                fontLigatures: '',
            });
            const KeyMod = monaco.KeyMod, KeyCode = monaco.KeyCode;
            this.editor.onDidChangeModelContent(this._onEdit.bind(this));
            this.editor.addCommand(KeyMod.CtrlCmd | KeyCode.KEY_S, this._rebuild.bind(this));

            // It might be that the code is available before the editor has been created.
            if (this.code && this.code.length > 0) {
                this.editor.setValue(this.code);
            }
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
        if (!this.active || !this.modified) {
            console.log('Called rebuild while variant is inactive or unmodified');
            return;
        }
        this.dispatchEvent(new CustomEvent(
            'rebuild-shader',
            {detail: this.editor.getValue(), bubbles: true, composed: true}
        ));
    }

    updated(props) {
        if (props.has('code') && this.code.length > 0) {
            // Note that the prop might have been updated before the editor is available.
            if (this.editor) {
                this.editor.setValue(this.code);
            }
        }
        if ((props.has('expectedWidth') || props.has('expectedHeight')) &&
            (this.expectedWidth > 0 && (this.expectedHeight - CODE_VIEWER_BOTTOM_ROW_HEIGHT) > 0)) {
            const actualWidth = Math.floor(this.expectedWidth);
            const actualHeight = (Math.floor(this.expectedHeight) - CODE_VIEWER_BOTTOM_ROW_HEIGHT);
            this._editorDiv.style.width = actualWidth + 'px';
            this._editorDiv.style.height = actualHeight + 'px';
        }
    }

    constructor() {
        super();
        this.code = '';
        this.active = false;
        this.modified = false;
        this.addEventListener('button-clicked', this._rebuild.bind(this));
        this.expectedWidth = 0;
        this.expectedHeight = 0;
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
                    <custom-button class="${divClass}"
                                   label="Rebuild"
                                   ?enabled="${this.active && this.modified}">
                    </custom-button>
                </div>
            </div>
        `;
    }
}
customElements.define("code-viewer", CodeViewer);

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

class MaterialInfo extends LitElement {
    static get properties() {
        return {
            info: {type: Object, state: true},
        };
    }

    static get styles() {
        return css`
            :host {
                font-size: ${unsafeCSS(REGULAR_FONT_SIZE)}px;
            }
        `;
    }

    constructor() {
        super();
        this.info = null;
    }

    _hasInfo() {
        return this.info && Object.keys(this.info).length > 0;
    }

    render() {
        let infoDivs = [];
        if (this._hasInfo()) {
            if (this.info.shading && this.info.shading.material_domain === 'surface') {
                infoDivs = infoDivs.concat(
                    Object.keys(this.info.shading)
                        .filter((propKey) => (propKey in MATERIAL_INFO_KEY_TO_STRING))
                        .map((propKey) => html`
                            <div key="${propKey}" class="info-item">
                                ${MATERIAL_INFO_KEY_TO_STRING[propKey]} = ${this.info.shading[propKey]}
                            </div>
                        `)
                );
            }
            if (this.info.raster) {
                infoDivs = infoDivs.concat(
                    Object.keys(this.info.raster)
                        .filter((propKey) => (propKey in MATERIAL_INFO_KEY_TO_STRING))
                        .map((propKey) => html`
                            <div key=${propKey} class="info-item">
                                ${MATERIAL_INFO_KEY_TO_STRING[propKey]} = ${this.info.raster[propKey]}
                            </div>
                        `)
                );
            }
        }
        const shouldHide = infoDivs.length == 0;
        if (infoDivs.length > 0) {
            return html`
                <menu-section title="Material Details">
                    ${infoDivs}
                </menu-section>
            `;
        }
        return html``;
    }
}
customElements.define('material-info', MaterialInfo);

class AdvancedOptions extends LitElement {
    static get properties() {
        return {
            currentBackend: {type: String, attribute: 'current-backend'},
            availableBackends: {type: Array, state: true},
        };
    }

    static get styles() {
        return css`
            :host {
                font-size: ${unsafeCSS(REGULAR_FONT_SIZE)}px;
            }
            .option {
                border: 1px solid ${unsafeCSS(UNSELECTED_COLOR)};
                border-radius: 5px;
                padding: 4px;
                display: flex;
                flex-direction: column;
                justify-content: center;
                align-items: flex-start;
            }
            label {
                display: flex;
                flex-direction: row;
                align-items: center;
                justify-content: center;
                margin-right: 5px;
            }
            label input {
                margin: 0 4px 0 0;
            }
            form {
                display: flex;
            }
            .option-heading {
                margin-bottom: 5px;
            }
        `;
    }

    get _backendOptionForm() {
        return this.renderRoot.querySelector('#backend-option-form');
    }

    updated(props) {
        if (props.has('currentBackend') || props.has('availableBackends')) {
            // Clear the radio button selections. The correct option will be selected
            // in _backendOption().
            if (this._backendOptionForm) {
                this._backendOptionForm.reset();
            }
        }
    }

    _backendOption() {
        if (this.availableBackends.length == 0) {
            return null;
        }

        const onChange = (ev) => {
            const backend = ev.currentTarget.getAttribute('name');
            this.dispatchEvent(
                new CustomEvent(
                    'option-backend',
                    {detail: backend, bubbles: true, composed: true}));
        }
        const div = this.availableBackends.map((backend) => {
            const selected = backend == this.currentBackend;
            return html`
                <label>
                    <input type="radio" name="${backend}"
                           ?checked=${selected} @change=${onChange}/>
                    ${backend}
                </label>
            `;
        });

        return html`
            <div class="option">
                <div class="option-heading">Current Backend</div>
                <form action="" id="backend-option-form">
                    ${div}
                </form>
            </div>
        `;
    }

    constructor() {
        super();
        this.availableBackends = [];
    }

    render() {
        return html`
            <menu-section title="Advanced Options">
                ${this._backendOption() ?? nothing}
            </menu-section>
        `;
    }
}
customElements.define('advanced-options', AdvancedOptions);


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
            .materials {
                display: flex;
                flex-direction: column;
                margin-bottom: 20px;
                font-size: ${REGULAR_FONT_SIZE}px;
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
                margin: 0 8px 0 0;
            }
            .languages {
                padding-left: 20px;
                flex-direction: row;
                display: flex;
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

    get _materialInfo() {
        return this.renderRoot.querySelector('#material-info');
    }

    get _advancedOptions() {
        return this.renderRoot.querySelector('#advanced-options');
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
                    duplicatedLabels[name] = index + 1;
                    name = `${name} (${index})`;
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
            if (this.currentBackend && this.database && this.currentMaterial) {
                const material = this.database[this.currentMaterial];
                const activeVariants =  _validDict(this.activeShaders) ? this.activeShaders[this.currentMaterial].variants : [];
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

            if (this.currentMaterial && this.database) {
                const material = this.database[this.currentMaterial];
                this._materialInfo.info = material;

                // The matinfo usecase
                if (_isMatInfoMode(this.database)) {
                    this._advancedOptions.availableBackends = BACKENDS.filter((backend) => !!material[backend]);
                }
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
            if (mats.length > 0) {
                return html`<menu-section title="${title}">${mats}</menu-section>`;
            }
            return null;
        };
        let advancedOptions = null;
        // Currently we only have one advanced option and it's only for when we're in matinfo
        if (_isMatInfoMode(this.database)) {
            advancedOptions =
                (() => html`
                    <advanced-options id="advanced-options"
                             current-backend=${this.currentBackend}></advanced-options>
                `)();
        }

        return html`
            <style>${this.dynamicStyle()}</style>
            <div class="container">
                <div class="title">matdbg</div>
                ${sections("Surface", "surface") ?? nothing}
                ${sections("Post-processing", "postpro") ?? nothing}
                <material-info id="material-info"></material-info>
                ${advancedOptions ?? nothing}
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

    _onResize() {
        const rect = this._sidepanel.getBoundingClientRect();
        this.codeViewerExpectedWidth = window.innerWidth - rect.width - 1;
        this.codeViewerExpectedHeight = window.innerHeight;
    }

    firstUpdated() {
        this._onResize();
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

        this.addEventListener('option-backend',
            (ev) => {
                this.currentBackend = ev.detail;
            }
        );

        addEventListener('resize', this._onResize.bind(this));
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
            codeViewerExpectedWidth: {type: Number, state: true},
            codeViewerExpectedHeight: {type: Number, state: true},
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

                // Size of the editor will be adjusted due to the code being loaded, we try to
                // fit the editor again by calling the resize signal.
                setTimeout(this._onResize.bind(this), 700);
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
            if (_validDict(this.activeShaders)) {
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
            } else if (!this.currentBackend) {
                // Make a guess on the backend if one wasn't from activeShaders.
                this.currentBackend = guessBackend();
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

                // In the `matinfo -w` usecase, we assume the current material to be the only
                // material available in the database.
                if (_isMatInfoMode(this.database)) {
                    this.currentMaterial = Object.keys(this.database)[0];
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
                 ?connected="${this.connected}"
                 expected-width="${this.codeViewerExpectedWidth}"
                 expected-height="${this.codeViewerExpectedHeight}" >
            </code-viewer>
        `;
    }
}
customElements.define("matdbg-viewer", MatdbgViewer);
