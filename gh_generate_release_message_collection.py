from pathlib import Path
import build_n64recomp
import build_llvm


release_md = build_llvm.proot.joinpath("_release.md")


release_md.write_text(f"""

# Build Info:

* LLVM Commit: [{build_llvm.llvm_commit}]({build_llvm.llvm_url}/tree/{build_llvm.llvm_commit})
* N64Recomp Commit: [{build_n64recomp.n64recomp_commit}]({build_n64recomp.n64recomp_url}/tree/{build_n64recomp.n64recomp_commit})

""".strip() + "\n")