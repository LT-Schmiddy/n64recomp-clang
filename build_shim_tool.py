import sys, os, shutil, subprocess, platform
from pathlib import Path
import build_common as bc

proot = Path(__file__).parent

shim_tool_dir = proot.joinpath(f"shim_tool")
build_dir = shim_tool_dir.joinpath("build/ci")
build_bin_dir = build_dir.joinpath("bin")

# In case we add more tools later.
exec_names = [
    "nrs._"
]

def get_shim_version_string() -> str:    
    try:
        nrs_result = subprocess.run(
            [
                build_bin_dir.joinpath("nrs"),
                "--version"
            ],
            cwd=build_bin_dir,
            stdout = subprocess.PIPE
        )

        bc.validate_command("Getting Built Shim Info", nrs_result)
        out_str = nrs_result.stdout.decode()
        name_str = out_str.split("\n")[0].split("\r")[0]
    except OSError as e:
        name_str = "unknown"
    
    return name_str

def build_tools(preset: str):
    print("Building...")
    tools: dict[str, str] = {}
    bc.find_tool(tools, "cmake")
    bc.find_tool(tools, "ninja")
    bc.validate_command(
        "Shim Tool CMake Configuration",
        subprocess.run(
            [
                tools["cmake"],
                "-S", str(shim_tool_dir),
                "-B", str(build_dir),
                "--preset", preset + "-Release"
            ],
            cwd=shim_tool_dir
        )
    )
    
    bc.validate_command(
        "Shim Tool CMake Build",
        subprocess.run(
            [
                tools["cmake"],
                "--build",
                "."
            ],
            cwd=build_dir
        )
    )
    
def get_binaries() -> list[Path]:
    retVal = []
    exec_suffix, libs_suffix = bc.get_file_extensions()
    
    for i in exec_names:
        binary = build_bin_dir.joinpath(i).with_suffix(exec_suffix)
        if binary.is_file():
            retVal.append(binary)
    
    return retVal

def main():
    build_tools(sys.argv[1])
    
if __name__ == '__main__':
    main()