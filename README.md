# Around the World in 26 Languages

[![Build Status](https://github.com/cderici/around-the-world-in-26-languages/actions/workflows/build.yml/badge.svg)](https://github.com/cderici/around-the-world-in-26-languages/actions/workflows/build.yml)
![C++23](https://img.shields.io/badge/C%2B%2B-23-blue.svg)
![LLVM 20](https://img.shields.io/badge/LLVM-20-orange.svg)

This is my collection of small experimental programming languages, one for every letter of the alphabet, each named after a city and written in C++23. (The city names have no semantic relation to the languages.)

Each language explores different designs, features, or optimizations, and they share pieces of compiler infrastructure whenever possible. Some target `LLVM IR`, some target `PTX` for CUDA experiments, some target `MLIR`, and some include an LLVM ORC-based JIT repl.

This project is my space to explore ideas in language design, IR construction, optimization behavior, and runtime implementation. It's an open-ended playground, so it will probably never be fully complete.

<br>

| Languages | Concept / Focus | Based&nbsp;On | Status |
|----------|-----------------|--------------|---------|
| Athens   |  Enhanced Kaleidoscope-based language with an LLVM ORC JIT repl; forms a base for other languages.  | -- | <small>Working</small> |
| Berlin   |  Lowers affine loops into MLIR suitable for polyhedral analysis using `affine` and `linalg` dialects. Enables loop tiling, fusion, interchange, and dependence analysis before final lowering. | Athens | <small>In&nbsp;Progress</small> |
| Cairo    |  Compiles to LLVM IR targeting `nvptx64-nvidia-cuda`, producing `PTX` kernels via NVIDIA GPU codegen path. Has an option to launch them via the CUDA driver API.  | Athens | <small>Planning</small> |
| Dublin   |                 |
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

