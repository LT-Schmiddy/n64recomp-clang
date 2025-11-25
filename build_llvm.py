import sys, os, shutil, subprocess, platform
from pathlib import Path
import build_common as bc

llvm_url = "https://github.com/llvm/llvm-project.git"
# Release 21.1.6
llvm_commit = "a832a5222e489298337fbb5876f8dcaf072c5cca"
llvm_clone_name = "clone2"

proot = Path(__file__).parent

llvm_clone_dir = proot.joinpath(f"llvm-project-{llvm_clone_name}")
llvm_dir = llvm_clone_dir.joinpath("llvm")
build_dir = llvm_dir.joinpath("build")
build_bin_dir = build_dir.joinpath("bin")
build_bin_essentials_dir = build_dir.joinpath("bin_essentials")
upload_dir = proot.joinpath("archives")

source_presets_file = proot.joinpath("./LLVM_CMakeUserPresets.json")
llvm_presets_file = llvm_dir.joinpath("./CMakeUserPresets.json")

essential_exec = [
    # Executables
    "clang._",
    "clang++._",
    "clang-cpp._",
    "clang-format._",
    "ld.lld._",
    "llvm-ar._",
    "llvm-as._",
    "llvm-objcopy._",
    "llvm-objdump._",
    "llvm-ranlib._",
    "llvm-readelf._",
    "llvm-strings._",
    "llvm-strip._",
]
essential_libs = [
    "libclang._",
    "LLVM-C._"
]

def get_clang_version_string() -> str:    
    try:
        clang_result = subprocess.run(
            [
                build_bin_dir.joinpath("clang"),
                "--version"
            ],
            cwd=build_bin_dir,
            stdout = subprocess.PIPE
        )

        bc.validate_command("Getting Built Clang Info", clang_result)
        out_str = clang_result.stdout.decode()
        name_str = out_str.split("\n")[0].split("(")[0].strip()
    except OSError as e:
        name_str = "unknown"
    
    print(f"{name_str}")
    return name_str

def get_git_info() -> dict[str, str]:
    retVal = {}
    
    tools: dict[str, str] = {}
    bc.find_tool(tools, "git")
    git_commit_result = subprocess.run(
            [
                tools["git"],
                "rev-parse",
                "HEAD"
            ],
            cwd=llvm_dir,
            stdout = subprocess.PIPE
        )
    bc.validate_command("Getting LLVM Commit", git_commit_result)
    
    git_branch_result = subprocess.run(
            [
                tools["git"],
                "branch",
                "--show-current"
            ],
            cwd=llvm_dir,
            stdout = subprocess.PIPE
        )
    bc.validate_command("Getting LLVM Branch", git_branch_result)
    
    retVal["commit"] = git_commit_result.stdout.decode().strip()
    retVal["branch"] = git_branch_result.stdout.decode().strip()
    
    return retVal


def download_llvm_commit():
    bc.download_commit(llvm_url, llvm_commit, llvm_clone_dir, "LLVM")

        
# Steps:
def build_tools(preset: str):
    print("Building...")
    tools: dict[str, str] = {}
    bc.find_tool(tools, "cmake")
    bc.find_tool(tools, "ninja")
    download_llvm_commit();
    
    shutil.copy(source_presets_file, llvm_presets_file)

    bc.validate_command(
        "CMake Configuration",
        subprocess.run(
            [
                tools["cmake"],
                "-S", str(llvm_dir),
                "-B", str(build_dir),
                "--preset", preset
            ],
            cwd=llvm_dir
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
            cwd=llvm_dir
        )
    )
    
    os.remove(llvm_presets_file)

def build_dummy():
    print("Preparing Dummy Build...")
    exec_suffix, libs_suffix = bc.get_file_extensions()
    
    os.makedirs(build_bin_dir, exist_ok=True)
    for i in essential_exec:
        src = build_bin_dir.joinpath(i).with_suffix(exec_suffix)
        src.write_text(i)
        print(f"Created dummy '{src}'...")
        
    for i in essential_libs:
        src = build_bin_dir.joinpath(i).with_suffix(libs_suffix)
        src.write_text(i)
        print(f"Created dummy '{src}'...")

def create_essentials_dir():
    print("Preparing Essentials...")
    exec_suffix, libs_suffix = bc.get_file_extensions()
    
    os.makedirs(build_bin_essentials_dir, exist_ok=True)
    for i in essential_exec:
        src = build_bin_dir.joinpath(i).with_suffix(exec_suffix)
        dst = build_bin_essentials_dir.joinpath(i).with_suffix(exec_suffix)
        if src.is_file():
            print(f"Copying '{src}' to '{dst}'...")
            shutil.copy(src, dst)
        
    for i in essential_libs:
        src = build_bin_dir.joinpath(i).with_suffix(libs_suffix)
        dst = build_bin_essentials_dir.joinpath(i).with_suffix(libs_suffix)
        if src.is_file():
            print(f"Copying '{src}' to '{dst}'...")
            shutil.copy(src, dst)

def create_release_archives():
    archive_type = "xztar"
    if os.name == 'nt':
        archive_type = "zip"
        
    name_str: str = get_clang_version_string().capitalize()
    essentials_archive_path = upload_dir.joinpath(f"{name_str.replace(' ', '_')}-MipsOnly-{platform.system()}-{platform.machine()}-N64RecompEssentials")
    full_archive_path = upload_dir.joinpath(f"{name_str.replace(' ', '_')}-MipsOnly-{platform.system()}-{platform.machine()}-Full")
    os.makedirs(upload_dir, exist_ok=True)
    
    print("Creating Essentials Release Archive...")
    shutil.make_archive(essentials_archive_path, archive_type, build_bin_essentials_dir.parent, build_bin_essentials_dir.name)
    
    print("Creating Full Release Archive...")
    shutil.make_archive(full_archive_path, archive_type, build_bin_dir.parent, build_bin_dir.name)
    
def main():
    if sys.argv[1] == "skip":
        pass
    if sys.argv[1] == "dummy":
        build_dummy()
    else:
        build_tools(sys.argv[1])
        
    create_essentials_dir()
    create_release_archives() 
    
if __name__ == '__main__':
    main()