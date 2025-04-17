from pathlib import Path
import run_build as rb

release_md = rb.proot.joinpath("_release.md")
repo_url = "https://github.com/llvm/llvm-project"

git_info = rb.get_git_info()
print(git_info)



release_md.write_text(f"""

# Build Info:

* LLVM Commit: [{git_info['commit']}]({repo_url}/tree/{git_info['commit']})

""".strip() + "\n")