# CMake toolchain file: llvm-mingw cross/native build for aarch64 Windows (used by tools/build_deps.ps1).
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR ARM64)
set(LLVM_MINGW "C:/work/toolchain/llvm-mingw-20261104-ucrt-aarch64/bin" CACHE PATH "llvm-mingw bin directory")
set(CMAKE_C_COMPILER   "${LLVM_MINGW}/aarch64-w64-mingw32-clang.exe")
set(CMAKE_CXX_COMPILER "${LLVM_MINGW}/aarch64-w64-mingw32-clang++.exe")
set(CMAKE_RC_COMPILER  "${LLVM_MINGW}/aarch64-w64-mingw32-windres.exe")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
