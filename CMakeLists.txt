cmake_minimum_required(VERSION 3.5)

message(STATUS ${GENERATOR})

project("QR video codec" C CXX)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_SKIP_RPATH TRUE)

option(USE_X86_32 "Build for x86 32 bits" OFF)
option(USE_MUSL "Build using musl-cross-make" OFF)
#option(USE_NATIVE "Build using native GCC toolset" OFF)

option(ENABLE_TESTS "Enable tests" OFF)

set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -fexceptions -fpermissive -static-libstdc++ -static-libgcc -mtune=generic")
set(CMAKE_C_FLAGS "-std=gnu11 -Wall -fexceptions -fpermissive -static-libstdc++ -static-libgcc -mtune=generic")
set(CMAKE_SHARED_LINKER_FLAGS "-static-libstdc++ -static-libgcc")
set(CMAKE_EXE_LINKER_FLAGS "-static-libstdc++ -static-libgcc")

if(USE_MINGW)
    set(MINGW_BUILD_FLAGS "-static -pthread")
    if(USE_X86_32)
        set(MINGW_BUILD_FLAGS "-O1 -fno-builtin-memmove ${MINGW_BUILD_FLAGS}")
    else()
        set(MINGW_BUILD_FLAGS "-O3 ${MINGW_BUILD_FLAGS}")
    endif()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MINGW_BUILD_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${MINGW_BUILD_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${MINGW_BUILD_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS "-static")
else()
    set(MUSL_BUILD_FLAGS "-pthread")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 ${MUSL_BUILD_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 ${MUSL_BUILD_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${MUSL_BUILD_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${MUSL_BUILD_FLAGS}")
endif()

if(USE_X86_32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32 -march=i686 -mfpmath=sse -msse -msse2")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32 -march=i686 -mfpmath=sse -msse -msse2")
    set(TARBALL_ARCH "x86_32")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64")
    set(TARBALL_ARCH "x86_64")
endif()

message(STATUS "CMAKE_FIND_ROOT_PATH ${CMAKE_FIND_ROOT_PATH}")
message(STATUS "CMAKE_CXX_COMPILER ${CMAKE_CXX_COMPILER}")
message(STATUS "CMAKE_C_COMPILER ${CMAKE_C_COMPILER}")
message(STATUS "CMAKE_LINKER ${CMAKE_LINKER}")
message(STATUS "CMAKE_CXX_LINK_EXECUTABLE ${CMAKE_CXX_LINK_EXECUTABLE}")
message(STATUS "CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_C_FLAGS ${CMAKE_C_FLAGS}")
message(STATUS "CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS}")

add_subdirectory(libs)

if(USE_MUSL)
    set(LD_MUSL_LINK "ld-musl.so")
    add_custom_target(libc ALL
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_FIND_ROOT_PATH}/lib/libc.so" "libc.so"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_FIND_ROOT_PATH}/lib/libc.so" "libc.so"
        COMMAND ${CMAKE_COMMAND} -E remove "-f" "${LD_MUSL_LINK}"
        COMMAND ln "-s" "./libc.so" "${LD_MUSL_LINK}"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()

set(DEC_HDR
include/pch.h
include/Config.h
include/ArgsParser.h
include/Chunk.h
include/Job.h
include/Decode.h
include/inputQueue.h
include/OutputQueue.h
include/MTDecoder.h
include/utilities.h
include/utilities_c.h
include/help.h
)

set(DEC_SRC
src/ArgsParser.cpp
src/Chunk.cpp
src/Job.cpp
src/Decode.cpp
src/inputQueue.cpp
src/OutputQueue.cpp
src/MTDecoder.cpp
src/Config.cpp
)

add_executable(qrvd ${DEC_HDR} ${DEC_SRC})
target_include_directories(qrvd PUBLIC include)
set_target_properties(qrvd PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(qrvd decquirc deczbar)
if(USE_MINGW)
    set_target_properties(qrvd PROPERTIES LINK_FLAGS "-Wl,--allow-multiple-definition")
elseif(USE_MUSL)
    set_target_properties(qrvd PROPERTIES LINK_FLAGS "-Wl,-rpath,'$ORIGIN' -Wl,--dynamic-linker,'${LD_MUSL_LINK}'")
    add_dependencies(qrvd libc)
endif()


SET(MTENC_HDR
include/pch.h
include/Config.h
include/ArgsParser.h
include/Chunk.h
include/Job.h
include/Encode.h
include/inputQueue.h
include/OutputQueue.h
include/MTEncoder.h
include/utilities.h
include/utilities_c.h
include/help.h
include/steganography.h
)

SET(MTENC_SRC
src/ArgsParser.cpp
src/Chunk.cpp
src/Job.cpp
src/Encode.cpp
src/inputQueue.cpp
src/OutputQueue.cpp
src/MTEncoder.cpp
src/Config.cpp
src/steganography.cpp
)

add_executable(qrve ${MTENC_HDR} ${MTENC_SRC})
target_include_directories(qrve PUBLIC include)
set_target_properties(qrve PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(qrve qrenc)
if(USE_MUSL)
    set_target_properties(qrve PROPERTIES LINK_FLAGS "-Wl,-rpath,'$ORIGIN' -Wl,--dynamic-linker,'${LD_MUSL_LINK}'")
    add_dependencies(qrve libc)
endif()

#TESTS SECTION
set(TESTS_PATH  ${CMAKE_SOURCE_DIR}/tests)

#package making target
if(MINGW)
    set(TARBALL_NAME qrvc_win)
    set(LIBS_TO_TAR *.dll)
else()
    set(TARBALL_NAME qrvc_linux)
    set(LIBS_TO_TAR *.so)
endif()
set(TARBALL_NAME "${TARBALL_NAME}_${TARBALL_ARCH}")
message(STATUS "TARBALL_NAME= ${TARBALL_NAME}")

add_custom_target(create_tar ALL
    COMMAND ${CMAKE_COMMAND} -E remove "-f" "versions.txt"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/versions.txt" "versions.txt"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/README" "README"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/LICENSE" "LICENSE"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/licenses" "${CMAKE_BINARY_DIR}/licenses"
    COMMAND ${CMAKE_COMMAND} -E tar "-cfvz" "${TARBALL_NAME}.tar.gz" "$<TARGET_FILE:qrve>" "$<TARGET_FILE:qrvd>" "versions.txt" "LICENSE"
    "README" "licenses" "${LIBS_TO_TAR}"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
add_dependencies(create_tar qrvd qrve)

if(ENABLE_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

