# Around the World in 26 Languages

<!-- [![Build Status](https://github.com/cderici/around-the-world-in-26-languages/actions/workflows/athens.yml/badge.svg)](https://github.com/cderici/around-the-world-in-26-languages/actions/workflows/athens.yml) -->
![C++23](https://img.shields.io/badge/C%2B%2B-23-blue.svg)
![LLVM 20](https://img.shields.io/badge/LLVM-20-orange.svg)

This is a collection of small experimental programming languages, one for every letter of the alphabet, each named after a world city.

Each language explores different designs, features, or optimizations, and they share pieces of compiler infrastructure whenever possible. Some target `LLVM IR`, some target `PTX` for CUDA experiments, some target `MLIR`, and the first language, Athens, includes an LLVM ORC-based JIT repl.

It's my personal lab to explore ideas in language design and implementation, IR construction, optimization behavior, and runtime. It's an open-ended playground, so it will probably never be fully complete.

<br>

There's no semantic relation between the cities and the languages.

<br>

| Languages | Concept / Focus | Based&nbsp;On | Status |
|----------|-----------------|--------------|---------|
| [Athens](https://github.com/cderici/around-the-world-in-26-languages/blob/main/langs/athens/README.md)   |  Enhanced Kaleidoscope-style language targeting LLVM IR, with an LLVM ORC JIT repl.  | -- | [![Athens CI](https://github.com/cderici/around-the-world-in-26-languages/actions/workflows/athens.yml/badge.svg)](https://github.com/cderici/around-the-world-in-26-languages/actions/workflows/athens.yml) |
| Berlin   |  Produces MLIR in `affine`/`linalg` dialects for polyhedral analysis. Enables loop tiling, fusion, interchange, and dependence analysis before final lowering. | Athens | <small>Planning</small> |
| Cairo    |  This is basically Athens with LLVM IR targeting `nvptx64-nvidia-cuda`, producing `PTX` kernels via NVIDIA GPU codegen. Has an option to launch them via the CUDA driver API.  | Athens | <small>In&nbsp;Progress</small> |
| Dublin   |  A shot at *architecture-aware* design for large-scale systems. I want to focus on making architectural considerations explicit and enforcable. First-class modules, layers, dependency directions, etc, for clean separation, stable interfaces, and explicit/intentional coupling. | -- | <small>Planning</small> |
| Edinburgh|                 |
| Florence |                 |
| Geneva   |                 |
| Havana   |                 |
| Istanbul |                 |
| Jakarta  |                 |
| Kyoto    |                 |
| Lisbon   |                 |
| Montreal |                 |
| Nairobi  |                 |
| Oslo     |                 |
| Prague   |                 |
| Quito    |                 |
| Reykjavík|                 |
| Seoul    |                 |
| Tallinn  |                 |
| Utrecht  |                 |
| Vienna   |                 |
| Warsaw   |                 |
| Xanthus  |                 |
| Yerevan  |                 |
| Zürich   |                 |

