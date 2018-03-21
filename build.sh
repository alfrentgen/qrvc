#!/bin/sh
#deleteandmake new build and production folders
cmake -E remove_directory build_linux_x86_32 build_linux_x86_64 build_win_x86_32 build_win_x86_64 production
cmake -E make_directory build_linux_x86_32 build_linux_x86_64 build_win_x86_32 build_win_x86_64 production
    #cmake -E remove_directory build_linux_x86_64
    #cmake -E remove_directory build_win_x86_32
    #cmake -E remove_directory build_win_x86_64
    #cmake -E remove_directory production
#configure-generate-build
#cd build_win_x86_32
#cmake -G "CodeBlocks - Unix Makefiles" -DUSE_X86_32=ON -DCMAKE_TOOLCHAIN_FILE="../cmake-mingw-toolchains/Toolchain-Ubuntu-mingw32.cmake" ..
#cmake --build .
#cd ..

#cd build_linux_x86_32
#cmake -G "CodeBlocks - Unix Makefiles" -DUSE_X86_32=ON ..
#cd ..

#cd build_win_x86_64
#cmake -G "CodeBlocks - Unix Makefiles" -DUSE_X86_32=OFF -DCMAKE_TOOLCHAIN_FILE="../cmake-mingw-toolchains/Toolchain-Ubuntu-mingw64.cmake" ..
#cd ..

#cd build_linux_x86_64
#cmake -G "CodeBlocks - Unix Makefiles" -DUSE_X86_32=OFF ..
#cd ..
#build
#copy to production
