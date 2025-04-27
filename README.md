# N64Recomp Clang

Binaries of LLVM tools (clang, ld.lld, etc) specially suited to [N64Recomp](https://github.com/N64Recomp/N64Recomp).

The latest official releases of LLVM for Windows no longer include much of their cross-compilation targets, including the `mips` target that N64Recomp relies on. This repository builds and releases the latest LLVM (currently 20.x) with ONLY `mips*` targets enabled, and is thus specially suited compiling N64Recomp patches and mods.

