// Core State Manager
const state = {
    zScore: 1.282,
    vsyncRate: 60,
    latchMargin: 3.0,
    pacingMode: 'auto',
    cpu: { mean: 8.0, std: 1.5 },
    backend: { mean: 4.0, std: 0.8 },
    gpu: { mean: 12.0, std: 2.0 }
};

// UI Elements
const els = {
    btnP50: document.getElementById('btn-p50'),
    btnP90: document.getElementById('btn-p90'),
    btnP95: document.getElementById('btn-p95'),
    zScoreInput: document.getElementById('z-score'),
    zScoreVal: document.getElementById('val-z-score'),
    
    vsyncRateInput: document.getElementById('vsync-rate'),
    vsyncRateVal: document.getElementById('val-vsync-rate'),
    latchMarginInput: document.getElementById('latch-margin'),
    latchMarginVal: document.getElementById('val-latch-margin'),
    
    cpuMeanInput: document.getElementById('cpu-mean'),
    cpuMeanVal: document.getElementById('val-cpu-mean'),
    cpuStdInput: document.getElementById('cpu-std'),
    cpuStdVal: document.getElementById('val-cpu-std'),
    
    backendMeanInput: document.getElementById('backend-mean'),
    backendMeanVal: document.getElementById('val-backend-mean'),
    backendStdInput: document.getElementById('backend-std'),
    backendStdVal: document.getElementById('val-backend-std'),
    
    gpuMeanInput: document.getElementById('gpu-mean'),
    gpuMeanVal: document.getElementById('val-gpu-mean'),
    gpuStdInput: document.getElementById('gpu-std'),
    gpuStdVal: document.getElementById('val-gpu-std'),
    
    pacingAutoBtn: document.getElementById('pacing-auto'),
    pacingVsyncBtn: document.getElementById('pacing-vsync'),
    
    kpiLatency: document.getElementById('kpi-val-latency'),
    kpiLatencyMs: document.getElementById('kpi-val-latency-ms'),
    kpiFps: document.getElementById('kpi-val-fps'),
    kpiFrameDuration: document.getElementById('kpi-val-frame-duration'),
    kpiDelay: document.getElementById('kpi-val-delay'),
    kpiBottleneck: document.getElementById('kpi-val-bottleneck'),
    kpiBottleneckDetail: document.getElementById('kpi-val-bottleneck-detail'),
    
    gaussianCanvas: document.getElementById('gaussian-canvas'),
    timelineCanvas: document.getElementById('pipeline-timeline-canvas')
};

// Initialize Event Listeners
function initListeners() {
    // Percentile Buttons
    const buttons = [els.btnP50, els.btnP90, els.btnP95];
    buttons.forEach(btn => {
        btn.addEventListener('click', () => {
            buttons.forEach(b => b.classList.remove('active'));
            btn.classList.add('active');
            const z = parseFloat(btn.dataset.z);
            state.zScore = z;
            els.zScoreInput.value = z;
            els.zScoreVal.innerText = z.toFixed(2);
            update();
        });
    });

    // Z-Score Slider
    els.zScoreInput.addEventListener('input', (e) => {
        buttons.forEach(b => b.classList.remove('active'));
        const z = parseFloat(e.target.value);
        state.zScore = z;
        els.zScoreVal.innerText = z.toFixed(2);
        
        // Match active class if close to presets
        if (Math.abs(z - 0.0) < 0.05) els.btnP50.classList.add('active');
        else if (Math.abs(z - 1.282) < 0.05) els.btnP90.classList.add('active');
        else if (Math.abs(z - 1.645) < 0.05) els.btnP95.classList.add('active');
        
        update();
    });

    // Vsync Slider
    els.vsyncRateInput.addEventListener('input', (e) => {
        state.vsyncRate = parseInt(e.target.value);
        els.vsyncRateVal.innerText = `${state.vsyncRate} Hz`;
        update();
    });

    // Latch Margin Slider
    els.latchMarginInput.addEventListener('input', (e) => {
        state.latchMargin = parseFloat(e.target.value);
        els.latchMarginVal.innerText = `${state.latchMargin.toFixed(1)} ms`;
        update();
    });

    // CPU Inputs
    els.cpuMeanInput.addEventListener('input', (e) => {
        state.cpu.mean = parseFloat(e.target.value);
        els.cpuMeanVal.innerText = `${state.cpu.mean.toFixed(1)} ms`;
        update();
    });
    els.cpuStdInput.addEventListener('input', (e) => {
        state.cpu.std = parseFloat(e.target.value);
        els.cpuStdVal.innerText = `${state.cpu.std.toFixed(1)} ms`;
        update();
    });

    // Backend Inputs
    els.backendMeanInput.addEventListener('input', (e) => {
        state.backend.mean = parseFloat(e.target.value);
        els.backendMeanVal.innerText = `${state.backend.mean.toFixed(1)} ms`;
        update();
    });
    els.backendStdInput.addEventListener('input', (e) => {
        state.backend.std = parseFloat(e.target.value);
        els.backendStdVal.innerText = `${state.backend.std.toFixed(1)} ms`;
        update();
    });

    // GPU Inputs
    els.gpuMeanInput.addEventListener('input', (e) => {
        state.gpu.mean = parseFloat(e.target.value);
        els.gpuMeanVal.innerText = `${state.gpu.mean.toFixed(1)} ms`;
        update();
    });
    els.gpuStdInput.addEventListener('input', (e) => {
        state.gpu.std = parseFloat(e.target.value);
        els.gpuStdVal.innerText = `${state.gpu.std.toFixed(1)} ms`;
        update();
    });
    
    // Pacing Buttons
    const pacingButtons = [els.pacingAutoBtn, els.pacingVsyncBtn];
    pacingButtons.forEach(btn => {
        btn.addEventListener('click', () => {
            pacingButtons.forEach(b => b.classList.remove('active'));
            btn.classList.add('active');
            
            const mode = btn.id.replace('pacing-', '');
            state.pacingMode = mode;
            update();
        });
    });

    // Auto-resize canvases on window resize
    window.addEventListener('resize', () => {
        setupCanvas(els.gaussianCanvas);
        setupCanvas(els.timelineCanvas);
        draw();
    });
}

// Setup Canvas for High DPI screens (retina support)
function setupCanvas(canvas) {
    const rect = canvas.parentNode.getBoundingClientRect();
    const dpr = window.devicePixelRatio || 1;
    const baseHeight = parseFloat(canvas.dataset.baseHeight || canvas.getAttribute('height'));
    
    canvas.width = rect.width * dpr;
    canvas.height = baseHeight * dpr;
    canvas.style.width = `${rect.width}px`;
    canvas.style.height = `${baseHeight}px`;
    
    const ctx = canvas.getContext('2d');
    ctx.scale(dpr, dpr);
}

// Normal Distribution Probability Density Function
function normalPDF(x, mu, sigma) {
    if (sigma === 0) return x === mu ? 1.0 : 0.0;
    const exponent = -0.5 * Math.pow((x - mu) / sigma, 2);
    return (1 / (sigma * Math.sqrt(2 * Math.PI))) * Math.exp(exponent);
}

// Core Calculations (FramePipelineEstimator math)
function calculate() {
    const vsyncInterval = 1000.0 / state.vsyncRate; // ms
    
    // Effective durations: mu + Z * std
    const effCpu = Math.max(0.1, state.cpu.mean + (state.zScore * state.cpu.std));
    const effBackend = Math.max(0.1, state.backend.mean + (state.zScore * state.backend.std));
    const effGpu = Math.max(0.1, state.gpu.mean + (state.zScore * state.gpu.std));
    
    // Bottleneck stage (slowest stage determines throughput limit)
    let bottleneckVal = effCpu;
    let bottleneckName = 'Main CPU';
    if (effBackend > bottleneckVal) {
        bottleneckVal = effBackend;
        bottleneckName = 'Backend Driver';
    }
    if (effGpu > bottleneckVal) {
        bottleneckVal = effGpu;
        bottleneckName = 'GPU Execution';
    }
    
    const idealFrameTime = bottleneckVal;
    
    // Pacing frame step (duration): auto (bottleneck) or vsync
    let pacingInterval = idealFrameTime;
    if (state.pacingMode === 'vsync') {
        pacingInterval = vsyncInterval;
    }
    
    const totalTransitTime = effCpu + effBackend + effGpu;
    const latchMargin = state.latchMargin;
    
    // Structural latency based on pacing interval and composition latch margin
    const idealLatency = Math.max(1, Math.ceil((totalTransitTime + latchMargin) / pacingInterval));
    
    // Target budget: idealLatency * pacingInterval - latchMargin
    const budget = idealLatency * pacingInterval - latchMargin;
    
    // Safe delay (slack)
    const safeDelay = Math.max(0, budget - totalTransitTime);
    
    // Recommended frame rate
    const idealFps = 1000.0 / pacingInterval;
    
    return {
        vsyncInterval,
        effCpu,
        effBackend,
        effGpu,
        bottleneckName,
        bottleneckVal,
        idealFrameTime,
        pacingInterval,
        totalTransitTime,
        idealLatency,
        budget,
        safeDelay,
        idealFps,
        latchMargin
    };
}

// Update KPI cards and text readouts
function updateKPIs(calc) {
    els.kpiLatency.innerText = `${calc.idealLatency} Frame${calc.idealLatency > 1 ? 's' : ''}`;
    els.kpiLatencyMs.innerText = `${(calc.idealLatency * calc.vsyncInterval).toFixed(1)} ms target latency`;
    
    els.kpiFps.innerText = `${calc.idealFps.toFixed(1)} Hz`;
    els.kpiFrameDuration.innerText = `${calc.pacingInterval.toFixed(1)} ms pacing step`;
    
    els.kpiDelay.innerText = `${calc.safeDelay.toFixed(1)} ms`;
    
    els.kpiBottleneck.innerText = calc.bottleneckName;
    els.kpiBottleneckDetail.innerText = `Effective workload: ${calc.bottleneckVal.toFixed(1)} ms`;
}

// Drawing Logic
function draw() {
    const calc = calculate();
    updateKPIs(calc);
    
    drawGaussian(calc);
    drawTimeline(calc);
}

// Draw Gaussian Distribution Canvas
function drawGaussian(calc) {
    const canvas = els.gaussianCanvas;
    const ctx = canvas.getContext('2d');
    const w = canvas.width / (window.devicePixelRatio || 1);
    const h = canvas.height / (window.devicePixelRatio || 1);
    
    ctx.clearRect(0, 0, w, h);
    
    const paddingLeft = 40;
    const paddingRight = 20;
    const paddingTop = 20;
    const paddingBottom = 30;
    
    const graphWidth = w - paddingLeft - paddingRight;
    const graphHeight = h - paddingTop - paddingBottom;
    
    // Max X limit (40ms base, or scaled up if workload is large)
    const maxVal = Math.max(40, state.cpu.mean + 3.5 * state.cpu.std, state.gpu.mean + 3.5 * state.gpu.std);
    
    // Find maximum Y value for scale mapping
    let maxPDF = 0.05; // baseline minimum scale
    for (let x = 0; x <= maxVal; x += 0.2) {
        maxPDF = Math.max(maxPDF, 
            normalPDF(x, state.cpu.mean, state.cpu.std),
            normalPDF(x, state.backend.mean, state.backend.std),
            normalPDF(x, state.gpu.mean, state.gpu.std)
        );
    }
    
    const scaleX = graphWidth / maxVal;
    const scaleY = graphHeight / (maxPDF * 1.1); // add 10% headroom
    
    // Helper: Map data coordinate to screen space
    function mapX(val) { return paddingLeft + val * scaleX; }
    function mapY(val) { return h - paddingBottom - val * scaleY; }
    
    // Draw Grid Lines (X-Axis intervals every 5ms)
    ctx.strokeStyle = '#2d3748';
    ctx.lineWidth = 1;
    ctx.fillStyle = '#6b7280';
    ctx.font = '10px var(--font-mono)';
    ctx.textAlign = 'center';
    
    for (let val = 0; val <= maxVal; val += 5) {
        const x = mapX(val);
        ctx.beginPath();
        ctx.moveTo(x, paddingTop);
        ctx.lineTo(x, h - paddingBottom);
        ctx.stroke();
        ctx.fillText(`${val}`, x, h - paddingBottom + 16);
    }
    
    // Axis line
    ctx.strokeStyle = '#4b5563';
    ctx.beginPath();
    ctx.moveTo(paddingLeft, h - paddingBottom);
    ctx.lineTo(w - paddingRight, h - paddingBottom);
    ctx.stroke();
    
    // Draw VSYNC Line (dashed green)
    ctx.strokeStyle = '#10b981';
    ctx.lineWidth = 1.5;
    ctx.setLineDash([4, 4]);
    ctx.beginPath();
    ctx.moveTo(mapX(calc.vsyncInterval), paddingTop);
    ctx.lineTo(mapX(calc.vsyncInterval), h - paddingBottom);
    ctx.stroke();
    ctx.setLineDash([]);
    ctx.fillStyle = '#34d399';
    ctx.font = '10px var(--font-sans)';
    ctx.textAlign = 'left';
    ctx.fillText('VSYNC Target', mapX(calc.vsyncInterval) + 4, paddingTop + 12);
    
    // Draw Compositor Latch Line (dashed red)
    if (calc.latchMargin > 0) {
        const deadlineMs = calc.vsyncInterval - calc.latchMargin;
        ctx.strokeStyle = '#ef4444';
        ctx.lineWidth = 1.5;
        ctx.setLineDash([3, 3]);
        ctx.beginPath();
        ctx.moveTo(mapX(deadlineMs), paddingTop);
        ctx.lineTo(mapX(deadlineMs), h - paddingBottom);
        ctx.stroke();
        ctx.setLineDash([]);
        ctx.fillStyle = '#f87171';
        ctx.font = '10px var(--font-sans)';
        ctx.textAlign = 'right';
        ctx.fillText('Compositor Latch', mapX(deadlineMs) - 4, paddingTop + 12);
    }
    
    // Function to draw a single stage curve
    function drawCurve(mu, sigma, color, effVal) {
        // Curve path
        ctx.strokeStyle = color;
        ctx.lineWidth = 2;
        ctx.beginPath();
        let started = false;
        
        for (let vx = 0; vx <= maxVal; vx += 0.2) {
            const vy = normalPDF(vx, mu, sigma);
            const x = mapX(vx);
            const y = mapY(vy);
            if (!started) {
                ctx.moveTo(x, y);
                started = true;
            } else {
                ctx.lineTo(x, y);
            }
        }
        ctx.stroke();
        
        // Draw shaded confidence region (0 up to mu + Z*sigma)
        ctx.fillStyle = color.replace(')', ', 0.05)').replace('rgb', 'rgba'); // semi transparent
        ctx.beginPath();
        ctx.moveTo(mapX(0), mapY(0));
        
        for (let vx = 0; vx <= effVal; vx += 0.2) {
            const vy = normalPDF(vx, mu, sigma);
            ctx.lineTo(mapX(vx), mapY(vy));
        }
        ctx.lineTo(mapX(effVal), mapY(0));
        ctx.closePath();
        ctx.fill();
        
        // Draw vertical cutoff line at effVal
        ctx.strokeStyle = color;
        ctx.lineWidth = 1;
        ctx.setLineDash([2, 2]);
        ctx.beginPath();
        ctx.moveTo(mapX(effVal), mapY(0));
        ctx.lineTo(mapX(effVal), mapY(normalPDF(effVal, mu, sigma)));
        ctx.stroke();
        ctx.setLineDash([]);
    }
    
    // Render the three stage curves
    drawCurve(state.cpu.mean, state.cpu.std, '#3b82f6', calc.effCpu);
    drawCurve(state.backend.mean, state.backend.std, '#14b8a6', calc.effBackend);
    drawCurve(state.gpu.mean, state.gpu.std, '#f59e0b', calc.effGpu);
    
    // Y-Axis label
    ctx.save();
    ctx.translate(12, h / 2);
    ctx.rotate(-Math.PI / 2);
    ctx.fillStyle = '#6b7280';
    ctx.font = '10px var(--font-sans)';
    ctx.textAlign = 'center';
    ctx.fillText('Probability Density', 0, 0);
    ctx.restore();
    
    // X-Axis label
    ctx.fillStyle = '#6b7280';
    ctx.font = '10px var(--font-sans)';
    ctx.textAlign = 'center';
    ctx.fillText('Duration (ms)', paddingLeft + graphWidth / 2, h - 4);
}

// Draw Interactive Pipeline Overlap (Timeline Flow)
function drawTimeline(calc) {
    const canvas = els.timelineCanvas;
    const ctx = canvas.getContext('2d');
    const w = canvas.width / (window.devicePixelRatio || 1);
    const h = canvas.height / (window.devicePixelRatio || 1);
    
    ctx.clearRect(0, 0, w, h);
    
    const paddingLeft = 80;
    const paddingRight = 40;
    const paddingTop = 30;
    const paddingBottom = 40;
    
    const timelineWidth = w - paddingLeft - paddingRight;
    const timelineHeight = h - paddingTop - paddingBottom;
    
    // Determine horizontal scale: we display 4 complete VSYNC intervals
    const totalDurationDisplayed = calc.vsyncInterval * 4.5;
    const scaleX = timelineWidth / totalDurationDisplayed;
    
    function mapX(ms) { return paddingLeft + ms * scaleX; }
    
    // Draw VSYNC vertical grids
    ctx.lineWidth = 1;
    ctx.fillStyle = '#9ca3af';
    ctx.font = '10px var(--font-mono)';
    ctx.textAlign = 'center';
    
    for (let i = 0; i <= 4; ++i) {
        const timeMs = i * calc.vsyncInterval;
        const x = mapX(timeMs);
        
        ctx.strokeStyle = '#1f2937';
        ctx.beginPath();
        ctx.moveTo(x, paddingTop - 10);
        ctx.lineTo(x, h - paddingBottom);
        ctx.stroke();
        
        // Text labels for Vsync ticks
        ctx.fillText(`VSYNC ${i}`, x, paddingTop - 16);
        ctx.fillText(`${timeMs.toFixed(1)}ms`, x, h - paddingBottom + 14);

        // Draw Composition Deadline (if latchMargin > 0 and i > 0)
        if (calc.latchMargin > 0 && i > 0) {
            const deadlineMs = timeMs - calc.latchMargin;
            const dx = mapX(deadlineMs);
            
            ctx.beginPath();
            ctx.strokeStyle = 'rgba(239, 68, 68, 0.35)'; // Red dash
            ctx.lineWidth = 1;
            ctx.setLineDash([2, 4]);
            ctx.moveTo(dx, paddingTop - 10);
            ctx.lineTo(dx, h - paddingBottom);
            ctx.stroke();
            ctx.setLineDash([]);
        }
    }
    
    // Stage rows properties
    const rowHeight = 36;
    const rowGap = 16;
    
    function getRowY(rowIndex) {
        return paddingTop + rowIndex * (rowHeight + rowGap);
    }
    
    // Label stages on the left
    ctx.fillStyle = '#9ca3af';
    ctx.font = '11px var(--font-sans)';
    ctx.fontWeight = '600';
    ctx.textAlign = 'right';
    
    ctx.fillText('Main CPU', paddingLeft - 10, getRowY(0) + rowHeight / 2 + 4);
    ctx.fillText('Backend', paddingLeft - 10, getRowY(1) + rowHeight / 2 + 4);
    ctx.fillText('GPU Exec', paddingLeft - 10, getRowY(2) + rowHeight / 2 + 4);
    
    // Helper to draw stage blocks for multiple parallel frames
    function drawFrame(frameIndex, offsetVsyncCount) {
        const frameStartMs = offsetVsyncCount * calc.pacingInterval + calc.safeDelay;
        
        const cpuStart = frameStartMs;
        const cpuWidth = calc.effCpu;
        
        const backendStart = cpuStart + cpuWidth;
        const backendWidth = calc.effBackend;
        
        const gpuStart = backendStart + backendWidth;
        const gpuWidth = calc.effGpu;
        
        const targetPresentTime = (offsetVsyncCount + calc.idealLatency) * calc.vsyncInterval;
        
        // Draw CPU Box
        ctx.fillStyle = 'rgba(59, 130, 246, 0.85)';
        ctx.strokeStyle = '#3b82f6';
        ctx.lineWidth = 1.5;
        ctx.beginPath();
        ctx.roundRect(mapX(cpuStart), getRowY(0), cpuWidth * scaleX, rowHeight, 4);
        ctx.fill();
        ctx.stroke();
        
        // Draw Backend Box
        ctx.fillStyle = 'rgba(20, 184, 166, 0.85)';
        ctx.strokeStyle = '#14b8a6';
        ctx.beginPath();
        ctx.roundRect(mapX(backendStart), getRowY(1), backendWidth * scaleX, rowHeight, 4);
        ctx.fill();
        ctx.stroke();
        
        // Draw GPU Box
        ctx.fillStyle = 'rgba(245, 158, 11, 0.85)';
        ctx.strokeStyle = '#f59e0b';
        ctx.beginPath();
        ctx.roundRect(mapX(gpuStart), getRowY(2), gpuWidth * scaleX, rowHeight, 4);
        ctx.fill();
        ctx.stroke();
        
        // Print frame index labels inside the boxes if wide enough
        ctx.fillStyle = '#ffffff';
        ctx.font = '10px var(--font-sans)';
        ctx.fontWeight = 'bold';
        ctx.textAlign = 'center';
        
        if (cpuWidth * scaleX > 24) ctx.fillText(`F${frameIndex}`, mapX(cpuStart + cpuWidth / 2), getRowY(0) + rowHeight / 2 + 4);
        if (backendWidth * scaleX > 24) ctx.fillText(`F${frameIndex}`, mapX(backendStart + backendWidth / 2), getRowY(1) + rowHeight / 2 + 4);
        if (gpuWidth * scaleX > 24) ctx.fillText(`F${frameIndex}`, mapX(gpuStart + gpuWidth / 2), getRowY(2) + rowHeight / 2 + 4);
        
        // Draw a dotted vertical line from end of GPU execution to presentation line height
        const gpuEnd = gpuStart + gpuWidth;
        ctx.strokeStyle = '#10b981';
        ctx.lineWidth = 1;
        ctx.setLineDash([2, 2]);
        ctx.beginPath();
        ctx.moveTo(mapX(gpuEnd), getRowY(2) + rowHeight);
        ctx.lineTo(mapX(gpuEnd), h - paddingBottom - 10);
        ctx.stroke();
        ctx.setLineDash([]);
        
        // Draw compositor deadline target anchor circle (subtle red dot)
        const targetDeadlineTime = targetPresentTime - calc.latchMargin;
        if (calc.latchMargin > 0) {
            ctx.fillStyle = '#ef4444';
            ctx.beginPath();
            ctx.arc(mapX(targetDeadlineTime), h - paddingBottom, 3.5, 0, 2 * Math.PI);
            ctx.fill();
        }

        // Draw presentation anchor circle
        ctx.fillStyle = '#10b981';
        ctx.beginPath();
        ctx.arc(mapX(targetPresentTime), h - paddingBottom, 5, 0, 2 * Math.PI);
        ctx.fill();
        
        // Show Present target text for the frame
        ctx.fillStyle = '#10b981';
        ctx.font = '10px var(--font-sans)';
        ctx.textAlign = 'center';
        ctx.fillText(`F${frameIndex} PRESENT`, mapX(targetPresentTime), h - paddingBottom - 10);
    }
    
    // Draw three overlapping consecutive frames to demonstrate structural pipeline latency
    drawFrame(1, 0);
    drawFrame(2, 1);
    drawFrame(3, 2);
    
    // Draw Safe Delay visual indicator bracket
    if (calc.safeDelay > 0) {
        const startX = mapX(0);
        const endX = mapX(calc.safeDelay);
        const lineY = getRowY(0) - 8;
        
        ctx.strokeStyle = '#c084fc';
        ctx.lineWidth = 1.5;
        
        // Horizontal line
        ctx.beginPath();
        ctx.moveTo(startX, lineY);
        ctx.lineTo(endX, lineY);
        ctx.stroke();
        
        // End brackets
        ctx.beginPath();
        ctx.moveTo(startX, lineY - 4);
        ctx.lineTo(startX, lineY + 4);
        ctx.moveTo(endX, lineY - 4);
        ctx.lineTo(endX, lineY + 4);
        ctx.stroke();
        
        // Text
        ctx.fillStyle = '#c084fc';
        ctx.font = '9px var(--font-mono)';
        ctx.textAlign = 'left';
        ctx.fillText(`Safe Delay: ${calc.safeDelay.toFixed(1)}ms`, endX + 6, lineY + 3);
    }

    // Draw Transit Time range bracket for Frame 1 at the bottom
    const transitStartX = mapX(calc.safeDelay);
    const transitEndX = mapX(calc.safeDelay + calc.totalTransitTime);
    const transitLineY = getRowY(2) + rowHeight + 12;
    
    ctx.strokeStyle = '#9ca3af'; // Neutral slate color
    ctx.lineWidth = 1.5;
    
    // Horizontal line
    ctx.beginPath();
    ctx.moveTo(transitStartX, transitLineY);
    ctx.lineTo(transitEndX, transitLineY);
    ctx.stroke();
    
    // End brackets
    ctx.beginPath();
    ctx.moveTo(transitStartX, transitLineY - 4);
    ctx.lineTo(transitStartX, transitLineY + 4);
    ctx.moveTo(transitEndX, transitLineY - 4);
    ctx.lineTo(transitEndX, transitLineY + 4);
    ctx.stroke();
    
    // Text label
    ctx.fillStyle = '#9ca3af';
    ctx.font = '9px var(--font-mono)';
    ctx.textAlign = 'center';
    ctx.fillText(`Total Transit Time (Pipeline Latency): ${calc.totalTransitTime.toFixed(1)}ms`, transitStartX + (transitEndX - transitStartX) / 2, transitLineY + 12);
}

// Initial Update Trigger
function update() {
    draw();
}

// Application Startup
document.addEventListener('DOMContentLoaded', () => {
    // Cache base height attributes to prevent double-scaling on subsequent resizes
    document.querySelectorAll('canvas').forEach(canvas => {
        canvas.dataset.baseHeight = canvas.getAttribute('height');
    });
    
    initListeners();
    setupCanvas(els.gaussianCanvas);
    setupCanvas(els.timelineCanvas);
    update();
});
