import sys
from pathlib import Path
import build_n64recomp
import build_shim_tool
import build_llvm


release_md = build_llvm.proot.joinpath("_release.md")

shim_version_str = build_shim_tool.get_shim_version_string()
if (len(sys.argv) > 1):
    shim_version_str = sys.argv[1]

clang_version_str = build_llvm.get_clang_version_string().title()
if (len(sys.argv) > 2):
    clang_version_str = sys.argv[2]

release_md.write_text(f"""

# Build Info:

* Shim Tool Version: {shim_version_str}
* LLVM Version: {clang_version_str}
* LLVM Commit: [{build_llvm.llvm_commit}]({build_llvm.llvm_url}/tree/{build_llvm.llvm_commit})
* N64Recomp Commit: [{build_n64recomp.n64recomp_commit}]({build_n64recomp.n64recomp_url}/tree/{build_n64recomp.n64recomp_commit})

""".strip() + "\n")