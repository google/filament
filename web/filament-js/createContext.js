Filament.createContext = (canvas, options) => {
    const ctx = canvas.getContext("webgl2", options);
    const handle = GL.registerContext(ctx, options);
    GL.makeContextCurrent(handle);
    return ctx;
};
