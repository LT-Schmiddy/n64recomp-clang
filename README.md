# N64Recomp Clang

Binaries of LLVM tools (clang, ld.lld, etc) specially suited to [N64Recomp](https://github.com/N64Recomp/N64Recomp).

The latest official releases of LLVM for Windows no longer include much of their cross-compilation targets, including the `mips` target that N64Recomp relies on. This repository builds and releases the latest LLVM (currently 20.x) with ONLY `mips*` targets enabled, and is thus specially suited compiling N64Recomp patches and mods.

Currently, this repository only produces x86_64 Windows builds. The `mips` is still included by the latest Linux and Clang release builds of LLVM, and therefore users of those operating systems should still use the official releases.
