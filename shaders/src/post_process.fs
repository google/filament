void main() {
    PostProcessInputs inputs;
    inputs.color = vec4(1.0);

    // Invoke user code
    postProcess(inputs);

    fragColor = inputs.color;
}
