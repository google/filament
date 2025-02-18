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

// api.js encapsulates all the REST endpoints that the server provides

async function _fetchJson(uri) {
    const response = await fetch(uri);
    return await response.json();
}

async function _fetchText(uri) {
    const response = await fetch(uri);
    return await response.text();
}

async function fetchFrameGraphs() {
    const fgJson = await _fetchJson("api/framegraphs")
    const ret = {};
    for (const fgInfo of fgJson) {
        ret[fgInfo.fgid] = fgInfo;
    }
    return ret;
}

async function fetchFrameGraph(fgid) {
    const fgInfo = await _fetchJson(`api/framegraph?fgid=${fgid}`);
    fgInfo.fgid = fgid;
    return fgInfo;
}

const STATUS_LOOP_TIMEOUT = 3000;

const STATUS_CONNECTED = 1;
const STATUS_DISCONNECTED = 2;
const STATUS_FRAMEGRAPH_UPDATED = 3;

// Status function should be of the form function(status, data)
async function statusLoop(isConnected, onStatus) {
    // This is a hanging get except for when transition from disconnected to connected, which
    // should return immediately.
    try {
        const fgid = await _fetchText("api/status" + (isConnected() ? '' : '?firstTime'));
        // A first-time request returned successfully
        if (fgid === '0') {
            onStatus(STATUS_CONNECTED);
        } else if (fgid !== '1') {
            onStatus(STATUS_FRAMEGRAPH_UPDATED, fgid);
        } // fgid == '1' is no-op, just loop again
        statusLoop(isConnected, onStatus);
    } catch {
        onStatus(STATUS_DISCONNECTED);
        setTimeout(() => statusLoop(isConnected, onStatus), STATUS_LOOP_TIMEOUT)
    }
}
