# Workspace Guidelines for Filament SDK Projects

This file defines machine-readable rules and instructions for AI agents (like Antigravity or standard MCP-equipped systems) when working on this codebase. It ensures they use the Filament SDK correctly.

---

## Filament AI SDK Rules

AI agents **must** consult and follow the corresponding skill files under the `skills/` directory:

### 1. Lifetime Management
*   [skills/filament_lifetime/SKILL.md](skills/filament_lifetime/SKILL.md): Core resource management instructions enforcing correct creation and destruction cycles for Filament components and engine objects.

### 2. Math Conventions
*   [skills/filament_math/SKILL.md](skills/filament_math/SKILL.md): Math primitives and namespace constraints (`filament::math` types instead of third-party templates).
