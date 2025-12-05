import sys, os, shutil, subprocess, platform
from pathlib import Path

import build_llvm as bl

clang_str = bl.get_clang_version_string().title()
print(f"generated_value=\"{clang_str}\"")