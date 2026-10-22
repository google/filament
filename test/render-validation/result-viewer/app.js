// Copyright (C) 2026 The Android Open Source Project
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

import './components/result-table.js';
import './components/image-viewer.js';

async function init() {
    const container = document.getElementById('container');
    
    try {
        const response = await fetch('./data.json');
        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`);
        }
        const data = await response.json();
        
        container.innerHTML = `
            <result-table></result-table>
            <image-viewer></image-viewer>
        `;
        
        const resultTable = container.querySelector('result-table');
        const imageViewer = container.querySelector('image-viewer');
        
        resultTable.results = data;
        
        // Listen for view events from the table
        container.addEventListener('view-result', (e) => {
            imageViewer.open(e.detail.device, e.detail.run);
        });

    } catch (e) {
        container.innerHTML = `<div style="color: red; padding: 20px;">Error loading results: ${e.message}.<br>Make sure you are running a local server.</div>`;
        console.error("Failed to load data:", e);
    }
}

document.addEventListener('DOMContentLoaded', init);
