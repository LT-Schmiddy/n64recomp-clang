import sys, os, shutil, subprocess, platform
from pathlib import Path
import build_common as bc

n64recomp_url = "https://github.com/N64Recomp/N64Recomp.git"
n64recomp_commit = "f14ffe6064881e29e4e7da016be525f6c04f4369"
n64recomp_clone_name = "clone2"

proot = Path(__file__).parent

n64recomp_clone_dir = proot.joinpath(f"n64recomp-{n64recomp_clone_name}")
build_dir = n64recomp_clone_dir.joinpath("build")
build_bin_dir = build_dir.joinpath("bin")
upload_dir = proot.joinpath("archives")

source_presets_file = proot.joinpath("./N64Recomp_CMakeUserPresets.json")
n64recomp_presets_file = n64recomp_clone_dir.joinpath("./CMakeUserPresets.json")


def build_tools(preset: str):
    print("Building...")
    tools: dict[str, str] = {}
    bc.find_tool(tools, "cmake")
    bc.find_tool(tools, "ninja")
    bc.download_commit(n64recomp_url, n64recomp_commit, n64recomp_clone_dir, "N64Recomp");
    
    shutil.copy(source_presets_file, n64recomp_presets_file)

    bc.validate_command(
        "CMake Configuration",
        subprocess.run(
            [
                tools["cmake"],
                "-S", str(n64recomp_clone_dir),
                "-B", str(build_dir),
                "--preset", preset
            ],
            cwd=n64recomp_clone_dir
        )
    )
    
    bc.validate_command(
        "CMake Build",
        subprocess.run(
            [
                tools["cmake"],
                "--build",
                "--preset", preset,
            ],
            cwd=n64recomp_clone_dir
        )
    )
    
    os.remove(n64recomp_presets_file)


def main():
    build_tools(sys.argv[1])
    
if __name__ == '__main__':
    main()