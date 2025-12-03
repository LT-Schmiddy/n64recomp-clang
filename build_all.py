import sys, os, shutil, subprocess, platform
from pathlib import Path

import build_common as bc
import build_shim_tool as bs
import build_n64recomp as bn
import build_llvm as bl

proot = Path(__file__).parent
archives_dir = proot.joinpath("archives")
build_archives = proot.joinpath("build_archives")
n64recomp_tools_only_dir = build_archives.joinpath("n64recomp_tools_only_dir")
recomp_essentials_dir = build_archives.joinpath("recomp_essentials_dir")
recomp_all_dir = build_archives.joinpath("recomp_all_dir")

archive_root_name = "n64recomp_tools"
archive_bin_name = "nrs_bin"

tools_only_prefix = "N64RecompToolsOnly"
essentials_prefix = "N64RecompAndClangEssentials"
full_prefix = "N64RecompAndClangFull"

def build_archive(name: str, archive_parent: Path, included_binaries: list[Path]):
    archive_root = archive_parent.joinpath(archive_root_name)
    archive_bin = archive_root.joinpath(archive_bin_name)
    
    os.makedirs(archive_bin, exist_ok=True)
    
    shim_binaries = bs.get_binaries()
    
    for src in shim_binaries:
        dst = archive_root.joinpath(src.name)
        print(f"Copying '{src}' to '{dst}'...")
        shutil.copy(src, dst)
    
    for src in included_binaries:
        dst = archive_bin.joinpath(src.name)
        print(f"Copying '{src}' to '{dst}'...")
        shutil.copy(src, dst)

    print(f"Creating '{name}' from {archive_root}")
    for i in os.listdir(archive_root):
        print(i)
    shutil.make_archive(archives_dir.joinpath(name), bc.get_archive_type(), archive_root, "")

def build_all_archives():
    os.makedirs(archives_dir, exist_ok=True)
    os.makedirs(build_archives, exist_ok=True)
    
    n64recomp_binaries = bn.get_binaries()
    recomp_essentials_binaries = bl.get_essential_binaries()
    recomp_all_binaries = bl.get_all_binaries()
    
    clang_ver: str = bl.get_clang_version_string().title().replace(' ', '')
    toolsonly_name = f"{tools_only_prefix}-{platform.system()}-{platform.machine()}"
    essentials_name = f"{essentials_prefix}-{clang_ver}-MipsOnly-{platform.system()}-{platform.machine()}"
    full_name = f"{full_prefix}-{clang_ver}-MipsOnly-{platform.system()}-{platform.machine()}"
    
    # Recomp Tools Only:
    build_archive(toolsonly_name, n64recomp_tools_only_dir, n64recomp_binaries)
    build_archive(essentials_name, recomp_essentials_dir, n64recomp_binaries + recomp_essentials_binaries)
    build_archive(full_name, recomp_all_dir, n64recomp_binaries + recomp_all_binaries)

def main():
    bs.build_tools(sys.argv[1])
    bn.build_tools(sys.argv[1])
    # bl.build_tools(sys.argv[1])
    bl.build_dummy()
        
    build_all_archives()
    
if __name__ == '__main__':
    main()
    
