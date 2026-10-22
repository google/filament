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

export class TestFilter extends LitElement {
  static properties = {
    filters: { type: Array },
    label: { type: String },
    placeholder: { type: String }
  };

  constructor() {
    super();
    this.filters = [];
    this.label = 'Test Name:';
    this.placeholder = 'Filter tests...';
  }

  static styles = css`
    :host {
      display: inline-flex;
      align-items: center;
      flex-wrap: wrap;
      gap: 8px;
    }
    input {
      padding: 4px;
      font-size: 11px;
      border: 1px solid #ccc;
      border-radius: 4px;
      max-width: 120px;
    }
    .tag {
      display: inline-flex;
      align-items: center;
      background: #e0e0e0;
      padding: 2px 6px;
      border-radius: 12px;
      font-size: 11px;
      gap: 4px;
      white-space: nowrap;
    }
    .remove-btn {
      background: none;
      border: none;
      color: #666;
      cursor: pointer;
      padding: 0;
      font-size: 14px;
      line-height: 1;
      display: flex;
      align-items: center;
      justify-content: center;
    }
    .remove-btn:hover {
      color: #000;
    }
  `;

  render() {
    return html`
      ${this.label ? html`<span style="font-weight: 600; font-size: 11px;">${this.label}</span>` : ''}
      <input 
        type="text" 
        .placeholder="${this.placeholder}" 
        @keydown="${this._handleKeydown}"
      >
      ${this.filters.map((filter, index) => html`
        <div class="tag">
          ${filter}
          <button class="remove-btn" @click="${() => this._removeFilter(index)}">×</button>
        </div>
      `)}
    `;
  }

  _handleKeydown(e) {
    if (e.key === 'Enter') {
      const val = e.target.value.trim();
      if (val && !this.filters.includes(val)) {
        this.filters = [...this.filters, val];
        this._dispatchChange();
      }
      e.target.value = '';
    }
  }

  _removeFilter(index) {
    this.filters = this.filters.filter((_, i) => i !== index);
    this._dispatchChange();
  }

  _dispatchChange() {
    this.dispatchEvent(new CustomEvent('filters-changed', {
      detail: { filters: this.filters },
      bubbles: true,
      composed: true
    }));
  }
}

customElements.define('test-filter', TestFilter);
