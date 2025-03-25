$src_dir = "./build/bin";
$dst_dir = "./build/bin_min";

$myArray = @(
    # Executables
    "clang.exe",
    "clang++.exe",
    "clang-cpp.exe",
    "clang-format.exe",
    "ld.lld.exe",
    "llvm-ar.exe",
    "llvm-as.exe",
    "llvm-objcopy.exe",
    "llvm-objdump.exe",
    "llvm-ranlib.exe",
    "llvm-readelf.exe",
    "llvm-strings.exe",
    "llvm-strip.exe",
    
    # DLLs
    "libclang.dll",
    "LLVM-C.dll"
);

New-Item -ItemType Directory -Force -Path $dst_dir;
foreach ($item in $myArray) {
    Write-Output "Src: $($src_dir)/$item; Dst: $($dst_dir)/$item";
    Copy-Item -Path "$($src_dir)/$item" -Destination "$($dst_dir)/$item";
}