from pathlib import Path
import build_llvm

release_md = build_llvm.proot.joinpath("_release.md")

build_llvm.download_llvm_commit()
git_info = build_llvm.get_git_info()
print(git_info)



release_md.write_text(f"""

# Build Info:

* LLVM Commit: [{git_info['commit']}]({build_llvm.llvm_url}/tree/{git_info['commit']})

""".strip() + "\n")