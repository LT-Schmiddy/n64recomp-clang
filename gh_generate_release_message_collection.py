import sys
from pathlib import Path
import build_n64recomp
import build_shim_tool
import build_llvm


release_md = build_llvm.proot.joinpath("_release.md")

# Not worrying about finding this at runtime, for now:
shim_version_str = "0.1.0"
clang_version_str = "21.1.8"

release_md.write_text(f"""

# Build Info:

* Shim Tool Version: {shim_version_str}
* LLVM Version: {clang_version_str}
* LLVM Commit: [{build_llvm.llvm_commit}]({build_llvm.llvm_url}/tree/{build_llvm.llvm_commit})
* N64Recomp Commit: [{build_n64recomp.n64recomp_commit}]({build_n64recomp.n64recomp_url}/tree/{build_n64recomp.n64recomp_commit})

""".strip() + "\n")