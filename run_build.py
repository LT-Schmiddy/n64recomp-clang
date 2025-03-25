import os, shutil, subprocess, platform
from pathlib import Path

proot = Path(__file__).parent
llvm_dir = proot.joinpath("llvm-project/llvm")
build_dir = proot.joinpath("build")
build_bin_dir = build_dir.joinpath("bin")
build_bin_essentials_dir = build_dir.joinpath("bin", "essentials")

essential_exec = [
    # Executables
    "clang",
    "clang++",
    "clang-cpp",
    "clang-format",
    "ld.lld",
    "llvm-ar",
    "llvm-as",
    "llvm-objcopy",
    "llvm-objdump",
    "llvm-ranlib",
    "llvm-readelf",
    "llvm-strings",
    "llvm-strip",
]
essential_libs = [
    "libclang",
    "LLVM-C"
]
# Utility Functions:
def find_tool(tools: dict[str, str], name: str):
    location = shutil.which(name);
    if location is None:
            raise RuntimeError(f"Couldn't find command '{name}'!")
    print(F"Found '{name}' at '{location}'")
    
    tools.update({name: location})

def validate_command(cmd_name: str, result: subprocess.CompletedProcess[str]):        
    if result.returncode != 0:
        raise RuntimeError(f"{cmd_name} falied!")

def get_file_extensions() -> tuple[str]:
    if platform.system() == 'Linux':
        return "", ".so"
    if platform.system() == 'Darwin':
        return "", ".dylib"
    if platform.system() == 'Windows':
        return ".exe", ".dll"
    
    raise RuntimeError(f"Couldn't determine executable file extensions for '{platform.system()=}'!")

# Steps:
def build_tools():
    print("Building...")
    tools: dict[str, str] = {}
    find_tool(tools, "cmake")
    find_tool(tools, "ninja")
    
    env_dict = os.environ.copy().update({
        "CXX":  "cl",
        "CXX":  "cl"
    })
    
    validate_command(
        "CMake Configuration",
        subprocess.run(
            [
                tools["cmake"],
                "-S", str(llvm_dir),
                "-B", str(build_dir),
                "-G", "Ninja",
                "-DCMAKE_BUILD_TYPE=Release",
                "-DLLVM_DEFAULT_TARGET_TRIPLE=mips",
                "-DLLVM_TARGETS_TO_BUILD=Mips",
                "-DLLVM_ENABLE_PROJECTS=clang;lld",
                "-DLLVM_USE_RELATIVE_PATHS_IN_FILES=ON",
                "-DLLVM_USE_RELATIVE_PATHS_IN_DEBUG_INFO=ON"
            ],
            cwd=proot,
            env=env_dict
        )
    )
    
    validate_command(
        "CMake Build",
        subprocess.run(
            [
                tools["cmake"],
                "--build",
                str(build_dir)
            ],
            cwd=proot,
            env=env_dict
        )
    )
    
def create_essentials_dir():
    print("Preparing Essentials...")
    exec_suffix, libs_suffix = get_file_extensions()
    
    os.makedirs(build_bin_essentials_dir, exist_ok=True)
    for i in essential_exec:
        src = build_bin_dir.joinpath(i).with_suffix(exec_suffix)
        dst = build_bin_essentials_dir.joinpath(i).with_suffix(exec_suffix)
        print(f"Copying '{src}' to '{dst}'...")
        shutil.copy(src, dst)
        
    for i in essential_libs:
        src = build_bin_dir.joinpath(i).with_suffix(libs_suffix)
        dst = build_bin_essentials_dir.joinpath(i).with_suffix(libs_suffix)
        print(f"Copying '{src}' to '{dst}'...")
        shutil.copy(src, dst)
    
def main():
    build_tools()
    create_essentials_dir()
    
if __name__ == '__main__':
    main()