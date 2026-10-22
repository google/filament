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

import { html, css, LitElement } from 'lit';
import './test-filter.js';

export class ResultTable extends LitElement {
  static properties = {
    results: { type: Array },
    sortBy: { type: String },
    backendFilter: { type: String },
    nameFilters: { type: Array },
    showOnlyFailures: { type: Boolean },
    gpuFilters: { type: Array }
  };

  constructor() {
    super();
    this.sortBy = 'name-gpu-os';
    this.backendFilter = 'all';
    this.nameFilters = [];
    this.showOnlyFailures = false;
    this.gpuFilters = [];
  }

  static styles = css`
    :host {
      display: block;
      background: white;
      border-radius: 8px;
      box-shadow: 0 4px 6px rgba(0,0,0,0.1);
      overflow: auto;
      max-height: 100vh;
      max-width: 100vw;
      font-size: 11px;
    }
    .filter-bar {
      padding: 5px 10px;
      background: #f8f9fa;
      border-bottom: 1px solid #eee;
      display: flex;
      align-items: center;
      gap: 10px;
      position: sticky;
      top: 0;
      left: 0;
      z-index: 20;
      margin-left: 200px;
      margin-right: -200px;
    }
    table {
      width: 100%;
      border-collapse: separate;
      border-spacing: 0;
    }
    th, td {
      padding: 4px;
      text-align: left;
      border-bottom: 1px solid #eee;
    }
    th {
      background-color: #f8f9fa;
      font-weight: 600;
      color: #2c3e50;
      white-space: nowrap;
      position: sticky;
      top: 35px; /* Offset by filter-bar height approx */
      z-index: 10;
    }
    .device-col {
      color: #34495e;
      border-right: 1px solid #eee;
      position: sticky;
      left: 0;
      background-color: #fff;
      z-index: 5;
      width: 200px;
      min-width: 200px;
      max-width: 200px;
      box-sizing: border-box;
    }
    th.device-col {
      z-index: 15;
      background-color: #f8f9fa;
    }
    .result-cell {
      text-align: center;
    }
    .result-container {
      display: flex;
      padding: 6px;
      border-radius: 6px;
      cursor: pointer;
      transition: transform 0.2s;
      width: 50px;
      height: 50px;
    }
    .result-container:hover {
      transform: scale(1.05);
    }
    .pass {
      background-color: #d4edda;
      border: 1px solid #c3e6cb;
    }
    .fail {
      background-color: #f8d7da;
      border: 1px solid #f5c6cb;
    }
    .thumb {
      display: block;
      width: 100%;
    }
    .test-name {
      font-size: 0.85em;
      color: #555;
    }
    .test-name-container {
      vertical-align: top;
    }
  `;

  render() {
      if (!this.results || this.results.length === 0) {
          return html`<p style="padding: 20px;">No results found.</p>`;
      }

      // Extract all unique test names to build columns
      const allTests = new Set();
      this.results.forEach(device => {
          device.runs.forEach(run => allTests.add(run.testName));
      });

      let testNames = Array.from(allTests).sort(
          (a, b) => {
              let [atest, abackend, amodel] = a.split('.');
              let [btest, bbackend, bmodel] = b.split('.');
              if (abackend === bbackend) {
                  return atest < btest ? -1 : 1;
              }
              return abackend < bbackend ? -1 : 1;
          }
      );

      // Apply backend filter
      if (this.backendFilter !== 'all') {
          testNames = testNames.filter(name => {
              const [, backend] = name.split('.');
              return backend === this.backendFilter;
          });
      }

      // Apply name filters (OR logic)
      if (this.nameFilters && this.nameFilters.length > 0) {
          testNames = testNames.filter(name => {
              return this.nameFilters.some(filterStr => name.includes(filterStr));
          });
      }

      // Apply failure filter
      if (this.showOnlyFailures) {
          testNames = testNames.filter(name => {
              return this.results.some(device => {
                  const run = device.runs.find(r => r.testName === name);
                  return run && !run.passed;
              });
          });
      }

      const nameToDiv = (name) => {
          const [testName, backend, modelName] = name.split('.');
          const sname = testName.split('_').map((n,i) => {
              const style = (i > 0) ? 'font-size:8px' : '';
              return html`<span style="${style}">${n}</span>`
          });
          const border = "border:1px solid black;border-radius:5px;padding:3px;";
          const buttonStyle = border + "font-size:9px;";
          const buttonColor = backend == 'opengl' ?
                              "background-color:#e0e3c0" :
                              "background-color:#b3b0f0";
          return html`
              <th class="test-name-container">
                  <div style="margin-bottom:5px;display:inline-flex;flex-direction:column;${border}">
                      ${sname}
                  </div>
                  <span style="display:flex">
                      <div style="${buttonStyle};${buttonColor}">
                          ${backend}
                      </div>
                  </span>
              </th>
          `;
      };
      const testRow = testNames.map(nameToDiv);

      const getShortGPUName = (device) => {
          const gpuStr = device.metadata?.gpu_driver_info?.opengl || '';
          if (gpuStr.includes('PowerVR')) return 'PowerVR';
          if (gpuStr.includes('Mali')) return 'Mali';
          if (gpuStr.includes('Adreno')) return 'Adreno';
          if (gpuStr.includes('Xclipse')) return 'Xclipse';
          return gpuStr;
      };

      let filteredDevices = this.results;
      if (this.gpuFilters && this.gpuFilters.length > 0) {
          filteredDevices = filteredDevices.filter(device => {
              const gpuStr = device.metadata?.gpu_driver_info?.opengl || '';
              return this.gpuFilters.some(filterStr => gpuStr.toLowerCase().includes(filterStr.toLowerCase()));
          });
      }

      const sortedResults = [...filteredDevices].sort((a, b) => {
          const nameA = a.metadata.device_name || '';
          const nameB = b.metadata.device_name || '';

          const gpuA = getShortGPUName(a);
          const gpuB = getShortGPUName(b);

          const osA = parseInt(a.metadata.android_version, 10) || 0;
          const osB = parseInt(b.metadata.android_version, 10) || 0;

          const cmpName = nameA.localeCompare(nameB);
          const cmpGpu = gpuA.localeCompare(gpuB);
          const cmpOs = osA - osB;

          if (this.sortBy === 'name-gpu-os') {
              if (cmpName !== 0) return cmpName;
              if (cmpGpu !== 0) return cmpGpu;
              return cmpOs;
          } else if (this.sortBy === 'gpu-os-name') {
              if (cmpGpu !== 0) return cmpGpu;
              if (cmpOs !== 0) return cmpOs;
              return cmpName;
          } else if (this.sortBy === 'os-gpu-name') {
              if (cmpOs !== 0) return cmpOs;
              if (cmpGpu !== 0) return cmpGpu;
              return cmpName;
          }
          return 0;
      });

      return html`
          <div style="position:absolute;width:200px;height:50px;background:white;z-index:10;">&nbsp;</div>
          <div class="filter-bar">
              <span style="font-weight: 600;">Backend Filter:</span>
              <select style="font-size: 11px; padding: 4px;" @change="${(e) => this.backendFilter = e.target.value}">
                  <option value="all" ?selected="${this.backendFilter === 'all'}">opengl / vulkan</option>
                  <option value="opengl" ?selected="${this.backendFilter === 'opengl'}">opengl</option>
                  <option value="vulkan" ?selected="${this.backendFilter === 'vulkan'}">vulkan</option>
              </select>
              <div style="width: 1px; height: 20px; background: #ccc; margin: 0 10px;"></div>
              <test-filter @filters-changed="${(e) => this.nameFilters = e.detail.filters}"></test-filter>
              <div style="width: 1px; height: 20px; background: #ccc; margin: 0 10px;"></div>
              <label style="display: flex; align-items: center; gap: 4px; font-weight: 600; font-size: 11px; cursor: pointer;">
                  <input type="checkbox" .checked="${this.showOnlyFailures}" @change="${(e) => this.showOnlyFailures = e.target.checked}">
                  Only columns with failed tests
              </label>
          </div>
          <table>
              <thead>
                  <tr>
                      <th class="device-col" style="font-size:15px; vertical-align: top; padding-top: 8px;">
                          <div>Device</div>
                          <select style="font-size: 11px; margin-top: 6px; width: 100%; padding: 2px;" @change="${(e) => this.sortBy = e.target.value}">
                              <option value="name-gpu-os" ?selected="${this.sortBy === 'name-gpu-os'}">Device -> GPU -> Android Ver</option>
                              <option value="gpu-os-name" ?selected="${this.sortBy === 'gpu-os-name'}">GPU -> Android Ver -> Device</option>
                              <option value="os-gpu-name" ?selected="${this.sortBy === 'os-gpu-name'}">Android Ver -> GPU -> Device</option>
                          </select>
                          <div style="margin-top: 8px;">
                              <test-filter label="GPU:" placeholder="Filter GPU..." @filters-changed="${(e) => this.gpuFilters = e.detail.filters}"></test-filter>
                          </div>
                      </th>
                      ${testRow}
                  </tr>
              </thead>
        <tbody>
            ${
            sortedResults.map(device => {
            const processBuildNumber = (rawBuild) => {
               if (rawBuild.indexOf(' ') >=0 && rawBuild.indexOf('dev-keys') >= 0) {
                   return rawBuild.split(' ')[2].split('.')[0];
               }
               return rawBuild.split('.')[0];
            };
            const androidVersion = device.metadata.android_version + " " +
                processBuildNumber(device.metadata.android_build_number);
            const glGPU = device.metadata.gpu_driver_info.opengl.split(' | ');
            const driverInfo = device.metadata.gpu_driver_info.vulkan.split(' | ')[2];
            const truncatedStyle = "display:block;width:200px;white-space:nowrap;overflow:hidden;text-overflow:ellipsis;";
            const marginLow = "margin-bottom:4px;"
            const androidVerStyle = marginLow + (this.sortBy.startsWith('os') ? "color:#f09090;" : '');
            const gpuStyle = marginLow + (this.sortBy.startsWith('gpu') ? "color:#f09090;" : '');
            const hardwareStyle = marginLow + "color:#bbb;";
            const deviceNameStyle = marginLow + "font-size:15px;font-weight:bold;";
            return html`
              <tr>
                <td class="device-col">
                    <div style="${deviceNameStyle}">${device.metadata.device_name} </div>
                    <div style="${hardwareStyle}">${device.metadata.device_hardware} </div>
                    <div style="${androidVerStyle}">Android ${androidVersion}</div>
                    <div style="${gpuStyle}">${glGPU[0]} ${glGPU[1]}</div>
                    <div style="${truncatedStyle}">${driverInfo}</div>
                </td>
                ${
                    testNames.map(testName => {
                    const run = device.runs.find(r => r.testName === testName);
                        if (!run) return html`<td>-</td>`;

                        return html`
                            <td class="result-cell">
                                <div class="result-container ${run.passed ? 'pass' : 'fail'}"
                                    @click="${() => this._handleThumbnailClick(device.device, run)}">
                                    <img class="thumb" src="${run.thumb}" loading="lazy" alt="Thumbnail" />
                                </div>
                            </td>
                        `;
                    }
                    )}
              </tr>
           `})
        }
        </tbody>
      </table>
    `;
  }

  _handleThumbnailClick(device, run) {
    this.dispatchEvent(new CustomEvent('view-result', {
      detail: { device, run },
      bubbles: true,
      composed: true
    }));
  }
}

customElements.define('result-table', ResultTable);
