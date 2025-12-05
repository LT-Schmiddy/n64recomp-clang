import sys, os, shutil, subprocess, platform
from pathlib import Path

import build_shim_tool as bs

shim_str = bs.get_shim_version_string()
print(f"generated_value=\"{shim_str}\"")