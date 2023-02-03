/*
 * Copyright (C) 2019 The Android Open Source Project
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
const kMonacoBaseUrl = 'https://cdnjs.cloudflare.com/ajax/libs/monaco-editor/0.25.2/min/';
const kUntitledPlaceholder = "untitled";

const materialList = document.getElementById("material-list");
const materialDetail = document.getElementById("material-detail");
const header = document.querySelector("header");
const footer = document.querySelector("footer");
const shaderSource = document.getElementById("shader-source");
const matDetailTemplate = document.getElementById("material-detail-template");
const matListTemplate = document.getElementById("material-list-template");

const gMaterialDatabase = {};

let gSocket = null;
let gEditor = null;
let gCurrentMaterial = "00000000";
let gCurrentLanguage = "glsl";
let gCurrentShader = { matid: "00000000", glindex: 0 };
let gCurrentSocketId = 0;
let gEditorIsLoading = false;

require.config({ paths: { "vs": `${kMonacoBaseUrl}vs` }});

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

function getShaderAPI(selection) {
    if (!selection) {
        selection = gCurrentShader;
    }
    if ("glindex" in selection) return "opengl";
    if ("vkindex" in selection) return "vulkan";
    if ("metalindex" in selection) return "metal";
    return "error";
}

function rebuildMaterial() {
    let api = 0, index = -1;

    const shader = getShaderRecord(gCurrentShader);
    const shaderApi = getShaderAPI();

    switch (shaderApi) {
        case "opengl": api = 1; index = gCurrentShader.glindex; break;
        case "vulkan": api = 2; index = gCurrentShader.vkindex; break;
        case "metal":  api = 3; index = gCurrentShader.metalindex; break;
    }

    if (shaderApi === "vulkan") {
        if (gCurrentLanguage === "glsl") {
            delete shader["spirv"];
        } else if (gCurrentLanguage === "spirv") {
            delete shader["glsl"];
        }
    }

    const editedText = shader[gCurrentLanguage];
    const byteCount = new Blob([editedText]).size;
    gSocket.send(`EDIT ${gCurrentShader.matid} ${api} ${index} ${byteCount} ${editedText}`);
}

document.querySelector("body").addEventListener("click", (evt) => {
    const anchor = evt.target.closest("a");
    if (!anchor) {
        return;
    }

    // Handle selection of a material.
    if (anchor.classList.contains("material")) {
        selectMaterial(anchor.dataset.matid, true);
        return;
    }

    // Handle selection of a shader.
    if (anchor.classList.contains("shader")) {
        selectShader(anchor.dataset);
        return;
    }

    // Handle a rebuild.
    if (anchor.classList.contains("rebuild")) {
        rebuildMaterial();
        return;
    }

    // Handle language selection.
    for (const lang of "glsl spirv msl".split(" ")) {
        if (anchor.classList.contains(lang)) {
            gCurrentLanguage = lang;
            selectShader(gCurrentShader);
            return;
        }
    }
});

// Handle Ctrl+Arrow for fast keyboard navigation between shader variants and materials. Either the
// materialStep or shaderStep argument can be non-zero (not both) and they must be -1, 0, or +1.
// TODO: this function could be vastly simplified by changing the format of the shader selector.
function selectNextShader(materialStep, shaderStep) {
    if (materialStep !== 0) {
        const matids = getDisplayedMaterials().map(m => m.matid).filter(m => m);
        const currentIndex = matids.indexOf(gCurrentMaterial);
        const nextIndex = currentIndex + materialStep;
        if (nextIndex >= 0 && nextIndex < matids.length) {
            selectMaterial(matids[nextIndex], true);
        }
        return;
    }
    const material = gMaterialDatabase[gCurrentMaterial];
    const variants = [];
    let currentIndex = 0;
    for (const [index, shader] of material.opengl.entries()) {
        if (index === gCurrentShader.glindex) currentIndex = variants.length;
        variants.push({ matid, glindex: index });
    }
    for (const [index, shader] of material.vulkan.entries()) {
        if (index === gCurrentShader.vkindex) currentIndex = variants.length;
        variants.push({ matid, vkindex: index });
    }
    for (const [index, shader] of material.metal.entries()) {
        if (index === gCurrentShader.metalindex) currentIndex = variants.length;
        variants.push({ matid, metalindex: index });
    }
    const nextIndex = currentIndex + shaderStep;
    if (nextIndex >= 0 && nextIndex < variants.length) {
        selectShader(variants[nextIndex]);
    }
}

function fetchMaterial(matid) {
    fetch(`api/material?matid=${matid}`).then(function(response) {
        return response.json();
    }).then(function(matInfo) {
        if (matid in gMaterialDatabase) {
            return;
        }
        matInfo.matid = matid;
        gMaterialDatabase[matid] = matInfo;
        renderMaterialList();
    });
}

function queryActiveShaders() {
    if (!gSocket) {
        for (matid in gMaterialDatabase) {
            const material = gMaterialDatabase[matid];
            material.active = false;
            for (const shader of material.opengl) shader.active = false;
            for (const shader of material.vulkan) shader.active = false;
            for (const shader of material.metal)  shader.active = false;
        }
        renderMaterialList();
        renderMaterialDetail();
        return;
    }
    fetch("api/active").then(function(response) {
        return response.json();
    }).then(function(activeMaterials) {
        // The only active materials are the ones with active variants.
        for (matid in gMaterialDatabase) {
            const material = gMaterialDatabase[matid];
            material.active = false;
        }
        for (matid in activeMaterials) {
            const material = gMaterialDatabase[matid];
            const activeBackend = activeMaterials[matid][0];
            const activeShaders = activeMaterials[matid].slice(1);
            for (const shader of material[activeBackend]) {
                shader.active = activeShaders.indexOf(shader.variant) > -1;
                material.active = material.active || shader.active;
            }
        }
        renderMaterialList();
        renderMaterialDetail();
    })
    .catch(error => {
        // This can occur if the JSON is invalid.
        console.error(error);
    });
}

function startSocket() {
    const url = new URL(document.URL)
    const ws = new WebSocket(`ws://${url.host}`);

    // When a new server has come online, ask it what materials it has.
    ws.addEventListener("open", () => {
        footer.innerText = `connection ${gCurrentSocketId}`;
        gCurrentSocketId++;

        fetch("api/matids").then(function(response) {
            return response.json();
        }).then(function(matInfo) {
            for (matid of matInfo) {
                if (!(matid in gMaterialDatabase)) {
                    fetchMaterial(matid);
                }
            }
        });
    });

    ws.addEventListener("close", (e) => {
        footer.innerText = "no connection";
        gSocket = null;
        setTimeout(() => startSocket(), 3000);
    });

    ws.addEventListener("message", event => {
        const matid = event.data;
        fetchMaterial(matid);
    });

    gSocket = ws;
}

function fetchMaterials() {
    fetch("api/materials").then(function(response) {
        return response.json();
    }).then(function(matJson) {
        for (const matInfo of matJson) {
            if (matInfo.matid in gMaterialDatabase) {
                continue;
            }
            gMaterialDatabase[matInfo.matid] = matInfo;
        }
        selectMaterial(matJson[0].matid, true);
    });
}

function fetchShader(selection, matinfo, onDone) {
    let query, target, index;
    switch (getShaderAPI(selection)) {
        case "opengl":
            index = parseInt(selection.glindex);
            query = `type=${gCurrentLanguage}&glindex=${index}`;
            target = matinfo.opengl[index];
            break;
        case "vulkan":
            index = parseInt(selection.vkindex);
            query = `type=${gCurrentLanguage}&vkindex=${index}`;
            target = matinfo.vulkan[index];
            break;
        case "metal":
            index = parseInt(selection.metalindex);
            query = `type=${gCurrentLanguage}&metalindex=${index}`;
            target = matinfo.metal[index];
            break;
    }
    fetch(`api/shader?matid=${matinfo.matid}&${query}`).then(function(response) {
        return response.text();
    }).then(function(shaderText) {
        target[gCurrentLanguage] = shaderText;
        onDone();
    });
}

function getDisplayedMaterials() {
    const items = [];

    // Names need not be unique, so we display a numeric suffix for non-unique names.
    // To achieve stable ordering of anonymous materials, we first sort by matid.
    const labels = new Set();
    const matids = Object.keys(gMaterialDatabase).sort();
    const duplicatedLabels = {};
    for (const matid of matids) {
        const name = gMaterialDatabase[matid].name || kUntitledPlaceholder;
        if (labels.has(name)) {
            duplicatedLabels[name] = 0;
        } else {
            labels.add(name);
        }
    }

    // Build a list of objects to pass into the template string.
    for (const matid of matids) {
        const item =  Object.assign({}, gMaterialDatabase[matid]);
        item.classes = matid === gCurrentMaterial ? "current " : "";
        if (!item.active) {
            item.classes += "inactive "
        }
        item.domain = item.shading.material_domain === "surface" ? "surface" : "postpro";
        item.is_material = true;

        const name = item.name || kUntitledPlaceholder;
        if (name in duplicatedLabels) {
            const index = duplicatedLabels[name];
            item.name = `${name} (${index})`;
            duplicatedLabels[name] = index + 1;
        } else {
            item.name = name;
        }

        items.push(item);
    }

    // The template takes a flat list of items, so here we insert items for section headers using
    // blank names, which causes them to sort to the top of their respective sections.
    const sectionLabel = {"is_label": true, "name": ""};
    items.push(Object.assign({"label": "Surface materials", "domain": "surface"}, sectionLabel));
    items.push(Object.assign({"label": "PostProcess materials", "domain": "postpro"}, sectionLabel));

    // Next, sort all materials and section headers.
    items.sort((a, b) => {
        if (a.domain > b.domain) return -1;
        if (a.domain < b.domain) return +1;
        if (a.name < b.name) return -1;
        if (a.name > b.name) return +1;
        return 0;
    });
    return items;
}

function renderMaterialList() {
    const items = getDisplayedMaterials();
    materialList.innerHTML = Mustache.render(matListTemplate.innerHTML, { "item": items } );
}

function updateClassList(array, indexProperty, selectedIndex) {
    for (let item of array) {
        const current = parseInt(item[indexProperty]) === selectedIndex;
        item.classes = current ? "current " : "";
        if (!item.active) {
            item.classes += "inactive "
        }
    }
}

function renderMaterialDetail() {
    const mat = gMaterialDatabase[gCurrentMaterial];
    const ok = mat.matid === gCurrentShader.matid;
    updateClassList(mat.opengl, "index", ok ? parseInt(gCurrentShader.glindex) : -1);
    updateClassList(mat.vulkan, "index", ok ? parseInt(gCurrentShader.vkindex) : -1);
    updateClassList(mat.metal, "index", ok ? parseInt(gCurrentShader.metalindex) : -1);
    const item =  Object.assign({}, mat);
    if (item.shading.material_domain !== "surface") {
        delete item.shading;
    }
    materialDetail.innerHTML = Mustache.render(matDetailTemplate.innerHTML, item);
}

function getShaderRecord(selection) {
    const mat = gMaterialDatabase[gCurrentMaterial];
    if (selection.glindex >= 0) return mat.opengl[parseInt(selection.glindex)];
    if (selection.vkindex >= 0) return mat.vulkan[parseInt(selection.vkindex)];
    if (selection.metalindex >= 0) return mat.metal[parseInt(selection.metalindex)];
    return null;
}

function renderShaderStatus() {
    const shader = getShaderRecord(gCurrentShader);
    let statusString = "";
    if (shader) {
        const glsl = "glsl " + (gCurrentLanguage === "glsl" ? "active" : "");
        const msl = "msl " + (gCurrentLanguage === "msl" ? "active" : "");
        const spirv = "spirv " + (gCurrentLanguage === "spirv" ? "active" : "");
        switch (getShaderAPI()) {
            case "opengl":
                statusString += ` &nbsp; <a class='status_button ${glsl}'>[GLSL]</a>`;
                break;
            case "metal":
                statusString += ` &nbsp; <a class='status_button ${msl}'>[MSL]</a>`;
                break;
            case "vulkan":
                statusString += ` &nbsp; <a class='status_button ${glsl}'>[GLSL]</a>`;
                statusString += ` &nbsp; <a class='status_button ${spirv}'>[SPIRV]</a>`;
                break;
        }
        if (shader.modified && gCurrentLanguage !== "spirv") {
            statusString += " &nbsp; <a class='status_button rebuild'>[rebuild]</a>";
        }
        if (!shader.active) {
            statusString += " &nbsp; <span class='warning'> selected variant is inactive </span>";
        }
    }
    header.innerHTML = "matdbg" + statusString;
}

function selectShader(selection) {
    const shader = getShaderRecord(selection);
    if (!shader) {
        console.error("Shader not yet available.")
        return;
    }

    // Change the current language selection if necessary.
    switch (getShaderAPI(selection)) {
        case "opengl":
            if (gCurrentLanguage !== "glsl") {
                gCurrentLanguage = "glsl";
            }
            break;
        case "vulkan":
            if (gCurrentLanguage !== "spirv" && gCurrentLanguage !== "glsl") {
                gCurrentLanguage = "spirv";
            }
            break;
        case "metal":
            if (gCurrentLanguage !== "msl") {
                gCurrentLanguage = "msl";
            }
            break;
    }

    const showShaderSource = () => {
        gCurrentShader = selection;
        gCurrentShader.matid = gCurrentMaterial;
        renderMaterialDetail();
        gEditorIsLoading = true;
        gEditor.setValue(shader[gCurrentLanguage]);
        gEditorIsLoading = false;
        shaderSource.style.visibility = "visible";
        renderShaderStatus();
    };
    if (!shader[gCurrentLanguage]) {
        const matInfo = gMaterialDatabase[gCurrentMaterial];
        fetchShader(selection, matInfo, showShaderSource);
    } else {
        showShaderSource();
    }
}

function onEdit(changes) {
    if (gEditorIsLoading) {
        return;
    }
    const shader = getShaderRecord(gCurrentShader);
    if (!shader) {
        return;
    }
    if (!shader.modified) {
        shader.modified = true;
        renderShaderStatus();
    }
    shader[gCurrentLanguage] = gEditor.getValue();
}

function selectMaterial(matid, selectFirstShader) {
    gCurrentMaterial = matid;
    renderMaterialList();
    renderMaterialDetail();
    if (selectFirstShader) {
        const mat = gMaterialDatabase[gCurrentMaterial];
        const selection = { matid };
        if (mat.opengl.length > 0) selection.glindex = 0;
        else if (mat.vulkan.length > 0) selection.vkindex = 0;
        else if (mat.metal.length > 0) selection.metalindex = 0;
        selectShader(selection);
    }
}

function init() {
    require(["vs/editor/editor.main"], function () {
        const KeyMod = monaco.KeyMod, KeyCode = monaco.KeyCode;
        gEditor = monaco.editor.create(shaderSource, {
            value: "",
            language: "cpp",
            scrollBeyondLastLine: false,
            readOnly: false,
            minimap: { enabled: false }
        });
        gEditor.onDidChangeModelContent((e) => { onEdit(e.changes); });

        gEditor.addCommand(KeyMod.CtrlCmd | KeyCode.KEY_S, () => rebuildMaterial());

        gEditor.addCommand(KeyMod.Shift | KeyMod.WinCtrl | KeyCode.UpArrow, () => selectNextShader(-1, 0));
        gEditor.addCommand(KeyMod.Shift | KeyMod.WinCtrl | KeyCode.DownArrow, () => selectNextShader(+1, 0));
        gEditor.addCommand(KeyMod.Shift | KeyMod.WinCtrl | KeyCode.LeftArrow, () => selectNextShader(0, -1));
        gEditor.addCommand(KeyMod.Shift | KeyMod.WinCtrl | KeyCode.RightArrow, () => selectNextShader(0, +1));

        fetchMaterials();
    });

    Mustache.parse(matDetailTemplate.innerHTML);
    Mustache.parse(matListTemplate.innerHTML);

    startSocket();

    // Poll for active shaders once every second.
    // Take care not to poll more frequently than the frame rate. Active variants are determined
    // by the list of variants that were fetched between this query and the previous query.
    setInterval(queryActiveShaders, 1000);
}

init();
