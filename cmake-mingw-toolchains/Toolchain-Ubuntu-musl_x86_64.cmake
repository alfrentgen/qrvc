# Sample toolchain file for building for Windows from an Ubuntu Linux system.
#
# Typical usage:
#    *) install cross compiler: `sudo apt-get install mingw-w64 g++-mingw-w64`
#    *) cd build
#    *) cmake -DCMAKE_TOOLCHAIN_FILE=~/Toolchain-Ubuntu-mingw32.cmake ..

set(CMAKE_SYSTEM_NAME Linux)
set(TOOLCHAIN_PREFIX x86_64-linux-musl)

# cross compilers to use for C and C++
set(CMAKE_C_COMPILER /opt/musl_toolch_x86_64/bin/${TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER /opt/musl_toolch_x86_64/bin/${TOOLCHAIN_PREFIX}-g++)
#set(CMAKE_RC_COMPILER ${TOOLCHAIN_PREFIX}-windres)

# target environment on the build host system
#   set 1st to dir with the cross compiler's C/C++ headers/libs
set(CMAKE_FIND_ROOT_PATH /opt/musl_toolch_x86_64)#/${TOOLCHAIN_PREFIX})

# modify default behavior of FIND_XXX() commands to
# search for headers/libs in the target environment and
# search for programs in the build host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

