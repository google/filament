# Filament Workspace Guidelines

For all tasks involving writing, editing, or refactoring code that uses the Filament rendering engine, you must strictly adhere to the guidelines defined in [docs/AI_CONTEXT.md](file:///Users/mathias/sources/git/filament/docs/AI_CONTEXT.md).

## Key Directives:
1. Always follow the explicit ECS Entity component lifecycle and destruction sequences.
2. Verify that all Engine-managed API objects are destroyed via `engine->destroy(ptr)` rather than deleted directly.
3. Use `filament::math` types (vectors, matrices, quaternions) instead of external types like GLM.
4. Align vertex attributes in your `VertexBuffer` builders with custom material definition (.mat) files.

See [docs/AI_CONTEXT.md](file:///Users/mathias/sources/git/filament/docs/AI_CONTEXT.md) for full context and code snippets.
