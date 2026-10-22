// Key mapping from standard JS e.code or e.key to Filament AppKey enum index (1-indexed)
const KeyMapping = {
        'Escape': 1,
    'Enter': 2,
    'Tab': 3,
    'Backspace': 4,
    'Insert': 5,
    'Delete': 6,
    'ArrowRight': 7,
    'ArrowLeft': 8,
    'ArrowDown': 9,
    'ArrowUp': 10,
    'PageUp': 11,
    'PageDown': 12,
    'Home': 13,
    'End': 14,
    'CapsLock': 15,
    'ScrollLock': 16,
    'NumLock': 17,
    'PrintScreen': 18,
    'Pause': 19,
    'ControlLeft': 37,
    'ShiftLeft': 38,
    'AltLeft': 39,
    'MetaLeft': 40,
    'ControlRight': 41,
    'ShiftRight': 42,
    'AltRight': 43,
    'MetaRight': 44,
    'ContextMenu': 45,
    'KeyA': 46,
    'KeyB': 47,
    'KeyC': 48,
    'KeyD': 49,
    'KeyE': 50,
    'KeyF': 51,
    'KeyG': 52,
    'KeyH': 53,
    'KeyI': 54,
    'KeyJ': 55,
    'KeyK': 56,
    'KeyL': 57,
    'KeyM': 58,
    'KeyN': 59,
    'KeyO': 60,
    'KeyP': 61,
    'KeyQ': 62,
    'KeyR': 63,
    'KeyS': 64,
    'KeyT': 65,
    'KeyU': 66,
    'KeyV': 67,
    'KeyW': 68,
    'KeyX': 69,
    'KeyY': 70,
    'KeyZ': 71,
    'Digit0': 72,
    'Digit1': 73,
    'Digit2': 74,
    'Digit3': 75,
    'Digit4': 76,
    'Digit5': 77,
    'Digit6': 78,
    'Digit7': 79,
    'Digit8': 80,
    'Digit9': 81,
    'Space': 108,
    'Comma': 109,
    'Period': 110,
    'Semicolon': 111,
    'Backquote': 112,
    'Minus': 113,
    'Equal': 114,
    'BracketLeft': 115,
    'BracketRight': 116,
    'Backslash': 117,
    'Quote': 118,
    'Slash': 119
};

// Register the Custom Resizable Filament Viewer Web Component
class FilamentViewer extends HTMLElement {
    constructor() {
        super();
        this.attachShadow({ mode: 'open' });

        const container = document.createElement('div');
        container.className = 'viewer-container';
        container.tabIndex = 0; // Allow container to be focusable

        const img = document.createElement('img');
        img.className = 'viewer-image';
        img.alt = 'Filament Stream Output';

        const badge = document.createElement('div');
        badge.className = 'resolution-badge';

        const style = document.createElement('style');
        style.textContent = `
            :host {
                display: block;
                position: relative;
            }
            .viewer-container {
                position: relative;
                width: 800px;
                height: 600px;
                min-width: 200px;
                min-height: 150px;
                max-width: 95vw;
                max-height: 80vh;
                resize: both;
                overflow: hidden;
                background: #000;
                border-radius: 12px;
                border: 1px solid rgba(255, 255, 255, 0.12);
                box-shadow: 0 20px 50px rgba(0, 0, 0, 0.6), 0 0 0 1px rgba(255, 255, 255, 0.05);
                display: flex;
                align-items: center;
                justify-content: center;
                outline: none;
                transition: border-color 0.3s, box-shadow 0.3s;
            }
            .viewer-container:focus {
                border-color: rgba(56, 139, 253, 0.6);
                box-shadow: 0 20px 50px rgba(0, 0, 0, 0.6), 0 0 0 2px rgba(56, 139, 253, 0.4);
            }
            .viewer-image {
                width: 100%;
                height: 100%;
                object-fit: contain;
                user-select: none;
                -webkit-user-drag: none;
                pointer-events: none; /* Allow mouse events to bubble to container */
            }
            .resolution-badge {
                position: absolute;
                bottom: 12px;
                left: 12px;
                background: rgba(10, 12, 16, 0.8);
                backdrop-filter: blur(8px);
                -webkit-backdrop-filter: blur(8px);
                border: 1px solid rgba(255, 255, 255, 0.1);
                padding: 4px 10px;
                border-radius: 20px;
                font-size: 11px;
                font-family: 'JetBrains Mono', monospace;
                color: #8b949e;
                pointer-events: none;
                opacity: 0;
                transition: opacity 0.2s;
            }
            .viewer-container:hover .resolution-badge {
                opacity: 1;
            }
            /* Custom resize handle style */
            .viewer-container::-webkit-resizer {
                background-image: url('data:image/svg+xml;utf8,<svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="rgba(255,255,255,0.5)" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><line x1="21" y1="21" x2="9" y2="9"></line><line x1="21" y1="21" x2="16" y2="16"></line><line x1="21" y1="21" x2="21" y2="11"></line><line x1="21" y1="21" x2="11" y2="21"></line></svg>');
                background-size: 10px 10px;
                background-repeat: no-repeat;
                background-position: bottom right;
                padding: 4px;
            }
        `;

        container.appendChild(img);
        container.appendChild(badge);
        this.shadowRoot.appendChild(style);
        this.shadowRoot.appendChild(container);

        this.container = container;
        this.img = img;
        this.badge = badge;

        this.setupInputListeners();
    }

    connectedCallback() {
        this.resizeObserver = new ResizeObserver(entries => {
            for (let entry of entries) {
                const { width, height } = entry.contentRect;
                const roundedW = Math.round(width);
                const roundedH = Math.round(height);
                this.badge.textContent = `${roundedW} × ${roundedH}`;

                this.dispatchEvent(new CustomEvent('viewer-resize',
                        { detail: { width: roundedW, height: roundedH } }));
            }
        });
        this.resizeObserver.observe(this.container);

        // Autofocus on mount
        setTimeout(() => this.container.focus(), 100);
    }

    disconnectedCallback() {
        if (this.resizeObserver) {
            this.resizeObserver.disconnect();
        }
    }

    updateImage(blob) {
        const oldSrc = this.img.src;
        this.img.src = URL.createObjectURL(blob);
        if (oldSrc) {
            URL.revokeObjectURL(oldSrc);
        }
    }

    // Get relative mouse coordinates bounded to the component
    getRelativeCoords(e) {
        const rect = this.container.getBoundingClientRect();
        const x = Math.max(0, Math.min(Math.round(e.clientX - rect.left), rect.width));
        const y = Math.max(0, Math.min(Math.round(e.clientY - rect.top), rect.height));
        return { x, y };
    }

    setupInputListeners() {
        // Mouse Events on focused container
        this.container.addEventListener('mousedown', (e) => {
            if (typeof eventMap['MOUSE_BUTTON_DOWN'] === 'undefined') return;
            this.container.focus();
            const { x, y } = this.getRelativeCoords(e);
            const buffer = new ArrayBuffer(10);
            const view = new DataView(buffer);
            view.setUint8(0, eventMap['MOUSE_BUTTON_DOWN']);
            view.setUint8(1, e.button + 1);
            view.setInt32(2, x, true);
            view.setInt32(6, y, true);
            this.dispatchEvent(new CustomEvent('viewer-message', { detail: buffer }));
        });

        this.container.addEventListener('mouseup', (e) => {
            if (typeof eventMap['MOUSE_BUTTON_UP'] === 'undefined') return;
            const { x, y } = this.getRelativeCoords(e);
            const buffer = new ArrayBuffer(10);
            const view = new DataView(buffer);
            view.setUint8(0, eventMap['MOUSE_BUTTON_UP']);
            view.setUint8(1, e.button + 1);
            view.setInt32(2, x, true);
            view.setInt32(6, y, true);
            this.dispatchEvent(new CustomEvent('viewer-message', { detail: buffer }));
        });

        this.container.addEventListener('mousemove', (e) => {
            if (typeof eventMap['MOUSE_MOVE'] === 'undefined') return;
            const { x, y } = this.getRelativeCoords(e);
            const buffer = new ArrayBuffer(9);
            const view = new DataView(buffer);
            view.setUint8(0, eventMap['MOUSE_MOVE']);
            view.setInt32(1, x, true);
            view.setInt32(5, y, true);
            this.dispatchEvent(new CustomEvent('viewer-message', { detail: buffer }));
        });

        this.container.addEventListener('wheel', (e) => {
            if (typeof eventMap['MOUSE_WHEEL'] === 'undefined') return;
            e.preventDefault();
            const buffer = new ArrayBuffer(5);
            const view = new DataView(buffer);
            view.setUint8(0, eventMap['MOUSE_WHEEL']);
            view.setInt32(1, e.deltaY > 0 ? -1 : 1, true);
            this.dispatchEvent(new CustomEvent('viewer-message', { detail: buffer }));
        }, { passive: false });

        // Keyboard Events (only active when container is focused!)
        this.container.addEventListener('keydown', (e) => {
            if (typeof eventMap['KEYDOWN'] === 'undefined') return;
            if (e.repeat) return;
            let code = KeyMapping[e.code] || KeyMapping[e.key] || 0;
            if (code === 0) return;

            if (['Space', 'ArrowUp', 'ArrowDown', 'ArrowLeft', 'ArrowRight'].includes(e.code)) {
                e.preventDefault();
            }

            const buffer = new ArrayBuffer(5);
            const view = new DataView(buffer);
            view.setUint8(0, eventMap['KEYDOWN']);
            view.setUint32(1, code, true);
            this.dispatchEvent(new CustomEvent('viewer-message', { detail: buffer }));
        });

        this.container.addEventListener('keyup', (e) => {
            if (typeof eventMap['KEYUP'] === 'undefined') return;
            let code = KeyMapping[e.code] || KeyMapping[e.key] || 0;
            if (code === 0) return;

            const buffer = new ArrayBuffer(5);
            const view = new DataView(buffer);
            view.setUint8(0, eventMap['KEYUP']);
            view.setUint32(1, code, true);
            this.dispatchEvent(new CustomEvent('viewer-message', { detail: buffer }));
        });
    }
}

customElements.define('filament-viewer', FilamentViewer);

// Connect to HtmlDisplayManager WebSocket
const viewer = document.getElementById('remote-viewer');
const statusIndicator = document.getElementById('connection-status');
const resolutionDisplay = document.getElementById('resolution-display');

const wsUrl = `ws://${location.host}/ws`;
let ws = null;
let eventMap = {};
let handshaked = false;
let reconnectTimeout = null;

function updateStatus(state, text) {
    statusIndicator.className = `status-indicator ${state}`;
    statusIndicator.querySelector('.text').textContent = text;
}

function connect() {
    if (ws) {
        ws.onopen = null;
        ws.onerror = null;
        ws.onclose = null;
        ws.onmessage = null;
        try {
            ws.close();
        } catch (e) {
        }
    }

    updateStatus('disconnected', 'Connecting...');
    ws = new WebSocket(wsUrl);
    ws.binaryType = 'arraybuffer';

    ws.onopen = () => {
        if (reconnectTimeout) {
            clearTimeout(reconnectTimeout);
            reconnectTimeout = null;
        }
        ws.send(JSON.stringify({ type: 'handshake', magic: 'FILAMENT_HEADLESS' }));
    };

    ws.onerror = () => { updateStatus('disconnected', 'Error'); };

    ws.onclose = () => {
        handshaked = false;
        updateStatus('disconnected', 'Disconnected');
        scheduleReconnect();
    };

    ws.onmessage = (event) => {
        if (typeof event.data === 'string') {
            try {
                const msg = JSON.parse(event.data);
                if (msg.type === 'handshake_ack' && msg.magic === 'FILAMENT_HEADLESS') {
                    eventMap = msg.event_map;
                    handshaked = true;
                    updateStatus('connected', 'Connected');
                    // Send initial size of container as resize package
                    const rect = viewer.container.getBoundingClientRect();
                    sendResize(Math.round(rect.width), Math.round(rect.height));
                }
            } catch (e) {
                console.error('Error parsing handshake message:', e);
            }
            return;
        }
        const blob = new Blob([event.data], { type: 'image/png' });
        viewer.updateImage(blob);
    };
}

function scheduleReconnect() {
    if (reconnectTimeout) return;
    reconnectTimeout = setTimeout(() => {
        reconnectTimeout = null;
        connect();
    }, 3000);
}

// Helper: Send resizing
function sendResize(w, h) {
    if (!handshaked || !ws || ws.readyState !== WebSocket.OPEN) return;
    const buffer = new ArrayBuffer(9);
    const view = new DataView(buffer);
    view.setUint8(0, eventMap['RESIZE']); // Dynamic event type from handshake!
    view.setUint32(1, w, true);
    view.setUint32(5, h, true);
    ws.send(buffer);
    resolutionDisplay.textContent = `${w} × ${h}`;
}

// Forward messages / resizing events from custom component to WebSocket
viewer.addEventListener('viewer-message', (e) => {
    if (handshaked && ws && ws.readyState === WebSocket.OPEN) {
        ws.send(e.detail);
    }
});

viewer.addEventListener('viewer-resize', (e) => { sendResize(e.detail.width, e.detail.height); });

// Initial connection
connect();
