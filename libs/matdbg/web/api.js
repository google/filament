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

// api.js encapsulates all of the REST endpoints that the server provides

async function _fetchJson(uri) {
    const response = await fetch(uri);
    return await response.json();
}

async function _fetchText(uri) {
    const response = await fetch(uri);
    return await response.text();
}

async function fetchShaderCode(matid, backend, language, index) {
    let query;
    switch (backend) {
        case "opengl":
        case "essl1":
            query = `type=${language}&glindex=${index}`;
            break;
        case "vulkan":
            query = `type=${language}&vkindex=${index}`;
            break;
        case "metal":
            query = `type=${language}&metalindex=${index}`;
            break;
    }
    return await _fetchText(`api/shader?matid=${matid}&${query}`);
}

async function fetchMaterials() {
    const matJson = await _fetchJson("api/materials")
    const ret = {};
    for (const matInfo of matJson) {
        ret[matInfo.matid] = matInfo;
    }
    return ret;
}

async function fetchMaterial(matId) {
    const matInfo = await _fetchJson(`api/material?matid=${matid}`);
    matInfo.matid = matid;
    return matInfo;
}

async function fetchMatIds() {
    const matInfo = await _fetchJson("api/matids");
    const ret = [];
    for (matid of matInfo) {
        ret.push(matid);
    }
    return ret;
}

async function queryActiveShaders() {
    const activeMaterials = await _fetchJson("api/active");
    const actives = {};
    for (matid in activeMaterials) {
        const backend = activeMaterials[matid][0];
        const variants = activeMaterials[matid].slice(1);
        actives[matid] = {
            backend, variants
        };
    }
    return actives;
}

function rebuildMaterial(materialId, backend, shaderIndex, editedText) {
    let api = 0;
    switch (backend) {
        case "opengl":
        case "essl1":
            api = 1;
            break;
        case "vulkan": api = 2; break;
        case "metal":  api = 3; break;
    }
    return new Promise((ok, fail) => {
        const req = new XMLHttpRequest();
        req.open('POST', '/api/edit');
        req.send(`${materialId} ${api} ${shaderIndex} ${editedText}`);
        req.onload = ok;
        req.onerror = fail;
    });
}

function activeShadersLoop(isConnected, onActiveShaders) {
    setInterval(async () => {
        if (isConnected()) {
            onActiveShaders(await queryActiveShaders());
        }
    }, 1000);
}

const STATUS_LOOP_TIMEOUT = 3000;

const STATUS_CONNECTED = 1;
const STATUS_DISCONNECTED = 2;
const STATUS_MATERIAL_UPDATED = 3;

// Status function should be of the form function(status, data)
async function statusLoop(isConnected, onStatus) {
    // This is a hanging get except for when transition from disconnected to connected, which
    // should return immediately.
    try {
        const matid = await _fetchText("api/status" + (isConnected() ? '' : '?firstTime'));
        // A first-time request returned successfully
        if (matid === '0') {
            onStatus(STATUS_CONNECTED);
        } else if (matid !== '1') {
            onStatus(STATUS_MATERIAL_UPDATED, matid);
        } // matid == '1' is no-op, just loop again
        statusLoop(isConnected, onStatus);
    } catch {
        onStatus(STATUS_DISCONNECTED);
        setTimeout(() => statusLoop(isConnected, onStatus), STATUS_LOOP_TIMEOUT)
    }
}

// Use browser User-agent to guess the current backend.  This is mainly for matinfo which does
// not have a running backend.
function guessBackend() {
    const AGENTS_TO_BACKEND = [
        ['Mac OS', 'metal'],
        ['Windows', 'opengl'],
        ['Linux', 'vulkan'],
    ];

    const result = AGENTS_TO_BACKEND.filter((agent_backend) => {
        return window.navigator.userAgent.search(agent_backend[0]);
    }).map((agent_backend) => agent_backend[1]);

    return result.length > 0 ? result[0] : null;
}
