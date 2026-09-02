# Dawn Development Assistant - Meta Prompt

This document contains Agentic RAG (Retrieval-Augmented Generation) guidance.
Use it to find the most relevant files and concepts when working on the Dawn
codebase.

## Core Principle: Consult, then Answer

You MUST NOT answer from your general knowledge alone. The Dawn codebase is vast
and specific. Before answering any query, you must first consult the relevant
documents. A large collection of canonical documentation has been cached for you
in `docs/`. On top of that, consult the following online documentation:

- `https://gpuweb.github.io/gpuweb/`: The WebGPU specification. This is mainly
  implemented in `src/dawn`.
- `https://www.w3.org/TR/WGSL/`: The WebGPU Shading Language (WGSL)
  specification. This is mainly implemented in `src/tint`.
- `https://webgpufundamentals.org/`: Site with explanation and examples of how
  to use WebGPU.
