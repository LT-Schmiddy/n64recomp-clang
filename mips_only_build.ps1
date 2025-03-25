# Run in VS Dev Shell with Launch-VsDevShell -Arch amd64

$source_dir = "./llvm-project/llvm";
$build_dir = "./build";

Set-Variable CC=cl;
Set-Variable CXX=cl;

cmake `
	-S $source_dir `
	-B $build_dir `
	-G Ninja `
	-DCMAKE_BUILD_TYPE=Release `
	-DLLVM_DEFAULT_TARGET_TRIPLE=mips `
	-DLLVM_TARGETS_TO_BUILD=Mips `
	-DLLVM_ENABLE_PROJECTS="clang;lld;" `
	-DLLVM_USE_RELATIVE_PATHS_IN_FILES=ON `
	-DLLVM_USE_RELATIVE_PATHS_IN_DEBUG_INFO=ON `
;
cmake --build $build_dir;