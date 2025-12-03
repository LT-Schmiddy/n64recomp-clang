import sys, os, shutil, subprocess, platform
from pathlib import Path

# Utility Functions:
def find_tool(tools: dict[str, str], name: str):
    location = shutil.which(name);
    if location is None:
            raise RuntimeError(f"Couldn't find command '{name}'!")
    print(F"Found '{name}' at '{location}'")
    
    tools.update({name: location})

def validate_command(cmd_name: str, result: subprocess.CompletedProcess[str]):        
    if result.returncode != 0:
        raise RuntimeError(f"{cmd_name} failed!")

def get_file_extensions() -> tuple[str]:
    if platform.system() == 'Linux':
        return "", ".so"
    if platform.system() == 'Darwin':
        return "", ".dylib"
    if platform.system() == 'Windows':
        return ".exe", ".dll"
    
    raise RuntimeError(f"Couldn't determine executable file extensions for '{platform.system()=}'!")

def get_archive_type() -> str:
    archive_type = "xztar"
    if os.name == 'nt':
       archive_type = "zip"
        
    return archive_type

def download_commit(repo_url: str, commit_id: str, clone_dir: Path, repo_name: str = "Repo"):
    tools: dict[str, str] = {}
    find_tool(tools, "git")
    if not clone_dir.exists():
        # Downloading LLVM Source
        validate_command(
            f"Downloading {repo_name} Source",
            subprocess.run(
                [
                    tools["git"],
                    "clone", repo_url,
                    clone_dir
                ],
            )
        )
    else:
        print(f"{repo_name} Source already downloaded...",)
    
    # Checking Out Commit
    validate_command(
        f"Checking Out Commit {commit_id}",
        subprocess.run(
            [
                tools["git"],
                "checkout", commit_id
            ],
            cwd=clone_dir
        )
    )
    
    # Fetching Submodules
    validate_command(
        f"Fetching {repo_name} Submodules",
        subprocess.run(
            [
                tools["git"],
                "submodule",
                "update",
                "--init",
                "--recursive"
            ],
            cwd=clone_dir
        )
    )
